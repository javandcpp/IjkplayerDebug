//
// Created by developer on 11/13/17.
//

#include "AudioEncoder.h"
#include "AudioEncodeArgs.h"

AudioEncoder *AudioEncoder::Get() {
    static AudioEncoder audioEncoder;
    return &audioEncoder;
}

AudioEncoder::AudioEncoder() {

}

AudioEncoder::~AudioEncoder() {
}

//AudioCapture *AudioEncoder::GetAudioCapture() {
//    return this->audioCapture;
//}

int AudioEncoder::EncodeAAC(AVData **originData) {

    int ret = 0;
    ret = avcodec_fill_audio_frame(outputFrame,
                                   audioCodecContext->channels,
                                   audioCodecContext->sample_fmt, (*originData)->data,
                                   8192, 0);
    outputFrame->pts = (*originData)->pts;
    ret = avcodec_send_frame(audioCodecContext, outputFrame);
    if (ret != 0) {
#ifdef SHOW_DEBUG_INFO
        LOG_D(DEBUG, "send frame failed!");
#endif
    }
    av_packet_unref(&audioPacket);

    ret = avcodec_receive_packet(audioCodecContext, &audioPacket);
    if (ret != 0) {
#ifdef SHOW_DEBUG_INFO
        LOG_D(DEBUG, "receive packet failed!");
#endif
    }
    (*originData)->Drop();
    (*originData)->data = (unsigned char *) &audioPacket;

#ifdef SHOW_DEBUG_INFO
    LOG_D(DEBUG, "encode audio packet size:%d pts:%lld", (*originData)->avPacket->size,
          (*originData)->avPacket->pts);
    LOG_D(DEBUG, "Audio frame encode success!");
#endif
//    (*originData)->avPacket->size;
    return audioPacket.size;
}

void AudioEncoder::update(AVData avData) {
    if (!avData.isAudio) {
        return;
    }
    while (!isExit) {
        if (aAudioframeQueue.Size() < maxList) {
            aAudioframeQueue.push(avData);
            LOGE("update push audio queue data,pts:%ld   listsize:%d", avData.pts,
                 aAudioframeQueue.Size());
            break;
        }
        xsleep(1);
    }
}


void AudioEncoder::main() {
    while (!isExit) {
        //判断音视频同步
//        if (!isAudio && syncAudioPts > 0) {
//            //如果音频PTS小于视频，停止视频编码
//            if (syncAudioPts < pts) {
//                continue;
//            }
//        }

        if (aAudioframeQueue.empty()) {
            xsleep(1);
            continue;
        }

        const shared_ptr<AVData> &ptr = aAudioframeQueue.wait_and_pop();
        AVData *pData = ptr.get();
        int ret = -1;
        ret = avcodec_fill_audio_frame(outputFrame,
                                       audioCodecContext->channels,
                                       audioCodecContext->sample_fmt, *(pData->datas),
                                       pData->size, 0);
        LOGE("audio encode %d", pData->size);
        if (ret < 0) {
            LOGE("audio fill frame failed!");
            continue;
        }
        //发送数据到解码线程，一个数据包，可能解码多个结果
        ret = avcodec_send_frame(audioCodecContext, outputFrame);
        LOGE("audio enencode avcodec_send_frame result:%d", ret);
        if (ret == 0) {
            while (!isExit) {
                //获取解码数据
                ret = avcodec_receive_packet(audioCodecContext, &audioPacket);
                if (ret != 0) break;

                AVData avData;
                LOGD("audio encode sucess  pts:%ld",pData->pts);
                avData.pts = pData->pts;
                AVPacket *avPacket=av_packet_alloc();
                memcpy(avPacket,&audioPacket, sizeof(avPacket));
                avData.data = (unsigned char *) avPacket;
                av_packet_unref(&audioPacket);
                this->notifyObserver(avData);
            }
        }
//        pData->Drop();
    }
}

//void *AudioEncoder::EncodeTask(void *p) {
//    AudioEncoder *audioEncoder = (AudioEncoder *) p;
//    audioEncoder->isEncoding = true;
//    AudioEncodeArgs *args = audioEncoder->audioCapture->GetAudioEncodeArgs();
//
//    FILE *PCM = fopen("/mnt/sdcard/iaudio.pcm", "wb+");
//    LOGD("begin audio encode");
//    audioEncoder->audioCapture->audioCaputureframeQueue.clear();
//    int64_t beginTime = av_gettime();
//    int64_t lastPts = 0;
//    while (true) {
//        //线程中断标记
//        if (audioEncoder->audioCapture->GetCaptureState()) {
//            break;
//        }
//        //获取音频采集中的对列中的数据
//        if (audioEncoder->audioCapture->audioCaputureframeQueue.empty()) {
//            continue;
//        }
//        AVData *srcData = audioEncoder->audioCapture->GetAudioData();
//        if (NULL == srcData) {
//#ifdef SHOW_DEBUG_INFO
//            LOG_D(DEBUG, "audio data NULL");
//#endif
//            continue;
//        }
//        if (NULL == srcData->data) {
//#ifdef SHOW_DEBUG_INFO
//            LOG_D(DEBUG, "audio src data NULL");
//#endif
//            continue;
//        }
//        audioEncoder->outputFrame->pts = srcData->pts - beginTime;
//        if (lastPts == audioEncoder->outputFrame->pts) {
//            audioEncoder->outputFrame->pts += 1300;
//        }
//        lastPts = audioEncoder->outputFrame->pts;
//
//        int ret = 0;
//        ret = avcodec_fill_audio_frame(audioEncoder->outputFrame,
//                                       audioEncoder->audioCodecContext->channels,
//                                       audioEncoder->audioCodecContext->sample_fmt, srcData->data,
//                                       4096, 0);
//        if (ret < 0) {
//            LOGE("fill frame failed!");
//            continue;
//        }
//
//#ifdef PTS_INFO
//        LOG_D(DEBUG, "audio pts:%lld", audioEncoder->outputFrame->pts);
//#endif
//        ret = avcodec_send_frame(audioEncoder->audioCodecContext, audioEncoder->outputFrame);
//        if (ret != 0) {
//#ifdef SHOW_DEBUG_INFO
//            LOG_D(DEBUG, "send frame failed!");
//#endif
//            continue;
//        }
//        av_packet_unref(&audioEncoder->audioPacket);
//
//        ret = avcodec_receive_packet(audioEncoder->audioCodecContext, &audioEncoder->audioPacket);
//        if (ret != 0) {
//#ifdef SHOW_DEBUG_INFO
//            LOG_D(DEBUG, "receive packet failed!");
//#endif
//            continue;
//        }
//        srcData->Drop();
////        //TODO 编码完成(数据传递问题不知道有没有问题)
//        srcData->avPacket = &audioEncoder->audioPacket;
//#ifdef SHOW_DEBUG_INFO
//        LOG_D(DEBUG, "encode audio packet size:%d pts:%lld", srcData->avPacket->size,
//              srcData->avPacket->pts);
//        LOG_D(DEBUG, "Audio frame encode success!");
//#endif
//        audioEncoder->aframeQueue.push(srcData);
//
//#ifdef WRITE_PCM_TO_FILE
//        fwrite(audioEncoder->outputFrame->data[0], 1, 4096, PCM);//V
//        fflush(PCM);
//#endif
//    }
//#ifdef WRITE_PCM_TO_FILE
//    fclose(PCM);
//#endif
//    return 0;
//}
//
int AudioEncoder::StartEncode() {
//    std::lock_guard<std::mutex> lk(mtx);
//    pthread_t t1;
//    pthread_create(&t1, NULL, AudioEncoder::EncodeTask, this);
//    LOGD("Start Audio Encode task!");
    return 0;
}


int AudioEncoder::InitEncode(AVCodecParameters *avCodecParameters) {
    std::lock_guard<std::mutex> lk(mtx);
//    avCodec = avcodec_find_encoder(avCodecParameters->codec_id);
    avCodec = avcodec_find_encoder_by_name("libfdk_aac");

    int ret = 0;
    if (!avCodec) {
        LOGE("aac encoder not found!");
        return -1;
    }
    audioCodecContext = avcodec_alloc_context3(avCodec);
    if (!audioCodecContext) {
        LOGE("avcodec alloc context3 failed!");
        return -1;
    }
    audioCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    audioCodecContext->sample_fmt = AV_SAMPLE_FMT_S16;
    audioCodecContext->sample_rate = 48000;
    audioCodecContext->thread_count = 8;
    audioCodecContext->bit_rate = 50 * 1024 * 8;
    audioCodecContext->channels = 2;
    audioCodecContext->frame_size = 1024;
    audioCodecContext->time_base = {1, 1000000};//AUDIO VIDEO 两边时间基数要相同
    audioCodecContext->channel_layout = av_get_default_channel_layout(audioCodecContext->channels);

    outputFrame = av_frame_alloc();
    outputFrame->channels = audioCodecContext->channels;
    outputFrame->channel_layout = av_get_default_channel_layout(audioCodecContext->channels);
    outputFrame->format = audioCodecContext->sample_fmt;
    outputFrame->nb_samples = 1024;
    ret = av_frame_get_buffer(outputFrame, 0);
    if (ret != 0) {
        LOGE("av_frame_get_buffer failed!");
        return -1;
    }
    LOGD("av_frame_get_buffer success!");
    ret = avcodec_open2(audioCodecContext, NULL, NULL);
    if (ret != 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf));
        LOGE("avcodec open failed! info:%s", buf);
        return -1;
    }
    LOGD("open audio codec success!");
    LOGD("Complete init Audio Encode!");
    return 0;
}

int AudioEncoder::Release() {
    LOGD("Release Audio Encode!");
    return 0;
}

int AudioEncoder::CloseEncode() {
    std::lock_guard<std::mutex> lk(mtx);
    if (isEncoding) {
        isEncoding = false;
        avcodec_close(audioCodecContext);
        LOGD("Close Audio Encode!");
    }
    return 0;
}

bool AudioEncoder::GetEncodeState() {
    return isEncoding;
}


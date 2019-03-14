//
// Created by developer on 11/13/17.
//

#include "AudioEncoder.h"
#include "AudioEncodeArgs.h"


AudioEncoder::AudioEncoder() {
//    pFile = fopen("/mnt/sdcard/test1.aac", "wb+");
}

AudioEncoder::~AudioEncoder() {
    LOG_E("audioencoder release");
}


void AudioEncoder::update(AVData avData) {
    mtx.lock();
    if (!avData.isAudio) {
        mtx.unlock();
        return;
    }
    while (!isExit) {
        aAudioframeQueue.push(avData);
        LOGE("update push audio queue data,pts:%ld   listsize:%d", avData.pts,
             aAudioframeQueue.Size());
        break;
    }
    mtx.unlock();
}


void AudioEncoder::main() {
#if 0
    struct  timeval    tv;
    gettimeofday(&tv,NULL);
    int frameCount=0;
#endif

    while (!isExit) {

        if (aAudioframeQueue.empty()) {
            xsleep(1);
            continue;
        }

        LOG_E("audio encode queue size:%d",aAudioframeQueue.Size());
        const shared_ptr<AVData> &pts = aAudioframeQueue.wait_and_pop();
        AVData *pData = pts.get();
//
        int ret = -1;
        ret = avcodec_fill_audio_frame(outputFrame,
                                       audioCodecContext->channels,
                                       audioCodecContext->sample_fmt, pData->datas[0],
                                       pData->size, 0);
        LOGE("audio encode %d", pData->size);
        if (ret < 0) {
            LOGE("audio fill frame failed!");
            continue;
        }



        //发送数据到解码线程，一个数据包，可能解码多个结果
//        outputFrame->pts = pData->pts;
        audioPts = pData->pts;
        ret = avcodec_send_frame(audioCodecContext, outputFrame);
        LOGE("audio enencode avcodec_send_frame result:%d", ret);
        if (ret == 0) {
            while (!isExit) {
                //获取解码数据
                ret = avcodec_receive_packet(audioCodecContext, &audioPacket);
                if (ret != 0) {
                    char buf[100] = {0};
                    av_strerror(ret, buf, sizeof(buf));
                    LOGD("audio encode failed:%s", buf);
                    break;
                };
#if 0
                struct  timeval    endval;
                gettimeofday(&endval,NULL);
                long diff = 1000000 * (endval.tv_sec-tv.tv_sec)+ endval.tv_usec-tv.tv_usec;
                LOG_E("diff:%ld",diff);
                if(diff/1000000>=3){
                    long i = diff / 1000/1000;
                    LOG_E("interval time:%ld  audio framecount:%d",i,frameCount/i);
                    frameCount=0;
                    gettimeofday(&tv,NULL);
                    gettimeofday(&endval,NULL);
                }

                frameCount++;
                if(pFile){
                  fwrite(avPacket->data, 1, avPacket->size, pFile);
                fflush(pFile);
                }
#endif
                AVData avData;
                AVPacket *avPacket = av_packet_alloc();
                av_packet_move_ref(avPacket, &audioPacket);//此处data指针指向重新分配的内存，并复制其他属性
                LOGD("audio encode sucess  pts:%lld", pData->pts);


                avData.pts = pData->pts;
                avData.avPacket = avPacket;
                avData.isAudio = true;
                avData.duration = pData->duration;
                this->notifyObserver(avData);
            }
        }
    }
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
    audioCodecContext->sample_rate = avCodecParameters->sample_rate;
    audioCodecContext->thread_count = 4;
    audioCodecContext->bit_rate = avCodecParameters->bit_rate;
    audioCodecContext->channels = avCodecParameters->channels;
    audioCodecContext->frame_size = 1024;
    audioCodecContext->time_base = (AVRational) {1,
                                                 avCodecParameters->sample_rate};//AUDIO VIDEO 两边时间基数要相同
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

AVCodecContext *AudioEncoder::getAudioCodecContext() {
    return audioCodecContext;
}


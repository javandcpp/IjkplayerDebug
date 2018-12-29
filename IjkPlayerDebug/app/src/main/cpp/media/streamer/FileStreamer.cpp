//
// Created by developer on 2018/12/22.
//

#include "FileStreamer.h"
#include "../encode/AudioEncoder.h"
#include "../encode/VideoEncoder.h"


void FileStreamer::update(AVData avData) {
    std::lock_guard<std::mutex> lk(mtx);
    while (!isExit) {
        if (avData.isAudio && mAudioframeQueue.Size() < 50) {
            mAudioframeQueue.push(avData);
            LOGD("push audio to streamer");
            break;
        } else if (!avData.isAudio && mVideoframeQueue.Size() < 50) {
            mVideoframeQueue.push(avData);
            LOGD("push video to streamer");
            break;
        }
        xsleep(1);//减少CPU时间片
    }
}

FileStreamer::FileStreamer() {

}

FileStreamer::~FileStreamer() {
    if (NULL != audioStream) {
        av_free(audioStream);
        audioStream = NULL;
    }
    if (NULL != videoStream) {
        av_free(videoStream);
        videoStream = NULL;
    }
    if (NULL != mAudioCodecContext) {
        avcodec_free_context(&mAudioCodecContext);
        mAudioCodecContext = NULL;
    }
    if (NULL != mVideoCodecContext) {
        avcodec_free_context(&mVideoCodecContext);
        mVideoCodecContext = NULL;
    }
    if (NULL != iAvFormatContext) {
        avformat_free_context(iAvFormatContext);
        iAvFormatContext = NULL;
    }
}

FileStreamer *FileStreamer::Get() {
    static FileStreamer fileStreamer;
    return &fileStreamer;
}

int FileStreamer::InitStreamer(const char *url) {
    std::lock_guard<std::mutex> lk(mtx);
    this->outputUrl = url;
    int ret = 0;
    ret = avformat_alloc_output_context2(&iAvFormatContext, NULL, "mp4", outputUrl);
    if (ret < 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf));
        LOGD("avformat alloc output context2 failed: %s", buf);

        return -1;
    }
    LOGD("InitStreamer Success!");
    return 0;
}

//int FileStreamer::setAudioCodecContext(AVCodecParameters *avCodecParameters) {
//    avcodec_parameters_to_context(mVideoCodecContext,avCodecParameters);
//}
//
//int FileStreamer::setVideoCodecContext(AVCodecParameters *avCodecParameters) {
//    avcodec_alloc_context3(avCodecParameters->codec)
//    avcodec_parameters_to_context(mVideoCodecContext,avCodecParameters);
//}

int FileStreamer::AddStream(AVCodecContext *avCodecContext) {
    std::lock_guard<std::mutex> lk(mtx);
    AVStream *pStream = avformat_new_stream(iAvFormatContext, avCodecContext->codec);
    if (!pStream) {
        LOGD("avformat_new_stream failed!");
        return -1;
    }
    LOGD("avformat new stream success!");
    int ret = 0;
    ret = avcodec_parameters_from_context(pStream->codecpar, avCodecContext);
    if (ret < 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf));
        LOGD("avcodec_parameters_from_context failed :%s", buf);
        return -1;
    }
    LOGD("avcodec_parameters_from_context success!");
    pStream->codecpar->codec_tag = 0;
    if (avCodecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
        LOGD("Add video stream success!");
        videoStream = pStream;
        mVideoCodecContext = avCodecContext;
    } else if (avCodecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
        LOGD("Add audio stream success!");
        audioStream = pStream;
        mAudioCodecContext = avCodecContext;
    }
    return pStream->index;
}


//int FileStreamer::SendAudioFrame(AVData *originData, int streamIndex) {
//    std::lock_guard<std::mutex> lk(mut1);
//    AVRational stime;
//    AVRational dtime;
//
//    AVPacket *packet = originData->avPacket;
//    packet->stream_index = streamIndex;
//#ifdef SHOW_DEBUG_INFO
//    LOGD("packet index:%d    index:%d", packet->stream_index, streamIndex);
//#endif
//    stime = audioCodecContext->time_base;
//    dtime = audioStream->time_base;
//    //push
//    packet->pts = av_rescale_q(packet->pts, stime, dtime);
//    packet->dts = av_rescale_q(packet->dts, stime, dtime);
//    packet->duration = av_rescale_q(packet->duration, stime, dtime);
//#ifdef SHOW_DEBUG_INFO
//    LOGD("writer frame stream Index:%d   size:%d",
//          packet->stream_index,
//          packet->size);
//#endif
//    int ret = av_interleaved_write_frame(iAvFormatContext, packet);
//    if (ret == 0) {
//        LOGD("write ++++++++++++++audio frame sucess!");
//    } else {
//        char buf[1024] = {0};
//        av_strerror(ret, buf, sizeof(buf));
//        LOGD("writer******************* audio frame failed! :%s", buf);
//    }
//    delete originData;
//    return 0;
//
//}

//int FileStreamer::SendVideoFrame(AVData *originData, int streamIndex) {
//    std::lock_guard<std::mutex> lk(mut1);
//    AVRational stime;
//    AVRational dtime;
//
//    AVPacket *packet = originData->avPacket;
//    packet->stream_index = streamIndex;
//#ifdef SHOW_DEBUG_INFO
//    LOGD("video packet index:%d    index:%d", packet->stream_index, streamIndex);
//#endif
//    stime = videoCodecContext->time_base;
//    dtime = videoStream->time_base;
//    packet->pts = originData->pts;
//    packet->dts = packet->pts;
//    packet->pts = av_rescale_q_rnd(packet->pts, stime, dtime,
//                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
//    packet->dts = av_rescale_q_rnd(packet->dts, stime, dtime,
//                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
//    packet->duration = av_rescale_q(packet->duration, stime, dtime);
//#ifdef SHOW_DEBUG_INFO
//    LOGD("writer frame stream Index:%d   size:%d",
//          packet->stream_index,
//          packet->size);
//
//#endif
//    int ret = av_interleaved_write_frame(iAvFormatContext, packet);
//    if (ret == 0) {
//        LOGD("write ------------------video frame success!");
//    } else {
//        char buf[1024] = {0};
//        av_strerror(ret, buf, sizeof(buf));
//        LOGD("writer*******************video frame failed! :%s", buf);
//    }
//    delete originData;
//    return 0;
//
//}


/**
 * 音频推流任务
 */
void *FileStreamer::PushAudioStreamTask(void *pObj) {
    FileStreamer *fileStreamer = (FileStreamer *) pObj;
//    FileStreamer->isPushStream = true;

//    if (NULL == fileStreamer->audioEncoder) {
//        return 0;
//    }
//    int64_t beginTime = av_gettime();
    while (!fileStreamer->isExit) {

        if (fileStreamer->mAudioframeQueue.empty()) {
            continue;
        }

        const shared_ptr<AVData> &ptr = fileStreamer->mAudioframeQueue.wait_and_pop();
        AVData *pData = ptr.get();
        if (pData != NULL && ((AVPacket *) pData->data)->size > 0) {
            fileStreamer->SendFrame(pData, fileStreamer->audioStreamIndex);
        } else {
//            av_usleep(1000);
        }
    }

    return 0;
}

/**
* 音频推流任务
*/
void *FileStreamer::PushVideoStreamTask(void *pObj) {
    FileStreamer *fileStreamer = (FileStreamer *) pObj;
    fileStreamer->isPushStream = true;

//    if (NULL == fileStreamer->videoEncoder) {
//        return 0;
//    }
//    int64_t beginTime = av_gettime();
    while (!fileStreamer->isExit) {

        if (fileStreamer->mVideoframeQueue.empty()) {
            continue;
        }
        const shared_ptr<AVData> &ptr = fileStreamer->mVideoframeQueue.wait_and_pop();
        AVData *pData = ptr.get();


        if (pData != NULL) {
            AVPacket *packet = (AVPacket *) pData->data;
            LOGD("pData Size:%d", packet->size);
            if (packet->size > 0) {
                fileStreamer->SendFrame(pData, fileStreamer->videoStreamIndex);
            }
        } else {
//            av_usleep(1000);
        }
    }
    return 0;
}



/**
* 音视频同时推流任务
*/
//void *FileStreamer::PushStreamTask(void *pObj) {
//    FileStreamer *FileStreamer = (FileStreamer *) pObj;
//    FileStreamer->isPushStream = true;
//
//    if (NULL == FileStreamer->videoEncoder || NULL == FileStreamer->audioEncoder) {
//        return 0;
//    }
//    VideoCapture *pVideoCapture = FileStreamer->videoEncoder->GetVideoCapture();
//    AudioCapture *pAudioCapture = FileStreamer->audioEncoder->GetAudioCapture();
//
//    if (NULL == pVideoCapture || NULL == pAudioCapture) {
//        return 0;
//    }
//    int64_t beginTime = av_gettime();
//    if (NULL != pVideoCapture) {
//        pVideoCapture->videoCaputureframeQueue.clear();
//    }
//    if (NULL != pAudioCapture) {
//        pAudioCapture->audioCaputureframeQueue.clear();
//    }
//    int64_t lastAudioPts = 0;
//    while (true) {
//
//        if (!FileStreamer->isPushStream ||
//            pVideoCapture->GetCaptureState() ||
//            pAudioCapture->GetCaptureState()) {
//            break;
//        }
//        OriginData *pVideoData = pVideoCapture->GetVideoData();
//        OriginData *pAudioData = pAudioCapture->GetAudioData();
//
//        if (pAudioData != NULL && pAudioData->data) {
//            pAudioData->pts = pAudioData->pts - beginTime;
////            if (pAudioData->pts == lastAudioPts) {
////                pAudioData->pts += 1300;
////            }
//            lastAudioPts = pAudioData->pts;
//            LOGD("before audio encode pts:%lld", pAudioData->pts);
//            FileStreamer->audioEncoder->EncodeAAC(&pAudioData);
//            LOGD("after audio encode pts:%lld", pAudioData->avPacket->pts);
//        }
//
//
//        if (pAudioData != NULL && pAudioData->avPacket->size > 0) {
//            FileStreamer->SendFrame(pAudioData, FileStreamer->audioStreamIndex);
//        }
//
//        //h264 encode
//        if (pVideoData != NULL && pVideoData->data) {
//            pVideoData->pts = pVideoData->pts - beginTime;
//            LOGD("before video encode pts:%lld", pVideoData->pts);
//            FileStreamer->videoEncoder->EncodeH264(&pVideoData);
//            LOGD("after video encode pts:%lld", pVideoData->avPacket->pts);
//        }
//
//        if (pVideoData != NULL && pVideoData->avPacket->size > 0) {
//            FileStreamer->SendFrame(pVideoData, FileStreamer->videoStreamIndex);
//        }
//    }
//    return 0;
//}

int FileStreamer::StartPushStream() {
    if (videoEncoder) {
        videoStreamIndex = AddStream(videoEncoder->getVideoCodecContext());
    }
    if (audioEncoder) {
        audioStreamIndex = AddStream(audioEncoder->getAudioCodecContext());
    }
    pthread_create(&t3, NULL, FileStreamer::WriteHead, this);
    pthread_join(t3, NULL);

//    VideoCapture *pVideoCapture = videoEncoder->GetVideoCapture();
//    AudioCapture *pAudioCapture = audioEncoder->GetAudioCapture();
//    pVideoCapture->videoCaputureframeQueue.clear();
//    pAudioCapture->audioCaputureframeQueue.clear();

    if (writeHeadFinish) {
        if (audioEncoder) {
            pthread_create(&t1, NULL, FileStreamer::PushAudioStreamTask, this);
        }
        if (videoEncoder) {
            pthread_create(&t2, NULL, FileStreamer::PushVideoStreamTask, this);
        }
    } else {
        return -1;
    }
//    pthread_create(&t2, NULL, FileStreamer::PushStreamTask, this);
//    pthread_create(&t2, NULL, FileStreamer::PushStreamTask, this);

    return 0;
}

int FileStreamer::ClosePushStream() {
    if (isPushStream) {
        isPushStream = false;
        pthread_join(t1, NULL);
        pthread_join(t2, NULL);
        if (NULL != iAvFormatContext) {
            av_write_trailer(iAvFormatContext);
            avio_close(iAvFormatContext->pb);
        }
    }
    writeHeadFinish = false;
    return 0;
}

/**
 * notice:AVStream创建完成开始写头信息
 */
void *FileStreamer::WriteHead(void *pObj) {
    FileStreamer *fileStreamer = (FileStreamer *) pObj;
    int ret = 0;
    ret = avio_open(&fileStreamer->iAvFormatContext->pb, fileStreamer->outputUrl,
                    AVIO_FLAG_WRITE);
    if (ret < 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf));
        LOGD("avio open failed: %s", buf);
        return 0;
    }
    LOGD("avio open success!");
    ret = avformat_write_header(fileStreamer->iAvFormatContext, NULL);
    if (ret != 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf));
        LOGD("avformat write header failed!: %s", buf);
        return 0;
    }
    fileStreamer->writeHeadFinish = true;
    return 0;
}

int FileStreamer::SendFrame(AVData *pData, int streamIndex) {
    std::lock_guard<std::mutex> lk(mtx);
    AVRational stime;
    AVRational dtime;
    AVPacket *packet = (AVPacket *) pData->data;
    packet->stream_index = streamIndex;
    LOGD("write packet index:%d    index:%d   pts:%lud", packet->stream_index, streamIndex,
         packet->pts);
    //判断是音频还是视频
    if (packet->stream_index == videoStreamIndex) {
        stime = mVideoCodecContext->time_base;
        dtime = videoStream->time_base;
    } else if (packet->stream_index == audioStreamIndex) {
        stime = mAudioCodecContext->time_base;
        dtime = audioStream->time_base;
    } else {
        LOGE("unknow stream index");
        return -1;
    }
//    packet->pts = av_rescale_q(packet->pts, stime, dtime);
//    packet->dts = av_rescale_q(packet->pts, stime, dtime);
//    packet->duration = av_rescale_q(0, stime, dtime);
    int ret = av_write_frame(iAvFormatContext, packet);

    if (ret == 0) {
        if (streamIndex == audioStreamIndex) {
            LOGE("---------->write @@@@@@@@@ frame success------->!");
        } else if (streamIndex == videoStreamIndex) {
            LOGE("---------->write ######### frame success------->!");
        }
    } else {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf));
        LOGE("stream index %d writer frame failed! :%s", streamIndex, buf);
    }
    return 0;
}

void FileStreamer::startThread() {

}

void FileStreamer::stopThread() {




}

void FileStreamer::setVideoEncoder(VideoEncoder *pEncoder) {
    this->videoEncoder = pEncoder;
}

void FileStreamer::setAudioEncoder(AudioEncoder *pEncoder) {
    this->audioEncoder = pEncoder;
}


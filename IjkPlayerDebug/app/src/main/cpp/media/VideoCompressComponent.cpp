//
// Created by developer on 2018/12/9.
//
#include "VideoCompressComponent.h"
#include "demux/FFmpegDemux.h"
#include "../event/EventBus.cpp"


VideoCompressComponent::VideoCompressComponent() {
    pRegistration = EventBus::AddHandler<MediaEvent>(*this);
}

VideoCompressComponent::~VideoCompressComponent() {
    LOG_E("videoCompressComponent release");

    if (destPath) {
        free((void *) destPath);
        destPath=NULL;
    }
    if(pRegistration){
        pRegistration->removeHandler();
        delete pRegistration;
        pRegistration=NULL;
    }
}


FFmpegDemux *VideoCompressComponent::getDemux() {
    return mDemux;
}

FFmpegDecode *VideoCompressComponent::getVideoDecode() {
    return mVideoFfmpegDecode;
}

FFmpegDecode *VideoCompressComponent::getAudioDecode() {
    return mAudioFfmpegDecode;
}

AudioEncoder *VideoCompressComponent::getAudioEncode() {
    return audioEncoder;
}

VideoEncoder *VideoCompressComponent::getVideoEncode() {
    return videoEncoder;
}

void closeStream(){
//    do {
////                LOG_D("writeaudiopts %lld", writeAudioPts);
////                LOG_D("writevideopts %lld", writeVideoPts);
////                LOG_D("readAudioPts %lld", readAudioPts);
////                LOG_D("readVideoPtS %lld", readVideoPts);
//        xsleep(1);
//        if (writeVideoPts == readVideoPts && readAudioPts == writeAudioPts) {
//            xsleep(2000);
//            ((RtmpStreamer *) streamer)->ClosePushStream();
//            isExit = true;
//            break;
//        }
//    } while (!isExit);
}

void VideoCompressComponent::onEvent(MediaEvent &e) {
    LOG_E("onEvent: writevideo:%lld,writeaudio:%lld,readvideo:%lld,readaudio:%lld",
          e.getWriteVideoPts(), e.getWriteAudioPts(), e.getReadVideoPts(), e.getReadAudioPts());
    if (e.getWriteVideoPts() ==e.getReadVideoPts() && e.getWriteAudioPts() == e.getReadAudioPts()) {
        xsleep(2000);
        rtmpStreamer->ClosePushStream();

    }
}

bool VideoCompressComponent::initialize() {

    isRunning = true;
    if (!mDemux) {
        mDemux = new FFmpegDemux();
    }
    if (!mVideoFfmpegDecode) {
        mVideoFfmpegDecode = new FFmpegDecode();
        mVideoFfmpegDecode->isAudio = false;
    }
    if (!mAudioFfmpegDecode) {
        mAudioFfmpegDecode = new FFmpegDecode();
        mAudioFfmpegDecode->isAudio = true;
    }
    if (!audioEncoder) {
        audioEncoder = new AudioEncoder();
    }
    if (!videoEncoder) {
        videoEncoder = new VideoEncoder();
    }
    if (mVideoFfmpegDecode) {
        mDemux->addVideoDecode(mVideoFfmpegDecode);
        mVideoFfmpegDecode->addObserver(videoEncoder);
    }

    if (mAudioFfmpegDecode) {
        mDemux->addAudioDecode(mAudioFfmpegDecode);//添加音频解码
        mAudioFfmpegDecode->addObserver(audioEncoder);// 添加音频编码
    }


    return true;
}

void VideoCompressComponent::stop() {
    release();
    if (mpF1) {
        mpF1(0);
    }
}

void VideoCompressComponent::release() {
    this->isRunning = false;
    getDemux()->isExit = true;
    getVideoDecode()->isExit = true;
    getAudioDecode()->isExit = true;
    getAudioEncode()->isExit = true;
    getVideoEncode()->isExit = true;
    getRtmpStreamer()->isExit = true;
    sleep(1);

    if (avFormatContext) {
        avformat_free_context(avFormatContext);
        avFormatContext = NULL;
    }
    if (mDemux) {
        delete mDemux;
        mDemux = NULL;
    }
    if (mVideoFfmpegDecode) {
        delete mVideoFfmpegDecode;
        mVideoFfmpegDecode = NULL;
    }
    if (mAudioFfmpegDecode) {
        delete mAudioFfmpegDecode;
        mAudioFfmpegDecode = NULL;
    }
    if (audioEncoder) {
        delete audioEncoder;
        audioEncoder = NULL;
    }
    if (videoEncoder) {
        delete videoEncoder;
        videoEncoder = NULL;
    }
}

void closeStreamCallBack(void *p) {
    LOG_D("this is callback");
    VideoCompressComponent *videoCompressComponent = (VideoCompressComponent *) p;
    videoCompressComponent->release();
    if (videoCompressComponent->mPf) {
        videoCompressComponent->mPf(0);
    }

}

bool VideoCompressComponent::open(const char *url) {
    int ret = 0;
    int rotate = 0;
    double d = 0.0;

    AVDictionaryEntry *m = NULL;


    avFormatContext = avformat_alloc_context();
    if (!avFormatContext) {
        LOGE("Could not allocate context.\n");
    }
    avFormatContext->interrupt_callback.opaque = this;
    if ((ret = avformat_open_input(&avFormatContext, url, 0, 0)) < 0) {
        LOGE("avformat open input failed  :%s!", av_err2str(ret));
        goto fail;
    }
    LOGD("avformat open input successful!");
    if ((ret = avformat_find_stream_info(avFormatContext, 0)) < 0) {
        LOGE("avformat find stream info failed:  %s", av_err2str(ret));
        goto fail;
    }

    getDemux()->setAvFormatContext(avFormatContext);

    getDemux()->setVideoStreamIndex(
            av_find_best_stream(avFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, 0, 0));
    getDemux()->setAudioStreamIndex(
            av_find_best_stream(avFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, 0, 0));


    getDemux()->getRotate();
    getDemux()->getDuration();


    getDemux()->audioStream = avFormatContext->streams[getDemux()->audioStreamIndex];
    getDemux()->audioPtsRatio = getDemux()->audioStream->nb_frames /
                                     getDemux()->audioStream->codecpar->channels;;

    getDemux()->videoStream = avFormatContext->streams[getDemux()->videoStreamIndex];
    getDemux()->video_src_width = getDemux()->videoStream->codecpar->width;
    getDemux()->video_src_height = getDemux()->videoStream->codecpar->height;

    d = getDemux()->get_rotation(getDemux()->videoStream);
    LOG_E("d %lf", d);
    getDemux()->videoPtsRatio = getDemux()->videoStream->time_base.den /
                                     getDemux()->videoStream->r_frame_rate.num;

    LOGD("find stream index  videoStream:%d   audioStream:%d",
         getDemux()->getVideoStreamIndex(),
         getDemux()->getAudioStreamIndex());


    return true;

    fail:
    return false;
}

bool VideoCompressComponent::openSource(const char *url) {
    FFmpegDemux *pDemux = getDemux();
    if (!pDemux) {
        return false;
    }

    open(url);
    int rotate = 0;
    const MetaData metaData = pDemux->getMetaData();
//    if (metaData.video_rotate) {
//        rotate = atoi(metaData.video_rotate);
//    }

//    if (rotate == 90 || rotate == 270) {
    getVideoDecode()->setVideoScaleHeight(getMScaleHeight());
    getVideoDecode()->setVideoScaleWidth(getMScaleWidth());
    getVideoEncode()->setVideoEncodeWidth(getMScaleWidth());
    getVideoEncode()->setVideoEncodeHeight(getMScaleHeight());
//    } else {
//        if (metaData.video_width < metaData.video_height) {
//            getVideoDecode()->setVideoScaleWidth(getMScaleHeight());
//            getVideoDecode()->setVideoScaleHeight(getMScaleWidth());
//            getVideoEncode()->setVideoEncodeWidth(getMScaleHeight());
//            getVideoEncode()->setVideoEncodeHeight(getMScaleWidth());
//        } else {
//            getVideoDecode()->setVideoScaleHeight(getMScaleHeight());
//            getVideoDecode()->setVideoScaleWidth(getMScaleWidth());
//            getVideoEncode()->setVideoEncodeWidth(getMScaleWidth());
//            getVideoEncode()->setVideoEncodeHeight(getMScaleHeight());
//        }
//    }



    //打开音视频解码器
    if (!getAudioDecode()->openCodec(*(pDemux->getAudioParameters()))) {
        return false;
    }
    if (!getVideoDecode()->openCodec(*(pDemux->getVideoParamters()))) {
        return false;
    }


//        音频编码
    getAudioEncode()->InitEncode(
            pDemux->getAudioParameters()->codecParameters);
    getVideoEncode()->InitEncode(
            pDemux->getVideoParamters()->codecParameters);

    //IO
    rtmpStreamer = RtmpStreamer::Get();
    rtmpStreamer->setProgressCallBack(functionP1);
    pDemux->setStreamer(rtmpStreamer);
    getRtmpStreamer()->setCloseCallBack(closeStreamCallBack, this);

    getRtmpStreamer()->inAudioTimeBase = pDemux->getAudioStream()->time_base;
    getRtmpStreamer()->inVideoTimeBase = pDemux->getVideoStream()->time_base;

    getVideoEncode()->addObserver(getRtmpStreamer());
    getAudioEncode()->addObserver(getRtmpStreamer());


    getRtmpStreamer()->setVideoEncoder(getVideoEncode());
    getRtmpStreamer()->setAudioEncoder(getAudioEncode());

    getRtmpStreamer()->InitStreamer(destPath);
    getRtmpStreamer()->setMetaData(pDemux->getMetaData());
    getRtmpStreamer()->WriteHead(getRtmpStreamer());


    //视频编码
    getAudioEncode()->startThread();
    getVideoEncode()->startThread();
//    //开启解码
    getAudioDecode()->startThread();
    getVideoDecode()->startThread();
    //开始解复用
    pDemux->startThread();
    //文件写入
    getRtmpStreamer()->startThread();


    return true;
}

long VideoCompressComponent::getMScaleWidth() {
    return mScaleWidth;
}

void VideoCompressComponent::setMScaleWidth(long mScaleWidth) {
    this->mScaleWidth = mScaleWidth;
}

long VideoCompressComponent::getMScaleHeight() {
    return mScaleHeight;
}

void VideoCompressComponent::setMScaleHeight(long mScaleHeight) {
    this->mScaleHeight = mScaleHeight;
}

RtmpStreamer *VideoCompressComponent::getRtmpStreamer() const {
    return rtmpStreamer;
}

void VideoCompressComponent::setCallback(void(*pF)(void *)) {
    mPf = pF;
}

void VideoCompressComponent::setDestPath(const char *string) {
    destPath = string;
}

void VideoCompressComponent::setStopCallBack(void(*pF)(void *)) {
    mpF1 = pF;
}

void VideoCompressComponent::setProgressCallBack(void (*fun)(long, long)) {
    functionP1 = fun;
}

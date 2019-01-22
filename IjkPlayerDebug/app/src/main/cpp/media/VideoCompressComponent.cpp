//
// Created by developer on 2018/12/9.
//
#include "VideoCompressComponent.h"
#include "demux/FFmpegDemux.h"

VideoCompressComponent::VideoCompressComponent() {

}

VideoCompressComponent::~VideoCompressComponent() {
    LOG_E("videoCompressComponent release");
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
    if (mDemux) {
        if (mVideoFfmpegDecode) {
            mDemux->addVideoDecode(mVideoFfmpegDecode);
            mVideoFfmpegDecode->addObserver(videoEncoder);
        }

        if (mAudioFfmpegDecode) {
            mDemux->addAudioDecode(mAudioFfmpegDecode);//添加音频解码
            mAudioFfmpegDecode->addObserver(audioEncoder);// 添加音频编码
        }
    } else {
        return false;
    }
    return true;
}

void closeStreamCallBack(void *p) {
    LOG_D("this is callback");
    VideoCompressComponent *videoCompressComponent = (VideoCompressComponent *) p;
    videoCompressComponent->getDemux()->isExit = true;
    videoCompressComponent->getVideoDecode()->isExit = true;
    videoCompressComponent->getAudioDecode()->isExit = true;
    videoCompressComponent->getAudioEncode()->isExit = true;
    videoCompressComponent->getVideoEncode()->isExit = true;
    videoCompressComponent->getFileStreamer()->isExit = true;

    delete videoCompressComponent->getDemux();
    delete videoCompressComponent->getVideoDecode();
    delete videoCompressComponent->getAudioDecode();
    delete videoCompressComponent->getAudioEncode();
    delete videoCompressComponent->getVideoEncode();
    videoCompressComponent->isRunning=false;
    if (videoCompressComponent->mPf) {
        videoCompressComponent->mPf(0);
    }

}

bool VideoCompressComponent::openSource(const char *url) {
    FFmpegDemux *pDemux = getDemux();
    if (!pDemux) {
        return false;
    }

    pDemux->open(url);
    int rotate = 0;
    const MetaData metaData = pDemux->getMetaData();
    if (metaData.video_rotate) {
        rotate = atoi(metaData.video_rotate);
    }

    if (rotate == 0 || rotate == 180) {
        getVideoDecode()->setVideoScaleWidth(getMScaleHeight());
        getVideoDecode()->setVideoScaleHeight(getMScaleWidth());
        getVideoEncode()->setVideoEncodeWidth(getMScaleHeight());
        getVideoEncode()->setVideoEncodeHeight(getMScaleWidth());
    } else {
        getVideoDecode()->setVideoScaleHeight(getMScaleHeight());
        getVideoDecode()->setVideoScaleWidth(getMScaleWidth());
        getVideoEncode()->setVideoEncodeWidth(getMScaleWidth());
        getVideoEncode()->setVideoEncodeHeight(getMScaleHeight());
    }
    //打开音视频解码器
    if (!getAudioDecode()->openCodec(*(pDemux->getAudioParameters()))) {
        return false;
    }
    if (!getVideoDecode()->openCodec(*(pDemux->getVideoParamters()))) {
        return false;
    }

//        音频编码
    getAudioEncode()->InitEncode(
            getDemux()->getAudioParameters()->codecParameters);
    getVideoEncode()->InitEncode(
            getDemux()->getVideoParamters()->codecParameters);

    //IO
    fileStreamer = FileStreamer::Get();
    pDemux->setStreamer(fileStreamer);
    getFileStreamer()->setCloseCallBack(closeStreamCallBack, this);

    getFileStreamer()->inAudioTimeBase = pDemux->getAudioStream()->time_base;
    getFileStreamer()->inVideoTimeBase = pDemux->getVideoStream()->time_base;

    getVideoEncode()->addObserver(getFileStreamer());
    getAudioEncode()->addObserver(getFileStreamer());


    getFileStreamer()->setVideoEncoder(getVideoEncode());
    getFileStreamer()->setAudioEncoder(getAudioEncode());

    getFileStreamer()->InitStreamer(destPath);
    getFileStreamer()->setMetaData(pDemux->getMetaData());

    //文件写入
    getFileStreamer()->startThread();
    //视频编码
    getAudioEncode()->startThread();
    getVideoEncode()->startThread();
    //开启解码
    getAudioDecode()->startThread();
    getVideoDecode()->startThread();
    //开始解复用
    getDemux()->startThread();


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

FileStreamer *VideoCompressComponent::getFileStreamer() const {
    return fileStreamer;
}

void VideoCompressComponent::setCallback(void(*pF)(void *)) {
    mPf = pF;
}

void VideoCompressComponent::setDestPath(const char *string) {
    destPath=string;
}

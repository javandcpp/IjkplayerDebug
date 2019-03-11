//
// Created by developer on 2018/12/9.
//
#include "VideoCompressComponent.h"
#include "demux/FFmpegDemux.h"

VideoCompressComponent::VideoCompressComponent() {

}

VideoCompressComponent::~VideoCompressComponent() {
    LOG_E("videoCompressComponent release");
    if (destPath) {
        free((void *) destPath);
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
    getFileStreamer()->isExit = true;
    sleep(1);
    delete getDemux();
    mDemux = NULL;
    delete getVideoDecode();
    mVideoFfmpegDecode = NULL;
    delete getAudioDecode();
    mAudioFfmpegDecode = NULL;
    delete getAudioEncode();
    audioEncoder = NULL;
    delete getVideoEncode();
    videoEncoder = NULL;
}

void closeStreamCallBack(void *p) {
    LOG_D("this is callback");
    VideoCompressComponent *videoCompressComponent = (VideoCompressComponent *) p;
    videoCompressComponent->release();
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

    if (rotate == 90 || rotate == 270) {
        getVideoDecode()->setVideoScaleHeight(getMScaleHeight());
        getVideoDecode()->setVideoScaleWidth(getMScaleWidth());
        getVideoEncode()->setVideoEncodeWidth(getMScaleWidth());
        getVideoEncode()->setVideoEncodeHeight(getMScaleHeight());
    } else {
        if (metaData.video_width < metaData.video_height) {
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
    fileStreamer->setProgressCallBack(functionP1);
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
    getFileStreamer()->WriteHead(getFileStreamer());


    //视频编码
    getAudioEncode()->startThread();
    getVideoEncode()->startThread();
    //开启解码
    getAudioDecode()->startThread();
    getVideoDecode()->startThread();
    //开始解复用
    getDemux()->startThread();
    //文件写入
    getFileStreamer()->startThread();


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
    destPath = string;
}

void VideoCompressComponent::setStopCallBack(void(*pF)(void *)) {
    mpF1 = pF;
}

void VideoCompressComponent::setProgressCallBack(void (*fun)(long, long)) {
    functionP1 = fun;
}

//
// Created by developer on 2018/12/9.
//
#include "VideoCompressComponent.h"
#include "demux/FFmpegDemux.h"

VideoCompressComponent::VideoCompressComponent() {

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

VideoEncoder* VideoCompressComponent::getVideoEncode() {
    return videoEncoder;
}

bool VideoCompressComponent::initialize() {
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
            mDemux->addObserver(mVideoFfmpegDecode);
            mVideoFfmpegDecode->addObserver(videoEncoder);
        }

        if (mAudioFfmpegDecode) {
            mDemux->addObserver(mAudioFfmpegDecode);//添加音频解码
            mAudioFfmpegDecode->addObserver(audioEncoder);// 添加音频编码
        }
    }
    return true;
}

bool VideoCompressComponent::openSource(const char *url) {
    std::lock_guard<std::mutex> lk(mut);
    FFmpegDemux *demux = getDemux();
    if (demux) {
        return demux->open(url);
    }

    return false;
}

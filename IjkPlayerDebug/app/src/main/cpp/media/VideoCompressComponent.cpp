//
// Created by developer on 2018/12/9.
//
#include "VideoCompressComponent.h"
#include "demux/FFmpegDemux.h"

VideoCompressComponent::VideoCompressComponent() {

}

FFmpegDemux* VideoCompressComponent::getDemux() {
    return mDemux;
}
FFmpegDecode* VideoCompressComponent::getDecode() {
    return mFfmpegDecode;
}

bool VideoCompressComponent::initialize() {
    if (!mDemux) {
        mDemux = new FFmpegDemux();
    }
    if (!mFfmpegDecode) {
        mFfmpegDecode = new FFmpegDecode();
    }
    return true;
}
bool VideoCompressComponent::openSource(const char *url) {
	mtx.lock();
	FFmpegDemux* demux=getDemux();
	if(demux){
		demux->open(url);
	}	
	mtx.unlock();
    return true;
}
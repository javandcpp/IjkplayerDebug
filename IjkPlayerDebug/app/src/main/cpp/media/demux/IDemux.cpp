//
// Created by developer on 2018/8/27.
//

#include "IDemux.h"
#include "../global_header.h"

void IDemux::main() {
    while (!isExit) {
        AVData data = readMediaData();
        if(data.isAudio){
            LOGD("audio stream   pts:%ld",data.pts);
            NativeThread::readAudioPts=data.pts;
            mAudioDecode->update(data);
            LOG_D("readaudio pts %lld",readAudioPts);
        }else{
            LOGD("video stream   pts:%ld",data.pts);
            NativeThread::readVideoPts=data.pts;
            mVideoDecode->update(data);
            LOG_D("readview pts %lld",readVideoPts);
        }
        xsleep(10);
//        if (data.size > 0) {
//            notifyObserver(data);
//        }
    }
}
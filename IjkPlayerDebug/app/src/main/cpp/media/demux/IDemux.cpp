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
        }else{
            LOGD("video stream   pts:%ld",data.pts);
        }
        xsleep(5);
        if (data.size > 0) {
            notifyObserver(data);
        }
    }
}
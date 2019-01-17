//
// Created by developer on 2018/8/27.
//

#ifndef XMEDIAPLAYER_IDEMUX_H
#define XMEDIAPLAYER_IDEMUX_H


#include "../IObserver.h"
#include "../AVParameters.h"
#include "../MetaData.h"
#include "../decode/FFmpegDecode.h"

class IDemux : public IObserver {

public:
    virtual bool open(const char *url)=0;

    virtual AVParameters * getVideoParamters()=0;

    virtual AVParameters * getAudioParameters()=0;

    virtual AVData readMediaData()=0;

    virtual MetaData getMetaData()=0;





protected:
    virtual void main();


    FFmpegDecode *mVideoDecode= nullptr;
    FFmpegDecode *mAudioDecode= nullptr;

};


#endif //XMEDIAPLAYER_IDEMUX_H

//
// Created by developer on 2018/8/27.
//

#ifndef XMEDIAPLAYER_AVPARAMETERS_H
#define XMEDIAPLAYER_AVPARAMETERS_H



#ifdef __cplusplus
extern "C" {
#endif

#include "libavcodec/avcodec.h"

class AVParameters {
public:
    AVCodecParameters *codecParameters=NULL;
    int channels = 2;
    int sample_rate = 44100;
};


#ifdef __cplusplus
}
#endif

#endif //XMEDIAPLAYER_AVPARAMETERS_H



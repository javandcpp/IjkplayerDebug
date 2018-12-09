//
// Created by developer on 2018/9/16.
//

#ifndef XMEDIAPLAYER_FFMPEGAUDIORESAMPLE_H
#define XMEDIAPLAYER_FFMPEGAUDIORESAMPLE_H


#ifdef __cplusplus
extern "C" {
#include "libswresample/swresample.h"
};
#endif

#include "../AVParameters.h"
#include "../AVData.h"
#include "IAudioResample.h"

class FFmpegAudioResample: public IAudioResample {
public:
    bool Init(AVParameters in, AVParameters out = AVParameters());
    AVData Resample(AVData indata);

protected:
    SwrContext *actx = 0;

};


#endif //XMEDIAPLAYER_FFMPEGAUDIORESAMPLE_H

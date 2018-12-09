//
// Created by developer on 2018/9/16.
//

#ifndef XMEDIAPLAYER_IAUDIORESAMPLE_H
#define XMEDIAPLAYER_IAUDIORESAMPLE_H


#include "../AVData.h"
#include "../AVParameters.h"
#include "../IObserver.h"

class IAudioResample : public IObserver {
public:
    virtual bool Init(AVParameters in, AVParameters out = AVParameters()) = 0;
    virtual AVData Resample(AVData indata) = 0;
    virtual void update(AVData data);

    int outChannels = 2;
    int outFormat = 1;
};


#endif //XMEDIAPLAYER_IAUDIORESAMPLE_H

//
// Created by developer on 2018/9/16.
//

#ifndef XMEDIAPLAYER_IAUDIOPLAY_H
#define XMEDIAPLAYER_IAUDIOPLAY_H


#include <mutex>
#include <list>
#include "../AVData.h"
#include "../AVParameters.h"
#include "../IObserver.h"
#include "../threadsafe_queue.cpp"

class IAudioPlay : public IObserver {
public:
    virtual void update(AVData data);

    virtual AVData GetData();

    virtual bool StartPlay(AVParameters out) = 0;

    //最大缓冲
    int maxFrame = 100;
    int pts = 0;
protected:
    threadsafe_queue<AVData> threadsafeQueue;
};


#endif //XMEDIAPLAYER_IAUDIOPLAY_H

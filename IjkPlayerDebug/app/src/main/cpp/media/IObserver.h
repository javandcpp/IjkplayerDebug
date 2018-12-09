//
// Created by developer on 2018/8/27.
//

#ifndef XMEDIAPLAYER_IOBSERVER_H
#define XMEDIAPLAYER_IOBSERVER_H


#include <vector>
#include "NativeThread.h"
#include "AVData.h"
#include <mutex>

class IObserver: public NativeThread {

public:
    virtual void update(AVData avData){};
    void addObserver(IObserver *observers);
    void notifyObserver(AVData data);

protected:
    std::vector<IObserver*> observerlist;
    std::mutex mtx;

};


#endif //XMEDIAPLAYER_IOBSERVER_H

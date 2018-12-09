//
// Created by developer on 2018/8/27.
//

#include "IObserver.h"
using namespace std;

void IObserver::addObserver(IObserver *observers) {
    if(!observers)return;
    mtx.lock();
    observerlist.push_back(observers);
    mtx.unlock();
}

void IObserver::notifyObserver(AVData data) {

    mtx.lock();
    for (int i = 0; i < observerlist.size(); ++i) {
        observerlist[i]->update(data);
    }
    mtx.unlock();
}

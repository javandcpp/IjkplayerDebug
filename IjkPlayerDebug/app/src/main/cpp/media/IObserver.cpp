//
// Created by developer on 2018/8/27.
//

#include "IObserver.h"
using namespace std;

void IObserver::addObserver(IObserver *observers) {
    std::lock_guard<std::mutex> lk(mtx);
    if(!observers)return;

    observerlist.push_back(observers);
}

void IObserver::notifyObserver(AVData data) {
    std::lock_guard<std::mutex> lk(mtx);
    for (int i = 0; i < observerlist.size(); ++i) {
        observerlist[i]->update(data);
    }
}

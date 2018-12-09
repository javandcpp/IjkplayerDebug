//
// Created by developer on 2018/9/16.
//

#include "IAudioResample.h"

/**
 * 重采样
 * @param data
 */
void IAudioResample::update(AVData data) {
    AVData avData = this->Resample(data);
    if (avData.size > 0) {
        this->notifyObserver(avData);
    }
}
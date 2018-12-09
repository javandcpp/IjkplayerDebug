//
// Created by developer on 2018/9/16.
//

#include "IAudioPlay.h"

AVData IAudioPlay::GetData() {
    AVData audioData;
    while (!isExit) {
        if (!threadsafeQueue.empty()) {
            const shared_ptr<AVData> &ptr = threadsafeQueue.wait_and_pop();
            if (ptr) {
                audioData = *ptr.get();
                pts = audioData.pts;
                return audioData;
            } else {
                continue;
            }
        }
        xsleep(1);
    }
    return audioData;
}

void IAudioPlay::update(AVData data) {
    if (data.size <= 0 || !data.data) return;
    while (!isExit) {
        if (threadsafeQueue.Size() < maxFrame) {
            threadsafeQueue.push(data);
            break;
        }
        xsleep(1);
    }
}
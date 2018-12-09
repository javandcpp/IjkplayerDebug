//
// Created by developer on 2018/8/27.
//

#include "IFFmpegDecode.h"
#include "../global_header.h"

void IFFmpegDecode::update(AVData pkt) {
    if (pkt.isAudio != isAudio) {
        return;
    }
    while (!isExit) {
        if (threadsafeQueue.Size() < maxList) {
            threadsafeQueue.push(pkt);
            break;
        }
        xsleep(1);
    }
}

void IFFmpegDecode::main() {
    while (!isExit) {
        //判断音视频同步
        if (!isAudio && syncAudioPts > 0) {
            //如果音频PTS小于视频，停止视频编码
            if (syncAudioPts < pts) {
                continue;
            }
        }

        if (threadsafeQueue.empty()) {
            xsleep(1);
            continue;
        }

        const shared_ptr<AVData> &ptr = threadsafeQueue.wait_and_pop();
        AVData *pData = ptr.get();
        //发送数据到解码线程，一个数据包，可能解码多个结果
        if (this->sendPacket(*pData)) {
            while (!isExit) {
                //获取解码数据
                AVData frame = this->receiveFrame();
                if (!frame.data) break;
                //XLOGE("RecvFrame %d",frame.size);
                pts = frame.pts;

                //发送数据给观察者
                this->notifyObserver(frame);

            }

        }
        pData->Drop();
    }
}

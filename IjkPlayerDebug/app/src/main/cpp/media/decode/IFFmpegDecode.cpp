//
// Created by developer on 2018/8/27.
//

#include "IFFmpegDecode.h"
#include "../global_header.h"

void IFFmpegDecode::update(AVData pkt) {
    mut.lock();
    if (pkt.isAudio != isAudio) {
        mut.unlock();
        return;
    }
    while (!isExit) {
//        if (threadsafeQueue.Size()) {
            threadsafeQueue.push(pkt);
            break;
        }
//        xsleep(1);
//    }
    mut.unlock();
}

void IFFmpegDecode::main() {
    while (!isExit) {
        if (threadsafeQueue.empty()) {
            xsleep(1);
            continue;
        }

        LOG_E("decode queue size:%d",threadsafeQueue.Size());
        const shared_ptr<AVData> &ptr = threadsafeQueue.wait_and_pop();
        AVData *pData = ptr.get();
        xsleep(1);
        //发送数据到解码线程，一个数据包，可能解码多个结果
        if (this->sendPacket(*pData)) {
            while (!isExit) {
                //获取解码数据
                AVData frame = this->receiveFrame();
                LOGD("receive frame size:%d",frame.size);
                if (frame.size<=0) {
                    break;
                }

                pts = frame.pts;
                frame.duration=pData->duration;
                frame.pts=pData->pts;
                //发送数据给观察者
                if(!frame.isAudio){
                    LOG_D("video");
                }else{
                    LOG_D("audio");
                }
                this->notifyObserver(frame);

            }

        }
//        pData->Drop();
    }
}

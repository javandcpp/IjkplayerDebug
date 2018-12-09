//
// Created by developer on 2018/8/27.
//

#ifndef XMEDIAPLAYER_IFFMPEGDECODE_H
#define XMEDIAPLAYER_IFFMPEGDECODE_H


#include "../IObserver.h"
#include "../AVParameters.h"
#include "../threadsafe_queue.cpp"
#include <mutex>
#include <list>


class IFFmpegDecode : public IObserver {

public:
    //打开解码器
    virtual bool openCodec(AVParameters parameters) = 0;

    //future模型 发送数据到线程解码
    virtual bool sendPacket(AVData pkt) = 0;

    //从线程中获取解码结果  再次调用会复用上次空间，线程不安全
    virtual AVData receiveFrame() = 0;

    virtual AVData receiveCacheFrame()=0;

    virtual void update(AVData pkt);

    bool isAudio = false;

    //最大的队列缓冲
    int maxList = 100;

    //同步时间，再次打开文件要清理
    int syncAudioPts = 0;
    int pts = 0;

protected:
    virtual void main();

    threadsafe_queue<AVData> threadsafeQueue;
    mutable std::mutex mut;
    std::list<AVData> packs;
    std::mutex packsMutex;
};


#endif //XMEDIAPLAYER_IFFMPEGDECODE_H

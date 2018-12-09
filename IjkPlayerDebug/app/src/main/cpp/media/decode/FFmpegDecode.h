//
// Created by developer on 2018/9/15.
//

#ifndef XMEDIAPLAYER_FFMPEGDECODE_H
#define XMEDIAPLAYER_FFMPEGDECODE_H


#include "IFFmpegDecode.h"



class FFmpegDecode : public IFFmpegDecode {

public:
    FFmpegDecode();

    virtual ~FFmpegDecode();

    bool openCodec(AVParameters parameters);

    //发送数据到线程解码
    bool sendPacket(AVData pkt);

    //从线程中获取解码结果  再次调用会复用上次空间，线程不安全
    AVData receiveFrame();

    AVData receiveCacheFrame();

protected:
    AVCodecContext *codecContext=NULL;
    AVFrame *avFrame = NULL;
};



#endif //XMEDIAPLAYER_FFMPEGDECODE_H

//
// Created by developer on 11/13/17.
//

#ifndef NATIVEAPP_VIDEOENCODER_H
#define NATIVEAPP_VIDEOENCODER_H

#include "MediaEncoder.h"
#include "../global_header.h"
#include "../AVData.h"
#include "../threadsafe_queue.cpp"

#include <string>
#include <iostream>

#include <list>

#define AV_PKT_FLAG_UNCODED_FRAME 0x2000
#define UNCODED_FRAME_PACKET_SIZE (INT_MIN / 3 * 2 + (int)sizeof(AVFrame))
#define CHUNK_START 0x1000

using namespace std;

class VideoEncoder : public MediaEncoder {

private:
//    VideoCapture *videoCapture = NULL;

    /**
  *
  */
    int InitFilter();

public:
    FILE *pFILE=NULL;

    bool isEncoding = false;

    static VideoEncoder *Get();

    static void *EncodeTask(void *obj);

    /**
     * 已编码数据队列
     */
    threadsafe_queue<AVData> aVideoframeQueue;
//    list<OriginData *> VideoDatalist;


    VideoEncoder();

    int maxList=500;

    ~VideoEncoder();

    /**
     * 开启编码
     */
    int StartEncode();

    /**
     * 初始化H264视频编码器
     */
    int InitEncode(AVCodecParameters* avCodecParameters);

    /**
     * 关闭编码器
     */
    int CloseEncode();


    int EncodeH264(AVData **originData);

    /**
     * 资源回收
     */
    int Release();

    /**
     * 设置数据采集
     */
//    void SetVideoCapture(VideoCapture *videoCapture);


//    VideoCapture *GetVideoCapture();


    /**
     * 获取编码器状态
     */
    bool GetEncodeState();


    /**
     * 前置镜像
     */
    void YUVProcessMirror();

    /**
     * 初始化过滤器
     */
//    int SetFilter(DrawTextFilter *drawTextFilter);

    AVCodec *avCodec = NULL;

    AVStream *outStream = NULL;

    AVFrame *vOutFrame = NULL;

    AVCodecContext *videoCodecContext = NULL;

    AVFrame *outputYUVFrame = NULL;

    AVFrame *inputYUVFrame = NULL;

    AVPacket videoPacket = {0};


    virtual void update(AVData avData);

    virtual void main();

    AVCodecContext *getVideoCodecContext();

};


#endif //NATIVEAPP_VIDEOENCODER_H

//
// Created by developer on 11/13/17.
//

#ifndef NATIVEAPP_AUDIOENCODER_H
#define NATIVEAPP_AUDIOENCODER_H

#include "MediaEncoder.h"
//#include "AudioCapture.h"
#include "../global_header.h"
#include "../AVData.h"
#include "../threadsafe_queue.cpp"
#include <list>

using namespace std;

class AudioEncoder : public MediaEncoder {

private:
//    AudioCapture *audioCapture = NULL;

public:
    AVCodec *avCodec = NULL;
    AVStream *outStream = NULL;
    AVFrame *outputFrame = NULL;
    AVCodecContext *audioCodecContext = NULL;
    AVPacket audioPacket = {0};

    //最大的队列缓冲
    int maxList = 500;


    static AudioEncoder *Get();


    bool isEncoding = false;

    static void *EncodeTask(void *p);

    /**
      * 音频已编码数据队列
      */
    threadsafe_queue<AVData> aAudioframeQueue;


//    list<OriginData *> AudioDatalist;

    AudioEncoder();

    ~AudioEncoder();

    /**
     * 开启编码
     */
    virtual int StartEncode();

    /**
     * 初始化视频编码器
     */
    int InitEncode(AVCodecParameters* avCodecParameters);

    /**
    * 关闭编码器
    */
    int CloseEncode();


     int EncodeAAC(AVData **originData);

    /**
     * 资源回收
     */
    int Release();

    /**
     * 设置数据采集
     */
//    void SetAudioCapture(AudioCapture *audioCapture);

//    AudioCapture *GetAudioCapture();

    /**
    * 获取编码器状态
    */
    bool GetEncodeState();

    virtual void update(AVData avData);

    virtual void main();

    AVCodecContext *getAudioCodecContext();

    FILE *pFile= nullptr;


};


#endif //NATIVEAPP_AUDIOENCODER_H

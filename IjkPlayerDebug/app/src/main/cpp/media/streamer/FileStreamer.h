//
// Created by developer on 2018/12/22.
//

#ifndef IJKPLAYERDEBUG_FILESTREAMER_H
#define IJKPLAYERDEBUG_FILESTREAMER_H


#include "../NativeThread.h"
#include "../IObserver.h"
#include "../global_header.h"
#include "../encode/AudioEncoder.h"
#include "../encode/VideoEncoder.h"
#include <mutex>

class FileStreamer : public IObserver {

protected:
    FileStreamer();

public:


    virtual ~FileStreamer();

    AVStream *videoStream = NULL;

    AVStream *audioStream = NULL;

    AudioEncoder *audioEncoder = nullptr;

    VideoEncoder *videoEncoder = nullptr;

    AVFormatContext *iAvFormatContext = NULL;

    AVCodecContext *mAudioCodecContext = nullptr;

    AVCodecContext *mVideoCodecContext = nullptr;

    virtual void startThread();

    virtual void stopThread();

    virtual void main() {}

    virtual void update(AVData avData);

    std::mutex mtx;

    static FileStreamer *Get();

    int InitStreamer(const char *url);

    const char *outputUrl;


    int AddStream(AVCodecContext *avCodecContext);


    int SendFrame(AVData *pData, int streamIndex);

    static void *WriteHead(void *pObj);

    bool writeHeadFinish;


    static void *PushAudioStreamTask(void *pObj);

    static void *PushVideoStreamTask(void *pObj);

    int StartPushStream();

    int ClosePushStream();

    pthread_t t2;

    pthread_t t3;

    pthread_t t1;


    bool isPushStream;
    int audioStreamIndex = -1;
    int videoStreamIndex = -1;

    threadsafe_queue<AVData> mAudioframeQueue;
    threadsafe_queue<AVData> mVideoframeQueue;

    void setVideoEncoder(VideoEncoder *pEncoder);

    void setAudioEncoder(AudioEncoder *pEncoder);

    int videoFrameCount = 0;
    int audioFrameCount = 0;
};


#endif //IJKPLAYERDEBUG_FILESTREAMER_H

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
#include "../MetaData.h"
#include <mutex>


typedef void(*pFunc)(void *);


class FileStreamer : public IObserver {

protected:
    FileStreamer();

public:


    virtual ~FileStreamer();

    AVStream *videoStream = NULL;

    AVStream *audioStream = NULL;

    AudioEncoder *audioEncoder = nullptr;

    VideoEncoder *videoEncoder = nullptr;

    AVFormatContext *iAvFormatContext = nullptr;

    AVCodecContext *mAudioCodecContext = nullptr;

    AVCodecContext *mVideoCodecContext = nullptr;

    MetaData metaData;

    void main();

    virtual void update(AVData avData);

    std::mutex mtx;

    static FileStreamer *Get();

    int InitStreamer(const char *url);

    const char *outputUrl;


    int AddStream(AVCodecContext *avCodecContext);


    int SendFrame(AVPacket *avPacket, int streamIndex);

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

    pthread_cond_t cond;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//    pthread_mutex_t mutex_audio;

    FILE *pFile = NULL;

    int64_t audioPts = 0;
    int64_t videoPts = 0;

    void setMetaData(MetaData data);

    AVRational inAudioTimeBase = {0, 0};
    AVRational inVideoTimeBase = {0, 0};

    int recordKeyValue = 1;

    pFunc mFunctionPoniter;

    void *p= nullptr;
    void setCloseCallBack(void (*fun)(void *), void *p);
};


#endif //IJKPLAYERDEBUG_FILESTREAMER_H

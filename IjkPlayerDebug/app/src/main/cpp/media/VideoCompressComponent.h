//
// Created by developer on 2018/12/9.
//

#ifndef IJKPLAYERDEBUG_VIDEOCOMPRESSCOMPONENT_H
#define IJKPLAYERDEBUG_VIDEOCOMPRESSCOMPONENT_H


#include "decode/FFmpegDecode.h"
#include "demux/FFmpegDemux.h"
#include "encode/AudioEncoder.h"
#include "encode/VideoEncoder.h"
#include <mutex>

using namespace std;

typedef void(*pF)(void *);

class VideoCompressComponent {
private:
    FFmpegDemux *mDemux = nullptr;
    FFmpegDecode *mVideoFfmpegDecode = nullptr;
    FFmpegDecode *mAudioFfmpegDecode = nullptr;
    AudioEncoder *audioEncoder = nullptr;
    VideoEncoder *videoEncoder = nullptr;
    FileStreamer *fileStreamer = nullptr;

    mutable mutex mut;

public:


    const char *destPath = nullptr;

    bool isRunning = false;

    VideoCompressComponent();

    virtual ~VideoCompressComponent();

    FileStreamer *getFileStreamer() const;

    bool initialize();

    FFmpegDemux *getDemux();

    FFmpegDecode *getVideoDecode();

    FFmpegDecode *getAudioDecode();

    AudioEncoder *getAudioEncode();

    VideoEncoder *getVideoEncode();

    bool openSource(const char *url);

    long mScaleWidth;

    long mScaleHeight;


    void setMScaleWidth(long mScaleWidth);


    void setMScaleHeight(long mScaleHeight);

    long getMScaleWidth();

    long getMScaleHeight();

    pF mPf = nullptr;

    void setCallback(void(*pF)(void *));

    void setDestPath(const char *string);
};


#endif //IJKPLAYERDEBUG_VIDEOCOMPRESSCOMPONENT_H

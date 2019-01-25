//
// Created by developer on 2018/8/27.
//

#ifndef XMEDIAPLAYER_FFMPEGDEMUX_H
#define XMEDIAPLAYER_FFMPEGDEMUX_H

#include "IDemux.h"
#include "../global_header.h"
#include "../streamer/FileStreamer.h"
#include "../decode/FFmpegDecode.h"


class FFmpegDemux : public IDemux {

public:
    static bool isFirst;

    long long audioPts = 0;

    long long videoPts = 0;

    void *streamer = nullptr;

    virtual bool open(const char *url);

    virtual AVParameters *getVideoParamters();

    virtual AVParameters *getAudioParameters();

    virtual AVData readMediaData();

    FFmpegDemux();

    virtual ~FFmpegDemux();

    AVFormatContext *avFormatContext = NULL;

    int getVideoStreamIndex() const;

    void setVideoStreamIndex(int videoStreamIndex);

    int getAudioStreamIndex() const;

    void setAudioStreamIndex(int audioStreamIndex);

    void getRotate();

    void getDuration();

    virtual MetaData getMetaData();


    AVParameters *videoAvParameters = NULL;
    AVParameters *audioAvParameters = NULL;


    void setStreamer(FileStreamer *pStreamer);

    int64_t frameCount = 0;

    AVDictionaryEntry avDictionaryEntry;

    bool hasRotateValue = false;

    char *mVideoRotate = nullptr;

    int videoRotate=0;

    int64_t mVideoDuration = 0;

    int video_src_width=0;

    int video_src_height=0;

    int audioPtsRatio = 0;

    int videoPtsRatio = 0;

    AVStream *audioStream = NULL;
    AVStream *videoStream = NULL;

    AVStream *getAudioStream() const;

    AVStream *getVideoStream() const;

    void addVideoDecode(FFmpegDecode *pDecode);

    void addAudioDecode(FFmpegDecode *pDecode);



protected:

    int videoStreamIndex = -1;
    int audioStreamIndex = -1;

    void initAVCodec();


};


#endif //XMEDIAPLAYER_FFMPEGDEMUX_H

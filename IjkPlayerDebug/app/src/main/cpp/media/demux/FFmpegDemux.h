//
// Created by developer on 2018/8/27.
//

#ifndef XMEDIAPLAYER_FFMPEGDEMUX_H
#define XMEDIAPLAYER_FFMPEGDEMUX_H

#include "IDemux.h"
#include "../global_header.h"



class FFmpegDemux : public IDemux {

public:
    static bool isFirst;

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

    AVParameters *videoAvParameters = NULL;
    AVParameters *audioAvParameters = NULL;


protected:

    int videoStreamIndex = -1;
    int audioStreamIndex = -1;

    void initAVCodec();

};


#endif //XMEDIAPLAYER_FFMPEGDEMUX_H

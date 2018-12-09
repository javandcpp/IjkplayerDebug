//
// Created by developer on 2018/12/9.
//

#ifndef IJKPLAYERDEBUG_VIDEOCOMPRESSCOMPONENT_H
#define IJKPLAYERDEBUG_VIDEOCOMPRESSCOMPONENT_H


#include "decode/FFmpegDecode.h"
#include "demux/FFmpegDemux.h"
#include "encode/AudioEncoder.h"
#include <mutex>
using namespace std;
class VideoCompressComponent{
private:
    FFmpegDemux *mDemux= nullptr;
    FFmpegDecode *mVideoFfmpegDecode= nullptr;
    FFmpegDecode *mAudioFfmpegDecode= nullptr;
    AudioEncoder *audioEncoder= nullptr;

    mutable mutex mut;
    
public:


    VideoCompressComponent();

    bool initialize();

    FFmpegDemux* getDemux();
    FFmpegDecode* getVideoDecode();
    FFmpegDecode* getAudioDecode();
    AudioEncoder* getAudioEncode();

    bool openSource(const char *url);


};


#endif //IJKPLAYERDEBUG_VIDEOCOMPRESSCOMPONENT_H

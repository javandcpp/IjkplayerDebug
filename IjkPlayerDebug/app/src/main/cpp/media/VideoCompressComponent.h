//
// Created by developer on 2018/12/9.
//

#ifndef IJKPLAYERDEBUG_VIDEOCOMPRESSCOMPONENT_H
#define IJKPLAYERDEBUG_VIDEOCOMPRESSCOMPONENT_H


#include "decode/FFmpegDecode.h"
#include "demux/FFmpegDemux.h"
#include <mutex>
class VideoCompressComponent{
private:
    FFmpegDemux *mDemux= nullptr;
    FFmpegDecode *mFfmpegDecode= nullptr;
    mutex mtx;
    
public:


    VideoCompressComponent();

    bool initialize();

    FFmpegDemux* getDemux();
    FFmpegDecode* getDecode();

    bool openSource(const char *url);

};


#endif //IJKPLAYERDEBUG_VIDEOCOMPRESSCOMPONENT_H

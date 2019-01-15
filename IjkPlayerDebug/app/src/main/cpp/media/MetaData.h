//
// Created by developer on 2019/1/8.
//

#ifndef IJKPLAYERDEBUG_METADATA_H
#define IJKPLAYERDEBUG_METADATA_H


#include <cstdint>

class MetaData {

public:
    int64_t duration;

    char* video_rotate= nullptr;

    int64_t getDuration() const;

    void setDuration(int64_t duration);

    char* getVideo_rotate() const;

    void setVideo_rotate(char* video_rotate);
};


#endif //IJKPLAYERDEBUG_METADATA_H

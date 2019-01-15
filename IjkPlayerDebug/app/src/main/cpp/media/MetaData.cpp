//
// Created by developer on 2019/1/8.
//

#include "MetaData.h"

int64_t MetaData::getDuration() const {
    return duration;
}

void MetaData::setDuration(int64_t duration) {
    MetaData::duration = duration;
}

char* MetaData::getVideo_rotate() const {
    return video_rotate;
}

void MetaData::setVideo_rotate(char* video_rotate) {
    MetaData::video_rotate = video_rotate;
}

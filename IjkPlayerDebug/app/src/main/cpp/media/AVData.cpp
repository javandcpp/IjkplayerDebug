//
// Created by developer on 2018/8/27.
//

#include <memory>

#ifdef __cplusplus
extern "C" {
#include "libavcodec/avcodec.h"
}
#endif

#include "AVData.h"

bool AVData::Alloc(int size, const char *d) {
    Drop();
    type = UCHAR_TYPE;
    if (size <= 0)return false;
    this->data = new unsigned char[size];
    if (!this->data) return false;
    if (d) {
        memcpy(this->data, d, size);
    }
    this->size = size;
    return true;
}

void AVData::Drop() {
    if (!data) return;
    if (type == AVPACKET_TYPE) {
        av_packet_free((AVPacket **)&data);
    }else {
        delete data;
    }
    data = 0;
    size = 0;
}
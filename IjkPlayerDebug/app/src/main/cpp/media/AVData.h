//
// Created by developer on 2018/8/27.
//

#ifndef XMEDIAPLAYER_AVDATA_H
#define XMEDIAPLAYER_AVDATA_H


enum AVDataType {
    AVPACKET_TYPE = 0, UCHAR_TYPE = 1
};

class AVData {
public:
    int type = 0;
    unsigned char *data = 0;
    unsigned char *datas[8] = {0};
    bool isAudio = false;
    int width = 0;

    int format=0;
    int height = 0;

    bool Alloc(int size, const char *data = 0);

    void Drop();
    int pts=0;

    bool end;

    int size = 0;
};


#endif //XMEDIAPLAYER_AVDATA_H

//
// Created by developer on 11/13/17.
//

#ifndef NATIVEAPP_MEDIAENCODER_H
#define NATIVEAPP_MEDIAENCODER_H
#include <iostream>
#include <mutex>
#include "../IObserver.h"
#include "../global_header.h"

using namespace std;
class MediaEncoder:public IObserver {
protected:

    mutable mutex mtx;

    MediaEncoder();

    virtual ~MediaEncoder();

    void RegisterAVCodec();

    void RegisterAVNetwork();

    virtual int StartEncode() = 0;

    virtual int InitEncode(AVCodecParameters* avCodecParameters) = 0;

    virtual int CloseEncode() = 0;

    virtual int Release() = 0;

    static bool first;

    void update(AVData avData){};

    void main(){};


};


#endif //NATIVEAPP_MEDIAENCODER_H

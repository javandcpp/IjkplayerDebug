//
// Created by developer on 2018/8/27.
//

#include "NativeThread.h"
#include "global_header.h"
#include <thread>

using namespace std;



long long NativeThread::readAudioPts=0;
long long NativeThread::writeAudioPts=0;
long long NativeThread::readVideoPts=0;
long long NativeThread::writeVideoPts=0;

void xsleep(int mills) {
    chrono::milliseconds milliseconds(mills);
    this_thread::sleep_for(milliseconds);
}

void NativeThread::startThread() {

    isExit = false;
    std::thread t1(&NativeThread::ThreadMainTask, this);
    t1.detach();
}

void NativeThread::ThreadMainTask() {
    isRunning = true;
    main();
    isRunning = false;
}



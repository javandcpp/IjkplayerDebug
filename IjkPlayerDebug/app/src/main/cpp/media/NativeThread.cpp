//
// Created by developer on 2018/8/27.
//

#include "NativeThread.h"
#include "global_header.h"
#include <thread>

using namespace std;

void xsleep(int mills) {
    chrono::milliseconds milliseconds(mills);
    this_thread::sleep_for(milliseconds);
}

void NativeThread::startThread() {

    isExit = false;
    std::thread t1(&NativeThread::ThreadMainTask, this);
    t1.detach();
}

void NativeThread::stopThread() {
    isExit = true;
    for (int i = 0; i < 200; i++) {
        if (!isRunning) {
            LOGD("Stop 停止线程成功!");
            return;
        }
        xsleep(1);
    }
    LOGD("Stop 停止线程超时!");
}

void NativeThread::ThreadMainTask() {
    isRunning = true;
    main();
    isRunning = false;
}



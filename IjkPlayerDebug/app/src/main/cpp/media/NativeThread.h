//
// Created by developer on 2018/8/27.
//

#ifndef XMEDIAPLAYER_NATIVETHREAD_H
#define XMEDIAPLAYER_NATIVETHREAD_H


void xsleep(int millseconds);



class NativeThread {


public:
    virtual void startThread();

    virtual void stopThread();

    virtual void main(){}

protected:
    bool isExit=false;
    bool isRunning =false;
    bool isEnd=false;
private:
    void ThreadMainTask();
};


#endif //XMEDIAPLAYER_NATIVETHREAD_H

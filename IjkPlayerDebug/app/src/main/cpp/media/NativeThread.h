//
// Created by developer on 2018/8/27.
//

#ifndef XMEDIAPLAYER_NATIVETHREAD_H
#define XMEDIAPLAYER_NATIVETHREAD_H


void xsleep(int millseconds);



class NativeThread {


public:
    virtual void startThread();

    virtual void main(){}

    static long long readAudioPts;
    static long long writeAudioPts;
    static long long readVideoPts;
    static long long writeVideoPts;
    bool isExit=false;
    bool isRunning =false;
    bool isEnd=false;

protected:



private:
    void ThreadMainTask();
};


#endif //XMEDIAPLAYER_NATIVETHREAD_H

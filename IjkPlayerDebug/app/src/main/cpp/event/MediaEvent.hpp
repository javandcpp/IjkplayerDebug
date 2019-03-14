//
// Created by developer on 2019/3/13.
//

#ifndef IJKPLAYERDEBUG_MEDIAEVENT_H
#define IJKPLAYERDEBUG_MEDIAEVENT_H

#include "Event.hpp"

class MediaEvent: public Event {
public:
    MediaEvent(Object &sender,int64_t writevideopts,int64_t writeaudioPts,int64_t readVideoPts,int64_t readAudioPts):Event(sender){
        this->writeVideoPts=writevideopts;
        this->writeAudioPts=writeaudioPts;
        this->readVideoPts=readVideoPts;
        this->readAudioPts=readAudioPts;
    }

    virtual ~MediaEvent() { }


    int64_t getWriteVideoPts(){
        return writeVideoPts;
    }

    int64_t getWriteAudioPts(){
        return writeAudioPts;
    }

    int64_t getReadVideoPts(){
        return readVideoPts;
    }

    int64_t getReadAudioPts(){
        return readAudioPts;
    }



private:
    int64_t writeVideoPts;
    int64_t writeAudioPts;
    int64_t readVideoPts;
    int64_t readAudioPts;
};




#endif //IJKPLAYERDEBUG_MEDIAEVENT_H

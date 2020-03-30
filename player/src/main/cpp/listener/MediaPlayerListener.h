//
// Created by xingzhe on 2020/3/29.
//

#ifndef MEDIAPLAYER_MEDIAPLAYERLISTENER_H
#define MEDIAPLAYER_MEDIAPLAYERLISTENER_H

#include <jni.h>

class MediaPlayerListener {
public:
    virtual void notify(int what, int thread) = 0;

    virtual void updateTime(int duration, double progress, int thread) = 0;

    virtual void
    onYUVCallback(int width, int height, uint8_t *yBuffer, uint8_t *uBuffer, uint8_t *vBuffer) = 0;
};


#endif //MEDIAPLAYER_MEDIAPLAYERLISTENER_H

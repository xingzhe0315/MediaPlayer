//
// Created by xingzhe on 2020/3/22.
//

#ifndef MEDIAPLAYER_VIDEOPLAYER_H
#define MEDIAPLAYER_VIDEOPLAYER_H


#include "IPlayer.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>

};

class VideoPlayer : public IPlayer {
public:
    SwsContext *swsContext;
    AVFrame *yuvFrame;
    uint8_t *buffer;
    int bufferSize;
    int width;
    int height;
    jobject surface;

    VideoPlayer();

    ~VideoPlayer();

    void initExtraParams() override;

    void play() override;

    int resampleData() override;

    void release() override;

    virtual void pause() override;

    virtual void resume() override;

    virtual void stop() override;

    virtual void seekTo(int time) override;
};


#endif //MEDIAPLAYER_VIDEOPLAYER_H

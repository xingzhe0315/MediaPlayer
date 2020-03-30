//
// Created by xingzhe on 2020/3/22.
//

#ifndef MEDIAPLAYER_IPLAYER_H
#define MEDIAPLAYER_IPLAYER_H

#include <jni.h>
#include "queue/PacketQueue.h"
#include "log_define.h"
#include "listener/MediaPlayerListener.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

};

enum media_player_states {
    MEDIA_PLAYER_STATE_ERROR = 0,
    MEDIA_PLAYER_IDLE = 1 << 0,
    MEDIA_PLAYER_INITIALIZED = 1 << 1,
    MEDIA_PLAYER_PREPARING = 1 << 2,
    MEDIA_PLAYER_PREPARED = 1 << 3,
    MEDIA_PLAYER_STARTED = 1 << 4,
    MEDIA_PLAYER_PAUSED = 1 << 5,
    MEDIA_PLAYER_STOPPED = 1 << 6,
    MEDIA_PLAYER_PLAYBACK_COMPLETE = 1 << 7
};

enum media_event_type {
    MEDIA_NOP = 0, // interface test message
    MEDIA_PREPARED = 1,
    MEDIA_PLAYBACK_COMPLETE = 2,
    MEDIA_BUFFERING_UPDATE = 3,
    MEDIA_SEEK_COMPLETE = 4,
    MEDIA_SET_VIDEO_SIZE = 5,
    MEDIA_STARTED = 6,
    MEDIA_PAUSED = 7,
    MEDIA_STOPPED = 8,
    MEDIA_SKIPPED = 9,
    MEDIA_NOTIFY_TIME = 98,
    MEDIA_TIMED_TEXT = 99,
    MEDIA_ERROR = 100,
    MEDIA_INFO = 200,
    MEDIA_SUBTITLE_DATA = 201,
    MEDIA_META_DATA = 202,
    MEDIA_DRM_INFO = 210,
    MEDIA_TIME_DISCONTINUITY = 211,
    MEDIA_AUDIO_ROUTING_CHANGED = 10000,
};

class IPlayer {
public:
    int status;
    int index;
    int sampleRate;
    double clock;
    double lastClock;
    AVRational timeBase;
    int64_t duration;
    AVCodecParameters *avCodecParameters;
    AVCodecContext *avCodecContext;
    PacketQueue *packetQueue;

    MediaPlayerListener *mListener;

    pthread_t mPlayThread;

    AVPacket *mAvPacket;

    IPlayer();

    ~IPlayer();


    void initParamsFromStream(AVStream *avStream);

    void openStream();

    void pushPacket(AVPacket *packet);

    void setListener(MediaPlayerListener *listener);

    virtual void play() = 0;

    virtual void initExtraParams() = 0;

    virtual int resampleData() = 0;

    virtual void release() = 0;

    virtual void pause() = 0;

    virtual void resume() = 0;

    virtual void stop() = 0;

    virtual void seekTo(int time) = 0;

};


#endif //MEDIAPLAYER_IPLAYER_H

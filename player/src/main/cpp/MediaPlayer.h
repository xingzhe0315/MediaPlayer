//
// Created by xingzhe on 2020/3/22.
//

#ifndef MEDIAPLAYER_MEDIAPLAYER_H
#define MEDIAPLAYER_MEDIAPLAYER_H

#include <jni.h>
#include <sys/types.h>
#include <pthread.h>
#include "AudioPlayer.h"
#include "VideoPlayer.h"
#include "listener/MediaPlayerListener.h"

#define TAG "MediaPlayer"


#define THREAD_MAIN 1 // 主线程
#define THREAD_WORK 2 // 子线程

class MediaPlayer {
public:
    const char *mDataSource;
    MediaPlayerListener *mListener;
    AVFormatContext *avFormatContext;
    int status;
    int duration;
    AVRational timeBase;
    int mSeekTime;
    volatile bool mDecodeFinished;

    // thread
    pthread_t mPreparedThread;
    pthread_t mDecodeThread;
    pthread_mutex_t mDecodeMutex;
    pthread_t mSeekThread;

    pthread_mutex_t mSeekMutex;

    AudioPlayer *audioPlayer;
    VideoPlayer *videoPlayer;

    MediaPlayer();

    ~MediaPlayer();

    void setListener(MediaPlayerListener *listener);

    void setDataSource(const char *dataSource);

    void prepare();

    void prepareAsync();

    void start();

    void stop();

    void release();

    void pause();

    void resume();

    void seekTo(jint time);

    void setVolume(jint volume);

    void setSoundChannel(jint channel);

    void setPitch(float pitch);

    void setSpeed(float speed);
};


#endif //MEDIAPLAYER_MEDIAPLAYER_H

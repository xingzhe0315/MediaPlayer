//
// Created by xingzhe on 2020/3/22.
//

#ifndef MEDIAPLAYER_AUDIOPLAYER_H
#define MEDIAPLAYER_AUDIOPLAYER_H

#include "IPlayer.h"

extern "C" {
#include <libswresample/swresample.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <libavutil/time.h>
};

#define TAG "AudioPlayer"

class AudioPlayer : public IPlayer {
public:
    SwrContext *swrContext;
    uint8_t *buffer;
    bool mPacketReadFinished = true;
    int mVolume = 100;
    int mChannel = 3;
    bool mPlayerInitialized = false;

    /****************** start OpenSL ES *****************/
    SLObjectItf engineObject = nullptr;
    SLEngineItf engineEngine;

    SLObjectItf outputMixObject;
    SLEnvironmentalReverbItf outputMixEnvironmentReverb;

    SLObjectItf audioPlayerObject;
    SLPlayItf audioPlayer;
    SLAndroidSimpleBufferQueueItf playerBufferQueue;

    SLEffectSendItf playerEffectSend;
    SLVolumeItf playerVolume;
    SLMuteSoloItf playerMuteSolo;

/****************** end OpenSL ES *****************/

    AudioPlayer();

    ~AudioPlayer();

    void createEngine();

    void createBufferQueuePlayer(int sampleRate);

    void setVolume(int volume);

    void setSoundChannel(int channel);

    void initExtraParams() override;

    void play() override;

    int resampleData() override;

    void release() override;

    void pause() override;

    void resume() override;

    void stop() override;

    void seekTo(int time) override;
};


#endif //MEDIAPLAYER_AUDIOPLAYER_H

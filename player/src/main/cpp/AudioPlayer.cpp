//
// Created by xingzhe on 2020/3/22.
//

#include "AudioPlayer.h"


AudioPlayer::AudioPlayer() {
    createEngine();
}

void AudioPlayer::initExtraParams() {
    int sampleRate = avCodecContext->sample_rate;
    AVSampleFormat inSampleFormat = avCodecContext->sample_fmt;
    uint64_t inChannelLayout = avCodecContext->channel_layout;
    swrContext = swr_alloc_set_opts(
            nullptr,
            AV_CH_LAYOUT_STEREO,
            AV_SAMPLE_FMT_S16,
            sampleRate,
            inChannelLayout,
            inSampleFormat,
            sampleRate,
            0, nullptr);
    swr_init(swrContext);
    createBufferQueuePlayer(avCodecContext->sample_rate);
    buffer = static_cast<uint8_t *>(av_malloc(
            static_cast<size_t>(sampleRate * 2 * 2)));
}

SLuint32 convertSampleRate(int sampleRate);

void playCallback(SLAndroidSimpleBufferQueueItf caller,
                  void *pContext) {
    auto *audioPlayer = static_cast<AudioPlayer *>(pContext);
    int dataSize = audioPlayer->resampleData();
    audioPlayer->clock += dataSize / ((double) (audioPlayer->avCodecContext->sample_rate * 2 * 2));
    if (dataSize > 0) {
        if (audioPlayer->clock - audioPlayer->lastClock >= 1) {
            audioPlayer->lastClock = audioPlayer->clock;
//            audioPlayer->mListener->updateTime(audioPlayer->duration, audioPlayer->clock,
//                                               THREAD_WORK);
        }
        (*audioPlayer->playerBufferQueue)->Enqueue(audioPlayer->playerBufferQueue,
                                                   audioPlayer->buffer,
                                                   static_cast<SLuint32>(dataSize));
    }
}

int AudioPlayer::resampleData() {
    AVFrame *avFrame = av_frame_alloc();
    int dataSize = 0;
    while (status & MEDIA_PLAYER_STARTED) {
        if (packetQueue->getQueueSize() <= 0) {
//            if (decodeFinished) {
//                status = PLAY_STATUS_FINISHED;
//                release();
//                mListener->notify(PLAY_STATUS_FINISHED, THREAD_WORK);
//                break;
//            }
//            if (status != PLAY_STATUS_LOADING) {
//                status = PLAY_STATUS_LOADING;
//                mListener->notify(PLAY_STATUS_LOADING, THREAD_WORK);
//            }
            av_usleep(1000 * 10);
            continue;
        } else {
//            if (status != PLAY_STATUS_PLAYING) {
//                status = PLAY_STATUS_PLAYING;
//                mListener->notify(PLAY_STATUS_PLAYING, THREAD_WORK);
//            }
        }
//        if (exit) {
//            break;
//        }

        if (mPacketReadFinished) {
            mAvPacket = av_packet_alloc();
            packetQueue->getPacket(mAvPacket);
            if (avcodec_send_packet(avCodecContext, mAvPacket) != 0) {
                mPacketReadFinished = true;
                av_packet_free(&mAvPacket);
                av_frame_free(&avFrame);
                continue;
            }
        }
        if (avcodec_receive_frame(avCodecContext, avFrame) == 0) {
            mPacketReadFinished = false;
            int nb = swr_convert(swrContext,
                                 &buffer,
                                 avFrame->nb_samples,
                                 reinterpret_cast<const uint8_t **>(&avFrame->data),
                                 avFrame->nb_samples);
            int channelNum = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            dataSize = av_samples_get_buffer_size(
                    nullptr,
                    channelNum,
                    nb,
                    AV_SAMPLE_FMT_S16,
                    1);
            double tempClock = avFrame->pts * av_q2d(timeBase);
            if (tempClock >= clock) {
                clock = tempClock;
            }
            av_packet_free(&mAvPacket);
            break;
        } else {
            mPacketReadFinished = true;
            av_packet_free(&mAvPacket);
            continue;
        }
    }
    av_frame_free(&avFrame);
    avFrame = nullptr;
    return dataSize;
}

void *_playAudio(void *data) {
    LOGE(TAG, "audio player _play");
    AudioPlayer *audioPlayer = static_cast<AudioPlayer *>(data);
    playCallback(audioPlayer->playerBufferQueue, audioPlayer);
    pthread_exit(&audioPlayer->mPlayThread);
}

void AudioPlayer::play() {
    if (status & MEDIA_PLAYER_STARTED) return;

    LOGE(TAG, "audio player play()");
    (*audioPlayer)->SetPlayState(audioPlayer, SL_PLAYSTATE_PLAYING);
    status = MEDIA_PLAYER_STARTED;
    pthread_create(&mPlayThread, nullptr, _playAudio, this);
}

AudioPlayer::~AudioPlayer() {
    if (buffer != nullptr) {
        free(buffer);
        buffer = nullptr;
    }
    if (outputMixObject != nullptr) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = nullptr;
        outputMixEnvironmentReverb = nullptr;
    }

    if (engineObject != nullptr) {
        (*engineObject)->Destroy(engineObject);
        engineObject = nullptr;
        engineEngine = nullptr;
    }
    if (packetQueue != nullptr) {
        packetQueue->clear();
        free(packetQueue);
        packetQueue = nullptr;
    }
}

void AudioPlayer::createBufferQueuePlayer(int sampleRate) {
    SLresult result;
    SLDataLocator_AndroidSimpleBufferQueue locatorBufferQueue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2
    };
    SLDataFormat_PCM slDataFormatPcm = {
            SL_DATAFORMAT_PCM,
            2,
            convertSampleRate(sampleRate),
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSource dataSource = {
            &locatorBufferQueue, &slDataFormatPcm
    };
    SLDataLocator_OutputMix locatorOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink dataSink = {&locatorOutputMix, nullptr};

    const SLInterfaceID ids[5] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_EFFECTSEND,
                                  SL_IID_PLAYBACKRATE,
                                  SL_IID_MUTESOLO};
    const SLboolean req[5] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
                              SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(
            engineEngine,
            &audioPlayerObject,
            &dataSource,
            &dataSink,
            5,
            ids,
            req
    );
    if (SL_RESULT_SUCCESS != result) {
        LOGE(TAG, "(*engineEngine)->CreateAudioPlayer error!!!");
    }
    assert(SL_RESULT_SUCCESS == result);
    result = (*audioPlayerObject)->Realize(audioPlayerObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(TAG, "(*audioPlayerObject)->Realize(audioPlayerObject, SL_BOOLEAN_FALSE); error!!!");
    }
    assert(SL_RESULT_SUCCESS == result);
    result = (*audioPlayerObject)->GetInterface(audioPlayerObject, SL_IID_PLAY, &audioPlayer);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(TAG,
             "(*audioPlayerObject)->GetInterface(audioPlayerObject, SL_IID_PLAY, &audioPlayer); error!!!");
    }
    assert(SL_RESULT_SUCCESS == result);
    result = (*audioPlayerObject)->GetInterface(audioPlayerObject, SL_IID_BUFFERQUEUE,
                                                &playerBufferQueue);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(TAG,
             "(*audioPlayerObject)->GetInterface(audioPlayerObject, SL_IID_BUFFERQUEUE, &playerBufferQueue); error!!!");
    }
    assert(SL_RESULT_SUCCESS == result);
    result = (*playerBufferQueue)->RegisterCallback(playerBufferQueue, playCallback, this);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(TAG,
             "(*playerBufferQueue)->RegisterCallback(playerBufferQueue, callback, nullptr); error!!!");
    }
    assert(SL_RESULT_SUCCESS == result);
    result = (*audioPlayerObject)->GetInterface(audioPlayerObject, SL_IID_EFFECTSEND,
                                                &playerEffectSend);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(TAG,
             "(*audioPlayerObject)->GetInterface(audioPlayerObject, SL_IID_EFFECTSEND, &playerEffectSend); error!!!");
    }
    assert(SL_RESULT_SUCCESS == result);
    result = (*audioPlayerObject)->GetInterface(audioPlayerObject, SL_IID_VOLUME, &playerVolume);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(TAG,
             "(*audioPlayerObject)->GetInterface(audioPlayerObject, SL_IID_VOLUME, &playerVolume); error!!!");
    }
    assert(SL_RESULT_SUCCESS == result);
    result = (*audioPlayerObject)->GetInterface(audioPlayerObject, SL_IID_MUTESOLO,
                                                &playerMuteSolo);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(TAG,
             "(*audioPlayerObject)->GetInterface(audioPlayerObject, SL_IID_MUTESOLO, &playerMuteSolo); error!!!");
    }
    assert(SL_RESULT_SUCCESS == result);
    mPlayerInitialized = true;
    setVolume(mVolume);
    setSoundChannel(mChannel);
}

void AudioPlayer::createEngine() {
    SLresult result;
    result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(TAG, "slCreateEngine error!!!");
    }
    assert(SL_RESULT_SUCCESS == result);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(TAG, "(*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE) error!!!");
    }
    assert(SL_RESULT_SUCCESS == result);
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(TAG,
             "(*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine); error!!!");
    }
    assert(SL_RESULT_SUCCESS == result);
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(TAG,
             " (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req); error!!!");
    }
    assert(SL_RESULT_SUCCESS == result);
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(TAG, " (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE); error!!!");
    }
    assert(SL_RESULT_SUCCESS == result);
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentReverb);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(TAG, "(*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,\n"
                  "                                     &outputMixEnvironmentReverb); error!!!");
    }
    assert(SL_RESULT_SUCCESS == result);
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
    result = (*outputMixEnvironmentReverb)->SetEnvironmentalReverbProperties(
            outputMixEnvironmentReverb,
            &reverbSettings);
    if (SL_RESULT_SUCCESS != result) {
        LOGE(TAG,
             "(*outputMixEnvironmentReverb)->SetEnvironmentalReverbProperties(outputMixEnvironmentReverb,\n"
             "                                                                    &reverbSettings); error!!!");
    }
}

void AudioPlayer::setVolume(int percent) {
    mVolume = percent;
    if (!mPlayerInitialized) return;
    if (percent > 30) {
        (*playerVolume)->SetVolumeLevel(playerVolume, (100 - percent) * -20);
    } else if (percent > 25) {
        (*playerVolume)->SetVolumeLevel(playerVolume, (100 - percent) * -22);
    } else if (percent > 20) {
        (*playerVolume)->SetVolumeLevel(playerVolume, (100 - percent) * -25);
    } else if (percent > 15) {
        (*playerVolume)->SetVolumeLevel(playerVolume, (100 - percent) * -28);
    } else if (percent > 10) {
        (*playerVolume)->SetVolumeLevel(playerVolume, (100 - percent) * -30);
    } else if (percent > 5) {
        (*playerVolume)->SetVolumeLevel(playerVolume, (100 - percent) * -34);
    } else if (percent > 3) {
        (*playerVolume)->SetVolumeLevel(playerVolume, (100 - percent) * -37);
    } else if (percent > 0) {
        (*playerVolume)->SetVolumeLevel(playerVolume, (100 - percent) * -40);
    } else {
        (*playerVolume)->SetVolumeLevel(playerVolume, (100 - percent) * -100);
    }
}

void AudioPlayer::setSoundChannel(int channel) {
    mChannel = channel;
    if (!mPlayerInitialized) return;

    if (channel == 0) {//center
        (*playerMuteSolo)->SetChannelMute(playerMuteSolo, 1, false);
        (*playerMuteSolo)->SetChannelMute(playerMuteSolo, 0, false);
    } else if (channel == 1) {//left
        (*playerMuteSolo)->SetChannelMute(playerMuteSolo, 1, true);
        (*playerMuteSolo)->SetChannelMute(playerMuteSolo, 0, false);
    } else if (channel == 2) {//right
        (*playerMuteSolo)->SetChannelMute(playerMuteSolo, 1, false);
        (*playerMuteSolo)->SetChannelMute(playerMuteSolo, 0, true);
    }
}

void AudioPlayer::release() {
    LOGE(TAG, "############ begin release ############");
    clock = 0;
    lastClock = 0;
    LOGE(TAG, "############ set exit to true ############");
//    pthread_mutex_lock(&prepareMutex);
//    pthread_mutex_lock(&decodeMutex);
    if (mAvPacket != nullptr) {
        av_packet_free(&mAvPacket);
        mAvPacket = nullptr;
    }
    if (packetQueue != nullptr) {
        packetQueue->clear();
        LOGE(TAG, "mPacketQueue cleared!!!");
    }
    if (audioPlayerObject != nullptr) {
        (*audioPlayerObject)->Destroy(audioPlayerObject);
        audioPlayerObject = nullptr;
        audioPlayer = nullptr;
        playerBufferQueue = nullptr;
        LOGE(TAG, "audioPlayerObject released!!!");
    }

    if (avCodecContext != nullptr) {
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = nullptr;
        LOGE(TAG, "avCodecContext freed!!!");
    }
//    if (avFormatContext != nullptr) {
//        avformat_close_input(&avFormatContext);
//        avformat_free_context(avFormatContext);
//        avFormatContext = nullptr;
//        LOGE(TAG, "avFormatContext freed!!!");
//    }
    if (swrContext != nullptr) {
        swr_close(swrContext);
        swr_free(&swrContext);
        swrContext = nullptr;
    }
//    if (soundTouch != nullptr) {
//        delete (soundTouch);
//        soundTouch = nullptr;
//    }
//    if (sampleBuffer != nullptr) {
//        free(sampleBuffer);
//        sampleBuffer = nullptr;
//    }
    LOGE(TAG, "############ end release ############");

}

void AudioPlayer::pause() {
    if (audioPlayer != nullptr) {
        (*audioPlayer)->SetPlayState(audioPlayer, SL_PLAYSTATE_PAUSED);
    }
}

void AudioPlayer::resume() {
    if (audioPlayer != nullptr) {
        (*audioPlayer)->SetPlayState(audioPlayer, SL_PLAYSTATE_PLAYING);
    }
}

void AudioPlayer::stop() {
    if (audioPlayer != nullptr) {
        (*audioPlayer)->SetPlayState(audioPlayer, SL_PLAYSTATE_PLAYING);
    }
}

void AudioPlayer::seekTo(int time) {
    packetQueue->clear();
    clock = 0;
    lastClock = 0;
    LOGE(TAG, "the queue size = %d", packetQueue->getQueueSize());
}

SLuint32 convertSampleRate(int sampleRate) {
    SLuint32 slSampleRate = 0;

    switch (sampleRate) {
        case 8000:
            slSampleRate = SL_SAMPLINGRATE_8;
            break;
        case 12000:
            slSampleRate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            slSampleRate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            slSampleRate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            slSampleRate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            slSampleRate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            slSampleRate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            slSampleRate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            slSampleRate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            slSampleRate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            slSampleRate = SL_SAMPLINGRATE_96;
            break;
        default:
            slSampleRate = SL_SAMPLINGRATE_44_1;
    }
    return slSampleRate;
}




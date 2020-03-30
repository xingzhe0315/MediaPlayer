//
// Created by xingzhe on 2020/3/22.
//

#include "MediaPlayer.h"

MediaPlayer::MediaPlayer() {
    audioPlayer = new AudioPlayer();
    videoPlayer = new VideoPlayer();
    pthread_mutex_init(&mSeekMutex, nullptr);
    pthread_mutex_init(&mDecodeMutex, nullptr);
}

MediaPlayer::~MediaPlayer() {
    pthread_mutex_destroy(&mSeekMutex);
    pthread_mutex_destroy(&mDecodeMutex);
}

void MediaPlayer::setListener(MediaPlayerListener *listener) {
    mListener = listener;
    if (videoPlayer != nullptr) {
        videoPlayer->setListener(listener);
    }
    if (audioPlayer != nullptr) {
        audioPlayer->setListener(listener);
    }
}

void MediaPlayer::setDataSource(const char *dataSource) {
    mDataSource = dataSource;
}

void *_prepare(void *data) {
    MediaPlayer *mediaPlayer = static_cast<MediaPlayer *>(data);
    mediaPlayer->status = MEDIA_PLAYER_PREPARING;
    AVFormatContext *avFormatContext = avformat_alloc_context();
    int result = -1;
    result = avformat_open_input(&avFormatContext, mediaPlayer->mDataSource, nullptr, nullptr);
    if (result != 0) {
        mediaPlayer->status = MEDIA_PLAYER_STATE_ERROR;
        return nullptr;
    }
    result = avformat_find_stream_info(avFormatContext, nullptr);
    if (result < 0) {
        mediaPlayer->status = MEDIA_PLAYER_STATE_ERROR;
        return nullptr;
    }
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        AVStream *stream = avFormatContext->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            mediaPlayer->videoPlayer->index = i;
            mediaPlayer->videoPlayer->initParamsFromStream(stream);
        } else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            mediaPlayer->audioPlayer->index = i;
            mediaPlayer->audioPlayer->initParamsFromStream(stream);
        }
    }
    mediaPlayer->duration = avFormatContext->duration / AV_TIME_BASE;
    mediaPlayer->videoPlayer->openStream();
    mediaPlayer->audioPlayer->openStream();
    if (mediaPlayer->mListener != nullptr) {
        mediaPlayer->mListener->notify(MEDIA_PREPARED, THREAD_WORK);
    }
    mediaPlayer->status = MEDIA_PLAYER_PREPARED;
    mediaPlayer->avFormatContext = avFormatContext;
    pthread_exit(&mediaPlayer->mPreparedThread);
}

void MediaPlayer::prepare() {
    _prepare(this);
}

void MediaPlayer::prepareAsync() {
    pthread_create(&mPreparedThread, nullptr, _prepare, this);
}

void *decode(void *data) {
    MediaPlayer *mediaPlayer = static_cast<MediaPlayer *>(data);
    LOGE(TAG, "开始解码");
    int retryCount = 0;
    int count = 0;
    while (mediaPlayer->status == MEDIA_PLAYER_STARTED) {
        AVPacket *avPacket = av_packet_alloc();
        int ret = -1;

        pthread_mutex_lock(&mediaPlayer->mDecodeMutex);
        pthread_mutex_lock(&mediaPlayer->mSeekMutex);
        if (mediaPlayer->avFormatContext != nullptr) {
            ret = av_read_frame(mediaPlayer->avFormatContext, avPacket);
        }
        pthread_mutex_unlock(&mediaPlayer->mSeekMutex);
        pthread_mutex_unlock(&mediaPlayer->mDecodeMutex);
        if (ret != 0) {
            LOGE(TAG, "read frame result = %d", ret);
            if (retryCount < 10) {
                av_usleep(100 * 1000);
                retryCount++;
                continue;
            }
            break;
        }
        if (avPacket->stream_index == mediaPlayer->videoPlayer->index) {
            mediaPlayer->videoPlayer->pushPacket(avPacket);
//            LOGE(TAG, "read packet count = %d", ++count);
//            LOGE(TAG, "push a video buffer to queue,the size is %d",
//                 mediaPlayer->videoPlayer->packetQueue->getQueueSize());
        } else if (avPacket->stream_index == mediaPlayer->audioPlayer->index) {
            mediaPlayer->audioPlayer->pushPacket(avPacket);
        }
    }
    LOGE(TAG, "解码完成");
    mediaPlayer->mDecodeFinished = true;
    pthread_exit(&mediaPlayer->mDecodeThread);
}

void MediaPlayer::start() {
    if (status & (MEDIA_PLAYER_STARTED | MEDIA_PLAYER_PAUSED)) return;

    if (status & (MEDIA_PLAYER_PREPARED | MEDIA_PLAYER_PLAYBACK_COMPLETE | MEDIA_PLAYER_STOPPED)) {
        status = MEDIA_PLAYER_STARTED;
        pthread_create(&mDecodeThread, nullptr, decode, this);
        if (audioPlayer != nullptr) {
            audioPlayer->play();
        }
        if (videoPlayer != nullptr) {
            videoPlayer->play();
        }
    }
}

void MediaPlayer::stop() {
    if (status & MEDIA_PLAYER_STOPPED) return;

    status = MEDIA_PLAYER_STOPPED;
    if (audioPlayer != nullptr) {
        audioPlayer->stop();
    }
    if (videoPlayer != nullptr) {
        videoPlayer->stop();
    }
}

void MediaPlayer::release() {
    pthread_mutex_lock(&mDecodeMutex);
    if (audioPlayer != nullptr) {
        audioPlayer->release();
    }
    if (videoPlayer != nullptr) {
        videoPlayer->release();
    }
    if (avFormatContext != nullptr) {
        avformat_close_input(&avFormatContext);
        avformat_free_context(avFormatContext);
        avFormatContext = nullptr;
        LOGE(TAG, "avFormatContext freed!!!");
    }
    pthread_mutex_unlock(&mDecodeMutex);
}

void MediaPlayer::pause() {
    if (status & MEDIA_PLAYER_STARTED) {
        status = MEDIA_PLAYER_PAUSED;
        if (audioPlayer != nullptr) {
            audioPlayer->pause();
        }
        if (videoPlayer != nullptr) {
            videoPlayer->pause();
        }
    }

}

void MediaPlayer::resume() {
    if (status & MEDIA_PLAYER_PAUSED) {
        status = MEDIA_PLAYER_STARTED;
        if (audioPlayer != nullptr) {
            audioPlayer->resume();
        }
        if (videoPlayer != nullptr) {
            videoPlayer->resume();
        }
    }

}

void *_seekTo(void *data) {
    MediaPlayer *mediaPlayer = static_cast<MediaPlayer *>(data);
    int timeInSeconds = static_cast<int>(mediaPlayer->duration * mediaPlayer->mSeekTime * 1.0F /
                                         100.0);
    auto audioTimeStamp = (double) timeInSeconds / av_q2d(mediaPlayer->audioPlayer->timeBase);
    auto videoTimeStamp = (double) timeInSeconds / av_q2d(mediaPlayer->videoPlayer->timeBase);
    LOGE(TAG, "seek to audio ts =  %f,video ts = %f", audioTimeStamp, videoTimeStamp);
    if (mediaPlayer->audioPlayer != nullptr) {
        mediaPlayer->audioPlayer->seekTo(audioTimeStamp);
    }
    if (mediaPlayer->videoPlayer != nullptr) {
        mediaPlayer->videoPlayer->seekTo(videoTimeStamp);
    }
    pthread_mutex_lock(&mediaPlayer->mSeekMutex);
    if (mediaPlayer->mDecodeFinished) {
        mediaPlayer->mDecodeFinished = false;
        pthread_create(&mediaPlayer->mDecodeThread, nullptr, decode, mediaPlayer);
    }
    avcodec_flush_buffers(mediaPlayer->audioPlayer->avCodecContext);
    avcodec_flush_buffers(mediaPlayer->videoPlayer->avCodecContext);
//    avformat_seek_file(mediaPlayer->avFormatContext, mediaPlayer->audioPlayer->index, 0,
//                       ts, INT64_MAX, 0);
    av_seek_frame(mediaPlayer->avFormatContext, mediaPlayer->audioPlayer->index, audioTimeStamp, 0);
    av_seek_frame(mediaPlayer->avFormatContext, mediaPlayer->videoPlayer->index, videoTimeStamp, 0);
    pthread_mutex_unlock(&mediaPlayer->mSeekMutex);
    pthread_exit(&mediaPlayer->mSeekThread);
}

void MediaPlayer::seekTo(int percent) {
    mSeekTime = percent;
    pthread_create(&mSeekThread, nullptr, _seekTo, this);

}

void MediaPlayer::setVolume(jint volume) {
    if (audioPlayer != nullptr) {
        audioPlayer->setVolume(volume);
    }
}

void MediaPlayer::setSoundChannel(jint channel) {
    if (audioPlayer != nullptr) {
        audioPlayer->setSoundChannel(channel);
    }
}

void MediaPlayer::setPitch(float pitch) {

}

void MediaPlayer::setSpeed(float speed) {

}

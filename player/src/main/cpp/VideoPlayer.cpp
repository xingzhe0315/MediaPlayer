//
// Created by xingzhe on 2020/3/22.
//

#include "VideoPlayer.h"

void VideoPlayer::initExtraParams() {
    width = avCodecContext->width;
    height = avCodecContext->height;
    AVPixelFormat pixFmt = AV_PIX_FMT_YUV420P;
    swsContext = sws_getContext(
            width,
            height,
            avCodecContext->pix_fmt,
            width,
            height,
            pixFmt, SWS_BILINEAR, nullptr, nullptr, nullptr);
    yuvFrame = av_frame_alloc();
    bufferSize = av_image_get_buffer_size(pixFmt, width, height, 1);
    buffer = static_cast<uint8_t *>(av_malloc(static_cast<size_t>(bufferSize)));
    av_image_fill_arrays(yuvFrame->data, yuvFrame->linesize, buffer, pixFmt, width,
                         height, 1);

}

void *_playVideo(void *data) {
    VideoPlayer *videoPlayer = static_cast<VideoPlayer *>(data);
    int count = 0;
    while (videoPlayer->status == MEDIA_PLAYER_STARTED) {
        if (videoPlayer->packetQueue->getQueueSize() <= 0) {
            av_usleep(10 * 1000);
            continue;
        }
        AVPacket *avPacket = av_packet_alloc();
        AVFrame *avFrame = av_frame_alloc();
        videoPlayer->packetQueue->getPacket(avPacket);
        int result = -1;
        result = avcodec_send_packet(videoPlayer->avCodecContext, avPacket);
        if (result != 0) {
            LOGE(TAG, "avcodec_send_packet error = %d", result);
            av_packet_unref(avPacket);
            av_packet_free(&avPacket);
            avPacket = nullptr;
            av_frame_free(&avFrame);
            avFrame = nullptr;
            continue;
        }
        result = avcodec_receive_frame(videoPlayer->avCodecContext, avFrame);
        if (result == 0) {
            sws_scale(videoPlayer->swsContext, avFrame->data, avFrame->linesize, 0, avFrame->height,
                      videoPlayer->yuvFrame->data, videoPlayer->yuvFrame->linesize);
            if (videoPlayer->mListener != nullptr) {
                videoPlayer->mListener->onYUVCallback(videoPlayer->width,
                                                      videoPlayer->height,
                                                      videoPlayer->yuvFrame->data[0],
                                                      videoPlayer->yuvFrame->data[1],
                                                      videoPlayer->yuvFrame->data[2]);
            }
        } else {
            LOGE(TAG, "receive frame error = %d", ++result);
            av_packet_unref(avPacket);
            av_packet_free(&avPacket);
            avPacket = nullptr;
            av_frame_free(&avFrame);
            avFrame = nullptr;
            continue;
        }
        av_packet_unref(avPacket);
        av_packet_free(&avPacket);
        avPacket = nullptr;
        av_frame_free(&avFrame);
        avFrame = nullptr;
    }
}

void VideoPlayer::play() {
    LOGE(TAG, "video play start status = %d", status);
//    if (status & MEDIA_PLAYER_STARTED) return;

    status = MEDIA_PLAYER_STARTED;
    pthread_create(&mPlayThread, nullptr, _playVideo, this);
}

int VideoPlayer::resampleData() {
    return 0;
}

void VideoPlayer::release() {

}

VideoPlayer::VideoPlayer() {

}

VideoPlayer::~VideoPlayer() {

}

void VideoPlayer::pause() {

}

void VideoPlayer::resume() {

}

void VideoPlayer::stop() {

}

void VideoPlayer::seekTo(int time) {
    packetQueue->clear();
}


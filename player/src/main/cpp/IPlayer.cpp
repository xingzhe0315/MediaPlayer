//
// Created by xingzhe on 2020/3/22.
//
#include "IPlayer.h"

void IPlayer::initParamsFromStream(AVStream *avStream) {
    sampleRate = avStream->codecpar->sample_rate;
    timeBase = avStream->time_base;
    duration = avStream->duration / AV_TIME_BASE;
    avCodecContext = avcodec_alloc_context3(nullptr);
    if (avcodec_parameters_to_context(avCodecContext, avStream->codecpar) < 0) {
        LOGE(TAG, "error on convert parameters to context");
    }
    initExtraParams();
}

void IPlayer::openStream() {
    AVCodec *avCodec = avcodec_find_decoder(avCodecContext->codec_id);
    if (avCodec == nullptr) {
        LOGE(TAG, "error find decoder");
        return;
    }
    if (avcodec_open2(avCodecContext, avCodec, nullptr) != 0) {
        LOGE(TAG, "error open codec");
    }
}

IPlayer::IPlayer() {
    packetQueue = new PacketQueue();
}

IPlayer::~IPlayer() {
    free(packetQueue);
}

void IPlayer::pushPacket(AVPacket *packet) {
    if (packetQueue != nullptr) {
        packetQueue->putPacket(packet);
    }
}

void IPlayer::setListener(MediaPlayerListener *listener) {
    mListener = listener;
}

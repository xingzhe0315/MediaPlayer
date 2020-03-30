//
// Created by xingzhe on 2020-02-13.
//

#ifndef AUDIOPLAYER_AUDIOPACKETQUEUE_H
#define AUDIOPLAYER_AUDIOPACKETQUEUE_H

#include "queue"
#include <pthread.h>

#define PACKET_QUEUE_CAPACITY 200
#define PACKET_QUEUE_LOW_LIMIT 50

extern "C" {
#include <libavcodec/avcodec.h>
};

#define TAG "PacketQueue"

#include "../log_define.h"


class PacketQueue {
public:
    std::queue<AVPacket *> mPacketQueue;
    pthread_mutex_t mPacketMutex;
    pthread_cond_t mEmptyCond;
    pthread_cond_t mFullCond;

    PacketQueue();

    ~PacketQueue();

    int putPacket(AVPacket *packet);

    int getPacket(AVPacket *packet);

    int getQueueSize();

    void clear();
};


#endif //AUDIOPLAYER_AUDIOPACKETQUEUE_H

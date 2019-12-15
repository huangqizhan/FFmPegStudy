//
//  packet_pool.cpp
//  FFM
//
//  Created by 黄麒展 on 2019/10/23.
//  Copyright © 2019 8km. All rights reserved.
//

#include "packet_pool.h"


PacketPool::PacketPool(){
}
PacketPool::~PacketPool(){
}

PacketPool * PacketPool::instance = new PacketPool();

PacketPool *PacketPool::getInstance(){
    return instance;
}
/** 原唱队列  */
void PacketPool::initDecoderOriginalSongPacketQueue() {
    const char* name = "decoder original song packet queue";
    decoderOriginalSongPacketQueue = new PacketQueue(name);
}
/** 退出原唱队列  */
void PacketPool::abortDecoderOriginalSongPacketQueue() {
    if(NULL != decoderOriginalSongPacketQueue){
        decoderOriginalSongPacketQueue->abort();
    }
}
/** 销毁原唱队列  */
void PacketPool::destoryDecoderOriginalSongPacketQueue() {
    if(NULL != decoderOriginalSongPacketQueue){
        delete decoderOriginalSongPacketQueue;
        decoderOriginalSongPacketQueue = NULL;
    }
}
/** 原唱队列获取数据  */
int PacketPool::getDecoderOriginalSongPacket(AudioPacket **audioPacket, bool block) {
    int result = -1;
    if(NULL != decoderOriginalSongPacketQueue){
        result = decoderOriginalSongPacketQueue->get(audioPacket, block);
    }
    return result;
}
/** 原唱队列size */
int PacketPool::getDecoderOriginalSongPacketQueueSize() {
    return decoderOriginalSongPacketQueue->size();
}
/** 原唱队列填充数据  */
void PacketPool::pushDecoderOriginalSongPacketToQueue(AudioPacket* audioPacket) {
    decoderOriginalSongPacketQueue->put(audioPacket);
}
/** 原唱队列清空数据  */
void PacketPool::clearDecoderOriginalSongPacketToQueue() {
    decoderOriginalSongPacketQueue->flush();
}

/**解码伴奏队列 */
void PacketPool::initDecoderAccompanyPacketQueue() {
    const char* name = "decoder accompany packet queue";
    decoderAccompanyPacketQueue = new PacketQueue(name);
}

void PacketPool::abortDecoderAccompanyPacketQueue() {
    if(NULL != decoderAccompanyPacketQueue){
        decoderAccompanyPacketQueue->abort();
    }
}

void PacketPool::destoryDecoderAccompanyPacketQueue() {
    if(NULL != decoderAccompanyPacketQueue){
        delete decoderAccompanyPacketQueue;
        decoderAccompanyPacketQueue = NULL;
    }
}

int PacketPool::getDecoderAccompanyPacket(AudioPacket **audioPacket, bool block) {
    int result = -1;
    if(NULL != decoderAccompanyPacketQueue){
        result = decoderAccompanyPacketQueue->get(audioPacket, block);
    }
    return result;
}

int PacketPool::geDecoderAccompanyPacketQueueSize() {
    return decoderAccompanyPacketQueue->size();
}

void PacketPool::clearDecoderAccompanyPacketToQueue() {
    decoderAccompanyPacketQueue->flush();
}

void PacketPool::pushDecoderAccompanyPacketToQueue(AudioPacket* audioPacket) {
    decoderAccompanyPacketQueue->put(audioPacket);
}
/** 人声队列操作 */

void PacketPool::initAudioPacketQueue() {
    //    LOGI("PacketPool::initAudioPacket queue");
    const char* name = "audioPacket queue";
    audioPacketQueue = new PacketQueue(name);
}
void PacketPool::abortAudioPacketQueue() {
    //    LOGI("PacketPool::abortAudioPacketQueue queue");
    if(NULL != audioPacketQueue){
        //        LOGI("PacketPool::abortAudioPacketQueue queue");
        audioPacketQueue->abort();
    }
}
void PacketPool::destoryAudioPacketQueue() {
    //    LOGI("PacketPool::destoryAudioPacket queue");
    if(NULL != audioPacketQueue){
        delete audioPacketQueue;
        audioPacketQueue = NULL;
    }
}

int PacketPool::getAudioPacket(AudioPacket **audioPacket, bool block) {
    //    LOGI("PacketPool::getAudioPacket");
    int result = -1;
    if(NULL != audioPacketQueue){
        result = audioPacketQueue->get(audioPacket, block);
    }
    return result;
}

int PacketPool::getAudioPacketQueueSize() {
    return audioPacketQueue->size();
}

void PacketPool::pushAudioPacketToQueue(AudioPacket* audioPacket) {
    //    LOGI("PacketPool::pushAudioPacketToQueue");
    audioPacketQueue->put(audioPacket);
}

void PacketPool::clearAudioPacketToQueue() {
    audioPacketQueue->flush();
}
/** 伴奏的packet queue的所有操作 **/

void PacketPool::initAccompanyPacketQueue() {
    //    LOGI("PacketPool::initAccompanyPacket queue");
    const char* name = "accompanyPacket queue";
    accompanyPacketQueue = new PacketQueue(name);
}
void PacketPool::abortAccompanyPacketQueue() {
    //    LOGI("PacketPool::abortAccompanyPacket queue");
    if(NULL != accompanyPacketQueue){
        //        LOGI("PacketPool::abortAccompanyPacket queue");
        accompanyPacketQueue->abort();
    }
}
void PacketPool::destoryAccompanyPacketQueue() {
    //    LOGI("PacketPool::destoryAccompanyPacket queue");
    if(NULL != accompanyPacketQueue){
        delete accompanyPacketQueue;
        accompanyPacketQueue = NULL;
    }
}

int PacketPool::getAccompanyPacket(AudioPacket **accompanyPacket, bool block) {
    //    LOGI("PacketPool::getAccompanyPacket");
    int result = -1;
    if(NULL != accompanyPacketQueue){
        result = accompanyPacketQueue->get(accompanyPacket, block);
    }
    return result;
}

int PacketPool::getAccompanyPacketQueueSize() {
    return accompanyPacketQueue->size();
}

void PacketPool::pushAccompanyPacketToQueue(AudioPacket* accompanyPacket) {
    //    LOGI("PacketPool::pushAccompanyPacketToQueue");
    accompanyPacketQueue->put(accompanyPacket);
}

void PacketPool::clearAccompanyPacketQueue() {
    if (accompanyPacketQueue != NULL) {
        accompanyPacketQueue->flush();
    }
}
/** 直播队列 */
void PacketPool::initLivePacketQueue() {
    //    LOGI("PacketPool::initLivePacket queue");
    const char* name = "livePacket queue";
    livePacketQueue = new PacketQueue(name);
}
void PacketPool::abortLivePacketQueue() {
    //    LOGI("PacketPool::abortLivePacket queue");
    if(NULL != livePacketQueue){
        livePacketQueue->abort();
    }
}
void PacketPool::destoryLivePacketQueue() {
    //    LOGI("PacketPool::destoryLivePacket queue");
    if(NULL != livePacketQueue){
        delete livePacketQueue;
        livePacketQueue = NULL;
    }
}

int PacketPool::getLivePacket(AudioPacket **livePacket, bool block) {
    //    LOGI("PacketPool::getLivePacket");
    int result = -1;
    if(NULL != livePacketQueue){
        result = livePacketQueue->get(livePacket, block);
    }
    return result;
}

int PacketPool::getLivePacketQueueSize() {
    int result = -1;
    if(NULL != livePacketQueue){
        result = livePacketQueue->size();
    }
    return result;
}

void PacketPool::pushLivePacketToQueue(AudioPacket* livePacket) {
    //    LOGI("PacketPool::pushLivePacketToQueue");
    livePacketQueue->put(livePacket);
}
/** 直播发送的packet queue的所有操作 **/
void PacketPool::initLiveSubscriberPacketQueue() {
    //    LOGI("PacketPool::initLiveSubscriberPacket queue");
    const char* name = "liveSubscriberPacket queue";
    liveSubstriberPacketQueue = new PacketQueue(name);
}
void PacketPool::abortLiveSubscriberPacketQueue() {
    //    LOGI("PacketPool::abortLiveSubscriberPacket queue");
    if(NULL != liveSubstriberPacketQueue){
        //        LOGI("PacketPool::abortLiveSubscriberPacket queue");
        liveSubstriberPacketQueue->abort();
    }
}
void PacketPool::destoryLiveSubscriberPacketQueue() {
    //    LOGI("PacketPool::destoryLiveSubscriberPacket queue");
    if(NULL != liveSubstriberPacketQueue){
        delete liveSubstriberPacketQueue;
        liveSubstriberPacketQueue = NULL;
    }
}

int PacketPool::getLiveSubscriberPacket(AudioPacket **livePacket, bool block) {
    //    LOGI("PacketPool::getLivePacket");
    int result = -1;
    if(NULL != liveSubstriberPacketQueue){
        result = liveSubstriberPacketQueue->get(livePacket, block);
    }
    return result;
}

int PacketPool::getLiveSubscriberPacketQueueSize() {
    return liveSubstriberPacketQueue->size();
}

void PacketPool::pushLiveSubscriberPacketToQueue(AudioPacket* livePacket) {
    //    LOGI("PacketPool::pushLivePacketToQueue");
    liveSubstriberPacketQueue->put(livePacket);
}
/** 试音的packet queue的所有操作 **/
void PacketPool::initTuningPacketQueue() {
    //    LOGI("PacketPool::initTuningPacket queue");
    const char* name = "tuningPacket queue";
    thingPacketQueue = new PacketQueue(name);
}
void PacketPool::abortTuningPacketQueue() {
    //    LOGI("PacketPool::abortTuningPacket queue");
    if(NULL != thingPacketQueue){
        thingPacketQueue->abort();
    }
}
void PacketPool::destoryTuningPacketQueue() {
    //    LOGI("PacketPool::destoryTuningPacket queue");
    if(NULL != thingPacketQueue){
        delete thingPacketQueue;
        thingPacketQueue = NULL;
    }
}

int PacketPool::getTuningPacket(AudioPacket **tuningPacket, bool block) {
    //    LOGI("PacketPool::getLivePacket");
    int result = -1;
    if(NULL != thingPacketQueue){
        result = thingPacketQueue->get(tuningPacket, block);
    }
    return result;
}

int PacketPool::getTuningPacketQueueSize() {
    return thingPacketQueue->size();
}

void PacketPool::pushTuningPacketToQueue(AudioPacket* livePacket) {
    //    LOGI("PacketPool::pushLivePacketToQueue");
    thingPacketQueue->put(livePacket);
}




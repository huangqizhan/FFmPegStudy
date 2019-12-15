//
//  packet_queue.cpp
//  FFM
//
//  Created by 黄麒展 on 2019/10/23.
//  Copyright © 2019 8km. All rights reserved.
//

#include "packet_queue.h"

PacketQueue::PacketQueue(){
    init();
}
PacketQueue::PacketQueue(const char *queueNameParam){
    init();
    queueName = queueNameParam;
}
void PacketQueue::init(){
    int initLockCode = pthread_mutex_init(&mLock, NULL);
    printf("initLockCode %d \n",initLockCode);
    int initCondLockCode = pthread_cond_init(&mCondtion, NULL);
    printf("initCondLockCode %d \n",initCondLockCode);
    mNbPacket = 0;
    mFirst = NULL;
    mLast = NULL;
    mAbortRequest = false;
}

PacketQueue::~PacketQueue(){
    printf("PacketQueue destory %s \n",queueName);
    flush();
    pthread_cond_destroy(&mCondtion);
    pthread_mutex_destroy(&mLock);
}

int PacketQueue::size(){
    pthread_mutex_lock(&mLock);
    int size = mNbPacket;
    printf("size %d \n",size);
    pthread_mutex_unlock(&mLock);
    return size;
}

/** 清空数据*/
void PacketQueue::flush(){
    AudioPacketList *pck,*pck1;
    AudioPacket *audioPacket;
    pthread_mutex_lock(&mLock);
    
    for (pck = mFirst; pck != NULL; pck = pck1) {
        pck1 = pck->next;
        audioPacket = pck->pkt;
        if (audioPacket != NULL) {
            delete audioPacket;
        }
        delete pck;
        pck = NULL;
    }
    mFirst = NULL;
    mLast = NULL;
    mNbPacket = 0;
    pthread_mutex_unlock(&mLock);
}

/** 填充数据 */
int PacketQueue::put(AudioPacket *packet){
    if (mAbortRequest) {
        delete packet;
        return -1;
    }
    
    AudioPacketList *pck1 = new AudioPacketList();
    if (!pck1) {
        return -1;
    }
    pck1->pkt = packet;
    pck1->next = NULL;
    
    pthread_mutex_lock(&mLock);
    
    if (mLast == NULL) {
        mFirst = pck1;
    }else{
        mLast->next = pck1;
    }
    
    mLast = pck1;
    mNbPacket ++ ;
    /** 给pthread_cond_wait 发送信号   */
    pthread_cond_signal(&mCondtion);
    pthread_mutex_unlock(&mLock);
    return 0;
}

int PacketQueue::get(AudioPacket **packet, bool block){
    AudioPacketList *pck1;
    int res = 0;
    pthread_mutex_lock(&mLock);
    for (;;){
        if (mAbortRequest) {
            res = -1;
            break;
        }
        pck1 = mFirst;
        if (pck1) {
            mFirst = pck1->next;
            if (!mFirst) {
                mLast = NULL;
            }
            mNbPacket--;
            *packet = pck1->pkt;
            delete pck1;
            res = 1;
            break;
        }else if (!block){
            res = 0;
        }else{
            /** 等待数据 */
            pthread_cond_wait(&mCondtion, &mLock);
        }
    }
    pthread_mutex_unlock(&mLock);
    return res;
}


void PacketQueue::abort(){
    pthread_mutex_lock(&mLock);
    mAbortRequest = true ;
    pthread_cond_signal(&mCondtion);
    pthread_mutex_unlock(&mLock);
}


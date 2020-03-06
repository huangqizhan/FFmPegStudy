//
//  packet_queue.hpp
//  FFM
//
//  Created by 黄麒展 on 2019/10/23.
//  Copyright © 2019 hqz. All rights reserved.
//

#ifndef packet_queue_hpp
#define packet_queue_hpp

#include <stdio.h>
#include <pthread.h>
#include <string.h>


#define byte uint8_t

#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

/** 语音数据包 */
typedef struct AudioPacket{
    /** 播放*/
    static const int AUDIO_PACKET_ACTION_PLAY = 0;
    /** 暂停 */
    static const int AUDIO_PACKET_ACTION_PAUSE = 100;
    /** 快进快退*/
    static const int AUDIO_PACKET_ACTION_SEEK = 101;
    
    /**  数据*/
    short * buffer;
    /** 数据size */
    int size;
    /** 位置 */
    float position;
    /** 操作 */
    int action;
    /** 参数1 */
    float extra_param1;
    /** 参数2 */
    float extra_param2;
    
    /** 初始化 */
    AudioPacket(){
        buffer = 0;
        size = 0;
        position = -1;
        action = 0;
        extra_param1 = 0;
        extra_param2 = 0;
    }
    
    /** destory */
    ~AudioPacket(){
        if(buffer != NULL){
            delete[] buffer;
            buffer = NULL;
        }
    }
}AudioPacket;


/** 数据集 */
typedef struct AudioPacketList {
    /**packet list */
    AudioPacket *pkt;
    /**next list */
    struct AudioPacketList *next;
    AudioPacketList(){
        pkt = NULL;
        next = NULL;
    }
    
} AudioPacketList;

/** build packet 填充数据  */
inline void buildPacketFromBuffer(AudioPacket *packet,short *sample , int sampleSize){
    short *packetBuffer = new short[sampleSize];
    if (packetBuffer != NULL) {
        memcpy(packetBuffer, sample, sampleSize*2);
        packet->buffer = packetBuffer;
        packet->size = sampleSize;
    }else{
        packet->size = -1;
    }
}

class PacketQueue {
public:
    PacketQueue();
    PacketQueue(const char *queueNameParam);
    ~PacketQueue();
    
    /** action */
    void init();
    /** 清空数据 */
    void flush();
    /** 填充数据*/
    int put(AudioPacket *packet);
    
    /* 获取数据   < 0 推出;  > 0 正常 ; = 0 无数据 */
    int get(AudioPacket **packet,bool block);
    
    int size();
    void abort();
    
private:
    /** 数据 */
    AudioPacketList *mFirst;
    AudioPacketList *mLast;
    
    /** packet 数量 */
    int mNbPacket;
    /**推出标志 */
    bool mAbortRequest;
    
    pthread_mutex_t mLock;
    pthread_cond_t mCondtion;
    const char*queueName;
};



#endif /* packet_queue_hpp */

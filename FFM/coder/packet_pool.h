//
//  packet_pool.hpp
//  FFM
//
//  Created by 黄麒展 on 2019/10/23.
//  Copyright © 2019 hqz. All rights reserved.
//

#ifndef packet_pool_hpp
#define packet_pool_hpp

#include <stdio.h>
#include "packet_queue.h"


class PacketPool {
private:
    /** 构造函数 */
    PacketPool();
    /**单例 */
    static PacketPool *instance;
    /** 边录音边合并的 伴奏 packet queue */
    PacketQueue *accompanyPacketQueue;
    /** 边录音边合并的 人声音 packet queue */
    PacketQueue *audioPacketQueue;
    /** 直播---发送的packet queue **/
    PacketQueue *livePacketQueue;
    /** 直播---接收的packet queue **/
    PacketQueue *liveSubstriberPacketQueue;
    /** 试音---的packet queue **/
    PacketQueue *thingPacketQueue;
    /** 解码线程---解码出来的伴奏的queue **/
    PacketQueue *decoderAccompanyPacketQueue;
    /** 解码线程---解码出来的原唱的queue **/
    PacketQueue *decoderOriginalSongPacketQueue;
    
public:
    /** 获取实例 */
    static PacketPool *getInstance();
    virtual ~PacketPool();
    
    /** 解码出来的伴奏的queue的所有操作 **/
    virtual void initDecoderAccompanyPacketQueue();
    virtual void abortDecoderAccompanyPacketQueue();
    virtual void destoryDecoderAccompanyPacketQueue();
    virtual int getDecoderAccompanyPacket(AudioPacket **audioPacket, bool block);
    virtual void pushDecoderAccompanyPacketToQueue(AudioPacket* audioPacket);
    virtual void clearDecoderAccompanyPacketToQueue();
    virtual int geDecoderAccompanyPacketQueueSize();
    
    /** 解码出来的原唱的queue的所有操作 **/
    virtual void initDecoderOriginalSongPacketQueue();
    virtual void abortDecoderOriginalSongPacketQueue();
    virtual void destoryDecoderOriginalSongPacketQueue();
    virtual int getDecoderOriginalSongPacket(AudioPacket **audioPacket, bool block);
    virtual void pushDecoderOriginalSongPacketToQueue(AudioPacket* audioPacket);
    virtual void clearDecoderOriginalSongPacketToQueue();
    virtual int getDecoderOriginalSongPacketQueueSize();
    
    /** 人声的packet queue的所有操作 **/
    virtual void initAudioPacketQueue();
    virtual void abortAudioPacketQueue();
    virtual void destoryAudioPacketQueue();
    virtual int getAudioPacket(AudioPacket **audioPacket, bool block);
    virtual void pushAudioPacketToQueue(AudioPacket* audioPacket);
    virtual void clearAudioPacketToQueue();
    virtual int getAudioPacketQueueSize();
    
    /** 伴奏的packet queue的所有操作 **/
    virtual void initAccompanyPacketQueue();
    virtual void abortAccompanyPacketQueue();
    virtual void destoryAccompanyPacketQueue();
    virtual int getAccompanyPacket(AudioPacket **accompanyPacket, bool block);
    virtual void pushAccompanyPacketToQueue(AudioPacket* accompanyPacket);
    virtual int getAccompanyPacketQueueSize();
    virtual void clearAccompanyPacketQueue();
    
    /** 直播发送的packet queue的所有操作 **/
    virtual void initLivePacketQueue();
    virtual void abortLivePacketQueue();
    virtual void destoryLivePacketQueue();
    virtual int getLivePacket(AudioPacket **livePacket, bool block);
    virtual void pushLivePacketToQueue(AudioPacket* livePacket);
    virtual int getLivePacketQueueSize();
    
    /** 直播接收的packet queue的所有操作 **/
    virtual void initLiveSubscriberPacketQueue();
    virtual void abortLiveSubscriberPacketQueue();
    virtual void destoryLiveSubscriberPacketQueue();
    virtual int getLiveSubscriberPacket(AudioPacket **livePacket, bool block);
    virtual void pushLiveSubscriberPacketToQueue(AudioPacket* livePacket);
    virtual int getLiveSubscriberPacketQueueSize();
    
    /** 试音的packet queue的所有操作 **/
    virtual void initTuningPacketQueue();
    virtual void abortTuningPacketQueue();
    virtual void destoryTuningPacketQueue();
    virtual int getTuningPacket(AudioPacket **livePacket, bool block);
    virtual void pushTuningPacketToQueue(AudioPacket* livePacket);
    virtual int getTuningPacketQueueSize();
    
};


#endif /* packet_pool_hpp */

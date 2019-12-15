//
//  dump.c
//  FFM
//
//  Created by 黄麒展 on 2019/12/11.
//  Copyright © 2019 8km. All rights reserved.
//

#include "dump.h"
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/log.h>
#include <libavformat/avio.h>

#define ADTS_HEADER_LEN  7;
void adts_header(char *szAdtsHeader, int dataLen){
    
    int audio_object_type = 2;
    int sampling_frequency_index = 7;
    int channel_config = 2;
    
    int adtsLen = dataLen + 7;
    
    szAdtsHeader[0] = 0xff;         //syncword:0xfff                          高8bits
    szAdtsHeader[1] = 0xf0;         //syncword:0xfff                          低4bits
    szAdtsHeader[1] |= (0 << 3);    //MPEG Version:0 for MPEG-4,1 for MPEG-2  1bit
    szAdtsHeader[1] |= (0 << 1);    //Layer:0                                 2bits
    szAdtsHeader[1] |= 1;           //protection absent:1                     1bit
    
    szAdtsHeader[2] = (audio_object_type - 1)<<6;            //profile:audio_object_type - 1                      2bits
    szAdtsHeader[2] |= (sampling_frequency_index & 0x0f)<<2; //sampling frequency index:sampling_frequency_index  4bits
    szAdtsHeader[2] |= (0 << 1);                             //private bit:0                                      1bit
    szAdtsHeader[2] |= (channel_config & 0x04)>>2;           //channel configuration:channel_config               高1bit
    
    szAdtsHeader[3] = (channel_config & 0x03)<<6;     //channel configuration:channel_config      低2bits
    szAdtsHeader[3] |= (0 << 5);                      //original：0                               1bit
    szAdtsHeader[3] |= (0 << 4);                      //home：0                                   1bit
    szAdtsHeader[3] |= (0 << 3);                      //copyright id bit：0                       1bit
    szAdtsHeader[3] |= (0 << 2);                      //copyright id start：0                     1bit
    szAdtsHeader[3] |= ((adtsLen & 0x1800) >> 11);           //frame length：value   高2bits
    
    szAdtsHeader[4] = (uint8_t)((adtsLen & 0x7f8) >> 3);     //frame length:value    中间8bits
    szAdtsHeader[5] = (uint8_t)((adtsLen & 0x7) << 5);       //frame length:value    低3bits
    szAdtsHeader[5] |= 0x1f;                                 //buffer fullness:0x7ff 高5bits
    szAdtsHeader[6] = 0xfc;
}
void dump_info(const char *path){
    /// 创建format上下文
    AVFormatContext *fmt_ctx = avformat_alloc_context();
    int ret = 0;
    /// 设置log 级别
    av_log_set_level(AV_LOG_INFO);
    ///打开输入
    ret = avformat_open_input(&fmt_ctx, path, NULL, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "%s open input error",av_err2str(ret));
        return ;
    }
    /// 获取多媒体文件信息
    av_dump_format(fmt_ctx , 0, path, 0);
    /// 关闭输入
    avformat_close_input(&fmt_ctx);
    
}


int  extrct_audio(const char *scr , const char *dest){
    /// 创建format上下文
    AVFormatContext *fmt_ctx = avformat_alloc_context();
    /// 音频流的索引
    int audio_index = 0;
    int ret = 0;
    /// 设置log 级别
    av_log_set_level(AV_LOG_INFO);
    ///打开输入
    ret = avformat_open_input(&fmt_ctx, scr, NULL, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "%s open input error\n",av_err2str(ret));
        return -1;
    }
    ///目标文件
    FILE *dst_df = fopen(dest, "wb");
    if (dst_df == NULL) {
        /// 关闭输入
        avformat_close_input(&fmt_ctx);
        av_log(NULL, AV_LOG_ERROR, "connt open dest file \n");
        return -1;
    }
    /// 获取多媒体文件信息
    av_dump_format(fmt_ctx , 0, scr, 0);
    
    ///读取音频stream
    ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "find best stream error\n");
        fclose(dst_df);
        return -1;
    }
    audio_index = ret;
     /// 创建包
    AVPacket *packet = av_packet_alloc();
    ///从流里面读取包
    while (av_read_frame(fmt_ctx, packet) >= 0) {
        ///音频流
        if (packet->stream_index == audio_index) {
            /// 写入数据之前添加adts头
            char adts_header_buf[7];
            adts_header(adts_header_buf, packet->size);
            /// 先写入dts
            fwrite(adts_header_buf, 1, 7, dst_df);
            /// 写入数据
            size_t len = fwrite(packet->data, 1, packet->size, dst_df);
            if (len != packet->size) {
                av_log(NULL, AV_LOG_ERROR, "connot write data\n");
            }
        }
        av_packet_unref(packet);
    }
    /// 关闭输入
    avformat_close_input(&fmt_ctx);
    ///关闭文件
    if (dst_df) {
        fclose(dst_df);
    }
    return 0;
}





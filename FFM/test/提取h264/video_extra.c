//
//  video_extra.c
//  FFM
//
//  Created by 黄麒展 on 2019/12/14.
//  Copyright © 2019 hqz. All rights reserved.
//

#include "video_extra.h"

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/log.h>
#include <libavformat/avio.h>


#ifndef AV_WB32
#   define AV_WB32(p, val) do {                 \
uint32_t d = (val);                     \
((uint8_t*)(p))[3] = (d);               \
((uint8_t*)(p))[2] = (d)>>8;            \
((uint8_t*)(p))[1] = (d)>>16;           \
((uint8_t*)(p))[0] = (d)>>24;           \
} while(0)
#endif

#ifndef AV_RB16
#   define AV_RB16(x)                           \
((((const uint8_t*)(x))[0] << 8) |          \
((const uint8_t*)(x))[1])
#endif

///一般帧处理
int alloc_and_copy(AVPacket *out,
                   const uint8_t *sps_pps, uint32_t sps_pps_size,
                   const uint8_t *in, uint32_t in_size){
    ///输出packet的偏移量
    uint32_t offset = out->size;
    ///nal unit 的header 部分
    uint8_t nal_header_size = offset ? 3 : 4;

    int err;
    /// 增加packet的size
    err = av_grow_packet(out, sps_pps_size + nal_header_size + in_size);
    if (err < 0) {
        return  err;
    }
    ///拷贝数据到内存
    if (sps_pps)
        memcpy(out->data + offset, sps_pps, sps_pps_size);
    memcpy(out->data + sps_pps_size + nal_header_size + offset, in, in_size);
    if (!offset) {
        AV_WB32(out->data + sps_pps_size, 1);
    } else {
        (out->data + offset + sps_pps_size)[0] =
        (out->data + offset + sps_pps_size)[1] = 0;
        (out->data + offset + sps_pps_size)[2] = 1;
    }
    
    return 0;
}

/// 关键帧处理
int h264_extradata_to_annexb(const uint8_t *codec_extradata,const int codec_extradata_size,AVPacket *out_extradata, int padding){
    uint16_t unit_size;
    uint64_t total_size                 = 0;
    uint8_t *out                        = NULL, unit_nb, sps_done = 0,
    sps_seen                   = 0, pps_seen = 0, sps_offset = 0, pps_offset = 0;
    const uint8_t *extradata            = codec_extradata + 4;
    static const uint8_t nalu_header[4] = { 0, 0, 0, 1 };
    int length_size = (*extradata++ & 0x3) + 1; // retrieve length coded size, 用于指示表示编码数据长度所需字节数
    
    sps_offset = pps_offset = -1;
    
    /* retrieve sps and pps unit(s) */
    unit_nb = *extradata++ & 0x1f; /* number of sps unit(s) */
    if (!unit_nb) {
        goto pps;
    }else {
        sps_offset = 0;
        sps_seen = 1;
    }
    
    while (unit_nb--) {
        int err;
        
        unit_size   = AV_RB16(extradata);
        total_size += unit_size + 4;
        if (total_size > INT_MAX - padding) {
            av_log(NULL, AV_LOG_ERROR,
                   "Too big extradata size, corrupted stream or invalid MP4/AVCC bitstream\n");
            av_free(out);
            return AVERROR(EINVAL);
        }
        if (extradata + 2 + unit_size > codec_extradata + codec_extradata_size) {
            av_log(NULL, AV_LOG_ERROR, "Packet header is not contained in global extradata, "
                   "corrupted stream or invalid MP4/AVCC bitstream\n");
            av_free(out);
            return AVERROR(EINVAL);
        }
        if ((err = av_reallocp(&out, total_size + padding)) < 0)
            return err;
        memcpy(out + total_size - unit_size - 4, nalu_header, 4);
        memcpy(out + total_size - unit_size, extradata + 2, unit_size);
        extradata += 2 + unit_size;
    pps:
        if (!unit_nb && !sps_done++) {
            unit_nb = *extradata++; /* number of pps unit(s) */
            if (unit_nb) {
                pps_offset = total_size;
                pps_seen = 1;
            }
        }
    }
    
    if (out)
        memset(out + total_size, 0, padding);
    
    if (!sps_seen)
        av_log(NULL, AV_LOG_WARNING,
               "Warning: SPS NALU missing or invalid. "
               "The resulting stream may not play.\n");
    
    if (!pps_seen)
        av_log(NULL, AV_LOG_WARNING,
               "Warning: PPS NALU missing or invalid. "
               "The resulting stream may not play.\n");
    
    out_extradata->data      = out;
//    out_extradata->size      = total_size;
    
    return length_size;
}

int h264_mp4toannexb(AVFormatContext *fmt_ctx,AVPacket *pkt_in , FILE *dst_fd){
    AVPacket *pkt_out = NULL;
    AVPacket spspps_pkt;
    size_t len = 0;
    
    uint8_t unit_type;
    ///此32位4个字节用来存储每一帧的大小
    int32_t nal_size;
    uint32_t cumul_size    = 0;
    
    const uint8_t *buf;
    const uint8_t *buf_end;
    int            buf_size;
    int ret = 0, i;
    
    
    pkt_out = av_packet_alloc();
    
    buf = pkt_in->data;
    buf_size = pkt_in->size;
    buf_end = pkt_in->data + pkt_in->size;
    
    /// 每一帧的处理
    do {
        if (buf + 4 > buf_end) {
            goto fail;
        }
        ///填充前4个字节的数据到nal_size
        for (nal_size = 0, i = 0; i<4; i++){
            nal_size = (nal_size << 8) | buf[i];
        }
        /// buf地址向后移动4个字节
        buf += 4;
        ///获取第一个字节的后五位 (存储帧的类型)
        unit_type = *buf & 0x1f;
        ///剩下的数据不够nal_size
        if (nal_size > buf_end - buf || nal_size < 0) {
            goto fail;
        }
        ///如果当前帧是关键帧就会获取sps pps 图像信息
        if (unit_type == 5) {
            h264_extradata_to_annexb( fmt_ctx->streams[pkt_in->stream_index]->codecpar->extradata,
                                     fmt_ctx->streams[pkt_in->stream_index]->codecpar->extradata_size,
                                     &spspps_pkt,
                                     AV_INPUT_BUFFER_PADDING_SIZE);
            
            if ((ret=alloc_and_copy(pkt_out,
                                    spspps_pkt.data, spspps_pkt.size,
                                    buf, nal_size)) < 0)
                goto fail;
        }else{
            if ((ret=alloc_and_copy(pkt_out, NULL, 0, buf, nal_size)) < 0)
                goto fail;
        }
        len = fwrite(pkt_out->data, 1, pkt_out->size, dst_fd);
        if(len != pkt_out->size){
            av_log(NULL, AV_LOG_DEBUG, "warning, length of writed data isn't equal pkt.size(%zu, %d)\n",
                   len,
                   pkt_out->size);
        }
        fflush(dst_fd);
        
    next_nal:
        buf        += nal_size;
        cumul_size += nal_size + 4;//s->length_size;
    } while (cumul_size < buf_size);
fail:
    return 0;
}
int video_exteac(const char *scr_filename , const char *out_filename){
    FILE *dst_df = NULL;
    /// 创建format上下文
    AVFormatContext *fmt_ctx = avformat_alloc_context();
    AVPacket packet;
    /// 流的索引
    int video_index = 0;
    int ret = 0;
    /// 设置log 级别
    av_log_set_level(AV_LOG_INFO);
    ///打开输入
    ret = avformat_open_input(&fmt_ctx, scr_filename, NULL, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "%s open input error\n",av_err2str(ret));
        return -1;
    }
    ///目标文件
    dst_df = fopen(out_filename, "wb");
    if (dst_df == NULL) {
        /// 关闭输入
        avformat_close_input(&fmt_ctx);
        av_log(NULL, AV_LOG_ERROR, "connt open dest file \n");
        return -1;
    }
    /// 获取多媒体文件信息
    av_dump_format(fmt_ctx , 0, scr_filename, 0);
    
    ///读取音频stream
    ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "find best stream error\n");
        fclose(dst_df);
        return -1;
    }
    video_index = ret;
    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;
    ///从流里面读取包
    while (av_read_frame(fmt_ctx, &packet) >= 0) {
        ///音频流
        if (packet.stream_index == video_index) {
            h264_mp4toannexb(fmt_ctx, &packet, dst_df);
        }
        av_packet_unref(&packet);
    }
    /// 关闭输入
    avformat_close_input(&fmt_ctx);
    ///关闭文件
    if (dst_df) {
        fclose(dst_df);
    }
    return 0;
}



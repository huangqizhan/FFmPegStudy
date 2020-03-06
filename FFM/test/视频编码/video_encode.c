//
//  video_encode.c
//  FFM
//
//  Created by 黄麒展 on 2019/12/31.
//  Copyright © 2019 hqz. All rights reserved.
//

#include "video_encode.h"
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>


int video_encode(const char *file_name,const char *code_name){
    AVCodecContext *code_ctx;
    AVCodec *codec;
    int i ,ret,x,y;
    FILE *file;
    AVFrame *frame;
    AVPacket *packet = NULL;
    uint8_t endcode[] = {0,0,1,0xb7};
    
    /// 查找AVCodec
    codec = avcodec_find_decoder_by_name(code_name);
    if (!codec) {
        printf("find codec error\n");
        goto end;
    }
    /// 创建AVCodecContext
    code_ctx = avcodec_alloc_context3(codec);
    if (!code_ctx) {
        printf("alloc context error\n");
        goto end;
    }
    ///码率
    code_ctx->bit_rate = 400000;
    /// 宽高
    code_ctx->width = 352;
    code_ctx->height = 288;
    /// 时间基
    code_ctx->time_base = (AVRational){1,25};
    /// 帧率
    code_ctx->framerate = (AVRational){25,1};
    
    /// group of frames
    code_ctx->gop_size = 10;
    /// b 帧的数量
    code_ctx->max_b_frames = 1;
    /// 像素格式
    code_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    
    if (codec->id == AV_CODEC_ID_H264) {
        av_opt_set(code_ctx->priv_data, "preset", "slow", 0);
    }
    
    /// 打开编码器
    ret = avcodec_open2(code_ctx, codec, NULL);
    if (ret < 0) {
        printf("codec open error\n");
        goto end;
    }
    
    file = fopen(file_name, "wb");
    if (!file) {
        printf("open file error %s \n",file_name);
        goto end;
    }
    
    frame = av_frame_alloc();
    if (!frame) {
        printf("frame alloc error\n");
        goto end;
    }
    
    frame->format = code_ctx->pix_fmt;
    frame->width = code_ctx->width;
    frame->height = code_ctx->height;

    /// 填充每一帧数据
    for (i = 0; i < 25; i++) {
        av_init_packet(packet);
        packet->data = NULL;
        packet->size = 0;
        fflush(stdout);
        
        ret = av_frame_make_writable(frame);
        if (ret < 0) {
            printf("make frame writeable error\n");
            goto end;
        }
        
        /// YUV 三个分量数据
        
        /* y */
        for (y = 0; y < code_ctx->height; y++) {
            for (x = 0; x < code_ctx->width; x++) {
                frame->data[0][y*frame->linesize[0]+x] = x + y + i*3;
            }
        }
        
        for (y = 0; y < code_ctx->height; y++) {
            for (x = 0; x < code_ctx->width; x++) {
                frame->data[1][y*frame->linesize[1]+x] = 128 + y + i * 2;
                frame->data[2][y*frame->linesize[2]+x] = 64 + x + i * 5;
            }
        }
        
        frame->pts = i;
        
        /// 编码图像数据
        
        ret = avcodec_receive_packet(code_ctx, packet);
        if (ret < 0) {
            printf("receive frame to code_ctx error \n");
            goto end;
        }
        
        ret = avcodec_send_frame(code_ctx, frame);
        if (ret < 0) {
            printf("send frame to code_ctx error \n");
            goto end;
        }
        
        /// 填充数据
        ret = av_frame_get_buffer(frame, 32);
        if (ret < 0 && ret != AVERROR_EOF && ret != AVERROR(EAGAIN)) {
            printf("frame allocte data error \n");
            goto end;
        }
        
        size_t rt = fwrite(packet->data, 1, packet->size, file);
        av_packet_unref(packet);
        if (rt < 0) {
            printf("write data error \n");
            goto end;
        }
    }
    /* add sequence end code to have a real MPEG file */
    fwrite(endcode, 1, sizeof(endcode), file);
    fclose(file);
    
    avcodec_free_context(&code_ctx);
    av_frame_free(&frame);
end:
    return 0;
    
}



int av_encode_frame(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *packet){
    int ret = -1;
    // 第一次发送flush packet会返回成功，进入冲洗模式，可调用avcodec_receive_packet()
    // 将编码器中缓存的帧(可能不止一个)取出来
    // 后续再发送flush packet将返回AVERROR_EOF
    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret == AVERROR_EOF){
        //av_log(NULL, AV_LOG_INFO, "avcodec_send_frame() encoder flushed\n");
    }else if (ret == AVERROR(EAGAIN)){
        //av_log(NULL, AV_LOG_INFO, "avcodec_send_frame() need output read out\n");
    }else if (ret < 0){
        //av_log(NULL, AV_LOG_INFO, "avcodec_send_frame() error %d\n", ret);
        return ret;
    }
    
    ret = avcodec_receive_packet(enc_ctx, packet);
    if (ret == AVERROR_EOF){
        av_log(NULL, AV_LOG_INFO, "avcodec_recieve_packet() encoder flushed\n");
    }else if (ret == AVERROR(EAGAIN)){
        //av_log(NULL, AV_LOG_INFO, "avcodec_recieve_packet() need more input\n");
    }
    
    return ret;
}

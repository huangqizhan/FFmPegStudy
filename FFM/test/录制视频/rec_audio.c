//
//  rec_audio.c
//  Media
//
//  Created by 黄麒展 on 2021/1/28.
//

#include "rec_audio.h"
#include <unistd.h>
#include <libavdevice/avdevice.h>
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>


void encode(AVCodecContext *code_ctx,AVFrame *frame,AVPacket *code_packet, FILE *outfile){
    int ret = 0;
    ret = avcodec_send_frame(code_ctx, frame);
    while (ret >= 0) {
        ret = avcodec_receive_packet(code_ctx, code_packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }else if (ret < 0 ){
            av_log(NULL, AV_LOG_DEBUG, "code frame error");
            exit(-1);
        }
        size_t size = fwrite(code_packet->data, 1, code_packet->size, outfile);
        printf("write size = %zu \n",size);
        fflush(outfile);
    }
}

void taction(){
    av_log_set_level(AV_LOG_DEBUG);
    
    int ret = 0;
    /// 注册所有的设备
    avdevice_register_all();
    /// 格式上下文
    AVFormatContext *ifmt_ctx = NULL;
    /// 采集到的数据
    AVPacket packet;   /// 栈创建
    /// 设备号  （:前是视频序号  :后是音频序号）
    char *device_name = ":0";
    /// 输入格式
    AVInputFormat *ifmt = av_find_input_format("avfoundation");
    ///
    AVDictionary *option = NULL;
    /// 重采样输入缓冲区
    uint8_t **src_data = NULL;
    /// 缓冲区大小
    int src_linesize = 0;
    /// 重采样的输出缓冲区
    uint8_t **dst_data = NULL;
    /// 缓冲区大小
    int dst_linesize = 0;
    /// 打开设备
    ret = avformat_open_input(&ifmt_ctx, device_name, ifmt, &option);
    if (ret < 0) {
        av_log(NULL, AV_LOG_DEBUG, "open input error \n");
        goto __ERROR;
    }
    /// 错误信息 (创建字符数组)
    char err_str[1024] = {0};
    
    char *out = "/Users/huangqizhan/Library/Containers/com.huangqizhan.Media/Data/Documents/audio22.aac";
    /// 打开文件
    FILE *outfile = fopen(out, "wb+");
    if (outfile == NULL) {
        av_strerror(-1, err_str, 1024);
        goto __ERROR;
    }
    /// 重采样上下文
    SwrContext *swr_ctx = NULL;
    swr_ctx = swr_alloc_set_opts(NULL,
                       AV_CH_LAYOUT_STEREO, /// 输出采样布局
                       AV_SAMPLE_FMT_S16,   /// 输出采样大小
                       44100,               /// 输出采样率
                       AV_CH_LAYOUT_STEREO, /// 输入采样布局
                       AV_SAMPLE_FMT_FLT,   /// 输入采样大小
                       44100,               /// 输入采样率
                       0, NULL);
    if (swr_ctx == NULL) {
        av_log(NULL, AV_LOG_DEBUG, "swrContext create error");
        goto __ERROR;
    }
    /// 输出话上下文
    if (swr_init(swr_ctx) < 0) {
        av_log(NULL, AV_LOG_DEBUG, "swrContext create error");
        goto __ERROR;
    }
    
    /// 创建重新采样的输入缓冲区
    av_samples_alloc_array_and_samples(&src_data,       /// 输入数据
                                       &src_linesize,   ///输入数据的大小
                                       2,               ///声道数
                                       512,             /// 采集的数据是32位 每次采集大小为 4096  所以每个声道的采集数量为 4096/(32/8)/2
                                       AV_SAMPLE_FMT_FLT,/// 采样大小 32位
                                       0);
    /// 创建重采样的输出缓冲区
    av_samples_alloc_array_and_samples(&dst_data,
                                       &dst_linesize,
                                       2,
                                       512,
                                       AV_SAMPLE_FMT_S16,/// 输出采样大小 16位
                                       0);
//    set_status(1);
    
    ///编码
//    AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_AAC);
    AVCodec *codec = avcodec_find_encoder_by_name("libfdk_aac");
    
    /// 创建编码上下文
    AVCodecContext *code_ctx = avcodec_alloc_context3(codec);
    
    ///设置编码参数
    code_ctx->sample_fmt = AV_SAMPLE_FMT_S16;            /// libfdk_aac 编码器的采样大小为s16
    code_ctx->channel_layout = AV_CH_LAYOUT_STEREO;      /// 双声道
    code_ctx->channels = 2;                              ///声道数
    code_ctx->sample_rate = 44100;                       /// 采样率
    code_ctx->bit_rate = 48000;                              /// 码率
    code_ctx->profile = FF_PROFILE_AAC_HE ;            /// 设置aac的版本（设置此项的时候码率应该设置为0）
    
    /// 打开编码器
    ret = avcodec_open2(code_ctx, codec, NULL);
    if (ret < 0 ) {
        av_log(NULL, AV_LOG_DEBUG, "open codec_context error \n");
        goto __ERROR;
    }
    
    /// 创建编码用的frame （重新采样后的数据）
    AVFrame *frame = av_frame_alloc(); /// 堆创建
    if (!frame) {
        av_log(NULL, AV_LOG_DEBUG, "alloc frame error \n");
        goto __ERROR;
    }
    /// 设置frame参数
    frame->format = AV_SAMPLE_FMT_S16;              /// 采样大小
    frame->channels = 2;                            /// 声道数
    frame->channel_layout = AV_CH_LAYOUT_STEREO;    /// 声道布局
    frame->sample_rate = 44100;                     /// 采样率
    frame->nb_samples = 512;                        /// 每个声道一个音频帧的采样数
    if (!frame) {
        av_log(NULL, AV_LOG_DEBUG, "alloc frame error \n");
        goto __ERROR;
    }
    ///创建frame 的buffer 缓冲区
    ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_DEBUG, "get frame buffer error \n");
        goto __ERROR;
    }
    /// 创建编码后的packet
    AVPacket *code_packet = av_packet_alloc();      /// 堆创建
    if (code_packet == NULL) {
        av_log(NULL, AV_LOG_DEBUG, "create packet error \n");
        goto __ERROR;
    }
    while (1) {
        ret = av_read_frame(ifmt_ctx, &packet);
        // 如果返回-35 则表明设备还没有准备好
        if (ret == -35) {
//            sleep(1);
            continue;
        }
        if (ret == 0) {
            memcpy((void *)(src_data[0]), (const void *)packet.data, packet.size);
            // 如果只需要pcm数据则就不需要重采样和编码
            /// 重采样
            swr_convert(swr_ctx,                    ///重采样的上下文
                        dst_data,                   ///输入的缓冲区
                        512,                        ///每个声道的采样数
                        (const uint8_t **)src_data, ///输出的缓冲区
                        512);                       ///每个声道的采样数
            /// copy 数据到frame 中
            memcpy((void *)frame->data[0], dst_data[0], dst_linesize);
            /// 编码pcm数据
            encode(code_ctx, frame, code_packet,outfile);
            av_packet_unref(&packet);
        }
    }
    /// 编码结束是因为 编码区的缓冲数据不够一次编码  所以此处编码为编码的数据
    encode(code_ctx, NULL, code_packet,outfile);
    
__ERROR:
    if (outfile) {
        fclose(outfile);
    }
    if (src_data) {
        av_free(src_data[0]);
    }
    av_free(src_data);
    
    if (dst_data) {
        av_free(dst_data[0]);
    }
    av_free(dst_data);
    swr_free(&swr_ctx);
    avformat_close_input(&ifmt_ctx);
    if (frame) {
        av_frame_free(&frame);
    }
    av_log(NULL, AV_LOG_DEBUG, "resmple finished \n");
}




//
//  remixing_video.c
//  FFM
//
//  Created by 黄麒展 on 2019/12/16.
//  Copyright © 2019 hqz. All rights reserved.
//

#include "remixing_video.h"
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

///输出日志
static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag){
    AVRational *time_base = &(fmt_ctx->streams[pkt->stream_index]->time_base);
    printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           tag,
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           pkt->stream_index);
}
///文件格式转化
int remix_video(const char *src, const char *dest){
    ///输出格式
    AVOutputFormat *ofmt;
    /// 输出输入上下文
    AVFormatContext *ifmt_ctx = NULL,*ofmt_ctx = NULL;
    AVPacket pkt;
    int ret,i;
    ///流的索引
    int stream_index = 0;
    ///存储流数据
    int *stream_mapping = NULL;
    /// 流的数量
    int stream_mapping_size = 0;
    
    av_log_set_level(AV_LOG_INFO);
    /// 打开输入文件
    if (avformat_open_input(&ifmt_ctx,src, 0, 0) < 0) {
        av_log(NULL, AV_LOG_ERROR, "av format open input error");
        ret = -1;
        goto fail;
    }
    
    /// 查找数据流
    if (avformat_find_stream_info(ifmt_ctx, 0) < 0) {
        av_log(NULL, AV_LOG_ERROR, "avformat find stream error");
        ret = -1;
        goto fail;
    }
    /// 打印信息
    av_dump_format(ifmt_ctx, 0, src, 0);
    
    /// 输出
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, dest);
    if (!ofmt_ctx) {
        av_log(NULL, AV_LOG_ERROR, "ofmt ctx create error");
        ret = AVERROR_UNKNOWN;
        goto fail;
    }
    AV_VERSION_INT(<#a#>, <#b#>, <#c#>)
    stream_mapping_size = ifmt_ctx->nb_streams;
    
    av_malloc_array(2, 2);
    
//    stream_mapping = av_malloc_array(stream_mapping_size, sizeof(*stream_mapping));
//    if (!stream_mapping) {
//        ret = AVERROR(ENOMEM);
//        goto fail;
//    }
    
//    ofmt = ofmt_ctx->oformat;
//    ///给 ofmt_ctx 创建streams
//    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
//        ///输入输出流
//        AVStream *istream = ifmt_ctx->streams[i];
//        AVStream *ostream ;
//        /// 输入流的编码参数
//        AVCodecParameters *inpar = istream->codecpar;
//        
//        ///过滤视频流 音频流 和字幕流
//        printf("inpar->codec_type = %d",inpar->codec_type);
//        if (inpar->codec_type != AVMEDIA_TYPE_SUBTITLE && inpar->codec_type != AVMEDIA_TYPE_AUDIO && inpar->codec_type != AVMEDIA_TYPE_VIDEO) {
//            stream_mapping[i] = -1;
//            continue;
//        }
//        
//        stream_mapping[i] = stream_index++;
//        ///创建输出流
//        ostream = avformat_new_stream(ofmt_ctx, NULL);
//        if (!ostream) {
//            av_log(NULL, AV_LOG_ERROR, "avformat new out stream error");
//            ret = AVERROR_UNKNOWN;
//            goto fail;
//        }
//        /// 输出流设置编码参数
//        ret = avcodec_parameters_copy(ostream->codecpar, istream->codecpar);
//        if (ret < 0) {
//            av_log(NULL, AV_LOG_ERROR, "copy outstream codepar to in stream error");
//            ret = AVERROR_UNKNOWN;
//            goto fail;
//        }
//        ostream->codecpar->codec_tag = 0;
//    }
//    ////打印输出上下文信息
//    av_dump_format(ofmt_ctx, 0, dest, 1);
//    /// 如果输出格式不可写
//    if (!(ofmt->flags & AVFMT_NOFILE)) {
//        /// 重新打开
//        ret = avio_open(&ofmt_ctx->pb, dest, AVIO_FLAG_WRITE);
//        if (ret< 0) {
//            av_log(NULL, AV_LOG_ERROR, "avio open error filename %s",dest);
//            goto fail;
//        }
//    }
//    
//    /// 写入header
//    ret = avformat_write_header(ofmt_ctx, NULL);
//    if (ret < 0) {
//        av_log(NULL, AV_LOG_ERROR, "avformat write header error");
//        goto fail;
//    }
//    
//    while (1) {
//        AVStream *in_stream;
//        AVStream *out_stream;
//        ret = av_read_frame(ifmt_ctx, &pkt);
//        if (ret < 0) {
//            break;
//        }
//        in_stream = ifmt_ctx->streams[pkt.stream_index];
//        if (pkt.stream_index >= stream_mapping_size || stream_mapping[pkt.stream_index] < 0) {
//            av_packet_unref(&pkt);
//            break;
//        }
//        
//        pkt.stream_index = stream_mapping[pkt.stream_index];
//        out_stream = ofmt_ctx->streams[pkt.stream_index];
//        log_packet(ifmt_ctx, &pkt, "in");
//        
//        /// copy 数据
//        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
//        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
//        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
//        pkt.pos = -1;
//        log_packet(ofmt_ctx, &pkt, "out");
//        
//        ///插入pkt
//        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
//        if (ret < 0) {
//            av_log(NULL, AV_LOG_ERROR, "interleaved error");
//            break;
//        }
//        av_packet_unref(&pkt);
//    }
//    av_write_trailer(ofmt_ctx);
//    
fail:
    ///释放资源
    avformat_close_input(&ifmt_ctx);
    if (ofmt_ctx && !(ofmt_ctx->flags & AVFMT_NOFILE)) {
        avio_closep(&ofmt_ctx->pb);
    }
    avformat_free_context(ofmt_ctx);
    avformat_free_context(ifmt_ctx);
    av_free(stream_mapping);
    
    if (ret < 0 && ret != AVERROR_EOF) {
        av_log(NULL, AV_LOG_ERROR, "error occured");
        return -1;
    }
    return 0;
}

//
//  cut_video.c
//  FFM
//
//  Created by 黄麒展 on 2019/12/27.
//  Copyright © 2019 8km. All rights reserved.
//

#include "cut_video.h"
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>


static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *packet,const char *tag){

    AVRational time_base = fmt_ctx->streams[packet->stream_index]->time_base;
    printf("%s: pts : %s pts_time : %s dts :%s dts_time : %s duration : %s duration_time : %s \n",tag,av_ts2str(packet->pts),av_ts2timestr(packet->pts, &time_base),av_ts2str(packet->dts),av_ts2timestr(packet->dts, &time_base),av_ts2str(packet->duration),av_ts2timestr(packet->duration, &time_base));
}


int cut_video(double from_second,double end_second,const char *in_filename,const char *out_filename){

    AVFormatContext *ifmt_ctx = NULL;
    AVFormatContext *ofmt_ctx = NULL;
    AVOutputFormat *ofmt = NULL;
    AVPacket pkt;
    AVCodecContext *outcdeo_ctx = NULL;

    int ret , i;

    ret = avformat_open_input(&ifmt_ctx, in_filename, NULL, NULL);
    if (ret < 0){
        printf("open input error \n");
        goto end;
    }

    ret = avformat_find_stream_info(ifmt_ctx, NULL);
    if (ret < 0) {
        printf("find stream info error\n");
        goto end;
    }

    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    if (!ofmt_ctx) {
        printf("alloc output error \n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    ofmt = ofmt_ctx->oformat;

    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVCodec *in_codec = avcodec_find_decoder(in_stream->codecpar->codec_id);
        AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_codec);

        outcdeo_ctx = avcodec_alloc_context3(in_codec);

        /// 输入流的参数拷贝到输出编码上下文
        ret = avcodec_parameters_to_context(outcdeo_ctx,in_stream->codecpar);
        if (ret < 0) {
            printf("faild set parmerters to outcode_ctx \n");
            goto end;
        }
        ret = avcodec_parameters_from_context(out_stream->codecpar, outcdeo_ctx);
        if (ret < 0) {
            printf("faild set out_stream.codecpar \n");
            goto end;
        }
        outcdeo_ctx->codec_tag = 0;
        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
            outcdeo_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }
    }
    av_dump_format(ofmt_ctx, 0, out_filename, 1);
    /// 如果不是文件类型 重新打开
    if (!(ofmt_ctx->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret) {
            printf("avio open faild file_name = %s \n",out_filename);
            goto end;
        }
    }

    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        printf("write header faild \n");
        goto end;
    }

    /// 截取响应的位置
    ret = av_seek_frame(ifmt_ctx, -1, from_second*AV_TIME_BASE, AVSEEK_FLAG_ANY);
    if (ret) {
        printf("Error seek \n");
        goto end;
    }

    /// pts dts
    int64_t *dts_start_from = malloc(sizeof(int64_t)*ifmt_ctx->nb_streams);
    memset(dts_start_from, 0, sizeof(int64_t)*ifmt_ctx->nb_streams);
    int64_t *pts_start_from = malloc(sizeof(int64_t)*ifmt_ctx->nb_streams);
    memset(pts_start_from, 0, sizeof(int64_t)*ifmt_ctx->nb_streams);

    while (1) {
        AVStream *in_stream;
        AVStream *out_stream;
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0) {
            break;
        }

        in_stream = ifmt_ctx->streams[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];
        log_packet(ifmt_ctx, &pkt, "in");

        /// 如果当前pcket的pts大于截取的最终时间 break
        if (av_q2d(in_stream->time_base)* pkt.pts > end_second) {
            av_packet_unref(&pkt);
            break;
        }
        ///  设置dts  pts
        if (dts_start_from[pkt.stream_index] == 0) {
            dts_start_from[pkt.stream_index] = pkt.dts;
            printf("dts_start_from is %s\n",av_ts2str(dts_start_from[pkt.stream_index]));
        }

        if (pts_start_from[pkt.stream_index] == 0) {
            pts_start_from[pkt.stream_index] = pkt.pts;
            printf("pts_start_from is %s \n",av_ts2str(pts_start_from[pkt.stream_index]));
        }

        /*copy packet */
        pkt.pts = av_rescale_q_rnd(pkt.pts - pts_start_from[pkt.stream_index], in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
        pkt.dts = av_rescale_q_rnd(pkt.dts - dts_start_from[pkt.stream_index], in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
        if (pkt.pts < 0) {
            pkt.pts = 0;
        }
        if (pkt.dts < 0) {
            pkt.dts = 0;
        }
        pkt.duration = (int)av_rescale_q((int64_t)pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        log_packet(ofmt_ctx, &pkt, "out");
        /// 每一帧的pts必须在dts之前 （先编解码再显示 ）
        if(pkt.pts < pkt.dts) {
            continue;
        }
        /// 顺序插入
        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        if (ret < 0) {
            printf("interleaved write frame error \n");
            goto end;
        }
        av_packet_unref(&pkt);
    }
    free(pts_start_from);
    free(dts_start_from);
    av_write_trailer(ofmt_ctx);
end:
    av_packet_unref(&pkt);
    /// 关闭输入
    avformat_close_input(&ifmt_ctx);
    /// 关闭输出
    if (!ofmt_ctx && !(ofmt_ctx->flags & AVFMT_NOFILE)) {
        avio_closep(&ofmt_ctx->pb);
    }
    avformat_free_context(ofmt_ctx);
    if (ret < 0 ) {
        return ret;
    }
    return 0;
}


//static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag)
//{
//    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;
//
//    printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
//           tag,
//           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
//           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
//           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
//           pkt->stream_index);
//}
//
//int cut_video(double from_seconds, double end_seconds, const char* in_filename, const char* out_filename) {
//    AVOutputFormat *ofmt = NULL;
//    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
//    AVPacket pkt;
//    int ret, i;
//
//    av_register_all();
//
//    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
//        fprintf(stderr, "Could not open input file '%s'", in_filename);
//        goto end;
//    }
//
//    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
//        fprintf(stderr, "Failed to retrieve input stream information");
//        goto end;
//    }
//
//    av_dump_format(ifmt_ctx, 0, in_filename, 0);
//
//    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
//    if (!ofmt_ctx) {
//        fprintf(stderr, "Could not create output context\n");
//        ret = AVERROR_UNKNOWN;
//        goto end;
//    }
//
//    ofmt = ofmt_ctx->oformat;
//
//    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
//        AVStream *in_stream = ifmt_ctx->streams[i];
//        AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
//        if (!out_stream) {
//            fprintf(stderr, "Failed allocating output stream\n");
//            ret = AVERROR_UNKNOWN;
//            goto end;
//        }
//
//        ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
//        if (ret < 0) {
//            fprintf(stderr, "Failed to copy context from input to output stream codec context\n");
//            goto end;
//        }
//        out_stream->codec->codec_tag = 0;
//        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
//            out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
//    }
//    av_dump_format(ofmt_ctx, 0, out_filename, 1);
//
//    if (!(ofmt->flags & AVFMT_NOFILE)) {
//        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
//        if (ret < 0) {
//            fprintf(stderr, "Could not open output file '%s'", out_filename);
//            goto end;
//        }
//    }
//
//    ret = avformat_write_header(ofmt_ctx, NULL);
//    if (ret < 0) {
//        fprintf(stderr, "Error occurred when opening output file\n");
//        goto end;
//    }
//
//    //    int indexs[8] = {0};
//
//
//    //    int64_t start_from = 8*AV_TIME_BASE;
//    ret = av_seek_frame(ifmt_ctx, -1, from_seconds*AV_TIME_BASE, AVSEEK_FLAG_ANY);
//    if (ret < 0) {
//        fprintf(stderr, "Error seek\n");
//        goto end;
//    }
//
//    int64_t *dts_start_from = malloc(sizeof(int64_t) * ifmt_ctx->nb_streams);
//    memset(dts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);
//    int64_t *pts_start_from = malloc(sizeof(int64_t) * ifmt_ctx->nb_streams);
//    memset(pts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);
//
//    while (1) {
//        AVStream *in_stream, *out_stream;
//
//        ret = av_read_frame(ifmt_ctx, &pkt);
//        if (ret < 0)
//            break;
//
//        in_stream  = ifmt_ctx->streams[pkt.stream_index];
//        out_stream = ofmt_ctx->streams[pkt.stream_index];
//
//        log_packet(ifmt_ctx, &pkt, "in");
//
//        if (av_q2d(in_stream->time_base) * pkt.pts > end_seconds) {
//            av_free_packet(&pkt);
//            break;
//        }
//
//        if (dts_start_from[pkt.stream_index] == 0) {
//            dts_start_from[pkt.stream_index] = pkt.dts;
//            printf("dts_start_from: %s\n", av_ts2str(dts_start_from[pkt.stream_index]));
//        }
//        if (pts_start_from[pkt.stream_index] == 0) {
//            pts_start_from[pkt.stream_index] = pkt.pts;
//            printf("pts_start_from: %s\n", av_ts2str(pts_start_from[pkt.stream_index]));
//        }
//
//        /* copy packet */
//        pkt.pts = av_rescale_q_rnd(pkt.pts - pts_start_from[pkt.stream_index], in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
//        pkt.dts = av_rescale_q_rnd(pkt.dts - dts_start_from[pkt.stream_index], in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
//        if (pkt.pts < 0) {
//            pkt.pts = 0;
//        }
//        if (pkt.dts < 0) {
//            pkt.dts = 0;
//        }
//        pkt.duration = (int)av_rescale_q((int64_t)pkt.duration, in_stream->time_base, out_stream->time_base);
//        pkt.pos = -1;
//        log_packet(ofmt_ctx, &pkt, "out");
//        printf("\n");
//        if(pkt.pts < pkt.dts) continue;
//        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
//        if (ret < 0) {
//            fprintf(stderr, "Error muxing packet\n");
//            break;
//        }
//        av_free_packet(&pkt);
//    }
//    free(dts_start_from);
//    free(pts_start_from);
//
//    av_write_trailer(ofmt_ctx);
//end:
//
//    avformat_close_input(&ifmt_ctx);
//
//    /* close output */
//    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
//        avio_closep(&ofmt_ctx->pb);
//    avformat_free_context(ofmt_ctx);
//
//    if (ret < 0 && ret != AVERROR_EOF) {
//        fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
//        return 1;
//    }
//
//    return 0;
//}
//

//
//  remote_oull_stream.c
//  FFM
//
//  Created by hjb_mac_mini on 2019/12/19.
//  Copyright © 2019 8km. All rights reserved.
//

#include "remote_pull_stream.h"
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>


void pull_rem_stream(const char *src_path,const char *dest_path){
    
    char input_str_full[500]={0};
    char output_str_full[500]={0};
    
    sprintf(input_str_full,"%s",src_path);
    sprintf(output_str_full,"%s",dest_path);
    
    /// 输出格式
    AVOutputFormat *ofmt = NULL;
    /// 输出上下文 跟输入上下文
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVPacket pkt;
    char in_filename[500]={0};
    char out_filename[500]={0};
    int ret, i;
    int videoindex=-1;
    int frame_index=0;
    int64_t start_time=0;
    
    strcpy(in_filename,input_str_full);
    strcpy(out_filename,output_str_full);
    
    //Network
    avformat_network_init();
    //Input
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        printf( "Could not open input file.");
        goto end;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        printf( "Failed to retrieve input stream information");
        goto end;
    }
    
    for(i=0; i<ifmt_ctx->nb_streams; i++){
        if (avcodec_find_decoder(ifmt_ctx->streams[i]->codecpar->codec_id)->type == AVMEDIA_TYPE_VIDEO) {
            videoindex=i;
            break;
        }
    }
    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    //Output
    
    avformat_alloc_output_context2(&ofmt_ctx, NULL, "flv", out_filename);
    if (!ofmt_ctx) {
        printf( "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    ofmt = ofmt_ctx->oformat;
    /// 遍历每个输入流 添加到输出fmt_ctx中
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *in_stream = ifmt_ctx->streams[i];
        ///获取输入流的编码格式
        AVCodec *in_codec = avcodec_find_decoder(in_stream->codecpar->codec_id);
        /// 根据输入流编码创建输出流
        AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_codec);
        if (!out_stream) {
            printf( "Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }
        AVCodecContext *outcdeo_ctx = avcodec_alloc_context3(in_codec);
        /// 输入流的参数拷贝到输出编码上下文
        ret = avcodec_parameters_to_context(outcdeo_ctx,in_stream->codecpar);
        if (ret < 0) {
            printf("faild set parmerters to outcode_ctx");
            goto end;
        }
        ret = avcodec_parameters_from_context(out_stream->codecpar, outcdeo_ctx);
        if (ret < 0) {
            printf("faild set out_stream.codecpar");
            goto end;
        }
        out_stream->codecpar->codec_tag = 0;
        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            outcdeo_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    //Dump Format------------------
    av_dump_format(ofmt_ctx, 0, out_filename, 1);
    
    // 打开输出
    if (!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            printf( "Could not open output URL '%s'", out_filename);
            goto end;
        }
    }

    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        printf( "Error occurred when opening output URL\n");
        goto end;
    }
    
    start_time=av_gettime();
    while (1) {
        AVStream *in_stream, *out_stream;
        //Get an AVPacket
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
            break;
        //FIX：No PTS (Example: Raw H.264)
        //Simple Write PTS
        if(pkt.pts==AV_NOPTS_VALUE){
            //Write PTS
            AVRational time_base1=ifmt_ctx->streams[videoindex]->time_base;
            //Duration between 2 frames (us)
            int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(ifmt_ctx->streams[videoindex]->r_frame_rate);
            //Parameters
            pkt.pts=(double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
            pkt.dts=pkt.pts;
            pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
        }
        //Important:Delay
        if(pkt.stream_index==videoindex){
            AVRational time_base=ifmt_ctx->streams[videoindex]->time_base;
            AVRational time_base_q={1,AV_TIME_BASE};
            int64_t pts_time = av_rescale_q(pkt.pts, time_base, time_base_q);
            int64_t now_time = av_gettime() - start_time;
            if (pts_time > now_time){
                av_usleep((unsigned)(pts_time - now_time));
            }
        }
        in_stream  = ifmt_ctx->streams[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];
        /* copy packet */
        //Convert PTS/DTS
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        //Print to Screen
        if(pkt.stream_index==videoindex){
//            printf("Send %8d video frames to output URL pts = %lld duration = %lld \n",frame_index,pkt.pts,pkt.duration);
            frame_index++;
        }
        //ret = av_write_frame(ofmt_ctx, &pkt);
        /// 交叉写入 
        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        
        if (ret < 0) {
            printf( "Error muxing packet\n");
            break;
        }
//        av_free_packet(&pkt);
        av_packet_unref(&pkt);
    }
    //写文件尾（Write file trailer）
    av_write_trailer(ofmt_ctx);
end:
    avformat_close_input(&ifmt_ctx);
    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    if (ret < 0 && ret != AVERROR_EOF) {
        printf( "Error occurred.\n");
        return;
    }
    return;
    
}

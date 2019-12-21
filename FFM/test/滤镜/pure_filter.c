//
//  pure_filter.c
//  FFM
//
//  Created by hjb_mac_mini on 2019/12/18.
//  Copyright © 2019 8km. All rights reserved.
//

#include "pure_filter.h"



#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>

/// 给 yuv 数据添加滤镜  生成新的YUV数据
int pure_filter(const char *arc_path , const char *dest_path){
    int ret = 0;
    /// 输入输出帧
    AVFrame *frame_in;
    AVFrame *frame_out;
    /// 输入输出数据
    unsigned char *frame_buffer_in;
    unsigned char *frame_buffer_out;
    /// 输入输出滤镜上下文
    AVFilterContext *buffersink_ctx;
    AVFilterContext *buffersrc_ctx;
    /// 滤镜图像
    AVFilterGraph *filter_graph;
    /// 视频流索引
//    static int video_stream_index = -1;
    
    av_log_set_level(AV_LOG_INFO);
    /// 输入文件
    FILE *in_fd = fopen(arc_path, "rb+");
    if (!in_fd) {
        av_log(NULL, AV_LOG_ERROR, "in file open error\n");
        return -1;
    }
    
    int in_width = 448;
    int in_height = 960;
    
    FILE *out_fd = fopen(dest_path, "wb+");
    if (!out_fd) {
        av_log(NULL, AV_LOG_ERROR, "open out file error\n");
        return -1;
    }
    
    const char *filter_descr = "boxblur";
    
//    const char *filter_descr = "hflip";
//    const char *filter_descr = "hue='h=60:s=-3'";
//    const char *filter_descr = "crop=2/3*in_w:2/3*in_h";
//    const char *filter_descr = "drawbox=x=100:y=100:w=100:h=100:color=pink@0.9";
//    const char *filter_descr = "drawtext=fontfile=arial.ttf:fontcolor=green:fontsize=30:text='Lei Xiaohua'";
    
    avfilter_register_all();
    
    char args[512];
    /// 原滤镜
    const AVFilter *buffersrc  = avfilter_get_by_name("buffer");
    /// 输出滤镜
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    
    /// 滤镜的输出
    AVFilterInOut *outputs = avfilter_inout_alloc();
    /// 滤镜的输入
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    /// 像素格式
    enum AVPixelFormat pix_fmts[] = {AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE};
    /// 输入滤镜的参数
    AVBufferSinkParams *buffersink_params;
    
    filter_graph = avfilter_graph_alloc();
    
    snprintf(args, sizeof(args),
        "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
        in_width,in_height,AV_PIX_FMT_YUV420P,
        1, 25,1,1);
    
    /// 创建输入滤镜上下文
    ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
        args, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
        return ret;
    }
    buffersink_params = av_buffersink_params_alloc();
    buffersink_params->pixel_fmts = pix_fmts;
    
    /// 创建输出滤镜上下文
    ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out", NULL, buffersink_params, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink \n");
        return  -1;
    }
    
    outputs->name       = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;

    inputs->name       = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;
    
    
    ret = avfilter_graph_parse_ptr(filter_graph, filter_descr, &inputs, &outputs, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "add filter graph ptr error \n");
        return -1;
    }
    ret = avfilter_graph_config(filter_graph, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "filter graph config error \n");
        return -1;
    }
    
    /// 创建输入frame
    frame_in = av_frame_alloc();
    /// 输入数据
    frame_buffer_in = av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, in_width, in_height, 1));
    /// 填充数据
    av_image_fill_arrays(frame_in->data, frame_in->linesize, frame_buffer_in, AV_PIX_FMT_YUV420P, in_width, in_height, 1);
    
    //// 输出frame
    frame_out = av_frame_alloc();
    frame_buffer_out = av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, in_width, in_height, 1));
    av_image_fill_arrays(frame_out->data, frame_out->linesize, frame_buffer_out, AV_PIX_FMT_YUV420P, in_width, in_height,1);
    
    
    frame_in->width = in_width;
    frame_in->height = in_height;
    frame_in->format = AV_PIX_FMT_YUV420P;
    
    
    while (1) {
        if (fread(frame_buffer_in, 1, in_width*in_height*3/2, in_fd) != in_width*in_height*3/2) {
            av_log(NULL, AV_LOG_INFO, "fread error \n");
            break;
        }
        
        /// 输入YUV 数据
        frame_in->data[0] = frame_buffer_in;
        frame_in->data[1] = frame_buffer_in + in_width*in_height;
        frame_in->data[2] = frame_buffer_in + in_width*in_height*5/4;
        /// 添加frame_in
        ret = av_buffersrc_add_frame(buffersrc_ctx, frame_in);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "buffersrc add frame error \n");
            break;
        }
        
        /// 获取frame_out
        ret = av_buffersink_get_frame(buffersink_ctx, frame_out);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "buffersink get frame error\n");
            break;
        }
        if (frame_out->format == AV_PIX_FMT_YUV420P) {
         /// 写入内存
            /// Y 分量  每四个Y共用一组UV分量
            for (int i = 0; i < frame_out->height ; i++) {
                fwrite(frame_out->data[0]+frame_out->linesize[0]*i, 1, frame_out->width, out_fd);
                
            }
          for(int i=0;i<frame_out->height/2;i++){
                fwrite(frame_out->data[1]+frame_out->linesize[1]*i,1,frame_out->width/2,out_fd);
                   }
         for(int i=0;i<frame_out->height/2;i++){
                fwrite(frame_out->data[2]+frame_out->linesize[2]*i,1,frame_out->width/2,out_fd);
                }
        }
        printf("Process 1 frame!\n");
        av_frame_unref(frame_out);
    }
    fclose(in_fd);
    fclose(out_fd);

    av_frame_free(&frame_in);
    av_frame_free(&frame_out);
    avfilter_graph_free(&filter_graph);
    return 0;
}




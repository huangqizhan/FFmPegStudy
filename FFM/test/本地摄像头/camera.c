//
//  camera.c
//  FFM
//
//  Created by hjb_mac_mini on 2019/12/21.
//  Copyright © 2019 8km. All rights reserved.
//

#include "camera.h"

#define ENABLE_SDL 0



#if ENAGLE_SDL


#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <SDL2/SDL.h>

#define BLOCK_SIZE 4096000
int thread_exit = 0;

int sfp_refresh_thread(void *opaque){
    thread_exit=0;
    while (!thread_exit) {
        SDL_Event event;
        event.type = SDL_DISPLAYEVENT;
        SDL_PushEvent(&event);
        SDL_Delay(40);
    }
    thread_exit=0;
    //Break
    SDL_Event event;
    event.type = SDL_QUIT;
    printf("push quit event \n");
    printf("event = %d",event.type);
    SDL_PushEvent(&event);

    return 0;
}


void show_avfoundation_device(){
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    AVDictionary* options = NULL;
    av_dict_set(&options,"list_devices","true",0);
    AVInputFormat *iformat = av_find_input_format("avfoundation");
    printf("==AVFoundation Device Info===\n");
    avformat_open_input(&pFormatCtx,"",iformat,&options);
    printf("=============================\n");
}

int camera(int argc ,char *argv[]){
    AVFormatContext *fmt_ctx;
    int i , video_index,ret;
    AVCodecContext *cd_ctx;
    AVCodec *code;
    AVDictionary *options = NULL;
    
    /// init
    avformat_network_init();
    fmt_ctx = avformat_alloc_context();
    avdevice_register_all();
//    av_dict_set(&options, "list_devices", "true", 0);
    av_dict_set(&options, "framerate", "30", 0);
    av_dict_set(&options, "video_size", "640x480", 0);
    av_dict_set(&options, "pix_fmt", "uyvy422", 0);
    
    show_avfoundation_device();
       //Mac
    AVInputFormat *ifmt = av_find_input_format("avfoundation");
       printf("ifmt.name = %s \n",ifmt->name);
//    while (ifmt->next) {
//        ifmt = ifmt->next;
//        printf("ifmt.name = %s \n",ifmt->name);
//    }
     ret = avformat_open_input(&fmt_ctx,"0",ifmt,&options);
      if(ret != 0){
           printf("Couldn't open input stream %d\n",ret);
                return -1;
      }
    if(avformat_find_stream_info(fmt_ctx,NULL)<0){
        printf("Couldn't find stream information.\n");
        return -1;
    }
    
    video_index = -1;
     for(i=0; i<fmt_ctx->nb_streams; i++){
         if(fmt_ctx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO){
             video_index = i;
             break;
         }
     }
    
    if (video_index < 0) {
        printf("not find video stream \n %d",ret);
        return -1;
    }
    printf("video_index = %d \n",video_index);

    /// 编码参数
    AVCodecParameters *pars = fmt_ctx->streams[video_index]->codecpar;
    /// 编码器
    code = avcodec_find_decoder(pars->codec_id);
    if (code == NULL) {
        printf("not find code\n");
        return -1;
    }
    /// 编码上下文
//    cd_ctx = fmt_ctx->streams[video_index]->codec;
    
        cd_ctx = avcodec_alloc_context3(code);
        ret = avcodec_parameters_to_context(cd_ctx,pars);
    
    if (cd_ctx == NULL) {
        printf("not find code_ctx \n");
        return -1;
    }
    ret = avcodec_open2(cd_ctx, code, NULL);
    if (ret < 0) {
        printf("avcodec open error \n");
        return -1;
    }
    
    /*  SDL   */
    SDL_Event event; // 监听事件
    SDL_Rect rect; // 矩形框

    Uint32 pixformat = 0; // 像素格式

    SDL_Window *win = NULL; // 窗口
    SDL_Renderer *renderer = NULL; // 渲染器
    SDL_Texture *texture = NULL; // 纹理

    SDL_Thread *timer_thread = NULL; // 每帧切换时间管理线程
    // 窗口宽高
    int w_width = cd_ctx->width, w_height = cd_ctx->height;
    // 视频展示宽高(纹理宽高)
    const int video_width = cd_ctx->width, video_height = cd_ctx->height;

    Uint8 *video_pos = NULL; // video_buf的首地址
    Uint8 *video_end = NULL; // video_buf的尾地址

    unsigned int remain_len = 0;
    unsigned int video_buff_len = 0; // 一张YUV图片的长度
    unsigned int blank_space_len = 0;
    Uint8 video_buf[BLOCK_SIZE]; // 读取的一张yuv图片的指针

    // 一帧YUV数据的大小是 width * height *1.5(即12/8)，用 12/8是为了计算的更快
    const unsigned int yuv_frame_len = video_width * video_height * 12 / 8;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
      printf("sdl init error\n");
      return -1;
    }
    // 创建窗口
    win = SDL_CreateWindow("camera",
                         SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED,
                         w_width, w_height,
                         SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    if(!win) {
      printf("create win error ！\n");
      return -1;
    }
    
// 创建渲染器
   renderer = SDL_CreateRenderer(win, -1, 0);

   // 像素格式
   pixformat= SDL_PIXELFORMAT_UYVY;

   // 创建纹理
   texture = SDL_CreateTexture(renderer,
                               pixformat,
                               SDL_TEXTUREACCESS_STREAMING,
                               video_width,
                               video_height);
    if (texture == NULL) {
        printf("texture create error \n");
    }
    
    struct SwsContext *img_convert_ctx;
     img_convert_ctx = sws_getContext(cd_ctx->width, cd_ctx->height, cd_ctx->pix_fmt, cd_ctx->width, cd_ctx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
     if (img_convert_ctx == NULL) {
         printf("sws_getContext error \n");
         return -1;
     }
    SDL_Thread *video_tid = SDL_CreateThread(sfp_refresh_thread,NULL,NULL);
    AVFrame *video_frame =av_frame_alloc();
    AVPacket *packet=(AVPacket *)av_malloc(sizeof(AVPacket));
    for (;;) {
         //Wait
        SDL_WaitEvent(&event);
        //Wait
        SDL_WaitEvent(&event);
        if (event.type == SDL_DISPLAYEVENT) {
            ret = av_read_frame(fmt_ctx, packet);
            if (ret >= 0) {
                if (packet->stream_index == video_index) {
                    ret = avcodec_send_packet(cd_ctx, packet);
                    av_packet_unref(packet);
                    if (ret < 0 ) {
                        printf("send packet error\n");
                        continue;
                    }
//                    ret = avcodec_receive_frame(cd_ctx, video_frame);
//                    if (ret < 0) {
//                        printf("receivt frame error \n");
//                        continue;
//                    }
                    
                    
                    ret = avcodec_receive_frame(cd_ctx, video_frame);
                    ret = SDL_UpdateTexture(texture, NULL, video_frame->data[0], video_frame->linesize[0]); // 这段代码调用出现崩溃。
                    if (ret < 0) {
                        printf("SDL_UpdateTexture failed \n");
                        continue;
                    }
                    SDL_RenderClear(renderer);
                    SDL_RenderCopy(renderer, texture, NULL, NULL);
                    SDL_RenderPresent(renderer);
                    SDL_Delay(40);
                    
                    
                    
                    
                    
                    
//                while (1) {
//
//                  }
                }
            }else{
                printf("read frame error \n");
            }
        }else if (event.type == SDL_WINDOWEVENT){
             SDL_GetWindowSize(win, &w_width, &w_height);
            printf("WINDOWEVENT \n");
        }else if (event.type == SDL_QUIT){
            thread_exit = 1;
            printf("QUIT \n");
            break;
        }else{
            printf("other event = %d",event.type);
        }
     }
    return 0;
}



#endif

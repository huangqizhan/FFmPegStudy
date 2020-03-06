//
//  sdl_player.c
//  FFM
//
//  Created by hjb_mac_mini on 2019/12/17.
//  Copyright © 2019 hqz. All rights reserved.
//

#include "sdl_player.h"

#define ENABLE_SDL 0


#if ENABLE_SDL


#define BLOCK_SIZE 4096000

//event message
#define REFRESH_EVENT  (SDL_USEREVENT + 1)
#define QUIT_EVENT  (SDL_USEREVENT + 2)

int thread_exit=0;

int refresh_video_timer(void *udata){

    thread_exit=0;

    while (!thread_exit) { //  每隔40毫秒发送一个REFRESH_EVENT事件，也就是一秒播放25帧。
        SDL_Event event;
        event.type = REFRESH_EVENT;
        SDL_PushEvent(&event);
        SDL_Delay(40);
    }

    thread_exit=0;

    // 线程结束时发送一个QUIT_EVENT事件
    SDL_Event event;
    event.type = QUIT_EVENT;
    SDL_PushEvent(&event);

    return 0;
}
int video_play(void){

    FILE *video_fd = NULL; // 播放文件的指针
 
    SDL_Event event; // 监听事件
    SDL_Rect rect; // 矩形框

    Uint32 pixformat = 0; // 像素格式

    SDL_Window *win = NULL; // 窗口
    SDL_Renderer *renderer = NULL; // 渲染器
    SDL_Texture *texture = NULL; // 纹理

    SDL_Thread *timer_thread = NULL; // 每帧切换时间管理线程

    int w_width = 448, w_height = 960; // 窗口宽高
    const int video_width = 448, video_height = 960; // 视频展示宽高(纹理宽高)

    Uint8 *video_pos = NULL; // video_buf的首地址
    Uint8 *video_end = NULL; // video_buf的尾地址

    unsigned int remain_len = 0;
    unsigned int video_buff_len = 0; // 一张YUV图片的长度
    unsigned int blank_space_len = 0;
    Uint8 video_buf[BLOCK_SIZE]; // 读取的一张yuv图片的指针

    const char *path = "test.yuv";

    // 一帧YUV数据的大小是 width * height *1.5(即12/8)，用 12/8是为了计算的更快
    const unsigned int yuv_frame_len = video_width * video_height * 12 / 8;

    // 初始化 SDL
    if(SDL_Init(SDL_INIT_VIDEO)) {
        printf("初始化SDL失败！");
        return -1;
    }

    // 创建窗口
    win = SDL_CreateWindow("YUV 播放器",
                           SDL_WINDOWPOS_UNDEFINED,
                           SDL_WINDOWPOS_UNDEFINED,
                           w_width, w_height,
                           SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    if(!win) {
        printf("创建窗口失败！");
        goto __FAIL;
    }

    // 创建渲染器
    renderer = SDL_CreateRenderer(win, -1, 0);

    //IYUV: Y + U + V  (3 planes)
    //YV12: Y + V + U  (3 planes)
    // 像素格式
    pixformat= SDL_PIXELFORMAT_IYUV;

    // 创建纹理
    texture = SDL_CreateTexture(renderer,
                                pixformat,
                                SDL_TEXTUREACCESS_STREAMING,
                                video_width,
                                video_height);

    // 打开yuv文件
    video_fd = fopen(path, "r");
    if( !video_fd ){
        fprintf(stderr, "Failed to open yuv file\n");
        goto __FAIL;
    }

    // 从yuv文件中读取一张图片
    if((video_buff_len = fread(video_buf, 1, BLOCK_SIZE, video_fd)) <= 0){
        fprintf(stderr, "Failed to read data from yuv file!\n");
        goto __FAIL;
    }

    //set video positon
    video_pos = video_buf;
    video_end = video_buf + video_buff_len;
    blank_space_len = BLOCK_SIZE - video_buff_len;

    // 创建一个线程管理渲染更新时间，refresh_video_timer是函数名
    timer_thread = SDL_CreateThread(refresh_video_timer,
                                    NULL,
                                    NULL);

    // 监听事件，每次收到REFRESH_EVENT事件就读取下一帧图片进行刷新
    do {
        //Wait
        SDL_WaitEvent(&event);
        if(event.type==REFRESH_EVENT){ // 收到刷新事件(需要展示一张图时就需要刷新)
            //not enought data to render
            if((video_pos + yuv_frame_len) > video_end){

                //have remain data, but there isn't space
                remain_len = video_end - video_pos;
                if(remain_len && !blank_space_len) {
                    //copy data to header of buffer
                    memcpy(video_buf, video_pos, remain_len);

                    blank_space_len = BLOCK_SIZE - remain_len;
                    video_pos = video_buf;
                    video_end = video_buf + remain_len;
                }

                //at the end of buffer, so rotate to header of buffer
                if(video_end == (video_buf + BLOCK_SIZE)){
                    video_pos = video_buf;
                    video_end = video_buf;
                    blank_space_len = BLOCK_SIZE;
                }

                //read data from yuv file to buffer
                if((video_buff_len = fread(video_end, 1, blank_space_len, video_fd)) <= 0){
                    fprintf(stderr, "eof, exit thread!");
                    thread_exit = 1;
                    continue;// to wait event for exiting
                }

                //reset video_end
                video_end += video_buff_len;
                blank_space_len -= video_buff_len;
                printf("not enought data: pos:%p, video_end:%p, blank_space_len:%d\n", video_pos, video_end, blank_space_len);
            }

            // 更新纹理
            SDL_UpdateTexture( texture, NULL, video_pos, video_width);

            //FIX: If window is resize
            // 矩形区域时视频显示区域
            rect.x = 0;
            rect.y = 0;
            rect.w = w_width;
            rect.h = w_height;

            SDL_RenderClear( renderer );
            SDL_RenderCopy( renderer, texture, NULL, &rect); // 将纹理拷贝到渲染器，rect是视频显示区域
            SDL_RenderPresent( renderer ); // 展示视频

            // printf("not enought data: pos:%p, video_end:%p, blank_space_len:%d\n", video_pos, video_end, blank_space_len);
            // 处理完后将video_pos指向下一帧
            video_pos += yuv_frame_len;

        }else if(event.type==SDL_WINDOWEVENT){
            // 窗口缩放
            SDL_GetWindowSize(win, &w_width, &w_height);
        }else if(event.type==SDL_QUIT){
            thread_exit=1;
        }else if(event.type==QUIT_EVENT){
            break;
        }
    }while ( 1 );

__FAIL:

    //close file
    if(video_fd){
        fclose(video_fd);
    }

    SDL_Quit();

    return 0;
}





#endif


//
//  rec_video.c
//  Media
//
//  Created by 黄麒展 on 2021/1/28.
//

#include "rec_video.h"
#include <unistd.h>
#include <libavdevice/avdevice.h>
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>




static int rec_status = 0;
void set_status(int status){
    rec_status = status;
}



#pragma mark ------ video
static void video_encode(AVCodecContext *codec_ctx,AVFrame *frame ,AVPacket *pkt,FILE *out_file){
    if (frame == NULL) {
        av_log(NULL, AV_LOG_DEBUG, "encode frame error \n");
        return;
    }
    int ret = 0;
    ret = avcodec_send_frame(codec_ctx, frame);
    if (ret < 0) {
        av_log(NULL, AV_LOG_DEBUG, "avcodec send frame error \n");
        return;
    }
    while (ret >= 0) {
        ret = avcodec_receive_packet(codec_ctx, pkt);
        /// 从新开始 已经结束
        if (ret == AVERROR(EAGAIN) || ret == AVERROR(AVERROR_EOF)) {
            return;
        }else if (ret < 0){
            av_log(NULL, AV_LOG_DEBUG, "encode error \n");
            exit(0);
        }
        fwrite(pkt->data, 1, pkt->size, out_file);
        fflush(out_file);
        av_packet_unref(pkt);
    }
}
/// 打开视频编码器
static void open_video(int width, int height , AVCodecContext **enc_ctx){
//    AVCodec *codec = NULL;
//    codec = avcodec_find_decoder_by_name("libx264");
//    printf("codec.type = %d",codec->type);

    AVCodec *codec = NULL;
    codec = avcodec_find_encoder_by_name("libx264");
    printf("codec.type = %d",codec->type);
    if (codec == NULL) {
        av_log(NULL, AV_LOG_DEBUG, "find codec error \n");
        return;
    }
    *enc_ctx = avcodec_alloc_context3(codec);
    if ((*enc_ctx) == NULL) {
        av_log(NULL, AV_LOG_DEBUG, "alloc avcodec error \n");
        return;
    }
#pragma mark ---- h264的 SPS PPS 参数
    //// h264 的编码格式
    (*enc_ctx)->profile = FF_PROFILE_H264_HIGH_444;
    /// 编码格式的级别
    (*enc_ctx)->level = 50;

    /// 设置码率
    (*enc_ctx)->bit_rate = 500000;     ///600kbps
//    (*enc_ctx)->rc_min_rate = 500000;
//    (*enc_ctx)->rc_max_rate = 500000;
//    (*enc_ctx)->rc_buffer_size= 500000;
    
    /// 分辨率
    (*enc_ctx)->width = 640; //不能改变分辩率大小
    (*enc_ctx)->height = 480;
    /// GOP
    /// gop帧数
    (*enc_ctx)->gop_size = 250;
    /// gop内的I帧数
    (*enc_ctx)->keyint_min = 25;   /// option

    ///参考帧数量
    (*enc_ctx)->max_b_frames = 3;   /// option
//    (*enc_ctx)->has_b_frames = 1;   /// option

    (*enc_ctx)->refs = 3;           /// 参考帧数量

    ///输入的YUV数据
    (*enc_ctx)->pix_fmt = AV_PIX_FMT_YUV420P;




    // 设置帧率
    (*enc_ctx)->time_base = (AVRational){1,25};  /// 每帧的时长
    (*enc_ctx)->framerate = (AVRational){25,1};  /// 帧率

//    (*enc_ctx)->flags|= AV_CODEC_FLAG_LOOP_FILTER;   // flags=+loop
//    (*enc_ctx)->me_cmp|= 1;                       // cmp=+chroma, where CHROMA = 1
//
//
//    (*enc_ctx)->me_subpel_quality = 7;   // subq=7
//    (*enc_ctx)->me_range = 16;   // me_range=16
//    (*enc_ctx)->i_quant_factor = 0.71; // i_qfactor=0.71
//    (*enc_ctx)->qcompress = 0; // qcomp=0.6
//    (*enc_ctx)->qmin = 51;   // qmin=10
//    (*enc_ctx)->qmax = 51;   // qmax=51
//    (*enc_ctx)->max_qdiff = 4;   // qdiff=4
//    (*enc_ctx)->trellis = 1; // trellis=1
    /// 打开编码器
    int ret = avcodec_open2(*enc_ctx, codec, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_DEBUG, "open codec faild \n");
    }
}

/// 创建frame
static AVFrame* create_frame(int width ,int height){
    AVFrame* frame = NULL;
    int ret = 0;
    frame = av_frame_alloc();
    if (frame == NULL) {
        av_log(NULL, AV_LOG_DEBUG, "alloc frame error \n");
        goto FAILD;
    }
    
    frame->width = width;
    frame->height = height;
    frame->format = AV_PIX_FMT_YUV420P;
    
    /// 创建buffer
    ret = av_frame_get_buffer(frame, 32);
    if (ret < 0) {
        av_log(NULL, AV_LOG_DEBUG, "alloc frame buffer error \n");
        goto FAILD;
    }
    return frame;
FAILD:
    if (frame) {
        av_frame_free(&frame);
    }
    return NULL;
}
void open_video_device(void){
    /// 注册设备
    avdevice_register_all();
    av_log_set_level(AV_LOG_DEBUG);
    /// 设备
    const char *device_name = "0";
    ///输入上下文
    AVFormatContext *infmt_ctx = NULL;
    /// 输入格式
    AVInputFormat *in_fmt = av_find_input_format("avfoundation");
    /// iotional
    AVDictionary *option = NULL;
    /// frame
    AVFrame *frame = NULL;
    AVPacket *new_packet = NULL;
    /// pts
    int basa = 0;
    //ret
    int ret = 0;
    // 输出文件
    const char *out = "/Users/huangqizhan/Library/Containers/com.huangqizhan.Media/Data/Documents/video2.yuv";
    const char *h264out = "/Users/huangqizhan/Library/Containers/com.huangqizhan.Media/Data/Documents/video2.h264";

    // packet 在栈上创建
    AVPacket pkt;
    /// file
    FILE *out_file;
    FILE *h264_file;
    /// 设置录制视频的参数 根据不同的参数采集数据的大小会有所不同
    /*
     分辨率: 640x480
     如果是yuv4:4:4 则每帧数据大小为 640x480x3
     如果是yuv4:2:2 则每帧数据大小为 640x480x2
     如果是yuv4:2:0 则每帧数据大小为 640x480x1.5
     */
    av_dict_set(&option, "video_size", "640x480", 0);
    av_dict_set(&option, "framerate", "30", 0);
    av_dict_set(&option, "pixel_format", "nv12", 0);
    /// 打开设备
    ret = avformat_open_input(&infmt_ctx, device_name, in_fmt, &option);
    if (ret < 0) {
        av_log(NULL, AV_LOG_DEBUG, "open input error \n");
        return;
    }
    if (infmt_ctx == NULL) {
        av_log(NULL, AV_LOG_DEBUG, "infmt_ctx create error \n");
        return;
    }
    out_file = fopen(out, "wb+");
    if (out_file == NULL) {
        av_log(NULL, AV_LOG_DEBUG, "open file error \n");
        return;
    }
    h264_file = fopen(h264out, "wb+");
    if (h264_file == NULL) {
        av_log(NULL, AV_LOG_DEBUG, "open h264file error");
    }
    set_status(1);
    
    AVCodecContext *ecode_ctx = NULL;
    open_video(640, 480, &ecode_ctx);
    if (ecode_ctx == NULL) {
        av_log(NULL, AV_LOG_DEBUG, "codec_ctx create error \n");
        return;
    }
    frame = create_frame(640, 480);
    
    /// 创建packet
    new_packet = av_packet_alloc();
    
    if (new_packet == NULL) {
        av_log(NULL, AV_LOG_DEBUG, "create packet error \n");
        return;
    }
    
    while (rec_status) {
        ret = av_read_frame(infmt_ctx, &pkt);
//        if (ret == -35) {
//            sleep(1);
//            continue;
//        }

//        size_t si = fwrite(pkt.data, 1, 614400, out_file);
        ///640x480x1.5
        /// NV12         YYYYYYYYUVUV     8个像素    采集数据的格式
        /// YUV420p   YYYYYYYYUUVV    8个像素    编码数据的格式
        /// 640x480 = 307200
        if (ret == 0) {
            
            /// nv12 -> yuv420p
            memcpy(frame->data[0], pkt.data, 307200);
            for (int i = 0; i < 307200/4; i++) {
                frame->data[1][i] = pkt.data[307200 + i*2];
                frame->data[2][i] = pkt.data[307200 + i*2 + 1];
            }
            fwrite(frame->data[0], 1, 307200, out_file);
            fwrite(frame->data[1], 1, 307200/4, out_file);
            fwrite(frame->data[2], 1, 307200/4, out_file);
            /// 刷新数据
            fflush(out_file);
            frame->pts = basa++;
            video_encode(ecode_ctx, frame, &pkt, h264_file);
            ///保存原始的采集数据
//                size_t si = fwrite(pkt.data, 1, 460800, out_file);
//                if (si > 0) {
//                    av_log(NULL, AV_LOG_DEBUG, "write size = %zu \n ",si);
//                }
//                fflush(out_file);
        }
    }
    /// 告诉编码器编码结束（可能会有丢帧）
    video_encode(ecode_ctx, NULL, &pkt, h264_file);
}


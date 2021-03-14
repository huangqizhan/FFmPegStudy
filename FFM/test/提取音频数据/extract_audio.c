//
//  extract_audio.c
//  Media
//
//  Created by 黄麒展 on 2021/1/30.
//

#include "extract_audio.h"

#include <stdio.h>
#include <libavutil/log.h>
#include <libavformat/avformat.h>


static int get_audio_obj_type(int aactype){
    //AAC HE V2 = AAC LC + SBR + PS
    //AAV HE = AAC LC + SBR
    //所以无论是 AAC_HEv2 还是 AAC_HE 都是 AAC_LC
    switch(aactype){
        case 0:
        case 2:
        case 3:
            return aactype+1;
        case 1:
        case 4:
        case 28:
            return 2;
        default:
            return 2;

    }
}

static int get_sample_rate_index(int freq, int aactype){
//    goto <#label#>;
//    av_log(NULL, AV_LOG_DEBUG, "%s",av_err2str(-1));
    int i = 0;
    int freq_arr[13] = {
        96000, 88200, 64000, 48000, 44100, 32000,
        24000, 22050, 16000, 12000, 11025, 8000, 7350
    };

    //如果是 AAC HEv2 或 AAC HE, 则频率减半
    if(aactype == 28 || aactype == 4){
        freq /= 2;
    }

    for(i=0; i< 13; i++){
        if(freq == freq_arr[i]){
            return i;
        }
    }
    return 4;//默认是44100
}

static int get_channel_config(int channels, int aactype){
    //如果是 AAC HEv2 通道数减半
    if(aactype == 28){
        return (channels / 2);
    }
    return channels;
}




static void adts_header(char *szAdtsHeader, int dataLen, int aactype, int frequency, int channels){

    int audio_object_type = get_audio_obj_type(aactype);
    int sampling_frequency_index = get_sample_rate_index(frequency, aactype);
    int channel_config = get_channel_config(channels, aactype);

    printf("aot=%d, freq_index=%d, channel=%d\n", audio_object_type, sampling_frequency_index, channel_config);

    int adtsLen = dataLen + 7;

    szAdtsHeader[0] = 0xff;         //syncword:0xfff                          高8bits
    szAdtsHeader[1] = 0xf0;         //syncword:0xfff                          低4bits
    szAdtsHeader[1] |= (0 << 3);    //MPEG Version:0 for MPEG-4,1 for MPEG-2  1bit
    szAdtsHeader[1] |= (0 << 1);    //Layer:0                                 2bits
    szAdtsHeader[1] |= 1;           //protection absent:1                     1bit

    szAdtsHeader[2] = (audio_object_type - 1)<<6;            //profile:audio_object_type - 1                      2bits
    szAdtsHeader[2] |= (sampling_frequency_index & 0x0f)<<2; //sampling frequency index:sampling_frequency_index  4bits
    szAdtsHeader[2] |= (0 << 1);                             //private bit:0                                      1bit
    szAdtsHeader[2] |= (channel_config & 0x04)>>2;           //channel configuration:channel_config               高1bit

    szAdtsHeader[3] = (channel_config & 0x03)<<6;     //channel configuration:channel_config      低2bits
    szAdtsHeader[3] |= (0 << 5);                      //original：0                               1bit
    szAdtsHeader[3] |= (0 << 4);                      //home：0                                   1bit
    szAdtsHeader[3] |= (0 << 3);                      //copyright id bit：0                       1bit
    szAdtsHeader[3] |= (0 << 2);                      //copyright id start：0                     1bit
    szAdtsHeader[3] |= ((adtsLen & 0x1800) >> 11);           //frame length：value   高2bits

    szAdtsHeader[4] = (uint8_t)((adtsLen & 0x7f8) >> 3);     //frame length:value    中间8bits
    szAdtsHeader[5] = (uint8_t)((adtsLen & 0x7) << 5);       //frame length:value    低3bits
    szAdtsHeader[5] |= 0x1f;                                 //buffer fullness:0x7ff 高5bits
    szAdtsHeader[6] = 0xfc;
}


/// 提取音频
int extract_audio(char * src_path, char *des_path){
    int ret = 0;
    
    char *res = NULL;

    char *dst = NULL;

    FILE *dst_file = NULL;
    
    AVPacket pkt;
    
    int audio_index;

//    if(argc < 3){
//
//        printf("parmar error \n");
//
//        return -1;
//    }
    
    res = src_path;

    dst = des_path;

    AVFormatContext *fmt_ctx = NULL;
  
    av_log_set_level(AV_LOG_INFO);
  
    ret = avformat_open_input(&fmt_ctx , res ,NULL,NULL);
   
    if(ret < 0){
   
        av_log(NULL, AV_LOG_ERROR,"open input error %s \n",av_err2str(ret));
  
        return -1;
     }
      /// 第四个三叔0 输入 1 输出
     av_dump_format(fmt_ctx, 0,"./mama1.mp4",0);
    
    dst_file = fopen(dst,"wb");

    if(dst_file == NULL){
        
        av_log(NULL,AV_LOG_ERROR,"open out file error \n");

         avformat_close_input(&fmt_ctx);
        return -1;
    }

    ret = av_find_best_stream(fmt_ctx,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0);

    if(ret < 0){

        av_log(NULL,AV_LOG_ERROR,"find best stream error  \n");

        avformat_close_input(&fmt_ctx);
     
        fclose(dst_file);

        return -1;
    }
    
    audio_index = ret;

    av_init_packet(&pkt);

    int aac_type = fmt_ctx->streams[audio_index]->codecpar->profile;
    int channels = fmt_ctx->streams[audio_index]->codecpar->channels;
    int sample_rate= fmt_ctx->streams[audio_index]->codecpar->sample_rate;

    while(av_read_frame(fmt_ctx,&pkt) >= 0){

        if(pkt.stream_index == audio_index){

            char adts_header_buf[7];

            adts_header(adts_header_buf, pkt.size, aac_type, sample_rate, channels);
           
           fwrite(adts_header_buf, 1, 7, dst_file);

            int len =  fwrite(pkt.data,1,pkt.size,dst_file);
        
            if(len != pkt.size){
                av_log(NULL,AV_LOG_WARNING,"write data size no equal pkt size \n");
            }
        }
        av_packet_unref(&pkt);
    }

     avformat_close_input(&fmt_ctx);
    
    fclose(dst_file);
    
      return 0;
 }
                                                                                                          




//
//  ffmpeg_config_info.c
//  FFM
//
//  Created by hjb_mac_mini on 2019/12/20.
//  Copyright © 2019 8km. All rights reserved.
//

#include "ffmpeg_config_info.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavdevice/avdevice.h>


char* ffm_config_info(void){
//    av_register_all();
     char info[10000] = { 0 };
     printf("%s\n", avcodec_configuration());
     sprintf(info, "%s\n", avcodec_configuration());
     char *data = info;
     return data;
}


char *ffm_protocol_info(void){
    
    char info[40000]={0};
//     av_register_all();
     
     struct URLProtocol *pup = NULL;
     //Input
     struct URLProtocol **p_temp = &pup;
     avio_enum_protocols((void **)p_temp, 0);
     while ((*p_temp) != NULL){
         sprintf(info, "%s[In ][%10s]\n", info, avio_enum_protocols((void **)p_temp, 0));
     }
     pup = NULL;
     //Output
     avio_enum_protocols((void **)p_temp, 1);
     while ((*p_temp) != NULL){
         sprintf(info, "%s[Out][%10s]\n", info, avio_enum_protocols((void **)p_temp, 1));
     }
    char *pro_data = info;
    return pro_data;
}

char *ffm_av_format_info(void){
    char info[40000] = { 0 };
      
//    av_register_all();
    AVInputFormat *if_temp = av_iformat_next(NULL);
    AVOutputFormat *of_temp = av_oformat_next(NULL);
    //Input
    while(if_temp!=NULL){
      sprintf(info, "%s[In ]%10s\n", info, if_temp->name);
      if_temp=if_temp->next;
    }
    //Output
    while (of_temp != NULL){
      sprintf(info, "%s[Out]%10s\n", info, of_temp->name);
      of_temp = of_temp->next;
    }
    char *for_data = info;
    return for_data;
}

char *ffm_av_codec_format(void){
    char info[40000] = { 0 };
    
    AVCodec *c_temp = av_codec_next(NULL);
    while(c_temp!=NULL){
        if (c_temp->decode!=NULL){
            sprintf(info, "%s[Dec]", info);
        }
        else{
            sprintf(info, "%s[Enc]", info);
        }
        switch (c_temp->type){
            case AVMEDIA_TYPE_VIDEO:
                sprintf(info, "%s[Video]", info);
                break;
            case AVMEDIA_TYPE_AUDIO:
                sprintf(info, "%s[Audio]", info);
                break;
            default:
                sprintf(info, "%s[Other]", info);
                break;
        }
        sprintf(info, "%s%10s\n", info, c_temp->name);
        
        
        c_temp=c_temp->next;
    }
    char *codec_fmt_data = info;
    return codec_fmt_data;
}

char *ffm_av_filter_info(void){
    char in_fo[40000] = { 0 };
//       avfilter_register_all();
       AVFilter *f_temp = (AVFilter *)avfilter_next(NULL);
       while (f_temp != NULL){
           f_temp = (AVFilter *)avfilter_next(f_temp);
           if (f_temp) {
               sprintf(in_fo, "%s[%10s]\n", in_fo, f_temp->name);
           }
       }
    char *filter_data = in_fo;
    return filter_data;
}


void ffmpeg_device(void){
    avdevice_register_all();
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    AVDictionary* options = NULL;
    av_dict_set(&options,"list_devices","true",0);
    AVInputFormat *iformat = av_find_input_format("avfoundation");
    printf("========Device Info=============\n");
    avformat_open_input(&pFormatCtx,"video=dummy",iformat,&options);
    printf("================================\n");
}

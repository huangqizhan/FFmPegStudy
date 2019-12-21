//
//  ffmpeg_config_info.h
//  FFM
//
//  Created by hjb_mac_mini on 2019/12/20.
//  Copyright © 2019 8km. All rights reserved.
//

#ifndef ffmpeg_config_info_h
#define ffmpeg_config_info_h

#include <stdio.h>
/// ffmpeg 配置信息
char* ffm_config_info(void);
/// ffmpeg 协议信息
char *ffm_protocol_info(void);
/// ffmpeg 音频视频格式
char *ffm_av_format_info(void);
/// ffmpeg 编解码格式
char *ffm_av_codec_format(void);
/// ffmpeg 滤镜信息
char *ffm_av_filter_info(void);
/// 设备信息
void ffmpeg_device(void);
#endif /* ffmpeg_config_info_h */

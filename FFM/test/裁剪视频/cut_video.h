//
//  cut_video.h
//  FFM
//
//  Created by 黄麒展 on 2019/12/27.
//  Copyright © 2019 hqz. All rights reserved.
//

#ifndef cut_video_h
#define cut_video_h

#include <stdio.h>


/**
 裁剪多媒体

 @param from_second 开始位置
 @param end_second 结束位置
 @param in_filename 源文件
 @param out_filename 目标文件
 @return int 
 */
int cut_video(double from_second,double end_second,const char *in_filename,const char *out_filename);



//int cut_video(double from_seconds, double end_seconds, const char* in_filename, const char* out_filename);
#endif /* cut_video_h */

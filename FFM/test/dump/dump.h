//
//  dump.h
//  FFM
//
//  Created by 黄麒展 on 2019/12/11.
//  Copyright © 2019 8km. All rights reserved.
//

#ifndef dump_h
#define dump_h

#include <stdio.h>

/// 获取多媒体文件信息
void dump_info(const char *path);

/// 获取多媒体文件的音频数据
int extrct_audio(const char *scr , const char *dest);


#endif /* dump_h */

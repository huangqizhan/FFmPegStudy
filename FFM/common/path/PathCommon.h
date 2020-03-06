//
//  PathCommon.h
//  FFM
//
//  Created by 黄麒展 on 2019/12/12.
//  Copyright © 2019 hqz. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>


@interface PathCommon : NSObject


//// 路径  library/cache
+ (NSString *)cacheDirectory;


////创建 library/cache 下 的子路径
+ (NSString *)creatSubDirectoryWith:(NSString *)subDirectory;

////// 删除 文件
+ (BOOL)removeFileAtFilePath:(NSString *)filePath;

/////文件路径
+ (NSString *)filePathWithFileName:(NSString *)fileKey
                           orgName:(NSString *)name
                              type:(NSString *)type;
/////文件主路径
+ (NSString *)fileMainPath;

/////返回字节数
+ (CGFloat)fileSizeAtFilePath:(NSString *)filePath;

/////清除所有的NSUserDefault
+ (void)clearUserDefault;

/////复制文件
+ (BOOL)copyFromPath:(NSString *)fromPath toPath:(NSString *)toPath;

+ (NSString *)homeDirectory;

+ (NSString *)documentDirectory;


+ (NSString *)tmpDirectory;


+ (NSURL *)createFolderWithName:(NSString *)folderName inDirectory:(NSString *)directory;

+ (NSString *)dataPath;

+ (void)removeFileAtPath:(NSString *)path;

/**
 *  返回文件大小，单位为字节
 */
+ (unsigned long long)getFileSize:(NSString *)path;

+ (void)SetUserDefault:(id)value forKey:(NSString *)key;
+ (id)GetUserDefaultWithKey:(NSString *)key;
+ (NSString *)medioPath:(NSString *)fileName;



@end


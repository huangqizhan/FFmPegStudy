//
//  ViewController.m
//  FFM
//
//  Created by hjb_mac_mini on 2019/9/20.
//  Copyright © 2019 hqz. All rights reserved.
//

#import "ViewController.h"
#import "dump.h"
#include "video_extra.h"



@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
//    NSString *path = [[NSBundle mainBundle] pathForResource:@"1" ofType:@".mp4"];
////    dump_info(path.UTF8String);
//    NSString *filep = [PathCommon medioPath:@"test.h264"];
//    NSLog(@"filep = %@",filep);
//    int ret = extrct_audio(path.UTF8String,filep.UTF8String);
//    NSLog(@"ret = %d",ret);
//    video_exteac(path.UTF8String,filep.UTF8String);
//    int e = 0x1f & 63;
//    printf("e = %d",e);
    
    UIImage *image = [UIImage imageNamed:@"IMG_0032.JPG"];
    [self yp_imageWithOriginalImage:image withScaleSize:CGSizeMake(image.size.width/3, image.size.height/3)];
    
}
- (UIImage *)yp_imageWithOriginalImage:(UIImage *)originalImage withScaleSize:(CGSize)size {
    UIGraphicsBeginImageContext(size);
    [originalImage drawInRect:CGRectMake(0, 0, size.width, size.height)];
    UIImage * image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return image;
}
- (void)t8{
<<<<<<< .merge_file_Y1ow4t
    UIImage *image = [UIImage imageNamed:@"IMG_0032.JPG"];
    [self yp_imageWithOriginalImage:image withScaleSize:CGSizeMake(image.size.width/3, image.size.height/3)];
=======
    NSLog(@"");
>>>>>>> .merge_file_KlOSTh
}

@end

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
//    self.player = [[SGPlayer alloc] init];
//    NSURL *i_see_fire = [NSURL URLWithString:@"rtmp://liveplay.haimianmba.com/live/9493_a0101r2f"];
//    _asset = [SGAsset assetWithURL:i_see_fire];
//    self.player.videoRenderer.view = self.view;
//    self.player.videoRenderer.displayMode = SGScalingModeResizeAspect;
//    _playItem = [[SGPlayerItem alloc] initWithAsset:_asset];
//    [self.player replaceWithPlayerItem:_playItem];
//    [self.player play];
//    NSString *path = [[NSBundle mainBundle] pathForResource:@"1" ofType:@".mp4"];
////    dump_info(path.UTF8String);
//    NSString *filep = [PathCommon medioPath:@"test.h264"];
//    NSLog(@"filep = %@",filep);
//    int ret = extrct_audio(path.UTF8String,filep.UTF8String);
//    NSLog(@"ret = %d",ret);
//    video_exteac(path.UTF8String,filep.UTF8String);
//    int e = 0x1f & 63;
//    printf("e = %d",e);
    

}
- (UIImage *)yp_imageWithOriginalImage:(UIImage *)originalImage withScaleSize:(CGSize)size {
    UIGraphicsBeginImageContext(size);
    [originalImage drawInRect:CGRectMake(0, 0, size.width, size.height)];
    UIImage * image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return image;
}


@end

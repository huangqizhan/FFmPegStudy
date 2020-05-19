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
#import "SGPlayer.h"

@interface ViewController ()
@property (nonatomic, strong) SGPlayer *player;
@property (nonatomic) SGPlayerItem *playItem;
@property (nonatomic) SGAsset *asset;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.player = [[SGPlayer alloc] init];
    NSURL *i_see_fire = [NSURL URLWithString:@"rtmp://liveplay.haimianmba.com/live/9493_a0101r2f"];
    _asset = [SGAsset assetWithURL:i_see_fire];
    self.player.videoRenderer.view = self.view;
    self.player.videoRenderer.displayMode = SGScalingModeResizeAspect;
    _playItem = [[SGPlayerItem alloc] initWithAsset:_asset];
    [self.player replaceWithPlayerItem:_playItem];
    [self.player play];
//    NSString *path = [[NSBundle mainBundle] pathForResource:@"1" ofType:@".mp4"];
////    dump_info(path.UTF8String);
//    NSString *filep = [PathCommon medioPath:@"test.h264"];
//    NSLog(@"filep = %@",filep);
//    int ret = extrct_audio(path.UTF8String,filep.UTF8String);
//    NSLog(@"ret = %d",ret);
//    video_exteac(path.UTF8String,filep.UTF8String);
//    int e = 0x1f & 63;
//    printf("e = %d",e);

    
<<<<<<< HEAD
//    UIImage *image = [UIImage imageNamed:@"IMG_0032.JPG"];
//    [self yp_imageWithOriginalImage:image withScaleSize:CGSizeMake(image.size.width/3, image.size.height/3)];
    
}
- (UIImage *)yp_imageWithOriginalImage:(UIImage *)originalImage withScaleSize:(CGSize)size {
    UIGraphicsBeginImageContext(size);
    [originalImage drawInRect:CGRectMake(0, 0, size.width, size.height)];
    UIImage * image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return image;
=======
>>>>>>> 60bb6a951954a00d11d7e6746a8e2aa9cc55ee4c
}


@end

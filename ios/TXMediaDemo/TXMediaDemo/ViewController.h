//
//  ViewController.h
//  TXMediaDemo
//
//  Created by taixin on 14-12-8.
//  Copyright (c) 2014年 taixin. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "../../TXMediaPlayer/TXMediaPlayer/TXMediaPlayer.h"

@class TXMediaControl;

@interface ViewController : UIViewController

@property(atomic, retain) id<TXMediaPlayback> player;

- (id)initView;

- (IBAction)onClickMediaControl:(id)sender;
- (IBAction)onClickOverlay:(id)sender;
- (IBAction)onClickBack:(id)sender;
- (IBAction)onClickPlay:(id)sender;
- (IBAction)onClickPause:(id)sender;

@property(nonatomic,strong) IBOutlet TXMediaControl *mediaControl;


@end

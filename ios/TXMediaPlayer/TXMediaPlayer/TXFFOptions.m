//
//  TXFFOptions.m
//  TXMediaPlayer
//
//  Created by ZhangRui on 13-10-17.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import "TXFFOptions.h"
#include "txplayer/ios/txplayer_ios.h"

@implementation TXFFOptions

+ (TXFFOptions *)optionsByDefault
{
    TXFFOptions *options = [[TXFFOptions alloc] init];

    options.skipLoopFilter  = TX_AVDISCARD_ALL;
    options.skipFrame       = TX_AVDISCARD_NONREF;

    options.frameBufferCount  = 3;
    options.maxFps            = 30;
    options.frameDrop         = 0;
    options.pauseInBackground = YES;

    options.timeout         = -1;

    return options;
}

- (void)applyTo:(txMediaPlayer *)mediaPlayer
{
    [self logOptions];

    [self setCodecOption:@"skip_loop_filter"
               withInt64:self.skipLoopFilter
                      to:mediaPlayer];
    [self setCodecOption:@"skip_frame"
               withInt64:self.skipFrame
                      to:mediaPlayer];

    txmp_set_picture_queue_capicity(mediaPlayer, _frameBufferCount);
    txmp_set_max_fps(mediaPlayer, _maxFps);
    txmp_set_framedrop(mediaPlayer, _frameDrop);

    if (self.timeout > 0) {
        [self setFormatOption:@"timeout"
                    withInt64:self.timeout
                           to:mediaPlayer];
    }
}

- (void)logOptions
{
    NSMutableString *echo = [[NSMutableString alloc] init];
    [echo appendString:@"========================================\n"];
    [echo appendString:@"= FFmpeg options:\n"];
    [echo appendFormat:@"= skip_loop_filter: %@\n",   [TXFFOptions getDiscardString:self.skipLoopFilter]];
    [echo appendFormat:@"= skipFrame:        %@\n",   [TXFFOptions getDiscardString:self.skipFrame]];
    [echo appendFormat:@"= frameBufferCount: %d\n",   self.frameBufferCount];
    [echo appendFormat:@"= maxFps:           %d\n",   self.maxFps];
    [echo appendFormat:@"= timeout:          %lld\n", self.timeout];
    [echo appendString:@"========================================\n"];
    NSLog(@"%@", echo);
}

+ (NSString *)getDiscardString:(TXAVDiscard)discard
{
    switch (discard) {
        case TX_AVDISCARD_NONE:
            return @"avdiscard none";
        case TX_AVDISCARD_DEFAULT:
            return @"avdiscard default";
        case TX_AVDISCARD_NONREF:
            return @"avdiscard nonref";
        case TX_AVDISCARD_BIDIR:
            return @"avdicard bidir;";
        case TX_AVDISCARD_NONKEY:
            return @"avdicard nonkey";
        case TX_AVDISCARD_ALL:
            return @"avdicard all;";
        default:
            return @"avdiscard unknown";
    }
}

- (void)setFormatOption:(NSString *)optionName
              withInt64:(int64_t)value
                     to:(txMediaPlayer *)mediaPlayer
{
    txmp_set_format_option(mediaPlayer,
                           [optionName UTF8String],
                           [[NSString stringWithFormat:@"%lld", value] UTF8String]);
}

- (void)setCodecOption:(NSString *)optionName
             withInt64:(int64_t)value
                    to:(txMediaPlayer *)mediaPlayer
{
    txmp_set_codec_option(mediaPlayer,
                           [optionName UTF8String],
                           [[NSString stringWithFormat:@"%lld", value] UTF8String]);
}

@end

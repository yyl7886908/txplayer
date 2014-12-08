//
//  TXFFOptions.h
//  TXMediaPlayer
//
//  Created by ZhangRui on 13-10-17.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef enum TXAVDiscard{
    /* We leave some space between them for extensions (drop some
     * keyframes for intra-only or drop just some bidir frames). */
    TX_AVDISCARD_NONE    =-16, ///< discard nothing
    TX_AVDISCARD_DEFAULT =  0, ///< discard useless packets like 0 size packets in avi
    TX_AVDISCARD_NONREF  =  8, ///< discard all non reference
    TX_AVDISCARD_BIDIR   = 16, ///< discard all bidirectional frames
    TX_AVDISCARD_NONKEY  = 32, ///< discard all frames except keyframes
    TX_AVDISCARD_ALL     = 48, ///< discard all
} TXAVDiscard;

typedef struct txMediaPlayer txMediaPlayer;

@interface TXFFOptions : NSObject

+(TXFFOptions *)optionsByDefault;

-(void)applyTo:(txMediaPlayer *)mediaPlayer;

@property(nonatomic) TXAVDiscard skipLoopFilter;
@property(nonatomic) TXAVDiscard skipFrame;

@property(nonatomic) int frameBufferCount;
@property(nonatomic) int maxFps;
@property(nonatomic) int frameDrop;
@property(nonatomic) BOOL pauseInBackground;

@property(nonatomic) int64_t timeout; ///< read/write timeout, -1 for infinite, in microseconds

@end

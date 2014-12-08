/*
 * TXAudioKit.m
 *
 * Copyright (c) 2013-2014 Zhang Rui <bbcallen@gmail.com>
 *
 * based on https://github.com/kolyvan/kxmovie
 *
 * This file is part of TXPlayer.
 *
 * TXPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * TXPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with TXPlayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#import "TXAudioKit.h"

@implementation TXAudioKit {
    __weak id<TXAudioSessionDelegate> _delegate;

    BOOL _audioSessionInitialized;
}

+ (TXAudioKit *)sharedInstance
{
    static TXAudioKit *sAudioKit = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sAudioKit = [[TXAudioKit alloc] init];
    });
    return sAudioKit;
}

- (void)setupAudioSession:(id<TXAudioSessionDelegate>) delegate
{
    _delegate = nil;
    if (delegate == nil) {
        return;
    }

    OSStatus status = noErr;
    if (!_audioSessionInitialized) {
        status = AudioSessionInitialize(NULL,
                                        kCFRunLoopCommonModes,
                                        TXAudioSessionInterruptionListener,
                                        NULL);
        if (status != noErr) {
            NSLog(@"TXAudioKit: AudioSessionInitialize failed (%d)", (int)status);
            return;
        }
        _audioSessionInitialized = YES;
    }

    /* Set audio session to mediaplayback */
    UInt32 sessionCategory = kAudioSessionCategory_MediaPlayback;
    status = AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(sessionCategory), &sessionCategory);
    if (status != noErr) {
        NSLog(@"TXAudioKit: AudioSessionSetProperty(kAudioSessionProperty_AudioCategory) failed (%d)", (int)status);
        return;
    }

    status = AudioSessionSetActive(true);
    if (status != noErr) {
        NSLog(@"TXAudioKit: AudioSessionSetActive(true) failed (%d)", (int)status);
        return;
    }

    _delegate = delegate;
    return ;
}

static void TXAudioSessionInterruptionListener(void *inClientData, UInt32 inInterruptionState)
{
    id<TXAudioSessionDelegate> delegate = [TXAudioKit sharedInstance]->_delegate;
    if (delegate == nil)
        return;

    switch (inInterruptionState) {
        case kAudioSessionBeginInterruption: {
            NSLog(@"kAudioSessionBeginInterruption\n");
            dispatch_async(dispatch_get_main_queue(), ^{
                AudioSessionSetActive(false);
                [delegate TXAudioBeginInterruption];
            });
            break;
        }
        case kAudioSessionEndInterruption: {
            NSLog(@"kAudioSessionEndInterruption\n");
            dispatch_async(dispatch_get_main_queue(), ^{
                AudioSessionSetActive(true);
                [delegate TXAudioEndInterruption];
            });
            break;
        }
    }
}

@end

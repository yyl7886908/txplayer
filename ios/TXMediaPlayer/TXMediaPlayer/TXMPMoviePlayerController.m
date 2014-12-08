/*
 * TXMPMoviePlayerController.m
 *
 * Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#import "TXMPMoviePlayerController.h"
#import "TXAudioKit.h"

@interface TXMPMoviePlayerController() <TXAudioSessionDelegate>
@end

@implementation TXMPMoviePlayerController

@dynamic view;
@dynamic currentPlaybackTime;
@dynamic duration;
@dynamic playableDuration;
@synthesize bufferingProgress = _bufferingProgress;

@dynamic isPreparedToPlay;
@dynamic playbackState;
@dynamic loadState;

@dynamic controlStyle;
@dynamic scalingMode;
@dynamic shouldAutoplay;
@synthesize numberOfBytesTransferred = _numberOfBytesTransferred;

- (id)initWithContentURL:(NSURL *)aUrl
{
    self = [super initWithContentURL:aUrl];
    if (self) {
        self.controlStyle = MPMovieControlStyleNone;
        self.scalingMode = MPMovieScalingModeAspectFit;
        self.shouldAutoplay = YES;

        [self TX_installMovieNotificationObservers];

        self.useApplicationAudioSession = YES;
        [[TXAudioKit sharedInstance] setupAudioSession:self];

        _bufferingProgress = -1;
    }
    return self;
}

- (id)initWithContentURLString:(NSString *)aUrl
{
    NSURL *url;
    if ([aUrl rangeOfString:@"/"].location == 0) {
        //本地
        url = [NSURL fileURLWithPath:aUrl];
    }
    else {
        url = [NSURL URLWithString:aUrl];
    }

    self = [self initWithContentURL:url];
    if (self) {

    }
    return self;
}

- (void)dealloc
{
    [self TX_removeMovieNotificationObservers];
}

- (BOOL)isPlaying
{
    switch (self.playbackState) {
    case MPMoviePlaybackStatePlaying:
        return YES;
    default:
        return NO;
    }
   
}

- (void)shutdown
{
    // do nothing
}

-(int64_t)numberOfBytesTransferred
{
    NSArray *events = self.accessLog.events;
    if (events.count>0) {
        MPMovieAccessLogEvent *currentEvent = [events objectAtIndex:events.count -1];
        return currentEvent.numberOfBytesTransferred;
    }
    return 0;
}

- (UIImage *)thumbnailImageAtCurrentTime
{
    return [super thumbnailImageAtTime:self.currentPlaybackTime timeOption:MPMovieTimeOptionExact];
}

#pragma mark Movie Notification Handlers

/* Register observers for the various movie object notifications. */
-(void)TX_installMovieNotificationObservers
{
	[[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(TX_dispatchMPMediaPlaybackIsPreparedToPlayDidChangeNotification:)
                                                 name:MPMediaPlaybackIsPreparedToPlayDidChangeNotification
                                               object:self];

	[[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(TX_dispatchMPMoviePlayerLoadStateDidChangeNotification:)
                                                 name:MPMoviePlayerLoadStateDidChangeNotification
                                               object:self];

	[[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(TX_dispatchMPMoviePlayerPlaybackDidFinishNotification:)
                                                 name:MPMoviePlayerPlaybackDidFinishNotification
                                               object:self];

	[[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(TX_dispatchMPMoviePlayerPlaybackStateDidChangeNotification:)
                                                 name:MPMoviePlayerPlaybackStateDidChangeNotification
                                               object:self];
}

- (void)TX_removeMovieNotificationObservers
{
    [[NSNotificationCenter defaultCenter]removeObserver:self name:MPMediaPlaybackIsPreparedToPlayDidChangeNotification object:self];

    [[NSNotificationCenter defaultCenter]removeObserver:self name:MPMoviePlayerLoadStateDidChangeNotification object:self];
    [[NSNotificationCenter defaultCenter]removeObserver:self name:MPMoviePlayerPlaybackDidFinishNotification object:self];
    [[NSNotificationCenter defaultCenter]removeObserver:self name:MPMoviePlayerPlaybackStateDidChangeNotification object:self];
}

- (void)TX_dispatchMPMediaPlaybackIsPreparedToPlayDidChangeNotification:(NSNotification*)notification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:TXMediaPlaybackIsPreparedToPlayDidChangeNotification object:notification.object userInfo:notification.userInfo];
}

- (void)TX_dispatchMPMoviePlayerLoadStateDidChangeNotification:(NSNotification*)notification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:TXMoviePlayerLoadStateDidChangeNotification object:notification.object userInfo:notification.userInfo];
}

- (void)TX_dispatchMPMoviePlayerPlaybackDidFinishNotification:(NSNotification*)notification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:TXMoviePlayerPlaybackDidFinishNotification object:notification.object userInfo:notification.userInfo];
}

- (void)TX_dispatchMPMoviePlayerPlaybackStateDidChangeNotification:(NSNotification*)notification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:TXMoviePlayerPlaybackStateDidChangeNotification object:notification.object userInfo:notification.userInfo];
}

#pragma mark TXAudioSessionDelegate

- (void)TXAudioBeginInterruption
{
    [self pause];
}

- (void)TXAudioEndInterruption
{
    [self pause];
}

@end

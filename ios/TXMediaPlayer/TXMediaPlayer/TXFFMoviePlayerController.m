#import "TXFFMoviePlayerController.h"
#import "TXFFMoviePlayerDef.h"
#import "TXMediaPlayback.h"
#import "TXMediaModule.h"
#import "TXFFMrl.h"
#import "TXAudioKit.h"

#include "string.h"

@interface TXFFMoviePlayerController() <TXAudioSessionDelegate>
@end

@implementation TXFFMoviePlayerController {
    TXFFMrl *_ffMrl;
    id<TXMediaSegmentResolver> _segmentResolver;

    txMediaPlayer *_mediaPlayer;
    TXFFMoviePlayerMessagePool *_msgPool;

    NSInteger _videoWidth;
    NSInteger _videoHeight;
    NSInteger _sampleAspectRatioNumerator;
    NSInteger _sampleAspectRatioDenominator;

    BOOL      _seeking;
    NSInteger _bufferingTime;
    NSInteger _bufferingPosition;

    BOOL _keepScreenOnWhilePlaying;
    BOOL _pauseInBackground;

    NSMutableArray *_registeredNotifications;
}

@synthesize view = _view;
@synthesize currentPlaybackTime;
@synthesize duration;
@synthesize playableDuration;
@synthesize bufferingProgress = _bufferingProgress;

@synthesize numberOfBytesTransferred = _numberOfBytesTransferred;

@synthesize isPreparedToPlay = _isPreparedToPlay;
@synthesize playbackState = _playbackState;
@synthesize loadState = _loadState;

@synthesize controlStyle = _controlStyle;
@synthesize scalingMode = _scalingMode;
@synthesize shouldAutoplay = _shouldAutoplay;

#define FFP_IO_STAT_STEP (50 * 1024)

// as an example
void TXFFIOStatDebugCallback(const char *url, int type, int bytes)
{
    static int64_t s_ff_io_stat_check_points = 0;
    static int64_t s_ff_io_stat_bytes = 0;
    if (!url)
        return;

    //if (type != TXMP_IO_STAT_READ)
    if (type != 0)
        return;

    if (!av_strstart(url, "http:", NULL))
        return;

    s_ff_io_stat_bytes += bytes;
    if (s_ff_io_stat_bytes < s_ff_io_stat_check_points ||
        s_ff_io_stat_bytes > s_ff_io_stat_check_points + FFP_IO_STAT_STEP) {
        s_ff_io_stat_check_points = s_ff_io_stat_bytes;
        NSLog(@"io-stat: %s, +%d = %"PRId64"\n", url, bytes, s_ff_io_stat_bytes);
    }
}

void TXFFIOStatRegister(void (*cb)(const char *url, int type, int bytes))
{
    //txmp_io_stat_register(cb);
}

void TXFFIOStatCompleteDebugCallback(const char *url,
                                      int64_t read_bytes, int64_t total_size,
                                      int64_t elpased_time, int64_t total_duration)
{
    if (!url)
        return;

    if (!av_strstart(url, "http:", NULL))
        return;

    NSLog(@"io-stat-complete: %s, %"PRId64"/%"PRId64", %"PRId64"/%"PRId64"\n",
          url, read_bytes, total_size, elpased_time, total_duration);
}

void TXFFIOStatCompleteRegister(void (*cb)(const char *url,
                                            int64_t read_bytes, int64_t total_size,
                                            int64_t elpased_time, int64_t total_duration))
{
    //txmp_io_stat_complete_register(cb);
}

- (id)initWithContentURL:(NSURL *)aUrl withOptions:(TXFFOptions *)options
{
    return [self initWithContentURL:aUrl
                        withOptions:options
                withSegmentResolver:nil];
}

- (id)initWithContentURL:(NSURL *)aUrl
             withOptions:(TXFFOptions *)options
     withSegmentResolver:(id<TXMediaSegmentResolver>)segmentResolver
{
    if (aUrl == nil)
        return nil;

    return [self initWithContentURLString:[aUrl absoluteString]
                              withOptions:options
                      withSegmentResolver:segmentResolver];
}

- (id)initWithContentURLString:(NSString *)aUrlString
                   withOptions:(TXFFOptions *)options
           withSegmentResolver:(id<TXMediaSegmentResolver>)segmentResolver
{
    if (aUrlString == nil)
        return nil;

    self = [super init];
    if (self) {
        txmp_global_init();

        // TXFFIOStatRegister(TXFFIOStatDebugCallback);
        // TXFFIOStatCompleteRegister(TXFFIOStatCompleteDebugCallback);

        // init fields
        _controlStyle = MPMovieControlStyleNone;
        _scalingMode = MPMovieScalingModeAspectFit;
        _shouldAutoplay = NO;

        // init media resource
        _ffMrl = [[TXFFMrl alloc] initWithMrl:aUrlString];
        _segmentResolver = segmentResolver;

        // init player
        _mediaPlayer = txmp_ios_create(media_player_msg_loop);
        _msgPool = [[TXFFMoviePlayerMessagePool alloc] init];

        txmp_set_weak_thiz(_mediaPlayer, (__bridge_retained void *) self);
        txmp_set_format_callback(_mediaPlayer, format_control_message, (__bridge void *) self);

        // init video sink
//        int chroma = SDL_FCC_RV24;
        int chroma = SDL_FCC_I420;
        TXSDLGLView *glView = [[TXSDLGLView alloc] initWithFrame:[[UIScreen mainScreen] bounds]
                                                        withChroma:chroma];
        self->_view = glView;

        txmp_ios_set_glview(_mediaPlayer, glView);
        txmp_set_overlay_format(_mediaPlayer, chroma);

        // init audio sink
        [[TXAudioKit sharedInstance] setupAudioSession:self];

        // apply ffmpeg options
        [options applyTo:_mediaPlayer];
        _pauseInBackground = options.pauseInBackground;

        // init extra
        _keepScreenOnWhilePlaying = YES;
        [self setScreenOn:YES];

        _registeredNotifications = [[NSMutableArray alloc] init];
        [self registerApplicationObservers];
    }
    return self;
}

- (void)setScreenOn: (BOOL)on
{
    [TXMediaModule sharedModule].mediaModuleIdleTimerDisabled = on;
    // [UIApplication sharedApplication].idleTimerDisabled = on;
}

- (void)dealloc
{
    [_ffMrl removeTempFiles];
    [self unregisterApplicationObservers];
}

- (void)prepareToPlay
{
    if (!_mediaPlayer)
        return;

    [self setScreenOn:_keepScreenOnWhilePlaying];

    txmp_set_data_source(_mediaPlayer, [_ffMrl.resolvedMrl UTF8String]);
    txmp_set_format_option(_mediaPlayer, "safe", "0"); // for concat demuxer
    txmp_prepare_async(_mediaPlayer);
}

- (void)play
{
    if (!_mediaPlayer)
        return;

    [self setScreenOn:_keepScreenOnWhilePlaying];

    txmp_start(_mediaPlayer);
}

- (void)pause
{
    if (!_mediaPlayer)
        return;

    txmp_pause(_mediaPlayer);
}

- (void)stop
{
    if (!_mediaPlayer)
        return;

    [self setScreenOn:NO];

    txmp_stop(_mediaPlayer);
}

- (BOOL)isPlaying
{
    if (!_mediaPlayer)
        return NO;

    return txmp_is_playing(_mediaPlayer);
}

- (void)setPauseInBackground:(BOOL)pause
{
    _pauseInBackground = pause;
}

+ (void)setLogReport:(BOOL)preferLogReport
{
    //txmp_global_set_log_report(preferLogReport ? 1 : 0);
}

- (void)shutdown
{
    if (!_mediaPlayer)
        return;

    [self setScreenOn:NO];

    [self performSelectorInBackground:@selector(shutdownWaitStop:) withObject:self];
}

- (void)shutdownWaitStop:(TXFFMoviePlayerController *) mySelf
{
    if (!_mediaPlayer)
        return;

    txmp_stop(_mediaPlayer);
    [self performSelectorOnMainThread:@selector(shutdownClose:) withObject:self waitUntilDone:YES];
}

- (void)shutdownClose:(TXFFMoviePlayerController *) mySelf
{
    if (!_mediaPlayer)
        return;

    txmp_shutdown(_mediaPlayer);
    txmp_dec_ref_p(&_mediaPlayer);
}

- (MPMoviePlaybackState)playbackState
{
    if (!_mediaPlayer)
        return NO;

    MPMoviePlaybackState mpState = MPMoviePlaybackStateStopped;
    int state = txmp_get_state(_mediaPlayer);
    switch (state) {
        case MP_STATE_STOPPED:
        case MP_STATE_COMPLETED:
        case MP_STATE_ERROR:
        case MP_STATE_END:
            mpState = MPMoviePlaybackStateStopped;
            break;
        case MP_STATE_IDLE:
        case MP_STATE_INITIALIZED:
        case MP_STATE_ASYNC_PREPARING:
        case MP_STATE_PAUSED:
            mpState = MPMoviePlaybackStatePaused;
            break;
        case MP_STATE_PREPARED:
        case MP_STATE_STARTED: {
            if (_seeking)
                mpState = MPMoviePlaybackStateSeekingForward;
            else
                mpState = MPMoviePlaybackStatePlaying;
            break;
        }
    }
    // MPMoviePlaybackStatePlaying,
    // MPMoviePlaybackStatePaused,
    // MPMoviePlaybackStateStopped,
    // MPMoviePlaybackStateInterrupted,
    // MPMoviePlaybackStateSeekingForward,
    // MPMoviePlaybackStateSeekingBackward
    return mpState;
}

- (void)setCurrentPlaybackTime:(NSTimeInterval)aCurrentPlaybackTime
{
    if (!_mediaPlayer)
        return;

    _seeking = YES;
    [[NSNotificationCenter defaultCenter]
     postNotificationName:TXMoviePlayerPlaybackStateDidChangeNotification
     object:self];

    txmp_seek_to(_mediaPlayer, aCurrentPlaybackTime * 1000);
}

- (NSTimeInterval)currentPlaybackTime
{
    if (!_mediaPlayer)
        return 0.0f;

    NSTimeInterval ret = txmp_get_current_position(_mediaPlayer);
    return ret / 1000;
}

- (NSTimeInterval)duration
{
    if (!_mediaPlayer)
        return 0.0f;

    NSTimeInterval ret = txmp_get_duration(_mediaPlayer);
    return ret / 1000;
}

- (NSTimeInterval)playableDuration
{
    //if (!_mediaPlayer)
     //   return 0.0f;

   // NSTimeInterval ret = txmp_get_playable_duration(_mediaPlayer);
    //return ret / 1000;
    return 0;
}

- (void)setScalingMode: (MPMovieScalingMode) aScalingMode
{
    MPMovieScalingMode newScalingMode = aScalingMode;
    switch (aScalingMode) {
        case MPMovieScalingModeNone:
            [_view setContentMode:UIViewContentModeCenter];
            break;
        case MPMovieScalingModeAspectFit:
            [_view setContentMode:UIViewContentModeScaleAspectFit];
            break;
        case MPMovieScalingModeAspectFill:
            [_view setContentMode:UIViewContentModeScaleAspectFill];
            break;
        case MPMovieScalingModeFill:
            [_view setContentMode:UIViewContentModeScaleToFill];
            break;
        default:
            newScalingMode = _scalingMode;
    }

    _scalingMode = newScalingMode;
}

// deprecated, for MPMoviePlayerController compatiable
- (UIImage *)thumbnailImageAtTime:(NSTimeInterval)playbackTime timeOption:(MPMovieTimeOption)option
{
    return nil;
}

- (UIImage *)thumbnailImageAtCurrentTime
{
    if ([_view isKindOfClass:[TXSDLGLView class]]) {
        TXSDLGLView *glView = (TXSDLGLView *)_view;
        return [glView snapshot];
    }

    return nil;
}

- (void)postEvent: (TXFFMoviePlayerMessage *)msg
{
    if (!msg)
        return;

    AVMessage *avmsg = &msg->_msg;
    switch (avmsg->what) {
        case FFP_MSG_FLUSH:
            break;
        case FFP_MSG_ERROR: {
            NSLog(@"FFP_MSG_ERROR: %d", avmsg->arg1);

            [self setScreenOn:NO];

            [[NSNotificationCenter defaultCenter]
             postNotificationName:TXMoviePlayerPlaybackStateDidChangeNotification
             object:self];

            [[NSNotificationCenter defaultCenter]
                postNotificationName:TXMoviePlayerPlaybackDidFinishNotification
                object:self
                userInfo:@{
                    MPMoviePlayerPlaybackDidFinishReasonUserInfoKey: @(MPMovieFinishReasonPlaybackError),
                    @"error": @(avmsg->arg1)}];
            break;
        }
        case FFP_MSG_PREPARED:
            NSLog(@"FFP_MSG_PREPARED:");

            _isPreparedToPlay = YES;

            [[NSNotificationCenter defaultCenter] postNotificationName:TXMediaPlaybackIsPreparedToPlayDidChangeNotification object:self];

            _loadState = MPMovieLoadStatePlayable | MPMovieLoadStatePlaythroughOK;

            [[NSNotificationCenter defaultCenter]
             postNotificationName:TXMoviePlayerLoadStateDidChangeNotification
             object:self];

            break;
        case FFP_MSG_COMPLETED: {

            [self setScreenOn:NO];

            [[NSNotificationCenter defaultCenter]
             postNotificationName:TXMoviePlayerPlaybackStateDidChangeNotification
             object:self];

            [[NSNotificationCenter defaultCenter]
             postNotificationName:TXMoviePlayerPlaybackDidFinishNotification
             object:self
             userInfo:@{MPMoviePlayerPlaybackDidFinishReasonUserInfoKey: @(MPMovieFinishReasonPlaybackEnded)}];
            break;
        }
        case FFP_MSG_VIDEO_SIZE_CHANGED:
            NSLog(@"FFP_MSG_VIDEO_SIZE_CHANGED: %d, %d", avmsg->arg1, avmsg->arg2);
            if (avmsg->arg1 > 0)
                _videoWidth = avmsg->arg1;
            if (avmsg->arg2 > 0)
                _videoHeight = avmsg->arg2;
            // TODO: notify size changed
            break;
        case FFP_MSG_SAR_CHANGED:
            NSLog(@"FFP_MSG_SAR_CHANGED: %d, %d", avmsg->arg1, avmsg->arg2);
            if (avmsg->arg1 > 0)
                _sampleAspectRatioNumerator = avmsg->arg1;
            if (avmsg->arg2 > 0)
                _sampleAspectRatioDenominator = avmsg->arg2;
            break;
        case FFP_MSG_BUFFERING_START: {
            NSLog(@"FFP_MSG_BUFFERING_START:");

            _loadState = MPMovieLoadStateStalled;

            [[NSNotificationCenter defaultCenter]
             postNotificationName:TXMoviePlayerLoadStateDidChangeNotification
             object:self];
            break;
        }
        case FFP_MSG_BUFFERING_END: {
            NSLog(@"FFP_MSG_BUFFERING_END:");

            _loadState = MPMovieLoadStatePlayable | MPMovieLoadStatePlaythroughOK;

            [[NSNotificationCenter defaultCenter]
             postNotificationName:TXMoviePlayerLoadStateDidChangeNotification
             object:self];
            [[NSNotificationCenter defaultCenter]
             postNotificationName:TXMoviePlayerPlaybackStateDidChangeNotification
             object:self];
            break;
        }
        case FFP_MSG_BUFFERING_UPDATE:
            _bufferingPosition = avmsg->arg1;
            _bufferingProgress = avmsg->arg2;
            // NSLog(@"FFP_MSG_BUFFERING_UPDATE: %d, %%%d", _bufferingPosition, _bufferingProgress);
            break;
        case FFP_MSG_BUFFERING_BYTES_UPDATE:
            // NSLog(@"FFP_MSG_BUFFERING_BYTES_UPDATE: %d", avmsg->arg1);
            break;
        case FFP_MSG_BUFFERING_TIME_UPDATE:
            _bufferingTime       = avmsg->arg1;
            // NSLog(@"FFP_MSG_BUFFERING_TIME_UPDATE: %d", avmsg->arg1);
            break;
        case FFP_MSG_PLAYBACK_STATE_CHANGED:
            [[NSNotificationCenter defaultCenter]
             postNotificationName:TXMoviePlayerPlaybackStateDidChangeNotification
             object:self];
            break;
        case FFP_MSG_SEEK_COMPLETE: {
            NSLog(@"FFP_MSG_SEEK_COMPLETE:");
            _seeking = NO;
            break;
        }
        default:
            // NSLog(@"unknown FFP_MSG_xxx(%d)", avmsg->what);
            break;
    }

    [_msgPool recycle:msg];
}

- (TXFFMoviePlayerMessage *) obtainMessage {
    return [_msgPool obtain];
}

inline static TXFFMoviePlayerController *ffplayerRetain(void *arg) {
    return (__bridge_transfer TXFFMoviePlayerController *) arg;
}

int media_player_msg_loop(void* arg)
{
    @autoreleasepool {
        txMediaPlayer *mp = (txMediaPlayer*)arg;
        __weak TXFFMoviePlayerController *ffpController = ffplayerRetain(txmp_set_weak_thiz(mp, NULL));

        while (ffpController) {
            @autoreleasepool {
                TXFFMoviePlayerMessage *msg = [ffpController obtainMessage];
                if (!msg)
                    break;

                int retval = txmp_get_msg(mp, &msg->_msg, 1);
                if (retval < 0)
                    break;

                // block-get should never return 0
                assert(retval > 0);

                [ffpController performSelectorOnMainThread:@selector(postEvent:) withObject:msg waitUntilDone:NO];
            }
        }

        // retained in prepare_async, before SDL_CreateThreadEx
        txmp_dec_ref_p(&mp);
        return 0;
    }
}

#pragma mark av_format_control_message

int onControlResolveSegment(TXFFMoviePlayerController *mpc, int type, void *data, size_t data_size)
{
    if (mpc == nil)
        return -1;

    IJKFormatSegmentContext *fsc = data;
    if (fsc == NULL || sizeof(IJKFormatSegmentContext) != data_size) {
        NSLog(@"TXAVF_CM_RESOLVE_SEGMENT: invalid call\n");
        return -1;
    }

    NSString *url = [mpc->_segmentResolver urlOfSegment:fsc->position];
    if (url == nil)
        return -1;

    const char *rawUrl = [url UTF8String];
    if (url == NULL)
        return -1;

    fsc->url = strdup(rawUrl);
    if (fsc->url == NULL)
        return -1;

    fsc->url_free = free;
    return 0;
}

// NOTE: support to be called from read_thread
int format_control_message(void *opaque, int type, void *data, size_t data_size)
{
    TXFFMoviePlayerController *mpc = (__bridge TXFFMoviePlayerController*)opaque;

    switch (type) {
        case IJKAVF_CM_RESOLVE_SEGMENT:
            return onControlResolveSegment(mpc, type, data, data_size);
        default: {
            return -1;
        }
    }
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

#pragma mark app state changed

- (void)registerApplicationObservers
{

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillEnterForeground)
                                                 name:UIApplicationWillEnterForegroundNotification
                                               object:nil];
    [_registeredNotifications addObject:UIApplicationWillEnterForegroundNotification];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationDidBecomeActive)
                                                 name:UIApplicationDidBecomeActiveNotification
                                               object:nil];
    [_registeredNotifications addObject:UIApplicationDidBecomeActiveNotification];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillResignActive)
                                                 name:UIApplicationWillResignActiveNotification
                                               object:nil];
    [_registeredNotifications addObject:UIApplicationWillResignActiveNotification];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationDidEnterBackground)
                                                 name:UIApplicationDidEnterBackgroundNotification
                                               object:nil];
    [_registeredNotifications addObject:UIApplicationDidEnterBackgroundNotification];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillTerminate)
                                                 name:UIApplicationWillTerminateNotification
                                               object:nil];
    [_registeredNotifications addObject:UIApplicationWillTerminateNotification];
}

- (void)unregisterApplicationObservers
{
    for (NSString *name in _registeredNotifications) {
        [[NSNotificationCenter defaultCenter] removeObserver:self
                                                        name:name
                                                      object:nil];
    }
}

- (void)applicationWillEnterForeground
{
    NSLog(@"TXFFMoviePlayerController:applicationWillEnterForeground: %d", (int)[UIApplication sharedApplication].applicationState);
}

- (void)applicationDidBecomeActive
{
    NSLog(@"TXFFMoviePlayerController:applicationDidBecomeActive: %d", (int)[UIApplication sharedApplication].applicationState);
}

- (void)applicationWillResignActive
{
    NSLog(@"TXFFMoviePlayerController:applicationWillResignActive: %d", (int)[UIApplication sharedApplication].applicationState);
    dispatch_async(dispatch_get_main_queue(), ^{
        if (_pauseInBackground) {
            [self pause];
        }
    });
}

- (void)applicationDidEnterBackground
{
    NSLog(@"TXFFMoviePlayerController:applicationDidEnterBackground: %d", (int)[UIApplication sharedApplication].applicationState);
    dispatch_async(dispatch_get_main_queue(), ^{
        if (_pauseInBackground) {
            [self pause];
        }
    });
}

- (void)applicationWillTerminate
{
    NSLog(@"TXFFMoviePlayerController:applicationWillTerminate: %d", (int)[UIApplication sharedApplication].applicationState);
    dispatch_async(dispatch_get_main_queue(), ^{
        if (_pauseInBackground) {
            [self pause];
        }
    });
}

@end


#import "TXMediaPlayback.h"
#import "TXFFOptions.h"

@interface TXFFMoviePlayerController : NSObject <TXMediaPlayback>

- (id)initWithContentURL:(NSURL *)aUrl
             withOptions:(TXFFOptions *)options;

- (id)initWithContentURL:(NSURL *)aUrl
             withOptions:(TXFFOptions *)options
     withSegmentResolver:(id<TXMediaSegmentResolver>)segmentResolver;

- (id)initWithContentURLString:(NSString *)aUrlString
                   withOptions:(TXFFOptions *)options
           withSegmentResolver:(id<TXMediaSegmentResolver>)segmentResolver;

- (void)prepareToPlay;
- (void)play;
- (void)pause;
- (void)stop;
- (BOOL)isPlaying;

- (void)setPauseInBackground:(BOOL)pause;

+ (void)setLogReport:(BOOL)preferLogReport;

@end

#define TX_FF_IO_TYPE_READ (1)
void TXFFIOStatDebugCallback(const char *url, int type, int bytes);
void TXFFIOStatRegister(void (*cb)(const char *url, int type, int bytes));

void TXFFIOStatCompleteDebugCallback(const char *url,
                                      int64_t read_bytes, int64_t total_size,
                                      int64_t elpased_time, int64_t total_duration);
void TXFFIOStatCompleteRegister(void (*cb)(const char *url,
                                            int64_t read_bytes, int64_t total_size,
                                            int64_t elpased_time, int64_t total_duration));

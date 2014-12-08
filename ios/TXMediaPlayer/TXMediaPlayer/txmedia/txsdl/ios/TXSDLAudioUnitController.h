#import <Foundation/Foundation.h>

#include "txsdl/txsdl_aout.h"

@interface TXSDLAudioUnitController : NSObject

- (id)initWithAudioSpec:(SDL_AudioSpec *)aSpec;

- (void)play;
- (void)pause;
- (void)flush;
- (void)stop;
- (void)close;

@property (nonatomic, readonly) SDL_AudioSpec spec;

@end

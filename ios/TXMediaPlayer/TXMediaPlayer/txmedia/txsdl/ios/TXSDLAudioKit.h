

#import <Foundation/Foundation.h>
#include "txsdl/txsdl_aout.h"

extern void TXSDLGetAudioComponentDescriptionFromSpec(const SDL_AudioSpec *spec, AudioComponentDescription *desc);
extern void TXSDLGetAudioStreamBasicDescriptionFromSpec(const SDL_AudioSpec *spec, AudioStreamBasicDescription *desc);

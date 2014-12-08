#import "TXSDLAudioUnitController.h"
#include "txutil/txutil.h"
#import "TXSDLAudioKit.h"

@implementation TXSDLAudioUnitController {
    AudioUnit _auUnit;
    BOOL _isPaused;
}

- (id)initWithAudioSpec:(SDL_AudioSpec *)aSpec
{
    self = [super init];
    if (self) {
        if (aSpec == NULL) {
            self = nil;
            return nil;
        }
        _spec = *aSpec;

        AudioComponentDescription desc;
        TXSDLGetAudioComponentDescriptionFromSpec(&_spec, &desc);

        AudioComponent auComponent = AudioComponentFindNext(NULL, &desc);
        if (auComponent == NULL) {
            ALOGE("AudioUnit: AudioComponentFindNext failed");
            self = nil;
            return nil;
        }

        AudioUnit auUnit;
        OSStatus status = AudioComponentInstanceNew(auComponent, &auUnit);
        if (status != noErr) {
            ALOGE("AudioUnit: AudioComponentInstanceNew failed");
            self = nil;
            return nil;
        }

        UInt32 flag = 1;
        status = AudioUnitSetProperty(auUnit,
                                      kAudioOutputUnitProperty_EnableIO,
                                      kAudioUnitScope_Output,
                                      0,
                                      &flag,
                                      sizeof(flag));
        if (status != noErr) {
            ALOGE("AudioUnit: failed to set IO mode (%d)", (int)status);
        }

        /* Get the current format */
        _spec.format = AUDIO_S16SYS;
        _spec.channels = 2;
        AudioStreamBasicDescription streamDescription;
        TXSDLGetAudioStreamBasicDescriptionFromSpec(&_spec, &streamDescription);

        /* Set the desired format */
        UInt32 i_param_size = sizeof(streamDescription);
        status = AudioUnitSetProperty(auUnit,
                                      kAudioUnitProperty_StreamFormat,
                                      kAudioUnitScope_Input,
                                      0,
                                      &streamDescription,
                                      i_param_size);
        if (status != noErr) {
            ALOGE("AudioUnit: failed to set stream format (%d)", (int)status);
            self = nil;
            return nil;
        }

        /* Retrieve actual format */
        status = AudioUnitGetProperty(auUnit,
                                      kAudioUnitProperty_StreamFormat,
                                      kAudioUnitScope_Input,
                                      0,
                                      &streamDescription,
                                      &i_param_size);
        if (status != noErr) {
            ALOGE("AudioUnit: failed to verify stream format (%d)", (int)status);
        }

        AURenderCallbackStruct callback;
        callback.inputProc = (AURenderCallback) RenderCallback;
        callback.inputProcRefCon = (__bridge void*) self;
        status = AudioUnitSetProperty(auUnit,
                                      kAudioUnitProperty_SetRenderCallback,
                                      kAudioUnitScope_Input,
                                      0, &callback, sizeof(callback));
        if (status != noErr) {
            ALOGE("AudioUnit: render callback setup failed (%d)", (int)status);
            self = nil;
            return nil;
        }

        SDL_CalculateAudioSpec(&_spec);

        /* AU initiliaze */
        status = AudioUnitInitialize(auUnit);
        if (status != noErr) {
            ALOGE("AudioUnit: AudioUnitInitialize failed (%d)", (int)status);
            self = nil;
            return nil;
        }

        _auUnit = auUnit;
    }
    return self;
}

- (void)dealloc
{
    [self close];
}

- (void)play
{
    if (!_auUnit)
        return;

    _isPaused = NO;
    AudioSessionSetActive(true);
    AudioOutputUnitStart(_auUnit);
}

- (void)pause
{
    if (!_auUnit)
        return;

    _isPaused = YES;
    // Delay > 1 seconds on ios8
    // Maybe we don't need this call in pause
    // AudioSessionSetActive(false);
    OSStatus status = AudioOutputUnitStop(_auUnit);
    if (status != noErr)
        ALOGE("AudioUnit: failed to stop AudioUnit (%d)", (int)status);
}

- (void)flush
{
    if (!_auUnit)
        return;

    AudioUnitReset(_auUnit, kAudioUnitScope_Global, 0);
}

- (void)stop
{
    AudioSessionSetActive(false);

    if (!_auUnit)
        return;

    OSStatus status = AudioOutputUnitStop(_auUnit);
    if (status != noErr)
        ALOGE("AudioUnit: failed to stop AudioUnit (%d)", (int)status);
}

- (void)close
{
    [self stop];

    if (!_auUnit)
        return;

    AURenderCallbackStruct callback;
    memset(&callback, 0, sizeof(AURenderCallbackStruct));
    AudioUnitSetProperty(_auUnit,
                         kAudioUnitProperty_SetRenderCallback,
                         kAudioUnitScope_Input, 0, &callback,
                         sizeof(callback));

    AudioComponentInstanceDispose(_auUnit);
    _auUnit = NULL;
}

static OSStatus RenderCallback(void                        *inRefCon,
                               AudioUnitRenderActionFlags  *ioActionFlags,
                               const AudioTimeStamp        *inTimeStamp,
                               UInt32                      inBusNumber,
                               UInt32                      inNumberFrames,
                               AudioBufferList             *ioData)
{
    @autoreleasepool {
        TXSDLAudioUnitController* auController = (__bridge TXSDLAudioUnitController *) inRefCon;

        if (!auController || auController->_isPaused) {
            for (UInt32 i = 0; i < ioData->mNumberBuffers; i++) {
                AudioBuffer *ioBuffer = &ioData->mBuffers[i];
                memset(ioBuffer->mData, auController.spec.silence, ioBuffer->mDataByteSize);
            }
            return noErr;
        }

        for (int i = 0; i < (int)ioData->mNumberBuffers; i++) {
            AudioBuffer *ioBuffer = &ioData->mBuffers[i];
            (*auController.spec.callback)(auController.spec.userdata, ioBuffer->mData, ioBuffer->mDataByteSize);
        }

        return noErr;
    }
}

@end
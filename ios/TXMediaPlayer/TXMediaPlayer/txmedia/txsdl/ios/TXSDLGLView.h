
#import <UIKit/UIKit.h>

#include "txsdl/txsdl_vout.h"

@interface TXSDLGLView : UIView

- (id) initWithFrame:(CGRect)frame
          withChroma:(int)chroma;
- (void) display: (SDL_VoutOverlay *) overlay;

- (UIImage*) snapshot;

@property(nonatomic,strong) NSLock *appActivityLock;

@end

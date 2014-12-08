#include "txplayer/txplayer.h"
#import "TXSDLGLView.h"

// ref_count is 1 after open
txMediaPlayer *txmp_ios_create(int (*msg_loop)(void*));

void            txmp_ios_set_glview(txMediaPlayer *mp, TXSDLGLView *glView);

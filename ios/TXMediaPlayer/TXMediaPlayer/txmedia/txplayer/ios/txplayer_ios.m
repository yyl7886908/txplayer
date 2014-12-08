#import "txplayer_ios.h"

#import "txsdl/ios/txsdl_ios.h"

#include <stdio.h>
#include <assert.h>
#include "txplayer/ff_fferror.h"
#include "txplayer/ff_ffplay.h"
#include "txplayer/txplayer_internal.h"

txMediaPlayer *txmp_ios_create(int (*msg_loop)(void*))
{
    txMediaPlayer *mp = txmp_create(msg_loop);
    if (!mp)
        goto fail;

    mp->ffplayer->vout = SDL_VoutIos_CreateForGLES2();
    if (!mp->ffplayer->vout)
        goto fail;

    mp->ffplayer->aout = SDL_AoutIos_CreateForAudioUnit();
    if (!mp->ffplayer->vout)
        goto fail;

    return mp;

fail:
    txmp_dec_ref_p(&mp);
    return NULL;
}

void txmp_ios_set_glview_l(txMediaPlayer *mp, TXSDLGLView *glView)
{
    assert(mp);
    assert(mp->ffplayer);
    assert(mp->ffplayer->vout);

    SDL_VoutIos_SetGLView(mp->ffplayer->vout, glView);
}

void txmp_ios_set_glview(txMediaPlayer *mp, TXSDLGLView *glView)
{
    assert(mp);
    //MPTRACE("ijkmp_ios_set_view(glView=%p)\n", (void*)glView);
    pthread_mutex_lock(&mp->mutex);
    txmp_ios_set_glview_l(mp, glView);
    pthread_mutex_unlock(&mp->mutex);
    //MPTRACE("ijkmp_ios_set_view(glView=%p)=void\n", (void*)glView);
}

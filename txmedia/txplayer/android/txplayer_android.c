/*
 * txplayer_android.c
 *
 * Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
 *
 * This file is part of txPlayer.
 *
 * txPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * txPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with txPlayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "txplayer_android.h"

#include <assert.h>
#include "txsdl/android/txsdl_android.h"
#include "../ff_fferror.h"
#include "../ff_ffplay.h"
#include "../txplayer_internal.h"

txMediaPlayer *txmp_android_create(int(*msg_loop)(void*))
{
    txMediaPlayer *mp = txmp_create(msg_loop);
    if (!mp)
        goto fail;

    mp->ffplayer->vout = SDL_VoutAndroid_CreateForAndroidSurface();
    if (!mp->ffplayer->vout)
        goto fail;

    mp->ffplayer->aout = SDL_AoutAndroid_CreateForAudioTrack();
    if (!mp->ffplayer->aout)
        goto fail;

    return mp;

    fail:
    txmp_dec_ref_p(&mp);
    return NULL;
}

void txmp_android_set_surface_l(JNIEnv *env, txMediaPlayer *mp, jobject android_surface)
{
    if (!mp || !mp->ffplayer || !mp->ffplayer->vout)
        return;


    SDL_VoutAndroid_SetAndroidSurface(env, mp->ffplayer->vout, android_surface);
}

void txmp_android_set_surface(JNIEnv *env, txMediaPlayer *mp, jobject android_surface)
{
    if (!mp)
        return;

    MPTRACE("txmp_set_android_surface(surface=%p)", (void*)android_surface);
    pthread_mutex_lock(&mp->mutex);
    txmp_android_set_surface_l(env, mp, android_surface);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("txmp_set_android_surface(surface=%p)=void", (void*)android_surface);
}

void txmp_android_set_volume(JNIEnv *env, txMediaPlayer *mp, float left, float right)
{
    if (!mp)
        return;

    MPTRACE("txmp_android_set_volume(%f, %f)", left, right);
    pthread_mutex_lock(&mp->mutex);

    if (mp && mp->ffplayer && mp->ffplayer->aout) {
        SDL_AoutSetStereoVolume(mp->ffplayer->aout, left, right);
    }

    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("txmp_android_set_volume(%f, %f)=void", left, right);
}

/*
 * txplayer_android.h
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

#ifndef TXPLAYER_ANDROID__TXPLAYER_ANDROID_H
#define TXPLAYER_ANDROID__TXPLAYER_ANDROID_H

#include <jni.h>
#include "../txplayer.h"

// ref_count is 1 after open
txMediaPlayer *txmp_android_create(int(*msg_loop)(void*));

void            txmp_android_set_surface(JNIEnv *env, txMediaPlayer *mp, jobject android_surface);

void            txmp_android_set_volume(JNIEnv *env, txMediaPlayer *mp, float left, float right);

#endif

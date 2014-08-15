/*****************************************************************************
 * txsdl_vout_android_surface.h
 *****************************************************************************
 *
 * copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#ifndef TXSDL_ANDROID__TXSDL_VOUT_ANDROID_SURFACE_H
#define TXSDL_ANDROID__TXSDL_VOUT_ANDROID_SURFACE_H

#include <jni.h>
#include "../txsdl_stdinc.h"
#include "../txsdl_vout.h"

SDL_Vout *SDL_VoutAndroid_CreateForAndroidSurface();
void SDL_VoutAndroid_SetAndroidSurface(JNIEnv*env, SDL_Vout *vout, jobject android_surface);

#endif

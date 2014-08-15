/*****************************************************************************
 * txsdl_vout_overlay_ffmpeg.h
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

#ifndef __TXSDL_VOUT_OVERLAY_FFMPEG_H
#define __TXSDL_VOUT_OVERLAY_FFMPEG_H

#include "../txsdl_stdinc.h"
#include "../txsdl_vout.h"
#include "txsdl_inc_ffmpeg.h"

// TODO: 9 alignment to speed up memcpy when display
SDL_VoutOverlay *SDL_VoutFFmpeg_CreateOverlay(int width, int height, Uint32 format, SDL_Vout *vout);

int SDL_VoutFFmpeg_ConvertFrame(
    SDL_VoutOverlay *overlay, AVFrame *frame,
    struct SwsContext **p_sws_ctx, int sws_flags);

#endif

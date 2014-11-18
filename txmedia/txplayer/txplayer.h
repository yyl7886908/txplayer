/*
 * txplayer.h
 *
  * Copyright (c) 2014 Yu Yun Long <yuyunlong1987@163.com>
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

#ifndef TXPLAYER_ANDROID__TXPLAYER_H
#define TXPLAYER_ANDROID__TXPLAYER_H

#include <stdbool.h>
#include "ff_ffmsg_queue.h"

#include "txutil/txutil.h"

#ifndef MPTRACE
#define MPTRACE ALOGW
#endif

typedef struct txMediaPlayer txMediaPlayer;
typedef struct FFPlayer FFPlayer;
typedef struct SDL_Vout SDL_Vout;

/*-
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_IDLE);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_INITIALIZED);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_ASYNC_PREPARING);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_PREPARED);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_STARTED);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_PAUSED);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_COMPLETED);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_STOPPED);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_ERROR);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_END);
 */

/*-
 * txmp_set_data_source()  -> MP_STATE_INITIALIZED
 *
 * txmp_reset              -> self
 * txmp_release            -> MP_STATE_END
 */
#define MP_STATE_IDLE               0

/*-
 * txmp_prepare_async()    -> MP_STATE_ASYNC_PREPARING
 *
 * txmp_reset              -> MP_STATE_IDLE
 * txmp_release            -> MP_STATE_END
 */
#define MP_STATE_INITIALIZED        1

/*-
 *                   ...    -> MP_STATE_PREPARED
 *                   ...    -> MP_STATE_ERROR
 *
 * txmp_reset              -> MP_STATE_IDLE
 * txmp_release            -> MP_STATE_END
 */
#define MP_STATE_ASYNC_PREPARING    2

/*-
 * txmp_seek_to()          -> self
 * txmp_start()            -> MP_STATE_STARTED
 *
 * txmp_reset              -> MP_STATE_IDLE
 * txmp_release            -> MP_STATE_END
 */
#define MP_STATE_PREPARED           3

/*-
 * txmp_seek_to()          -> self
 * txmp_start()            -> self
 * txmp_pause()            -> MP_STATE_PAUSED
 * txmp_stop()             -> MP_STATE_STOPPED
 *                   ...    -> MP_STATE_COMPLETED
 *                   ...    -> MP_STATE_ERROR
 *
 * txmp_reset              -> MP_STATE_IDLE
 * txmp_release            -> MP_STATE_END
 */
#define MP_STATE_STARTED            4

/*-
 * txmp_seek_to()          -> self
 * txmp_start()            -> MP_STATE_STARTED
 * txmp_pause()            -> self
 * txmp_stop()             -> MP_STATE_STOPPED
 *
 * txmp_reset              -> MP_STATE_IDLE
 * txmp_release            -> MP_STATE_END
 */
#define MP_STATE_PAUSED             5

/*-
 * txmp_seek_to()          -> self
 * txmp_start()            -> MP_STATE_STARTED (from beginning)
 * txmp_pause()            -> self
 * txmp_stop()             -> MP_STATE_STOPPED
 *
 * txmp_reset              -> MP_STATE_IDLE
 * txmp_release            -> MP_STATE_END
 */
#define MP_STATE_COMPLETED          6

/*-
 * txmp_stop()             -> self
 * txmp_prepare_async()    -> MP_STATE_ASYNC_PREPARING
 *
 * txmp_reset              -> MP_STATE_IDLE
 * txmp_release            -> MP_STATE_END
 */
#define MP_STATE_STOPPED            7

/*-
 * txmp_reset              -> MP_STATE_IDLE
 * txmp_release            -> MP_STATE_END
 */
#define MP_STATE_ERROR              8

/*-
 * txmp_release            -> self
 */
#define MP_STATE_END                9

 void           txmp_global_init();
 void           txmp_global_uninit();

// ref_count is 1 after open
txMediaPlayer *txmp_create(int (*msg_loop)(void*));
void            txmp_set_format_callback(txMediaPlayer *mp, ijk_format_control_message cb, void *opaque);
void            txmp_set_format_option(txMediaPlayer *mp, const char *name, const char *value);
void            txmp_set_codec_option(txMediaPlayer *mp, const char *name, const char *value);
void            txmp_set_sws_option(txMediaPlayer *mp, const char *name, const char *value);
void            txmp_set_overlay_format(txMediaPlayer *mp, int chroma_fourcc);
void            txmp_set_picture_queue_capicity(txMediaPlayer *mp, int frame_count);
void            txmp_set_max_fps(txMediaPlayer *mp, int max_fps);

// preferred to be called explicity, can be called multiple times
// NOTE: txmp_shutdown may block thread
void            txmp_shutdown(txMediaPlayer *mp);

void            txmp_inc_ref(txMediaPlayer *mp);

// call close at last release, also free memory
// NOTE: txmp_dec_ref may block thread
void            txmp_dec_ref(txMediaPlayer *mp);
void            txmp_dec_ref_p(txMediaPlayer **pmp);

int             txmp_set_data_source(txMediaPlayer *mp, const char *url);
int             txmp_prepare_async(txMediaPlayer *mp);
int             txmp_start(txMediaPlayer *mp);
int             txmp_pause(txMediaPlayer *mp);
int             txmp_stop(txMediaPlayer *mp);
int             txmp_seek_to(txMediaPlayer *mp, long msec);
int             txmp_get_state(txMediaPlayer *mp);
bool            txmp_is_playing(txMediaPlayer *mp);
long            txmp_get_current_position(txMediaPlayer *mp);
long            txmp_get_duration(txMediaPlayer *mp);

void           *txmp_set_weak_thiz(txMediaPlayer *mp, void *weak_thiz);

/* return < 0 if aborted, 0 if no packet and > 0 if packet.  */
int             txmp_get_msg(txMediaPlayer *mp, AVMessage *msg, int block);

/* 照相接口 */
void          txmp_photo_image(txMediaPlayer *mp, char *filename);

/* 录制接口 */
void           txmp_start_recording_rtsp_stream(char *rtsp_stream, char *filename, int time);



/* 停止录制接口 */
void            txmp_stop_recording_rtsp_stream();


#endif

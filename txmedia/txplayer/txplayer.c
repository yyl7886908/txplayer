/*
 * txplayer.c
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

#include "txplayer.h"

#include "txplayer_internal.h"

#define MP_RET_IF_FAILED(ret) \
    do { \
        int retval = ret; \
        if (retval != 0) return (retval); \
    } while(0)

#define MPST_RET_IF_EQ_INT(real, expected, errcode) \
    do { \
        if ((real) == (expected)) return (errcode); \
    } while(0)

#define MPST_RET_IF_EQ(real, expected) \
    MPST_RET_IF_EQ_INT(real, expected, EIJK_INVALID_STATE)

inline static void txmp_destroy(txMediaPlayer *mp)
{
    if (!mp)
        return;

    ffp_destroy_p(&mp->ffplayer);

    pthread_mutex_destroy(&mp->mutex);

    av_freep(&mp->data_source);
    memset(mp, 0, sizeof(txMediaPlayer));
    av_freep(&mp);
}

inline static void txmp_destroy_p(txMediaPlayer **pmp)
{
    if (!pmp)
        return;

    txmp_destroy(*pmp);
    *pmp = NULL;
}

void txmp_global_init()
{
    ffp_global_init();
}

void txmp_global_uninit()
{
    ffp_global_uninit();
}

void txmp_change_state_l(txMediaPlayer *mp, int new_state)
{
    mp->mp_state = new_state;
    ffp_notify_msg1(mp->ffplayer, FFP_MSG_PLAYBACK_STATE_CHANGED);
}

txMediaPlayer *txmp_create(int (*msg_loop)(void*))
{
    txMediaPlayer *mp = (txMediaPlayer *) av_mallocz(sizeof(txMediaPlayer));
    if (!mp)
        goto fail;

    mp->ffplayer = ffp_create();
    if (!mp)
        goto fail;

    mp->msg_loop = msg_loop;

    txmp_inc_ref(mp);
    pthread_mutex_init(&mp->mutex, NULL);

    return mp;

    fail:
    txmp_destroy_p(&mp);
    return NULL;
}

void txmp_set_overlay_format(txMediaPlayer *mp, int chroma_fourcc)
{
    if (!mp)
        return;

    MPTRACE("txmp_set_overlay_format(%.4s(0x%x))\n", (char*)&chroma_fourcc, chroma_fourcc);
    if (mp->ffplayer) {
        ffp_set_overlay_format(mp->ffplayer, chroma_fourcc);
    }
    MPTRACE("txmp_set_overlay_format()=void\n");
}

void txmp_set_format_callback(txMediaPlayer *mp, ijk_format_control_message cb, void *opaque)
{
    assert(mp);

    MPTRACE("txmp_set_format_callback(%p, %p)\n", cb, opaque);
    pthread_mutex_lock(&mp->mutex);
    ffp_set_format_callback(mp->ffplayer, cb, opaque);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("txmp_set_format_callback()=void\n");
}

void txmp_set_format_option(txMediaPlayer *mp, const char *name, const char *value)
{
    assert(mp);

    MPTRACE("txmp_set_format_option(%s, %s)\n", name, value);
    pthread_mutex_lock(&mp->mutex);
    ffp_set_format_option(mp->ffplayer, name, value);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("txmp_set_format_option()=void\n");
}

void txmp_set_codec_option(txMediaPlayer *mp, const char *name, const char *value)
{
    assert(mp);

    MPTRACE("txmp_set_codec_option()\n");
    pthread_mutex_lock(&mp->mutex);
    ffp_set_codec_option(mp->ffplayer, name, value);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("txmp_set_codec_option()=void\n");
}

void txmp_set_sws_option(txMediaPlayer *mp, const char *name, const char *value)
{
    assert(mp);

    MPTRACE("txmp_set_sws_option()\n");
    pthread_mutex_lock(&mp->mutex);
    ffp_set_sws_option(mp->ffplayer, name, value);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("txmp_set_sws_option()=void\n");
}

void txmp_set_picture_queue_capicity(txMediaPlayer *mp, int frame_count)
{
    assert(mp);

    MPTRACE("txmp_set_picture_queue_capicity(%d)\n", frame_count);
    pthread_mutex_lock(&mp->mutex);
    ffp_set_picture_queue_capicity(mp->ffplayer, frame_count);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("txmp_set_picture_queue_capicity()=void\n");
}

void txmp_set_max_fps(txMediaPlayer *mp, int max_fps)
{
    assert(mp);

    MPTRACE("txmp_set_max_fp(%d)\n", max_fps);
    pthread_mutex_lock(&mp->mutex);
    ffp_set_max_fps(mp->ffplayer, max_fps);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("txmp_set_max_fp()=void\n");
}

void txmp_shutdown_l(txMediaPlayer *mp)
{
    assert(mp);

    MPTRACE("txmp_shutdown_l()\n");
    if (mp->ffplayer) {
        ffp_stop_l(mp->ffplayer);
        ffp_wait_stop_l(mp->ffplayer);
    }
    MPTRACE("txmp_shutdown_l()=void\n");
}

void txmp_shutdown(txMediaPlayer *mp)
{
    return txmp_shutdown_l(mp);
}

void txmp_inc_ref(txMediaPlayer *mp)
{
    assert(mp);
    __sync_fetch_and_add(&mp->ref_count, 1);
}

void txmp_dec_ref(txMediaPlayer *mp)
{
    if (!mp)
        return;

    int ref_count = __sync_sub_and_fetch(&mp->ref_count, 1);
    if (ref_count == 0) {
        MPTRACE("txmp_dec_ref(): ref=0\n");
        txmp_shutdown(mp);
        txmp_destroy_p(&mp);
    }
}

void txmp_dec_ref_p(txMediaPlayer **pmp)
{
    if (!pmp)
        return;

    txmp_dec_ref(*pmp);
    *pmp = NULL;
}

static int txmp_set_data_source_l(txMediaPlayer *mp, const char *url)
{
    assert(mp);
    assert(url);

    // MPST_RET_IF_EQ(mp->mp_state, MP_STATE_IDLE);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_INITIALIZED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_ASYNC_PREPARING);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_PREPARED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_STARTED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_PAUSED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_COMPLETED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_STOPPED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_ERROR);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_END);

    av_freep(&mp->data_source);
    mp->data_source = av_strdup(url);
    if (!mp->data_source)
        return EIJK_OUT_OF_MEMORY;

    txmp_change_state_l(mp, MP_STATE_INITIALIZED);
    return 0;
}

int txmp_set_data_source(txMediaPlayer *mp, const char *url)
{
    assert(mp);
    assert(url);
    MPTRACE("txmp_set_data_source(url=\"%s\")\n", url);
    pthread_mutex_lock(&mp->mutex);
    int retval = txmp_set_data_source_l(mp, url);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("txmp_set_data_source(url=\"%s\")=%d\n", url, retval);
    return retval;
}

static int txmp_prepare_async_l(txMediaPlayer *mp)
{
    assert(mp);

    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_IDLE);
    // MPST_RET_IF_EQ(mp->mp_state, MP_STATE_INITIALIZED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_ASYNC_PREPARING);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_PREPARED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_STARTED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_PAUSED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_COMPLETED);
    // MPST_RET_IF_EQ(mp->mp_state, MP_STATE_STOPPED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_ERROR);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_END);

    assert(mp->data_source);

    txmp_change_state_l(mp, MP_STATE_ASYNC_PREPARING);

    msg_queue_start(&mp->ffplayer->msg_queue);

    // released in msg_loop
    txmp_inc_ref(mp);
    mp->msg_thread = SDL_CreateThreadEx(&mp->_msg_thread, mp->msg_loop, mp, "ff_msg_loop");
    // TODO: 9 release weak_thiz if pthread_create() failed;

    int retval = ffp_prepare_async_l(mp->ffplayer, mp->data_source);
    if (retval < 0) {
        txmp_change_state_l(mp, MP_STATE_ERROR);
        return retval;
    }

    return 0;
}

int txmp_prepare_async(txMediaPlayer *mp)
{
    assert(mp);
    MPTRACE("txmp_prepare_async()\n");
    pthread_mutex_lock(&mp->mutex);
    int retval = txmp_prepare_async_l(mp);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("txmp_prepare_async()=%d\n", retval);
    return retval;
}

static int ikjmp_chkst_start_l(int mp_state)
{
    MPST_RET_IF_EQ(mp_state, MP_STATE_IDLE);
    MPST_RET_IF_EQ(mp_state, MP_STATE_INITIALIZED);
    MPST_RET_IF_EQ(mp_state, MP_STATE_ASYNC_PREPARING);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_PREPARED);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_STARTED);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_PAUSED);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_COMPLETED);
    MPST_RET_IF_EQ(mp_state, MP_STATE_STOPPED);
    MPST_RET_IF_EQ(mp_state, MP_STATE_ERROR);
    MPST_RET_IF_EQ(mp_state, MP_STATE_END);

    return 0;
}

static int txmp_start_l(txMediaPlayer *mp)
{
    assert(mp);

    MP_RET_IF_FAILED(ikjmp_chkst_start_l(mp->mp_state));

    ffp_remove_msg(mp->ffplayer, FFP_REQ_START);
    ffp_remove_msg(mp->ffplayer, FFP_REQ_PAUSE);
    ffp_notify_msg1(mp->ffplayer, FFP_REQ_START);

    return 0;
}

int txmp_start(txMediaPlayer *mp)
{
    assert(mp);
    MPTRACE("txmp_start()\n");
    pthread_mutex_lock(&mp->mutex);
    int retval = txmp_start_l(mp);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("txmp_start()=%d\n", retval);
    return retval;
}

static int ikjmp_chkst_pause_l(int mp_state)
{
    MPST_RET_IF_EQ(mp_state, MP_STATE_IDLE);
    MPST_RET_IF_EQ(mp_state, MP_STATE_INITIALIZED);
    MPST_RET_IF_EQ(mp_state, MP_STATE_ASYNC_PREPARING);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_PREPARED);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_STARTED);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_PAUSED);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_COMPLETED);
    MPST_RET_IF_EQ(mp_state, MP_STATE_STOPPED);
    MPST_RET_IF_EQ(mp_state, MP_STATE_ERROR);
    MPST_RET_IF_EQ(mp_state, MP_STATE_END);

    return 0;
}

static int txmp_pause_l(txMediaPlayer *mp)
{
    assert(mp);

    MP_RET_IF_FAILED(ikjmp_chkst_pause_l(mp->mp_state));

    ffp_remove_msg(mp->ffplayer, FFP_REQ_START);
    ffp_remove_msg(mp->ffplayer, FFP_REQ_PAUSE);
    ffp_notify_msg1(mp->ffplayer, FFP_REQ_PAUSE);

    return 0;
}

int txmp_pause(txMediaPlayer *mp)
{
    assert(mp);
    MPTRACE("txmp_pause()\n");
    pthread_mutex_lock(&mp->mutex);
    int retval = txmp_pause_l(mp);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("txmp_pause()=%d\n", retval);
    return retval;
}

static int txmp_stop_l(txMediaPlayer *mp)
{
    assert(mp);

    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_IDLE);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_INITIALIZED);
    // MPST_RET_IF_EQ(mp->mp_state, MP_STATE_ASYNC_PREPARING);
    // MPST_RET_IF_EQ(mp->mp_state, MP_STATE_PREPARED);
    // MPST_RET_IF_EQ(mp->mp_state, MP_STATE_STARTED);
    // MPST_RET_IF_EQ(mp->mp_state, MP_STATE_PAUSED);
    // MPST_RET_IF_EQ(mp->mp_state, MP_STATE_COMPLETED);
    // MPST_RET_IF_EQ(mp->mp_state, MP_STATE_STOPPED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_ERROR);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_END);

    ffp_remove_msg(mp->ffplayer, FFP_REQ_START);
    ffp_remove_msg(mp->ffplayer, FFP_REQ_PAUSE);
    int retval = ffp_stop_l(mp->ffplayer);
    if (retval < 0) {
        return retval;
    }

    txmp_change_state_l(mp, MP_STATE_STOPPED);
    return 0;
}

int txmp_stop(txMediaPlayer *mp)
{
    assert(mp);
    MPTRACE("txmp_stop()\n");
    pthread_mutex_lock(&mp->mutex);
    int retval = txmp_stop_l(mp);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("txmp_stop()=%d\n", retval);
    return retval;
}

bool txmp_is_playing(txMediaPlayer *mp)
{
    assert(mp);
    if (mp->mp_state == MP_STATE_PREPARED ||
        mp->mp_state == MP_STATE_STARTED) {
        return true;
    }

    return false;
}

static int ikjmp_chkst_seek_l(int mp_state)
{
    MPST_RET_IF_EQ(mp_state, MP_STATE_IDLE);
    MPST_RET_IF_EQ(mp_state, MP_STATE_INITIALIZED);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_ASYNC_PREPARING);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_PREPARED);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_STARTED);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_PAUSED);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_COMPLETED);
    MPST_RET_IF_EQ(mp_state, MP_STATE_STOPPED);
    MPST_RET_IF_EQ(mp_state, MP_STATE_ERROR);
    MPST_RET_IF_EQ(mp_state, MP_STATE_END);

    return 0;
}

int txmp_seek_to_l(txMediaPlayer *mp, long msec)
{
    assert(mp);

    MP_RET_IF_FAILED(ikjmp_chkst_seek_l(mp->mp_state));

    mp->seek_req = 1;
    mp->seek_msec = msec;
    ffp_remove_msg(mp->ffplayer, FFP_REQ_SEEK);
    ffp_notify_msg2(mp->ffplayer, FFP_REQ_SEEK, (int)msec);
    // TODO: 9 64-bit long?

    return 0;
}

int txmp_seek_to(txMediaPlayer *mp, long msec)
{
    assert(mp);
    MPTRACE("txmp_seek_to(%ld)\n", msec);
    pthread_mutex_lock(&mp->mutex);
    int retval = txmp_seek_to_l(mp, msec);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("txmp_seek_to(%ld)=%d\n", msec, retval);

    return retval;
}

int txmp_get_state(txMediaPlayer *mp)
{
    return mp->mp_state;
}

static long txmp_get_current_position_l(txMediaPlayer *mp)
{
    return ffp_get_current_position_l(mp->ffplayer);
}

long txmp_get_current_position(txMediaPlayer *mp)
{
    assert(mp);
    pthread_mutex_lock(&mp->mutex);
    long retval;
    if (mp->seek_req)
        retval = mp->seek_msec;
    else
        retval = txmp_get_current_position_l(mp);
    pthread_mutex_unlock(&mp->mutex);
    return retval;
}

static long txmp_get_duration_l(txMediaPlayer *mp)
{
    return ffp_get_duration_l(mp->ffplayer);
}

long txmp_get_duration(txMediaPlayer *mp)
{
    assert(mp);
    pthread_mutex_lock(&mp->mutex);
    long retval = txmp_get_duration_l(mp);
    pthread_mutex_unlock(&mp->mutex);
    return retval;
}

void *txmp_set_weak_thiz(txMediaPlayer *mp, void *weak_thiz)
{
    void *prev_weak_thiz = mp->weak_thiz;

    mp->weak_thiz = weak_thiz;

    return prev_weak_thiz;
}

int txmp_get_msg(txMediaPlayer *mp, AVMessage *msg, int block)
{
    assert(mp);
    while (1) {
        int continue_wait_next_msg = 0;
        int retval = msg_queue_get(&mp->ffplayer->msg_queue, msg, block);
        if (retval <= 0)
            return retval;

        switch (msg->what) {
        case FFP_MSG_PREPARED:
            MPTRACE("txmp_get_msg: FFP_MSG_PREPARED\n");
            pthread_mutex_lock(&mp->mutex);
            if (mp->mp_state == MP_STATE_ASYNC_PREPARING) {
                txmp_change_state_l(mp, MP_STATE_PREPARED);
            } else {
                // FIXME: 1: onError() ?
                ALOGE("FFP_MSG_PREPARED: expecting mp_state==MP_STATE_ASYNC_PREPARING\n");
            }
            pthread_mutex_unlock(&mp->mutex);
            break;

        case FFP_MSG_COMPLETED:
            MPTRACE("txmp_get_msg: FFP_MSG_COMPLETED\n");

            pthread_mutex_lock(&mp->mutex);
            mp->restart_from_beginning = 1;
            txmp_change_state_l(mp, MP_STATE_COMPLETED);
            pthread_mutex_unlock(&mp->mutex);
            break;

        case FFP_MSG_SEEK_COMPLETE:
            MPTRACE("txmp_get_msg: FFP_MSG_SEEK_COMPLETE\n");

            pthread_mutex_lock(&mp->mutex);
            mp->seek_req = 0;
            mp->seek_msec = 0;
            pthread_mutex_unlock(&mp->mutex);
            break;

        case FFP_REQ_START:
            MPTRACE("txmp_get_msg: FFP_REQ_START\n");
            continue_wait_next_msg = 1;
            pthread_mutex_lock(&mp->mutex);
            if (0 == ikjmp_chkst_start_l(mp->mp_state)) {
                // FIXME: 8 check seekable
                if (mp->mp_state == MP_STATE_COMPLETED) {
                    if (mp->restart_from_beginning) {
                        ALOGD("txmp_get_msg: FFP_REQ_START: restart from beginning\n");
                        retval = ffp_start_from_l(mp->ffplayer, 0);
                        if (retval == 0)
                            txmp_change_state_l(mp, MP_STATE_STARTED);
                    } else {
                        ALOGD("txmp_get_msg: FFP_REQ_START: restart from seek pos\n");
                        retval = ffp_start_l(mp->ffplayer);
                        if (retval == 0)
                            txmp_change_state_l(mp, MP_STATE_STARTED);
                    }
                    mp->restart_from_beginning = 0;
                } else {
                    ALOGD("txmp_get_msg: FFP_REQ_START: start on fly\n");
                    retval = ffp_start_l(mp->ffplayer);
                    if (retval == 0)
                        txmp_change_state_l(mp, MP_STATE_STARTED);
                }
            }
            pthread_mutex_unlock(&mp->mutex);
            break;

        case FFP_REQ_PAUSE:
            MPTRACE("txmp_get_msg: FFP_REQ_PAUSE\n");
            continue_wait_next_msg = 1;
            pthread_mutex_lock(&mp->mutex);
            if (0 == ikjmp_chkst_pause_l(mp->mp_state)) {
                int pause_ret = ffp_pause_l(mp->ffplayer);
                if (pause_ret == 0)
                    txmp_change_state_l(mp, MP_STATE_PAUSED);
            }
            pthread_mutex_unlock(&mp->mutex);
            break;

        case FFP_REQ_SEEK:
            MPTRACE("txmp_get_msg: FFP_REQ_SEEK\n");
            continue_wait_next_msg = 1;

            pthread_mutex_lock(&mp->mutex);
            if (0 == ikjmp_chkst_seek_l(mp->mp_state)) {
                if (0 == ffp_seek_to_l(mp->ffplayer, msg->arg1)) {
                    ALOGD("txmp_get_msg: FFP_REQ_SEEK: seek to %d\n", (int)msg->arg1);
                    mp->restart_from_beginning = 0;
                }
            }
            pthread_mutex_unlock(&mp->mutex);
            break;
        }

        if (continue_wait_next_msg)
            continue;

        return retval;
    }

    return -1;
}

/* 照相接口 */
void txmp_photo_image(txMediaPlayer *mp, char *filename)
{
    ALOGD("xxx===============filename txplayer.c%s\n", filename);
    assert(mp);
    MPTRACE("txmp_photo_image filename = %s)\n", filename);
    pthread_mutex_lock(&mp->mutex);
    ffp_photo_image(filename);
    pthread_mutex_unlock(&mp->mutex);
}

/* 录制接口 */
void           txmp_start_recording_rtsp_stream(char *rtsp_stream, char *filename, int time)
{
    MPTRACE("txmp_start_recording_rtsp_stream  rtsp_stream= %s, filename = %s\n",rtsp_stream,  filename);
    start_recording_rtsp_stream(rtsp_stream, filename, time);
}

/* 停止录制接口 */
void            txmp_stop_recording_rtsp_stream()
{
    MPTRACE("txmp_start_recording_rtsp_stream\n");
    stop_recording_rtsp_stream();
}

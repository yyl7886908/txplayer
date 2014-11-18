/*
 * txplayer_jni.c
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

#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <jni.h>
#include "txutil/txutil.h"
#include "txadk/txadk.h"
#include "../ff_ffplay.h"
#include "txplayer_android_def.h"
#include "txplayer_android.h"
#include "txsdl/android/txsdl_android_jni.h"


/* #define JNI_MODULE_PACKAGE      "tv/danmaku/ijk/media/player" */
/* #define JNI_CLASS_IJKPLAYER     "tv/danmaku/ijk/media/player/IjkMediaPlayer" */
/* #define JNI_IJK_MEDIA_EXCEPTION "tv/danmaku/ijk/media/player/IjkMediaException" */

#define JNI_MODULE_PACKAGE      "com/taixin/ffmpeg/player"
#define JNI_CLASS_IJKPLAYER     "com/taixin/ffmpeg/player/TXMediaPlayer"
#define JNI_IJK_MEDIA_EXCEPTION "com/taixin/ffmpeg/player/TXMediaException"

#define IJK_CHECK_MPRET_GOTO(retval, env, label) \
    JNI_CHECK_GOTO((retval != EIJK_INVALID_STATE), env, "java/lang/IllegalStateException", NULL, LABEL_RETURN); \
    JNI_CHECK_GOTO((retval != EIJK_OUT_OF_MEMORY), env, "java/lang/OutOfMemoryError", NULL, LABEL_RETURN); \
    JNI_CHECK_GOTO((retval == 0), env, JNI_IJK_MEDIA_EXCEPTION, NULL, LABEL_RETURN);

static JavaVM* g_jvm;

typedef struct player_fields_t {
    pthread_mutex_t mutex;
    jclass clazz;

    jfieldID mNativeMediaPlayer;

    jfieldID surface_texture;

    jmethodID postEventFromNative;
    jmethodID onControlResolveSegmentCount;
    jmethodID onControlResolveSegmentUrl;
    jmethodID onControlResolveSegmentOfflineMrl;
    jmethodID onControlResolveSegmentDuration;
} player_fields_t;
static player_fields_t g_clazz;

static int format_control_message(void *opaque, int type, void *data, size_t data_size);

static txMediaPlayer *jni_get_media_player(JNIEnv* env, jobject thiz)
{
    pthread_mutex_lock(&g_clazz.mutex);

    txMediaPlayer *mp = (txMediaPlayer *) (intptr_t) (*env)->GetLongField(env, thiz, g_clazz.mNativeMediaPlayer);
    if (mp) {
        txmp_inc_ref(mp);
    }

    pthread_mutex_unlock(&g_clazz.mutex);
    return mp;
}

static txMediaPlayer *jni_set_media_player(JNIEnv* env, jobject thiz, txMediaPlayer *mp)
{
    pthread_mutex_lock(&g_clazz.mutex);

    txMediaPlayer *old = (txMediaPlayer*) (intptr_t) (*env)->GetLongField(env, thiz, g_clazz.mNativeMediaPlayer);
    if (mp) {
        txmp_inc_ref(mp);
    }
    (*env)->SetLongField(env, thiz, g_clazz.mNativeMediaPlayer, (intptr_t) mp);

    pthread_mutex_unlock(&g_clazz.mutex);

    // NOTE: txmp_dec_ref may block thread
    if (old != NULL) {
        txmp_dec_ref_p(&old);
    }

    return old;
}

static int message_loop(void *arg);

static void
txMediaPlayer_setDataSourceAndHeaders(
    JNIEnv *env, jobject thiz, jstring path,
    jobjectArray keys, jobjectArray values)
{
    MPTRACE("txMediaPlayer_setDataSourceAndHeaders");
    int retval = 0;
    const char *c_path = NULL;
    txMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(path, env, "java/lang/IllegalArgumentException", "mpjni: setDataSource: null path", LABEL_RETURN);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: setDataSource: null mp", LABEL_RETURN);

    c_path = (*env)->GetStringUTFChars(env, path, NULL);
    JNI_CHECK_GOTO(c_path, env, "java/lang/OutOfMemoryError", "mpjni: setDataSource: path.string oom", LABEL_RETURN);

    ALOGV("setDataSource: path %s", c_path);
    retval = txmp_set_data_source(mp, c_path);
    (*env)->ReleaseStringUTFChars(env, path, c_path);

    IJK_CHECK_MPRET_GOTO(retval, env, LABEL_RETURN);

    LABEL_RETURN:
    txmp_dec_ref_p(&mp);
}

static void
txMediaPlayer_setVideoSurface(JNIEnv *env, jobject thiz, jobject jsurface)
{
    MPTRACE("txMediaPlayer_setVideoSurface");
    txMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, NULL, "mpjni: setVideoSurface: null mp", LABEL_RETURN);

    txmp_android_set_surface(env, mp, jsurface);

    LABEL_RETURN:
    txmp_dec_ref_p(&mp);
    return;
}

static void
txMediaPlayer_prepareAsync(JNIEnv *env, jobject thiz)
{
    MPTRACE("txMediaPlayer_prepareAsync");
    int retval = 0;
    txMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: prepareAsync: null mp", LABEL_RETURN);

    retval = txmp_prepare_async(mp);
    IJK_CHECK_MPRET_GOTO(retval, env, LABEL_RETURN);

    LABEL_RETURN:
    txmp_dec_ref_p(&mp);
}

static void
txMediaPlayer_start(JNIEnv *env, jobject thiz)
{
    MPTRACE("txMediaPlayer_start");
    txMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: start: null mp", LABEL_RETURN);

    txmp_start(mp);

    LABEL_RETURN:
    txmp_dec_ref_p(&mp);
}

/* 照相函数 */
static void txMediaPlayer_photoImage(JNIEnv *env, jobject thiz,  jstring filename)
{
     txMediaPlayer *mp = jni_get_media_player(env, thiz);
     JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: stop: null mp", LABEL_RETURN);
     char *c_filename = NULL;
     c_filename = (*env)->GetStringUTFChars(env, filename, NULL);
     ALOGE("=====txplayer_jni.c  c_filename = %s\n", c_filename);
     txmp_photo_image(mp, c_filename);
     LABEL_RETURN:
     txmp_dec_ref_p(&mp);
}
/* 照相函数 */

/* 开始录制函数 */
static void txMediaPlayer_startRecordingRtsp(JNIEnv *env, jobject thiz,jstring rtsp_stream,  jstring filename, jint time)
{
    ALOGE("=====txplayer_jni.c startRecordingRtsp filename = %s\n", (*env)->GetStringUTFChars(env, filename, NULL));
    txmp_start_recording_rtsp_stream((*env)->GetStringUTFChars(env, rtsp_stream, NULL), (*env)->GetStringUTFChars(env, filename, NULL), (int)time);
}

/* 停止录制函数 */
static void txMediaPlayer_stopRecordingRtsp(JNIEnv *env, jobject thiz)
{
    txmp_stop_recording_rtsp_stream();
}

static void
txMediaPlayer_stop(JNIEnv *env, jobject thiz)
{
    txMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: stop: null mp", LABEL_RETURN);

    txmp_stop(mp);

    LABEL_RETURN:
    txmp_dec_ref_p(&mp);
}

static void
txMediaPlayer_pause(JNIEnv *env, jobject thiz)
{
    txMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: pause: null mp", LABEL_RETURN);

    txmp_pause(mp);

    LABEL_RETURN:
    txmp_dec_ref_p(&mp);
}

static void
txMediaPlayer_seekTo(JNIEnv *env, jobject thiz, int msec)
{
    MPTRACE("txMediaPlayer_seekTo");
    txMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: seekTo: null mp", LABEL_RETURN);

    txmp_seek_to(mp, msec);

    LABEL_RETURN:
    txmp_dec_ref_p(&mp);
}

static jboolean
txMediaPlayer_isPlaying(JNIEnv *env, jobject thiz)
{
    jboolean retval = JNI_FALSE;
    txMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, NULL, "mpjni: isPlaying: null mp", LABEL_RETURN);

    retval = txmp_is_playing(mp) ? JNI_TRUE : JNI_FALSE;

    LABEL_RETURN:
    txmp_dec_ref_p(&mp);
    return retval;
}

static int
txMediaPlayer_getCurrentPosition(JNIEnv *env, jobject thiz)
{
    int retval = 0;
    txMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, NULL, "mpjni: getCurrentPosition: null mp", LABEL_RETURN);

    retval = txmp_get_current_position(mp);

    LABEL_RETURN:
    txmp_dec_ref_p(&mp);
    return retval;
}

static int
txMediaPlayer_getDuration(JNIEnv *env, jobject thiz)
{
    int retval = 0;
    txMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, NULL, "mpjni: getDuration: null mp", LABEL_RETURN);

    retval = txmp_get_duration(mp);

    LABEL_RETURN:
    txmp_dec_ref_p(&mp);
    return retval;
}

static void
txMediaPlayer_release(JNIEnv *env, jobject thiz)
{
    MPTRACE("txMediaPlayer_release");
    txMediaPlayer *mp = jni_get_media_player(env, thiz);
    if (!mp)
        return;

    // explicit shutdown mp, in case it is not the last mp-ref here
    txmp_android_set_surface(env, mp, NULL);
    txmp_shutdown(mp);
    jni_set_media_player(env, thiz, NULL);

    txmp_dec_ref_p(&mp);
}

static void txMediaPlayer_native_setup(JNIEnv *env, jobject thiz, jobject weak_this);
static void
txMediaPlayer_reset(JNIEnv *env, jobject thiz)
{
    MPTRACE("txMediaPlayer_reset");
    txMediaPlayer *mp = jni_get_media_player(env, thiz);
    if (!mp)
        return;

    jobject weak_thiz = (jobject) txmp_set_weak_thiz(mp, NULL);

    txMediaPlayer_release(env, thiz);
    txMediaPlayer_native_setup(env, thiz, weak_thiz);
}

static void
txMediaPlayer_setVolume(JNIEnv *env, jobject thiz, jfloat leftVolume, jfloat rightVolume)
{
    MPTRACE("txMediaPlayer_setVolume");
    txMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, NULL, "mpjni: setVolume: null mp", LABEL_RETURN);

    txmp_android_set_volume(env, mp, leftVolume, rightVolume);

    LABEL_RETURN:
    txmp_dec_ref_p(&mp);
}

static void
txMediaPlayer_setAvFormatOption(JNIEnv *env, jobject thiz, jobject name, jobject value)
{
    MPTRACE("txMediaPlayer_setAvFormatOption");
    txMediaPlayer *mp = jni_get_media_player(env, thiz);
    const char *c_name = NULL;
    const char *c_value = NULL;
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: setAvFormatOption: null mp", LABEL_RETURN);

    c_name = (*env)->GetStringUTFChars(env, name, NULL);
    JNI_CHECK_GOTO(c_name, env, "java/lang/OutOfMemoryError", "mpjni: setAvFormatOption: name.string oom", LABEL_RETURN);

    c_value = (*env)->GetStringUTFChars(env, value, NULL);
    JNI_CHECK_GOTO(c_name, env, "java/lang/OutOfMemoryError", "mpjni: setAvFormatOption: name.string oom", LABEL_RETURN);

    txmp_set_format_option(mp, c_name, c_value);

    LABEL_RETURN:
    if (c_name)
        (*env)->ReleaseStringUTFChars(env, name, c_name);
    if (c_value)
        (*env)->ReleaseStringUTFChars(env, value, c_value);
    txmp_dec_ref_p(&mp);
}

static void
txMediaPlayer_setAvCodecOption(JNIEnv *env, jobject thiz, jobject name, jobject value)
{
    MPTRACE("txMediaPlayer_setAvCodecOption");
    txMediaPlayer *mp = jni_get_media_player(env, thiz);
    const char *c_name = NULL;
    const char *c_value = NULL;
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: setAvCodecOption: null mp", LABEL_RETURN);

    c_name = (*env)->GetStringUTFChars(env, name, NULL);
    JNI_CHECK_GOTO(c_name, env, "java/lang/OutOfMemoryError", "mpjni: setAvCodecOption: name.string oom", LABEL_RETURN);

    c_value = (*env)->GetStringUTFChars(env, value, NULL);
    JNI_CHECK_GOTO(c_name, env, "java/lang/OutOfMemoryError", "mpjni: setAvCodecOption: name.string oom", LABEL_RETURN);

    txmp_set_codec_option(mp, c_name, c_value);

    LABEL_RETURN:
    if (c_name)
        (*env)->ReleaseStringUTFChars(env, name, c_name);
    if (c_value)
        (*env)->ReleaseStringUTFChars(env, value, c_value);
    txmp_dec_ref_p(&mp);
}

static void
txMediaPlayer_setOverlayFormat(JNIEnv *env, jobject thiz, jint chromaFourCC)
{
    MPTRACE("txMediaPlayer_setOverlayFormat");
    txMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: setAvCodecOption: null mp", LABEL_RETURN);

    txmp_set_overlay_format(mp, chromaFourCC);

    LABEL_RETURN:
    txmp_dec_ref_p(&mp);
}

static void
txMediaPlayer_native_init(JNIEnv *env)
{
    MPTRACE("txMediaPlayer_native_init");
}

static void
txMediaPlayer_native_setup(JNIEnv *env, jobject thiz, jobject weak_this)
{
    MPTRACE("txMediaPlayer_native_setup");
    txMediaPlayer *mp = txmp_android_create(message_loop);
    JNI_CHECK_GOTO(mp, env, "java/lang/OutOfMemoryError", "mpjni: native_setup: txmp_create() failed", LABEL_RETURN);

    jni_set_media_player(env, thiz, mp);
    txmp_set_weak_thiz(mp, (*env)->NewGlobalRef(env, weak_this));
    txmp_set_format_callback(mp, format_control_message, (*env)->NewGlobalRef(env, weak_this));

    LABEL_RETURN:
    txmp_dec_ref_p(&mp);
}

static void
txMediaPlayer_native_finalize(JNIEnv *env, jobject thiz, jobject name, jobject value)
{
    MPTRACE("txMediaPlayer_native_finalize");
    txMediaPlayer_release(env, thiz);
}

static int
_onNativeControlResolveSegmentConcat(JNIEnv *env, jobject weak_thiz, int type, void *data, size_t data_size)
{
    if (weak_thiz == NULL || data == NULL)
        return -1;

    IJKFormatSegmentConcatContext *fsc_concat = (IJKFormatSegmentConcatContext *)data;

    jint count = (*env)->CallStaticIntMethod(env, g_clazz.clazz, g_clazz.onControlResolveSegmentCount, weak_thiz);
    if (count <= 0)
        return -1;

    fsc_concat->count = count;
    return 0;
}

static int
_onNativeControlResolveSegment(JNIEnv *env, jobject weak_thiz, int type, void *data, size_t data_size)
{
    if (weak_thiz == NULL || data == NULL)
        return -1;

    IJKFormatSegmentContext *fsc = (IJKFormatSegmentContext *)data;

    jint duration = (*env)->CallStaticIntMethod(env, g_clazz.clazz, g_clazz.onControlResolveSegmentDuration, weak_thiz, fsc->position);

    jstring url = (*env)->CallStaticObjectMethod(env, g_clazz.clazz, g_clazz.onControlResolveSegmentUrl, weak_thiz, fsc->position);
    if (url == NULL)
        return -1;

    const char* c_url = (*env)->GetStringUTFChars(env, url, NULL);
    if (c_url == NULL)
        return -1;

    fsc->url = strdup(c_url);
    (*env)->ReleaseStringUTFChars(env, url, c_url);

    if (fsc->url == NULL)
        return -1;

    fsc->duration  = duration;
    fsc->duration *= 1000;
    fsc->url_free  = free;
    return 0;
}

static int
_onNativeControlResolveSegmentOffline(JNIEnv *env, jobject weak_thiz, int type, void *data, size_t data_size)
{
    if (weak_thiz == NULL || data == NULL)
        return -1;

    IJKFormatSegmentContext *fsc = (IJKFormatSegmentContext *)data;
    jint duration = (*env)->CallStaticIntMethod(env, g_clazz.clazz, g_clazz.onControlResolveSegmentDuration, weak_thiz, fsc->position);

    jstring mrl = (*env)->CallStaticObjectMethod(env, g_clazz.clazz, g_clazz.onControlResolveSegmentOfflineMrl, weak_thiz, fsc->position);
    if (mrl == NULL)
        return -1;

    const char* c_mrl = (*env)->GetStringUTFChars(env, mrl, NULL);
    if (c_mrl == NULL)
        return -1;

    fsc->url = strdup(c_mrl);
    (*env)->ReleaseStringUTFChars(env, mrl, c_mrl);

    if (fsc->url == NULL)
        return -1;

    fsc->duration  = duration;
    fsc->duration *= 1000;
    fsc->url_free  = free;
    return 0;
}

// NOTE: support to be called from read_thread
static int
format_control_message(void *opaque, int type, void *data, size_t data_size)
{
    JNIEnv *env = NULL;
    SDL_AndroidJni_SetupThreadEnv(&env);

    jobject weak_thiz = (jobject) opaque;
    if (weak_thiz == NULL)
        return -1;

    switch (type) {
        case IJKAVF_CM_RESOLVE_SEGMENT_CONCAT:
            return _onNativeControlResolveSegmentConcat(env, weak_thiz, type, data, data_size);
        case IJKAVF_CM_RESOLVE_SEGMENT:
            return _onNativeControlResolveSegment(env, weak_thiz, type, data, data_size);
        case IJKAVF_CM_RESOLVE_SEGMENT_OFFLINE:
            return _onNativeControlResolveSegmentOffline(env, weak_thiz, type, data, data_size);
        default: {
            return -1;
        }
    }

    return -1;
}

inline static void post_event(JNIEnv *env, jobject weak_this, int what, int arg1, int arg2)
{
    // MPTRACE("post_event(%p, %p, %d, %d, %d)", (void*)env, (void*) weak_this, what, arg1, arg2);
    (*env)->CallStaticVoidMethod(env, g_clazz.clazz, g_clazz.postEventFromNative, weak_this, what, arg1, arg2, NULL);
    // MPTRACE("post_event()=void");
}

static void message_loop_n(JNIEnv *env, txMediaPlayer *mp)
{
    jobject weak_thiz = (jobject) txmp_set_weak_thiz(mp, NULL);
    JNI_CHECK_GOTO(mp, env, NULL, "mpjni: message_loop_n: null weak_thiz", LABEL_RETURN);

    while (true) {
        AVMessage msg;

        int retval = txmp_get_msg(mp, &msg, 1);
        if (retval < 0)
            break;

        // block-get should never return 0
        assert(retval > 0);

        switch (msg.what) {
        case FFP_MSG_FLUSH:
            MPTRACE("FFP_MSG_FLUSH:");
            post_event(env, weak_thiz, MEDIA_NOP, 0, 0);
            break;
        case FFP_MSG_ERROR:
            MPTRACE("FFP_MSG_ERROR: %d", msg.arg1);
            post_event(env, weak_thiz, MEDIA_ERROR, MEDIA_ERROR_TX_PLAYER, msg.arg1);
            break;
        case FFP_MSG_PREPARED:
            MPTRACE("FFP_MSG_PREPARED:");
            post_event(env, weak_thiz, MEDIA_PREPARED, 0, 0);
            break;
        case FFP_MSG_COMPLETED:
            MPTRACE("FFP_MSG_COMPLETED:");
            post_event(env, weak_thiz, MEDIA_PLAYBACK_COMPLETE, 0, 0);
            break;
        case FFP_MSG_VIDEO_SIZE_CHANGED:
            MPTRACE("FFP_MSG_VIDEO_SIZE_CHANGED: %d, %d", msg.arg1, msg.arg2);
            post_event(env, weak_thiz, MEDIA_SET_VIDEO_SIZE, msg.arg1, msg.arg2);
            break;
        case FFP_MSG_SAR_CHANGED:
            MPTRACE("FFP_MSG_SAR_CHANGED: %d, %d", msg.arg1, msg.arg2);
            post_event(env, weak_thiz, MEDIA_SET_VIDEO_SAR, msg.arg1, msg.arg2);
            break;
        case FFP_MSG_BUFFERING_START:
            MPTRACE("FFP_MSG_BUFFERING_START:");
            post_event(env, weak_thiz, MEDIA_INFO, MEDIA_INFO_BUFFERING_START, 0);
            break;
        case FFP_MSG_BUFFERING_END:
            MPTRACE("FFP_MSG_BUFFERING_END:");
            post_event(env, weak_thiz, MEDIA_INFO, MEDIA_INFO_BUFFERING_END, 0);
            break;
        case FFP_MSG_BUFFERING_UPDATE:
            // MPTRACE("FFP_MSG_BUFFERING_UPDATE: %d, %d", msg.arg1, msg.arg2);
            post_event(env, weak_thiz, MEDIA_BUFFERING_UPDATE, msg.arg1, msg.arg2);
            break;
        case FFP_MSG_BUFFERING_BYTES_UPDATE:
            break;
        case FFP_MSG_BUFFERING_TIME_UPDATE:
            break;
        case FFP_MSG_SEEK_COMPLETE:
            MPTRACE("FFP_MSG_SEEK_COMPLETE:");
            post_event(env, weak_thiz, MEDIA_SEEK_COMPLETE, 0, 0);
            break;
        default:
            ALOGE("unknown FFP_MSG_xxx(%d)", msg.what);
            break;
        }
    }

    LABEL_RETURN:
    (*env)->DeleteGlobalRef(env, weak_thiz);
}

static int message_loop(void *arg)
{
    MPTRACE("message_loop");

    JNIEnv *env = NULL;
    (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);

    txMediaPlayer *mp = (txMediaPlayer*) arg;
    JNI_CHECK_GOTO(mp, env, NULL, "mpjni: native_message_loop: null mp", LABEL_RETURN);

    message_loop_n(env, mp);

    LABEL_RETURN:
    txmp_dec_ref_p(&mp);
    (*g_jvm)->DetachCurrentThread(g_jvm);

    MPTRACE("message_loop exit");
    return 0;
}

// ----------------------------------------------------------------------------

static JNINativeMethod g_methods[] = {
    {
        "_setDataSource",
        "(Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;)V",
        (void *) txMediaPlayer_setDataSourceAndHeaders
    },
    { "_setVideoSurface",   "(Landroid/view/Surface;)V", (void *) txMediaPlayer_setVideoSurface },
    { "_prepareAsync",      "()V",      (void *) txMediaPlayer_prepareAsync },
    { "_start",             "()V",      (void *) txMediaPlayer_start },
    { "_stop",              "()V",      (void *) txMediaPlayer_stop },
    { "seekTo",             "(J)V",     (void *) txMediaPlayer_seekTo },
    { "_pause",             "()V",      (void *) txMediaPlayer_pause },
    { "isPlaying",          "()Z",      (void *) txMediaPlayer_isPlaying },
    { "getCurrentPosition", "()J",      (void *) txMediaPlayer_getCurrentPosition },
    { "getDuration",        "()J",      (void *) txMediaPlayer_getDuration },
    { "_release",           "()V",      (void *) txMediaPlayer_release },
    { "_reset",             "()V",      (void *) txMediaPlayer_reset },
    { "setVolume",          "(FF)V",    (void *) txMediaPlayer_setVolume },
    { "native_init",        "()V",      (void *) txMediaPlayer_native_init },
    { "native_setup",       "(Ljava/lang/Object;)V", (void *) txMediaPlayer_native_setup },
    { "native_finalize",    "()V",      (void *) txMediaPlayer_native_finalize },

    { "_setAvFormatOption", "(Ljava/lang/String;Ljava/lang/String;)V", (void *) txMediaPlayer_setAvFormatOption },
    { "_setAvCodecOption",  "(Ljava/lang/String;Ljava/lang/String;)V", (void *) txMediaPlayer_setAvCodecOption },
    { "_setOverlayFormat",  "(I)V",                                    (void *) txMediaPlayer_setOverlayFormat },
    { "_photoImage",  "(Ljava/lang/String;)V",       (void *) txMediaPlayer_photoImage },
    { "_startRecordingRtspStream",  "(Ljava/lang/String;Ljava/lang/String;I)V",       (void *) txMediaPlayer_startRecordingRtsp },
    { "_stopRecordingRtspStream",  "()V",       (void *) txMediaPlayer_stopRecordingRtsp },
};

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    JNIEnv* env = NULL;
    jclass clazz;

    g_jvm = vm;
    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    assert(env != NULL);

    pthread_mutex_init(&g_clazz.mutex, NULL);

    clazz = (*env)->FindClass(env, JNI_CLASS_IJKPLAYER);
    TX_CHECK_RET(clazz, -1, "missing %s", JNI_CLASS_IJKPLAYER);

    // FindClass returns LocalReference
    g_clazz.clazz = (*env)->NewGlobalRef(env, clazz);
    TX_CHECK_RET(g_clazz.clazz, -1, "%s NewGlobalRef failed", JNI_CLASS_IJKPLAYER);
    (*env)->DeleteLocalRef(env, clazz);

    (*env)->RegisterNatives(env, g_clazz.clazz, g_methods, NELEM(g_methods));

    g_clazz.mNativeMediaPlayer = (*env)->GetFieldID(env, g_clazz.clazz, "mNativeMediaPlayer", "J");
    TX_CHECK_RET(g_clazz.mNativeMediaPlayer, -1, "missing mNativeMediaPlayer");

    g_clazz.postEventFromNative = (*env)->GetStaticMethodID(env, g_clazz.clazz, "postEventFromNative", "(Ljava/lang/Object;IIILjava/lang/Object;)V");
    TX_CHECK_RET(g_clazz.postEventFromNative, -1, "missing postEventFromNative");

    g_clazz.onControlResolveSegmentCount = (*env)->GetStaticMethodID(env, g_clazz.clazz, "onControlResolveSegmentCount", "(Ljava/lang/Object;)I");
    TX_CHECK_RET(g_clazz.onControlResolveSegmentCount, -1, "missing onControlResolveSegmentCount");

    g_clazz.onControlResolveSegmentDuration = (*env)->GetStaticMethodID(env, g_clazz.clazz, "onControlResolveSegmentDuration", "(Ljava/lang/Object;I)I");
    TX_CHECK_RET(g_clazz.onControlResolveSegmentDuration, -1, "missing onControlResolveSegmentDuration");

    g_clazz.onControlResolveSegmentUrl = (*env)->GetStaticMethodID(env, g_clazz.clazz, "onControlResolveSegmentUrl", "(Ljava/lang/Object;I)Ljava/lang/String;");
    TX_CHECK_RET(g_clazz.onControlResolveSegmentUrl, -1, "missing onControlResolveSegmentUrl");

    g_clazz.onControlResolveSegmentOfflineMrl = (*env)->GetStaticMethodID(env, g_clazz.clazz, "onControlResolveSegmentOfflineMrl", "(Ljava/lang/Object;I)Ljava/lang/String;");
    TX_CHECK_RET(g_clazz.onControlResolveSegmentUrl, -1, "missing onControlResolveSegmentOfflineMrl");

    txadk_global_init(env);
    txmp_global_init();

    return JNI_VERSION_1_4;
}

JNIEXPORT void JNI_OnUnload(JavaVM *jvm, void *reserved)
{
    txmp_global_uninit();

    pthread_mutex_destroy(&g_clazz.mutex);
}

#include <android/log.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <jni.h>
#include <assert.h>
#include "loghelp.h"
/* #include "tx_recorder_jni_define.h" */
#include "ff_recorder.h"

#define JNIREG_CLASS "com/taixin/ffmpeg/player/TXFFMpegRecorder"
/* 开始录制函数 */
static void _startRecordingRtsp(JNIEnv *env, jobject thiz,jstring rtsp_stream,  jstring filename, jint time)
{
    ALOGE("=====txplayer_jni.c startRecordingRtsp filename = %s\n", (*env)->GetStringUTFChars(env, filename, NULL));
 
    start_recording_rtsp_stream((*env)->GetStringUTFChars(env, rtsp_stream, NULL), (*env)->GetStringUTFChars(env, filename, NULL), (int)time);
}

/* 停止录制函数 */
static void _stopRecordingRtsp(JNIEnv *env, jobject thiz)
{
    stop_recording_rtsp_stream();
}

/* 虚拟机 */
static JNINativeMethod gMethods[] = {

    {"_startRecoderRTSPStream", "(Ljava/lang/String;Ljava/lang/String;I)V", (void*)_startRecordingRtsp},
    {"_stopRecordRTSPStream", "()V", (void*)_stopRecordingRtsp},

};

/*
* Register several native methods for one class.
*将此组件提供的各个本地函数(Native Function)登记到VM里，以便能加快后续呼叫本地函数的效率
*/
static int registerNativeMethods(JNIEnv* env, const char* className,
        JNINativeMethod* gMethods, int numMethods)
{
        jclass clazz;
        clazz = (*env)->FindClass(env, className);
        if (clazz == NULL) {
                return JNI_FALSE;
        }
        if ((*env)->RegisterNatives(env, clazz, gMethods, numMethods) < 0) {
                return JNI_FALSE;
        }

        return JNI_TRUE;
}


/*
* Register native methods for all classes we know about.
*/
static int registerNatives(JNIEnv* env)
{
        if (!registerNativeMethods(env, JNIREG_CLASS, gMethods, 
                                 sizeof(gMethods) / sizeof(gMethods[0])))
                return JNI_FALSE;

        return JNI_TRUE;
}

/*
* Set some test stuff up.
*
* Returns the JNI version on success, -1 on failure.
*该方法是在android vm调用System.loadLibrary方法时，就立即调用该方法
* 该函数做两件事，第一：注册所有的方法，第二：确认JNI的版本
*/
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
        JNIEnv* env = NULL;
        jint result = -1;

        if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
                return -1;
        }
        assert(env != NULL);

        if (!registerNatives(env)) {//注册
                return -1;
        }
        /* success -- return valid version number */
        result = JNI_VERSION_1_4;

        return result;
}

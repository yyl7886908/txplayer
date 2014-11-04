#include "ff_recorder.h"

#if defined(__ANDROID__)
#define printf(...) ALOGD(__VA_ARGS__)
#endif

/********************************
开始录制接口
参数：1视频流地址 rtsp_stream, 2存储的文件地址 filename

* * ********************/
void start_recording_rtsp_stream(char *rtsp_stream, char *filename)
{
    ALOGI("ff_recorder.c start recording rtsp stream rtsp_stream = %s\n filename = %s\n",rtsp_stream, filename );
}


/* 停止录制的接口 */

void stop_recording_rtsp_stream()
{
     ALOGI("ff_recorder.c stop recording rtsp stream rtsp_stream \n" );
}

/* #include "./txplayer/ff_ffinc.h" */
#include <stdbool.h>
#include <assert.h>
#include "libavutil/avstring.h"
#include "libavutil/time.h"
#include "libavformat/avformat.h"
#include "libavcodec/avfft.h"
#include "libswscale/swscale.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h"


/********************************
开始录制接口
参数：1视频流地址 rtsp_stream, 2存储的文件地址 filename

* * ********************/
void start_recording_rtsp_stream(char *rtsp_stream, char *filename, int time);


/* 停止录制的接口 */

void stop_recording_rtsp_stream();

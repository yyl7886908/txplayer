#include "ff_recorder.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avio.h"
#include <sys/time.h>

#if defined(__ANDROID__)
#define printf(...) ALOGD(__VA_ARGS__)
#endif

#define CODEC_TYPE_VIDEO AVMEDIA_TYPE_VIDEO
#define CODEC_TYPE_AUDIO AVMEDIA_TYPE_AUDIO

static bool bStop = false;
static int record_time = -1;
static char rtsp_url[200];
static char out_file_name[200];

/* RTSP 录制为avi */
time_t get_time()
{
  struct timeval tv;

  gettimeofday( &tv, NULL );

  return tv.tv_sec; 
} 

void * rtsp2avi(void * args)
{
    AVFormatContext *ifcx = NULL;
    AVInputFormat *ifmt;
    AVCodecContext *iccx;
    AVCodec *icodec;
    AVStream *ist;
    int i_index;
    time_t timenow, timestart;
    int got_key_frame = 0;

    AVFormatContext *ofcx;
    AVOutputFormat *ofmt;
    AVCodecContext *occx;
    AVCodec *ocodec;
    AVStream *ost;
    int o_index;

    AVPacket pkt;

    int ix;

    const char *sFileInput ;
    const char *sFileOutput;

    int bRunTime ;
    sprintf(sFileInput, "%s", rtsp_url);
    sprintf(sFileOutput, "%s", out_file_name);
    bRunTime = record_time;
    // Initialize library
    av_log_set_level( AV_LOG_DEBUG );
    av_register_all();
    avcodec_register_all(); 
    avformat_network_init();

    //
    // Input
    //

    //open rtsp
    if ( avformat_open_input( &ifcx, sFileInput, NULL, NULL) != 0 ) {
        ALOGE( "ERROR: Cannot open input file\n" );
        return ;
    }

    if ( avformat_find_stream_info( ifcx, NULL ) < 0 ) {
        ALOGE( "ERROR: Cannot find stream info\n" );
        avformat_close_input( &ifcx );
        return ;
    }

    snprintf( ifcx->filename, sizeof( ifcx->filename ), "%s", sFileInput );

    //search video stream
    i_index = -1;
    for ( ix = 0; ix < ifcx->nb_streams; ix++ ) {
        iccx = ifcx->streams[ ix ]->codec;
        if ( iccx->codec_type == AVMEDIA_TYPE_VIDEO ) {
            ist = ifcx->streams[ ix ];
            i_index = ix;
            break;
        }
    }
    if ( i_index < 0 ) {
        ALOGE( "ERROR: Cannot find input video stream\n" );
        avformat_close_input( &ifcx );
        return ;
    }

    //
    // Output
    //

    //open output file
    ofmt = av_guess_format( NULL, sFileOutput, NULL );
    ofcx = avformat_alloc_context();
    ofcx->oformat = ofmt;
    ALOGI("before avio open2 1.avi!!!!\n");
    avio_open2( &ofcx->pb, sFileOutput, AVIO_FLAG_WRITE, NULL, NULL );

        // Create output stream
    //ost = avformat_new_stream( ofcx, (AVCodec *) iccx->codec );
    ost = avformat_new_stream( ofcx, NULL );
    avcodec_copy_context( ost->codec, iccx );
    

    ost->sample_aspect_ratio.num = iccx->sample_aspect_ratio.num;
    ost->sample_aspect_ratio.den = iccx->sample_aspect_ratio.den;

    // Assume r_frame_rate is accurate
    ost->r_frame_rate = ist->r_frame_rate;
    ost->avg_frame_rate = ost->r_frame_rate;
    ost->time_base = av_inv_q( ost->r_frame_rate );
    ost->codec->time_base = ost->time_base;

    avformat_write_header( ofcx, NULL );
    ALOGI("avformat write header !!!\n");
    snprintf( ofcx->filename, sizeof( ofcx->filename ), "%s", sFileOutput );

    //start reading packets from stream and write them to file

    av_dump_format( ifcx, 0, ifcx->filename, 0 );
    av_dump_format( ofcx, 0, ofcx->filename, 1 );

    timestart = timenow = get_time();

    ix = 0;
    //av_read_play(context);//play RTSP (Shouldn't need this since it defaults to playing on connect)
    av_init_packet( &pkt );
    
    /* if(!bStop) */
    /*     { */
            
    /*     } */
    while ( av_read_frame( ifcx, &pkt ) >= 0 && timenow - timestart <= bRunTime ) {
        if ( pkt.stream_index == i_index ) { //packet is video
            // Make sure we start on a key frame
            if ( timestart == timenow && ! ( pkt.flags & AV_PKT_FLAG_KEY ) ) {
                timestart = timenow = get_time();
                continue;
            }
            got_key_frame = 1;

            pkt.stream_index = ost->id;

            pkt.pts = ix++;
            pkt.dts = pkt.pts;

            av_interleaved_write_frame( ofcx, &pkt );
        }
        av_free_packet( &pkt );
        av_init_packet( &pkt );

        timenow = get_time();
    }
    av_read_pause( ifcx );
    av_write_trailer( ofcx );
    avio_close( ofcx->pb );
    avformat_free_context( ofcx );

    avformat_network_deinit();

    /* return EXIT_SUCCESS; */
}

/********************************
开始录制接口
参数：1视频流地址 rtsp_stream, 2存储的文件地址 filename

* * ********************/
void start_recording_rtsp_stream(char *rtsp_stream, char *filename)
{
    ALOGI("ff_recorder.c start recording rtsp stream ===\nrtsp_stream = %s\nfilename = %s\n",rtsp_stream, filename );
    bStop = false;
    memset(rtsp_url, 0,sizeof(rtsp_url));
    memset(out_file_name, 0, sizeof(out_file_name));
    sprintf(rtsp_url, "%s", rtsp_stream);
    sprintf(out_file_name, "%s", filename);
    pthread_t record_thread;

    int ret=pthread_create(&record_thread,NULL,(void*)rtsp2avi ,NULL);
    /* rtsp2mp4(); */
    if(ret)
        {
            ALOGE("create record thread error!!!!!!!\n");
            return;
        }
    ALOGI("create record thread success!!!!\n");
}


/* 停止录制的接口 */

void stop_recording_rtsp_stream()
{
     ALOGI("ff_recorder.c stop recording rtsp stream rtsp_stream \n" );
}

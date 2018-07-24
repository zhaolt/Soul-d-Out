//
// Created by ZhaoLiangtai on 2018/7/7.
//

/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

#include "common.h"
#include "libswresample/swresample.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
#include "queue.h"

typedef  struct _VideoDec {
    // 视频上下文
    AVCodecContext *pVCodecCtx;
    // 音频上下文
    AVCodecContext *pACodecCtx;
    AVFrame *pFrame, *pFrameYUV, *pAFrame;
    struct SwsContext *pSwsCtx;
    // 视频解码器
    AVCodec *pVCodec;
    // 音频解码器
    AVCodec *pACodec;
    AVPacket packet;
    // 视频流
    AVStream *pVStream;
    // 音频流
    AVStream *pAStream;
    AVFormatContext *pFormatCtx;
    AVDictionaryEntry *pTag;
    SwrContext *pSwrCtx;
} VideoDec;
VideoDec *pDecoder;
int video_index, audio_index;
uint8_t * out_buffer;
queue_t* pVideoQueue;
queue_t* pAudioQueue;
int packet_num;
double audio_clock;

struct Decode_Url_Para {
    JNIEnv *env;
    jobject jobj;
    jstring url;
};

int init_264decoder();
int decode264(uint8_t* h264_data, int data_len, uint8_t* yuv420);
void init_decoder(char* url);
void decode();
int decode_url(JNIEnv *env, jobject jobj, jstring url);
void sendData2Java(uint8_t *deData, int dataLen, JNIEnv *env, jobject jobj);

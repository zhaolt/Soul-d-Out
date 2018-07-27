//
// Created by ZhaoLiangtai on 2018/7/26.
//

#ifndef SOUL_D_OUT_P_PLAYER_H
#define SOUL_D_OUT_P_PLAYER_H


#define AV_SYNC_THRESHOLD_MIN 0.04
#define AV_SYNC_THRESHOLD_MAX 0.1
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
#define AV_NOSYNC_THRESHOLD 10.0

#include "common.h"
#include "libswresample/swresample.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
#include "queue.h"
typedef struct _VideoDec {
    AVCodecContext *pCodecCtx;
    AVFormatContext *pFormatCtx;
    AVCodec *pCodec;
    AVFrame *pFrame;
    struct AVPacket packet;
    struct SwsContext *pSwsCtx;
} VideoDec;

typedef struct _Decoder {
    // 视频上下文
    AVCodecContext *pVCodecCtx;
    // 音频上下文
    AVCodecContext *pACodecCtx;
    struct SwsContext *pSwsCtx;
    AVCodec *pVCodec;
    AVCodec *pACodec;
    AVStream *pVStream;
    AVStream *pAStream;
    struct AVPacket packet;
    AVFormatContext *pFormatCtx;
    SwrContext *pSwrCtx;
} Decoder;

int video_index, audio_index;
int decoder_init_flag;
double audio_clock;
Decoder *pDecoder;
uint8_t *out_buffer;


int init_decoder(char *url);
void decode();
uint8_t *de_que_video();
uint8_t *de_que_audio();
/**
 * 只解码视频,不按视频标准时间解码
 * @param env
 * @param jobj
 * @param url
 * @return
 */
int decode_url(JNIEnv *env, jobject jobj, jstring url);

#endif //SOUL_D_OUT_P_PLAYER_H

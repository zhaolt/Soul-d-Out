//
// Created by ZhaoLiangtai on 2018/7/7.
//

#include "common.h"

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"

int init_avdecoder();
int decode264(uint8_t* h264_data, int data_len, uint8_t* yuv420);
int decodeUrl(JNIEnv* env, jobject jobj, jstring url);
//
// Created by ZhaoLiangtai on 2018/7/7.
//

#include "common.h"

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"

int init_264decoder();
int decode264(uint8_t* h264_data, int data_len, uint8_t* yuv420);
int decode_url(JNIEnv* env, jobject jobj, jstring url);
void sendData2Java(uint8_t *deData, int dataLen, JNIEnv *env, jobject jobj);
//
// Created by ZhaoLiangtai on 2018/7/26.
//

#ifndef SOUL_D_OUT_OPENSLES_H_H
#define SOUL_D_OUT_OPENSLES_H_H

#include "common.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <pthread.h>
#include <assert.h>

void createAudioEngine();
void createBufferQueueAudioPlayer(int sampleRate, int channel);
void audioStart();
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context);
void releaseResampleBuf(void);
#endif //SOUL_D_OUT_OPENSLES_H_H

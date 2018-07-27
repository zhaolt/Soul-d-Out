//
// Created by ZhaoLiangtai on 2018/7/7.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#ifdef ANDROID
#include <jni.h>
#include <android/log.h>
#define LOGV(format, ...) __android_log_print(ANDROID_LOG_VERBOSE, "movei_tools", format, ##__VA_ARGS__)
#define LOGD(format, ...)  __android_log_print(ANDROID_LOG_DEBUG,  "movie_tools", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "movie_tools", format, ##__VA_ARGS__)
#define LOGW(format, ...)  __android_log_print(ANDROID_LOG_WARN,  "movie_tools", format, ##__VA_ARGS__)
#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR,  "movie_tools", format, ##__VA_ARGS__)
#else
#define LOGV(format, ...) printf("movie_tools" format "\n", ##__VA_ARGS__)
#define LOGD(format, ...) printf("movie_tools" format "\n", ##__VA_ARGS__)
#define LOGI(format, ...) printf("movie_tools" format "\n", ##__VA_ARGS__)
#define LOGW(format, ...) printf("movie_tools" format "\n", ##__VA_ARGS__)
#define LOGE(format, ...) printf("movie_tools" format "\n", ##__VA_ARGS__)
#endif

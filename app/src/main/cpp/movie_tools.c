#include "p_player.h"

jint Java_com_jesse_soulout_util_VideoUtils_getVideoRotation(JNIEnv *env, jclass jobj,
                                                             jstring filePath) {
    char *file_path = (*env)->GetStringUTFChars(env, filePath, NULL);
    LOGI("file_path: %s", file_path);
    // 1.初始化
    avcodec_register_all();
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    // 2.打开文件
    if (avformat_open_input(&pFormatCtx, file_path, NULL, NULL) != 0) {
        LOGE("%s", "无法打开视频文件");
        return -1;
    }
    // 3.
    if (avformat_find_stream_info(pFormatCtx, NULL) != 0) {
        LOGE("%s", "无法获取视频文件信息");
        return -2;
    }
    // 4.寻找视频流
    int i = 0, v_stream_idx = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            v_stream_idx = i;
            break;
        }
    }
    if (v_stream_idx == -1) {
        LOGE("%s", "无法获取视频流");
        return -3;
    }
    // 5.读取角度信息
    AVDictionaryEntry *tag = NULL;
    tag = av_dict_get(pFormatCtx->streams[v_stream_idx]->metadata, "rotate", tag, 0);
    int angle = -1;
    if (tag) {
        angle = atoi(tag->value);
        LOGI("angle:%d", angle);
    }
    avformat_free_context(pFormatCtx);
    return angle;
}

void Java_com_jesse_soulout_codec_VideoCodec_decodeVideo(JNIEnv *env, jobject jobj, jstring url) {
    decode_url(env, jobj, url);
}

void Java_com_jesse_soulout_codec_VideoCodec_initDecoder(JNIEnv *env, jobject jobj, jstring jUrl) {
//    char* url = (*env)->GetStringUTFChars(env, jUrl, NULL);
//    init_decoder(url);
}

void Java_com_jesse_soulout_codec_VideoCodec_startDecode(JNIEnv *env, jobject jobj) {
//    decode();
}

/**
 * 这个方法需要java层开线程读取队列视频数据
 * @param env
 * @param jobj
 * @param outBuffer
 */
jbyteArray Java_com_jesse_soulout_codec_VideoCodec_getVideoRealData(JNIEnv *env, jobject jobj) {
//    AVFrame *pFrame = de_queue_pic();
//    if (pFrame != NULL) {
//        char pictype_str[10] = {0};
//        int dataLen = pFrame->width * pFrame->height +
//                      (pFrame->width * pFrame->height / 2);
//        uint8_t *out_data = malloc(dataLen);
//        LOGD("get video data linesize[0] = %d, linesize[1] = %d, linesize[2] = %d\n", pFrame->linesize[0], pFrame->linesize[1], pFrame->linesize[2]);
//        pgm_save2(pFrame->data[0], pFrame->linesize[0], pFrame->width, pFrame->height, out_data);
//        pgm_save2(pFrame->data[1], pFrame->linesize[1], pFrame->width / 2, pFrame->height / 2,
//                  out_data + pFrame->width * pFrame->height);
//        pgm_save2(pFrame->data[2], pFrame->linesize[2], pFrame->width / 2, pFrame->height / 2,
//                  out_data + pFrame->width * pFrame->height * 5 / 4);
//        switch (pFrame->pict_type) {
//            case AV_PICTURE_TYPE_I:
//                sprintf(pictype_str, "I");
//                break;
//            case AV_PICTURE_TYPE_P:
//                sprintf(pictype_str, "P");
//                break;
//            case AV_PICTURE_TYPE_B:
//                sprintf(pictype_str, "B");
//                break;
//            default:
//                sprintf(pictype_str, "Other");
//                break;
//        }
//        jbyteArray data = (*env)->NewByteArray(env, dataLen);
//        (*env)->SetByteArrayRegion(env, data, 0, dataLen, out_data);
//        return data;
//    } else {
//        return NULL;
//    }
}

void Java_com_jesse_soulout_codec_VideoCodec_startAudioTask(JNIEnv *env, jobject jobj) {
//    startAudioThread();
}
//
// Created by ZhaoLiangtai on 2018/7/26.
//

#include "p_player.h"

queue_t *pVideoQueue;
queue_t *pAudioQueue;
int video_frame_num, audio_frame_num;
uint64_t pts_flags;

void sendData2Java(uint8_t *deData, int dataLen, JNIEnv *env, jobject jobj) {
    jclass jclazz = (*env)->GetObjectClass(env, jobj);
    jmethodID jmethodid = (*env)->GetMethodID(env, jclazz, "sendData2Java", "([B)V");
    jbyte *jbyteDeData = (jbyte *) deData;
    jbyteArray outData = (*env)->NewByteArray(env, dataLen);
    (*env)->SetByteArrayRegion(env, outData, 0, dataLen, jbyteDeData);
    (*env)->CallVoidMethod(env, jobj, jmethodid, outData);
    (*env)->DeleteLocalRef(env, outData);
    (*env)->DeleteLocalRef(env, jclazz);
}

void pgm_save2(unsigned char *buf, int wrap, int xsize, int ysize, uint8_t *pDataOut) {
    int i;
    for (i = 0; i < ysize; i++) {
        memcpy(pDataOut + i * xsize, buf + i * wrap, xsize);
    }
}

void release_decoder() {
    sws_freeContext(pDecoder->pSwsCtx);
    if (pDecoder->pSwrCtx != NULL) {
        swr_free(pDecoder->pSwrCtx);
    }
    avcodec_close(pDecoder->pVCodecCtx);
    if (pDecoder->pACodecCtx != NULL) {
        avcodec_close(pDecoder->pACodecCtx);
    }
    av_free(pDecoder->pVCodecCtx);
    if (pDecoder->pACodec != NULL) {
        av_free(pDecoder->pACodecCtx);
    }
    avformat_network_deinit();
}

void en_queue(int index, AVFrame *frame) {
    if (index == video_index) {
        item_t *item = malloc(sizeof(item_t));
        item->key = video_frame_num;
        item->statellite = frame;
        deque_tail_insert(pVideoQueue, *item);
        video_frame_num++;
    } else if (index == audio_index) {
        item_t *item = malloc(sizeof(item_t));
        item->key = audio_frame_num;
        item->statellite = frame;
        deque_tail_insert(pAudioQueue, *item);
        audio_frame_num++;
    }
}

void decode_video(AVPacket *pPacket) {
    AVFrame *pFrame = av_frame_alloc();
    av_image_fill_arrays(pFrame->data, pFrame->linesize, out_buffer, AV_PIX_FMT_YUV420P,
                         pDecoder->pVCodecCtx->width, pDecoder->pVCodecCtx->height, 1);
    int got_pic;
    int ret = avcodec_decode_video2(pDecoder->pVCodecCtx, pFrame, &got_pic, pPacket);
    if (ret < 0) {
        LOGE("Decode Error.\n");
        return;
    }
    pts_flags = pPacket->pts;
    if (got_pic) {
        en_queue(pPacket->stream_index, pFrame);
    }
    av_packet_unref(&(pDecoder->packet));
}

void decode_audio(AVPacket *pPacket) {
    AVFrame *pFrame = av_frame_alloc();
    avcodec_send_packet(pDecoder->pACodecCtx, pPacket);
    int ret = avcodec_receive_frame(pDecoder->pACodecCtx, pFrame);
    if (!ret) {
        en_queue(pPacket->stream_index, pFrame);
    }
    av_packet_unref(&(pDecoder->packet));
}

int init_decoder(char *url) {
    int i, v_ret, a_ret;
    if (decoder_init_flag) {
        release_decoder();
    }
    pDecoder = av_mallocz(sizeof(Decoder));
    pVideoQueue = malloc(sizeof(queue_t));
    pAudioQueue = malloc(sizeof(queue_t));
    queue_init(pVideoQueue, 1024);
    queue_init(pAudioQueue, 1024);
    pDecoder->pVCodec = NULL;
    pDecoder->pACodec = NULL;
    pDecoder->pSwsCtx = NULL;
    pDecoder->pSwrCtx = NULL;
    video_index = -1;
    audio_index = -1;
    avcodec_register_all();
    avformat_network_init();
    pDecoder->pFormatCtx = avformat_alloc_context();
    v_ret = avformat_open_input(&(pDecoder->pFormatCtx), url, NULL, NULL);
    if (v_ret != 0) {
        LOGE("Couldn't open input stream.\n");
        return -1;
    }
    v_ret = avformat_find_stream_info(pDecoder->pFormatCtx, NULL);
    if (v_ret < 0) {
        LOGE("Couldn't find stream information\n");
        return -1;
    }
    // 分别找到音频流和视频流
    for (i = 0; i < pDecoder->pFormatCtx->nb_streams; i++) {
        if (pDecoder->pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_index = i;
        }
        if (pDecoder->pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_index = i;
        }
    }
    if (video_index >= 0) {
        // 拿到视频流
        pDecoder->pVStream = pDecoder->pFormatCtx->streams[video_index];
        // 拿到视频上下文
        pDecoder->pVCodecCtx = pDecoder->pFormatCtx->streams[video_index]->codec;
        // 根据解码器id拿到视频解码器
        pDecoder->pVCodec = avcodec_find_decoder(pDecoder->pVCodecCtx->codec_id);
        // 打开解码器
        v_ret = avcodec_open2(pDecoder->pVCodecCtx, pDecoder->pVCodec, NULL);
        if (v_ret < 0) {
            LOGE("Could't open video codec.\n");
            return -1;
        }
        int data_len = pDecoder->pVCodecCtx->width * pDecoder->pVCodecCtx->height
                       + pDecoder->pVCodecCtx->width * pDecoder->pVCodecCtx->height / 2;
        out_buffer = malloc(sizeof(data_len));
        pDecoder->pSwsCtx = sws_getContext(pDecoder->pVCodecCtx->width,
                                           pDecoder->pVCodecCtx->height,   // 原始宽高
                                           pDecoder->pVCodecCtx->pix_fmt,  // 原始像素格式
                                           pDecoder->pVCodecCtx->width,    // 目标宽高
                                           pDecoder->pVCodecCtx->height,
                                           AV_PIX_FMT_YUV420P,             // 目标数据格式
                                           SWS_BICUBIC,
                                           NULL,
                                           NULL,
                                           NULL);
        decoder_init_flag = 1;
    } else {
        return -1;
    }
    if (audio_index >= 0) {
        pDecoder->pAStream = pDecoder->pFormatCtx->streams[audio_index];
        pDecoder->pACodecCtx = pDecoder->pFormatCtx->streams[video_index]->codec;
        pDecoder->pACodec = avcodec_find_decoder(pDecoder->pACodecCtx->codec_id);
        a_ret = avcodec_open2(pDecoder->pACodecCtx, pDecoder->pACodec, NULL);
        if (a_ret < 0) {
            LOGW("Could't open audio codec.\n");
        }
    }
    return v_ret;
}

void decode() {
    int ret;
    av_init_packet(&(pDecoder->packet));
    while (ret = av_read_frame(pDecoder->pFormatCtx, &(pDecoder->packet)) >= 0) {
        if (pDecoder->packet.stream_index == video_index) {
            decode_video(&(pDecoder->packet));
        } else if (pDecoder->packet.stream_index == audio_index) {
            decode_audio(&(pDecoder->packet));
        }
    }
}

uint8_t *de_que_video() {
    double timestamp;
    item_t *item;
    deque_head_delete(pVideoQueue, item);
    AVFrame *pFrame = item->statellite;
    if (NULL == pFrame) {
        return NULL;
    }
    if (pts_flags == AV_NOPTS_VALUE) {
        timestamp = 0;
    } else {
        timestamp = av_frame_get_best_effort_timestamp(pFrame)
                    * av_q2d(pDecoder->pVStream->time_base);
    }
    double frame_rate = av_q2d(pDecoder->pVStream->avg_frame_rate);
    frame_rate += pFrame->repeat_pict * (frame_rate * 0.5);
    if (timestamp == 0.0) {
        usleep((unsigned long)frame_rate * 1000);
    } else {
        if (fabs(timestamp - audio_clock) > AV_SYNC_THRESHOLD_MIN &&
            fabs(timestamp - audio_clock) < AV_NOSYNC_THRESHOLD) {
            if (timestamp > audio_clock) {
                usleep((unsigned long) ((timestamp - audio_clock) * 1000000));
            }
        }
    }

    pgm_save2(pFrame->data[0], pFrame->linesize[0], pFrame->width, pFrame->height, out_buffer);
    pgm_save2(pFrame->data[1], pFrame->linesize[1], pFrame->width / 2, pFrame->height / 2,
              out_buffer + pFrame->width * pFrame->height);
    pgm_save2(pFrame->data[2], pFrame->linesize[2], pFrame->width / 2, pFrame->height / 2,
              out_buffer + pFrame->width * pFrame->height * 5 / 4);
    av_free(pFrame);
    free(item);
    return out_buffer;
}

uint8_t *de_que_audio() {
    return NULL;
}


int decode_url(JNIEnv *env, jobject jobj, jstring url) {
    int i, video_index;
    int ret, got_pic;
    int frame_cnt;
    char input_str[500] = {0};
    char info[500] = {0};
    sprintf(input_str, "%s", (*env)->GetStringUTFChars(env, url, NULL));
    avcodec_register_all();
    avformat_network_init();
    VideoDec *pDec = (VideoDec *) av_mallocz(sizeof(VideoDec));
    pDec->pFormatCtx = avformat_alloc_context();
    LOGD("input_str = %s\n", input_str);
    if (avformat_open_input(&(pDec->pFormatCtx), input_str, NULL, NULL) != 0) {
        LOGE("Couldn't open input stream.\n");
        return -1;
    }
    if (avformat_find_stream_info(pDec->pFormatCtx, NULL) < 0) {
        LOGE("Couldn't find stream information\n");
        return -1;
    }

    video_index = -1;
    for (i = 0; i < pDec->pFormatCtx->nb_streams; i++) {
        if (pDec->pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_index = i;
            break;
        }
    }
    if (video_index == -1) {
        LOGE("Couldn't find a video stream.\n");
        return -1;
    }
    pDec->pCodecCtx = pDec->pFormatCtx->streams[video_index]->codec;
    pDec->pCodec = avcodec_find_decoder(pDec->pCodecCtx->codec_id);
    if (pDec->pCodec == NULL) {
        LOGE("Couldn't find Codec.\n");
        return -1;
    }
    if (avcodec_open2(pDec->pCodecCtx, pDec->pCodec, NULL) < 0) {
        LOGE("Couldn't open codec.\n");
        return -1;
    }
    int dataLen = pDec->pCodecCtx->width * pDec->pCodecCtx->height +
                  (pDec->pCodecCtx->width * pDec->pCodecCtx->height / 2);
    uint8_t *deData = malloc(dataLen);
    pDec->pFrame = av_frame_alloc();
    av_image_fill_arrays(pDec->pFrame->data, pDec->pFrame->linesize, deData,
                         AV_PIX_FMT_YUV420P, pDec->pCodecCtx->width, pDec->pCodecCtx->height, 1);

    av_init_packet(&(pDec->packet));

    pDec->pSwsCtx = sws_getContext(pDec->pCodecCtx->width, pDec->pCodecCtx->height,
                                   pDec->pCodecCtx->pix_fmt, pDec->pCodecCtx->width,
                                   pDec->pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC,
                                   NULL, NULL, NULL);

    sprintf(info, "[Input     ]%s\n", url);
    sprintf(info, "%s[Format    ]%s\n", info, pDec->pFormatCtx->iformat->name);
    sprintf(info, "%s[Codec     ]%s\n", info, pDec->pCodecCtx->codec->name);
    sprintf(info, "%s[Resolution]%dx%d\n", info, pDec->pCodecCtx->width, pDec->pCodecCtx->height);
    frame_cnt = 0;

    while (av_read_frame(pDec->pFormatCtx, &(pDec->packet)) >= 0) {
        if (pDec->packet.stream_index == video_index) {
            ret = avcodec_decode_video2(pDec->pCodecCtx, pDec->pFrame, &got_pic,
                                        &(pDec->packet));
            if (ret < 0) {
                LOGE("Decode Error.\n");
                return -1;
            }
            if (got_pic) {
                char pictype_str[10] = {0};
                pgm_save2(pDec->pFrame->data[0],
                          pDec->pFrame->linesize[0], pDec->pFrame->width, pDec->pFrame->height,
                          deData);
                pgm_save2(pDec->pFrame->data[1],
                          pDec->pFrame->linesize[1],
                          pDec->pFrame->width / 2,
                          pDec->pFrame->height / 2,
                          deData + pDec->pFrame->width * pDec->pFrame->height);
                pgm_save2(pDec->pFrame->data[2],
                          pDec->pFrame->linesize[2],
                          pDec->pFrame->width / 2,
                          pDec->pFrame->height / 2,
                          deData + pDec->pFrame->width * pDec->pFrame->height * 5 / 4);

                switch (pDec->pFrame->pict_type) {
                    case AV_PICTURE_TYPE_I:
                        sprintf(pictype_str, "I");
                        break;
                    case AV_PICTURE_TYPE_P:
                        sprintf(pictype_str, "P");
                        break;
                    case AV_PICTURE_TYPE_B:
                        sprintf(pictype_str, "B");
                        break;
                    default:
                        sprintf(pictype_str, "Other");
                        break;
                }
                LOGI("Frame Index: %5d. Type:%s", frame_cnt, pictype_str);
                sendData2Java(deData, dataLen, env, jobj);
                frame_cnt++;
            }
        } else {
            LOGI("可能是音频");
        }
    }
    free(deData);
    return 1;
}



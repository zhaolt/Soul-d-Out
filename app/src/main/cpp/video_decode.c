//
// Created by ZhaoLiangtai on 2018/7/7.
//
#include "video_decode.h"

int init_flag = 0;
void en_queue(AVPacket *packet);

void pgm_save2(unsigned char *buf, int wrap, int xsize, int ysize, uint8_t *pDataOut) {
    int i;
    for (i = 0; i < ysize; i++) {
        memcpy(pDataOut + i * xsize, buf + i * wrap, xsize);
    }
}

int init_264decoder() {
    if (init_flag == 1) {
        sws_freeContext(pDecoder->pSwsCtx);
        av_packet_unref(&(pDecoder->packet));
        av_free(pDecoder->pFrame);
        avcodec_close(pDecoder->pVCodecCtx);
        av_free(pDecoder->pVCodecCtx);
    }
    pDecoder = av_mallocz(sizeof(VideoDec));
    pDecoder->pVCodec = NULL;
    pDecoder->pSwsCtx = NULL;
    avcodec_register_all();
    pDecoder->pFrame = av_frame_alloc();
    av_init_packet(&(pDecoder->packet));
    pDecoder->pVCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (NULL != pDecoder->pVCodec) {
        pDecoder->pVCodecCtx = avcodec_alloc_context3(pDecoder->pVCodec);
        pDecoder->pVCodecCtx->flags |= AV_CODEC_FLAG_LOW_DELAY;
        if (avcodec_open2(pDecoder->pVCodecCtx, pDecoder->pVCodec, NULL) >= 0) {
            init_flag = 1;
            return 1;
        }
    } else {
        return 0;
    }
}

int decode264(uint8_t *h264_data, int data_len, uint8_t *yuv420) {
    int send_ret = 0;
    int receive_ret = 0;
    int got_pic;
    if (!pDecoder) {
        return -1;
    }
    pDecoder->packet.data = h264_data;
    pDecoder->packet.size = data_len;
//    ret = avcodec_decode_video2(decoder->pCodecCtx, decoder->pFrame, &got_pic, &(decoder->packet));
    // 返回0表示成功 new API
    send_ret = avcodec_send_packet(pDecoder->pVCodecCtx, &(pDecoder->packet));
    if (send_ret != 0) {
        return -1;
    }
    while (avcodec_receive_frame(pDecoder->pVCodecCtx, pDecoder->pFrame) == 0) {
        // y数据
        pgm_save2(pDecoder->pFrame->data[0],
                  pDecoder->pFrame->linesize[0],
                  pDecoder->pFrame->width,
                  pDecoder->pFrame->height, yuv420);
        // u数据
        pgm_save2(pDecoder->pFrame->data[1],
                  pDecoder->pFrame->linesize[1],
                  pDecoder->pFrame->width / 2,
                  pDecoder->pFrame->height / 2,
                  yuv420 + pDecoder->pFrame->width * pDecoder->pFrame->height);
        // v数据
        pgm_save2(pDecoder->pFrame->data[2],
                  pDecoder->pFrame->linesize[2],
                  pDecoder->pFrame->width / 2,
                  pDecoder->pFrame->height / 2,
                  yuv420 + pDecoder->pFrame->width * pDecoder->pFrame->height * 5 / 4);
        receive_ret = 1;
    }
    return receive_ret;
}

/**
 * 初始化解码器
 * @param url
 */
void init_decoder(char *url) {
    int i;
    int ret;
    pDecoder = NULL;
    video_index = -1;
    audio_index = -1;
    pVideoQueue = malloc(sizeof(queue_t));
    queue_init(pVideoQueue, 1024);
    pAudioQueue = malloc(sizeof(queue_t));
    queue_init(pAudioQueue, 1024);
    avcodec_register_all();
    avformat_network_init();
    pDecoder = av_mallocz(sizeof(VideoDec));
    pDecoder->pFormatCtx = avformat_alloc_context();
    ret = avformat_open_input(&(pDecoder->pFormatCtx), url, NULL, NULL);
    if (ret != 0) {
        LOGE("Couldn't open input stream.\n");
        return;
    }
    ret = avformat_find_stream_info(pDecoder->pFormatCtx, NULL);
    if (ret < 0) {
        LOGE("Couldn't find stream information\n");
        return;
    }
    // 分辨找到音频流和视频流
    for (i = 0; i < pDecoder->pFormatCtx->nb_streams; i++) {
        if (pDecoder->pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_index = i;
        }
        if (pDecoder->pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_index = i;
        }
    }
    // 初始化视频解码器
    if (video_index >= 0) {
        pDecoder->pVStream = pDecoder->pFormatCtx->streams[video_index];
        pDecoder->pVCodecCtx = pDecoder->pFormatCtx->streams[video_index]->codec;
        pDecoder->pVCodec = avcodec_find_decoder(pDecoder->pVCodecCtx->codec_id);
        ret = avcodec_open2(pDecoder->pVCodecCtx, pDecoder->pVCodec, NULL);
        if (ret < 0) {
            LOGE("Couldn't open video codec.\n");
            return;
        }
        int dataLen = pDecoder->pVCodecCtx->width * pDecoder->pVCodecCtx->height +
                      (pDecoder->pVCodecCtx->width * pDecoder->pVCodecCtx->height / 2);
        out_buffer = malloc(dataLen);
        pDecoder->pFrame = av_frame_alloc();
        pDecoder->pFrameYUV = av_frame_alloc();
        av_image_fill_arrays(pDecoder->pFrameYUV->data, pDecoder->pFrameYUV->linesize, out_buffer,
                             AV_PIX_FMT_YUV420P, pDecoder->pVCodecCtx->width,
                             pDecoder->pVCodecCtx->height, 1);
        pDecoder->pSwsCtx = sws_getContext(pDecoder->pVCodecCtx->width,
                                           pDecoder->pVCodecCtx->height,
                                           pDecoder->pVCodecCtx->pix_fmt,
                                           pDecoder->pVCodecCtx->width,
                                           pDecoder->pVCodecCtx->height, AV_PIX_FMT_YUV420P,
                                           SWS_BICUBIC,
                                           NULL, NULL, NULL);
    } else {
        LOGE("Couldn't find video stream.\n");
        return;
    }
    // 初始化音频解码器
    if (audio_index >= 0) {
        pDecoder->pAStream = pDecoder->pFormatCtx->streams[audio_index];
        pDecoder->pACodecCtx = pDecoder->pFormatCtx->streams[audio_index]->codec;
        pDecoder->pACodec = avcodec_find_decoder(pDecoder->pACodecCtx->codec_id);
        ret = avcodec_open2(pDecoder->pACodecCtx, pDecoder->pACodec, NULL);
        if (ret < 0) {
            LOGE("Couldn't open audio codec.\n");
            return;
        }
        pDecoder->pAFrame = av_frame_alloc();
        pDecoder->pSwrCtx = swr_alloc();
        pDecoder->pSwrCtx = swr_alloc_set_opts(NULL, pDecoder->pACodecCtx->channel_layout,
                                               AV_SAMPLE_FMT_S16, pDecoder->pACodecCtx->sample_rate,
                                               pDecoder->pACodecCtx->channel_layout,
                                               pDecoder->pACodecCtx->sample_fmt,
                                               pDecoder->pACodecCtx->sample_rate, 0, NULL);
        swr_init(pDecoder->pSwrCtx);
    }

}

void decode() {
    while (1) {
        AVPacket *pPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
        int ret = av_read_frame(pDecoder->pFormatCtx, pPacket);
        if (ret < 0) {
            break;
        }
        en_queue(pPacket);
    }
}

void en_queue(AVPacket *packet) {
    if (packet == NULL) return;
    if (packet->stream_index == video_index) {
        item_t *item = malloc(sizeof(item_t));
        item->key = packet_num;
        item->statellite = packet;
        packet_num++;
        deque_tail_insert(pVideoQueue, *item);
    }
    if (packet->stream_index == audio_index) {
        item_t *item = malloc(sizeof(item_t));
        item->key = packet_num;
        item->statellite = packet;
        packet_num++;
        deque_tail_insert(pAudioQueue, *item);
    }
}

AVFrame *de_queue_pic() {
    begin:
    if (queue_empty(pVideoQueue)) return NULL;
    item_t *item = malloc(sizeof(item_t));
    deque_head_delete(pVideoQueue, item);
    AVPacket packet = *((AVPacket *) (item->statellite));
    av_gettime_relative();
    int ret = avcodec_send_packet(pDecoder->pVCodecCtx, &packet);
    if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
        goto begin;
    }
    ret = avcodec_receive_frame(pDecoder->pVCodecCtx, pDecoder->pFrame);
    if (ret < 0 && ret != AVERROR_EOF)
        goto begin;
    sws_scale(pDecoder->pSwsCtx, (const uint8_t *const *) pDecoder->pFrame->data,
              pDecoder->pFrame->linesize, 0, pDecoder->pVCodecCtx->height,
              pDecoder->pFrameYUV->data,
              pDecoder->pFrameYUV->linesize);
    double timestamp;
    if (packet.pts == AV_NOPTS_VALUE) {
        timestamp = 0;
    } else {
        timestamp = av_frame_get_best_effort_timestamp(pDecoder->pFrame) *
                    av_q2d(pDecoder->pVStream->time_base);
    }
    double frameRate = av_q2d(pDecoder->pVStream->avg_frame_rate);
    frameRate += pDecoder->pFrame->repeat_pict * (frameRate * 0.5);
    if (timestamp == 0.0) {
        usleep((unsigned long) (frameRate * 1000));
    } else {
        if (fabs(timestamp - audio_clock) > AV_SYNC_THRESHOLD_MIN &&
            fabs(timestamp - audio_clock) < AV_NOSYNC_THRESHOLD) {
            if (timestamp > audio_clock) {
                usleep((unsigned long) ((timestamp - audio_clock) * 1000000));
            }
        }
    }
    av_packet_unref(&packet);
    free(item);
    return pDecoder->pFrameYUV;
}

void de_queue_audio(int *next_size, uint8_t *out_buffer) {
    begin:
    if (queue_empty(pAudioQueue)) return;
    item_t *item = malloc(sizeof(item_t));
    deque_head_delete(pAudioQueue, item);
    AVPacket packet = *((AVPacket *) item->statellite);
    int ret = avcodec_send_packet(pDecoder->pACodecCtx, &packet);
    if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
        goto begin;
    ret = avcodec_receive_frame(pDecoder->pACodecCtx, pDecoder->pAFrame);
    if (ret < 0 && ret != AVERROR_EOF)
        goto begin;
    audio_clock = pDecoder->pAFrame->pkt_pts * av_q2d(pDecoder->pAStream->time_base);
    if (pDecoder->pACodecCtx->sample_fmt == AV_SAMPLE_FMT_S16P) {
        *next_size = av_samples_get_buffer_size(pDecoder->pAFrame->linesize,
                                                pDecoder->pACodecCtx->channels,
                                                pDecoder->pACodecCtx->frame_size,
                                                pDecoder->pACodecCtx->sample_fmt, 1);
    } else {
        av_samples_get_buffer_size(next_size, pDecoder->pACodecCtx->channels,
                                   pDecoder->pACodecCtx->frame_size,
                                   pDecoder->pACodecCtx->sample_fmt, 1);
    }
    ret = swr_convert(pDecoder->pSwrCtx, &out_buffer, pDecoder->pAFrame->nb_samples,
                      (uint8_t const **) (pDecoder->pAFrame->extended_data),
                      pDecoder->pAFrame->nb_samples);
    free(item);
    av_packet_unref(&packet);
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
    pDec->pVCodecCtx = pDec->pFormatCtx->streams[video_index]->codec;
    pDec->pVCodec = avcodec_find_decoder(pDec->pVCodecCtx->codec_id);
    if (pDec->pVCodec == NULL) {
        LOGE("Couldn't find Codec.\n");
        return -1;
    }
    if (avcodec_open2(pDec->pVCodecCtx, pDec->pVCodec, NULL) < 0) {
        LOGE("Couldn't open codec.\n");
        return -1;
    }
    int dataLen = pDec->pVCodecCtx->width * pDec->pVCodecCtx->height +
                  (pDec->pVCodecCtx->width * pDec->pVCodecCtx->height / 2);
    uint8_t *deData = malloc(dataLen);
    pDec->pFrame = av_frame_alloc();
    pDec->pFrameYUV = av_frame_alloc();
    av_image_fill_arrays(pDec->pFrameYUV->data, pDec->pFrameYUV->linesize, deData,
                         AV_PIX_FMT_YUV420P, pDec->pVCodecCtx->width, pDec->pVCodecCtx->height, 1);

    av_init_packet(&(pDec->packet));

    pDec->pSwsCtx = sws_getContext(pDec->pVCodecCtx->width, pDec->pVCodecCtx->height,
                                   pDec->pVCodecCtx->pix_fmt, pDec->pVCodecCtx->width,
                                   pDec->pVCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC,
                                   NULL, NULL, NULL);

    sprintf(info, "[Input     ]%s\n", url);
    sprintf(info, "%s[Format    ]%s\n", info, pDec->pFormatCtx->iformat->name);
    sprintf(info, "%s[Codec     ]%s\n", info, pDec->pVCodecCtx->codec->name);
    sprintf(info, "%s[Resolution]%dx%d\n", info, pDec->pVCodecCtx->width, pDec->pVCodecCtx->height);
    frame_cnt = 0;

    while (av_read_frame(pDec->pFormatCtx, &(pDec->packet)) >= 0) {
        if (pDec->packet.stream_index == video_index) {
            ret = avcodec_decode_video2(pDec->pVCodecCtx, pDec->pFrame, &got_pic,
                                        &(pDec->packet));
            if (ret < 0) {
                LOGE("Decode Error.\n");
                return -1;
            }
            if (got_pic) {
                sws_scale(pDec->pSwsCtx, (const uint8_t *const *) pDec->pFrame->data,
                          pDec->pFrame->linesize, 0, pDec->pVCodecCtx->height,
                          pDec->pFrameYUV->data,
                          pDec->pFrameYUV->linesize);
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

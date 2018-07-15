//
// Created by ZhaoLiangtai on 2018/7/7.
//
#include "video_decode.h"

int init_flag = 0;

typedef  struct _VideoDec {
    AVCodecContext *pCodecCtx;
    AVFrame *pFrame, *pFrameYUV;
    AVPacket packet;
    struct SwsContext *pSwsCtx;
    AVCodec *pCodec;
    AVFormatContext *pFormatCtx;
    AVDictionaryEntry *pTag;
} VideoDec;
VideoDec *decoder = NULL;

void pgm_save2(unsigned char *buf, int wrap, int xsize, int ysize, uint8_t *pDataOut) {
    int i;
    for (i = 0; i < ysize; i++) {
        memcpy(pDataOut + i * xsize, buf + i * wrap, xsize);
    }
}

int init_264decoder() {
    if (init_flag == 1) {
        sws_freeContext(decoder->pSwsCtx);
        av_packet_unref(&(decoder->packet));
        av_free(decoder->pFrame);
        avcodec_close(decoder->pCodecCtx);
        av_free(decoder->pCodecCtx);
    }
    decoder = av_mallocz(sizeof(VideoDec));
    decoder->pCodec = NULL;
    decoder->pSwsCtx = NULL;
    avcodec_register_all();
    decoder->pFrame = av_frame_alloc();
    av_init_packet(&(decoder->packet));
    decoder->pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (NULL != decoder->pCodec) {
        decoder->pCodecCtx = avcodec_alloc_context3(decoder->pCodec);
        decoder->pCodecCtx->flags |= AV_CODEC_FLAG_LOW_DELAY;
        if (avcodec_open2(decoder->pCodecCtx, decoder->pCodec, NULL) >= 0) {
            init_flag = 1;
            return 1;
        }
    } else {
        return 0;
    }
}

int decode264(uint8_t* h264_data, int data_len, uint8_t* yuv420) {
    int send_ret = 0;
    int receive_ret = 0;
    int got_pic;
    if (!decoder) {
        return -1;
    }
    decoder->packet.data = h264_data;
    decoder->packet.size = data_len;
//    ret = avcodec_decode_video2(decoder->pCodecCtx, decoder->pFrame, &got_pic, &(decoder->packet));
    // 返回0表示成功 new API
    send_ret = avcodec_send_packet(decoder->pCodecCtx, &(decoder->packet));
    if (send_ret != 0) {
        return -1;
    }
    while (avcodec_receive_frame(decoder->pCodecCtx, decoder->pFrame) == 0) {
        // y数据
        pgm_save2(decoder->pFrame->data[0],
                  decoder->pFrame->linesize[0],
                  decoder->pFrame->width,
                  decoder->pFrame->height, yuv420);
        // u数据
        pgm_save2(decoder->pFrame->data[1],
                  decoder->pFrame->linesize[1],
                  decoder->pFrame->width / 2,
                  decoder->pFrame->height / 2,
                  yuv420 + decoder->pFrame->width * decoder->pFrame->height);
        // v数据
        pgm_save2(decoder->pFrame->data[2],
                  decoder->pFrame->linesize[2],
                  decoder->pFrame->width / 2,
                  decoder->pFrame->height / 2,
                  yuv420 + decoder->pFrame->width * decoder->pFrame->height * 5 / 4);
        receive_ret = 1;
    }
    return receive_ret;
}

int decode_url(JNIEnv* env, jobject jobj, jstring url) {
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
    if (avformat_open_input(&(pDec->pFormatCtx), input_str
            /*"rtmp://live.hkstv.hk.lxdns.com/live/hks"*/, NULL, NULL) != 0) {
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
    pDec->pFrameYUV = av_frame_alloc();
    av_image_fill_arrays(pDec->pFrameYUV->data, pDec->pFrameYUV->linesize, deData,
                         AV_PIX_FMT_YUV420P, pDec->pCodecCtx->width, pDec->pCodecCtx->height, 1);

    av_init_packet(&(pDec->packet));

    pDec->pSwsCtx = sws_getContext(pDec->pCodecCtx->width, pDec->pCodecCtx->height,
                                   pDec->pCodecCtx->pix_fmt, pDec->pCodecCtx->width,
                                   pDec->pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC,
                                   NULL, NULL, NULL);

    sprintf(info, "[Input     ]%s\n", input_str);
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
                sws_scale(pDec->pSwsCtx, (const uint8_t *const *) pDec->pFrame->data,
                          pDec->pFrame->linesize, 0, pDec->pCodecCtx->height, pDec->pFrameYUV->data,
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
        }
    }
    free(deData);
    return 2;
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

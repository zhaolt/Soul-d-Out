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
} VideoDec;
VideoDec *decoder = NULL;

void pgm_save2(unsigned char *buf, int wrap, int xsize, int ysize, uint8_t *pDataOut) {
    int i;
    for (i = 0; i < ysize; i++) {
        memcpy(pDataOut + i * xsize, buf + i * wrap, xsize);
    }
}

int init_avdecoder() {
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
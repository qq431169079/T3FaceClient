#ifndef ENCODER_H
#define ENCODER_H

#include <stdio.h>
#include <QString>
#include <QObject>
#include <QTimer>

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif
#endif

//test different codec
#define TEST_H264  1
#define TEST_HEVC  0
#include <QTimer>
#include <QDateTime>

struct H265Data
{
  uint8_t *outData;
  int dataSize;
};

class Encoder : public QObject
{
    Q_OBJECT
public:

    Encoder(QObject *parent);

    H265Data _h265Data_;
    int initEncoder();
    int encodeFrame(uint8_t *data);
    int closeEncoder();
    int flashEncoder();
    bool _isClose_ = true;


private:
        QTimer *_recordVideoTimer;

        void startRecordVideo();
        AVCodec *pCodec;
        AVCodecContext *pCodecCtx= NULL;
        int i, ret, got_output;
        FILE *fp_in;
        FILE *fp_out;
        AVFrame *pFrame;
        AVPacket pkt;
        //AVPacket *pkt;
        int y_size;
        int framecnt=0;
        int _pts = 0;
        int yuyvToYuv(uint8_t * temp_buffer);

    #if TEST_HEVC
        AVCodecID codec_id=AV_CODEC_ID_HEVC;
        //char filename_out[]="ds.hevc";
    #else
        AVCodecID codec_id=AV_CODEC_ID_H264;
        QString filename_out ="ds.h264";
    #endif


        int in_w=640,in_h=480;
        AVPixelFormat src_pixfmt=AV_PIX_FMT_YUYV422;
        int src_bpp;
        AVPixelFormat dst_pixfmt=AV_PIX_FMT_YUV420P;
        int dst_bpp;
        uint8_t *src_data[4];
        int src_linesize[4];

        uint8_t *dst_data[4];
        int dst_linesize[4];

        int rescale_method=SWS_BICUBIC;
        struct SwsContext *img_convert_ctx;
        uint8_t *temp_buffer;
        int frame_idx=0;


};

#endif // ENCODER_H

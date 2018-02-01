#include "encoder.h"
#include "t3_log.h"
#include <QDateTime>

Encoder::Encoder(QObject *parent)
{

}
int Encoder::initEncoder()
{
    T3LOG  <<"initEncoder";
    _isClose_ = false;
    avcodec_register_all();

    pCodec = avcodec_find_encoder(codec_id);
    //pCodec = avcodec_find_encoder_by_name("h264_qsv");
    if (!pCodec) {
        printf("Codec not found\n");
        return -1;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx) {
        printf("Could not allocate video codec context\n");
        return -1;
    }
    pCodecCtx->bit_rate = 400000;
    pCodecCtx->width = in_w;
    pCodecCtx->height = in_h;
    pCodecCtx->time_base.num=1;
    pCodecCtx->time_base.den=25;
    pCodecCtx->gop_size = 10;
    pCodecCtx->max_b_frames = 1;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

    AVDictionary *AParam = NULL;
    if (codec_id == AV_CODEC_ID_H264)
        T3LOG << "preset";
        //av_opt_set(pCodecCtx->priv_data, "preset", "slow", 0);
        //
        av_dict_set(&AParam,"preset", "superfast",   0);
        av_dict_set(&AParam, "tune",   "zerolatency", 0);

    if (avcodec_open2(pCodecCtx, pCodec, &AParam) < 0) {
        printf("Could not open codec\n");
        return -1;
    }

    pFrame = av_frame_alloc();
    if (!pFrame) {
        printf("Could not allocate video frame\n");
        return -1;
    }
    pFrame->format = pCodecCtx->pix_fmt;
    pFrame->width  = pCodecCtx->width;
    pFrame->height = pCodecCtx->height;

    ret = av_image_alloc(pFrame->data, pFrame->linesize, pCodecCtx->width, pCodecCtx->height,
                             pCodecCtx->pix_fmt, 16);
    if (ret < 0) {
        printf("Could not allocate raw picture buffer\n");
        return -1;
    }
    _recordVideoTimer = new QTimer(this);
    _recordVideoTimer->start(1000*60*10);
    connect(_recordVideoTimer,&QTimer::timeout,this,&Encoder::startRecordVideo);
   /* QString dataTimeString = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    fileName_ = "/home/t001/video/"+dataTimeString+".h264";
    //Output bitstream
        fp_out = fopen(fileName_, "wb");
        if (!fp_out) {
            printf("Could not open ");
            return -1;
        }
*/
        y_size = pCodecCtx->width * pCodecCtx->height;



        //yuyvToyuvInit

        src_bpp=av_get_bits_per_pixel(av_pix_fmt_desc_get(src_pixfmt));
        dst_bpp=av_get_bits_per_pixel(av_pix_fmt_desc_get(dst_pixfmt));
        temp_buffer=(uint8_t *)malloc(in_w*in_h*src_bpp/8);
        ret= av_image_alloc(src_data, src_linesize,in_w, in_h, src_pixfmt, 1);
            if (ret< 0) {
                printf( "Could not allocate source image\n");
                return -1;
            }
        ret = av_image_alloc(dst_data, dst_linesize,in_w, in_h, dst_pixfmt, 1);
            if (ret< 0) {
                printf( "Could not allocate destination image\n");
                return -1;
            }

            //-----------------------------
                //Init Method 1
                img_convert_ctx =sws_alloc_context();
                T3LOG << "test";
                //Show AVOption
                //av_opt_show2(img_convert_ctx,stdout,AV_OPT_FLAG_VIDEO_PARAM,0);
                //Set Value


                av_opt_set_int(img_convert_ctx,"sws_flags",SWS_BICUBIC|SWS_PRINT_INFO,0);
                av_opt_set_int(img_convert_ctx,"srcw",in_w,0);
                av_opt_set_int(img_convert_ctx,"srch",in_h,0);
                av_opt_set_int(img_convert_ctx,"src_format",src_pixfmt,0);
                //'0' for MPEG (Y:0-235);'1' for JPEG (Y:0-255)
                av_opt_set_int(img_convert_ctx,"src_range",1,0);
                av_opt_set_int(img_convert_ctx,"dstw",in_w,0);
                av_opt_set_int(img_convert_ctx,"dsth",in_h,0);
                av_opt_set_int(img_convert_ctx,"dst_format",dst_pixfmt,0);
                av_opt_set_int(img_convert_ctx,"dst_range",1,0);
                sws_init_context(img_convert_ctx,NULL,NULL);

                //Init Method 2
                //img_convert_ctx = sws_getContext(src_w, src_h,src_pixfmt, dst_w, dst_h, dst_pixfmt,
                //	rescale_method, NULL, NULL, NULL);
                //-----------------------------


}

int Encoder::encodeFrame(uint8_t *data)

{

    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;
    av_free_packet(&pkt);
    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;
    //pkt = new AVPacket();
    //pkt = av_packet_alloc();
    //av_free_packet(&pkt);

    yuyvToYuv(data);

    pFrame->data[0] = dst_data[0];
    pFrame->data[1] = dst_data[1];
    pFrame->data[2] = dst_data[2];

    ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_output);

    pFrame->pts = _pts++;
    if (ret < 0) {
        printf("Error encoding frame\n");
        return -1;
    }
    if (got_output) {
        framecnt++;
        _h265Data_.outData = pkt.data;
        _h265Data_.dataSize = pkt.size;

        //av_free_packet(&pkt);
       fwrite(pkt.data, 1, pkt.size, fp_out);
    }

}


int Encoder::flashEncoder()
{
    for (got_output = 1; got_output; i++) {
        ret = avcodec_encode_video2(pCodecCtx, &pkt, NULL, &got_output);
        if (ret < 0) {
            printf("Error encoding frame\n");
            return -1;
        }
        if (got_output) {
            printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n",pkt.size);
            /*fwrite(pkt.data, 1, pkt.size, fp_out);*/
            //av_free_packet(&pkt);
        }
    }
}


int Encoder::closeEncoder()
{
    //Flush Encoder
    _isClose_ = true;
    for (got_output = 1; got_output; i++) {
        ret = avcodec_encode_video2(pCodecCtx, &pkt, NULL, &got_output);
        if (ret < 0) {
            printf("Error encoding frame\n");
            return -1;
        }
        if (got_output) {
            printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n",pkt.size);
            /*fwrite(pkt.data, 1, pkt.size, fp_out);*/
            //av_free_packet(&pkt);
        }
    }

    fclose(fp_out);
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);
    av_freep(&pFrame->data[0]);
    av_frame_free(&pFrame);
    T3LOG << "closeDecoder";
    return 0;
}

int Encoder::yuyvToYuv( uint8_t * temp_buffer)
{
    switch(src_pixfmt){
            case AV_PIX_FMT_GRAY8:{
                memcpy(src_data[0],temp_buffer,in_w*in_h);
                break;
                                  }
            case AV_PIX_FMT_YUV420P:{
                memcpy(src_data[0],temp_buffer,in_w*in_h);                    //Y
                memcpy(src_data[1],temp_buffer+in_w*in_h,in_w*in_h/4);      //U
                memcpy(src_data[2],temp_buffer+in_w*in_h*5/4,in_w*in_h/4);  //V
                break;
                                    }
            case AV_PIX_FMT_YUV422P:{
                memcpy(src_data[0],temp_buffer,in_w*in_h);                    //Y
                memcpy(src_data[1],temp_buffer+in_w*in_h,in_w*in_h/2);      //U
                memcpy(src_data[2],temp_buffer+in_w*in_h*3/2,in_w*in_h/2);  //V
                T3LOG << "encoder";
                break;
                                    }
            case AV_PIX_FMT_YUV444P:{
                memcpy(src_data[0],temp_buffer,in_w*in_h);                    //Y
                memcpy(src_data[1],temp_buffer+in_w*in_h,in_w*in_h);        //U
                memcpy(src_data[2],temp_buffer+in_w*in_h*2,in_w*in_h);      //V
                break;
                                    }
            case AV_PIX_FMT_YUYV422:{
                memcpy(src_data[0],temp_buffer,in_w*in_h*2);                  //Packed
                break;
                                    }
            case AV_PIX_FMT_RGB24:{
                memcpy(src_data[0],temp_buffer,in_w*in_h*3);                  //Packed
                break;
                                    }
            default:{
                printf("Not Support Input Pixel Format.\n");
                break;
                                  }
            }
            sws_scale(img_convert_ctx, src_data, src_linesize, 0, in_h, dst_data, dst_linesize);
            //frame_idx++;

}

void Encoder::startRecordVideo()
{
    QString dataTimeString = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString fileName_ = "/home/t001/video/"+dataTimeString+".h264";
    std::string str = fileName_.toStdString();
    const char * fileNa_= str.c_str();
        //Output bitstream
            fp_out = fopen(fileNa_, "wb");
            if (!fp_out) {
                printf("Could not open ");

            }
}

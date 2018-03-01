#include "t3_face_imageformat.h"

T3_Face_ImageFormat::T3_Face_ImageFormat()
{

}

int T3_Face_ImageFormat::initFormat(AVPixelFormat srcFormat,
                                     AVPixelFormat dstFormat,
                                     int width,
                                     int height)
{
    _height = height;
    _width = width;
    _srcFmt = srcFormat;
    _dstFmt = dstFormat;
    _srcBpp=av_get_bits_per_pixel(av_pix_fmt_desc_get(_srcFmt));
    _dstBpp=av_get_bits_per_pixel(av_pix_fmt_desc_get(_dstFmt));
    //temp_buffer=(uint8_t *)malloc(in_w*in_h*src_bpp/8);
    ret= av_image_alloc(_tempData, src_linesize,_width, _height, _srcFmt, 1);
        if (ret< 0) {
            printf( "Could not allocate source image\n");
            return -1;
        }
    ret = av_image_alloc(dst_data, dst_linesize,_width, _height, _dstFmt, 1);
        if (ret< 0) {
            printf( "Could not allocate destination image\n");
            return -1;
        }

    img_convert_ctx =sws_alloc_context();

    av_opt_set_int(img_convert_ctx,"sws_flags",SWS_BICUBIC|SWS_PRINT_INFO,0);
    av_opt_set_int(img_convert_ctx,"srcw",_width,0);
    av_opt_set_int(img_convert_ctx,"srch",_height,0);
    av_opt_set_int(img_convert_ctx,"src_format",_srcFmt,0);
    //'0' for MPEG (Y:0-235);'1' for JPEG (Y:0-255)
    av_opt_set_int(img_convert_ctx,"src_range",1,0);
    av_opt_set_int(img_convert_ctx,"dstw",_width,0);
    av_opt_set_int(img_convert_ctx,"dsth",_height,0);
    av_opt_set_int(img_convert_ctx,"dst_format",_dstFmt,0);
    av_opt_set_int(img_convert_ctx,"dst_range",1,0);
    sws_init_context(img_convert_ctx,NULL,NULL);
}

uint8_t *T3_Face_ImageFormat::chengeImageFormat(uint8_t *srcData)
{

    switch(_srcFmt){
            case AV_PIX_FMT_GRAY8:{
                memcpy(_tempData[0],srcData,_width*_height);
                break;
                                  }
            case AV_PIX_FMT_YUV420P:{
                memcpy(_tempData[0],srcData,_width*_height);                    //Y
                memcpy(_tempData[1],srcData+_width*_height,_width*_height/4);      //U
                memcpy(_tempData[2],srcData+_width*_height*5/4,_width*_height/4);  //V
                break;
                                    }
            case AV_PIX_FMT_YUV422P:{
                memcpy(_tempData[0],srcData,_width*_height);                    //Y
                memcpy(_tempData[1],srcData+_width*_height,_width*_height/2);      //U
                memcpy(_tempData[2],srcData+_width*_height*3/2,_width*_height/2);  //V
                T3LOG << "encoder";
                break;
                                    }
            case AV_PIX_FMT_YUV444P:{
                memcpy(_tempData[0],srcData,_width*_height);                    //Y
                memcpy(_tempData[1],srcData+_width*_height,_width*_height);        //U
                memcpy(_tempData[2],srcData+_width*_height*2,_width*_height);      //V
                break;
                                    }
            case AV_PIX_FMT_YUYV422:{
                memcpy(_tempData[0],srcData,_width*_height*2);                  //Packed
                break;
                                    }
            case AV_PIX_FMT_RGB24:{
                memcpy(_tempData[0],srcData,_width*_height*3);                  //Packed
                break;
                                    }
            default:{
                printf("Not Support Input Pixel Format.\n");
                break;
                                  }
            }
            sws_scale(img_convert_ctx, _tempData, src_linesize, 0, _height, dst_data, dst_linesize);
            return *dst_data;
}


T3_Face_ImageFormat::~T3_Face_ImageFormat()
{

}

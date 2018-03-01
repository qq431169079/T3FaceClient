#ifndef T3_FACE_IMAGEFORMAT_H
#define T3_FACE_IMAGEFORMAT_H
extern "C"
{
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
#include <libswscale/swscale.h>
}
#include "t3_log.h"

class T3_Face_ImageFormat
{
public:
    T3_Face_ImageFormat();
    ~T3_Face_ImageFormat();
    /**
     * @brief initFormat 设置图像转换的格式
     * @param srcFormat　原图像格式
     * @param dstFormat　目标图像格式
     * @param width　图像的宽
     * @param height　图像的高
     * @return 是否设置成功
     */
    int initFormat(AVPixelFormat srcFormat,
                   AVPixelFormat dstFormat,
                    int width,
                    int height);
    /**
     * @brief 图像数据格式转换
     * @param srcData　原图像数据
     * @param dstData　目标图像数据
     */
    uint8_t *  chengeImageFormat(uint8_t *srcData);

private:
    int _height = 0;
    int _width = 0;
    AVPixelFormat _srcFmt;
    int _srcBpp;
    AVPixelFormat _dstFmt;
    int _dstBpp;
    uint8_t *_tempData[4];
    int src_linesize[4];

    uint8_t *dst_data[4];
    int dst_linesize[4];

    int rescale_method=SWS_BICUBIC;
    struct SwsContext *img_convert_ctx;
    uint8_t *temp_buffer;
    int frame_idx=0;
    int ret = 0;
};

#endif // T3_FACE_IMAGEFORMAT_H

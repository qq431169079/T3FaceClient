#ifndef CAMERA_H
#define CAMERA_H

#include "t3_face_imageformat.h"
#include <QObject>
#include "t3_library.h"
#include "framesource.h"
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d/calib3d.hpp>

using namespace cv;
class CameraSource :public QObject,public FrameSource
{
	Q_OBJECT
public:
    CameraSource(int width, int height, int format);
    virtual ~CameraSource();

    virtual void ReadFrame(unsigned char *pBuffer,int bufsize);
    virtual void Open(unsigned int id,QString devicePath);
    virtual void Close(void);
    virtual bool Ready(void);
    virtual int Width(void);
    virtual int Height(void);
    virtual int Format(void);
private:
	int mWidth;
	int mHeight;
	int mFormat;
    bool bOpen;
    T3_Face_ImageFormat *_imageFormat;
    T3_Face_ImageFormat *_imageFormatYuv2yuyv;
    Mat undistort_frame ;//= frame.clone();
    Mat camera_matrix = Mat::eye(3, 3, CV_64F);
    Mat distortion_coefficients;
    FileStorage *file_storage;
#ifdef WIN32
    unsigned int mVideoDeviceCount;
    void **mVideoDevices;
    void *mSourceReader;
#else
    int mV4l2_fd;
    void *mV4l2_bufstart[4];
    size_t mV4l2_buflength[4];
    int mV4l2_bufcount;
    uint8_t * _yuv420pData;
    uint8_t * _yuyv422Data;
#endif

};

#endif // CAMERA_H

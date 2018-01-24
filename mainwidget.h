#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QTcpSocket>
#include <QTimer>
#include <QSqlDatabase>
#include <QDateTime>
#include <QSqlQuery>
#include <QMap>


#include "arcfaceengine.h"
#include "encoder.h"
#include "serialport.h"
#include "t3_library.h"
#include "t3_face_network.h"

#define kWidth 0;
#define kHeight 0;

class VideoFileSource;
class CameraSource;
class GLHelper;
class Encoder;

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)


/**
  ******************************************************************************
  * @file    mainwidget.h
  * @author  HXP
  * @version  V1.0.0
  * @date    2017/12/26
  * @brief
  ******************************************************************************
*/

class MainWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);

    ~MainWidget();

protected:
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
	void initializeGL() override;
	void paintGL() override;
    void resizeGL(int width, int height) override;

private slots:



	void openCamera();
    void readMessage();
    void sendFrameData();
    void startSendVideo();
    void stopTime();
    void startReConnect();
    void reconnect();
    void stopReConnect();
    void deleteFaceInfoById(int id);


signals:
    void recognBroadCastSignal(int id,float fscore);
    void sendVideo();
    void stopVideo();
    void updateState(int id);
private:

    void loadFaceDB();
    void readDatabase();
    void getNewFaceInfo();
    void paramControle(quint8 sign);


    //grender
    int _maleNum = 0;
    int _femaleNum = 0;
    int _frameNum = 0;
    bool _sendSerialGreet = true;
    //parameterControl
    bool _openSound = true;
    bool _record = true;

    T3_Face_Network *_network;

    QSqlDatabase _database;
    Encoder * _encoder;
    GLHelper* mGLHelper = nullptr;
    CameraSource *mCameraSource = nullptr;
    QTimer *_sendVideoTimer;
    QTimer *_reConnectTimer;
    ArcFaceEngine *mArcFaceEngine;
    QTcpSocket *_socket;

    QMap<int,QDateTime> dateTime;//添加记录的判断条件，判断两次的时间间隔。
    const int KMaxStateNum = 0;
    QByteArray  outdata;



    quint32 _blockSize = 0;
    quint32 _readSign = 0;
    QByteArray _databaseData;
    quint32 _sign = 0;
    quint32 _right = 0;
    quint32 _left = 0;
    quint32 _top = 0;
    quint32 _bottom = 0;

    bool _existFrame[MAX_FT_FACE];
    int _existFrames[];
    int oldFaceId[MAX_FT_FACE];

    int detailId = 0 ;
    int frameNum = 0;
    unsigned char *mFrameData = nullptr;
    int mFrameDataSize;
    int mFrameWidth;
    int mFrameHeight;
    int mFrameFormat;
    bool bFrameMirrored = false;
    quint32 _id = 0;
    QString _name ;
    QByteArray _feature;
    QByteArray _thumbnail;
    quint8 _state = 0;
    unsigned char *mWidgetData = nullptr;
	int mWidgetWidth;
	int mWidgetHeight;


    int _newFaceId = -1;
    bool bWaitForInputName;
    int mFaceWidth = 0;
    int mFaceHeight = 0;
    int numFrame = 0;
    int oldFaceNum = 0;
    int facenumber = 0;
    QString _role;
    QString _age;
    QString _updateTime;
    bool _netWorkState = false;
    SerialPort *serial;

     bool showName = false;


};

#endif

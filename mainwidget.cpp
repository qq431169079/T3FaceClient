#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMouseEvent>
#include <QDebug>
#include <QInputDialog>
#include <QPainter>
#include <QFileDialog>
#include <QtNetwork>
#include <QImage>
#include <QPixmap>
#include <QHostAddress>
#include <QSqlDriver>

#include "mainwidget.h"
#include "camerasource.h"
#include "glhelper.h"
#include "arcfaceengine.h"
#include "asvloffscreen.h"
#include "t3_log.h"
#include "t3_library.h"



MainWidget::MainWidget(QWidget *parent)
	: QOpenGLWidget(parent)
{

    //相关初始化
    mArcFaceEngine = ArcFaceEngine::getFaceEngine();
    _encoder = new Encoder(this);
    _sendVideoTimer = new QTimer(this);
    _reConnectTimer = new QTimer(this);
    _socket = new QTcpSocket(this);
    _network = T3_Face_Network::getNetwork();
    _database1 = T3_Face_Database::getDatabase();
    _tts = T3_Face_TTS::getTTS();


    openCamera();
    startReConnect();
    //监听相关信号
    //connect(_network,&T3_Face_Network::connectedNetwork);
    connect(_reConnectTimer,&QTimer::timeout,this,&MainWidget::reconnect);
    connect(_socket,&QTcpSocket::connected,this,&MainWidget::stopReConnect);    
    connect(_socket,&QTcpSocket::disconnected,this,&MainWidget::startReConnect);
    connect(_socket,&QTcpSocket::disconnected,this,&MainWidget::stopTime);
    connect(_socket,&QTcpSocket::readyRead,this,&MainWidget::readMessage);
    connect(this,&MainWidget::sendVideo,this,&MainWidget::startSendVideo);
    connect(this,&MainWidget::stopVideo,this,&MainWidget::stopTime);
    connect(_sendVideoTimer,&QTimer::timeout,this,&MainWidget::sendFrameData);
    connect(mArcFaceEngine,&ArcFaceEngine::log,this,&MainWidget::log);

    setFixedSize(600, 450);
    move(20,170);
}

void MainWidget::openCamera()
{


    if(mCameraSource == nullptr)
    {

        bFrameMirrored = true;
        mCameraSource = new CameraSource(640, 480, ASVL_PAF_YUYV);
        mCameraSource->Open(0,"");
    }
}

void MainWidget::startReConnect()
{
    _netWorkState = false;
    _reConnectTimer->start(4000);

}
void MainWidget::stopReConnect()
{
    mArcFaceEngine->mRegisterFaces.clear();
    _netWorkState = true;
    _reConnectTimer->stop();
    _database1->loadFaceDatabase();
    T3LOG << "network connected";
}
void MainWidget::reconnect()
{
    _socket->abort();
    _socket->connectToHost("192.168.0.136",kServerPort);
}





QSize MainWidget::minimumSizeHint() const
{
	return QSize(mWidgetWidth, mWidgetHeight);
}

QSize MainWidget::sizeHint() const
{
    return QSize(mWidgetWidth, mWidgetHeight);
}

void MainWidget::initializeGL()
{
    if (mGLHelper == NULL) {
        mGLHelper = new GLHelper();
	}

	initializeOpenGLFunctions();
	glClearDepthf(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
    glLineWidth(2.0);
	glEnable(GL_MULTISAMPLE);

}

void MainWidget::paintGL()
{



    if(oldFaceNum!=0)
    {
        numFrame++;
        showName = true;
        facenumber = mArcFaceEngine->mFaceNum;
    }else
    {
        showName  = false;
        facenumber = 0;
    }

    QSqlQuery query_(_database);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QPainter painter(this);
    QPen pen(Qt::green,5);
    painter.setPen(pen);
    painter.setFont(QFont(QString::fromLocal8Bit("宋体"),12,-1,false));
    painter.beginNativePainting();
    bool bNativePainting = true;

    FrameSource *frameSource = nullptr;


    frameSource = mCameraSource;


    if((frameSource != nullptr)&&(frameSource->Ready())){
        if(mFrameData == nullptr){
            mFrameFormat = frameSource->Format();
            mFrameWidth = frameSource->Width();


            mFrameHeight = frameSource->Height();
            T3LOG << mFrameHeight;
            T3LOG << mFrameWidth;

            if ((mFrameFormat == ASVL_PAF_NV21) || (mFrameFormat == ASVL_PAF_NV12) || (mFrameFormat == ASVL_PAF_I420)) {
                mFrameDataSize = mFrameWidth*mFrameHeight * 3 / 2;
            }else if(mFrameFormat == ASVL_PAF_YUYV){
                mFrameDataSize = mFrameWidth*mFrameHeight *2;
            }else {
                mFrameDataSize = mFrameWidth*mFrameHeight *4;
            }

            mFrameData = new unsigned char[mFrameDataSize];
            if(mFrameWidth>mFrameHeight){
                mWidgetWidth = 640;

                mWidgetHeight = mFrameHeight*mWidgetWidth/mFrameWidth;
            }else{
                mWidgetHeight = 640;

                mWidgetWidth = mFrameWidth*mWidgetHeight/mFrameHeight;
            }

            mWidgetData = new unsigned char[640 * 480 * 4];
           // setFixedSize(460, 350);

        }else{
            frameSource->ReadFrame(mFrameData,mFrameDataSize);
            mGLHelper->setMirror(bFrameMirrored);
            mGLHelper->drawTexture(mFrameData, mFrameFormat, mFrameWidth, mFrameHeight);
            glReadPixels(0,0, 640, 480, GL_BGRA, GL_UNSIGNED_BYTE, mWidgetData);
            if(mArcFaceEngine->processFrame(mFrameData,mFrameWidth,mFrameHeight,mFrameFormat)){
                if(mArcFaceEngine->mFaceNum!=oldFaceNum)
                {
                    numFrame = 0;
                    oldFaceNum = mArcFaceEngine->mFaceNum;
                    _sendSerialGreet = true;
                }
                for (int i = 0; i < mArcFaceEngine->mFaceNum; i++) {
                     if(mArcFaceEngine->mFaceID[i]!=-1)
                     {
                         _sendSerialGreet = false;
                     }
                     float rectPoints[8];
                     MRECT *rect = &mArcFaceEngine->mFaceRect[i];
                     rectPoints[0] = (float)rect->left;
                     rectPoints[1] = (float)rect->top;
                     rectPoints[2] = (float)rect->right;
                     rectPoints[3] = (float)rect->top;
                     rectPoints[4] = (float)rect->right;
                     rectPoints[5] = (float)rect->bottom;
                     rectPoints[6] = (float)rect->left;
                     rectPoints[7] = (float)rect->bottom;

                     if(i == 0){
                         if(!bWaitForInputName){
                             int leftInWidget = mArcFaceEngine->mFaceRect[i].left*mWidgetWidth/mFrameWidth;
                             int topInWidget = mArcFaceEngine->mFaceRect[i].top*mWidgetHeight/mFrameHeight;
                             int rightInWidget = mArcFaceEngine->mFaceRect[i].right*mWidgetWidth/mFrameWidth;
                             int bottomInWidget = mArcFaceEngine->mFaceRect[i].bottom*mWidgetHeight/mFrameHeight;

                             mFaceWidth = rightInWidget-leftInWidget;
                             mFaceHeight = bottomInWidget-topInWidget;

                             if(bFrameMirrored){
                                  glReadPixels(0,0, mFrameWidth, mFrameHeight, GL_BGRA, GL_UNSIGNED_BYTE, mWidgetData);
                             }else{
                                  glReadPixels(leftInWidget,mWidgetHeight-1-bottomInWidget, mFaceWidth, mFaceHeight, GL_BGRA, GL_UNSIGNED_BYTE, mWidgetData);
                             }
                          }
                     }

                     mGLHelper->drawPoints(4, rectPoints, mFrameWidth, mFrameHeight, 1.0f, 1.0f, 0.0f, 1.0f, GL_LINE_LOOP);

                 }

                 painter.endNativePainting();
                 bNativePainting = false;
                 //判断画面中只有一个人，确认其性别并播放迎宾词。

/*                if(_sendSerialGreet)
                 {

                     if(1 == mArcFaceEngine->mFaceNum)
                     {
                        serial->greet(mArcFaceEngine->mFaceGrender[0]);
                        _sendSerialGreet = false;
                     }
                     if(mArcFaceEngine->mFaceNum>1)
                     {
                        serial->greet(2);
                        _sendSerialGreet = false;
                     }
                 }

*/
                 for (int i = 0; i < mArcFaceEngine->mFaceNum; i++) {
                     int leftInWidget = mArcFaceEngine->mFaceRect[i].left*mWidgetWidth/mFrameWidth;
                     int topInWidget = mArcFaceEngine->mFaceRect[i].top*mWidgetHeight/mFrameHeight;
                     int rightInWidget = mArcFaceEngine->mFaceRect[i].right*mWidgetWidth/mFrameWidth;
                      QString showString_ = mArcFaceEngine->mFaceName[i];
                     if(0 == mArcFaceEngine->mFaceGrender[i])
                     {
                          showString_ = QString(mArcFaceEngine->mFaceName[i]+"--男"+"   %1").arg(mArcFaceEngine->mFaceAge[i]);
                     }
                     if(1 == mArcFaceEngine->mFaceGrender[i])
                     {
                          showString_ = QString(mArcFaceEngine->mFaceName[i]+"--女"+"   %1").arg(mArcFaceEngine->mFaceAge[i]);
                     }

                     if(showName&&numFrame>30)
                     {
                          painter.drawText(QPointF(mWidgetWidth-1-rightInWidget,topInWidget), showString_);
                     }


                    if(mArcFaceEngine->mFaceID[i]>=0){


                        oldFaceId[i] = mArcFaceEngine->mFaceID[i];

                        //T3LOG << mArcFaceEngine->mScore[i];
                        if(showName&&numFrame>10){
                            if(dateTime.value(mArcFaceEngine->mFaceID[i]).addSecs(kInterval) <= QDateTime::currentDateTime())
                            {


                                if(_netWorkState&&false)
                                {

                                    QImage faceimage(mWidgetData,640,480, QImage::Format_RGB32);

                                    QString dataTimeString = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
                                    T3LOG << faceimage.height();
                                    T3LOG << faceimage.width();
                                    if(_record)
                                    {
                                        query_.prepare("insert into T3FaceRecord values(NULL,?,?,?,?)");
                                        QByteArray faceData;
                                        faceData.resize(640*480*4);
                                        memcpy(faceData.data(),mWidgetData,640*480*4);
                                        query_.bindValue(0,mArcFaceEngine->mFaceID[i]);
                                        query_.bindValue(1,mArcFaceEngine->mFaceName[i]);
                                        query_.bindValue(2,"");
                                        query_.bindValue(3,dataTimeString);
                                        bool succ = query_.exec();
                                        T3LOG << succ;
                                    }

                                    dateTime.insert(mArcFaceEngine->mFaceID[i] ,QDateTime::currentDateTime());
                                    query_.prepare("update T3Face set num = ? where id = ?");
                                    mArcFaceEngine->mRegisterFaces[mArcFaceEngine->mFaceID[i]].num ++ ;
                                    query_.bindValue(0,mArcFaceEngine->mRegisterFaces[mArcFaceEngine->mFaceID[i]].num);
                                    query_.bindValue(1,mArcFaceEngine->mFaceID[i]);
                                    query_.exec();

                                }else
                                {

                                }

                            }
                            if(bFrameMirrored){

                                if(mArcFaceEngine->mFaceState[i] >= KMaxStateNum){
                                   // painter.drawText(QPointF(mWidgetWidth-1-rightInWidget,topInWidget), mArcFaceEngine->mFaceName[i]);
                                }
                            }else{
                                painter.drawText(QPointF(leftInWidget,topInWidget), mArcFaceEngine->mFaceName[i]);
                            }
                        }
                    }
                }
            }else
            {
                numFrame = 0;
                T3LOG << "numFrame";
            }

        }
    }

    if(bNativePainting){
        painter.endNativePainting();
    }
	update();

}



void MainWidget::resizeGL(int width, int height)
{
	glViewport(mWidgetWidth, mWidgetHeight, 0, 0);
}

void MainWidget::readMessage()
{

    QDataStream stream_(_socket);
    stream_.setVersion(QDataStream::Qt_5_5);
    if(0 == _blockSize)
    {
        stream_ >> _blockSize;
        stream_ >> _readSign;
        switch (_readSign) {
        case 1:
            getNewFaceInfo();
            _blockSize = 0;
            break;
        case 2:
            emit sendVideo();
            _blockSize = 0;
            break;
        case 3:
            emit stopVideo();
            _blockSize = 0;
        case 4:
            quint32 id ;
            stream_ >> id;
            emit deleteFaceInfoById(id);
            _blockSize = 0;
            break;
        case 5:
            stream_ >> mArcFaceEngine->mThreshold;
            stream_ >> mArcFaceEngine->_autoRegister;
            stream_ >> _record;
            T3LOG  << mArcFaceEngine->mThreshold;
            _blockSize = 0;
        case 7:
            _tts->playLowPowerSound();
            break;

        default:
            break;
        }

    }


}
void MainWidget::deleteFaceInfoById(int id)
{
    T3LOG << id;
    mArcFaceEngine->mRegisterFaces.remove(id);
}

void MainWidget::getNewFaceInfo()
{
    /*QSqlQuery query(_database);
    query.prepare("select * from T3Face where id > ?");
    T3LOG << mArcFaceEngine->mUniqueIncID;
    query.bindValue(0,mArcFaceEngine->mUniqueIncID);
    query.exec();
    while(query.next())
    {
        int id = query.value(0).toInt();
        QString name_ = query.value(1).toString();
        QByteArray feature_ = query.value(4).toByteArray();
        int state_ = query.value(5).toInt();
        QString dateTime_ = query.value(6).toString();
        mArcFaceEngine->addFace(id,name_,feature_,state_,dateTime_);
        mArcFaceEngine->mUniqueIncID = id ;
        T3LOG << id ;
        T3LOG << name_ ;
    }*/
    _database1->loadFaceDatabase();

    T3LOG << "newFace";


}

void MainWidget::sendFrameData()
{
    _sign = 1;
    QByteArray block_;
    QDataStream stream_(&block_,QIODevice::WriteOnly);
    stream_.setVersion(QDataStream::Qt_5_5);

     stream_ << (quint32) 0;
     _encoder->encodeFrame((uint8_t *)mFrameData);
     //T3LOG << _encoder->_h265Data_.dataSize;
     if(_encoder->_h265Data_.dataSize<100000&&_encoder->_h265Data_.dataSize > 0)
     {
     QByteArray frameData_((const char*) _encoder->_h265Data_.outData,_encoder->_h265Data_.dataSize);

     stream_ << frameData_;
     stream_ << (quint32) mArcFaceEngine->mFaceNum;

     for(int i = 0;i < mArcFaceEngine->mFaceNum;i++)
     {

            MRECT *rect = &mArcFaceEngine->mFaceRect[i];

            int topInWidget = mArcFaceEngine->mFaceRect[i].top*mWidgetHeight/mFrameHeight;

            int rightInWidget = mArcFaceEngine->mFaceRect[i].right*mWidgetWidth/mFrameWidth;

             if(showName&&numFrame>10)
             {
                 stream_ <<  mArcFaceEngine->mFaceID[i];

             }else
             {
                 stream_ << (int) -1;

             }
            stream_ << (quint32)rect->left;
            stream_ << (quint32)rect->right;
            stream_ << (quint32)rect->top;
            stream_ << (quint32)rect->bottom;
            stream_ << (quint32)topInWidget;
            stream_ << (quint32)rightInWidget;




     }


      QString dataTimeString = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
     stream_ << dataTimeString;
     stream_.device()->seek(0);
     stream_ << (quint32) block_.size();
     //_socket->write(block_);
     _network->sendDataByUDP(block_.data(),block_.size());
     }


}


void MainWidget::startSendVideo()
{
    T3LOG << "start play video";
     _encoder->initEncoder();
    _sendVideoTimer->start(20);

}
void MainWidget::stopTime()
{
     T3LOG << "stop play video";
    if(!_encoder->_isClose_)
    {
        _encoder->closeEncoder();
    }

    _sendVideoTimer->stop();

}


MainWidget::~MainWidget()
{
    _encoder->closeEncoder();
    T3LOG << "close";
    if(mArcFaceEngine){
        delete mArcFaceEngine;
        mArcFaceEngine = nullptr;
    }

    if(mWidgetData){
        delete[] mWidgetData;
        mWidgetData = nullptr;
    }

    if(mFrameData){
        delete[] mFrameData;
        mFrameData = nullptr;
    }

    if(mCameraSource != nullptr){
        delete mCameraSource;
        mCameraSource = nullptr;
    }



    if (mGLHelper != nullptr) {
        mGLHelper->uninit();
        delete mGLHelper;
        mGLHelper = nullptr;
    }


}


void MainWidget::log(int id,
                     QString name)
{
    QString dateTime_ = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QSqlQuery query_;
    query_.prepare("insert into T3FaceRecord values(NULL,?,?,?,?)");
    query_.bindValue(0,id);
    query_.bindValue(1,name);
    query_.bindValue(2,"");
    query_.bindValue(3,dateTime_);
    if(_netWorkState)
    {
        _sign = 2;
        QByteArray block_;
        QDataStream stream_(&block_,QIODevice::WriteOnly);
        stream_.setVersion(QDataStream::Qt_5_5);
        stream_ <<(quint32) 0;
        stream_ << _sign;
        stream_ << id;
        stream_.device()->seek(0);
        stream_ << (quint32) block_.size();
        T3LOG << "send the log" << id;
        _socket->write(block_);
    }


}

#ifndef SERIALPORT_H
#define SERIALPORT_H

/**
  ******************************************************************************
  * @author  HXP
  * @version  V1.0.0
  * @date    2018/01/02
  * @brief   主要作用是向科大讯飞开发板发送语音合成信息
  ******************************************************************************
*/
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QObject>
#include <QSqlQuery>
#include <QtGlobal>
#include <QTime>
#include <QTimer>


#include "t3_library.h"

//在电脑没有科大迅飞开发板时模拟使用
#define simulate 0
#define methrd1 0
#define methrd2 1

class Message
{

public:
    QString message;
    int role;

};

class SerialPort : public QObject
{
    Q_OBJECT
public:

    ~SerialPort();

     void initSeialPort();


     static SerialPort* getSerialPort();
    /**
     * @brief sendMessage  接收姓名和角色，根据不同的角色发出不同的欢迎词。
     * @param name
     * @param role
     */
    void sendMessage(QString name,int role);
    /**
     * @brief playSound 发出指定语音
     * @param index 语音编号
     */
    void playSound(int index);
    /**
     * @brief greet 根据不同的性别，向陌生人播放欢迎词
     * @param gender 不同性别，0表示男性，1表示女性，2表示有多人。
     */
    void greet(int gender);

private:

    SerialPort();


    void StringToHex(QString str, QByteArray &senddata);
    char ConvertHexChar(char ch);

    void initVoiceLibrary();


    void serialRead();
    void sendSerialMessage();
    void introduceMyself();
    void introduceGarden();
    void writeSerialport(QString message);
    void startIntroduce();
    void randomGreetForMeal();
    void randomGreetForFemeal();
    void randomFreetForManyPeople();
    bool startOpenSerial();
    void reWriteSerialPort();



    QByteArray _reWriteString;
    QTimer *_reWriteTimer;




     QSerialPort serial;
     bool _isSounding = false;
     bool _isintroduce = false;
     bool _startIntroduce = false;
     QList<Message> _messages;
     QList<QString> _introduce;
     int _introduceIndex = 0;
     int _index = 0;
     bool _isPlaySound = false;
     bool _isRandomGreet = false;
     QTimer *_connectSerialPortTimer;
     bool _isSerialPortOpen = false;
#if simulate
     QTimer *_simulateTimer;
#endif


private slots:

    void openSerialPort();

#if methrd2
     void reWrite();
#endif
#if simulate
    void simulateDelayForPlay();
    void simulateTimeStop();
#endif




};



#endif // SERIALPORT_H

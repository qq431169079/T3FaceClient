
#include "serialport.h"
#include "t3_log.h"



SerialPort::SerialPort()
{

}



SerialPort::~SerialPort()
{
    serial.close();
}

SerialPort* SerialPort::getSerialPort ()
{
    static SerialPort serialPort;
    return &serialPort;
}


void SerialPort::initSeialPort()
{


    _reWriteTimer = new QTimer(this);
#if methrd2
    connect (_reWriteTimer,&QTimer::timeout,this,&SerialPort::reWrite);
#endif
    serial.setBaudRate(115200);
    serial.setDataBits(QSerialPort::Data8);           //数据位8位
    serial.setFlowControl(QSerialPort::NoFlowControl);//无流控
    QSerialPortInfo info;
    QList<QSerialPortInfo> infos = QSerialPortInfo::availablePorts();
    foreach (info, infos) {
        T3LOG << info.portName();
    }
    startOpenSerial();
    introduceMyself();
#if simulate
    _simulateTimer = new QTimer(this);
    connect(_simulateTimer,&QTimer::timeout,this,&SerialPort::simulateTimeStop);
#endif
}

bool SerialPort::startOpenSerial()
{
    _connectSerialPortTimer = new QTimer(this);
    _connectSerialPortTimer->start(4000);
    connect(_connectSerialPortTimer,&QTimer::timeout,this,&SerialPort::openSerialPort);
}

void SerialPort::openSerialPort()
{
    serial.setPortName(kSerialPortName);
    if(serial.open(QIODevice::ReadWrite))
    {
        T3LOG << "serialPort Open Success";
        _connectSerialPortTimer->stop();
        connect(&serial,&QSerialPort::readyRead,this,&SerialPort::serialRead);
//        QString str("19B20400010FFF001D");//调节音量，禁止休眠。
//        QByteArray senddata;
//        StringToHex(str,senddata);
//        serial.write(senddata);
//        _reWriteString.append (senddata);
        _isSerialPortOpen = true;
    }else
    {
        serial.close();
        T3LOG << "SerialPort not open !";
    }
}

void SerialPort::sendMessage(QString message,int role)
{
    T3LOG << _isintroduce;
    T3LOG << message;
    T3LOG << role;
    T3LOG << _isSounding;
    T3LOG << _isSerialPortOpen;
    if(_isSerialPortOpen)
    {
        if(!_isintroduce)
        {
            Message message_;
            message_.role = role;
            message_.message = message;
	    bool isHaveName_ = false;
	    for(int i=0;i<_messages.size();i++)
	    {
	    	if(_messages[i].message == message)
		{
			isHaveName_ = true;
		}
	    }
	    if(!isHaveName_)
            _messages.append(message_);
            if(!_isSounding)
	    {           
                sendSerialMessage();
            }
        }
    }


}


void SerialPort::serialRead()
{
    QString sign =  serial.readAll().toHex();
    T3LOG << sign;
#if methrd1
    if("19b30200010003" == sign)
    {
        QString str("19B20400010FFF001D");//调节音量，禁止休眠。
        QByteArray senddata;
        StringToHex(str,senddata);
        serial.write(senddata);

    }
#endif

#if methrd2
    if("19b30200010003" == sign)
    {
        reWriteSerialPort ();
    }
#endif
    //19b301003031 播音结束
    if("19b301003031" == sign)
    {

        _reWriteString.clear ();
        T3LOG << "播放缓存";
        if(_isPlaySound)
        {
            _isPlaySound = false;
            _isSounding = false;
        }
        if(_isRandomGreet)
        {
            _isRandomGreet = false;
            _isSounding = false;
        }
        if(_isintroduce)
        {
            startIntroduce();
        }else
        {
            sendSerialMessage();
        }

    }

}

#if methrd2

void SerialPort::reWriteSerialPort ()
{
    _reWriteTimer->start (500);
}

void SerialPort::reWrite ()
{
    _reWriteTimer->stop ();
    serial.write (_reWriteString);
}

#endif

void SerialPort::sendSerialMessage()
{
    QString message;
    T3LOG << _messages.size();
    T3LOG << _index;
    if(_index < _messages.size()){
        _isSounding = true;
        QByteArray senddata;
        senddata.append(0x19);
        senddata.append(0xB2);

        T3LOG << _messages.at(_index).message;
        //message = "欢迎"+_messages.at(_index).message+"莅临指导。";
        T3LOG << _messages.at(_index).role;
        if(1 == _messages.at(_index).role)
        {
	 if("陈伟阳" == _messages.at(_index).message)
          message = _messages.at(_index).message+"先生您好，杉科机器人欢迎您。";
	 // _startIntroduce = true;
        
	if("郭霞" == _messages.at(_index).message)
          message = _messages.at(_index).message+"女士您好，杉科机器人欢迎您。";
         // _startIntroduce = true;
	if("郭霞" != _messages.at(_index).message&&"陈伟阳" != _messages.at(_index).message)
          message = _messages.at(_index).message+"您好，杉科机器人欢迎您。";

        }

	 if(4 == _messages.at(_index).role)
        {
          message ="我叫杉杉，来自杉科机器人，欢迎来到金领之嘟，请问有什么可以帮您。";
         // _startIntroduce = true;
        }

        T3LOG << _startIntroduce;
        T3LOG << message;
        QByteArray mess =  message.toUtf8();
        //T3LOG << mess;
        int length = message.size();
        //T3LOG << length;
        senddata.append(length*3+2);
        int s = 0x00;
        senddata.append(s);
        senddata.append(0x13);
        senddata.append(0x03);
        senddata.append(mess);
        int sum = 0;
        //T3LOG <<senddata.size();
        for(int i = 2;i<senddata.size();i++)
        {
            //T3LOG <<(unsigned char)senddata[i];
            sum+=(unsigned char)senddata[i];
            //T3LOG <<sum;
        }
        //T3LOG <<sum;
        int checkCode = sum%256;
        //T3LOG << checkCode;
        senddata.append(checkCode);


        //StringToHex(str,senddata);
        T3LOG << senddata;
        //T3LOG << senddata[0];
        serial.write(senddata);
        _reWriteString.append (senddata);
#if simulate
        simulateDelayForPlay();
#endif
        _index++;
    }else
    {

        T3LOG << "播报结束";
        _messages.clear();
        _index = 0;
        _isSounding = false;
         T3LOG << _startIntroduce;
        if(_startIntroduce)
        {

            startIntroduce();
        }

    }
}

void SerialPort::startIntroduce()
{
    T3LOG << "start introduce";
    _isintroduce = true;

    if(_introduceIndex < _introduce.size())
    {
        writeSerialport(_introduce[_introduceIndex]);
        _introduceIndex++;
    }else
    {
	_startIntroduce = false;

        _isSounding = false;
        _isintroduce = false;
        _introduceIndex = 0;
        T3LOG << "introduce end";
    }

}
void SerialPort::introduceMyself()
{
    //_introduce.append("滴嘟滴嘟滴嘟滴嘟滴嘟滴嘟滴嘟滴嘟滴嘟滴嘟滴嘟滴嘟滴嘟滴嘟滴嘟滴嘟滴嘟滴嘟滴嘟滴嘟");
    _introduce.append("各位领导好，我叫杉杉，来自杉科机器人大家庭");
    _introduce.append("接下来由我给各位领导，介绍一下新业纺");

    _introduce.append("，静安新业纺，属于区一轴三带中的，中环两翼产城融合发展集聚带");
  _introduce.append("，位居中环上的，大国际影视产业圈核心区域");
  
  _introduce.append("我们将引进各类精品商业业态，将项目建设成为，静安城市的新标签");
//    _introduce.append( "星子在无意中闪，"
//                       "细雨点洒在花前。"
//                       "那轻，那娉婷，你是，"
//                       "鲜妍百花的冠冕，你戴着，");
//    _introduce.append( "你是天真，庄严，你是夜夜的月圆。"
//                       "雪化后那篇鹅黄，你象，新鲜初放芽的绿，");
//     _introduce.append("你是，柔嫩喜悦水光浮动着你梦期待中白莲。"
//                       "你是一树一树的花开，是燕在梁间呢喃，");
//     _introduce.append("你是爱，是暖，是希望，你是人间的四月天。");

}

void SerialPort::introduceGarden()
{
    QString gardenIntroduce = "这是我美丽的出生地，杉科机器人科技公司。";
    writeSerialport(gardenIntroduce);
}

void SerialPort::writeSerialport(QString message)
{
    _isSounding = true;
    QByteArray senddata;
    senddata.append(0x19);
    senddata.append(0xB2);
    T3LOG << message;
    QByteArray mess =  message.toUtf8();
    //T3LOG << mess;
    int length = message.size();
    //T3LOG << length;
    senddata.append(length*3+2);
    int s = 0x00;
    senddata.append(s);
    senddata.append(0x13);
    senddata.append(0x03);
    senddata.append(mess);
    int sum = 0;
    //T3LOG <<senddata.size();
    for(int i = 2;i<senddata.size();i++)
    {
        //T3LOG <<(unsigned char)senddata[i];
        sum+=(unsigned char)senddata[i];
        //T3LOG <<sum;
    }
    int checkCode = sum%256;

    senddata.append(checkCode);



    T3LOG << senddata;

    serial.write(senddata);
    _reWriteString.append (senddata);

#if simulate
    simulateDelayForPlay();
#endif

}




void SerialPort::playSound(int index)
{
    if(!_isSounding)
    {
        T3LOG << "TT";
       QString sound_;
       _isPlaySound = true;
       switch (index) {
       case 1:
           sound_="语音一";
            T3LOG << "语音1";
           break;
       case 2:
           sound_="语音二";
           T3LOG << "语音2";
           break;
       case 3:
           sound_="语音三";
            T3LOG << "语音3";
           break;
       default:
           break;
       }
       writeSerialport(sound_);
    }

}


void SerialPort::greet(int gender)
{
    T3LOG << "greet";
    T3LOG << _isSerialPortOpen;
    //_isintroduce  = true;
    if(_isSerialPortOpen)
    {
        if(!_isSounding)
        {

            _isRandomGreet = true;
            switch (gender) {
            case 0:
                randomGreetForMeal();
                break;
            case 1:
                randomGreetForFemeal();
                break;
            case 2:
                randomFreetForManyPeople();
                break;
            default:
                break;
            }
        }
    }


}

void SerialPort::randomGreetForMeal()
{

    QList<QString> greetString_ ;
    greetString_.append("这位先生您好，欢迎来到杉科机器人");
    greetString_.append("这位先生您好，欢迎来到杉科机器人");
    greetString_.append("这位先生您好，欢迎来到杉科机器人");
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    int random_ =qrand()%3;
    writeSerialport(greetString_[random_]);
}

void SerialPort::randomGreetForFemeal()
{
    QList<QString> greetString_ ;
    greetString_.append("这位女士您好，欢迎来到杉科机器人");
    greetString_.append("这位女士您好，欢迎来到杉科机器人");
    greetString_.append("这位女士您好，欢迎来到杉科机器人");
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    int random_ =qrand()%3;
    writeSerialport(greetString_[random_]);
}

void SerialPort::randomFreetForManyPeople()
{
    QList<QString> greetString_ ;
    greetString_.append("欢迎来到杉科机器人");
    greetString_.append("欢迎来到杉科机器人");
    greetString_.append("欢迎来到杉科机器人");
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    int random_ =qrand()%3;
    writeSerialport(greetString_[random_]);
}


void SerialPort::StringToHex(QString str, QByteArray &senddata) //字符串转换为十六进制数据0-F
{
    int hexdata,lowhexdata;
    int hexdatalen = 0;
    int len = str.length();
    senddata.resize(len/2);
    char lstr,hstr;

    for(int i=0; i<len; )
    {
        //char lstr,
        hstr=str[i].toLatin1();
        if(hstr == ' ')
        {
            i++;
            continue;
        }
        i++;
        if(i >= len)
            break;
        lstr = str[i].toLatin1();
        hexdata = ConvertHexChar(hstr);
        lowhexdata = ConvertHexChar(lstr);
        if((hexdata == 16) || (lowhexdata == 16))
            break;
        else
            hexdata = hexdata*16+lowhexdata;
        i++;
        senddata[hexdatalen] = (char)hexdata;
        hexdatalen++;
    }
    senddata.resize(hexdatalen);
}

char SerialPort::ConvertHexChar(char ch)
{
    if((ch >= '0') && (ch <= '9'))
        return ch-0x30;
    else if((ch >= 'A') && (ch <= 'F'))
        return ch-'A'+10;
    else if((ch >= 'a') && (ch <= 'f'))
        return ch-'a'+10;
    else return ch-ch;//不在0-f范围内的会发送成0
}

#if simulate
void SerialPort::simulateDelayForPlay()
{
    T3LOG << "播音开始";
    _simulateTimer->start(4000);

}

void SerialPort::simulateTimeStop()
{
    T3LOG << "播音结束";
    _simulateTimer->stop();
    _isSounding = false;
}

#endif

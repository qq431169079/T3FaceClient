#ifndef T3_LIBRARY_H
#define T3_LIBRARY_H

//Database

#define kDatabaseName "T3Face"
//#define kDatabaseName "face"
#define kDatabaseUserName "T3"
#define kDatabasePassword "T3"
#define kDatabaseEngine "QMYSQL"


//Server
//#define kServerURL "192.168.0.210"
//#define kServerURL "192.168.0.192"
//#define kServerURL "192.168.43.246"
#define kServerURL "localhost"
//#define kServerURL "192.168.50.106"
#define kServerPort 6666

//FaceEngine

#define kThreshold 0.62f


//MainWindow

#define kVideoPoint_X 200
#define kVideoPoint_Y 170
#define kVideoWidth 480
#define kVideoHeight 640
#define kBackgroundImage ":/bg/bg/bg.jpg"

#define kInterval 30    //识别记录及语音输出的时间间隔。


#define kGenderFrameNum 25
#define kAgeFrameNum 25;
#define kFaceNum 25;


#define kSerialPortName "ttyUSBT3"




#endif // T3_LIBRARY_H




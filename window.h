#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include "arcfaceengine.h"
#include "t3_face_database.h"
#include "t3_face_network.h"
#include "t3_face_tts.h"

/**
  ******************************************************************************
  * @author  HXP
  * @version  V1.0.0
  * @date    2018/01/02
  * @brief  显示界面主窗口
  ******************************************************************************
*/

class MainWidget;

class Window : public QWidget
{
	Q_OBJECT

public:
	Window();
    ~Window();
private:
    MainWidget *mainWidget;
    ArcFaceEngine *_faceEngine;
    T3_Face_Database *_database;
    T3_Face_Network *_network;
    T3_Face_TTS *_tts;

};

#endif

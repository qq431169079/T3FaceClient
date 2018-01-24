#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

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

};

#endif

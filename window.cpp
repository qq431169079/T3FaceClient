#include <QtWidgets>

#include "mainwidget.h"
#include "window.h"
#include "t3_log.h"
#include "t3_library.h"


Window::Window()
{
    //设置界面
    this->showFullScreen();
    int width_ = QApplication::desktop()->width();
    int height_ = QApplication::desktop()->height();
    mainWidget = new MainWidget(this);
    mainWidget->setGeometry(200,kVideoPoint_Y,kVideoWidth,kVideoHeight);
    mainWidget->show();
    QImage bgImage;
    bgImage.load(kBackgroundImage);
    this->setAutoFillBackground(true);
    QPalette pal(palette());
    pal.setBrush(QPalette::Window,QBrush(bgImage.scaled(width_,height_,Qt::IgnoreAspectRatio,Qt::SmoothTransformation)));
    setPalette(pal);
    setWindowTitle(tr("T3_Face"));
}

Window::~Window()
{
    if(mainWidget){
        delete mainWidget;
        mainWidget = nullptr;
    }

}

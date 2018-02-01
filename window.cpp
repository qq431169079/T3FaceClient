#include <QtWidgets>

#include "mainwidget.h"
#include "window.h"
#include "t3_log.h"
#include "t3_library.h"
#include <QCursor>
Window::Window()
{
    //设置界面
    this->showFullScreen();
    int width_ = QApplication::desktop()->width();
    int height_ = QApplication::desktop()->height();
    this->setCursor(Qt::BlankCursor);
    mainWidget = new MainWidget(this);
    mainWidget->show();
    QImage bgImage;
    bgImage.load(kBackgroundImage);
    this->setAutoFillBackground(true);
    QPalette pal(palette());
    pal.setBrush(QPalette::Window,QBrush(bgImage.scaled(width_,height_,Qt::IgnoreAspectRatio,Qt::SmoothTransformation)));
    setPalette(pal);
    setWindowTitle(tr("T3_Face"));

    //初始化相关单例
    _faceEngine = ArcFaceEngine::getFaceEngine();
    _database = T3_Face_Database::getDatabase();
    _network = T3_Face_Network::getNetwork();
    _tts = T3_Face_TTS::getTTS();
}

Window::~Window()
{
    if(mainWidget)
    {
        delete mainWidget;
        mainWidget = nullptr;
    }
    if(_faceEngine)
    {
        delete _faceEngine;
        _faceEngine = nullptr;
    }
    if(_database)
    {
        delete _database;
        _database = nullptr;
    }
    if(_network)
    {
        delete _network;
        _network = nullptr;
    }
    if(_tts)
    {
        delete _tts;
        _tts = nullptr;
    }

}

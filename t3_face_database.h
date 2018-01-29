#ifndef T3_FACE_DATABASE_H
#define T3_FACE_DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "t3_library.h"
#include "t3_log.h"
#include "arcfaceengine.h"

/**
  ******************************************************************************
  * @author  HXP
  * @version  V1.0.0
  * @date    2018/01/29
  * @brief   数据库相关操作函数
  ******************************************************************************
*/

class T3_Face_Database : public QObject
{
public:
    /**
     * @brief getDatabase　得到一个数据库对象的单例。
     * @return　返回一个对象指针。
     */
    static T3_Face_Database * getDatabase();
    /**
     * @brief loadFaceDatabase　加载人脸数据库
     * @return 加载数据的条数
     */
    int loadFaceDatabase();

    ~T3_Face_Database();


private:
    T3_Face_Database(QObject *parent = 0);
    bool connectDatabasae();

    QSqlDatabase _database;
    ArcFaceEngine *_faceEngine;


};

#endif // T3_FACE_DATABASE_H

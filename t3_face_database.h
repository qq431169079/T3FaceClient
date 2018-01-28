#ifndef T3_FACE_DATABASE_H
#define T3_FACE_DATABASE_H

#include <QObject>
class T3_Face_Database : public QObject
{
public:
    static T3_Face_Database * getDatabase();

    ~T3_Face_Database();

private:
    T3_Face_Database(QObject *parent = 0);

};

#endif // T3_FACE_DATABASE_H

#include "t3_face_database.h"

T3_Face_Database::T3_Face_Database(QObject *parent)
{

}

T3_Face_Database* T3_Face_Database::getDatabase()
{
    static T3_Face_Database database;
    return &database;
}

T3_Face_Database::~T3_Face_Database()
{

}

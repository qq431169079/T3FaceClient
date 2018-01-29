#include "t3_face_database.h"

T3_Face_Database::T3_Face_Database(QObject *parent)
{
    connectDatabasae();
    _faceEngine = ArcFaceEngine::getFaceEngine();
    T3LOG << "database init success";
    loadFaceDatabase();
}

T3_Face_Database* T3_Face_Database::getDatabase()
{
    static T3_Face_Database database;
    return &database;
}

bool T3_Face_Database::connectDatabasae()
{
    _database = QSqlDatabase::addDatabase(kDatabaseEngine);
    _database.setDatabaseName(kDatabaseName);
    _database.setHostName(kServerURL);
    _database.setUserName(kDatabaseUserName);
    _database.setPassword(kDatabasePassword);
    if(!_database.open())
    {
        T3LOG << "database not open";
        return false;

    }
    return true;
}

int T3_Face_Database::loadFaceDatabase()
{
    QSqlQuery query(_database);
    query.exec("select * from T3Face where state > 0");
    while (query.next())
    {
        int id_ = query.value(0).toInt();
        QString name_ = query.value(1).toString();
        QString role_ = query.value(2).toString();
        QByteArray feature_ = query.value(4).toByteArray();
        int num_ = query.value(6).toInt();
        QString dateTime_ = query.value(7).toString();
        _faceEngine->addFace(id_,name_,role_,feature_,num_,dateTime_);
        _faceEngine->mUniqueIncID = id_ ;
        _faceEngine->_isLoadFace = true;
        T3LOG << id_ ;
        T3LOG << name_ ;
    }
}

T3_Face_Database::~T3_Face_Database()
{

}

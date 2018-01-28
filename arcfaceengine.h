#ifndef ARCFACEENGINE_H
#define ARCFACEENGINE_H

#include <QObject>
#include <QVector>
#include <QDateTime>
#include <QMap>
#include <QMapIterator>
#include "asvloffscreen.h"
#include "arcsoft_fsdk_face_recognition.h"
#include "arcsoft_fsdk_face_tracking.h"
#include "arcsoft_fsdk_age_estimation.h"
#include "t3_face_tts.h"
#define MAX_FT_FACE (50)



class FaceInfo{
public:
    unsigned char *pFeature;
    int featureSize;
    QString name;
    int id;
    int role;
    int num;
    QString dateTime;
    FaceInfo(){
        pFeature = NULL;
        id = -1;
    }
 };



class ArcFaceEngine : public QObject
{
    Q_OBJECT
public:
    bool _autoRegister = false;
    float mThreshold;
    bool _databaseReady_ = false;
    bool _isLoadFace = false;


    explicit ArcFaceEngine(QObject *parent);
    virtual ~ArcFaceEngine();
    QMap<int,FaceInfo> mRegisterFaces;
    //QVector<FaceInfo> mRegisterFaces;
    bool processFrame(unsigned char *frameData,int frameWidth,int frameHeight,int frameFormat);
    void removeFace(int id);
    int loadDB();
    void addFace(int id,
                 QString name,
                 int role,
                 QByteArray feature,
                 int num,
                 QString dateTime);
    void updateFaceName(int id,QString name, AFR_FSDK_FACEMODEL *localFaceModels);
    int registerFace(unsigned char *frameData,int frameWidth,int frameHeight,int frameFormat,MRECT *pRect,int faceOrient,AFR_FSDK_FACEMODEL *localFaceModels);
    int recognitionFace(unsigned char *frameData,int frameWidth,int frameHeight,int frameFormat,MRECT *pRect,int faceOrient,int *pID,float *pfScore);

    MRECT mFaceRect[MAX_FT_FACE];
    int mFaceOrient[MAX_FT_FACE];
    QString mFaceName[MAX_FT_FACE];
    float mScore[MAX_FT_FACE];
    int mRole[MAX_FT_FACE];
    int mFaceID[MAX_FT_FACE];
    int mFaceState[MAX_FT_FACE];
    int mFramenum[MAX_FT_FACE];
    int mFaceGrender[MAX_FT_FACE];
    int mFaceAge[MAX_FT_FACE];
    int mFaceNum;
    int mUniqueIncID;
    QByteArray _feature_ = 0;
    int frameNum = 0;
    bool oldFrame = false;
    int fframeNum = 0;




signals:
    void newFace(int id ,QString name, QByteArray feature);

private:
    void *hFTEngine;
    unsigned char *mFTWorkMem;

    void *hFREngine;
    unsigned char *mFRWorkMem;

    void *hFGEngine;
    unsigned char *mFGWorkMem;

    void *hFAEngine;
    unsigned char *mFAWorkMem;

    bool bFaceChange = false;
    int _faceIndex = 0;
    int _ageIndex = 0;
    int _genderIndex = 0;
    int _mealNum[MAX_FT_FACE] = {0};
    int _femealNum[MAX_FT_FACE] = {0};
    int _ageCount[MAX_FT_FACE] = {0};
    bool _genderStart = false;
    bool _ageStart = false;
    bool _faceStart = false;
    int _frameNum = 0;
    bool _isNewFace = false;
    int _genderSign = -1;
    bool _faceNumChange = false;
    T3_Face_TTS *_tts;


    bool bReady;
    int doFRFrameCount;
    int computeOverlapArea(int leftA, int bottomA, int rightA, int topA, int leftB, int bottomB, int rightB, int topB);
    QString getFaceNameByID(int id);
    int ExtractFRFeature(unsigned char *frameData,int frameWidth,int frameHeight,int frameFormat,MRECT *pRect,int faceOrient,AFR_FSDK_FACEMODEL *pFaceModels);
};

#endif // ARCFACEENGINE_H

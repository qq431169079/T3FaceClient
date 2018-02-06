#include <QDebug>
#include <QElapsedTimer>
#include <QImage>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QBuffer>

#include "arcfaceengine.h"
#include "arcsoft_fsdk_face_recognition.h"
#include "arcsoft_fsdk_face_tracking.h"
#include "arcsoft_fsdk_gender_estimation.h"
#include "key.h"
#include "t3_log.h"


ArcFaceEngine::ArcFaceEngine()
{

     doFRFrameCount = 0;
     bReady = false;
     mThreshold = 0.55f;
     mFaceNum = 0;
     for(int k=0;k<MAX_FT_FACE;k++){
         mFaceID[k] = -1;
         _femealNum[k] = 0;
         _mealNum[k] = 0;
         _ageCount[k] = 0;
     }

     mUniqueIncID = 0;

     int ft_workmem_size = 20 * 1024 * 1024;
     mFTWorkMem = new unsigned char[ft_workmem_size];
     int fr_workmem_size = 40 * 1024 * 1024;
     mFRWorkMem = new unsigned char[fr_workmem_size];
     int fg_workmem_size = 30 * 1024 * 1024;
     mFGWorkMem = new unsigned char[fg_workmem_size];
     int fa_workmem_size = 30 * 1024 * 1024;
     mFAWorkMem = new unsigned char[fa_workmem_size];

     hFTEngine = nullptr;
     hFREngine = nullptr;
     hFGEngine = nullptr;
     hFAEngine = nullptr;
     _tts = T3_Face_TTS::getTTS();


     int ret = AFT_FSDK_InitialFaceEngine((MPChar)APPID, (MPChar)FT_SDKKEY, mFTWorkMem, ft_workmem_size, &hFTEngine, AFT_FSDK_OPF_0_HIGHER_EXT, 16, MAX_FT_FACE);
     if(ret != 0){
         qDebug() <<"fail to AFT_FSDK_InitialFaceEngine():"<<ret;
         return;
     }


     ret = AFR_FSDK_InitialEngine((MPChar)APPID, (MPChar)FR_SDKKEY, mFRWorkMem, fr_workmem_size, &hFREngine);
     if(ret != 0){
         T3LOG <<"fail to AFR_FSDK_InitialEngine():"<<ret;
         return;
     }

     ret = ASGE_FSDK_InitGenderEngine((MPChar)APPID, (MPChar)FG_SDKKEY, mFGWorkMem, fg_workmem_size, &hFGEngine);
     if (ret != 0) {
         T3LOG << stderr <<  "fail to ASGE_FSDK_InitGenderEngine():" << ret;
         free(mFGWorkMem);
         exit(0);
     }
     ret = ASAE_FSDK_InitAgeEngine((MPChar)APPID, (MPChar)FA_SDKKEY, mFAWorkMem, fa_workmem_size, &hFAEngine);
     if (ret != 0) {
         T3LOG << "fail to ASAE_FSDK_InitAgeEngine(): 0x%x\r\n" << ret;
         free(mFAWorkMem);
         exit(0);
     }


     bReady = true;

}

ArcFaceEngine * ArcFaceEngine::getFaceEngine()
{
    static ArcFaceEngine faceEngine;
    return &faceEngine;
}

ArcFaceEngine::~ArcFaceEngine(){

    if (hFTEngine) {
        AFT_FSDK_UninitialFaceEngine(hFTEngine);
    }

    if (hFREngine) {
        AFR_FSDK_UninitialEngine(hFREngine);
    }
    if (hFGEngine) {
        ASGE_FSDK_UninitGenderEngine(hFGEngine);
    }
    if(hFAEngine)  {
        ASAE_FSDK_UninitAgeEngine(hFAEngine);
    }

    if(mFTWorkMem){
        delete[] mFTWorkMem;
        mFTWorkMem = nullptr;
    }

    if(mFRWorkMem){
        delete[] mFRWorkMem;
        mFRWorkMem = nullptr;
    }
    if(mFGWorkMem){
        delete[] mFGWorkMem;
        mFGWorkMem = nullptr;
    }
    if(mFAWorkMem){
        delete[] mFAWorkMem;
        mFAWorkMem = nullptr;
    }

    int facenum = mRegisterFaces.count();
    for (int i = 0; i <facenum; i++) {
         FaceInfo faceinfo = mRegisterFaces[i];
         delete faceinfo.pFeature;
    }
}

void ArcFaceEngine::removeFace(int id){
    int facenum = mRegisterFaces.count();
    for (int i = 0; i <facenum; i++) {
        FaceInfo faceinfo = mRegisterFaces[i];
        if(faceinfo.id == id){
            delete[] faceinfo.pFeature;
            mRegisterFaces.remove(i);
            break;
        }
    }
}



bool ArcFaceEngine::processFrame(unsigned char *frameData,int frameWidth,int frameHeight,int frameFormat){
   if(!bReady){
       return false;
   }
   if(!_isLoadFace&&!_isNewFace)
   {
    _isNewFace = true;
   }

    _frameNum++;
   int frameSize = 0;
   ASVLOFFSCREEN inputImg = { 0 };
   ASGE_FSDK_GENDERFACEINPUT genderFaceInput = { 0 };
   ASAE_FSDK_AGEFACEINPUT ageFaceInput = { 0 };

   inputImg.u32PixelArrayFormat = frameFormat;
   inputImg.i32Width = frameWidth;
   inputImg.i32Height = frameHeight;
   inputImg.ppu8Plane[0] = frameData;
   inputImg.pi32Pitch[0] = inputImg.i32Width * 2;
   frameSize = inputImg.i32Width*inputImg.i32Height*2;


   LPAFT_FSDK_FACERES faceResult = nullptr;

   int ret = AFT_FSDK_FaceFeatureDetect(hFTEngine, &inputImg, &faceResult);


   if (ret != 0) {
       mFaceNum = 0;

   } else {




       //FaceGenderInput
       genderFaceInput.lFaceNumber = faceResult->nFace;
       MRECT faceRectArray[faceResult->nFace];
       MInt32 faceOrientArray[faceResult->nFace];
       genderFaceInput.pFaceRectArray = faceRectArray;
       genderFaceInput.pFaceOrientArray = faceOrientArray;



       //FaceOld
       int oldFaceNum = mFaceNum;
       MRECT oldFaceRect[MAX_FT_FACE];
       int oldFaceID[MAX_FT_FACE];
       memcpy(oldFaceRect,mFaceRect,sizeof(MRECT)*oldFaceNum);
       for(int k=0;k<oldFaceNum;k++){
           oldFaceID[k] = mFaceID[k];
       }

       mFaceNum = faceResult->nFace;


       for (int i = 0; i < mFaceNum; i++) {

           mFaceOrient[i] = faceResult->lfaceOrient;
           mFaceRect[i].left = faceResult->rcFace[i].left;
           mFaceRect[i].top = faceResult->rcFace[i].top;
           mFaceRect[i].right = faceResult->rcFace[i].right;
           mFaceRect[i].bottom = faceResult->rcFace[i].bottom;
           genderFaceInput.pFaceRectArray[i].left = faceResult->rcFace[i].left;
           genderFaceInput.pFaceRectArray[i].top = faceResult->rcFace[i].top;
           genderFaceInput.pFaceRectArray[i].right = faceResult->rcFace[i].right;
           genderFaceInput.pFaceRectArray[i].bottom = faceResult->rcFace[i].bottom;
           genderFaceInput.pFaceOrientArray[i] = faceResult->lfaceOrient;

       }
        ageFaceInput.lFaceNumber = genderFaceInput.lFaceNumber;
        ageFaceInput.pFaceOrientArray = genderFaceInput.pFaceOrientArray;
        ageFaceInput.pFaceRectArray = genderFaceInput.pFaceRectArray;
	if(mFaceNum !=oldFaceNum)
        {
            _faceNumChange  = true;
        }else
        {
            _faceNumChange  = false;
        }
       for(int i = 0;i < mFaceNum; i++)
       {
//		T3LOG << i;
            int maxOverlapArea = 0;
            int maxOverlapIndex = 0;
            int halfArea = ((mFaceRect[i].bottom - mFaceRect[i].top)*(mFaceRect[i].right - mFaceRect[i].left)) / 4;
//		T3LOG << oldFaceNum;
// 		T3LOG << mFaceRect[i].left;
                //T3LOG << oldFaceRect[k].left;
            for (int k = 0; k < oldFaceNum; k++) {
//                T3LOG << oldFaceRect[k].left;
		int area = computeOverlapArea(mFaceRect[i].left, mFaceRect[i].bottom,
                    mFaceRect[i].right, mFaceRect[i].top,
                    oldFaceRect[k].left, oldFaceRect[k].bottom,
                    oldFaceRect[k].right, oldFaceRect[k].top);
                if (area > maxOverlapArea) {
                    maxOverlapArea = area;
                    maxOverlapIndex = k;
                }
            }
//		T3LOG << maxOverlapIndex;
            if (maxOverlapArea <= halfArea || _faceNumChange) {
//                T3LOG << mFaceID[i];
//                T3LOG << mFaceName[i];
                mFaceID[i] = -1;
                bFaceChange = true;
                if(_isLoadFace)
                {
                    _genderStart = false;
                    _ageStart = false;
                    _faceStart = true;
                     _isNewFace = false;
                }else
                {
                    _genderStart = true;
                    _ageStart = false;
                    _faceStart = false;
                }


                mFaceName[i] = "";
                _genderSign = -1;
                _genderIndex = 0;
                _ageIndex = 0;
                _faceIndex = 0;
                T3LOG << "bFaceChange";

                _mealNum[i] = 0;
                _femealNum[i] = 0;
                _ageCount[i] = 0;

            } else {
//		T3LOG << i;
//                T3LOG << mFaceID[i];
//                T3LOG << mFaceName[i];
                mFaceID[i] = oldFaceID[maxOverlapIndex];
            }

       }



        if(bFaceChange){

        ASGE_FSDK_GENDERRESULT  genderResult = { 0 };
        if(_genderStart)
        {
            if(_frameNum%2 == 0)
            {


                ret = ASGE_FSDK_GenderEstimation_StaticImage(hFGEngine, &inputImg, &genderFaceInput, &genderResult);
                _genderIndex++;
                for (int i = 0; i < genderResult.lFaceNumber; i++) {
                    if(0 == genderResult.pGenderResultArray[i])
                    {
                        _mealNum[i]++;
                    }
                    if(1 == genderResult.pGenderResultArray[i])
                    {
                        _femealNum[i]++;
                    }


                }
                if(3 == _genderIndex)
                {
                    _genderIndex = 0;

                    _genderStart = false;
                    _ageStart = true;
                    if(genderResult.lFaceNumber >1)
                    {
                        _genderSign = 2;
                    }
                    if(genderResult.lFaceNumber == 1)
                    {
                        _genderSign = genderResult.pGenderResultArray[0];
                    }
                    for(int i =0;i < genderResult.lFaceNumber;i++)
                    {
                        if(_mealNum[i] > _femealNum[i])
                        {
                            mFaceGrender[i] = 0;

                            _mealNum[i] = _femealNum[i] = 0;
                        }else
                        {
                            mFaceGrender[i] = 1;
                            _mealNum[i] = _femealNum[i] = 0;
                        }
                    }
                }
            }
        }

            if (ret != 0) {
                T3LOG << "fail to ASGE_FSDK_GenderEstimation_StaticImage():" << ret;

            }


        ASAE_FSDK_AGERESULT  ageResult = { 0 };
        if(_ageStart)
        {
            if(_frameNum%2 == 0)
            {
                ret = ASAE_FSDK_AgeEstimation_StaticImage(hFAEngine, &inputImg, &ageFaceInput, &ageResult);
                _ageIndex++;
                for(int i = 0;i< ageResult.lFaceNumber;i++)
                {
                    _ageCount[i]+=ageResult.pAgeResultArray[i];

                }
                if(3 == _ageIndex)
                {
                    _ageIndex = 0;
                     _ageStart = false;
                    if(_isLoadFace)
                    {
                        bFaceChange = false;

                    }else
                    {
                        _faceStart = true;
                    }


                    for(int i = 0;i<ageResult.lFaceNumber;i++)
                    {
                        mFaceAge[i] = _ageCount[i]/3;
                        T3LOG << mFaceAge[i];
                        _ageCount[i] = 0;
                    }
                    T3LOG << _isNewFace;
                    T3LOG << bFaceChange;
                    if(_isNewFace)
                    {
                        _tts->inputToText("",_genderSign,"");
                    }
                }
            }


        }

            if (ret != 0) {
                T3LOG << "fail to ASAE_FSDK_AgeEstimation_StaticImage(): 0x%x\r\n " << ret;
                exit(0);
            }

       if(fframeNum == 10)
       {
           fframeNum = 0;
           frameNum = 0;
       }

       if((doFRFrameCount>=10)&&(mRegisterFaces.count()>0)&&_faceStart)
       {
            T3LOG << "face";
            _faceIndex++;
            fframeNum++;
            if(3 == _faceIndex)
            {
                _faceStart = false;
                if(_isLoadFace)
                {
                    _genderStart = true;
                }else
                {
                    bFaceChange = false;
                }
            }


           doFRFrameCount = 0;
           for (int i = 0; i < mFaceNum; i++) {
               AFR_FSDK_FACEMODEL localFaceModels = { 0 };
               int ret = ExtractFRFeature(frameData,frameWidth,frameHeight,frameFormat,&mFaceRect[i],mFaceOrient[i],&localFaceModels);
               if(ret == 0){
                   float fMaxScore = 0.0f;
                   int iMaxIndex = -1;
                   int iMax2Index = -1;
                   int j = 0;
                   QMapIterator<int, FaceInfo> iterrator(mRegisterFaces);
                   for (;iterrator.hasNext();) {
                       j++;
                       FaceInfo faceinfo = iterrator.next().value();
                       float fScore = 0.0f;
                       AFR_FSDK_FACEMODEL targetFaceModels = { 0 };
                       targetFaceModels.lFeatureSize = faceinfo.featureSize;
                       targetFaceModels.pbFeature = faceinfo.pFeature;
                       ret = AFR_FSDK_FacePairMatching(hFREngine, &targetFaceModels, &localFaceModels, &fScore);

                       //T3LOG << score;
                       if(ret == 0){
                           if(fScore>fMaxScore){
                               iMax2Index = iMaxIndex;
                               iMaxIndex = iterrator.key();
                               fMaxScore = fScore;
                           }
                       }
                   }

                   if((iMaxIndex>=0)&&(fMaxScore>mThreshold)){
                       T3LOG << mFaceID[i];
                       T3LOG << mFaceName[i];
                       if(-1 == mFaceID[i])
                       {
                           mFaceID[i] = mRegisterFaces[iMaxIndex].id;
                           mFaceName[i] = mRegisterFaces[iMaxIndex].name;
                           mRole[i] =  mRegisterFaces[iMaxIndex].role;
                           T3LOG << mRole[i];
                           T3LOG << fMaxScore;
                           mScore[i] = fMaxScore;
                           _tts->inputToText(mRole[i],-1,mFaceName[i]);
                        //serialPort->sendMessage(mFaceName[i],mRole[i]);
                           emit log(mFaceID[i],mFaceName[i]);
                       }




                   }else{

                       if(-1 == mFaceID[i])
                       {
                           mFaceID[i] = -1;
                           mFaceName[i] = "";
                           mScore[i] = fMaxScore;
                       }

                        T3LOG << fMaxScore;
                       frameNum ++;
                       if(3 == _faceIndex)
                        _isNewFace = true;
                        if(frameNum == 10){
                            frameNum = 0;
                            T3LOG  << "new Face";
                            T3LOG  << fMaxScore;


                            //QString nameID = QString("%1").arg(mUniqueIncID);
                            if(_autoRegister)
                            {
                                 mUniqueIncID++;
                                updateFaceName(mUniqueIncID,"未注册",&localFaceModels);
                            }


                        }

                   }

               }else{

               }
           }
       }
    }
   }
    doFRFrameCount++;

    return true;
}


int ArcFaceEngine::ExtractFRFeature(unsigned char *frameData,int frameWidth,int frameHeight,int frameFormat,MRECT *pRect,int faceOrient,AFR_FSDK_FACEMODEL *pFaceModels){

    ASVLOFFSCREEN inputImg = { 0 };
    inputImg.u32PixelArrayFormat = frameFormat;
    inputImg.i32Width = frameWidth;
    inputImg.i32Height = frameHeight;
    inputImg.ppu8Plane[0] = frameData;
    inputImg.pi32Pitch[0] = inputImg.i32Width * 2;

    AFR_FSDK_FACEINPUT faceInput;
    faceInput.lOrient = faceOrient;
    faceInput.rcFace.left = pRect[0].left;
    faceInput.rcFace.top = pRect[0].top;
    faceInput.rcFace.right = pRect[0].right;
    faceInput.rcFace.bottom = pRect[0].bottom;

    return AFR_FSDK_ExtractFRFeature(hFREngine, &inputImg, &faceInput, pFaceModels);
}



void ArcFaceEngine::addFace(int id,
                            QString name,
                            QString role,
                            QByteArray feature,
                            int num,
                            QString dateTime)
{
    FaceInfo faceInfo;
    faceInfo.name = name;
    faceInfo.role = role;
    faceInfo.id = id;
    faceInfo.featureSize = feature.size();
    faceInfo.pFeature = new MByte[faceInfo.featureSize];
    faceInfo.num = num;
    faceInfo.dateTime = dateTime;
    memcpy(faceInfo.pFeature, feature.data(), faceInfo.featureSize);
    mRegisterFaces.insert(id,faceInfo);
    T3LOG << mRegisterFaces.count();

}


void ArcFaceEngine::updateFaceName( int id,
                                    QString name,
                                   AFR_FSDK_FACEMODEL *localFaceModels)
{
    FaceInfo faceinfo ;
    faceinfo.id = id;
    faceinfo.role = 3;
    faceinfo.name = name ;
    faceinfo.featureSize = localFaceModels->lFeatureSize;
    faceinfo.pFeature = new MByte[faceinfo.featureSize];
    faceinfo.num = 0;
    memcpy(faceinfo.pFeature, localFaceModels->pbFeature, faceinfo.featureSize);
    mRegisterFaces.insert(id,faceinfo);


    _feature_.resize(faceinfo.featureSize);
    memcpy(_feature_.data(),faceinfo.pFeature,faceinfo.featureSize);
    QSqlDatabase database_ = QSqlDatabase::database();
    if(!database_.open())
    {
        T3LOG << "database open failed";
    }
    QSqlQuery query_(database_);
    query_.prepare("insert into T3Face values(NULL,?,?,?,?,?,?,?)");
    T3LOG << name;
    query_.bindValue(0,name);
    query_.bindValue(1,"访客");
    query_.bindValue(2,0);
    query_.bindValue(3,_feature_,QSql::Binary);
    query_.bindValue(4,1);
    query_.bindValue(5,0);
    QString datetime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    query_.bindValue(6,datetime);
    query_.exec();
}

int ArcFaceEngine::computeOverlapArea(int leftA, int bottomA, int rightA, int topA, int leftB, int bottomB, int rightB, int topB) {
    if ((leftB >= rightA) || (topA >= bottomB) || (topB >= bottomA) || (leftA >= rightB)) return 0;
    return ((qMin(rightB, rightA) - qMax(leftA, leftB)) * (qMin(bottomA, bottomB) - qMax(topA, topB)));
}



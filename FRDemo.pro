HEADERS       = \
                window.h \
                glhelper.h \
    camerasource.h \
    mainwidget.h \
    framesource.h \
    arcfaceengine.h \
    key.h \
    arcsoft_fsdk_face_recognition.h \
    arcsoft_fsdk_face_tracking.h \
    netsdk.h \
    encoder.h \
    t3_log.h \
    t3_library.h \
    serialport.h \
    arcsoft_fsdk_gender_estimation.h \
    arcsoft_fsdk_age_estimation.h \
    t3_face_network.h \
    t3_face_tts.h \
    msp_cmn.h \
    msp_errors.h \
    msp_types.h \
    qtts.h

SOURCES       = \
                main.cpp \
                window.cpp \
                glhelper.cpp \
    camerasource.cpp \
    mainwidget.cpp \
    arcfaceengine.cpp \
    encoder.cpp \
    serialport.cpp \
    t3_face_network.cpp \
    t3_face_tts.cpp

QT           += widgets  multimedia sql network serialport
win32:LIBS       +=  -L../T3FaceClient -llibarcsoft_fsdk_face_tracking -llibarcsoft_fsdk_face_recognition
unix:LIBS        +=  -L../T3FaceClient -larcsoft_fsdk_face_tracking -larcsoft_fsdk_face_recognition -lxmnetsdk
unix:LIBS        +=  -L../T3FaceClient -larcsoft_fsdk_gender_estimation -larcsoft_fsdk_age_estimation -lmsc
unix|win32: LIBS += -lavcodec  -lavutil -lswscale



RESOURCES += \
    t3_face_logo.qrc
RC_FILE += t3_face_logo.rc

QMAKE_CXXFLAGS += -std=c++0x



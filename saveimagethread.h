#ifndef SAVEIMAGETHREAD_H
#define SAVEIMAGETHREAD_H


#include <QThread>

class saveImageThread :QThread
{
public:
    saveImageThread();
    void saveImage(unsigned char *frameData ,int width,int height);

protected:
    void run();
};

#endif // SAVEIMAGETHREAD_H

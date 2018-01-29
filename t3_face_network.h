#ifndef T3_FACE_NETWORK_H
#define T3_FACE_NETWORK_H

#include <QUdpSocket>
#include <QTcpSocket>
#include <QHostAddress>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDataStream>
#include <QTimer>

#define kServerURL "localhost"
#define kServerPort 6666


class T3_Face_Network : public QObject
{
    Q_OBJECT
public:
    static T3_Face_Network *getNetwork();

    ~T3_Face_Network();
    /**
     * @brief 使用UDP方式传输数据
     * @param data :传输的数据首地址
     * @param dataSize:传输数据的大小
     * @return　成功传输的数据大小，如果传输失败则返回-1
     */
    int sendDataByUDP(char* data,
                 int dataSize);
    /**
     * @brief sendDataByTcp　使用TCP协议传输数据
     * @param jsonObject 传输数据的JSON对象
     * @return
     */
    int sendDataByTcp(QJsonObject jsonObject);

private:
    T3_Face_Network();
    QUdpSocket *_udpSocket;
    QTcpSocket *_tcpSocket;
    bool _netWorkState = false;
    QTimer *_connectNetworkTimer;
    quint32 _blockSize = 0;
    QString _jsonDocument;
    QByteArray _networkDataBuffer;

private slots:
    void startConnectNetWork();
    void stopConnectNetWork();
    void disposeDisconnectNetwork();
    void readNetworkData();
    void connectNetwork();

signals:
    void connectedNetwork();
    void disconnectNetwork();
    void getNetworkData(QByteArray jsonByteArray);
};

#endif // T3_FACE_NETWORK_H

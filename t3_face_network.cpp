#include "t3_face_network.h"

T3_Face_Network::T3_Face_Network()
{
    _udpSocket = new QUdpSocket(this);
    _tcpSocket = new QTcpSocket(this);
    _connectNetworkTimer = new QTimer(this);

    //startConnectNetWork();
    connect(_connectNetworkTimer,&QTimer::timeout,this,&T3_Face_Network::connectNetwork);
    connect(_tcpSocket,&QTcpSocket::connected,this,&T3_Face_Network::stopConnectNetWork);
    connect(_tcpSocket,&QTcpSocket::disconnected,this,&T3_Face_Network::disposeDisconnectNetwork);
    connect(_tcpSocket,&QTcpSocket::readyRead,this,&T3_Face_Network::readNetworkData);
}


int T3_Face_Network::sendDataByUDP(char* data,
                                   int dataSize)
{

    qDebug() << "udp";
//    return _udpSocket->writeDatagram(data,dataSize,QHostAddress::LocalHost,8888);

    return _udpSocket->writeDatagram(data,dataSize,QHostAddress("192.168.0.192"),8888);


}

int T3_Face_Network::sendDataByTcp(QJsonObject jsonObject)
{
    QJsonDocument document_;
    document_.setObject(jsonObject);
    QByteArray jsonByteArray_ = document_.toJson(QJsonDocument::Compact);
    QByteArray block_;
    QDataStream stream_(&block_,QIODevice::WriteOnly);
    stream_.setVersion(QDataStream::Qt_5_5);
    stream_ << (quint32) 0;
    stream_ << jsonByteArray_;
    stream_.device()->seek(0);
    stream_ << (quint32) block_.size();

}

void T3_Face_Network::startConnectNetWork()
{
    _netWorkState = false;
    _connectNetworkTimer->start(4000);

}

void T3_Face_Network::stopConnectNetWork()
{
    _netWorkState = true;
    _connectNetworkTimer->stop();
    emit connectedNetwork();
}

void T3_Face_Network::disposeDisconnectNetwork()
{
    emit disconnectNetwork();
    startConnectNetWork();
}

void T3_Face_Network::readNetworkData()
{
    _networkDataBuffer.append(_tcpSocket->readAll());
    QDataStream stream_(&_networkDataBuffer,QIODevice::ReadOnly);
    stream_.setVersion(QDataStream::Qt_5_5);
    stream_ << _blockSize;
    if(_networkDataBuffer.size() < _blockSize)
    {
        return ;
    }else
    {
        QByteArray jsonByteArray_;
        stream_ << jsonByteArray_;
        emit getNetworkData(jsonByteArray_);
    }

}

void T3_Face_Network::connectNetwork()
{
    _tcpSocket->abort();
    _tcpSocket->connectToHost(kServerURL,kServerPort);
}

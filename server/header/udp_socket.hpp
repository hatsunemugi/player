#ifndef UDP_SOCKET_HPP
#define UDP_SOCKET_HPP

#include <QNetworkDatagram>
#include "instance.hpp"
#include <QUdpSocket>
#include <QVariant>


class UdpSocket : public QObject
{
    Q_OBJECT
public:
    UdpSocket();
    void listen(int);
    void setWriteAddress(QString,quint16);
signals:
    void readyRead(QVariant);
public slots:
    void write(QByteArray);
    void write(QByteArray, int,QString,quint16);
public:
    QString buffer;
    quint16 port;
    QString ip;
    quint16 port_w;
    QString ip_w;

public:
    QUdpSocket socket;
};

namespace  Instance {
    static UdpSocket* udp;
    static UdpSocket* getUdpSocket(){
        if(udp == nullptr)
            udp = new UdpSocket;
        if(core != nullptr)
            if(udp->thread() != core->thread())
                udp->moveToThread(core->thread());
        return udp;
    }
};

inline UdpSocket::UdpSocket()
    :socket(this)
{
    socket.setSocketOption(QAbstractSocket::SendBufferSizeSocketOption,64*1024*1024);
    socket.setReadBufferSize(64*1024*1024);
}

inline void UdpSocket::write(QByteArray content)
{
    socket.writeDatagram(QNetworkDatagram(content,QHostAddress(ip_w),port_w));
}

inline void UdpSocket::write(QByteArray bytes, int max_size,QString ip_,quint16 port_)
{
    int size = bytes.size();
    if(max_size < size)
    {
        int n = ceil(size/max_size);
        for(int i=0; i<n-1; i++)
        {
            QByteArray part = bytes.mid(i*max_size,max_size);
            socket.writeDatagram(QNetworkDatagram(part,QHostAddress(ip_w),port_));
        }
        QByteArray part = bytes.mid((n-1)*max_size,size-(n-1)*max_size);
        socket.writeDatagram(QNetworkDatagram(part,QHostAddress(ip_w),port_));
    }
    else
    {
        socket.writeDatagram(QNetworkDatagram(bytes,QHostAddress(ip_w),port_));
    }
}

inline void UdpSocket::listen(int value)
{
    port =value;
    socket.bind(QHostAddress::Any,value);
    qDebug()<<"udp linstening port:"<<value;
    disconnect(&socket,&QUdpSocket::readyRead,this,0);
    connect(&socket,&QUdpSocket::readyRead,this,[&](){
        while (socket.hasPendingDatagrams()) {
            QNetworkDatagram datagram = socket.receiveDatagram();
            buffer = datagram.data();
            break;
        }
//        qDebug()<<buffer;
        emit readyRead(buffer);
    });
}

inline void UdpSocket::setWriteAddress(QString ip_, quint16 port_)
{
    ip_w = ip_;
    port_w = port_;
}

#endif // UDP_SOCKET_HPP

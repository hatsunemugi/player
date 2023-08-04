#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include <QCoreApplication>
#include <QNetworkProxyFactory>
#include <QTcpServer>
#include <QTcpSocket>
#include "asio.hpp"


namespace Instance
{
    class TcpServer : public QObject
    {
        Q_OBJECT
    public:
        TcpServer();
        void listen(int);
        void write(QByteArray);
    signals:
        void readyRead(QByteArray);
        void newConnection(QTcpSocket*);
    public:
        QTcpSocket* client;
        QTcpServer server;
        QHash<QHostAddress,QTcpSocket*> clients;
    };
    static QCoreApplication* core = nullptr;
    static asio::io_context* context = nullptr;
    static TcpServer* server = nullptr;
    static TcpServer* getTcpServerInstance()
    {
        if(server == nullptr)
            server = new TcpServer;
        if(core != nullptr)
            if(server->thread() != core->thread()){
                server->moveToThread(core->thread());
                server->server.moveToThread(core->thread());
            }
        return server;
    }
    inline TcpServer::TcpServer():server(this)
    {
        client = nullptr;
        QNetworkProxyFactory::setUseSystemConfiguration(false);
    }

    inline void TcpServer::listen(int port)
    {
        server.listen(QHostAddress::Any,port);
        connect(&server,&QTcpServer::newConnection,this,[&](){
            client=server.nextPendingConnection();
            clients.insert(client->peerAddress(),client);
            qDebug()<<"new tcp connection from"<<client->peerAddress()<<client->peerName();
            emit newConnection(client);
            connect(client,&QTcpSocket::disconnected,this,[&](){qDebug()<<"tcp disconnection from"<<client->peerAddress();});
            connect(client,&QTcpSocket::readyRead,this,[&](){
                QByteArray data=client->readAll();
                emit readyRead(data);
            });
        });
    }

    inline void TcpServer::write(QByteArray bytes)
    {
        if(client != nullptr)
        client->write(bytes);
    }
};




#endif // INSTANCE_HPP

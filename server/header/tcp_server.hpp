#ifndef TCPSERVER_H
#define TCPSERVER_H
#include <QNetworkInterface>
#include <QHostAddress>
#include <QHash>
#include <iostream>
#include <queue>
#include "asio.hpp"
#include "http.hpp"
using namespace std;
using namespace std::placeholders;
using namespace asio::ip;
using std::error_code;
using asio::buffer;
using std::shared_ptr;
using asio::io_context;
typedef shared_ptr<tcp::socket> sock_ptr;

class TcpServer
{
public:
    //构造函数
    TcpServer(io_context &_context_,int _port_):context(&_context_),port(_port_)
    {
        session= nullptr;
    }
    int (*new_session)(Session*);
    //启动异步接受客户端连接
    void init(int);
    void start();
    Session* getSession(QString);
public:
    Session* latest;
private:
    io_context* context;
    tcp::acceptor* acceptor;
    Session* session;
    QHash<QString,Session*> sessions;
    int port;
public:
};

inline void TcpServer::init(int port)
{
    acceptor = new tcp::acceptor(*context,tcp::endpoint(tcp::v4(),port));
    qDebug()<<"listening port:"<<port;
}

inline void TcpServer::start()
{
    if(!session)
        session = new HttpSession(*context);
    auto& socket = session->socket;
    acceptor->async_accept(socket,[&](std::error_code ec)
    {
       if (!ec)
       {
            latest = session;
            session->start();
            session->await();
            sessions.insert(session->getAddress(),session);
            session = new HttpSession(*context);
       }
       else{
            qDebug()<<"READ failed, error code: " << ec.value();
            qDebug()<<"category name: " << ec.category().name();
            cout<<"message: "<<ec.message()<<std::endl;
       }
       start();
    });
    //      std::bind(&TcpServer::accept_handler,this,session,std::placeholders::_1));
}

inline Session *TcpServer::getSession(QString ip_address)
{
    return sessions.value(ip_address);
}
static QString _get_ip_(QString head="192.")
{
    QString ip_address;
    QList<QHostAddress> ip_addresses_list = QNetworkInterface::allAddresses(); // 获取所有ip
    for (int i = 0; i < ip_addresses_list.size(); ++i)
    {
    QHostAddress ip = ip_addresses_list.at(i);
    if ((ip.protocol() == QAbstractSocket::IPv4Protocol))// 筛选出ipv4
    {
        qDebug()<< "---- ipv4 :" << ip.toString();
        if (ip.toString().startsWith(head))// 过滤掉网卡，路由器等的ip
        {
            ip_address = ip_address.isEmpty()?ip.toString():ip_address;
          //                break;
      }
    }
    }
    if (ip_address.isEmpty())
    ip_address = QHostAddress(QHostAddress::LocalHost).toString();
    qDebug()<< "----ip set:" <<ip_address;
    return ip_address;
}
#endif // TCPSERVER_H

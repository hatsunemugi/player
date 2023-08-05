#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <iostream>
#include "overrides.h"
#include "asio.hpp"
#include <extras.h>
#include <QDebug>
using namespace asio::ip;
using namespace std::placeholders;
using std::error_code;
using std::shared_ptr;
using std::string;
using std::bind;
using std::cout;
class Session: public QObject
{
    Q_OBJECT
public:
    port_type port;
    asio::ip::address address;
    Session(asio::io_context& context):socket(context){}
    ~Session(){
        qDebug()<<"session"<<getAddress()<<"off";
    }
    void start();
    void write(QByteArray s);
//    void write(string);
    void write(const char*,size_t);
    static bool isHttp(QString);

    void await();
    QString getAddress(){ return QString::fromStdString(address.to_string());}
    tcp::socket& getSocket(){return socket;}
public:
    tcp::socket socket;
signals:
    void message(QString);
protected:
    enum { max_length = 1024 };
    char data_[max_length];
    bool inited;
};

inline void Session::start()
{
    inited = false;
    port = socket.remote_endpoint().port();
    address = socket.remote_endpoint().address();
    cout<<"remote ip:"<<address.to_string()<<std::endl;
    cout<<"remote port:"<<port<<std::endl;
}

inline void Session::write(QByteArray s)
{
    write(s,s.size());
}

//inline void Session::write(string s)
//{
//    size_t length = s.length();
//    char* data =new char[length];
//    strcpy(data,s.c_str());
//    write(data,length);
//}

inline void Session::write(const char *data, size_t length)
{
    qDebug()<<"try writing "<<data;
    socket.async_send(asio::buffer(data,length),[&](std::error_code ec, std::size_t /*length*/)
    {
        if (!ec)
        {
            cout<<"success"<<std::endl;
        }
        else{
            cout<< "READ failed, error code:" << ec.value()<<std::endl
                << "category name:" << ec.category().name()<<std::endl
                << "message:" << ec.message()<<std::endl;
        }
    });
}

inline bool Session::isHttp(QString content)
{
    auto a = content.split("\r\n");
    for(const QString& s:a)
    {
        //        qDebug()<<s;
        if(s.startsWith("GET"))
        {
            if(s.split("/")[1]==" HTTP"){
                qDebug()<<"HTTP"<<s.split("/").back();
                return true;
            }
        }
    }
    return false;
}

inline void Session::await()
{
    qDebug()<<"await";
    socket.async_receive(asio::buffer(data_,max_length),[&](std::error_code ec, std::size_t bytes){
        if(!ec){
//            cout<<"msg: ";
            QString data(string(data_,bytes).c_str());
            qDebug()<<data;
            emit message(data);
            await();
        }else{
            cout<< "READ failed, error code:" << ec.value()<<std::endl
                << "category name:" << ec.category().name()<<std::endl
                << "message:" << ec.message()<<std::endl;
            if(ec==asio::error::eof||ec==asio::error::connection_reset )
                qDebug()<<"断开连接";
        }
    });
}

#endif // TCPCLIENT_H

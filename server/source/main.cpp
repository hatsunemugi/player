#include <QCoreApplication>
#include <QTextStream>
#include <QHash>
#include <QThread>
#include <QTextCodec>
#include "session.hpp"
#include "command.hpp"
#include "tcp_server.hpp"
#include "udp_socket.hpp"
#include <stdlib.h>
#include "extras.h"
#include "sql.hpp"
QTextStream in(stdin);
int port;
Sql* sql;
Session* client;
Session* latest;
TcpServer* server;
io_context context;
tcp::endpoint* host;
int new_session(Session*);
QString command;
void console();
void _listen_(int);
void init();
int main(int argc, char *argv[])
{
//    QGuiApplication
    QGuiApplication a(argc, argv);
    Instance::core = &a;
    Instance::getTcpServerInstance();
    Instance::getMediaInstance();
    Instance::server->listen(52000+(1<<8));
    Instance::getUdpSocket()/*->listen(52000+(1<<8))*/;
    Instance::context = &context;
    system("chcp 936 & cls");
    QTextCodec *codec = QTextCodec::codecForName("UTF-8"); // 按照您的控制台编码选择对应的编码
    QTextCodec::setCodecForLocale(codec);
    qDebug()<<"entry";
    if(argc > 1)
        port = atoi(argv[1]);
    else
        port = 80;
//    init();
    std::thread asio_thread(_listen_,port);
    console();
    asio_thread.detach();

    return a.exec();
}
void init()
{
    sql =new Sql("media");
    QDateTime time = QDateTime::currentDateTime();
    QList<QString> list = {"test_table", "KEY", "A INT", "B TEXT NO NULL", "C TEXT"};
//    QList<QVariant> list_ = {"test_table",
//                             "A", 4545,
//                             "B", "buvidc",
//                             "C", time};
    sql->create_table(sql->database,list);
//    sql->insert_row(sql->database, list_);
//    sql->exec(sql->database,"select * from test_table");

}
int new_session(Session* session)
{
    latest = session;
    client = session;
    return 1;
}
void console()
{
    std::thread thread([&](){
//        qDebug()<<"console";
        QString s;
        for(;;){
            /*=input()*/;
            string i=input(string(""));
//            cout<<i<<std::endl;
            s = trans_s(i,"gbk","utf-8");
            qDebug()<<s;
            if(Command::isEmpty(s)||s.isEmpty())
                continue;
            const QVector<Word>& words = Command::analyse(s);
            print(words);
            if(words=="set port value")
            {
                port = words[2].body.toUInt();
            }
            else if(words=="set ip value")
            {

            }
            else if(words=="set session value")
            {
                client = server->getSession(words[2].body);
            }
            else if(words=="write value")
            {
                server-> latest -> write(words[1].body.toUtf8());
            }
        }
    });
    thread.detach();
}
QString input(QString terminator)
{
    std::string i="";
    char c='\0';
    char prev;
    while (true)
    {
        prev=c;
        c=getchar();
        if(c=='\n'&&(terminator.contains(prev)||terminator.isEmpty()))
            break;
        i+=c;
    }
    return QString::fromStdString(i);
}
void _listen_(int value)
{
    try{
        if(!server){
            server = new TcpServer(context,port);
        }
        if(!server -> new_session)
            server -> new_session = new_session;
        server -> init(port);
        server -> start();
        context.run();
        qDebug()<<"ip:"<<_get_ip_()<<" port:"<<port;
    }
    catch (std::exception& e)
    {
        cout<<port<<e.what()<<std::endl;
        port++;
        _listen_(port);
    }
}

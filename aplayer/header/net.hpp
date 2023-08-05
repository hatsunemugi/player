#ifndef NET_H
#define NET_H
#include <QDir>
#include "extras.h"
#include <QAudioSink>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QAudioFormat>
#include <QHostAddress>
#include <QMediaDevices>
#include <QNetworkProxy>
#include <QXmlStreamReader>
#include <QNetworkDatagram>
using namespace Database;
//#include <QStringView>
//using namespace std::literals::string_literals;
static bool operator == (QStringView sv,const char* s)
{
    return sv==QString(s);
}
class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);
    void connect_to(QString,QString);
    void reconnect();
    void write(const QString&);
    void get(QString);
    QVariant trans(QString&);
    void dataHandler(QString);
signals:
    void exec(QVariant);
    void sync_progress(double);
    void audio_frame(QByteArray);
    void video_frame(QImage);
    void first_frame();
    void state(bool);
public slots:
    void listen(int,int);
private:
    int index;
    int audio_size;
//    int video_size;
    int video_lag;
    int video_count;
    int video_error;
    QByteArray video_data;
    QString url;
    QVariant lastKeys;
    QUdpSocket message;
    QUdpSocket audio_socket;
    QTcpSocket video_socket;
    QTcpSocket* tcp;
    QString separator;
    QVector<int> video_size;
    bool bOnceConnected;
    bool bIsFirstFrame;
    bool bIsWrong;
private:
    QAudioFormat* audio_format;
    QAudioSink* audio_sink;
    QIODevice* io;
public:
    QString ip;
    int port;
};

inline Client::Client(QObject *parent)
{
    separator = "|:|";
    bOnceConnected = false;
    bIsFirstFrame = false;
    bIsWrong = false;
    audio_format = new QAudioFormat();
    audio_format->setSampleRate(44100);     //设置采样率
    audio_format->setChannelCount(2);        //设置通道数
    audio_format->setSampleFormat(QAudioFormat::SampleFormat::Int16);  //样本数据16位
    QAudioDevice info(QMediaDevices::defaultAudioOutput());
    if (!info.isFormatSupported(*audio_format)) {
        qWarning() << "Raw audio format not supported by backend, cannot play audio.";
        return;
    }

    audio_sink = new QAudioSink(*audio_format,this);
    audio_sink -> setBufferSize(100000);

    io = audio_sink->start();

    QNetworkProxyFactory::setUseSystemConfiguration(false);

}

inline void Client::listen(int port_1, int port_2)
{
    message.bind(QHostAddress::LocalHost,port_1);
    audio_socket.bind(QHostAddress::LocalHost,port_2);


    qDebug()<<"open message socket port:"<<message.localPort();
    qDebug()<<"open audio socket port:"<<audio_socket.localPort();
    //    qDebug()<<"open video socket port:"<<video_socket.localPort();
    disconnect(&message,&QUdpSocket::readyRead,this,0);
    connect(&message,&QUdpSocket::readyRead,this,[&](){
        QString s;
        while (message.hasPendingDatagrams()) {
            QNetworkDatagram datagram = message.receiveDatagram();
            s = datagram.data();
            break;
        }
        //        qDebug()<<s;
        if(s.startsWith("sync_progress"))
            emit sync_progress(s.split(" ").back().toDouble());
        else if(s.startsWith("audio_frame"))
        {
            audio_size = s.split(" ").back().toInt();
        }
        else if(s.startsWith("video_frame"))
        {
            auto array = s.split(" ");
            if(s.split(" ").size() == 3){
                qDebug()<<array[2];
                bIsFirstFrame = true;
            }
            video_size<<array[1].toInt();
        }
    });
    disconnect(&audio_socket,&QUdpSocket::readyRead,this,0);
    connect(&audio_socket,&QUdpSocket::readyRead,this,[&](){
        QByteArray data;
        while (audio_socket.hasPendingDatagrams()) {
            QNetworkDatagram datagram = audio_socket.receiveDatagram();
            data = datagram.data();
            break;
        }
        //        qDebug()<<"audio frame come in";
        //            emit audio_frame(data);
        io->write(data.data(),data.size());

        if(video_socket.state() != QTcpSocket::ConnectedState){
            video_socket.connectToHost(QHostAddress(ip),52256);
//            qDebug()<<video_socket.state();
        }
    });
    disconnect(&video_socket,&QUdpSocket::readyRead,this,0);
    connect(&video_socket,&QUdpSocket::readyRead,this,[&](){
        QByteArray data = video_socket.readAll();;
        if(video_data.size() < video_size.front()){
//            qDebug()<<video_data.size()<<data.size()<<video_size.front();
            video_data.append(data);
        }

        if(video_data.size() >= video_size.front())
        {
//            qDebug()<<video_data.size()<<video_size.front();
            if(video_data.size() > video_size.front())
            {
                video_socket.write("wrong data");
                video_data.clear();
                video_size.clear();
                video_socket.disconnectFromHost();
                get("/play=pause");
                emit state(false);
                return ;
            }
            if(video_data.size() == video_size.front()){
                QImage image;
                image.loadFromData(video_data, "jpg");
//                qDebug()<<"video_frame";
                emit video_frame(image);
            }
            if(bIsFirstFrame)
            {
                emit first_frame();
                bIsFirstFrame = false;
            }
            video_data.clear();
            video_size.pop_front();
            video_socket.write("done");
        }
    });
}

inline void Client::connect_to(QString ip_, QString port_)
{
    if(bOnceConnected)
    {
        if(ip == ip_ && port == port_.toInt()){
            reconnect();
            return;
        }
    }

    this->ip = ip_;
    this->port = port_.toUShort();
    bIsFirstFrame = true;
    tcp = new QTcpSocket(this);
    tcp->setProxy(QNetworkProxy::NoProxy);
    tcp->connectToHost(QHostAddress(ip),port);

    connect(tcp,&QTcpSocket::connected,this,[&](){
        bOnceConnected=true;
        write("GET / HTTP/1.1");
        connect(tcp,&QTcpSocket::readyRead,this,[&](){
            QByteArray data=tcp->readAll();
            QString s(data);
            qDebug()<<s;

//            if(s.startsWith("config"+separator))
//            {
//                QStringList sl=s.split(separator);
//                qDebug()<<"name:"<<sl[1];
//            }
            auto array = s.split("\r\n\r\n");
            QString text = array.back();
            QString lastStartElement;
            QXmlStreamReader reader(text);
//            reader.readNext();//跳过<!DOCTYPE html>
            while (!reader.atEnd()) //外部循环，未到文件结尾就一直循环读取
            {
                if(reader.hasError())
                {
                    qDebug()<<"读取XML异常";
                    break;
                }

                QXmlStreamReader::TokenType type = reader.readNext();
                switch((int)type)
                {
                case QXmlStreamReader::NoToken:
                    qDebug()<<"没有读到任何东西";
                    break;
                case QXmlStreamReader::Invalid:
                    qDebug()<<"发生错误,在error()和errorString()中报告."<<reader.error()<<reader.errorString();
                    break;
                case QXmlStreamReader::StartDocument:
                    qDebug()<<"读取文件开始-"<<"版本号:"<<reader.documentVersion()<<"编码格式:"<<reader.documentEncoding();
                    break;
                case QXmlStreamReader::EndDocument:
                    qDebug()<<"读取文件结束";
                    break;
                case QXmlStreamReader::StartElement:    //开始读取一个元素
                {
                    QString MMMMM = reader.attributes().value("text").toString();
                    lastStartElement = reader.name().toString();
                    if(!MMMMM.isEmpty())
                    {
                        qDebug()<<"元素读取开始:"<<MMMMM;
                    }
                }
                    break;
                case QXmlStreamReader::EndElement:  //读取一个元素结束
                    qDebug()<<"读取元素结束";
                    break;
                case QXmlStreamReader::Characters:  //读取元素中的文本信息
                {
                    QString str = reader.text().toString();
                    if(!reader.isWhitespace())
                    {
                        qDebug()<<"Characters"<<lastStartElement<<str;
                        if(lastStartElement == "body")
                        {
                            dataHandler(str);
                        }

                    }
                }
                break;
                case QXmlStreamReader::Comment: //文本注释
                    break;
                case QXmlStreamReader::ProcessingInstruction:
                    qDebug()<<"ProcessingInstruction"<<reader.text();
                    break;
                }
            }

        });
    });
    connect(tcp,&QTcpSocket::disconnected,this,[&](){
        qDebug()<<"disconnected from"<<ip;
        tcp->close();
    });
}

inline void Client::reconnect()
{
    if(tcp != nullptr)
        tcp->connectToHost(QHostAddress(ip),port);
}

inline void Client::write(const QString& content)
{
    tcp->write(content.toUtf8());
}

inline void Client::get(QString url_)
{
    url = url_;
    write("GET "+url+" HTTP/1.1");
    lastKeys = trans(url);
}

inline QVariant Client::trans(QString &text)
{
    auto array = url.split("/");
    QVector<QString> keys;
    url.clear();
    for(auto& s: array)
    {
        auto splits = s.split("=");
        if(splits.size() < 2)
            url += s + "/";
        else{
            url += splits[0] + "={}/";
            keys.push_back(QUrl::fromEncoded(splits[1].toUtf8()).toString());
        }
    }
    url.chop(1);
    return keys;
}

inline void Client::dataHandler(QString str)
{
    if((url) == "/sql/table={}" )
    {
        int size = str.toInt();
        if(size >= 0)
        {
            if(database.value("media")->size() != size){
                qDebug()<<"resize"<<size;
                database.value("media")->resize(str.toInt());
                index = 1;
                connect(this,&Client::exec,this,[&](){
                    if(index <= database.value("media")->size())
                    {
                        get("/sql/table=media/id="+QString::number(index));
                    }
                    else
                    {
                        disconnect(this,&Client::exec,this,0);
                    }
                });
                get("/sql/table=media/id=1");

            }
        }
        video_socket.connectToHost(QHostAddress(ip),52256);
        video_socket.write("hello");
    }
    else if(url == "/sql/table={}/id={}")
    {
        auto& array = *database.value("media");
        auto keys = lastKeys.toStringList();
        int i = keys[1].toInt();
        array[i-1] = str;
        index++;
        emit exec(index);
    }
    else if(url == "/sql/table={}/id={}/play")
    {
        auto ports = str.split(":");
        if(ports.size() != 5 || ports[0] != "port")
            return;
        video_size.clear();
        bIsFirstFrame = true;
//        QString hello = "ok";
//        message.writeDatagram(QNetworkDatagram(hello.toUtf8(),QHostAddress(ip),ports[4].toInt()));
        qDebug()<<"server is"<<ip<<ports[4].toInt();
    }
    else
    {

    }
}


#endif // NET_H

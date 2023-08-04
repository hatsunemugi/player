#ifndef MEDIA_H
#define MEDIA_H

#include <QObject>
#include <QFile>
#include <QImage>
#include <QThread>
#include <QUdpSocket>
#include <QGuiApplication>
#include <QMediaDevices>
#include <QAudioFormat>
#include <QAudioSink>
#include <QGuiApplication>
#include <QNetworkDatagram>
#include <QTimer>
#include <QPixmap>
#include <QBuffer>

#include "audio_player.hpp"
#include "video_player.hpp"

#include "instance.hpp"
#include "udp_socket.hpp"

using asio::ip::udp;

class Media : public QObject
{
    Q_OBJECT
public:
    explicit Media(QObject *parent = nullptr);
    QImage* getIcon(){return icon;}
    QString getName(){return name;}
    void open(QString);
    void setAddress(QString ip,int port);
signals:
    void first_frame(QImage);
    void frame(QImage);
    void speed(double);
    void FPS(int);
    void seek(double);
    void sync_video();
    void sync_eof();
    void sync_progress(double);
    void exec(QString);
private:
    QString name;
    QFile* file;
    QImage* icon;
    QAudioSink* audio_sink;
    VideoCapture* capture;
    QString ip;
    QString filepath;
    quint16 port;
    QVector<QImage> images;
    bool bIsDone;
    char* data_;
    int fps;

private:
    VideoPlayer* video_player;
    AudioPlayer* audio_player;
    QThread* audio_thread;
    QThread* video_thread;
    UdpSocket* socket;
//    Instance::TcpServer* server;
    QTcpSocket* client;

};

namespace Instance {
    static Media* media = nullptr;
    static Media* getMediaInstance()
    {
        if(media == nullptr)
            media = new Media;
//        if(core != nullptr)
//            if(media ->thread() != core->thread())
//                media->moveToThread(core->thread());
        return media;
    }
};

typedef QHash<QString,Media*> MediaTable;

inline Media::Media(QObject *parent):client(nullptr)
{
    bIsDone = true;
    audio_thread = new QThread(this);
    video_thread = new QThread(this);
    audio_player = new AudioPlayer;
    video_player = new VideoPlayer;
    audio_player->moveToThread(audio_thread);
    video_player->moveToThread(video_thread);
    connect(audio_thread, &QThread::finished, audio_player, &QObject::deleteLater);
    connect(video_thread, &QThread::finished, video_player, &QObject::deleteLater);
    connect(this, &Media::exec, audio_player, &AudioPlayer::handler);
    connect(this, &Media::exec, video_player, &VideoPlayer::handler);
    connect(this, &Media::seek, audio_player, &AudioPlayer::seek);
    connect(audio_player, &AudioPlayer::sync_seek, video_player, &VideoPlayer::seek);
    connect(audio_player,&AudioPlayer::sync_inited,video_player,&VideoPlayer::setReader);
//    connect(video_player,&VideoPlayer::sync_inited,audio_player,&AudioPlayer::play);
    connect(audio_player,&AudioPlayer::sync_video,video_player,&VideoPlayer::play);
    connect(this, &Media::speed,audio_player,&AudioPlayer::setSpeed);
    connect(audio_player,&AudioPlayer::sync_progress,this,&Media::sync_progress);
//    connect(video_player, &VideoPlayer::draw,this, &Media::frame);
//    connect(video_player, &VideoPlayer::first_frame,this, &Media::first_frame);
    audio_thread->start();
    video_thread->start();
}

inline void Media::open(QString path)
{
    if(filepath == path)
        return ;
    filepath = path;
    audio_player->init(path);
    video_player->init(path);
}

inline void Media::setAddress(QString ip_, int port_)
{
    if(ip == ip_ && port == port_)
        return;
    ip = ip_;
    port = port_;
    bIsDone = true;
    Instance::TcpServer* server = Instance::getTcpServerInstance();
    socket =Instance::getUdpSocket();
    connect(server,&Instance::TcpServer::newConnection,this,[&](QTcpSocket* socket){
        client = socket;
    });
    connect(server,&Instance::TcpServer::readyRead,this,[&](QString value){
//        qDebug()<<value;
        if(value == "wrong data"){
            socket->write("handled");
            if(video_player->scale >= 20)
                video_player->scale--;
            qDebug()<<"scale"<<video_player->scale;
        }
        else if(value == "done")
            bIsDone =true;
    });
    socket->setWriteAddress(ip,port);
    connect(this,&Media::sync_progress,this,[&](double progress){
        socket->write(("sync_progress "+QString::number(progress)).toUtf8());
    });
    connect(audio_player,&AudioPlayer::sync_socket,this,[&](QByteArray audio){
        int size = audio.size();
        socket->write(QString("audio_frame %1").arg(size).toUtf8());
        socket->socket.writeDatagram(QNetworkDatagram(audio,QHostAddress(socket->ip_w),52064));

        if(bIsDone)
        {
            QByteArray data;
            QBuffer buffer(&data);
            //        pixmap.save(&buffer,"jpg",video_player->scale);
            images.front().save(&buffer,"jpg",video_player->scale);
            qint64 all = data.size();//总文件大小
            qint64 max_size = (1<<16)-1 ;
            int n = ceil(all/max_size);
            socket->write(QString("video_frame %1").arg(all).toUtf8());
            for(int i=0; i<n-1;i++)
            {
                client->write(data.mid(i*max_size,max_size));
            }
            client->write(data.mid((n-1)*max_size,all - (n-1)*max_size));
            bIsDone = false;
            images.pop_front();
            if(images.size() > 5){
                QImage image = images[2];
                images.clear();
                images<<image;
//                if(video_player->scale >= 20)
//                    video_player->scale--;
                qDebug()<<"too lag scale"<<video_player->scale;

            }
        }


    });
    connect(video_player,&VideoPlayer::draw,this,[&](QImage image){
        images<<image;
    });

}





#endif // MEDIA_H

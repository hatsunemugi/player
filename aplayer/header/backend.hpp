#ifndef BACKEND_H
#define BACKEND_H

#include <QtQml>
#include <QObject>
#include "net.hpp"
#include "media.hpp"
#include "command.hpp"
#include "painteditem.hpp"
#include <QGuiApplication>

using namespace Database;

class BackEnd : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(PaintedItem* painter READ getPainter WRITE setPainter NOTIFY painterChanged)
public:
    explicit BackEnd(QObject *parent = nullptr);
    BackEnd(QGuiApplication* gui);
signals:
    void painterChanged();
    void fpsUpdated(int);
    void select(QString);
    void state(bool);
    void sync(double);
    void draw(QImage);
public slots:
    QString test(){return "666";}
    void open_ffmpeg_opencv(QString);
    void open(QString);
    void seek(double);
    void speed(double);
    void play();
    void stop();
    void pause();
    QVariant exec(QString);
    void connect_to(QString,QString);
    PaintedItem* getPainter(){return painter;}
    void setPainter(PaintedItem* item){painter = item;emit painterChanged();}
private:
    QGuiApplication* gui;
    PaintedItem* painter;
    Client* client;
    Media* media;
    bool bIsLocal;
};

inline BackEnd::BackEnd(QObject *parent):gui(nullptr)
{
    backend = this;
    bIsLocal = true;
}

inline BackEnd::BackEnd(QGuiApplication* gui):gui(gui)
{
    backend = this;
    bIsLocal = true;
}

inline void BackEnd::open_ffmpeg_opencv(QString filepath)
{

}

inline void BackEnd::open(QString filepath)
{
    qDebug()<<filepath;
    if(media == nullptr){
        media = new Media;
        media->gui = this->gui;
        media->paint(this->painter);
    }
    emit media->open(filepath);
    emit media->exec("play");
    emit state(true);
    connect(media,&Media::sync_progress,this,&BackEnd::sync);
}

inline void BackEnd::seek(double value)
{
    qDebug()<<"seek"<<value;
    if(bIsLocal){
        if(media)
        {
            qDebug()<<"local";
            emit media->seek(value);
        }
    }
    else{
        if(client != nullptr){
            qDebug()<<"net";
            client->get(QString("/seek=%1").arg(value));
        }
    }
}


inline void BackEnd::speed(double value)
{
    qDebug()<<value;
    if(bIsLocal){
        if(media)
        {
            emit media->speed(value);
        }
    }
}

inline void BackEnd::play()
{
    qDebug()<<"play";
    if(bIsLocal){
        if(media)
        {
            qDebug()<<"local";
//            emit state(true);
            emit media->exec("play");
        }
    }
    else{
        if(client != nullptr){
            qDebug()<<"net";
            client->get("/play=play");
        }
    }
}

inline void BackEnd::stop()
{
    qDebug()<<"stop";
    if(bIsLocal){
        emit state(false);
    }
}

inline void BackEnd::pause()
{
    qDebug()<<"pause";
    if(bIsLocal){
        emit state(false);
        if(media)
        {
            qDebug()<<"local";
            emit media->exec("pause");
        }
    }
    else{
        if(client != nullptr){
            qDebug()<<"net";
            client->get("/play=pause");
        }
    }
}

inline QVariant BackEnd::exec(QString command)
{
    if(Command::isEmpty(command)||command.isEmpty())
        return 0;
    const QVector<Word>& words = Command::analyse(command);
    print(words);
    if(words=="count value")
    {
        return getTableSize(words[1].body);
    }
    if(words=="select value id value")
    {
        return getRecord(words[1].body,words[3].body.toInt());
    }
    if(words=="play value id value")
    {
        if(client != nullptr)
            client->get(QString("/sql/table=%1/id=%2/play").arg(words[1].body).arg(words[3].body.toInt()+1));
        return 0;
    }
    return 0;
}

inline void BackEnd::connect_to(QString ip, QString port)
{
    client = new Client;
    client->connect_to(ip,port);
    client->get("/sql/table=media");
    connect(client,&Client::sync_progress,this,&BackEnd::sync);
    connect(client,&Client::video_frame,painter,&PaintedItem::setImage);
    connect(client,&Client::first_frame,painter,&PaintedItem::another);
    if(database.contains("media"))
        delete database.value("media");
    database.insert("media",new QVector<QString>());
    bIsLocal = false;
//    qDebug()<<ip<<port;
}


#endif // BACKEND_H

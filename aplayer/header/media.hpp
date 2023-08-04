#ifndef MEDIA_H
#define MEDIA_H
#include <QtQml>
#include <QObject>
#include <QFile>
#include <QImage>
#include <QThread>
#include <QMediaDevices>
#include <QAudioFormat>
#include <QAudioSink>
#include <QtConcurrent>
#include <QGuiApplication>

#include "audio_player.hpp"
#include "video_player.hpp"

#include "extras.h"
#include "painteditem.hpp"
using namespace cv;
using cv::VideoCapture;
using namespace QtConcurrent;



class Media : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    enum through{
        OPENCV,
        FFMPEG
    };
    QGuiApplication* gui;
public:
    explicit Media(QObject *parent = nullptr);
    QImage* getIcon(){return icon;}
    QString getName(){return name;}
    void paint(PaintedItem*);
    PaintedItem* painter_;
signals:
    void Frame(QImage);
    void speed(double);
    void FPS(int);
    void seek(double);
    void sync_video();
    void sync_eof();
    void sync_progress(double);
    void exec(QString);
    void open(QString);
    void painter(PaintedItem*);
private:
    QString name;
    QFile* file;
    QImage* icon;
    QAudioSink* audio_sink;
    VideoCapture* capture;
    bool bIsDone;
    int fps;
private:
    VideoPlayer* video_player;
    AudioPlayer* audio_player;
    QThread* audio_thread;
    QThread* video_thread;
};
typedef QHash<QString,Media*> MediaTable;

inline Media::Media(QObject *parent)
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
    connect(this, &Media::open, audio_player, &AudioPlayer::init);
    connect(this, &Media::open, video_player, &VideoPlayer::init);
    connect(audio_player,&AudioPlayer::sync_inited,video_player,&VideoPlayer::setReader);
    connect(video_player,&VideoPlayer::sync_inited,audio_player,&AudioPlayer::play);
    connect(audio_player,&AudioPlayer::sync_video,video_player,&VideoPlayer::play);
    connect(this, &Media::painter,video_player,&VideoPlayer::setPainter);
    connect(this, &Media::speed,audio_player,&AudioPlayer::setSpeed);
    connect(audio_player,&AudioPlayer::sync_progress,this,&Media::sync_progress);
    audio_thread->start();
    video_thread->start();

}

inline void Media::paint(PaintedItem *painter__)
{
    this->painter_ = painter__;
    emit painter(this->painter_);
    connect(video_player, &VideoPlayer::draw,painter_, &PaintedItem::setImage);
    connect(video_player, &VideoPlayer::first_frame,this, [&](QImage image){
        painter_->setImage(image);
        emit painter_->another();
        disconnect(video_player, &VideoPlayer::first_frame,this,0);
    });
}




#endif // MEDIA_H

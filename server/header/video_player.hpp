#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QDebug>
#include <QImage>
#include <QVector>
#include <QThread>
#include <QMediaDevices>

//#include "opencv.hpp"
//#include "core.hpp"
#include "videoio.hpp"
#include "core/mat.hpp"

#include "extras.h"
using State = enums::State;
using cv::CAP_PROP_FRAME_COUNT;
using cv::CAP_PROP_POS_MSEC;
using cv::CAP_PROP_FPS;
using cv::VideoCapture;
using cv::Mat;
//using namespace cv;

class VideoPlayer : public QObject
{
    Q_OBJECT
public:
    VideoPlayer();
    void init(QString);
    void handler(QString);
    void play();
    void stop();
    void pause();
    void resume();
signals:
    void draw(QImage);
    void sync_inited();
    void first_frame(QImage);
    void sync_progress(double);
public slots:
    void setReader();
    void seek(double);
private:
    State state;
private:
    VideoCapture* capture;
    Mat frame_cv;
private:
    QString filepath;
    QImage* frame_qt;
    int sleep_time;
    int frame_count;
    int duration;
    int fps;
private:
    bool bIsDone;
    bool bIsInited;
public:
    int scale;
};

inline VideoPlayer::VideoPlayer()
{
    state = State::none;
    bIsInited = false;
}

inline void VideoPlayer::init(QString content)
{
    bIsInited = false;

    while(true){
        if(bIsInited)
            break;
        sleep(50);
    }

    filepath = content;

    if(filepath.startsWith("file://"))
    {
        filepath = filepath.split("///").back();
        qDebug()<<"filepath set to"<<filepath;
    }
    else
    {
        qDebug()<<"filepath set to"<<filepath;
    }
    capture = new VideoCapture(filepath.toStdString());
    if (!capture->isOpened()){
        qDebug()<<"fail to open";
        return ;
    }

    qDebug()<<"entry opencv";


    VideoCapture& cap = *capture;
    fps = cap.get(CAP_PROP_FPS);
    sleep_time = 1000/fps;
    frame_count = (int)cap.get(CAP_PROP_FRAME_COUNT);
    duration = frame_count / fps;
    qDebug()<<"帧率:"<<fps;
    qDebug()<<"时长:"<<duration;
    bIsDone = true;
    scale = 100;
    state = State::playing;

    emit sync_inited();

    cap>>frame_cv;
    if(frame_cv.empty())
        return;
    emit first_frame(MatToQImage(&frame_cv,frame_qt));
}

inline void VideoPlayer::handler(QString)
{

}

inline void VideoPlayer::play()
{
    *capture>>frame_cv;
    if(frame_cv.empty())
        return;
    emit draw(MatToQImage(&frame_cv,frame_qt));
}

inline void VideoPlayer::stop()
{
    state = State::stopped;
}

inline void VideoPlayer::pause()
{
    state = State::paused;
}

inline void VideoPlayer::resume()
{
    state = State::playing;
    qDebug()<<"resume";
}

inline void VideoPlayer::setReader()
{
    bIsInited = true;
}

inline void VideoPlayer::seek(double value)
{
    if(state == State::none)
        return ;
    capture->set(CAP_PROP_POS_MSEC,value);
    qDebug()<<capture->get(CAP_PROP_POS_MSEC);
}

#endif // VIDEOPLAYER_H

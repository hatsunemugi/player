#ifndef EXTRAS_H
#define EXTRAS_H

#include <QTime>
#include <QHash>
#include <QImage>
#include <QQmlEngine>
#include <QAudioFormat>
#include <QCoreApplication>



namespace Database {
    static QHash<QString,QVector<QString>*> database;
    Q_INVOKABLE static int getTableSize(QString name)
    {
        if(!database.contains(name))
            return 0;
        return database.value(name)->size();
    }
    Q_INVOKABLE static QString getRecord(QString name,int id)
    {
        if(!database.contains(name))
            return "";
        if(!database.value(name)->size()>id || id < 0)
            return "";
        return (*database.value(name))[id];
    }
}

extern "C"
{
#include "libavutil/error.h"
#include "libavformat/avformat.h"
}


#include "core/mat.hpp"

class BackEnd;

static BackEnd* backend;

using cv::Mat;

class enums : public QObject
{
    Q_OBJECT
public:
    enum class State : int
    {
        none = 0,
        playing,
        paused,
        stopped,
    };
    Q_ENUM(State)
    enum class Control : int
    {
        none = 0,
        play,
        pause,
        resume,
        stop,
        seek
    };
    Q_ENUM(Control)
    enum class AspectRatio : int
    {
        autofill,
        stretch,
        fill,
        _16_9_,
        _4_3_

    };
    Q_ENUM(AspectRatio)
};


static void sleep(int ms)
{
    QTime time = QTime::currentTime().addMSecs(ms);
    while( QTime::currentTime() < time )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

static QAudioFormat::SampleFormat trans_sample_format(AVSampleFormat format)
{
    switch(format)
    {
    case AVSampleFormat::AV_SAMPLE_FMT_NONE: return QAudioFormat::SampleFormat::Unknown;
    case AVSampleFormat::AV_SAMPLE_FMT_U8:   return QAudioFormat::SampleFormat::UInt8;          ///< unsigned 8 bits
    case AVSampleFormat::AV_SAMPLE_FMT_S16:  return QAudioFormat::SampleFormat::Int16;          ///< signed 16 bits
    case AVSampleFormat::AV_SAMPLE_FMT_S32:  return QAudioFormat::SampleFormat::Int32;          ///< signed 32 bits
    case AVSampleFormat::AV_SAMPLE_FMT_FLT:  return QAudioFormat::SampleFormat::Float;          ///< float
    case AVSampleFormat::AV_SAMPLE_FMT_DBL:  return QAudioFormat::SampleFormat::Float;          ///< double

    case AVSampleFormat::AV_SAMPLE_FMT_U8P:  return QAudioFormat::SampleFormat::UInt8;          ///< unsigned 8 bits, planar
    case AVSampleFormat::AV_SAMPLE_FMT_S16P: return QAudioFormat::SampleFormat::Int16;          ///< signed 16 bits, planar
    case AVSampleFormat::AV_SAMPLE_FMT_S32P: return QAudioFormat::SampleFormat::Int32;          ///< signed 32 bits, planar
    case AVSampleFormat::AV_SAMPLE_FMT_FLTP: return QAudioFormat::SampleFormat::Float;          ///< float, planar
    case AVSampleFormat::AV_SAMPLE_FMT_DBLP: return QAudioFormat::SampleFormat::Float;          ///< double, planar
    case AVSampleFormat::AV_SAMPLE_FMT_S64:  return QAudioFormat::SampleFormat::NSampleFormats; ///< signed 64 bits
    case AVSampleFormat::AV_SAMPLE_FMT_S64P: return QAudioFormat::SampleFormat::NSampleFormats; ///< signed 64 bits, planar

    case AVSampleFormat::AV_SAMPLE_FMT_NB:  return QAudioFormat::SampleFormat::NSampleFormats;   ///< Number of sample formats. DO NOT USE if linking dynamically

    default:    return QAudioFormat::SampleFormat::Unknown;
    }

}

static QString trans_error(int error){
    switch(error)
    {
    case AVERROR_BSF_NOT_FOUND:return "Bitstream filter not found";
    case AVERROR_BUG:return "Internal bug, also see AVERROR_BUG2";
    case AVERROR_BUFFER_TOO_SMALL: return "Buffer too small";
    case AVERROR_DECODER_NOT_FOUND:return "Decoder not found";
    case AVERROR_DEMUXER_NOT_FOUND:return "Demuxer not found";
    case AVERROR_ENCODER_NOT_FOUND:return "Encoder not found";
    case AVERROR_EOF:return "End of file";
    case AVERROR_EXIT:return "Immediate exit was requested; the called function should not be restarted";
    case AVERROR_EXTERNAL:return "Generic error in an external library";
    case AVERROR_FILTER_NOT_FOUND:return "Filter not found";
    case AVERROR_INVALIDDATA:return "Invalid data found when processing input";
    case AVERROR_MUXER_NOT_FOUND:return "Muxer not found";
    case AVERROR_OPTION_NOT_FOUND:return "Option not found";
    case AVERROR_PATCHWELCOME:return "Not yet implemented in FFmpeg, patches welcome";
    case AVERROR_PROTOCOL_NOT_FOUND:return "Protocol not found";
    case  AVERROR_STREAM_NOT_FOUND:return "Stream not found";
    default:return "unhandled error";
    }
}

static QImage& MatToQImage(Mat* mat,QImage*& image){
    image = new QImage((uchar*) mat->data, mat->cols, mat->rows, mat->step, QImage::Format_BGR888);
    return *image;
}

static QImage& AVFrameToQImage(AVFrame* frame,QImage*& image){
    image = new QImage((uchar*) frame->data, frame->width, frame->height, frame->linesize[0], QImage::Format_BGR888);
    return *image;
}

#endif // EXTRAS_H

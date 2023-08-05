#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QDebug>
#include <QThread>
#include <QVector>
#include <QAudioSink>
#include <QAudioFormat>
#include <QMediaDevices>

extern "C"
{
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
#include "libavformat/avformat.h"
}

#include "extras.h"
using State = enums::State;

class AudioPlayer : public QObject
{
    Q_OBJECT
public:
    AudioPlayer();
    void play();
    void stop();
    void pause();
    void resume();
    void seek(double);
    void setSpeed(double);
    void init(QString);
    void handler(QString);
signals:
    void sync_video();
    void sync_eof();
    void sync_seek(double);
    void sync_inited();
    void sync_progress(double);
    void sync_socket(QByteArray);
private:
    State state;
public:
    QString filepath;
    double speed;
    QVector<QByteArray> buffer;
private:
    AVFormatContext* fmt_context;
    AVCodecContext* codec_context;
    SwrContext* swr_context;
    AVPacket *packet;
    AVFrame *frame;
    AVStream* audio;
    AVStream* video;
private:
    int audio_index;
    int video_index;
    int sleep_time;
    int frame_number;
    int duration;
    int sample_rate_out;
    int result;
    double fps;
    double current_time;
    int progress_;
private:
    QAudioFormat* audio_format;
    QAudioSink* audio_sink;
    QIODevice* io;
};

inline AudioPlayer::AudioPlayer()
{
    state = State::none;
}

inline void AudioPlayer::play()
{
    state = State::playing;
    io = audio_sink->start();
    while(true){

        switch(state)
        {
            case State::stopped: return;
            case State::paused: sleep(500);continue;
            case State::playing: break;
            default: break;
        }



//        qDebug()<<state;

        if(av_read_frame(fmt_context,packet) < 0){
            qDebug()<<"fail to read frame";
            break;
        }

        int stream_index = packet->stream_index;
//        qDebug()<<stream_index<<video_index<<audio_index<<(stream_index == audio_index ? "audio":"video");
        if(stream_index == video_index){
//            progress_++;
            emit sync_video();
//            qDebug()<<"emit sync_video";
        }
        else if(stream_index == audio_index)
        {
            if((result = avcodec_send_packet(codec_context, packet)) < 0)
            {
                qDebug()<<"fail to send audio packet"<<result;
                return;
            }

            for(;;){
                if((result = avcodec_receive_frame(codec_context, frame))<0)
                {
//                    qDebug()<<"fail to receive audio frame"<<trans_error(result)<<result;
                    if(result == AVERROR_EOF){
                        qDebug()<<"read done";
                        emit sync_eof();
                    }
                    break;
                }

                current_time = (double)frame->best_effort_timestamp * av_q2d(audio->time_base);
                emit sync_progress((double)current_time/duration);

//                else
//                    qDebug()<<"play audio frame"<<result;
                uint8_t *data[2] = { 0 };
                int size=frame->nb_samples * 2 * 2;
                unsigned char *pcm = new uint8_t[size];     //frame->nb_samples*2*2表示分配样本数据量*两通道*每通道2字节大小
                data[0] = pcm;  //输出格式为AV_SAMPLE_FMT_S16(packet类型),所以转换后的LR两通道都存在data[0]中
                const int out_samples=swr_get_out_samples(swr_context,frame->nb_samples);
                if((result = swr_convert(swr_context, data, out_samples, (const uint8_t**)(frame->data), frame->nb_samples)) < 0)
                    qDebug()<<"fail to convert swr"<<result;
//                io->write((const char*)pcm,size);
                QByteArray bytes(size,0);
                memcpy((void*)bytes.data(),(void*)pcm,size);
                emit sync_socket(bytes);
                sleep(sleep_time / speed);
//                buffer.push_back(bytes);
                delete[] pcm;
//                qDebug()<<bytes.data();
            }
        }
    }
}

inline void AudioPlayer::stop()
{
    state = State::stopped;
    buffer.clear();
}

inline void AudioPlayer::pause()
{
    state = State::paused;
}

inline void AudioPlayer::resume()
{
    state = State::playing;
}

inline void AudioPlayer::seek(double value)
{
    if(state == State::none)
        return ;
    int s = duration * value;
    av_seek_frame(fmt_context,-1,s * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);
    qDebug()<< (double)duration * value;
    emit sync_seek(1000 * s);
}

inline void AudioPlayer::setSpeed(double value)
{
    speed = value;
//    audio_format->setSampleRate(sample_rate_out/speed);
}

inline void AudioPlayer::init(QString content)
{
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
    avformat_network_init();
    fmt_context = nullptr;
    if((result = avformat_open_input(&fmt_context, filepath.toStdString().c_str(), 0, 0)) < 0)
    {
        qDebug()<<"failed to open"<<result;
    }
    if((result = avformat_find_stream_info(fmt_context,0))<0)
        qDebug()<<"failed to find_stream_info"<<result;
    audio_index = av_find_best_stream(fmt_context,AVMEDIA_TYPE_AUDIO,-1,-1,nullptr,0);//获取音频流的通道
    video_index = av_find_best_stream(fmt_context,AVMEDIA_TYPE_VIDEO,-1,-1,nullptr,0);//获取视频流的通道
    audio = fmt_context->streams[audio_index];
    video = fmt_context->streams[video_index];
    if(!audio)
    {
        qDebug()<<"audio is empty";
    }
    if(!video)
    {
        qDebug()<<"video is empty";
    }

    AVCodecParameters* audio_codecpar = audio->codecpar;

    const AVCodec* audio_decoder = avcodec_find_decoder(audio_codecpar->codec_id);

    codec_context = avcodec_alloc_context3(audio_decoder);

    if((result = avcodec_parameters_to_context(codec_context,audio_codecpar))<0)
    {
        qDebug()<<"fail to parameters acodec";
    }

    if((result = avcodec_open2(codec_context,nullptr,nullptr))<0)//初始化编码器上下文，如果在调用av_alloc_context3时已传入编码器，则参数2可以设置为nullptr;
    {
        qDebug()<<"fail to open acodec";
    }


    swr_context = nullptr;



    AVSampleFormat format_in = codec_context -> sample_fmt;
    AVSampleFormat format_out = AV_SAMPLE_FMT_S16;

    int sample_rate_in = codec_context -> sample_rate;
    sample_rate_out = 44100;

    auto& in_ch_layout = codec_context->ch_layout;
    AVChannelLayout out_ch_layout;
    out_ch_layout.nb_channels = 2;

    swr_context = swr_alloc_set_opts(swr_context, av_get_default_channel_layout(2),format_out,sample_rate_out,
                                     in_ch_layout.nb_channels,format_in,codec_context->sample_rate, NULL,NULL);


    if((result = swr_init(swr_context))<0)
    {
        qDebug()<<"fail to swr_init"<<result;
    }
    frame_number = video->nb_frames;
    duration = fmt_context->duration/AV_TIME_BASE;
    double fps = av_q2d(video->avg_frame_rate);
    qDebug()<<"码率:"<<codec_context->bit_rate;
    qDebug()<<"格式:"<<trans_sample_format(codec_context->sample_fmt)<<"to"<<trans_sample_format(format_out);
    qDebug()<<"通道:"<<codec_context->ch_layout.nb_channels;
    qDebug()<<"采样率:"<<codec_context->sample_rate;
    qDebug()<<"时长:"<<duration;
    qDebug()<<"帧数:"<<video->nb_frames;
    qDebug()<<"帧率:"<<fps;
    qDebug()<<"解码器:"<<audio_decoder->name;

    packet = av_packet_alloc();
    frame = av_frame_alloc();

    if(packet == nullptr)
    {
        qDebug()<<"bad alloc av packet";
    }

    if(frame == nullptr)
    {
        qDebug()<<"bad alloc av frame";
    }

    sleep_time = 1000 / fps * video->nb_frames / audio->nb_frames;


    audio_format = new QAudioFormat();
    audio_format->setSampleRate(sample_rate_out);     //设置采样率
    audio_format->setChannelCount(2);        //设置通道数
    audio_format->setSampleFormat(trans_sample_format(format_out));  //样本数据16位
    QAudioDevice info(QMediaDevices::defaultAudioOutput());
    if (!info.isFormatSupported(*audio_format)) {
        qWarning() << "Raw audio format not supported by backend, cannot play audio.";
        return;
    }

    audio_sink = new QAudioSink(*audio_format,this);
    audio_sink -> setBufferSize(100000);

    state = State::none;
    progress_ = 0;
    speed = 1;
    emit sync_inited();

//    play();
}

inline void AudioPlayer::handler(QString command)
{
    qDebug()<<"command:"<<command;
    if(state == State::playing && command=="pause"){
        pause();
    }
    else if(state == State::paused && command=="resume"){
        resume();
    }
    else if(command=="play"){
        if(state == State::paused)
            resume();
        else if(state == State::stopped)
            play();
        else if(state == State::none)
            play();
    }

}

#endif // AUDIOPLAYER_H

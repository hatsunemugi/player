#ifndef PACKET_H
#define PACKET_H

#include <QNetworkDatagram>
class Packet : public QObject
{
    Q_OBJECT
public:
    Packet(){}
    void setAudioPart(const char*);
    void setVideoPart(const char*);
    QByteArray &data();
private:
    QByteArray data_;
};

inline void Packet::setAudioPart(const char *audio)
{
    QDataStream stream(data_);
    stream<<"audio/pcm "<<audio<<"\r\n";
}

inline void Packet::setVideoPart(const char *video)
{
    QDataStream stream(data_);
    stream<<"video/QImage "<<video<<"\r\n";
}

inline QByteArray &Packet::data()
{
    return data_;
}


#endif // PACKET_H

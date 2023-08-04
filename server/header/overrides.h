#ifndef OVERRIDES_H
#define OVERRIDES_H
#include <iostream>
#include <QDebug>
static std::ostream& operator << (std::ostream& stream,QString s)
{
    stream<<s.toStdString();
    stream.flush();
    return stream;
}
//static QDebug& operator <<(QDebug& dbg, const std::string& s )
//{
//    qDebug()<<s.c_str();
//    return dbg;
//}
#endif // OVERRIDES_H

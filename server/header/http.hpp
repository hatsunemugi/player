#ifndef HTTP_HPP
#define HTTP_HPP

#include <QUrl>
#include <QHash>
#include "extras.h"
#include <QVariant>
#include "media.hpp"
#include <QRegularExpression>
#include "session.hpp"
#include "sql.hpp"

class HttpSession : public Session
{
    Q_OBJECT
public:
    HttpSession(asio::io_context& context);
    void get(QVariant);
    void post(QVariant);
    bool route(QString,std::function<QVariant (QVariant)>);
    QVariant trans(QString&);
signals:
    void table(QVariant);
public slots:
    void reply(QString);
    QVariant sqlHandler(QVariant);
    QVariant sqlHandler(QVariant,QVariant&);

private:
    Sql* sql;
private:
    QHash<QString,std::function<QVariant (QVariant)>> map;;
};

inline HttpSession::HttpSession(asio::io_context &context):Session(context),sql(nullptr)
{
    connect(this,&Session::message,this,&HttpSession::reply);
    route("/",[&](QVariant request){
        return "你好";
    });
    route("/version",[&](QVariant request){
        return 1.0;
    });
    route("/sql",[&](QVariant request){
        return "需要参数";
    });
    route("/sql/table",[&](QVariant request){
        QVector<QString> keys{"size"};
        return sqlHandler(keys);
    });
    route("/sql/table={}",[&](QVariant request){
        QVector<QString> keys = request.toStringList();
        keys.push_front("size");
        return sqlHandler(keys);
    });
    route("/sql/table={}/id={}",[&](QVariant request){
        QVector<QString> keys = request.toStringList();
        return sqlHandler(keys);
    });
    route("/sql/table={}/id={}/play",[&](QVariant request){
        QVariant path;
        QVector<QString> keys = request.toStringList();
        QString name = sqlHandler(keys,path).toString();
        Media* media = Instance::getMediaInstance();
        media->setAddress(address.to_string().c_str(),52000);
        media->open(path.toString());
        return "port:52000:52064:52128:52256";
    });
    route("/play",[&](QVariant request){
        return "需要参数";
    });
    route("/play={}",[&](QVariant request){
        Media* media = Instance::getMediaInstance();
        emit media->exec(request.toString());
        return request;
    });
    route("/seek",[&](QVariant request){
        return "需要参数";
    });
    route("/seek={}",[&](QVariant request){
        Media* media = Instance::getMediaInstance();
        double value = request.toString().toDouble();
        emit media->seek(value);
        return request;
    });
    route("/frame",[&](QVariant request){
        return "需要参数";
    });
    route("/frame/video",[&](QVariant request){
        return "nothing";
    });
    route("/frame/audio",[&](QVariant request){
        return "nothing";
    });
}
inline bool HttpSession::route(QString url,std::function<QVariant (QVariant)> function)
{
    if(!map.contains(url))
        map.insert(url,function);
    return true;
}

inline QVariant HttpSession::trans(QString& url)
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


inline void HttpSession::reply(QString content)
{
    auto array = content.split("\r\n");
    auto request = array[0].split(" ");
    qDebug()<<request;
    if(request.size()>2){
        QString url = request[1];
        auto keys = trans(url);
        qDebug()<<url<<keys;
        if(map.contains(url)){
            QString head =  "HTTP/1.1 200 OK\r\n\r\n"
                            "<!DOCTYPE html>"
                            "<html>"
                            "<head>"
                            "<meta charset=\"utf-8\"></meta>"
                            "<title>home</title>"
                            "</head>"
                            "<body>";
            auto f = map.value(url);
            QString result = head + f(keys).toString() + "</body></html>";
            write(result.toUtf8());
        }
    }
    else
    {
        write("config|:|windows10");
    }
}

inline QVariant HttpSession::sqlHandler(QVariant value)
{
    QVector<QString> keys = value.toStringList();
    QSqlQuery query;
    QString content;
    if(keys.size() == 1)
    {
        if(keys[0] == "size")
        {
            content = "select count(*) from sqlite_master";
        }
    }
    else if(keys.size() == 2)
    {
        if(keys[0] == "size" )
        {
            content = "select count(*) from " + keys[1];
        }
        else
        {
            content = "select name from " + keys[0] + " where id = "+ keys[1];
        }
    }
    if(sql == nullptr)
        sql = Instance::getSqlInstance("media");
    query = sql->exec(sql->database,content);
    if(query.next())
        return query.value(0);
    else
        return -1;
}

inline QVariant HttpSession::sqlHandler(QVariant value, QVariant &path)
{
    QVector<QString> keys = value.toStringList();
    QSqlQuery query;
    QString content;
    if(keys.size() == 1)
    {
        if(keys[0] == "size")
        {
            content = "select count(*) from sqlite_master";
        }
    }
    else if(keys.size() == 2)
    {
        if(keys[0] == "size" )
        {
            content = "select count(*) from " + keys[1];
        }
        else
        {
            content = "select name,path from " + keys[0] + " where id = "+ keys[1];
        }
    }
    if(sql == nullptr)
        sql = Instance::getSqlInstance("media");
    query = sql->exec(sql->database,content);
    if(query.next()){
        path = query.value(1);
        return query.value(0);
    }
    else
        return -1;
}

#endif // HTTP_HPP

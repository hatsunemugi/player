#ifndef PAINTEDITEM_H
#define PAINTEDITEM_H

#include <QtQml>
#include <QRectF>
#include <QImage>
#include <QObject>
#include <QPainter>
#include <QVector>
#include <QTimer>
#include <QQuickPaintedItem>

#include "extras.h"

using AspectRatio = enums::AspectRatio;

class PaintedItem : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
//    QML_EXTENDED_NAMESPACE(enums)
    Q_PROPERTY(QImage image READ getImage WRITE setImage NOTIFY imageChanged)
    Q_PROPERTY(AspectRatio AspectRatio READ getAspectRatio WRITE setAspectRatio NOTIFY aspectRatioChanged)
public:
    explicit PaintedItem(QQuickItem *parent = nullptr);
    void paint(QPainter *painter)override;
    QImage getImage(){return image;}
    void setImage(QImage image_){image=image_;emit imageChanged();}
    AspectRatio getAspectRatio(){return aspect_ratio;}
    void setAspectRatio(AspectRatio _aspect_ratio_){aspect_ratio = _aspect_ratio_; emit aspectRatioChanged();}
    void draw_autofill(QPainter *painter);
    void draw_stretch(QPainter *painter);
public slots:
    void setSize(int,int);
    void setScale(double);
signals:
    void imageChanged();
    void updated();
    void aspectRatioChanged();
    void fps(int);
    void retarget(int,int);
    void another();
private:
    int width;
    int height;
    int retarget_width;
    int retarget_height;
    QTimer timer;
    double scale;
    QImage image;
    QVector<QImage> buffer;
private:
    AspectRatio aspect_ratio;
};

inline PaintedItem::PaintedItem(QQuickItem *parent): QQuickPaintedItem{parent}
{
    connect(this,&PaintedItem::imageChanged,this,[&](){update();});
    aspect_ratio = AspectRatio::autofill;
    scale=1;
}

inline void PaintedItem::paint(QPainter *painter)
{
//    qDebug()<<"paint";
    if (painter == nullptr)
    {
        qDebug()<<"nullptr";
        return;
    }
    QRectF target;
    QRectF source;

    scale=QString::number(scale, 'f', 1).toDouble();
    if(scale>1)
    {
        double origin=(1-1/scale)/2;
        target = QRectF(0, 0, width, height); //建立目标矩形，表示从0，0开始位置绘制图像
        source = QRectF(image.width()*origin, image.height()*origin, image.width()/scale, image.height()/scale); //建立源矩形，表示从图像x，y位置开始，取l大小的区域去绘制

    }
    else if(scale<1)
    {
        double origin=(1-scale)/2;
        target = QRectF(width*origin, height*origin, width*scale, height*scale); //建立目标矩形，表示从0，0开始位置绘制图像
        source = QRectF(0, 0, image.width(), image.height()); //建立源矩形，表示从图像x，y位置开始，取l大小的区域去绘制

    }
    painter->setRenderHint(QPainter::Antialiasing, true);//抗锯齿
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true );
    if(scale==1)
    {
        switch(aspect_ratio)
        {
        case AspectRatio::autofill: draw_autofill(painter);break;
        case AspectRatio::stretch:  painter->scale((double)width/image.width(),(double)height/image.height()); painter->drawImage(0,0,image);break;
        case AspectRatio::fill: break;
        case AspectRatio::_16_9_: break;
        case AspectRatio::_4_3_: break;
        }



    }
    else
        painter->drawImage(target,image,source);
    emit updated();
}

inline void PaintedItem::draw_autofill(QPainter *painter)
{
    if(!image.isNull()){
        retarget_width = height * image.width()/ image.height();
        retarget_height = width * image.height()/image.width() ;
    }
//    draw_stretch(painter);
    if(width>=retarget_width){
//        emit retarget(retarget_width,height);
        painter->scale((double)height/image.height(),(double)height/image.height());
        painter->drawImage(0,0,image);
    }
    else if(height>=retarget_height){
//        emit retarget(width,retarget_height);
        painter->scale((double)width/image.width(),(double)width/image.width());
        painter->drawImage(0,0,image);
    }

}

inline void PaintedItem::draw_stretch(QPainter *painter)
{
    painter->scale((double)width/image.width(),(double)height/image.height());
    painter->drawImage(0,0,image);
}

inline void PaintedItem::setSize(int width_, int height_)
{
    width=width_;
    height=height_;

    if(!image.isNull()){
        retarget_width = height * image.width()/ image.height();
        retarget_height = width * image.height()/image.width() ;
        if(width>=retarget_width){
            emit retarget(retarget_width,height);
        }
        else if(height>=retarget_height){
            emit retarget(width,retarget_height);
        }
    }
    qDebug()<<width<<height;
}

inline void PaintedItem::setScale(double scale_)
{
    scale = scale_;
}

#endif // PAINTEDITEM_H

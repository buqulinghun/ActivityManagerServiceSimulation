#ifndef MYSVGITEM_H
#define MYSVGITEM_H

#include <QGraphicsSvgItem>
#include <QPointF>

#include "configuration.h"

class MySvgItem
{
public:
    MySvgItem(const QString &fileName, const QPoint& point);


  //  QGraphicsSvgItem *svgItem;
    QPoint offset;
    QPixmap svgItem;

};

class MyLineSvgItem : public MySvgItem
{
public:
    MyLineSvgItem(const QString &fileName, const QPoint& point);

    void setMapColor(const int &r, const int &g, const int &b);

    Rgb mapColor;   //绘图颜色，空间小时直接画直线

};

class MyPatternSvgItem : public MySvgItem
{
public:
    MyPatternSvgItem(const QString &fileName, const QPoint& point);

    void setPatternPara(const quint16 &mini, const quint16 &max, const bool &stg, const bool &con);

    //模板所特有
    bool stgLin;   //true is stg, false is lin 错综排列和线性排列
    bool conScl;   //true is con, false is scl 间距恒定和间距根据比例尺变化
    quint16 miniDist;  //模板填充最小间距
    quint16 maxDist;   //模板填充最大间距，当间距为con时无用

};





#endif // MYSVGITEM_H

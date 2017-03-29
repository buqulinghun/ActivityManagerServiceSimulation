/********************************************************************
 *日期: 2016-01-22
 *作者: 王名孝
 *作用: 绘制复杂线型图形项，参数：复杂图形，坐标点链表，颜色
 *修改:
 ********************************************************************/

#ifndef MYCOMPLEXLINEITEM_H
#define MYCOMPLEXLINEITEM_H

#include <QGraphicsItem>
#include <QVector>

#include "configuration.h"
#include "mysvgitem.h"

class MyComplexLineItem : public QGraphicsItem
{
public:
    MyComplexLineItem(MyLineSvgItem *line, const QVector<QPointF> &indices);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QVector<QPointF> index;   //坐标点
    MyLineSvgItem *lineItem;   //所需绘制图形项

};

#endif // MYCOMPLEXLINEITEM_H

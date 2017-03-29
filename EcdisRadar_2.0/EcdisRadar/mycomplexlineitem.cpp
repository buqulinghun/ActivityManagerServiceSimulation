/********************************************************************
 *日期: 2016-01-22
 *作者: 王名孝
 *作用: 绘制复杂线型图形项，参数：复杂图形，坐标点链表，颜色
 *修改:
 ********************************************************************/
#include "mycomplexlineitem.h"

#include <QPainter>


MyComplexLineItem::MyComplexLineItem(MyLineSvgItem *line, const QVector<QPointF> &indices)
{
    //赋值操作
    lineItem = line;
    index = indices;

}

QRectF MyComplexLineItem::boundingRect() const
{

}

void MyComplexLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    const int size = index.size() - 1;
    const QPixmap map = lineItem->svgItem;   //图像
    const int mapWidth = map.width();
    const Rgb color = lineItem->mapColor;   //图像颜色
    const QPointF offset = lineItem->offset;  //绘制时的偏移量

    for(int i=0; i < size; i++) {
        //每段线段填充复杂图形
        const QPointF start = index.at(i);
        const QLineF line = QLineF(start, index.at(i+1));
        const int lineLength = line.length();
        const double angle = line.angle();   //0度表示三点方向,绘制时需要加上90度
        const int num = lineLength / mapWidth + 1;   //绘制的图形个数

        painter->rotate(angle + 90);
        for(int j=0; j<num; j++) {
            painter->drawPixmap(start-offset, map);
        }   

    }

}








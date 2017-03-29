#include "mysvgitem.h"


/******************点物标图形项***************************/
MySvgItem::MySvgItem(const QString &fileName, const QPoint& point)
          :offset(0.0, 0.0)
{
  //  svgItem = new QGraphicsSvgItem(fileName);
    offset = point;
    svgItem = QPixmap(fileName);
}


/****************线物标图形项****************************/
MyLineSvgItem::MyLineSvgItem(const QString &fileName, const QPoint &point) :
        MySvgItem(fileName, point)
{
    mapColor.r = 0;
    mapColor.g = 0;
    mapColor.b = 0;
}

void MyLineSvgItem::setMapColor(const int &r, const int &g, const int &b)
{
    mapColor.r = r;
    mapColor.g = g;
    mapColor.b = b;
}


/****************填充物标图形项*************************/
MyPatternSvgItem::MyPatternSvgItem(const QString &fileName, const QPoint &point) :
        MySvgItem(fileName, point), stgLin(true), conScl(true), miniDist(0), maxDist(0)
{

}

void MyPatternSvgItem::setPatternPara(const quint16 &mini, const quint16 &max, const bool &stg, const bool &con)
{
    miniDist = mini;
    maxDist = max;
    stgLin = stg;
    conScl = con;
}

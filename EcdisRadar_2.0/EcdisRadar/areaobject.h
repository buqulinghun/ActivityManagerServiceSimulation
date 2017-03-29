#ifndef AREAOBJECT_H_
#define AREAOBJECT_H_

#include <vector>
#include <QVector>
#include <QString>
#include <QList>
#include <QMap>
#include <QPointF>
#include <QStringList>
#include"spaceobject.h"

#include "s57/s57.h"


//plain和symbol表的查表只是有些物标的周围线型是LS和LC的区别，其他都一样，要怎么区分呢。。。
//plain为LS, symbol为LC
//LS和LC只有一个，所以可以设定标志查找其为哪个表的

/*
typedef struct quoteedge{
    QVector<int> quoteClass;   //引用的物标种类
    QVector<int> index;    //该线段的坐标
    float valdco;        //物标有DEPCNT时的VALDCO
}EDGEQUOTE; */

class AreaObject:public SpaceObject
{
    public:
        AreaObject();
        ~AreaObject();




        //空间属性， 其属性信息后面需要再加
        std::vector<std::vector<int > > contours;

        QPointF centerPoint;  //中心坐标


};

#endif

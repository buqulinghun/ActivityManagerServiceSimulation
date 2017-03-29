/********************************************************************
 *日期: 2016-01-12
 *作者: 王名孝
 *作用: 存储点物标结构
 *修改:
 ********************************************************************/

#ifndef POINTOBJECT_H_
#define POINTOBJECT_H_

#include <iostream>
#include <vector>
#include <QVector>
#include <QString>
#include <QList>
#include <QMap>
#include <QPointF>
#include <QStringList>

#include "s57/s57.h"
#include "spaceobject.h"

/**
 * Point object
 * 分为simple和paper两种绘图方式：其中查询时属性组合不一样，显示物标名称不一样，但是字符提示一样,将两者共同保存
 */

//绘制类型枚举
/*enum PointType{
    SYMBOL = 1, CONDITIONAL, SYMBOL_AND_CONDITIONAL
}; */



class PointObject :public SpaceObject
{
    public:
        PointObject();
        ~PointObject();


        //空间属性
        int index;                    //经纬度坐标索引
        std::vector<attv_t> attvs;


};

#endif

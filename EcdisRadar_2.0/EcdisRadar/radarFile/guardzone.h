#ifndef GUARDZONE_H
#define GUARDZONE_H


#include "define.h"

class GuardZone
{
public:
    GuardZone();

    void setFirstPoint(const QPoint& scpt);  //设置绘图起点
    void setSecondPoint(const QPoint& scpt, quint8 flag=false); //设置绘图终点

    void updateScreenPoint();


    void paint(QPainter* p);

    quint8 crntPoint() const
    {  return flag_point;  }
    void setCrntPoint(quint8 flag)
    {   flag_point = flag;    }

    QPointF first_r_point() const
     {   return r_firstPoint;  }
    QPointF second_r_point() const
     {  return r_secondPoint;  }

    void clearPoint();


private:
    //经纬度坐标
    QPointF FirstPoint;
    QPointF SecondPoint;

    //极坐标，角度用弧度表示
    QPointF r_firstPoint;
    QPointF r_secondPoint;

    quint8 flag_point;

};

#endif // GUARDZONE_H

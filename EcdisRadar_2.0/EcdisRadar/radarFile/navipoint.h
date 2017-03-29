#ifndef NAVIPOINT_H
#define NAVIPOINT_H

#include "define.h"

/*********包含4个航路点********************/

class NaviPoint
{
public:
    NaviPoint();

    quint8 setPoint(const QPoint& scpt);
    quint8 delPoint(const QPoint& scpt);
    void updateScreenPoint();
    void resetCrntIndex()
    {	crntIndex = 0;	}

    void paint(QPainter* p);

    QPointF getNaviPoint(quint8 idx)
    {
        return naviPointFlag[idx] ? naviPoints[idx] : QPointF(181, 91);
    }

private:
    quint8 crntIndex;
    quint8 naviPointFlag[4];
    QPointF naviPoints[4];
    QPoint screenPoints[4];
};

#endif // NAVIPOINT_H

#include "navipoint.h"
#include "mouseoperation.h"

extern MouseOperation *lpMouseOpetarion;

/*****航路点设置绘制*****************************/

NaviPoint::NaviPoint()
{
    crntIndex = 0;
    naviPointFlag[0] = naviPointFlag[1] = naviPointFlag[2] = naviPointFlag[3] = 0;
}

quint8 NaviPoint::setPoint(const QPoint& scpt)
{   
        const quint8 lastIndex = crntIndex;

        naviPointFlag[crntIndex] = 1;
        naviPoints[crntIndex] = lpMouseOpetarion->screen_to_latitude(scpt);
        screenPoints[crntIndex] = scpt;
        ++crntIndex;
        if(crntIndex >= 4) crntIndex = 0;

        return lastIndex;
}

quint8 NaviPoint::delPoint(const QPoint& scpt)
{
    for(int i=0; i<4; i++)
    {
        if(!naviPointFlag[i])
            continue;
        const float dx = scpt.x() - screenPoints[i].x(), dy = scpt.y() - screenPoints[i].y();
        if(sqrt(dx*dx+dy*dy) < 10){  //距离在一定范围内表示同一个点
            naviPointFlag[i] = 0;
            return i;
        }
    }
    return 0xff;
}

void NaviPoint::updateScreenPoint()
{
    //更改导航点屏幕坐标
    for(int i=0; i<4; i++)
    {
        if(!naviPointFlag[i])
            continue;
        screenPoints[i] = lpMouseOpetarion->latitude_to_screen(naviPoints[i]);
    }
}

void NaviPoint::paint(QPainter* p)
{
    extern void drawSymbol(QPainter* painter, quint8 type, const QPoint& pt, quint8 ptsize);

    for(int i=0; i<4; i++)
    {
        if(!naviPointFlag[i])
            continue;
        //画正方形加十字
        drawSymbol(p, SYMBOL_SQUARE_CROSS, screenPoints[i], 4);
        p->drawText(screenPoints[i]+QPoint(5, 4), QString("%1").arg(i+1));
    }
}

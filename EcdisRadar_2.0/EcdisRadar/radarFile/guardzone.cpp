#include "guardzone.h"
#include "glwidget.h"
#include "mouseoperation.h"



extern GLWidget *pView;
extern MouseOperation *lpMouseOpetarion;

GuardZone::GuardZone()
{
    flag_point = 0;
}



void GuardZone::setFirstPoint(const QPoint &scpt)
{
    if(! flag_point) {
        //保存经纬度坐标
        FirstPoint = lpMouseOpetarion->screen_to_latitude(scpt);
        SecondPoint = lpMouseOpetarion->screen_to_latitude(scpt);
        flag_point = 1;
    }
    updateScreenPoint();

}

void GuardZone::updateScreenPoint()
{
    RTHETA_POINT r1,r2;
    LATITUDE_POINT z1,z2;
    z1.setPoint(FirstPoint.x(), FirstPoint.y());
    z2.setPoint(SecondPoint.x(), SecondPoint.y());
    r1 = lpMouseOpetarion->latitude_to_rtheta(z1);
    r2 = lpMouseOpetarion->latitude_to_rtheta(z2);

    r_firstPoint = QPointF(r1.x(), r1.y()/*RADIANTODEGREE(r1.y())*/);
    r_secondPoint = QPointF(r2.x(), r2.y()/*RADIANTODEGREE(r2.y())*/);

}

void GuardZone::setSecondPoint(const QPoint &scpt, quint8 flag)
{
    if(flag_point)
        SecondPoint = lpMouseOpetarion->screen_to_latitude(scpt);
    updateScreenPoint();

    //此处选择警戒区域后需要做后面的处理，比如计算所在区域的像素点，提取像素点等
    if(flag)
        pView->setGuardZonePixel();

}
void GuardZone::clearPoint()
{
    FirstPoint = lpMouseOpetarion->screen_to_latitude(QPoint(512, 512));
    SecondPoint = lpMouseOpetarion->screen_to_latitude(QPoint(512, 512));
    updateScreenPoint();
    flag_point = 0;
}


void GuardZone::paint(QPainter *p)
{
   // if((qAbs(FirstPoint.x() - SecondPoint.x()) < 0.00001) && (qAbs(FirstPoint.y() - SecondPoint.y()) < 0.00001))
    //    return;
    //if(!flag_point)
      //  return;

    const float rotation = DEGREETORADIAN(pView->rotation());  //返回的是角度，不是弧度
  /*  LATITUDE_POINT fp1, fp2;
    fp1.setPoint(FirstPoint.x(), FirstPoint.y());
    fp2.setPoint(SecondPoint.x(), SecondPoint.y());
    const RTHETA_POINT pt1 = lpMouseOpetarion->latitude_to_rtheta(fp1);
    const RTHETA_POINT pt2 = lpMouseOpetarion->latitude_to_rtheta(fp2);  */
    RTHETA_POINT pt1,pt2;
    pt1.setPoint(r_firstPoint.x(), r_firstPoint.y());
    pt2.setPoint(r_secondPoint.x(), r_secondPoint.y());
    double r01 = pt1.r(), a01 = pt1.theta();
    double r02 = pt2.r(), a02 = pt2.theta();

    if (r01 > r02)  //距离调换
    {
        double r = r01;
        r01 = r02;
        r02 = r;
    }

    //限制区域，不能出界
    p->save();
    QPainterPath clip;
    clip.addEllipse(QPointF(512,512), 500,500);
    p->setClipPath(clip);

    const double rmin = r01;
    const double rmax = r02;
    const double abgn = a01, aend = a02;
    const double da = (aend < abgn ? aend-abgn+M_2PI : aend-abgn); //以弧度计算

    //计算角落的四个点
    RTHETA_POINT rtheta;
    rtheta.setPoint(rmin, abgn);
    SCREEN_POINT sc1 = lpMouseOpetarion->rtheta_to_screen(rtheta);
    rtheta.setPoint(rmax, abgn);
    SCREEN_POINT sc2 = lpMouseOpetarion->rtheta_to_screen(rtheta);
    rtheta.setPoint(rmax, aend);
    SCREEN_POINT sc3 = lpMouseOpetarion->rtheta_to_screen(rtheta);
    rtheta.setPoint(rmin, aend);
    SCREEN_POINT sc4 = lpMouseOpetarion->rtheta_to_screen(rtheta);


    const SQUARE_POINT sq0 = {0, 0};
    const SCREEN_POINT center = lpMouseOpetarion->square_to_screen(sq0);
    QLineF line1 (center.x(), center.y(), sc1.x(), sc1.y());
    QLineF line2 (center.x(), center.y(), sc2.x(), sc2.y());
    const int r1 = line1.length();
    const int r2 = line2.length();
    QRect rc1 (center.x()-r1, center.y()-r1, 2*r1, 2*r1);
    QRect rc2 (center.x()-r2, center.y()-r2, 2*r2, 2*r2);



    p->drawLine(sc1.toPoint(), sc2.toPoint());
    p->drawArc(rc2, 90*16-(abgn+rotation)*5760.0/M_2PI, -da*5760.0/M_2PI);
    p->drawLine(sc3.toPoint(), sc4.toPoint());
    p->drawArc(rc1, 90*16-(abgn+rotation)*5760.0/M_2PI, -da*5760.0/M_2PI);

    //qDebug() << sc1.toPoint() << sc2.toPoint() << sc4.toPoint();
    p->restore();

}

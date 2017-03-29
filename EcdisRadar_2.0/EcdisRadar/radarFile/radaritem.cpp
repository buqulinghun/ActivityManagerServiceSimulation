#include "radaritem.h"
#include "ecdis.h"
#include "interact.h"

#include <math.h>
#include <QGraphicsSceneMouseEvent>



double EachDegreeSin2[360], EachDegreeCos2[360];  //外围刻度线绘制使用

extern SYSTEMINFO RadarSysinfo;
extern Interact * lpInteract;

quint8 FirstWaitTime = 0;

RadarItem::RadarItem()
{
    //默认颜色设置  green
   color_flag = 0;
   color_alpha = 0x8f;
   refreshEchoColor();
    echobackColor = 0x00000000;   //背景不显示


    m_offset_x = 0;
    m_offset_y = 0;
    m_offsetPoint = QPointF(0, 0);
    offset_flag = 0;

    azi_adjust = 450;


    //外围方位圈使用
    for(int i=0; i<360; i++)
    {
        const double radian = DEGREETORADIAN(i);
        EachDegreeSin2[i] = sin(radian);
        EachDegreeCos2[i] = cos(radian);
    }

    m_screenIndex = new qint32[DIAMETER*DIAMETER];

    //将显示格中的每个像素点极坐标转换成直角坐标 并利用直角坐标计算出对应的索引值，
    for(quint16 x = 0; x < DIAMETER; x++)
        {
        for(quint16 y = 0; y < DIAMETER; y++)
                {
                        int x_offset = x - MAXRADIUS;
                        int y_offset = y - MAXRADIUS;
                        const qint32 idx = y * DIAMETER + x;  //得到一维坐标索引
                        const int r = (int)sqrt((double)(x_offset*x_offset+y_offset*y_offset));  //计算半径
                        int i = (int)((atan2((double)x_offset, (double)y_offset) * MAXDISPAZI) / (2 * M_PI));  //计算角度
                        if(i < 0) i += MAXDISPAZI;
                        m_pixelIndex[i][r].append(idx);
        }
    }

    // 从虚拟视图索引到实际的屏幕视图索引映射
    for(int y=0; y<DIAMETER; y++)
    {
            const bool y_valid = (y >= RADIUS && y < 3*RADIUS);
            for(int x=0; x<DIAMETER; x++)
            {
                    const bool scrn_valid = (y_valid && (x >= RADIUS && x < 3*RADIUS));
                    const int idx_virt = y * DIAMETER + x;

                    const int idx_scrn = (scrn_valid ? (3*RADIUS -1 - y)*800 + (x-RADIUS) : -1);

                    m_screenIndex[idx_virt] = idx_scrn;

            }
    }



    //设置回波背景索引
    for(int i=0; i < MAXRADIUS; i++)
        for(int j=0;j < MAXRADIUS; j++) {
            if(sqrt((double)((i-RADIUS)*(i-RADIUS) + (j-RADIUS)*(j-RADIUS))) < RADIUS)
                echoviewCircle.append(j*MAXRADIUS + i);
            else
                echoviewOutCircle.append(j*MAXRADIUS + i);
        }



     CurrentRange = new quint16[MAXREALAZI2];  //360个角度
      setCurrentRange();
      //设置量程
      setRange(RadarSysinfo.RangeScale.range(), RadarSysinfo.RangeScale.scale());


      initEchoView();

    //设置回波旋转角度
  //  setAziAdjust(MenuConfig.installMenu.aziAdjust);

}


RadarItem::~RadarItem()
{

    if(CurrentRange)
        delete[] CurrentRange;
    if(m_screenIndex)
        delete[] m_screenIndex;

}

void RadarItem::setCurrentRange()
{
    //计算当前中心到圆周的距离
    //根据角度和当前的位置，可以写出直线方程
    //先计算出当前中心相对原中心的坐标
    const int MaxAzimuth = MAXREALAZI2;
    const int MaxDispRadius = RADIUS;
    //没有偏心时的默认值
    if(m_offset_x == 0 && m_offset_y == 0 )
    {
        for(int i = 0; i < MaxAzimuth; i++){
           CurrentRange[i] = MaxDispRadius;
        }
        m_offset_maxradius = RADIUS;
        return;
    }

    const qint16 offset_x = m_offset_x;
    const qint16 offset_y = m_offset_y;
    const int Sign = (offset_y >= 0 ? -1 : 1);
    double TempRange = fabs((double)offset_x);
    //计算原中心到现在中心的距离
    const double  Range2 = sqrt((double)(offset_x * offset_x + offset_y * offset_y));
    //先计算角度为0和180度的半径
    //如果偏心中心在y轴上
    if(offset_x == 0) {
        //计算出0度和180度的距离
        CurrentRange[0] = MaxDispRadius - offset_y;  //MAX_POINTS为半径
        CurrentRange[MaxAzimuth/2] = MaxDispRadius + offset_y;///&&&&&&&&&&&&&&&
    }
    //否则偏心点不在y轴上
    else
    {
        //计算弦的半径长度
        double Range1 = (double)sqrt((double)(MaxDispRadius * MaxDispRadius - offset_x * offset_x));
        CurrentRange[0] = Range1 - offset_y;
        CurrentRange[MaxAzimuth/2] = Range1 + offset_y;
    }
    //计算其它角度的距离
    //如果中心点在原坐标的2，4象限
    if(((offset_x <= 0) && (offset_y >= 0)) ||
       ((offset_x > 0) && (offset_y < 0)) )
    {
        //从触发1开始计算
        for(int kk=1; kk < MaxAzimuth/2; kk++)
        {
            //计算直线的斜率
            double LineK = tan(M_PI/2-2*M_PI*kk/MaxAzimuth);
            //直线的方程是：k*x-y+TempY-k*TempX=0;
            //计算原中心到此直线的距离 ,即点到直线的距离公式
            double Range1 = fabs(offset_y-LineK*offset_x)/sqrt(LineK*LineK+1);
            //计算弦的中心点到现在中心的距离
            quint32 Range3 = (quint32)sqrt(Range2*Range2-Range1*Range1);
            //计算弦的半径
            quint32 Range4 = (quint32)sqrt(MaxDispRadius*MaxDispRadius-Range1*Range1);
            //计算此触发的中心点到圆弧的距离
            if(TempRange<=Range1)
            {
                TempRange = Range1;
                CurrentRange[kk] = Range4+Sign*Range3;
                CurrentRange[kk+MaxAzimuth/2] = Range4-Sign*Range3;
            }
            else
            {
                CurrentRange[kk] = Range4-Sign*Range3;
                CurrentRange[kk+MaxAzimuth/2] = Range4+Sign*Range3;
            }
        }
    }
    //如果中心点在圆坐标的1，3象限
    else
    {
        //从触发MAX_TRIGES/2-1开始计算
        for(int kk=MaxAzimuth/2-1;kk>0;kk--)
        {
            //计算直线的斜率
            double LineK = tan(M_PI/2-2*M_PI*kk/MaxAzimuth);
            //直线的方程是：k*x-y+TempY-k*TempX=0;
            //计算原中心到此直线的距离
            double Range1 = fabs(offset_y-LineK*offset_x)/sqrt(LineK*LineK+1);
            //计算弦的中心点到现在中心的距离
            quint32 Range3 = (quint32)sqrt(Range2*Range2-Range1*Range1);
            //计算弦的半径
            quint32 Range4 = (quint32)sqrt(MaxDispRadius*MaxDispRadius-Range1*Range1);
            //计算此触发的中心点到圆弧的距离
            if(TempRange<=Range1)
            {
                TempRange = Range1;
                CurrentRange[kk] = Range4-Sign*Range3;
                CurrentRange[kk+MaxAzimuth/2] = Range4+Sign*Range3;
            }
            else
            {
                CurrentRange[kk] = Range4+Sign*Range3;
                CurrentRange[kk+MaxAzimuth/2] = Range4-Sign*Range3;
            }
        }
    }

    //计算最大的显示半径,在画距离圈的时候使用
    for(int kk=0; kk<MaxAzimuth; kk++) {
        if(CurrentRange[kk] > m_offset_maxradius)
            m_offset_maxradius = CurrentRange[kk];

    }
}

void RadarItem::setRange(float rng, float sca)
{
    m_range = rng;
    m_ration = RADIUS / m_range;

    //setCurrentRangeScale(sca);

    //设置相应的绘图参数
    const quint8 RngCount = 19;
    //每个量程的径向数据
    const float rnguints[RngCount] = {78, 155, 310, 465, 310, 465, 414, 465, 496, 465, 496, 496, 496, 496, 496, 496, 496, 485, 496};

    const quint8 i = RadarSysinfo.RangeScale.rngIndex();
    if(i < RngCount) {
        m_pixelPerRnguint = (float) RADIUS/rnguints[i];
    }


}

#define SetFbBuffer(tr, p, clr0) {  \
const QList<qint32> &idx_list = m_pixelIndex[tr][p];  \
const int idx_size = idx_list.size();   \
for(int i=0; i<idx_size; i++) {   \
    const int virt_idx = idx_list[i]+m_offset_index;   \
    if(virt_idx<0 || virt_idx>=2560000) { /*DIAMETER*DIAMETER*/ continue;  }  \
    const qint32 scrn_idx = m_screenIndex[virt_idx];   \
    if(scrn_idx < 0) {  continue;  }   \
    else {    echoView[scrn_idx] = clr0;   }  \
    }  \
}

#define NEXIDX(v, a)  ((v)+(a) < MAXDISPAZI ? (v)+(a) : (v)+(a)-MAXDISPAZI)

#define DrawScanLine(tr, pmax, color) {  \
int tr1,tr2,tr3,tr4,tr5,tr6,tr7,tr8,tr9,tr10,tr11,tr12;\
tr1 = NEXIDX(tr, 1);\
tr2 = NEXIDX(tr, 2);\
tr3 = NEXIDX(tr, 3);\
tr4 = NEXIDX(tr, 4);\
tr5 = NEXIDX(tr, 5);\
tr6 = NEXIDX(tr, 6);\
    tr7 = NEXIDX(tr, 7);\
    tr8 = NEXIDX(tr, 8);\
    tr9 = NEXIDX(tr, 9);\
    tr10 = NEXIDX(tr, 10);\
    tr11= NEXIDX(tr, 11);\
    tr12 = NEXIDX(tr, 12);\
for(int p=0; p<pmax; ++p)\
{\
    SetFbBuffer(tr1, p, color);\
    SetFbBuffer(tr2, p, color);\
    SetFbBuffer(tr3, p, color);\
    SetFbBuffer(tr4, p, color);\
    SetFbBuffer(tr5, p, color);\
    SetFbBuffer(tr6, p, color);\
    SetFbBuffer(tr7, p, color);\
    SetFbBuffer(tr8, p, color);\
    SetFbBuffer(tr9, p, color);\
    SetFbBuffer(tr10, p, color);\
    SetFbBuffer(tr11, p, color);\
    SetFbBuffer(tr12, p, color);\
}\
}

//单色模式下的绘图定义
#define DrawPixel2(tr, p, echoflag) {  \
if(tr >= MAXDISPAZI)  {  continue;  };  \
const QList<qint32> &idx_list = m_pixelIndex[tr][p];  \
const int idx_size = idx_list.size();   \
for(int i=0; i<idx_size; i++) {   \
    const int virt_idx = idx_list[i]+m_offset_index;   \
    if(virt_idx<0 || virt_idx>=2560000) { /*DIAMETER*DIAMETER*/ continue;  }  \
    const qint32 scrn_idx = m_screenIndex[virt_idx];   \
    if(scrn_idx < 0) {  continue;  }   \
    switch(echoflag) {  \
    case 0x01:  \
        echoView[scrn_idx] = echoclutterColor; \
    break; \
    case 0x02:   \
        echoView[scrn_idx] = echoclutterColor; \
    break; \
    case 0x03:  \
        echoView[scrn_idx] = secondEchoColor; \
    break; \
    case 0x04:   \
         echoView[scrn_idx] = secondEchoColor; \
    break; \
    case 0x05:  \
         echoView[scrn_idx] = secondEchoColor; \
    break; \
    case 0x06:   \
        echoView[scrn_idx] = echoColor; \
      /*  timeDelay[scrn_idx] = m_keepTime_5s; */ \
    break; \
    case 0x07:   \
        echoView[scrn_idx] = echoColor; \
     /*   timeDelay[scrn_idx] = m_keepTime_5s; */ \
    break; \
    default:  \
        echoView[scrn_idx] = echobackColor; \
   break; \
  } \
}  \
}

void RadarItem::drawEchoData()
{

            QMutexLocker locker(&m_echoReceivedMutex);
            if(m_lastEchoData.isEmpty())   return;
            int lastAngle = 0;

            const int size = m_lastEchoData.size() - 1;  //留下一个作为后面的角度限制
            for(int i=0; i < size; ++i) {
                const ECHODATA tempData = m_lastEchoData.takeFirst();
                const int angle = m_lastEchoData[0].angle;

                //设置量程
                if(RadarSysinfo.RangeScale.rngIndex() != tempData.range) {
                     RadarSysinfo.RangeScale.setRange(tempData.range);
                     this->setRange(RadarSysinfo.RangeScale.range(), RadarSysinfo.RangeScale.scale());
                     //更新显示条
                   //  lpMainWindow->updateDispCtrl();
                    // qDebug()<<"range is"<<tempData.range;
                }

                if(tempData.angle < MAXDISPAZI && (tempData.length < 1000)) {
                    const int rota = rotation() * 10;  //返回的角度非弧度值
                    int azimuth1 = tempData.angle % 3600;
                    int azimuth2 = angle % 3600;



                    azimuth1 = azimuth1 + rota + azi_adjust;  //加上方位调节
                    azimuth2 = azimuth2 + rota + azi_adjust;

                    //角度超出范围
                    if(azimuth1 >= MAXDISPAZI) azimuth1 -= MAXDISPAZI;
                    if(azimuth1 < 0)  azimuth1 += MAXDISPAZI;
                    if(azimuth2 >= MAXDISPAZI) azimuth2 -= MAXDISPAZI;
                    if(azimuth2 < 0)  azimuth2 += MAXDISPAZI;
                    if(azimuth2 < azimuth1)      azimuth2 = azimuth2 + MAXDISPAZI;

                    if((azimuth2 - azimuth1) > 200) {
                        //qDebug()<<"azimuth1,azimuth2,crntIndex,angle1,angle2"<< azimuth1 << azimuth2 <<tempData.angle<<angle;
                        continue;
                    }
                    lastAngle = azimuth2;

                    const int pmax = CurrentRange[azimuth1/10] - 1;  //最大显示半径
                    const int length = tempData.length;

                    int p = 0,  m = 0;
                    float  pp = 0.0;
                    for(p=pp,m=0; m<length && p<pmax; m++)
                    {

                        //使用高4位作为前一个数据
                        {
                        pp += m_pixelPerRnguint;  //一个距离单元对应的像素宽度
                        const int p0 = (int)pp;
                        if(p != p0) {
                            const quint8 echoflag = ((tempData.echo[m] & 0xf0) >> 4);
                            const int pmax0 = (p0 < pmax ? p0 : pmax);
                            for(; p < pmax0; p++) {
                                for(int azi=azimuth1; azi < azimuth2; azi++) {
                                    if(azi < 0 ) azi += MAXDISPAZI;
                                    const int tr = (MAXDISPAZI > azi ? azi : (azi - MAXDISPAZI));
                                        DrawPixel2(tr, p,echoflag);
                                }  //end azi
                            }

                        }
                        }

                        //使用低4位作为后一个数据
                        {
                            pp += m_pixelPerRnguint;  //一个距离单元对应的像素宽度
                            const int p0 = (int)pp;
                            if(p != p0) {
                                const quint8 echoflag = (tempData.echo[m] & 0x0f);
                                const int pmax0 = (p0 < pmax ? p0 : pmax);
                                for(; p < pmax0; p++) {
                                    for(int azi=azimuth1; azi < azimuth2; azi++) {
                                        if(azi < 0 ) azi += MAXDISPAZI;
                                        const int tr = (MAXDISPAZI > azi ? azi : (azi - MAXDISPAZI));
                                            DrawPixel2(tr, p,echoflag);
                                    }  //end azi
                                }

                            }
                        }


                   }  //end for

                }  //end if

            }   //end for


       //画天线扫描线
       int pmax = CurrentRange[lastAngle / 10] - 5;   //得到长度
       pmax = pmax > 0 ? pmax : 10;
       DrawScanLine(lastAngle, pmax, 0x4f66ff99);
       lastAngle += 12;
       DrawScanLine(lastAngle, pmax, 0x6f66ff99);
}


// 画外围的方位圈,旋转角度与回波角度不一样
void RadarItem::drawOuterAzimuthCircle(QPainter* p0)
{
    QPainter& p = *p0;
    int radius = RADIUS;

        // 雷达中心点所对应的屏幕坐标(以雷达为圆心)
        const QPoint center = QPoint(RADIUS, RADIUS);  //square_to_screen(QPointF(0,0));
        QFont font = p.font();
        const int left = QFontMetrics(font).boundingRect(("456")).width();
        const int right = QFontMetrics(font).boundingRect(("456")).height() / 2;

        // 画方位刻度线
        for(int i=0; i<360; i++)
        {
            int idx = i + rotation2();  //旋转角度，后面更改,感觉不需要
            if(idx >= 360)
                idx -= 360;
            else if(idx < 0)
                idx += 360;

             //刻度是5的倍数时，刻度线加长一点
            qint16 radius0 = radius - 4;
            if((i%5)==0)
                radius0 = radius - 8;
            else if((i%10)==0)
                radius0 = radius - 12;

            if(radius0 < 0)  	radius0 = 0;

            int x1 = center.x() + radius * EachDegreeSin2[idx];
            int y1 = center.y() - radius * EachDegreeCos2[idx];
            int x2 = center.x() + radius0 * EachDegreeSin2[idx];
            int y2 = center.y() - radius0 * EachDegreeCos2[idx];
            p.drawLine(x1, y1, x2, y2);


            if(i % 10 == 0)
            {
                radius0 = radius + 3;
                int x3 = center.x() + radius0 * EachDegreeSin2[idx];
                int y3 = center.y() - radius0 * EachDegreeCos2[idx];

                p.save();
                p.setRenderHint(QPainter::Antialiasing, false);
                p.translate(x3, y3);
                if((i == 90) || (i == 270))
                    p.rotate(idx);

                if(i == 0 )
                    p.drawText(-left/6, 0, QString::number(i));
                else if(i == 90 || i == 270)
                    p.drawText(-left/3, 0, QString::number(i));
                else if(i == 180)
                    p.drawText(-left/3, right, QString::number(i));
                else if( i < 100)
                    p.drawText(0, 0, QString::number(i));
                else if(i < 180)
                    p.drawText(0, right, QString::number(i));
                else if(i < 270)
                     p.drawText(-left*4/5, right, QString::number(i));
                else
                     p.drawText(-left*4/5, 0, QString::number(i));

              p.restore();
           }
        }

        // 画外圈(以屏幕中心为圆心)
        p.drawEllipse (QPointF(RADIUS, RADIUS), RADIUS, RADIUS);

}
//画指引线
void RadarItem::drawGuildLine(QPainter *p0)
{
  //  if(!SystemInfo.ViewDisplay.guildline)
   //     return;

    //船艏线应该和图像选转角度一样
    float  dispAzi = rotation();

    QPoint m_scCenter;
    //偏心或者真运动的情况下图形中心都在偏心点
    if(RadarSysinfo.offset)
        m_scCenter = QPoint(m_offsetPoint.x(), m_offsetPoint.y());  //雷达中心
    else
        m_scCenter = QPoint(RADIUS, RADIUS);
    const int m_radius = CurrentRange[(quint16)dispAzi];

    const QPoint screen_point = QPoint((m_radius * EachDegreeSin2[(int)dispAzi]+m_scCenter.x()), (m_scCenter.y()-m_radius * EachDegreeCos2[(int)dispAzi]));

    p0->drawLine(m_scCenter, screen_point);

}
//画VRM/EBL
void RadarItem::drawVrmEbl(QPainter *painter)
{
 /*   const QColor color = m_varlineColor.lighter(MenuConfig.dispMenu.varlineBright);

    QPen pen(color);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    //画两个
    for(quint8 i=0;i<2;i++)
    {
        VRMEBL& vrmebl = SystemInfo.VrmEblCtrl.vrmebl[i];
        if(!vrmebl.show)
            continue;

        if(1 == i) {
                pen.setDashPattern(m_dashes);
                painter->setPen(pen);
        }

        const QPoint sc = QPoint(m_offsetPoint.x(),m_offsetPoint.y());

        const float r = CurrentRange[(quint16)vrmebl.ebl << 1];
        const float rp = vrmebl.vrm * ration();  //vrm显示半径



        // 显示VRM
        if((vrmebl.show & 0x01) && ((!SystemInfo.ViewDisplay.offset && vrmebl.vrm < range()) || (SystemInfo.ViewDisplay.offset && vrmebl.vrm < 2*range())) && BIGZERO(rp))
        {{
                 const int x0 = (int)(sc.x()-rp), y0 = (int)(sc.y()-rp), r = (int)(rp*2);
                 painter->drawEllipse (x0, y0, r, r);
            }
        }

        // 显示EBL
        if(vrmebl.show & 0x02)
        {

        const QPoint screen_point = QPoint(r*EachDegreeSin2[(int)vrmebl.ebl] + sc.x(), sc.y() - r*EachDegreeCos2[(int)vrmebl.ebl]);

        painter->drawLine(sc, screen_point);  //圆心为起点的直线

        }
    } */

}
//画p显刻度线
void RadarItem::drawScanLineP(QPainter *p0)
{

 /*   QPainter &p = *p0;
    QPen pen = p.pen();


    if(!SystemInfo.ViewDisplay.rngring)
        return;

    const float rngScale = CurrentRangeScale;
    const float aziScale = CurrentAzimuthScale;


     // 计算雷达中心原点相对窗口中心的坐标位置,就是雷达中心的屏幕坐标
     // 该点作为距离刻度线和方位刻度线的起始位置
     const QPoint sc_pt = QPoint(m_offsetPoint.x(), m_offsetPoint.y());  //square_to_screen(QPointF(0,0));

     pen.setWidth(1);
     p.setBrush(Qt::NoBrush);

    //绘制距离刻度线
    if(BIGZERO(rngScale))
    {
        //调整起始距离为dr的整数倍
        const float dr = rngScale;
        const float dp = dr*ration();  //距离圈的像素半径

        if(!SystemInfo.ViewDisplay.offset) {
            float rp0;
            for(rp0=dp; rp0 < m_offset_maxradius; rp0+=dp)  //把最外圈放在画方位圈那里绘出来
            {
                const int diameter =(int)(rp0);  //半径
                p.drawEllipse (sc_pt, diameter, diameter);
            }
        }else {  //偏心显示使用画圆弧，因为不能使用裁剪功能
            float rp0;

            for(rp0=dp; rp0 < m_offset_maxradius; rp0+=dp)  //把最外圈放在画方位圈那里绘出来
            {
                const int diameter =(int)(rp0);  //距标圈半径

                 { //半径小于等于最短距离时画圆
                    p.drawEllipse (sc_pt, diameter, diameter);
                }
            }

        }  */
    }



//////////////////接受回波数据处理函数/////////////////
void RadarItem::setEchoData(ECHODATA& m_data)
{

    if((m_data.packetNum <= MAXPACKETNUM) && (m_data.packetNum > 0)) {
        //数据同步
        QMutexLocker locker(&m_echoReceivedMutex);
        {
             m_lastEchoData.append(m_data);
        }
        locker.unlock(); //数据解锁
    }else {
        qDebug()<<"packetnum error:"<<m_data.packetNum;
        return;
    }

    //绘制雷达回波
    if(m_lastEchoData.size() > 30) {
        drawEchoData();
        update();

    }

}



QRectF RadarItem::boundingRect() const
{
    return QRectF(0, 0, 800, 800);
}

void RadarItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QImage image((uchar *)&echoView, 800 ,800, QImage::Format_ARGB32);

    painter->drawImage(QPoint(0, 0), image);


    QColor color = QColor(Qt::red);
    color.setAlpha(color_alpha);
    painter->setPen(color);

    drawOuterAzimuthCircle(painter);
    drawGuildLine(painter);
}


void RadarItem::initEchoView(void)
{
    quint32 z;

    //设置背景色，后续要根据选择更改
    quint32 size=echoviewOutCircle.size();
    for(z=0; z<size; z++)
        echoView[echoviewOutCircle.at(z)] = 0x00ff0000;

    size = echoviewCircle.size();
    for(z=0; z<size; z++)
        echoView[echoviewCircle.at(z)] = 0x00ffff00;

}

void RadarItem::setOffsetStatus(quint8 flag)
{
    if(flag) {
        offset_flag = flag;
      //  setCursor(Qt::CrossCursor);
    }else {
       // setCursor(Qt::ArrowCursor);
        lpInteract->setOffset(0);
        m_offset_x = 0;
        m_offset_y = 0;
        m_offset_index = 0;
        m_offsetPoint = QPointF(0, 0);
        setCurrentRange();
    }
}
void RadarItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(offset_flag) {
        QPointF pos = event->lastScenePos();
        pos = mapFromScene(pos.x(), pos.y());
        QPointF pos2 = QPointF(pos.x()-RADIUS, pos.y()-RADIUS);
        if(sqrt(pos2.x() * pos2.x() + pos2.y()*pos2.y()) < RADIUS) {
            m_offsetPoint = pos;
            m_offset_x = pos.x() - RADIUS +1;
            m_offset_y = pos.y() - RADIUS + 1;
            m_offset_index = -m_offset_y*DIAMETER + m_offset_x;  //虚拟视图偏心索引
            //由于显示关系，上下位置取反
            m_offset_y = -m_offset_y;
            setCurrentRange();   //得到每个角度的长度
           // setCursor(Qt::OpenHandCursor);
            lpInteract->setOffset(1);
            offset_flag = 0;
        }
    }

 //  QGraphicsItem::mousePressEvent(event);
}









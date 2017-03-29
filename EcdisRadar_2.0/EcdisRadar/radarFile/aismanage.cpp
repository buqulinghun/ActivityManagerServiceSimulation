#include "aismanage.h"
#include "glwidget.h"
#include "boatinfo.h"
#include "dialog.h"
#include "boatalarm.h"
#include "TargetManage/TargetManage_global.h"
#include "TargetManage/track.h"
#include "mouseoperation.h"
#include <math.h>



extern GLWidget *pView;
extern MENUCONFIG MenuConfig;
extern Dialog*   m_lpDialog;
extern boatalarm* m_boatAlarm;
extern SYSTEMINFO SystemInfo;

extern Alarm* lpAlarm ;
extern SYSTEM_PARA g_systemPara;
extern MouseOperation *lpMouseOpetarion;






////////设置s52对象/////////////////////////////////////
typedef enum S52_objType {
    S52__META_T  = 0,         // meta geo stuff (ex: C_AGGR)
    S52_AREAS_T  = 1,         // 1
    S52_LINES_T  = 2,         // 2
    S52_POINT_T  = 3,         // 3
    S52_N_OBJ_T  = 4          // number of object type
} S52_objType;
// listAttVal format: "att1:val1,att2:val2,..."
S52_objHandle  S52_newMarObj(const char *plibObjName, S52_objType objType, unsigned int xyznbr, double *xyz, const char *listAttVal)
{
    return NULL;
}
// return NULL if S52_objHandle was successefully deleted (obj freed)
S52_objHandle  S52_delMarObj(S52_objHandle obj)
{
    return NULL;
}
// set Position & Vector & Dimension of OWNSHP & VESSEL
S52_objHandle S52_setDimension(S52_objHandle objH, double a, double b, double c, double d)
{
    return NULL;
}
S52_objHandle S52_setPosition (S52_objHandle objH, double latitude, double longitude, double heading)
{
    return NULL;
}
S52_objHandle S52_setVector   (S52_objHandle objH, double course, double speed)
{
    return NULL;
}
S52_objHandle  S52_iniVESSEL(int vescre, int vestat, const char *label)
{
    return NULL;
}
// (re) set label
S52_objHandle  S52_setVESSELlabel(S52_objHandle objH, const char *newLabel)
{
    return NULL;
}
S52_objHandle  S52_setVESSELstate(S52_objHandle objH, int vestat)
{
    return NULL;
}
S52_objHandle  S52_setVESSELselected(S52_objHandle objH, int selected)
{
    return NULL;
}





#define RateRadianToDegree	0.01745329252	// M_PI/180.0

// 计算纬度渐长率
double calLatGrowRatio(double lat)
{
        lat = lat * RateRadianToDegree;
        double e = 0.081813334;
        double temp1 = tan(M_PI/4+lat/2);
        double temp = e*sin(lat);//增加了一个临时变量，避免了重复计算
        double temp2 = (1-temp)/(1+temp);
        temp2 = pow(temp2, e/2);
        return 7915.704468*log10(temp1*temp2);
}

// 计算两点的相对弧度
double calRelativRadian(double lon1, double lat1, double lon2, double lat2)
{
    lon1 = lon1 * RateRadianToDegree;
    lat1 = lat1 * RateRadianToDegree;
    lon2 = lon2 * RateRadianToDegree;
    lat2 = lat2 * RateRadianToDegree;
    return acos(sin(lat1)*sin(lat2)+cos(lat1)*cos(lat2)*cos(lon2-lon1));
}

// 计算两点间的相对方位和距离，返回值单位：azi度，rng海里
void calRelativeAziRng(double lon1, double lat1, double lon2, double lat2, double &azi, double &rng)
{
        if(qAbs(lat1-lat2) < 1.0e-5)
        {	// 采用中分纬度算法
                if(qAbs(lon1-lon2) < 1.0e-5) azi = 0;
                else if(lon1 <= lon2) azi = 90;
                else azi = 270;
                rng = qAbs((lon2 - lon1) * cos((lat1+lat2)/2.0*RateRadianToDegree) * 60);
        }
        else
        {	// 采用墨卡托算法
                const double MP1 = calLatGrowRatio(lat1);
                const double MP2 = calLatGrowRatio(lat2);
                const double tanc = (lon2 - lon1) * 60.0 / (MP2-MP1);
                azi = atan(qAbs(tanc))/RateRadianToDegree;
                const bool xflag = lon2>lon1, yflag = lat2>lat1;
                if(xflag && yflag)
                        azi = azi;
                else if(xflag && !yflag)
                        azi = 180 - azi;
                else if(!xflag && !yflag)
                        azi = 180 + azi;
                else
                        azi = 360 - azi;
                rng = qAbs((lat2-lat1)/cos(azi*RateRadianToDegree)*60.0);
        }
}

// 计算两点的相对距离
double calRelativeRange(double lon1, double lat1, double lon2, double lat2)
{
        double azi, rng;
        calRelativeAziRng(lon1, lat1, lon2, lat2, azi, rng);
        return rng;
}

// 计算两点的相对方位(0-360度)
double calRelativeAzi(double lon1, double lat1, double lon2, double lat2)
{
        double azi, rng;
        calRelativeAziRng(lon1, lat1, lon2, lat2, azi, rng);
        return azi;
}

// 已知起始点的经纬度，航速、航向和航行时间，计算到达点的经纬度
// speed:节，course:度，time:秒
void calArrivePos(double startLon, double startLat, double speed, double course,
                  unsigned int time, double& arriveLon, double& arriveLat)
{
        const bool flag = (qAbs(course-90) < 1.0 || qAbs(course-270) < 1.0);
        course = course * RateRadianToDegree;
        double cosc = cos(course);
        arriveLat = startLat + speed*time*cos(course)/216000.0;//将原来的分母：3600.0改为216000.0
        if(flag)
        {	// 使用中分纬度算法
                const double dlon = speed*time*sin(course)/cos((startLat+arriveLat)*RateRadianToDegree/2)/216000.0;
                arriveLon = startLon + dlon;
        }
        else
        {	// 使用墨卡托算法
        const double MP1 = calLatGrowRatio(startLat);
        const double MP2 = calLatGrowRatio(arriveLat);
        const double dlon = (MP2-MP1)*tan(course)/60.0;
        arriveLon = startLon + dlon;
        }

        if(arriveLon < -180) arriveLon += 360;
        else if(arriveLon > 180) arriveLon -= 360;
}
/*********************************************
class: AisManage
function:  AIS对象的管理和显示/图形的绘制
author:
date:
**********************************************/

// 在指定位置画指定类型的符号
void drawSymbol(QPainter* painter, quint8 type, const QPoint& pt, quint8 ptsize);

AisManage::AisManage():/*m_pShipListDlg(NULL),m_pShipInfoDlg(NULL),*/m_color(Qt::yellow)
{
    m_selectedShipMMSI = 0;

    g_cLat = 29.16;
    g_cLon = 121.56;

    for(int q=0; q<MAXAISNUM_OVER; q++) {
        max[q] = 0;
        mmsi[q] = 0;
    }


    for(int i=0; i<10; i++)
        addSimuAis(i);


}

AisManage::~AisManage()
{
}

void AisManage::paint(QPainter* p)
{

// 需要使用符号大小(symbSize)，向量长度(veclen)，颜色方案(color)，
// AIS类别和状态
    QMutexLocker lock(&m_mutex);
    AIS_SHIP* ship = NULL;


    p->save();
    //限制显示区域
    if(!MenuConfig.dispMenu.offset) {
    QPainterPath clip;
    clip.addEllipse(QPointF(512,512), 500,500);
    p->setClipPath(clip);
    }


    //获取各个角度的显示半径长度，角度分割成了720个，所以需要乘以2
    extern const quint16 * CurrentRange();
    const quint16* lpCurrentRange = CurrentRange();
    const QPoint pt_00 = QPoint(0,0);
    const QPoint sc_center = QPoint(512, 512);  //图像中心位置
    const float rotation = pView->rotation();  //得到旋转角度，不是弧度


    QHash<unsigned int, AIS_SHIP>::iterator i = m_aisShips.begin(), ais_it_end = m_aisShips.end();
    for(; i != ais_it_end; ++i)
    {
        //遍历所有AIS对象
        ship = &(i.value());
        AIS_DYNAMIC_INFO& info = ship->dynamicInfo;
        ship->screenRect.setRect(0,0,0,0);

         // 如果动态信息无效则不画
         if((ship->flag & 0x02) != 0x02)
               continue;

        // 角度转化为弧度
        //const QPointF ld(info.lon*M_PI/180.0, info.lat*M_PI/180.0);
        //const QPoint pt = lpMainView->radarView()->latitude_to_screen(ld);
         const QPoint pt = ship->points.crntPoint.position.screen_point[0].toPoint();

        // 如果距离出界则不显示
         const int dispAzi = 720 + (ship->navigationInfo.azimuth * 2) - rotation * 2;
         const int dx = pt.x() - sc_center.x(), dy = pt.y() - sc_center.y();
         if(sqrt((float)dx*dx+dy*dy) >= lpCurrentRange[dispAzi%720])
               continue;
         //画笔尺寸由消息是否正常/符号大小选择有关
         const int ptsize = (ship->msgStatic == 0 ? 6 : 4) + MenuConfig.dispMenu.symbolSize * 2;
         QPen pen(ship->msgStatic == 0 ? m_color : QColor(0x88,0x88,0x88));  //目标暂时丢失变为灰色
         pen.setWidth(1);
         p->setPen(pen);

         QPoint pt1;
         int dispLength;
         if(MenuConfig.dispMenu.vecterLength < 5)
         {
            const int times[] = {1, 3, 6, 10, 15, 20};
            const int tm = times[MenuConfig.dispMenu.vecterLength];
            if(info.sog > 100)
                dispLength = 0;
            else
                dispLength = info.sog * (float)tm / 60.0 * pView->ration();  //得到绘制的矢量线的长度
         }
         else
         {	// 使用固定线长
            dispLength = 40;
         }
         const float course = (info.course - rotation) * M_PI / 180.0 ;   //弧度表示
         pt1 = pt + QPoint(dispLength * sin(course), -dispLength * cos(course));   //线段的另一个端点
         //qDebug() << pt << pt1 << info.course << rotation * 180.0 / M_PI;

        //qDebug() << ship->staticInfo.name << ld << pt << ld1 << pt1 << ptsize;
         ship->screenRect.setRect(pt.x()-ptsize,pt.y()-ptsize,2*ptsize,2*ptsize);  //得到图形的矩形大小


        // 画符号
        p->save();
        p->translate(pt);  //已经移动
        p->rotate(course*180.0/M_PI);
        //绘图/////////////////////////////////////////////
        drawSymbol(p, SYMBOL_TRIANGLE_UP, pt_00, ptsize);
        p->restore();

        // 画外部选择框
        const int boxIndex = m_lpDialog->getAisDispIndex(ship->staticInfo.MMSI);
        if(boxIndex > 0)
        {
            //绘图/////////////////////////////////////////////
            drawSymbol(p, SYMBOL_AIS_BORDER, pt, 2*ptsize);
            p->drawText(pt.x()+1.5*ptsize+6, pt.y()+ptsize, QString::number(boxIndex));
        }

        // 画船艏线
        p->drawLine(pt, pt1);

        // 4.显示尾点(从第二点开始)
        const quint8 displayFlag = 1;
        const AisPoints &points = ship->points;
        const int count = points.curPoints;
        QPoint lastPoint, crntPoint;
       // 显示小圆点
        {
            /*所有尾点使用同一颜色*/
            //painter->setBrush (QBrush(trackcolor));
            int index = points.curPointPos;
            for (int i=1; i<count; ++i)
            {
                if(points.points[index].position.displayFlag & displayFlag)
                {
                    //drawHistoryPoint (painter, track->points[index].position.screen_point[view].toPoint());
                    crntPoint = points.points[index].position.screen_point[0].toPoint();
                    //drawHistoryPoint(painter, crntPoint);
                    p->drawRect (QRect(crntPoint.x(), crntPoint.y(), 1, 1));
                }
                // 取前一个索引
                index = PrevTrackPointIndex(index);
            }
        }

    }

    p->restore();

}

// 删除所有AIS船显示
void AisManage::clearAisShips()
{
        QMutexLocker lock(&m_mutex);

        m_selectedShipMMSI = 0;
        m_aisStaticInfos.clear();
        m_aisDynamicInfos.clear();

        AIS_SHIP* ship = NULL;
        QHash<unsigned int, AIS_SHIP>::iterator i = m_aisShips.begin(), ais_it_end = m_aisShips.end();
        for(; i != ais_it_end; ++i)
        {
                ship = &(i.value());
                if(ship->vesselObj)
                        S52_delMarObj(ship->vesselObj);
        }
        m_aisShips.clear();



       // gUpdateMainViewFlag |= 0x01;
}

// 更新AIS船状态
void AisManage::updateAisShips(quint32 tm)
{
        QMutexLocker lock(&m_mutex);

        //处理静态信息/////////////////////////////////////////
        QHash<quint32, int>::const_iterator it1 = m_aisStaticInfos.constBegin(), it2 = m_aisStaticInfos.constEnd();
        for(; it1 != it2; ++it1)
        {
                const quint32 MMSI = it1.key();
                if(!m_aisShips.contains(MMSI))
                        continue;

                AIS_SHIP& aisShip = m_aisShips[MMSI];
                SHIP_STATIC_INFO& info = aisShip.staticInfo;
                if(!aisShip.vesselObj)
                {
                        aisShip.vesselObj = S52_iniVESSEL(2, 1, info.name);
                        aisShip.flag |= 0x08;
                }

                if(aisShip.msgStatic != 0)
                {
                        S52_setVESSELstate(aisShip.vesselObj, 1);
                        aisShip.msgStatic = 0;
                }

                S52_setDimension(aisShip.vesselObj, info.headRange, info.tailRange, info.leftRange, info.rightRange);
                S52_setVESSELlabel(aisShip.vesselObj, SystemInfo.showShipName ? info.name : " ");
        }
        m_aisStaticInfos.clear();

        //处理动态信息////////////////////////////////////////
        QHash<quint32, int>::const_iterator it3 = m_aisDynamicInfos.constBegin(), it4 = m_aisDynamicInfos.constEnd();
        for(; it3 != it4; ++it3)
        {
                const quint32 MMSI = it3.key();
                if(!m_aisShips.contains(MMSI))
                        continue;

                AIS_SHIP& aisShip = m_aisShips[MMSI];
                AIS_DYNAMIC_INFO& info = aisShip.dynamicInfo;

                // 计算AIS导航信息
                calAisNavigationInfo(&(aisShip));

                // 从睡眠态变为活动态
                if(aisShip.msgStatic != 0)
                {
                     aisShip.msgStatic = 0;
                }

                // CPA报警
                if(lpAlarm && (aisShip.flag & 0x04))
                    cpaAlarm(aisShip);


                // 更新历史点，将该点插入列表
                LATITUDE_POINT ld;
                ld.setPoint(info.lon * M_PI / 180, info.lat * M_PI / 180);
                enterPoint(&aisShip, ld);

                // 更新窗口中显示
                m_lpDialog->updateAis(MMSI, TgtBox::TGTBOX_UPDATE);

        }
        m_aisDynamicInfos.clear();

        //根据时间刷新AIS船舶列表的状态////////////////////////////////////////
        AIS_SHIP* ship = NULL;
        QHash<unsigned int, AIS_SHIP>::iterator i = m_aisShips.begin(), ais_it_end = m_aisShips.end();
        // 说明：由于i = m_aisShips.erase(i);已经将i移到了下一个位置，如果使用for循环，则++i会将i移向再下一位置
        // 实际上i加了两次，如果是对最后一个进行操作，则会出现异常
       //for(; i != m_aisShips.end(); ++i)
        while(i != ais_it_end)
        {
                const quint32 MMSI = i.key();
                ship = &(i.value());
                //AIS信号丢失
                if(ship->updateTime < tm - AIS_MISS_INTERVAL)
                {

                    m_lpDialog->updateAis(MMSI, TgtBox::TGTBOX_CLEAR);

                    if(MMSI == m_selectedShipMMSI)
                          m_selectedShipMMSI = 0;

                    S52_delMarObj(ship->vesselObj);
                    i = m_aisShips.erase(i);
                   // gUpdateMainViewFlag |= 0x01;
                    continue;
                }
                else if(ship->updateTime < tm - AIS_HALT_INTERVAL)
                {
                        ship->msgStatic = 1;
                        const QString msg = ship->staticInfo.name + QString("-miss");
                        S52_setVESSELstate(ship->vesselObj, 2);
                        //gUpdateMainViewFlag |= 0x01;
                        //S52_setVESSELlabel(ship->vesselObj, (const char*)(msg.toLocal8Bit().data()));
                }

                ++i;
        }

       //最大保存300个信息，根据距离调整,一次去掉所有超过的AIS数据
       if(m_aisShips.size() > MAXAISNUM) {
           quint16 length = m_aisShips.size() - MAXAISNUM;  //超过的个数
           if(length > MAXAISNUM_OVER)
               length = MAXAISNUM_OVER;
          // double max[length];  //局部定义感觉不安全
          // quint32 mmsi[length];
           quint32 num=0;
           for(int q=0; q<MAXAISNUM_OVER; q++) {
               max[q] = 0;
               mmsi[q] = 0;
           }

           i = m_aisShips.begin();

           while(i != ais_it_end)
           {
               const quint32 MMSI = i.key();
               ship = &(i.value());

               if(num < length) {
                   max[num] = ship->navigationInfo.range;
                   mmsi[num] = MMSI;
                   if(num == (length-1)) {
                       //对距离进行排序操作
                       int z,w;
                       bool change;
                       for(z=0; z<length; z++) {
                           change = false;
                           for(w=z+1; w<length; w++) {
                               if(max[z] > max[w]) {
                                   double t;
                                   quint32 mt;
                                   t = max[z];
                                   mt = mmsi[z];
                                   max[z] = max[w];
                                   mmsi[z] = mmsi[w];
                                   max[w] = t;
                                   mmsi[w] = mt;
                                   change = true;
                               }
                           }
                           if(!change)  //提前退出
                               break;
                       }

                   }

               }else {
                   double r = ship->navigationInfo.range;
                   if(r > max[0]) {
                       int n,m;
                       for(n=1; n<length;) {
                           if(r < max[n])
                               break;
                           else
                               n++;
                       }

                       for(m=0; m<n-1; m++){
                           max[m] = max[m+1];
                           mmsi[m] = mmsi[m+1];
                       }

                       max[m] = r;
                       mmsi[m] = MMSI;
                   }

               }

               ++num;
               ++i;
          }


           for(int p=0; p<length; p++) {
               m_aisShips.remove(mmsi[p]);  //去掉AIS信息
               m_lpDialog->updateAis(mmsi[p], TgtBox::TGTBOX_CLEAR);
           }

      }


}

// 更新所有的船名显示
void AisManage::updateShipNameDisplay()
{
        QMutexLocker lock(&m_mutex);

        QHash<unsigned int, AIS_SHIP>::iterator i = m_aisShips.begin(), ais_it_end = m_aisShips.end();
        for(; i != m_aisShips.end(); ++i)
        {
                AIS_SHIP& aisShip = i.value();
                if(aisShip.vesselObj)
                        S52_setVESSELlabel(aisShip.vesselObj, SystemInfo.showShipName ? aisShip.staticInfo.name : " ");
        }
}


void AisManage::initializeNewShip(unsigned int MMSI)
{

        AIS_SHIP* aisShip = &m_aisShips[MMSI];  //hash表中如果没有这样的项，则会自动插入一个默认项
        aisShip->aisClass = 0;// AIS类别  0:无效，1:A类，2:B类
        aisShip->flag = 0;		// bit0:静态信息有效，bit1:动态信息有效, bit2:导航信息有效,bit3:S52对象有效
        aisShip->msgStatic = 3;	// 消息状态 0:正常 1:中断 2:消失
        aisShip->vesselObj = 0;
        aisShip->updateTime = 0;

        aisShip->staticInfo.MMSI = 0;
        aisShip->staticInfo.IMO = 0;
        aisShip->staticInfo.version = 4;
        //memcpy(&aisShip->staticInfo.callCode, "----", 4);
        aisShip->staticInfo.callCode[0] = 0;
        //memcpy(&aisShip->staticInfo.name, "----", 4);
        aisShip->staticInfo.name[0] = 0;
        //memcpy(&aisShip->staticInfo.destination, "----", 4);
        aisShip->staticInfo.destination[0] = 0;
        aisShip->staticInfo.shipType = 0;
        aisShip->staticInfo.arriveTime = 0;//(QDateTime::fromTime_t(SystemInfo.crnt_time)).toTime_t();


        aisShip->dynamicInfo.course = 360.1;
        aisShip->dynamicInfo.dateTime = 0;
        aisShip->dynamicInfo.heading = 511;
        aisShip->dynamicInfo.lat = 91.0;
        aisShip->dynamicInfo.lon = 181.0;
        aisShip->dynamicInfo.naviStatus = 16;//UnUsedNs;
        aisShip->dynamicInfo.speed = 102.3;

        memset(&aisShip->navigationInfo, 0, sizeof(AIS_NAVIGATION_INFO));
}

// 添加或更新一条AIS船舶静态信息
void AisManage::addorUpdateAisShip(unsigned int MMSI, SHIP_STATIC_INFO* pShipStaticInfo, quint8 aisclass)
{
        QMutexLocker lock(&m_mutex);
        const bool newShipFlag = (!m_aisShips.contains(MMSI));
        if (newShipFlag)
        {
                initializeNewShip(MMSI);
        }

        if(pShipStaticInfo->shipType < 10)
                pShipStaticInfo->shipType = 10;
        else if(pShipStaticInfo->shipType > 99)
                pShipStaticInfo->shipType = 99;

        if(pShipStaticInfo->version > 4)
                pShipStaticInfo->version = 4;

    // 添加或更新静态信息
    //memcpy(&(m_aisShips[MMSI].staticInfo), pShipStaticInfo, sizeof(SHIP_STATIC_INFO));
        m_aisShips[MMSI].staticInfo = *pShipStaticInfo;

        // 更新时间
        m_aisShips[MMSI].updateTime = SystemInfo.crnt_time;//time(0);
        m_aisShips[MMSI].aisClass = aisclass;
        m_aisShips[MMSI].flag |= 0x01;
        //m_aisShips[MMSI].msgStatic = 0;
      // 创建对应的S52对象
        if(newShipFlag)
                m_aisShips[MMSI].vesselObj = 0;//S52_iniVESSEL(2, 1, pShipStaticInfo->name);

        m_aisStaticInfos.insert(MMSI, 0);

}

// 更新AIS船舶动态信息
void AisManage::updateAisDynamicInfo(unsigned int MMSI, AIS_DYNAMIC_INFO* pAisDynamicInfo, quint8 aisclass)
{
        QMutexLocker lock(&m_mutex);
        const bool newShipFlag = (!m_aisShips.contains(MMSI));
        if(newShipFlag)
                initializeNewShip(MMSI);

        if(pAisDynamicInfo->naviStatus > 15)
                pAisDynamicInfo->naviStatus = 15;

        // 更新AIS动态信息
        memcpy(&(m_aisShips[MMSI].dynamicInfo), pAisDynamicInfo, sizeof(AIS_DYNAMIC_INFO));

        // 更新时间
        m_aisShips[MMSI].updateTime = SystemInfo.crnt_time;//time(0);
        m_aisShips[MMSI].aisClass = aisclass;
        m_aisShips[MMSI].flag |= 0x02;
        //m_aisShips[MMSI].msgStatic = 0;

        if(newShipFlag)
        {
                m_aisShips[MMSI].staticInfo.MMSI = MMSI;
                m_aisStaticInfos.insert(MMSI, 0);
        }
        m_aisDynamicInfos.insert(MMSI, 0);



    // 计算AIS导航信息
    //calAisNavigationInfo(&(m_aisShips[MMSI]));

}

// 获取AIS船舶信息
AIS_SHIP* AisManage::getAisShip(unsigned int MMSI, bool lock)
{
        //QMutexLocker lock(&m_mutex);
    if(lock)
        m_mutex.lock();

    QHash<unsigned int, AIS_SHIP>::iterator i = m_aisShips.find(MMSI);
    if (i != m_aisShips.end() && i.key() == MMSI) {
        return &(i.value());
    }

    return NULL;
}

// 判断MMSI号是否有效
bool AisManage::isShipValid(quint32 MMSI)
{
        QMutexLocker lock(&m_mutex);
        return m_aisShips.contains(MMSI);
}

// 本船位置改变
void AisManage::ownshipPositionChanged()
{
    QMutexLocker lock(&m_mutex);

    AIS_SHIP* ship = NULL;
    QHash<unsigned int, AIS_SHIP>::iterator i = m_aisShips.begin();
    for(; i != m_aisShips.end(); ++i)
    {
         ship = &(i.value());
          // 计算AIS导航信息
         calAisNavigationInfo(ship);
          // CPA报警
         cpaAlarm(*ship);
    }
}

// 计算AIS导航信息
void AisManage::calAisNavigationInfo(AIS_SHIP* pAisShip)
{
    if (EQUAL3(pAisShip->dynamicInfo.lon, 181.0, 0.01) || EQUAL3(pAisShip->dynamicInfo.lat, 91.0, 0.01))
    {//位置信息无效
        pAisShip->flag &= (~0x04);
           return;
    }

    const double degree2radian = M_PI/180.0;
    // 计算相对本船方位、距离
    double azi, rng;
    calRelativeAziRng(g_ownshpLon, g_ownshpLat, pAisShip->dynamicInfo.lon, pAisShip->dynamicInfo.lat, azi, rng);
    pAisShip->navigationInfo.range = rng;
    pAisShip->navigationInfo.azimuth = azi;
        // 将方位从角度转化为弧度
    const double dazi = pAisShip->navigationInfo.azimuth*degree2radian;

        /* 计算DCPA和TCPA */
    AIS_DYNAMIC_INFO& aisInfo = pAisShip->dynamicInfo;
    AIS_DYNAMIC_INFO ownInfo;
    m_boatAlarm->dynamicInfo(ownInfo);

        if((IsDataValid0(ownInfo.sog,0,100) && IsAzimuthValid(ownInfo.course)) ||
           (IsDataValid0(aisInfo.sog,0,100) && IsAzimuthValid(aisInfo.course)) )
        {
                //qDebug() << "cal navigate info:" << ownInfo.speed << ownInfo.course << aisInfo.speed << aisInfo.course;
                // 本船和他船速度航向,从AIS数据得到只有对地航向和速度,根据选择计算AIS 数据的本船航向和速度
                double V0 = ownInfo.sog, C0 = ownInfo.course*degree2radian;;
                if(MenuConfig.otherMenu.corseSelect != 1){   //船艏向
                    C0 = ownInfo.heading*degree2radian;
                }
                if(MenuConfig.otherMenu.speedSelect != 1) {  //对水速度
                    V0 = ownInfo.speed;
                }
                const double Vt = aisInfo.sog, Ct = aisInfo.course*degree2radian;
                const double sinc0 = sin(C0), cosc0 = cos(C0), sinct = sin(Ct), cosct = cos(Ct);
                const double Vrx = Vt*sinct - V0*sinc0, Vry = Vt*cosct - V0*cosc0;  //計算相對速度
                // 计算相对运动矢量的角度
                const double Cr = atan2(Vrx, Vry);
                const double Vr = sqrt(Vrx*Vrx + Vry*Vry);
                //qDebug() << pAisShip->staticInfo.MMSI << Vr << Cr * 360 / M_2PI;
                if(Vr >= 0.01){
                double theta = Cr - dazi - M_PI;
                pAisShip->navigationInfo.DCPA = qAbs(pAisShip->navigationInfo.range * sin(theta));
                pAisShip->navigationInfo.TCPA = qAbs(pAisShip->navigationInfo.range * cos(theta)/Vr) * 60 + 0.5;
                }
                else{
                        pAisShip->navigationInfo.DCPA = 1.0e10;
                        pAisShip->navigationInfo.TCPA = 0xffffffff;
                }
        }
        else
        {
        pAisShip->navigationInfo.DCPA = 1.0e10;
        pAisShip->navigationInfo.TCPA = 0xffffffff;
        }

        pAisShip->flag |= 0x04;
}

#define LESS(a,b) (a<b)
#define RngBetween(r, rmin, rmax) ((r) >= (rmin) && (r) <= (rmax))
#define AziBetween(a, amin, amax) (LESS(amin, amax) ? RngBetween(a, amin, amax) : ((a) >= (amin) || (a) <= (amax)))
#define IsInGuardZone(r, a) ( RngBetween(r, MenuConfig.otherMenu.guardZoneValue[0], MenuConfig.otherMenu.guardZoneValue[2]) && \
        AziBetween(a, MenuConfig.otherMenu.guardZoneValue[1], MenuConfig.otherMenu.guardZoneValue[3]) )
// CPA报警
void AisManage::cpaAlarm(const AIS_SHIP& ship)
{
        //if((ship.flag & 0x04) != 0x04)
        //	return;

        const AIS_NAVIGATION_INFO& info = ship.navigationInfo;

        if(MenuConfig.otherMenu.dcpaEnable && IsDataValid0(info.DCPA, 0, 100) && info.DCPA <= MenuConfig.otherMenu.dcpaValue)
        {
                const QString msg = QString("DCPA目标报警");
                lpAlarm->startAlarm((ALARMLEVEL)g_systemPara.alarmPara.cpaAlarmLevel,msg);
        }
        else if(MenuConfig.otherMenu.tcpaEnable && IsDataValid0(info.TCPA, 0, 100) && info.TCPA <= MenuConfig.otherMenu.tcpaValue)
        {
                const QString msg = QString("TCPA目标报警");
                lpAlarm->startAlarm((ALARMLEVEL)g_systemPara.alarmPara.cpaAlarmLevel,msg);
        }
        else if(MenuConfig.otherMenu.guardZoneEnable && IsInGuardZone(info.range, info.azimuth))
        {
                const QString msg = QString("警戒区目标报警");
                lpAlarm->startAlarm((ALARMLEVEL)g_systemPara.alarmPara.cpaAlarmLevel,msg);
        }
        else if(MenuConfig.otherMenu.cpaEnable && info.range <= MenuConfig.otherMenu.cpaValue)
        {
                const QString msg = QString("警戒圈目标报警");
                lpAlarm->startAlarm((ALARMLEVEL)g_systemPara.alarmPara.cpaAlarmLevel,msg);
        }
}

void AisManage::cpaAlarm()
{
        QMutexLocker lock(&m_mutex);

        AIS_SHIP* pShip;
        QHash<unsigned int, AIS_SHIP>::const_iterator it=m_aisShips.constBegin(), it2 = m_aisShips.constEnd();
        for(; it != it2; ++it)
        {
                pShip = (AIS_SHIP*)&(it.value());            
                cpaAlarm(*pShip);
        }
}

// 获取航迹尾点间隔时间
extern int GetTrackIntervalTime();
void AisManage::enterPoint(AIS_SHIP* lpShip, const LATITUDE_POINT& ld)
{
    //插入历史点迹
    const int crnttime = time(0);

    AisPoints & points = lpShip->points;
    TRACKPOINT& trackpoint = points.crntPoint;//track.points[track.curPointPos];
    trackpoint.updateTime = crnttime;


    // 更新当前点位置
    PLOTPOSITION& position = trackpoint.position;
    position.updateFlag = 0;
    //调用转换函数转为直角坐标
    position.square_point = lpMouseOpetarion->latitude_to_square(ld);
    position.latitude_point = ld;
    //不转换为极坐标，没什么用
    position.rtheta_point = lpMouseOpetarion->square_to_rtheta(position.square_point);

     //屏幕坐标/////////////////////////////////////////////////////////////////
     position.screen_point[0] = lpMouseOpetarion->square_to_screen(position.square_point);

    if(pView->isPointDisplay(position))
        position.displayFlag |= 1;


}

void AisManage::updateScreenPoint()
{
    QMutexLocker lock(&m_mutex);
    AIS_SHIP* pShip;

    const quint8 displayflag = 1;
    QHash<unsigned int, AIS_SHIP>::iterator i = m_aisShips.begin();
    for (; i != m_aisShips.end(); ++i)
    {
        pShip = &(i.value());

        // 判断动态信息是否有效
        if(!(pShip->flag & 0x02))
             continue;

        const int count = pShip->points.curPoints;
        int pos = pShip->points.curPointPos;  //当前位置索引
        for (int cnt=0; cnt<count; cnt++, pos = PrevTrackPointIndex(pos))
        {
            PLOTPOSITION& position = pShip->points.points[pos].position;
            position.displayFlag = 0;
            {
                //得到屏幕坐标//////////////////////////////////////////////////////////////////
                position.screen_point[0] = lpMouseOpetarion->square_to_screen(position.square_point);

                if(pView->isPointDisplay(position))
                    position.displayFlag |= displayflag;
            }
        }

        //画最新点
        PLOTPOSITION& position = pShip->points.crntPoint.position;
        position.displayFlag = 0;
        {
            //得到屏幕坐标//////////////////////////////////////////////////////////////////
            position.screen_point[0] = lpMouseOpetarion->square_to_screen(position.square_point);

            if(pView->isPointDisplay(position))
                position.displayFlag |= displayflag;
        }
        }
}

// 判断某位置点是否选中AIS船舶
quint32 AisManage::ptOnAisShip(const QPoint& pt)
{
    QMutexLocker lock(&m_mutex);

    AIS_SHIP* pShip;
    QHash<unsigned int, AIS_SHIP>::iterator i = m_aisShips.begin();
    for (; i != m_aisShips.end(); ++i)
    {
        pShip = &(i.value());

        // 判断动态信息是否有效
       if(!(pShip->flag & 0x02))
            continue;

       if(pShip->screenRect.contains(pt))
           return i.key();
    }
    return 0;
}

// 设置当前选择船泊
void AisManage::setSelectedShip(unsigned int MMSI)
{
    if(m_selectedShipMMSI == MMSI)
          return;

    QMutexLocker lock(&m_mutex);

    if(!m_aisShips.contains(MMSI))
    {
        if(m_aisShips.contains(m_selectedShipMMSI))
        S52_setVESSELselected(m_aisShips[m_selectedShipMMSI].vesselObj, 0);
        m_selectedShipMMSI = 0;
    }
    else
    {
        const AIS_SHIP ship = m_aisShips.value(MMSI);
        const AIS_SHIP* pShip = &ship;
        if(m_aisShips.contains(m_selectedShipMMSI))
        S52_setVESSELselected(m_aisShips[m_selectedShipMMSI].vesselObj, 0);
        S52_setVESSELselected(pShip->vesselObj, 1);
        m_selectedShipMMSI = MMSI;
    }
}

// 添加模拟AIS船舶
void AisManage::addSimuAis(int flag)
{
    // 添加AIS船舶信息
    SHIP_STATIC_INFO shipStaticInfo;
    memset(&shipStaticInfo, 0, sizeof(SHIP_STATIC_INFO));
    shipStaticInfo.MMSI = 412419440 + flag;
    shipStaticInfo.IMO = 1234;
    strcpy(shipStaticInfo.callCode, "4321");
    strcpy(shipStaticInfo.name, "JIE YUN");
    strcpy(shipStaticInfo.destination, "WEI HAI");
    shipStaticInfo.maxDraught = 3.0;
    shipStaticInfo.headRange = 6;
    shipStaticInfo.tailRange = 2.5;
    shipStaticInfo.leftRange = 1.2;
    shipStaticInfo.rightRange = 1.8;
    shipStaticInfo.arriveTime = QDateTime::currentDateTime().addDays(1).toTime_t();
    addorUpdateAisShip(shipStaticInfo.MMSI, &shipStaticInfo, 1);

    AIS_DYNAMIC_INFO aisDynamicInfo;
    aisDynamicInfo.lon = 121.57 + flag*0.01;
    aisDynamicInfo.lat = 29.1 + flag*0.01;
    aisDynamicInfo.course = 0.0;
    aisDynamicInfo.speed = 0.1;
    updateAisDynamicInfo(shipStaticInfo.MMSI, &aisDynamicInfo, 1);
}


// 模拟AIS船舶运动
void AisManage::simuAisMove(unsigned int interval)
{
    foreach(AIS_SHIP ship, m_aisShips)
    {
        AIS_DYNAMIC_INFO aisDynamicInfo;
        aisDynamicInfo.course = 55.0 + rand()%50/10.0;   //船艏向跟航向相等
        aisDynamicInfo.heading = aisDynamicInfo.course;
        aisDynamicInfo.naviStatus = 0;
        if(ship.dynamicInfo.speed<0.00001)
        {
            aisDynamicInfo.speed = 10.0 + rand()%50/10.0;
            aisDynamicInfo.lon = g_cLon - 0.1;
            aisDynamicInfo.lat = g_cLat - 0.05;
        }
        else
        {
            aisDynamicInfo.speed = 10.0 + rand()%50/10.0;
            aisDynamicInfo.lon = ship.dynamicInfo.lon + aisDynamicInfo.speed*sin(aisDynamicInfo.course*M_PI/180.0)/60.0/3600.0*interval/1000.0;
            aisDynamicInfo.lat = ship.dynamicInfo.lat + aisDynamicInfo.speed*cos(aisDynamicInfo.course*M_PI/180.0)/60.0/3600.0*interval/1000.0;
        }
        updateAisDynamicInfo(ship.staticInfo.MMSI, &aisDynamicInfo, 1);
    }
}

int GetAtaDispIndex(int index)
{
    return m_lpDialog->getAtaDispIndex(index+1);
}

int GetAtaSymbolSize()
{
    return 10 + MenuConfig.dispMenu.symbolSize * 2;
}

// 获取本船的速度航向
bool GetOwnShipCourseSpeed(float* course, float* speed)
{
    AIS_DYNAMIC_INFO ownInfo;
    m_boatAlarm->dynamicInfo(ownInfo);
    *course = ownInfo.course;
    *speed = ownInfo.sog;

    return (IsDataValid0(ownInfo.sog,0,100) && IsAzimuthValid(ownInfo.course));
}

// 获取cpa/tcpa参数
quint8 GetCpaTcpaParam(float *cpa, float *dcpa, float *tcpa)
{
    quint8 flag = 0;

    if(MenuConfig.otherMenu.cpaEnable) flag  |= 0x01;
    if(MenuConfig.otherMenu.dcpaEnable) flag |= 0x02;
    if(MenuConfig.otherMenu.tcpaEnable) flag |= 0x04;

    *cpa  = MenuConfig.otherMenu.cpaValue;
    *dcpa = MenuConfig.otherMenu.dcpaValue;
    *tcpa = MenuConfig.otherMenu.tcpaValue;

    return flag;
}

// 获取警报区参数
quint8 GetGuardZoneParam(float *zone)
{
    memcpy(zone, MenuConfig.otherMenu.guardZoneValue, 4*sizeof(float));
    return MenuConfig.otherMenu.guardZoneEnable;
}

bool GuardZoneAlarm(float azi, float rng)
{
    return (MenuConfig.otherMenu.guardZoneEnable && IsInGuardZone(rng, azi));
}

void StartAlarm(int level, const QString& msg)
{
        lpAlarm->startAlarm((ALARMLEVEL)level, msg);
}






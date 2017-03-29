/****************************************************************************
file name: openglview.cpp
author: wang ming xiao
date: 2015/07/25
comments: 回波绘图的主要函数，包括回波/刻度线等等
***************************************************************************/
#include "openglview.h"
#include "TargetManage/transform.h"
#include "TargetManage/TargetManage_global.h"
#include "boatinfo.h"
#include "interact.h"
#include "define.h"
#include "mainwindow.h"
#include "mouseoperation.h"
#include "dialog.h"
#include "aismanage.h"
#include "boatalarm.h"


#include <QtOpenGL>
#include <math.h>


void DrawFanPath (QPainter* p, const RTHETA_POINT& pt1, const RTHETA_POINT& pt2);

//double m_EachDegreeSin[MAXDISPAZI], m_EachDegreeCos[MAXDISPAZI];  //回波绘制使用
double EachDegreeSin2[360], EachDegreeCos2[360];  //外围刻度线绘制使用

quint8 oncetime = 0;  //面板鼠标操作
quint8 guardZone_alarm=0;  //报警标志
quint8 waitFirstAngleData = 1;   //等待绘图第一个角度标志
extern quint8 firstStartFlag; //启动标志

extern COLORCONFIG *lpColorConfig;
extern MENUCONFIG MenuConfig;
extern Interact *lpInteract;
extern SYSTEMINFO SystemInfo;
extern MainWindow *lpMainWindow;
extern MouseOperation *lpMouseOpetarion;
extern quint8 FirstWaitTime;
extern Dialog* m_lpDialog;
extern AisManage* lpAisManage;
extern boatalarm* m_boatAlarm;
extern Alarm* lpAlarm;
extern SYSTEM_PARA g_systemPara;

//颜色转换位16位的，按5-6-5的顺序,8位为332的顺序
#ifdef OPENGL_RGBA
    #define ColorConvert(r, g, b)    ((quint32)255 | ((quint32)b << 8) | ((quint32)g << 16) | ((quint32)r << 24))
#endif
#ifdef OPENGL_SHORT
    #define ColorConvert(r, g, b)    ((quint16)(b >> 3) | ((quint16)(g >> 2) << 5) | ((quint16)(r >> 3) << 11))
#endif
#ifdef OPENGL_BYTE
     #define ColorConvert(r, g, b)    ((b >> 6) | ((g >> 5) << 2) | ((r >> 5) << 5))
#endif
OpenGLView::OpenGLView(QWidget *parent) : QGraphicsView(parent)
{
    m_dashes << 10 << 10;
    //回波颜色默认黄色
    echoColor = ColorConvert(255, 255, 0);
    //杂波颜色 4e764e
    echoclutterColor = ColorConvert(0x28, 0x2b, 0x28);
    //多色中的第二种颜色
    echoColor2 = ColorConvert(0xb6, 0xe3, 0x41);
    //多色中的第三种颜色
    echoColor3 = ColorConvert(0xca, 0x9d, 0x79);   //0xca,9d,79
    //尾迹颜色  蓝色
    echotrailColor  = ColorConvert(0, 0, 255);   // RGB 003

    //设置颜色显示模式
    if((MenuConfig.dispMenu.colorSelect == 0) || (MenuConfig.dispMenu.colorSelect == 1))
        setColorMode(0);
    else if(MenuConfig.dispMenu.colorSelect == 2)
        setColorMode(1);

    flag_newCircle = 0;
    flag_next_Circle = 0;
    //警戒区域标志设置
    gzaBgn = 0;
    gzaEnd = 0;
    gzrMix = 0;
    gzrMax = 0;
    flag_guardZone = 0;
    flag_paintBack =0;
    flag_paintEcho = 0;
    //调节标志设置
    azi_adjust = 0;
    rng_adjust = 0;
    m_rngStart = 0;
    //尾迹标志
    m_keepTime = 0;
    m_keepTime_5s = 0;
    //鼠标标志
    mouseShapeChanged = 0;
    theta_offset = QPointF(0.0, 0.0);
/**********************视窗设置部分*********************************/
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  //设置禁止滚动条
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::Antialiasing, false);  //抗锯齿设置
    setOptimizationFlags(QGraphicsView::DontClipPainter);
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);   //设置画面刷新区域最小
  //  setCacheMode(QGraphicsView::CacheBackground);  //提高刷新速度
   // setAutoFillBackground(false);
    setFixedSize(MAINVIEW_RADIUS, MAINVIEW_RADIUS);  //设置视图大小
    setFrameStyle(QFrame::NoFrame);

/*******************数据处理设置******************************************/
    m_paintNum = 1;

    m_startPacketNum = 0;
    m_endPacketNum = 0;

    //清空缓存区
    for(int i=0; i< MAXPACKETNUM; i++) {
        memset(&m_echoDataList[i], 0, sizeof(ECHODATA));
    }

    radius = 0;
    x = y = 0;
    echoExpandMode = 0;
    m_gain_real = 0;

/*******************绘图相关设置****************************************/
    //刷新信号槽连接，在另外一个线程中不能直接调用repaint函数
    connect(this,SIGNAL(painting()),this, SLOT(receivedpaint()));

    //设定默认背景色
    const quint8 bc_r= (quint8)((lpColorConfig->color_back[MenuConfig.dispMenu.colorSelect]>>16) & 0x000000ff);
    const quint8 bc_g = (quint8)((lpColorConfig->color_back[MenuConfig.dispMenu.colorSelect]>>8) & 0x000000ff);
    const quint8 bc_b = (quint8)(lpColorConfig->color_back[MenuConfig.dispMenu.colorSelect] & 0x000000ff);
    backColor = ColorConvert(bc_r, bc_g, bc_b);
    //回波背景色设置
    echobackColor = ColorConvert(0x34, 0, 0);  //RGB  100 ,332格式

    m_headingColor.setRgb(lpColorConfig->color_headl[MenuConfig.dispMenu.colorSelect]);
    m_varlineColor.setRgb(lpColorConfig->color_eblvrm[MenuConfig.dispMenu.colorSelect]);

    //设置回波背景索引
    for(int i=0; i<echoViewWidth;i++){
        for(int j=0;j<echoViewHeight;j++) {
            if(sqrt((double)((i-512)*(i-512) + (j-512)*(j-512))) < ECHOVIEW_RADIUS) {
                QPoint e(i,j);
                echoviewCircle.append(e);
            }else {
                QPoint e(i,j);
                echoviewOutCircle.append(e);
            }

        }
     }


    initEchoView();  //初始化内存显示部分

    //初始化尾迹时间数组
    timeDelay = new quint8[RADIUS3 * RADIUS3];
    memset(timeDelay, 0, RADIUS3*RADIUS3);

    //外围方位圈使用
    for(int i=0; i<360; i++)
    {
        const double radian = DEGREETORADIAN(i);
        EachDegreeSin2[i] = SIN(radian);
        EachDegreeCos2[i] = COS(radian);
    }

    m_screenIndex = new qint32[DIAMETER*DIAMETER];

    //将显示格中的每个像素点极坐标转换成直角坐标 并利用直角坐标计算出对应的索引值，
    for(quint16 x = 0; x < DIAMETER; x++)
        {
        for(quint16 y = 0; y < DIAMETER; y++)
                {
                        int x_offset = x - RADIUS3;
                        int y_offset = y - RADIUS3;
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

                    const int idx_scrn = (scrn_valid ? (y-RADIUS)*1024+(x-RADIUS) : -1);

                    m_screenIndex[idx_virt] = idx_scrn;

            }
    }


    //设置显示模式和距离方位刻度值,量程
    setScanMode(NORMAL_SCAN);   
    setCurrentAzimuthScale(0); //不绘方位刻度线
    setRange(SystemInfo.RangeScale.range(), SystemInfo.RangeScale.scale());

    m_gain = 60;
    m_clutter = 0;
    m_restrain = 0;
    m_tune = 0;
    radiusToDisp = ECHOVIEW_RADIUS / ECHODATA_LENGTH;  //主要用于回波显示
    m_mouseTracking = false;
    m_offset_x = 0;
    m_offset_y = 0;
    m_offset_maxradius = 500;
    m_offsetPoint = QPoint(512, 512);
    CurrentRange = new quint16[MAXREALAZI2];  //360个角度

    setCurrentRange();
    flag_break = 0;
    m_offset_index = 0;

    //创建导航对象
    m_lpNaviPoint = new NaviPoint;
    //创建警戒区域对象
    m_lpGuardZone = new GuardZone;


    connect(lpMainWindow, SIGNAL(mousePressed(QMouseEvent*)), this, SLOT(updateMouseOperate(QMouseEvent*)));
    //设置选装角度
    setAziAdjust(MenuConfig.installMenu.aziAdjust);

}

OpenGLView::~OpenGLView()
{
    if(CurrentRange)
        delete[] CurrentRange;
    if(timeDelay)
        delete[] timeDelay;  
    if(m_screenIndex)
        delete[] m_screenIndex;

}

void OpenGLView::updateViewRotation()
{
    //船艏向上：回波角度不变，刻度盘变化
    //真北向上：回波角度变化，刻度盘不变
    //航向向上：回波角度旋转，刻度盘选装，都是航向角度
    if(SystemInfo.ViewDisplay.upmode == C_UP) {
        float azi = SystemInfo.ShipInfo.head - SystemInfo.ShipInfo.course;
        if(azi < 0)  azi += 360.0;
        //回波显示旋转差值
        setRotation(azi);
        //外圈刻度盘根据航向旋转
        setRotation2(-SystemInfo.ShipInfo.course);

    }else if(SystemInfo.ViewDisplay.upmode == H_UP) {  //船艏线向上，刻度盘以船艏方向旋转
        setRotation(0.0);
        setRotation2(-SystemInfo.ShipInfo.head);
    }else if(SystemInfo.ViewDisplay.upmode == N_UP){
        setRotation(SystemInfo.ShipInfo.head);
        setRotation2(0.0);  //刻度盘
    }

    //设置警戒区域
    if(MenuConfig.otherMenu.guardZoneEnable) {
        setGuardZoneClear();
        //setGuardZoneText();
        m_lpGuardZone->updateScreenPoint();
        setGuardZonePixel();  //重新计算警戒区域
    }
    screenCoordinateChanged();
}
void OpenGLView::setUpMode(quint8 mode, bool forceupdateflag)
{
    if(forceupdateflag || SystemInfo.ViewDisplay.upmode != mode)
    {
        SystemInfo.ViewDisplay.upmode = mode;
        /*设置视图屏幕坐标旋转角度*/
        updateViewRotation ();

        initEchoView();  //清空屏幕
        //qDebug()<< "change the upmode"<< SystemInfo.ViewDisplay.upmode<<mode;

        /* 更新状态栏显示 */
        lpMainWindow->updateCtrlPanel(VIEW_UPDATE_UP_BIT);
        /*更新显示*/
        viewport()->update();
        /* 更新工具栏显示 */
        //g_lpDialog->updateUpmode();
    }
}

// 运动方式
void OpenGLView::setMotion(quint8 motion)
{
    if(SystemInfo.ViewDisplay.moving != motion)
    {
        SystemInfo.ViewDisplay.moving = motion;
        if(SystemInfo.ViewDisplay.moving == REALMOVING) { //真运动方式
              double lon, lat;
              if(NULL== m_boatAlarm) {
                   return;
              }
              m_boatAlarm->ownshpPosition(lat, lon);//获取本船的位置信息
              LATITUDE_POINT ld;
              ld.setPoint(lon, lat);
              SCREEN_POINT sd = lpMouseOpetarion->latitude_to_screen(ld);//地理坐标转换为屏幕坐标
              //真运动从中心开始
              const QPoint sc = rect().center();
              setOffsetPoint(sc.x(), sc.y());

        }
        else if(SystemInfo.ViewDisplay.moving == RELATIVEMOVING){//相对运动方式
              const QPoint sc = rect().center();
              setOffsetPoint(sc.x(), sc.y());

        }
        /*设置视图屏幕坐标旋转角度*/
        updateViewRotation ();

        /*更新模式改变后，需要重新计算目标在屏幕上的位置点*/
         //screenCoordinateChanged();
        /* 更新状态栏显示 */
        lpMainWindow->updateCtrlPanel(VIEW_UPDATE_TMRM_BIT);
        //更改偏心显示
        if(lpMainWindow)
            lpMainWindow->setOffsetMode(false);

        /*更新显示*/
        initEchoView();
        viewport()->update(rect());
    }
}

void OpenGLView::setCurrentRange()
{
    //计算当前中心到圆周的距离
    //根据角度和当前的位置，可以写出直线方程
    //先计算出当前中心相对原中心的坐标
    const int MaxAzimuth = MAXREALAZI2;
    const int MaxDispRadius = ECHOVIEW_RADIUS;
    //没有偏心时的默认值
    if(m_offset_x == 0 && m_offset_y == 0 )
    {
        for(int i = 0; i < MaxAzimuth; i++){
           CurrentRange[i] = MaxDispRadius;
        }
        m_offset_maxradius = 500;
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

  //  qDebug() << "===============:" << offset_x << offset_y;
  //  for(int i=0; i<MaxAzimuth; i+= 5)
   //     qDebug() << i/5+1 << CurrentRange[i] << CurrentRange[i+1] << CurrentRange[i+2] << CurrentRange[i+3] << CurrentRange[i+4];


}

bool OpenGLView::isPointInOffArea(const QPoint &p) const
{
    const int r = ECHOVIEW_RADIUS;
    const QPoint sc = QPoint(521, 521);
    const int dx = sc.x() - p.x(), dy = sc.y() - p.y();
    return sqrt((double)(dx*dx + dy*dy)) < r * 3/4;
}
//偏心设置
void OpenGLView::setOffset(quint8 ofst)
{
    QPoint pos;
    if(SystemInfo.ViewDisplay.offset != ofst) {
        SystemInfo.ViewDisplay.offset = ofst;
        //偏心操作
        if(ofst == 1) {
             pos = lpMouseOpetarion->mouseScreenPoint();

             //得到偏转的角度信息
             theta_offset = lpMouseOpetarion->mouseRThetaPoint();
            if(! isPointInOffArea(pos)) {  //超出显示范围半径的3/4，不再外移
                const double dx = pos.x() - 512;
                const double dy = pos.y() - 512;
                int deg = RADIANTODEGREE(atan2(dy, dx));  //atan2计算与x轴正方向之间的角度
                if(deg < 0)
                    deg += 360;
                pos = QPoint((int)(375*EachDegreeCos2[deg]) + 512, 521 + (int)(375*EachDegreeSin2[deg]));
            }

            setOffsetPoint(pos.x(), pos.y());
           // m_offsetPoint = pos;

          //  qDebug()<<"offset point is"<<pos.x()<<pos.y();

            initEchoView();  //重新显示回波，数据都变了

            //设置警戒区域
            if(MenuConfig.otherMenu.guardZoneEnable) {
                setGuardZoneClear();
                //setGuardZoneText();
                m_lpGuardZone->updateScreenPoint();
                setGuardZonePixel();  //重新计算警戒区域
            }
            screenCoordinateChanged();

            if(! MenuConfig.otherMenu.transmite)
                viewport()->update();




            //向FPGA发送命令
            QByteArray senddata;
            QDataStream in(&senddata, QIODevice::WriteOnly);
            if(lpInteract) {
                in<<(quint8)170<<(quint8)0<<(quint8)8<<(quint8)0<<(quint8)1;  //AA0008 0001 --- AA0008 0000   AM  NM
                if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
                   // qDebug()<< "FPGA write offset open success!";
            }      

        }else {
            pos = QPoint(512, 512);
            m_offsetPoint = pos;
            lpMouseOpetarion->setCrenterScreenPoint(pos);
            m_offset_x = 0;
            m_offset_y = 0;
            setCurrentRange();  //恢复各角度的量程
            m_offset_index = 0;

            //更改VRM的数值，避免绘制的图形超出范围
            SystemInfo.VrmEblCtrl.vrmebl[0].vrm = m_range / 3.0;
            SystemInfo.VrmEblCtrl.vrmebl[1].vrm = m_range / 2.0;

         //   qDebug()<<"center is"<<m_offsetPoint.x()<<m_offsetPoint.y();

            initEchoView();  //重新显示回波

            //设置警戒区域
            if(MenuConfig.otherMenu.guardZoneEnable) {
                setGuardZoneClear();
                //setGuardZoneText();
                m_lpGuardZone->updateScreenPoint();
                setGuardZonePixel();  //重新计算警戒区域
            }
            screenCoordinateChanged();
            if(! MenuConfig.otherMenu.transmite)
                viewport()->update();


            //向FPGA发送命令
            QByteArray senddata;
            QDataStream in(&senddata, QIODevice::WriteOnly);
            if(lpInteract) {
                in<<(quint8)170<<(quint8)0<<(quint8)8<<(quint8)0<<(quint8)0;
                if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
                   // qDebug()<< "FPGA write offset close success!";
            }
        }


    }



}
void OpenGLView::setOffsetPoint(quint16 x, quint16 y)
{
    const int offset_x = x - RADIUS + 1;
    const int offset_y = y - RADIUS + 1;
    if(offset_x == m_offset_x && offset_y == m_offset_y)
        return;

    //QMutexLocker locker(&m_echoMutex);

    m_offset_x = offset_x;
    m_offset_y = -offset_y;

    setCurrentRange();
    m_offsetPoint = QPoint(x+1, y+1);
    lpMouseOpetarion->setCrenterScreenPoint(QPoint(m_offsetPoint.x(), m_offsetPoint.y()));

    m_offset_index = m_offset_y*DIAMETER + m_offset_x;  //虚拟视图偏心索引


}

void OpenGLView::setRange(float rng, float sca)
{


    m_range = rng;
    m_ration = ECHOVIEW_RADIUS / m_range;

    setCurrentRangeScale(sca);

    //更改VRM的数值，避免绘制的图形超出范围
  //  SystemInfo.VrmEblCtrl.vrmebl[0].vrm = m_range / 3.0;
 //   SystemInfo.VrmEblCtrl.vrmebl[1].vrm = m_range / 2.0;

    //设置相应的绘图参数
    const quint8 RngCount = 19;
    //每个量程的径向数据
    const float rnguints[RngCount] = {78, 155, 310, 465, 310, 465, 414, 465, 496, 465, 496, 496, 496, 496, 496, 496, 496, 485, 496};

    const quint8 i = SystemInfo.RangeScale.rngIndex();
    if(i < RngCount) {
        m_pixelPerRnguint = (float) ECHOVIEW_RADIUS/rnguints[i];
    }

    //清空屏幕和数据，重新开始画
   // initEchoView();

    //设置警戒区域
    if(MenuConfig.otherMenu.guardZoneEnable) {
        setGuardZoneClear();
        //setGuardZoneText();
        m_lpGuardZone->updateScreenPoint();
        setGuardZonePixel();  //重新计算警戒区域
    }

    //调谐变为自动调谐
    MenuConfig.otherMenu.tuneMan = 1;
    MenuConfig.otherMenu.tuneAuto = 1;
    lpMainWindow->updateOtherDlg();

    //刷新AIS数据
    screenCoordinateChanged();

    //所有尾迹情掉
    memset(timeDelay, 0, RADIUS3*RADIUS3);

}

void OpenGLView::rangeChanged(bool flag)
{
    quint8 x = SystemInfo.RangeScale.rngIndex();
    const quint32 ran = MenuConfig.installMenu.rangeChecked;
    //1：up  0: down
    if(flag) {
        if(x < 18)
            x++;
        else
            return;

        while(!(ran & ((quint32)1 << x))) {  //该位有量程显示
           if(x < 18)
               x++;
           else
               return;
        }
    }else {
        if(x > 0)
            x--;
        else
            return;

        while(!(ran & ((quint32)1 << x))) {  //该位有量程显示
           if(x > 0)
               x--;
           else
               return;
        }
    }


    //等待一圈的开始才绘图
     waitFirstAngleData = 1;
     //清空屏幕和数据，重新开始画
     initEchoView();
   //  m_paintNum = 1;  //角度大于10度



    if(MenuConfig.otherMenu.transmite)
    {
        //向FPGA发送量程变化命令
        QByteArray senddata;
        QDataStream in(&senddata, QIODevice::WriteOnly);
        if(lpInteract) {
            if(MenuConfig.installMenu.bestTuneValue[x] != 0) {
                in<<(quint8)170<<(quint8)0<<(quint8)9<<(quint16)(MenuConfig.installMenu.bestTuneValue[x]);  //AA0009 0000----发送最佳调谐值
                lpMainWindow->setManTuneValue(MenuConfig.installMenu.bestTuneValue[x]);
            } else {
                in<<(quint8)170<<(quint8)0<<(quint8)9<<(quint16)(256);
                lpMainWindow->setManTuneValue(256);
             }   if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
              //  qDebug()<< "FPGA write the best tune success!"<< x;
            senddata.clear();


            in<<(quint8)170<<(quint8)0<<(quint8)1<<(quint8)0<<(quint8)x;  //AA0001 0001 --- AA0001 0012   0-18
            if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
              //  qDebug()<< "FPGA write range success!"<< m_gain;
        }
   //     viewport()->update();

    }else{
        SystemInfo.RangeScale.setRange(x);
        setRange(SystemInfo.RangeScale.range(), SystemInfo.RangeScale.scale());
        //更新显示条
        lpMainWindow->updateDispCtrl();

        viewport()->update();
    }

}


void OpenGLView::change_HL1710(int flag)
{
    QByteArray senddata;
    QDataStream in(&senddata, QIODevice::WriteOnly);

    if(lpInteract) {
        in<<(quint8)170<<(quint8)0<<(quint8)6<<(quint8)0<<(quint8)flag;  //AA0006 0000 --- AA0006 0001
        if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
           // qDebug()<< "FPGA write HL1710 success!"<< flag;
    }

}


void OpenGLView::change_Xingneng(quint8 flag)
{
    QByteArray senddata;
    QDataStream in(&senddata, QIODevice::WriteOnly);

     if(lpInteract) {
         in<<(quint8)170<<(quint8)0<<(quint8)10<<(quint8)0<<(quint8)flag;  //AA000a 0000 --- AA000a 0001
         if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
         // qDebug()<< "FPGA write xingneng success!"<< flag;
     }
}

void OpenGLView::setJam(quint8 flag)
{
    //flag:ff ef df
    QByteArray senddata;
    QDataStream in(&senddata, QIODevice::WriteOnly);
    quint8 num[3] = {0xff, 0xef, 0xdf};

     if(lpInteract) {
         in<<(quint8)170<<(quint8)0<<(quint8)14<<(quint8)0<<(quint8)(num[flag]);  //AA000e 0000 --- AA000e 0001
         if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
         // qDebug()<< "FPGA write xingneng success!"<< flag;
     }
}

void OpenGLView::echoExpand(int flag)
{
    //flag:567
    QByteArray senddata;
    QDataStream in(&senddata, QIODevice::WriteOnly);

     if(lpInteract) {
         in<<(quint8)170<<(quint8)0<<(quint8)11<<(quint8)0<<(quint8)flag;  //AA000b 0000 --- AA000b 0001
         if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
         // qDebug()<< "FPGA write echoexpand success!"<< flag;
     }

}



//信号处理参数控制
void OpenGLView::change_gain(int flag)
{

    QByteArray senddata;
    int x;
    QDataStream in(&senddata, QIODevice::WriteOnly);
    //flag  1:up   0:down 2:nochange
    switch(flag) {
        case 1:
        if(m_gain < 100){
            m_gain++;
            x = 1;
         }
        break;
        case 0:
        if(m_gain > 0) {
            m_gain--;
            x = 1;
        }
        break;
        case 2:
        x=1;
        break;

    }

    if(x && lpInteract) {
        if(m_gain < 30)
            m_gain_real = m_gain + m_gain;
        else
            m_gain_real = m_gain + 30;

        in<<(quint8)170<<(quint8)0<<(quint8)4<<(quint8)0<<(quint8)m_gain_real;  //AA0004 0001 --- AA0004 007F   0-127
        if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
           // qDebug()<< "FPGA write gain success!"<< m_gain;

    }



}
void OpenGLView::change_restrain(int flag)
{
    quint8 m_restrain_real;
    QByteArray senddata;
    int x;
    QDataStream in(&senddata, QIODevice::WriteOnly);
    //flag  1:up   0:down
    switch(flag) {
        case 1:
        if(m_restrain < 100){
            m_restrain++;
            x = 1;
         }
        break;
        case 0:
        if(m_restrain > 0) {
            m_restrain--;
            x = 1;
        }
        break;
        case 2:
        x=1;
        break;

    }

    if(x && lpInteract) {
        if(m_restrain < 70)
            m_restrain_real = m_restrain;
        else
            m_restrain_real = m_restrain + m_restrain - 70;

        in<<(quint8)170<<(quint8)0<<(quint8)2<<(quint8)0<<(quint8)m_restrain_real;  //AA0002 0001 --- AA0002 007F   0-127
        if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
          //  qDebug()<< "FPGA write restrain success!"<< m_restrain;
    }
}
void OpenGLView::change_clutter(int flag)
{
    quint8 m_clutter_real;
    QByteArray senddata;
    int x;
    QDataStream in(&senddata, QIODevice::WriteOnly);
    //flag  1:up   0:down
    switch(flag) {
        case 1:
        if(m_clutter < 100){
            m_clutter++;
            x = 1;
         }
        break;
        case 0:
        if(m_clutter > 0) {
            m_clutter--;
            x = 1;
        }
        break;
        case 2:
        x=1;
        break;

    }
    if(x && lpInteract) {
        m_clutter_real = (quint8)(m_clutter * 0.64);
        in<<(quint8)170<<(quint8)0<<(quint8)3<<(quint8)0<<(quint8)m_clutter_real;  //AA0003 0001 --- AA0003 003F   0-63
        if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
          //  qDebug()<< "FPGA write restrain success!"<< m_clutter;
    }
}
//调谐
void OpenGLView::change_tune(quint8 flag, quint32 val, bool firstflag)
{
    QByteArray senddata;
    QDataStream in(&senddata, QIODevice::WriteOnly);
    //0:手动  1：自动
    if(!flag) {

        if(firstflag) {  //第一次手动时发送命令，其他时间不发送
            in<<(quint8)170<<(quint8)0<<(quint8)5<<(quint8)0<<(quint8)1;  //AA0005 0000 --- AA0005 0001  MT-AT
            if(lpInteract && (lpInteract->FpgaWrite(senddata) == Device_FPGA))
           // qDebug()<< "FPGA write  man tune success!";
            senddata.clear();
        }

            //发送手动调谐值
            in<<(quint8)170<<(quint8)0<<(quint8)7<<(quint16)val;  //AA0007 0000 --- AA0007 03FF  0-1023
            if(lpInteract && (lpInteract->FpgaWrite(senddata) == Device_FPGA));
            //   qDebug()<< "FPGA write  man tune data success!"<<val;


    }else {
        if(val == 65500) {  //表示开机后的自动调谐
            in<<(quint8)170<<(quint8)0<<(quint8)5<<(quint8)0<<(quint8)3;
            if(lpInteract && (lpInteract->FpgaWrite(senddata) == Device_FPGA));
             //   qDebug()<< "FPGA write  auto tune success!";

        }else {  //正常运行时的调谐，命令为0
            in<<(quint8)170<<(quint8)0<<(quint8)5<<(quint8)0<<(quint8)0;
            if(lpInteract && (lpInteract->FpgaWrite(senddata) == Device_FPGA));
              //  qDebug()<< "FPGA write normal auto tune success!";
        }
    }

}

void OpenGLView::change_tune_disp(quint32 val)
{
    //设置显示的调谐值
   // if(m_tune != val)
        m_tune = val;

}

void OpenGLView::setRngringShow(int flag)
{
    if(SystemInfo.ViewDisplay.rngring != flag) {
        SystemInfo.ViewDisplay.rngring = flag;
        setCurrentRangeScale(flag ? SystemInfo.RangeScale.scale() : 0);

    }
}
void OpenGLView::setHeadlineShow(int flag)
{
    if(SystemInfo.ViewDisplay.guildline != flag) {
        SystemInfo.ViewDisplay.guildline = flag;

    }
}
void OpenGLView::setTrailTime(int flag)
{
    switch(flag)
    { case 0:
           m_keepTime = 0;
           for(quint32 i = 0; i < RADIUS3*RADIUS3; i++){
                   timeDelay[i] = 0;
           }
           break;
      case 1:
           m_keepTime = 5;
           break;
      case 2:
           m_keepTime = 10;
           break;
     case 3:
           m_keepTime = 60;
           break;
     case 4:
           m_keepTime = 120;
           break;
     case 5:
           m_keepTime = -1;
           break;
    default:
           m_keepTime = 0;
             break;
   }
   m_keepTime_5s = (m_keepTime>=0 ? m_keepTime/5 : -1);

   lpMainWindow->updateCtrlPanel(VIEW_UPDATE_TRAIL_BIT);
}
void OpenGLView::updateDelayTime()
{
    if(m_keepTime_5s < 0)
        return;

    for(quint32 i = 0; i < RADIUS3*RADIUS3; i++){
        if( timeDelay[i] > 0)
            timeDelay[i] --;
    }
}



void OpenGLView::initEchoView(void)
{
    quint32 i,j,z;

    //设置背景色，后续要根据选择更改
    quint32 size=echoviewOutCircle.size();
    for(z=0; z<size; z++) {
        i = echoviewOutCircle.at(z).x();
        j = echoviewOutCircle.at(z).y();

        echoView[i][j] = backColor;
    }

    size = echoviewCircle.size();
    for(z=0; z<size; z++) {
        i = echoviewCircle.at(z).x();
        j = echoviewCircle.at(z).y();

        echoView[i][j] = echobackColor;

    }

}

void OpenGLView::setScene(OpenGLSence *pScene)
{
    QGraphicsView::setScene(pScene);
}
void OpenGLView::setBackColor(quint32 color)
{

    //RRGGBB
   const quint8 backC_r = (quint8)((color>>16) & 0xff);
   const quint8 backC_g = (quint8)((color>>8) & 0xff);
    const quint8 backC_b = (quint8)(color & 0xff);
    backColor = ColorConvert(backC_r, backC_g, backC_b);
    initEchoView();
    //刷新显示
    viewport()->update();
}
void OpenGLView::setForeColor(quint32 color)
{
    const quint8 echoC_r = (quint8)((color>>16) & 0xff);
    const quint8 echoC_g = (quint8)((color>>8) & 0xff);
    const quint8 echoC_b = (quint8)(color & 0xff);
    echoColor = ColorConvert(echoC_r, echoC_g, echoC_b);

}
void OpenGLView::setLineColor(quint32 head, quint32 var)
{
    m_headingColor.setRgb(head);
    m_varlineColor.setRgb(var);
}


void OpenGLView::InitGL(void)
{

    glEnable(GL_MULTISAMPLE);
 //   glEnable(GL_DEPTH_TEST);
    glClearColor(0.4, 0.4, 0.4, 1.0);
    glShadeModel(GL_SMOOTH);

}

void OpenGLView::ResizeGL(int width, int height)
{

    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
  //使窗口坐标跟屏幕大小对应，使用2D绘图
 // gluPerspective(45.0, (GLfloat)width/(GLfloat)height, 0.1, 100.0);
    //gluOrtho2D(0,width,0,height);
    glOrtho(0, width, 0, height, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}
/*07回波强度最大， 0102算杂波，强度最低*/
//定义绘制一个像素点//角度 半径 显示标志   多色模式下的绘图
#define DrawPixel(tr, p, echoflag) {  \
if(tr >= MAXDISPAZI)  {  continue;  };  \
const QList<qint32> &idx_list = m_pixelIndex[tr][p];  \
const int idx_size = idx_list.size();   \
for(int i=0; i<idx_size; i++) {   \
    const int virt_idx = idx_list[i]+m_offset_index;   \
    if(virt_idx<0 || virt_idx>=4194304) { /*DIAMETER*DIAMETER*/ continue;  }  \
    const qint32 scrn_idx = m_screenIndex[virt_idx];   \
    if(scrn_idx < 0) {  continue;  }   \
    const quint32 y = (scrn_idx>>10);  \
    const quint32 x = (scrn_idx - (y << 10));    \
    switch(echoflag) {  \
    case 0x01:  \
        echoView[y][x] = echoclutterColor; \
    break; \
    case 0x02:   \
        echoView[y][x] = echoclutterColor; \
    break; \
    case 0x03:  \
        echoView[y][x] = echoColor3; \
        timeDelay[scrn_idx] = m_keepTime_5s; \
    break; \
    case 0x04:   \
        echoView[y][x] = echoColor3; \
        timeDelay[scrn_idx] = m_keepTime_5s; \
    break; \
    case 0x05:  \
        echoView[y][x] = echoColor2; \
        timeDelay[scrn_idx] = m_keepTime_5s; \
    break; \
    case 0x06:   \
        echoView[y][x] = echoColor2; \
        timeDelay[scrn_idx] = m_keepTime_5s; \
    break; \
    case 0x07:   \
        echoView[y][x] = echoColor; \
        timeDelay[scrn_idx] = m_keepTime_5s; \
    break; \
    default:  \
        if(m_keepTime_5s >= 0) { \
            if(timeDelay[scrn_idx] == 0) {   \
                echoView[y][x] = echobackColor; \
            }else{   \
                echoView[y][x] = echotrailColor; \
            } \
        }else if((echoView[y][x] == echoColor)){  \
            echoView[y][x] = echotrailColor;  \
        }  \
   break; \
  } \
}  \
}

//单色模式下的绘图定义
#define DrawPixel2(tr, p, echoflag) {  \
if(tr >= MAXDISPAZI)  {  continue;  };  \
const QList<qint32> &idx_list = m_pixelIndex[tr][p];  \
const int idx_size = idx_list.size();   \
for(int i=0; i<idx_size; i++) {   \
    const int virt_idx = idx_list[i]+m_offset_index;   \
    if(virt_idx<0 || virt_idx>=4194304) { /*DIAMETER*DIAMETER*/ continue;  }  \
    const qint32 scrn_idx = m_screenIndex[virt_idx];   \
    if(scrn_idx < 0) {  continue;  }   \
    const quint32 y = (scrn_idx>>10);  \
    const quint32 x = (scrn_idx - (y << 10));    \
    switch(echoflag) {  \
    case 0x01:  \
        echoView[y][x] = echoclutterColor; \
    break; \
    case 0x02:   \
        echoView[y][x] = echoclutterColor; \
    break; \
    case 0x03:  \
        echoView[y][x] = echoclutterColor; \
    break; \
    case 0x04:   \
         echoView[y][x] = echoclutterColor; \
    break; \
    case 0x05:  \
         echoView[y][x] = echoclutterColor; \
    break; \
    case 0x06:   \
        echoView[y][x] = echoColor; \
        timeDelay[scrn_idx] = m_keepTime_5s; \
    break; \
    case 0x07:   \
        echoView[y][x] = echoColor; \
        timeDelay[scrn_idx] = m_keepTime_5s; \
    break; \
    default:  \
        if(m_keepTime_5s >= 0) { \
            if(timeDelay[scrn_idx] == 0) {   \
                echoView[y][x] = echobackColor; \
            }else{   \
                echoView[y][x] = echotrailColor; \
            } \
        }else if((echoView[y][x] == echoColor)){  \
            echoView[y][x] = echotrailColor;  \
        }  \
   break; \
  } \
}  \
}



//回波绘图函数
void OpenGLView::PaintGL(void)
{

    QTime tt;
    tt.start();


    glClear(GL_COLOR_BUFFER_BIT);

    //显示回波数据
    glRasterPos2i(0,0);
#ifdef OPENGL_RGBA
    glDrawPixels(echoViewWidth, echoViewHeight, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, echoView);
#endif
#ifdef OPENGL_SHORT
    glDrawPixels(echoViewWidth, echoViewHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, echoView);
#endif
#ifdef OPENGL_BYTE
      glDrawPixels(echoViewWidth, echoViewHeight, GL_RGB, GL_UNSIGNED_BYTE_3_3_2, echoView);
#endif

    qDebug() << "write to screen time elapsed:" << tt.elapsed() ;
}

void OpenGLView::drawBackground(QPainter *painter, const QRectF &rect)
{

    painter->beginNativePainting();
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    InitGL();
    ResizeGL(echoViewWidth, echoViewHeight);
    PaintGL();

   glPopAttrib();
    painter->endNativePainting();


}

void OpenGLView::drawForeground(QPainter *painter, const QRectF &rect)
{
     QColor color = QColor(Qt::red);
     color = color.lighter(MenuConfig.dispMenu.fixlineBright);
     painter->setPen(color);
     painter->drawText(QPointF(512,512), "");  //真尼玛奇怪，一定要这句话后面才能画出来。。。。。。。

     //画外围方位圈
     drawOuterAzimuthCircle(painter);


     if(MenuConfig.otherMenu.transmite) {  //发射才能画

     if(SystemInfo.ViewDisplay.offset) {
         QPainterPath clip;
         clip.addEllipse(QPointF(512,512), 500,500);
         painter->setClipPath(clip);
     }

     //画刻度线
     drawScanLineP(painter);
     //画正北/航向船艏指示线
     drawGuildLine(painter);
     //绘制EBLVRM
     drawVrmEbl(painter);


     //绘警戒区域,guardSelect 0表示手动
     if(MenuConfig.otherMenu.guardZoneEnable) {
          if(MenuConfig.otherMenu.guardSelect) {
              m_lpGuardZone->paint(painter);             //根据鼠标点画扇形
          }
      }

     // 画导航点
     m_lpNaviPoint->paint(painter);


     //绘AIS目标，GPS信号和罗经的船艏信号有效才显示
     const LINKSTATUS* lpStatus = lpMainWindow->linkStatus();
   //  if(NORMAL == lpStatus[GPS_DEVICE].status && NORMAL == lpStatus[COMP_DEVICE].status)
     {
             extern quint8 gAtaAisSwitch;
             if(1 == gAtaAisSwitch && MenuConfig.dispMenu.showArpa)
             {

             }

             if(0 == gAtaAisSwitch && MenuConfig.dispMenu.showAis)
             {
             if(lpAisManage)
                 lpAisManage->paint(painter);
             }
     }


    }



     QFont font0 = painter->font();
     // 显示首次启动信息
     if(FirstWaitTime > 0)
     {
         const QString textLine1 = "S E N Y A";
         const QString textLine2 = QString("%1").arg(FirstWaitTime);
         const QString textLine3 = QString("ON TIME %1 h").arg(MenuConfig.installMenu.timeUsed/3600);
         const QString textLine4 = QString("TX TIME %1 h").arg(MenuConfig.installMenu.timeTran/3600);
         font0.setPixelSize(60);
         painter->setPen(Qt::yellow);
         painter->setFont(font0);

         QFontMetrics fm(font0);  //得到字体的实际像素长度
         const int width1 = fm.width(textLine1), width2 = fm.width(textLine2);
         const int height0 = fm.height();

         painter->drawText((width()-width1)/2, height()/2-height0, textLine1);
         painter->drawText((width()-width2)/2, height()/2, textLine2);

         font0.setPixelSize(20);
         painter->setFont(font0);

         QFontMetrics fm2(font0);  //得到字体的实际像素长度
         const int width3 = fm2.width(textLine3), width4 = fm2.width(textLine4);
         const int height2 = fm2.height();
         painter->drawText((width()-width3)/2, height()/2+height2 + 20, textLine3);
         painter->drawText((width()-width4)/2, height()/2+2*height2 + 20, textLine4);
     }
     else if(!MenuConfig.otherMenu.transmite)
     {
         const QString textLine1 = "S T - B Y";
         font0.setPixelSize(60);
         painter->setPen(Qt::yellow);
         painter->setFont(font0);
         QFontMetrics fm(font0);
         const int width1 = fm.width(textLine1);
         painter->drawText((width()-width1)/2, height()/2, textLine1);
     }

}


//  packetnum  绘图数据索引，angle 绘图截止角度
void OpenGLView::drawAngle(const quint32 &startPacket, const quint32 &endPacket, bool last_flag)
{
    /****取当前角度信息作为绘图范围，取前一个角度数据作为绘图数据******/
        if(! MenuConfig.otherMenu.transmite) {          
            flag_guardZone = 0;
            flag_paintBack =0;
            flag_paintEcho =0;
            return;
         }

        //警戒区域闪烁，先画上次其隐藏的回波，再更新，就能变闪烁变更新，一直闪烁，手动关闭
            if((flag_guardZone != 0) && (flag_guardZone%2 == 0) && !flag_paintEcho && flag_paintBack)
            {
                flag_guardZone++;
                changeGuardColor(0);  //变为回波色
                flag_paintEcho = 1;
            }



        for(quint32 num = startPacket; num < endPacket; num++) {

             const ECHODATA tempData = m_echoDataList[num];
             const int angle  = (last_flag && (num == (endPacket - 1))) ? m_echoDataList[0].angle : m_echoDataList[num+1].angle;

             //设置量程
             if(SystemInfo.RangeScale.rngIndex() != tempData.range) {
                  SystemInfo.RangeScale.setRange(tempData.range);
                  this->setRange(SystemInfo.RangeScale.range(), SystemInfo.RangeScale.scale());
                  //更新显示条
                  lpMainWindow->updateDispCtrl();
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
                     qDebug()<<"azimuth1,azimuth2,crntIndex,angle1,angle2"<< azimuth1 << azimuth2 <<num <<tempData.angle<<angle;
                     return;
                 }

                 const int pmax = CurrentRange[azimuth1/5] - 3;  //最大显示半径
                 const int length = tempData.length;

                 int p = 0,  m = 0;
                 float  pp = 0.0;
                 for(p=pp,m=0; m<length && p<pmax; m++)
                 {
                     pp += m_pixelPerRnguint;  //一个距离单元对应的像素宽度
                     const int p0 = (int)pp;
                     if(p != p0) {
                         const quint8 echoflag = tempData.echo[m];
                         const int pmax0 = (p0 < pmax ? p0 : pmax);
                         for(; p < pmax0; p++) {
                             for(int azi=azimuth1; azi < azimuth2; azi++) {
                                 if(azi < 0 ) azi += MAXDISPAZI;
                                 const int tr = (MAXDISPAZI > azi ? azi : (azi - MAXDISPAZI));

                                 if(gzaBgn < gzaEnd){
                                     //先检测角度再半径再回波数据
                                     if(MenuConfig.otherMenu.guardZoneEnable && (flag_guardZone==0) && (tr>gzaBgn)&& (gzaEnd>tr) && (p>gzrMix) && (p<gzrMax) && ((echoflag==0x07)||(echoflag==0x06)))
                                     {
                                         flag_guardZone = 1;
                                         flag_paintBack = 0;
                                         flag_paintEcho = 0;
                                         guardZone_alarm = 11;  //报警标志
                                     }
                                 }else{  //选择区域跨越船艏线
                                      if(MenuConfig.otherMenu.guardZoneEnable && (flag_guardZone==0) && (((tr>gzaBgn)&&(3600>tr)) || ((tr>=0)&&(gzaEnd>tr)))&& (p>gzrMix) && (p<gzrMax) && ((echoflag==0x07)||(echoflag==0x06)))
                                     {
                                         flag_guardZone = 1;
                                         flag_paintBack = 0;
                                         flag_paintEcho = 0;
                                         guardZone_alarm = 11;  //报警标志
                                     }
                                 }

                                 if(colorMode) {  //多色
                                     DrawPixel(tr, p, echoflag);  //绘制像素
                                 }else {
                                     DrawPixel2(tr, p,echoflag);
                                 }

                             }  //end azi
                         }

                     }

                }

             }  //end if

        }  //end for

        if((flag_guardZone%2 == 1) && !flag_paintBack && !flag_paintEcho)  //间隔2次再闪烁
        {
            flag_guardZone++;
            if(flag_guardZone == 200)   flag_guardZone = 2;
            changeGuardColor(1);  //变为背景色
            flag_paintBack = 1;

        }
        if(flag_paintEcho) {
            flag_paintBack =0;
           flag_paintEcho = 0;
        }


}



//////////////////接受回波数据处理函数/////////////////
void OpenGLView::setEchoData(ECHODATA& m_data)
{

    if((m_data.packetNum < MAXPACKETNUM) && (m_data.packetNum > 0)) {
        //数据同步
        QMutexLocker locker(&m_echoReceivedMutex);
        {
             m_echoDataList[m_data.packetNum - 1] = m_data;
        }
        locker.unlock(); //数据解锁
    }else {
        qDebug()<<"packetnum error:"<<m_data.packetNum;
        return;
    }


    if(m_data.packetNum == 1) {
        //新的一圈
        if(lastPaint) {
            lastPaint = 0;
            flag_newCircle = 1;

            m_startPacketNum = m_endPacketNum;   //存的都是索引
            m_endPacketNum = m_data.packetNum - 1;
            drawAngle(m_startPacketNum, m_lastPacketNum, true);  //绘制像素，最后一帧到第一帧之间的角度也要绘制

            emit painting();
        }
        m_paintNum = 1;  //角度大于10度
    }

    //角度超过10度就绘图或者最后一次绘图到了
    if(m_data.angle >= (m_paintNum * 80)) {
        lastPaint = 1;
        m_paintNum++;  //角度增加8度
        flag_newCircle = 0;

        m_startPacketNum = m_endPacketNum;   //存的都是索引
        m_endPacketNum = m_data.packetNum - 1;
        drawAngle(m_startPacketNum, m_endPacketNum);  //绘制像素

        emit painting();
    }

    m_lastPacketNum = m_data.packetNum;  //保存上一次数据包包数

}
/***************保存回波数据****************************************/
/* QFile file(QDir::currentPath() + "/echo.txt");
if(!file.open(QIODevice::WriteOnly | QIODevice::Append))
{
    return;
}
QTextStream out(&file);
out<< "length:";
out << m_data.length;
out<< "range:";
out<<m_data.range;
out<<"angle:";
out<<m_data.angle;
out<<"packetnum:";
out<<m_data.packetNum;
out<<"data:";
int l = m_data.length;
for(int x=0; x<l; x++) {
    out<<m_data.echo[x];
}
out<<"\n";
file.close(); */
/********************************************************************/
void OpenGLView::resetEchoView()
{
    lastPaint = 0;
    for(int i=0; i< MAXPACKETNUM; i++) {
        memset(&m_echoDataList[i], 0, sizeof(ECHODATA));
    }
}
//天线警告信息处理
void OpenGLView::antennaWarning(quint8 warning)
{
    //天线报警信息,最高位表示报警，后三位分别为视频，bp方位角，hd船艏信号
    if((quint8)(warning & 0x01)) {
        //船艏线号丢失报警
        QString msg = QString("船艏线信号丢失！");
        lpAlarm->startAlarm((ALARMLEVEL)g_systemPara.alarmPara.signalLostLevel, msg);
    }
    if((quint8)(warning & 0x02)) {
        //方位角报警
        QString msg = QString("方位角信号丢失！");
        lpAlarm->startAlarm((ALARMLEVEL)g_systemPara.alarmPara.signalLostLevel, msg);
    }
    if((quint8)(warning & 0x04)) {
        //视频信号报警
        QString msg = QString("视频信号丢失！");
        lpAlarm->startAlarm((ALARMLEVEL)g_systemPara.alarmPara.signalLostLevel, msg);
    }

}

void OpenGLView::receivedpaint(void)
{
    //只能以这种方式更新屏幕，不能在另外一个线程中调用
   viewport()->repaint();
 //   viewport()->update(rect());
}
void OpenGLView::updateView(void)
{
    viewport()->update(rect());
}


void OpenGLView::mousePressEvent(QMouseEvent *event)
{
   // if(m_mouseTracking)
    //    mouseMoved(event->globalPos(), true);

    // 鼠标选择航迹标牌
   // trackBoardSelect(event->pos());

    const int x = event->x(), y = event->y();
    if(event->button() == Qt::LeftButton)
    {
        lButtonDownProcess(x, y);
    }
    else if(event->button() == Qt::RightButton)
    {
        //rButtonDownProcess(x, y);
    }

}
void OpenGLView::mouseMoveEvent(QMouseEvent *event)
{
    QPoint p(event->globalX()-512, event->globalY()-512);
    if(m_mouseTracking && sqrt((double)(p.x()*p.x() + p.y()*p.y())) <= ECHOVIEW_RADIUS) {
        if(!mouseShapeChanged)
            setCursor(Qt::CrossCursor);
        else {
            setCursor(Qt::PointingHandCursor);
            if(m_lpGuardZone->crntPoint())  //为1表示没有选中第二点
                m_lpGuardZone->setSecondPoint(event->globalPos(), 0); //设置的屏幕坐标
        }

        lpMouseOpetarion->mouseMoved(event->globalPos(), true);
    }else {
       // lpMouseOpetarion->mouseMoved(event->globalPos(), false);
        setCursor(Qt::ArrowCursor);
        if(oncetime == 0) {
            m_lpDialog->initOperateWidget();  //监视面板加入鼠标操作
            oncetime =1;
        }
    }

}

// 鼠标左键按下处理
void OpenGLView::lButtonDownProcess (int x, int y)
{
    const int MouseOperator = lpMouseOpetarion->mouseProcessID();
    if(MouseOperator != NULL_PROCESS)
    {
        const int dx = x - 511, dy = y - 511;
        if(sqrt((double)(dx*dx+dy*dy)) < 500){
                viewOperate(x, y);
        }
    }else {
        const int dx = x - 511, dy = y - 511;
        if(sqrt((double)(dx*dx+dy*dy)) < 500){
              //AIS/ATA
            const quint32 mmsi = lpAisManage->ptOnAisShip(QPoint(x,y));
            lpAisManage->setSelectedShip(mmsi);
            if(mmsi > 0)
                m_lpDialog->updateAis(mmsi, TgtBox::TGTBOX_SWITCH);
        }

    }

}

// 视图操作处理
void OpenGLView::viewOperate(const int x, const int y)
{

    const SCREEN_POINT sc = {x, y};
    const int MouseOperator = lpMouseOpetarion->mouseProcessID();
    switch(MouseOperator)
    {
        // 设置导航点
        case SET_NAVI_POINT:
        {
                if(m_lpNaviPoint){
                        const quint8 index = m_lpNaviPoint->setPoint(sc.toPoint());
                        if(index < 4){
                                viewport()->update(rect());
                                OtherMenu *lpMenu = qobject_cast<OtherMenu*>(lpMainWindow->crntTopMenu());
                                if(lpMenu)
                                        lpMenu->updateNaviPointDisplay(1<<index);
                        }
                }
                break;
        }

        // 删除导航点
        case DEL_NAVI_POINT:
        {
                if(m_lpNaviPoint){
                        const quint8 index = m_lpNaviPoint->delPoint(sc.toPoint());
                        if(index < 4) {
                                viewport()->update(rect());
                                OtherMenu *lpMenu = qobject_cast<OtherMenu*>(lpMainWindow->crntTopMenu());
                                if(lpMenu)
                                        lpMenu->updateNaviPointDisplay(1<<index);
                        }
                }
                break;
        }

        //设置警戒圈对角点
        case SELECT_AZI_SECTOR_PROCESS:
        {
            if(m_lpGuardZone){
                if(!m_lpGuardZone->crntPoint())
                    m_lpGuardZone->setFirstPoint(sc.toPoint());
                else {
                    m_lpGuardZone->setSecondPoint(sc.toPoint(), 1);  //需要执行其他操作
                    m_lpGuardZone->setCrntPoint(0);

                    setMouseShape(0);
                    lpMouseOpetarion->setMouseProcessID(NULL_PROCESS);
                }

                viewport()->update(rect());
            }

            break;
        }


    default:
        break;
    }
}





/**************量程变化函数****************/
void OpenGLView::rationChanged(bool changed)
{
    if(!changed)    return;
}

//画p显刻度线
void OpenGLView::drawScanLineP(QPainter *p0)
{

    QPainter &p = *p0;
    QPen pen = p.pen();

    //设置裁剪区域,很耗CPU。。。。。。。。。。。。。
   /* if(SystemInfo.ViewDisplay.offset) {
        QPainterPath path;
        path.addEllipse(QPointF(512, 512), ECHOVIEW_RADIUS, ECHOVIEW_RADIUS); //调整放大4个像素
        p.setClipPath(path);
        p.setClipping(true);
    }*/
    //使用画圆弧的函数，消耗cpu更少

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
           // const int mr = 500;  //绘图区域半径
           // const int ml = m_offset_maxradius - 500;  //偏移圆心距离
           // const int sl = 1000 - m_offset_maxradius;  //偏移外圈距离


            for(rp0=dp; rp0 < m_offset_maxradius; rp0+=dp)  //把最外圈放在画方位圈那里绘出来
            {
                const int diameter =(int)(rp0);  //距标圈半径
              /*  if(diameter > sl) {
                    const double cosdata = (double)(ml*ml + diameter*diameter - mr*mr) / (2 * ml * diameter);
                    const double theta1 = acos(cosdata);
                    const double theta2 = RADIANTODEGREE(theta1);  //将弧度转换为角度
                    const QRect rect = QRect(sc_pt.x()-diameter, sc_pt.y()-diameter, diameter+diameter, diameter+diameter);

                    int offsetangle = (int)(theta_offset.y()/M_PI*180.0 - 90);
                    offsetangle = offsetangle < 0 ? offsetangle+360 : offsetangle;

                    int spanangle = (int)(2 * theta2 * 16) - 32;  //少画一点
                    int startangle = -(int)(offsetangle - 180 + theta2) * 16 + 16;
                    p.drawArc(rect, startangle, spanangle);


                }else */ { //半径小于等于最短距离时画圆
                    p.drawEllipse (sc_pt, diameter, diameter);
                }
            }

        }
    }

    // 绘制方位刻度线
  /*  if (BIGZERO(aziScale))
    {

        float rmax4aziscal = rmax;
        float fixedrng[MAXDEGREE];

        for(int i=0;i<MAXDEGREE; i++)
            fixedrng[i] = rmax4aziscal;

        RTHETA_POINT rtheta;
        rtheta.setX (rmax4aziscal);

        // 先画30度
      //  pen.setColor (Degree30Color);
        p.setPen (pen);
        for (int a=0; a<360; a+=30)
        {
            rtheta.setX(fixedrng[a]);
            rtheta.setY(DEGREETORADIAN(a));
            SCREEN_POINT sc0 = squaretoscreen_view(rtheta_to_square(rtheta));
            QPoint sc1 = sc_pt;
            if (m_scanMode == EMPTY_SCAN)
                sc1 = square_to_screen(QPointF(m_scanKiloRange*EachDegreeSin2[a], m_scanKiloRange*EachDegreeCos2[a]));
            p.drawLine (sc1, sc0.toPoint());
        }
        // 再画10度
        if (aziScale < 30)
        {
          //  pen.setColor (Degree10Color);
            p.setPen (pen);
            for (int a=0; a<360; a+=10)
            {
                if (a%30==0)
                    continue;
                rtheta.setX(fixedrng[a]);
                rtheta.setY(DEGREETORADIAN(a));
                SCREEN_POINT sc0 = squaretoscreen_view(rtheta_to_square(rtheta));
                QPoint sc1 = sc_pt;
                if (m_scanMode == EMPTY_SCAN)
                    sc1 = square_to_screen(QPointF(m_scanKiloRange*EachDegreeSin2[a], m_scanKiloRange*EachDegreeCos2[a]));
                p.drawLine (sc1, sc0.toPoint());
            }
        }
        // 再画5度
        if (aziScale == 5)
        {
           // pen.setColor (Degree5Color);
            p.setPen (pen);
            for (int a=0; a<360; a+=5)
            {
                if (a%10==0)
                    continue;
                rtheta.setX(fixedrng[a]);
                rtheta.setY(DEGREETORADIAN(a));
                SCREEN_POINT sc0 = squaretoscreen_view(rtheta_to_square(rtheta));
                QPoint sc1 = sc_pt;
                if (m_scanMode == EMPTY_SCAN)
                    sc1 = square_to_screen(QPointF(m_scanKiloRange*EachDegreeSin2[a], m_scanKiloRange*EachDegreeCos2[a]));
                p.drawLine (sc1, sc0.toPoint());
            }
        }
    }*/

}


// 画外围的方位圈,旋转角度与回波角度不一样
void OpenGLView::drawOuterAzimuthCircle(QPainter* p0)
{
    QPainter& p = *p0;
    int radius = ECHOVIEW_RADIUS;

    // 雷达中心点所对应的屏幕坐标(以雷达为圆心)
    const QPoint center = QPoint(512,512);  //square_to_screen(QPointF(0,0));

    QFont font = p.font();
//    font.setPointSize(13);	// 这里不再设置字体大小，由调用处设置
//    p.setFont(font);
    const int left = QFontMetrics(font).boundingRect(("456")).width() / 2;


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

        if(radius0 < 0)	radius0 = 0;

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
            p.setRenderHint(QPainter::Antialiasing);
            p.translate(x3, y3);
            p.rotate(idx);
            if(i < 10)
                p.drawText(-left/3, 0, QString::number(i));
            else if( i< 100)
                p.drawText(-left*2/3, 0, QString::number(i));
            else
                p.drawText(-left, 0, QString::number(i));
            p.restore();
        }

    }

    // 画外圈(以屏幕中心为圆心)
    p.drawEllipse (QPointF(512,512), 500, 500);

}
//画指引线
void OpenGLView::drawGuildLine(QPainter *p0)
{
    if(!SystemInfo.ViewDisplay.guildline)
        return;

    const QColor color = m_headingColor.lighter(MenuConfig.dispMenu.varlineBright);

    QPen pen (color);
    pen.setWidth(2.0);
    p0->setPen(pen);

    //船艏线应该和图像选转角度一样
    float dispAzi = 0;//SystemInfo.ShipInfo.head;

    dispAzi = rotation();
   /* if(N_UP == SystemInfo.ViewDisplay.upmode)  //真北
        {
        dispAzi = SystemInfo.ShipInfo.head;
        //qDebug()<<"real north";
        }
    else if(C_UP == SystemInfo.ViewDisplay.upmode){  //航向
      //  dispAzi = SystemInfo.ShipInfo.head - SystemInfo.ShipInfo.course;
       // if(dispAzi < 0) dispAzi += 360.0;
        dispAzi == SystemInfo.ShipInfo.course;
        //qDebug()<<"course north";
    }*/
    //setRotation(dispAzi);
    QPoint m_scCenter;
    //偏心或者真运动的情况下图形中心都在偏心点
    if(SystemInfo.ViewDisplay.offset ||  SystemInfo.ViewDisplay.moving == REALMOVING)
        m_scCenter = QPoint(m_offsetPoint.x(), m_offsetPoint.y());  //雷达中心
    else
        m_scCenter = QPoint(512, 512);
   // qDebug()<<"guild line center is"<<m_scCenter.x()<<m_scCenter.y();
   // const int m_radius = SystemInfo.RangeScale.range() * ration();  //得到显示半径
    const int m_radius = CurrentRange[(quint16)dispAzi << 1];

    //const float theta = DEGREETORADIAN(SystemInfo.ShipInfo.head);
    //qDebug()<<"head line and bright"<<SystemInfo.ShipInfo.head<<MenuConfig.dispMenu.varlineBright;
    const QPoint screen_point = QPoint((m_radius * EachDegreeSin2[(int)dispAzi]+m_scCenter.x()), (m_scCenter.y()-m_radius * EachDegreeCos2[(int)dispAzi]));

    p0->drawLine(m_scCenter, screen_point);

}
//画VRM/EBL
void OpenGLView::drawVrmEbl(QPainter *painter)
{
    const QColor color = m_varlineColor.lighter(MenuConfig.dispMenu.varlineBright);

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

        /*
        SCREEN_POINT sc;
        if(vrmebl.offset)
        {
            SQUARE_POINT sq;
            sq.fromPoint(vrmebl.offsetPos);
            sc = m_radarView->square_to_screen(sq);
        }
        else
            sc.fromPoint(m_scCenter);*/

        // 显示VRM
        if((vrmebl.show & 0x01) && ((!SystemInfo.ViewDisplay.offset && vrmebl.vrm < range()) || (SystemInfo.ViewDisplay.offset && vrmebl.vrm < 2*range())) && BIGZERO(rp))
        {
            /* const int mr = 500;  //绘图区域半径
             const int ml = m_offset_maxradius - 500;  //偏移圆心距离
             const int sl = 1000 - m_offset_maxradius;  //偏移外圈距离

             if(SystemInfo.ViewDisplay.offset && (rp > sl)) {

                 const double cosdata = (double)(ml*ml + rp*rp - mr*mr) / (2 * ml * rp);
                 const double theta1 = acos(cosdata);
                 const double theta2 = RADIANTODEGREE(theta1);  //将弧度转换为角度
                 const QRect rect = QRect(sc.x()-rp, sc.y()-rp, rp+rp, rp+rp);

                 int offsetangle = (int)(theta_offset.y()/M_PI*180.0 - 90);
                 offsetangle = offsetangle < 0 ? offsetangle+360 : offsetangle;

                 int spanangle = (int)(2 * theta2 * 16) - 32;  //少画一点
                 int startangle = -(int)(offsetangle - 180 + theta2) * 16 + 16;
                 painter->drawArc(rect, startangle, spanangle);

             }else */ {
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
    }

}



void OpenGLView::setBoatHeading(float head)
{
    if(!EQUAL(SystemInfo.ShipInfo.head, head))
    {
        SystemInfo.ShipInfo.head = head;
        //旋转视图
        updateViewRotation();
        if(SystemInfo.ViewDisplay.upmode == H_UP)
        {
            //更改相应的AIS和目标的坐标
            //screenCoordinateChanged();
            viewport()->update(rect());
        }
    }
}
void OpenGLView::setBoatCorse(float course)
{
    if(!EQUAL(SystemInfo.ShipInfo.course, course))
    {
        SystemInfo.ShipInfo.course = course;
        updateViewRotation();
        if(SystemInfo.ViewDisplay.upmode == C_UP)
        {
            //screenCoordinateChanged();
            this->viewport()->update();
        }
    }
}

// 判断指定位置点是否在显示区域内
bool OpenGLView::isPointDisplay(const PLOTPOSITION& position)
{
    const float r = position.rtheta_point.r();

    if(SystemInfo.ViewDisplay.offset)
    {
        const int a = RADIANTODEGREE(position.rtheta_point.theta());
        return r < CurrentRange[a << 1];  //乘2
    }
    else
        return r < range();

}


void OpenGLView::ownshipPositionChanged()
{
    double lon, lat;
    if(NULL== m_boatAlarm) {
        return;
    }
    m_boatAlarm->ownshpPosition(lat, lon);

         // set EchoView,offset
     LATITUDE_POINT ld;// = {lon, lat};
     ld.setPoint(lon*COEF_DEGREETORADIAN, lat*COEF_DEGREETORADIAN);  //需转化为弧度
     if(SystemInfo.ViewDisplay.moving == RELATIVEMOVING) {	//相对运动
          lpMouseOpetarion->setCenterLatitude(ld);

          viewport()->update(rect());
     }else if(SystemInfo.ViewDisplay.moving == REALMOVING){
         //真运动
         SCREEN_POINT sd = lpMouseOpetarion->latitude_to_screen(ld);
         double distance = (sd.x()-300)*(sd.x()-300) +(sd.y()-300)*(sd.y()-300);
         distance = sqrt(distance);
         if(distance > width()/3.0){
                 lpMouseOpetarion->setCenterLatitude(ld);
                 setOffsetPoint(300, 300);

                 viewport()->update(rect());
         }else {
             lpMouseOpetarion->setCenterLatitude(ld);
             setOffsetPoint(sd.x(), sd.y());  //中心点在屏幕上移动
             viewport()->update(rect());
         }
    }
}

void OpenGLView::setMouseShape(quint8 flag)
{
    mouseShapeChanged = flag;
}


void OpenGLView::setGuardZonePixel()
{
    //获取警戒区域坐标，极坐标,角度弧度表示
    double r01,r02,a01,a02;
    const QPointF pt1 = m_lpGuardZone->first_r_point();
    const QPointF pt2 = m_lpGuardZone->second_r_point();  //得到两个对角点的坐标，
    RTHETA_POINT rt1,rt2;
    rt1.setPoint(pt1.x(), pt1.y());
    rt2.setPoint(pt2.x(), pt2.y());
    SCREEN_POINT sc1 = lpMouseOpetarion->rtheta_to_screen(rt1);
    SCREEN_POINT sc2 = lpMouseOpetarion->rtheta_to_screen(rt2);


    if(!SystemInfo.ViewDisplay.offset) {
        float x = sc1.x() - 512.0;
        float y = sc1.y() - 512.0;
        float x1 = sc2.x() - 512.0;
        float y1 = sc2.y() - 512.0;
        r01 = sqrt((double)(x*x + y*y)), a01 = pt1.y();
        r02 = sqrt((double)(x1*x1 + y1*y1)), a02 = pt2.y();
    }else {
        float x = sc1.x() - m_offsetPoint.x();
        float y = sc1.y() - m_offsetPoint.y();
        float x1 = sc2.x() - m_offsetPoint.x();
        float y1 = sc2.y() - m_offsetPoint.y();
        r01 = sqrt((double)(x*x + y*y)), a01 = pt1.y();
        r02 = sqrt((double)(x1*x1 + y1*y1)), a02 = pt2.y();
    }
    if (r01 > r02)  //距离调换
    {
        double r = r01;
        r01 = r02;
        r02 = r;
    }
    const double rmin = r01;
    const double rmax = r02;
    const double abgn = a01, aend = a02;


    //保存这些区域变量
    gzrMix = rmin;
    gzrMax = rmax;
    gzaBgn = RADIANTODEGREE(abgn)*10;  //跟回波的角度一致,最大3600
    gzaEnd = RADIANTODEGREE(aend)*10;

    quint32 size = echoviewCircle.size();
    for(int z=0; z<size; z++) {
        int i = echoviewCircle.at(z).x();
        int j = echoviewCircle.at(z).y();

        QPointF ra = lpMouseOpetarion->square_to_rtheta(QPoint(i,1024-j));  //这个square实际时屏幕坐标，i,j为像素坐标，中心点不同且方向不同
        const float r = ra.x(),a = ra.y();
        if(abgn <= aend) {

            if((a > abgn) && (a < aend) && (r > rmin) && (r < rmax)) {
                guardzoneArea.append(QPoint(i, j));
            }
        }else {
            if((((a > abgn)&&(a < M_2PI)) || ((a >= 0)&&(a < aend)))  \
                && (r > rmin) && (r < rmax)) {

                guardzoneArea.append(QPoint(i, j));
            }
        }
    }
}

void OpenGLView::changeGuardColor(quint8 flag)
{
    int i,j;

    if(flag) {  //为1时表示变为背景色
        quint32 echonum=0;  //计算回波点的个数，为0时表示没有目标
        quint32 size = guardzoneArea.size();
        for(int z=0; z<size; z++) {
            i = guardzoneArea.at(z).x();
            j = guardzoneArea.at(z).y();
            if((echoView[j][i] == echoColor))
            {
                echoView[j][i] = echobackColor;  //画背景色
                guardzoneNum.append((quint32)z);  //将位置保存
                echonum++;
            }

        }
        if(echonum ==0) {
            flag_guardZone = 0;  //不再闪烁
            flag_paintBack =0;
            flag_paintEcho =0;
            guardZone_alarm = 0; //不再报警
            extern quint8 closeSpeaking;
            closeSpeaking = 1;

        }
    }else {   //为0时表示变回回波色
        quint32 index;
        quint32 size = guardzoneNum.size();
        for(int z=0; z<size; z++)
        {
            index = guardzoneNum.at(z);
            i = guardzoneArea.at(index).x();
            j = guardzoneArea.at(index).y();

            echoView[j][i] = echoColor;
        }
        //清空数据，下次重新画
        guardzoneNum.clear();

    }
}

void OpenGLView::setAziAdjust(float azi)
{

    MenuConfig.installMenu.aziAdjust = azi;  //回波角度最大3600
    setAziAdjustValue(MenuConfig.installMenu.aziAdjust*10);
}

void OpenGLView::setRngAdjust(float rng)
{
    QByteArray senddata;
    QDataStream in(&senddata, QIODevice::WriteOnly);


    MenuConfig.installMenu.rngAdjust = rng;
    rng_adjust = rng;
     //m_rngStart = (int)(ECHOVIEW_RADIUS * rng_adjust / m_range);

     if(lpInteract) {
         in<<(quint8)170<<(quint8)0<<(quint8)13<<(quint8)0<<(quint8)rng;  //AA000d 0000 --- AA000d 00FF
         if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
         // qDebug()<< "FPGA write rng success!"<< flag;
     }

}

void OpenGLView::setMBSAdjust(quint8 val)
{
    QByteArray senddata;
    QDataStream in(&senddata, QIODevice::WriteOnly);

     if(lpInteract) {
         in<<(quint8)170<<(quint8)0<<(quint8)12<<(quint8)0<<(quint8)val;  //AA000c 0000 --- AA000c 00FA
         if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
         // qDebug()<< "FPGA write mbs success!"<< flag;
     }
}

// 计算扇形区域
void DrawFanPath (QPainter* p, const RTHETA_POINT& pt1, const RTHETA_POINT& pt2)
{
    extern OpenGLView *pView;
    const float rotation = DEGREETORADIAN(pView->rotation());  //返回的是角度，不是弧度
    double r01 = pt1.r(), a01 = pt1.theta();
    double r02 = pt2.r(), a02 = pt2.theta();

    if (r01 > r02)  //距离调换
    {
        double r = r01;
        r01 = r02;
        r02 = r;
    }

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
    p->drawArc(rc2, 90*16-(abgn-rotation)*5760.0/M_2PI, -da*5760.0/M_2PI);
    p->drawLine(sc3.toPoint(), sc4.toPoint());
    p->drawArc(rc1, 90*16-(abgn-rotation)*5760.0/M_2PI, -da*5760.0/M_2PI);

    //qDebug() << sc1.toPoint() << sc2.toPoint() << sc4.toPoint();
}


void OpenGLView::screenCoordinateChanged()
{
    if(MenuConfig.dispMenu.showAis) {
        if(lpAisManage)
            lpAisManage->updateScreenPoint();
    }
    if(m_lpNaviPoint)
        m_lpNaviPoint->updateScreenPoint();
}

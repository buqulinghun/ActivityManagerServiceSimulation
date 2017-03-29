/****************************************************************************
file name: openglview.h
author: wang ming xiao
date: 2015/07/25
comments:  opengl采用16位格式，5-6-5, 绘制像素在接受线程
***************************************************************************/
#ifndef OPENGLVIEW_H
#define OPENGLVIEW_H

#include <QGraphicsView>
#include <QList>
#include <QSemaphore>
#include <QMutexLocker>
#include <QtOpenGL>

#include "openglsence.h"
#include "define.h"
#include "TargetManage/transform.h"
#include "navipoint.h"
#include "guardzone.h"


#define echoViewWidth 1024  //设置内存宽度和长度
#define echoViewHeight 1024
#define RADIUS2 512
#define MAXDISPAZI 3600  //显示角度分割
#define MAXREALAZI 360  //主要在每个角度的量程
#define MAXREALAZI2 720  //CurrentRange的个数

#define CROSSRADIUS  724  //就是与圆心最远的点的长度   512*1.414.....

#define CROSSDIAMETER 1448  //当矩形长度为2048时的最长半径
#define DIAMETER 2048  //放大两倍，因为偏心
#define RADIUS3 1024

#define MAXDEGREE 360

#define MAXPACKETNUM 1250  //一圈的最大包数

//#define OPENGL_RGBA
#define OPENGL_SHORT
//#define OPENGL_BYTE


class OpenGLView : public QGraphicsView
{
Q_OBJECT
public:
    //显示模式
    enum {
        NULL_SCAN = 0,
        A_SCAN,
        B_SCAN,
        NORMAL_SCAN,
        EMPTY_SCAN,
        DELAY_SCAN
    };

    explicit OpenGLView(QWidget *parent = 0);
    ~OpenGLView();





/*****************信号处理参数控制****************************/
public:
    void change_gain(int flag=true);  //修改增益
    void change_restrain(int flag=true);  //修改雨雪抑制
    void change_clutter(int flag=true);  //杂波
    void change_tune(quint8 flag, quint32 val, bool firstflag=false);  //调谐
    //显示的调谐值变化
    void change_tune_disp(quint32 val);
    //更改HL1710变化
    void change_HL1710(int flag);

    //性能检测
    void change_Xingneng(quint8 flag);
    //同频干扰
    void setJam(quint8 flag);

    void setRngringShow(int flag);  //设置距标圈
    void setHeadlineShow(int flag);  //设置船艏线

    //旋转显示视图角度
    void updateViewRotation();
    //设置向上模式
    void setUpMode(quint8 mode, bool forceupdateflag = false);
    //设置运动方式
    void setMotion(quint8 motion);
    //设置偏心
    void setOffset(quint8 ofst);
    //设置偏心坐标
    void setOffsetPoint(quint16 x = RADIUS, quint16 y = RADIUS);
    //检测坐标是否在偏心区域内
    bool isPointInOffArea(const QPoint &p) const;
    //设置各个角度的量程值
    void setCurrentRange();
    //设置尾迹保留时间
    void setTrailTime(int flag);
    //更新保留时间
    void updateDelayTime();
    //回波扩展
    void echoExpand(int flag);

    //本船位置变化
    void ownshipPositionChanged();


    bool isPointDisplay(const PLOTPOSITION& position);

    //天线报警信息处理
    void antennaWarning(quint8 warning);


    int gain() const
    {  return m_gain;   }
    int restrain()  const
    {  return m_restrain;  }
    int clutter()  const
    {   return m_clutter;   }
    int tune()  const
    {  return m_tune;   }


    const quint16 * currentRange() const
    {	return CurrentRange;	}




public:
    // 设置显示模式
    void setScanMode(quint8 mode)
    {    m_scanMode = mode;    }
    quint8 scanMode() const
    {    return m_scanMode;   }

    // 设置当前距离刻度线
    void setCurrentRangeScale(float rngscale)
    {   CurrentRangeScale = rngscale;   }
    // 设置当前方位刻度线
    void setCurrentAzimuthScale(float aziscale)
    {   CurrentAzimuthScale = aziscale;   }
    // 获取当前距离刻度线
    float currentRangeScale() const
    {	return CurrentRangeScale;	}
    // 获取当前方位刻度线
    float currentAzimuthScale() const
    {	return CurrentAzimuthScale;	}

    //设置量程
    void setRange(float rng, float sca);
    //获取量程
    float range() const
    {  return m_range;  }
    //获取像素距离比
    float ration() const
    {  return m_ration;  }
    //设置旋转角度
    void setRotation (float rotation=0.0)
    {   m_rotation = rotation;      }
    // 返回旋转角度---图像旋转
    float rotation() const
    {   return m_rotation;  }
    //设置旋转角度2----外围方位圈角度
    void setRotation2(float rotation=0.0)
    {   m_rotation2 = rotation;   }
    float rotation2() const
    {   return m_rotation2;  }

    // 设置本船艏向
    void setBoatHeading(float head);
    //设置航向
    void setBoatCorse(float course);

    void rangeChanged(bool flag);

    //画p显刻度线
 inline   void drawScanLineP(QPainter *p0);
    //画外围的方位圈
 inline    void drawOuterAzimuthCircle(QPainter* p0);
    //画船艏指示线
 inline   void drawGuildLine(QPainter *p0);
    //画VRM/EBL
 inline   void drawVrmEbl(QPainter *painter);



     void screenCoordinateChanged();
    // 像素距离比变化
   void rationChanged(bool changed=true);



   // 复位导航点索引
   void resetNaviPointIndex()
   {
       m_lpNaviPoint->resetCrntIndex();
   }
   // 获取导航点位置,经纬度位置
   QPointF getNaviPoint(quint8 idx)
   {
       return m_lpNaviPoint->getNaviPoint(idx);
   }

   //设置颜色模式
   void setColorMode(quint8 val)
   {    colorMode = val;   }

   GuardZone* guardzone() const
   {  return m_lpGuardZone;  }


   quint16 m_offset_maxradius;  //最大显示半径
   QPointF m_offsetPoint;  //偏心坐标
private:

   quint8  m_scanMode; // 显示模式
   //当前方位.距离刻度线
   float CurrentRangeScale;
   float CurrentAzimuthScale;
   quint8 echoExpandMode;
   quint8 m_gain_real;


    //信号处理参数
    int m_gain;
    int m_restrain;
    int m_clutter;
    int m_tune;  //调谐值

    int m_keepTime;    //单位为s,保留时间
    quint8 *timeDelay;  //尾迹保留时间
    int m_keepTime_5s;


    QColor m_headingColor;
    QColor m_varlineColor;
    QVector<qreal> m_dashes;



    //导航点设置
    NaviPoint* m_lpNaviPoint;
    GuardZone* m_lpGuardZone;

    quint8 flag_paintBack;
    quint8 flag_paintEcho;

/***********************回波显示相关变量**************************************/
    volatile double radius; //点迹的显示半径
    volatile int x,y;

    volatile double radiusToDisp;  //距离/数据长度比

    float m_range;  //量程
    float m_ration;  //像素距离比

    float m_rotation;  //回波旋转角度
    float m_rotation2;  //刻度盘旋转角度


    qint16 m_offset_x;  //偏心的偏移量
    qint16 m_offset_y;
    quint16 *CurrentRange;


    float m_pixelPerRnguint;  //一个距离单元对应的像素宽度
    QList<qint32> m_pixelIndex[MAXDISPAZI][CROSSDIAMETER];  //像素点索引值 1024*1.414 = 1448, 使用放大2倍的虚拟视图索引
    qint32 *m_screenIndex;	// 虚拟视图索引到屏幕视图索引映射表
    qint32 m_offset_index;  //偏心虚拟索引
    QPointF theta_offset;

    quint8 mouseShapeChanged;  //绘制警戒区域鼠标形状改变
    quint8 colorMode;  //颜色模式，0：单色 1：多色



/***********************回波绘制相关*************************/
   public:
       void setScene(OpenGLSence *pScene);
       //设置触发的回波数据
       void setEchoData(ECHODATA& m_data);
       //设置背景颜色
       void setBackColor(quint32 color);
       //设置回波颜色
       void setForeColor(quint32 color);
       //设置各类线的颜色,船艏线，活动线
       void setLineColor(quint32 head, quint32 var);

       //设置跟踪鼠标位置
       void enableMouseTracking(bool enable = true)
       {   m_mouseTracking = enable ? 1 : 0;  }
       //设置鼠标形状
       void setMouseShape(quint8 falg);

       void initEchoView(void);
       void resetEchoView(void);

       //警戒区域选择之后计算所在像素点
       void setGuardZonePixel();

       //设置警戒区域关闭
       void setGuardZoneText()
       {
           gzaBgn = 0;
           gzaEnd = 0;
           gzrMax = 0;
           gzrMix = 0;
           flag_guardZone = 0;
       }
       //清空警戒区域缓存
       void setGuardZoneClear()
       {
           guardzoneArea.clear();
           guardzoneNum.clear();
       }


       //设置方位调节
       void setAziAdjust(float azi);
       //设置距离调节
       void setRngAdjust(float rng);
       //MBS调节
       void setMBSAdjust(quint8 val);


       void setAziAdjustValue(float val)
       {   azi_adjust = (int)val;   }
       void setRngAdjustValue(float val)
        {    rng_adjust = val;  }

   protected:
       void mousePressEvent(QMouseEvent *event);
       void mouseMoveEvent(QMouseEvent *event);


       //鼠标按键处理
       void lButtonDownProcess (int x, int y);
       // 视图操作处理
       inline void viewOperate(const int x, const int y);


       //opengl相关函数
       void InitGL(void);
       void ResizeGL(int width, int height);
       void PaintGL(void);

       void drawBackground(QPainter *painter, const QRectF &rect);
       void drawForeground(QPainter *painter, const QRectF &rect);




   signals:
       void painting(void);  //绘图信号
   public slots:
       void receivedpaint(void);  //绘图信号处理槽
       void updateView(void);
       void updateMouseOperate(QMouseEvent *event)
       {   mousePressEvent(event);   }



   private:
#ifdef OPENGL_RGBA

       quint32 backColor;  //其他部分背景色
       quint32 echobackColor;  //回波显示背景色
       quint32 echoColor;  //回波颜色
       quint32 echoColor2;  //回波2颜色
       quint32 echoColor3; //回波3颜色
       quint32 echoclutterColor;  //杂波颜色
       quint32 echotrailColor;  //尾迹色
#endif
#ifdef OPENGL_BYTE
       quint8 backColor;  //其他部分背景色
       quint8 echobackColor;  //回波显示背景色
       quint8 echoColor;  //回波颜色
       quint8 echoColor2;  //回波2颜色
       quint8 echoColor3; //回波3颜色
       quint8 echoclutterColor;  //杂波颜色
       quint8 echotrailColor;  //尾迹色
#endif
#ifdef OPENGL_SHORT
       quint16 backColor;  //其他部分背景色
       quint16 echobackColor;  //回波显示背景色
       quint16 echoColor;  //回波颜色
       quint16 echoColor2;  //回波2颜色
       quint16 echoColor3; //回波3颜色
       quint16 echoclutterColor;  //杂波颜色
       quint16 echotrailColor;  //尾迹色
#endif

       //跟踪鼠标标志
       quint8 m_mouseTracking;

       QList<QPoint> echoviewCircle;   //回波背景显示
       QList<QPoint> echoviewOutCircle;   //回波外部背景

       QList<QPoint> guardzoneArea;   //警戒区域
       QList<quint32> guardzoneNum;  //回波点数在警戒区域

       //闪烁警戒区域变量
       int gzrMix,gzrMax;  //半径
       int gzaBgn, gzaEnd;  //角度

       quint8 flag_guardZone;  //闪烁标志,当为1时表示有警报，需要闪烁，为2的倍数时显示回波，否则显示背景，闪烁3次就到6，然后显示回波不变

       void changeGuardColor(quint8 flag);  //将警戒区域更改颜色


   /***********************保存回波数据量的变量**************************************/
       //保存暂时回波绘图数据
       QSemaphore m_echoReceived;    //互斥量
       QMutex m_echoReceivedMutex;   //同步锁
       QMutex m_mutex;
      // QList<ECHODATA> m_echoDataList;   //保存一圈回波数据
       ECHODATA m_echoDataList[MAXPACKETNUM];  //保存回波数据,最大420*2.5
#ifdef OPENGL_RGBA
       unsigned int  echoView[echoViewWidth ][ echoViewHeight];
#endif
#ifdef OPENGL_SHORT
       unsigned short echoView[echoViewWidth ][ echoViewHeight];      //保存opengl绘图的内存数据
#endif
#ifdef OPENGL_BYTE
       unsigned char echoView[echoViewWidth ][ echoViewHeight];
#endif
       quint8 flag_break;
       quint8 flag_newCircle;  //新的一圈标志
       quint8 flag_next_Circle;  //新的一圈第一个山行标志


   /***********************数据包处理变量设置**************************************/
        quint16 m_paintNum;  //当前绘图的数据索引
        quint8 lastPaint;  //绘最后一帧图像标志

        quint32 m_startPacketNum;   //绘图起始数据包索引
        quint32 m_endPacketNum;    //终止数据包索引
        quint32 m_lastPacketNum;    //上一次数据包索引
        void drawAngle(const quint32 &startPacket, const quint32 &endPacket, bool last_flag = false);

       //方位调节值
       int azi_adjust;
       float rng_adjust;
       int   m_rngStart;  //实际屏幕距离调节值




};

#endif // OPENGLVIEW_H

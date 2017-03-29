#ifndef RADARITEM_H
#define RADARITEM_H

#include <QGraphicsItem>

#include "define.h"



#define MAXDEGREE 360
#define MAXPACKETNUM  1750          // 一圈的最大包数
#define MAXRADIUS 800   //雷达显示直径
#define RADIUS 400

#define MAXDISPAZI 3600  //显示角度分割
#define CROSSDIAMETER 1414  //当矩形长度为1600时的最长半径，像素点索引值 800*1.414 = 1132, 使用放大2倍的虚拟视图索引
#define DIAMETER 1600  //放大两部的矩形
#define MAXREALAZI2 360


class RadarItem : public QGraphicsItem
{

public:
    RadarItem();
    ~RadarItem();


    void initEchoView();  //初始化雷达图像缓存部分
    void setCurrentRange();    //设置各个角度的量程值
    void setRange(float rng, float sca);  //设置量程


    //画p显刻度线
   inline   void drawScanLineP(QPainter *p0);
    //画外围的方位圈
   inline    void drawOuterAzimuthCircle(QPainter* p0);
    //画船艏指示线
   inline   void drawGuildLine(QPainter *p0);
    //画VRM/EBL
   inline   void drawVrmEbl(QPainter *painter);

    void drawEchoData(void);   //绘制雷达回波
    void setEchoData(ECHODATA& m_data);    //设置最新雷达数据


    //设置旋转角度
    void setRotation (float rotation=0.0)
    {   m_rotation = rotation;      }
    // 返回旋转角度---图像旋转
    float rotation() const
    {   return m_rotation;  }
    //设置旋转角度2----外围方位圈角度
    void setRotation2(float rotation=0.0)
    {   m_rotation2 = rotation;      }
    float rotation2() const
    {   return m_rotation2;  }

    //设置回波颜色
    void setEchoColor(quint8 flag)
    {        color_flag = flag;     refreshEchoColor();       }
    void setColorAlpha(quint8 alpha)
    {     color_alpha = alpha;    refreshEchoColor();     }
    //设置偏心状态
    void setOffsetStatus(quint8 flag);


    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

//protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);



private:
    /***********************回波显示相关变量**************************************/
        volatile double radius; //点迹的显示半径
        volatile int x,y;

        volatile double radiusToDisp;  //距离/数据长度比

        float m_range;  //量程
        float m_ration;  //像素距离比

        float m_rotation;  //回波旋转角度
        float m_rotation2;  //刻度盘旋转角度

        quint8 offset_flag;    //偏心标志
        qint16 m_offset_x;  //偏心的偏移量
        qint16 m_offset_y;
        QPointF m_offsetPoint;
        quint16 m_offset_maxradius;  //最大显示半径
        quint16 *CurrentRange;


        float m_pixelPerRnguint;  //一个距离单元对应的像素宽度
        QList<qint32> m_pixelIndex[MAXDISPAZI][CROSSDIAMETER];
        qint32 *m_screenIndex;	// 虚拟视图索引到屏幕视图索引映射表
        qint32 m_offset_index;  //偏心虚拟索引



        quint8 color_flag;   //颜色选择
        quint8 color_alpha;   //颜色透明度

        quint32 echoColor;  //回波颜色
        quint32 secondEchoColor;   //黄色回波的第二种颜色
        quint32 echoclutterColor;  //杂波颜色
        quint32 echobackColor;  //回波显示背景色


        /***********************保存回波数据量的变量**************************************/
            //保存暂时回波绘图数据
            QMutex m_echoReceivedMutex;   //同步锁
            QList<ECHODATA> m_lastEchoData;   //最新雷达数据

            //方位调节值
            int azi_adjust;
            int   m_rngStart;  //实际屏幕距离调节值

            unsigned int  echoView[MAXRADIUS * MAXRADIUS];   //雷达图像缓存
            QList<unsigned int> echoviewCircle;   //回波显示区域
             QList<unsigned int> echoviewOutCircle;  //矩形外圈


            quint32 getColor(quint8 alpha, quint32 color)
            {
                quint32 alp = (quint32)alpha<<24;
                quint32 col = alp | color;
                return col;
            }
             void refreshEchoColor()   //重新设置颜色
             {
                  //0：green   1:yellow   2:red
                 switch(color_flag) {
                 case 0:
                     echoColor = getColor(color_alpha, (quint32)0x0000ff00);
                     secondEchoColor = getColor(color_alpha, (quint32)0x0000ff33);
                     echoclutterColor = getColor(color_alpha, (quint32)0x0000ff66);
                     break;
                 case 1:
                     echoColor = getColor(color_alpha, (quint32)0x00ffff00);
                     secondEchoColor = getColor(color_alpha, (quint32)0x00ffff33);
                     echoclutterColor = getColor(color_alpha, (quint32)0x00ffff66);
                     break;
                 case 2:
                     echoColor = getColor(color_alpha, (quint32)0x00ff0000);
                     secondEchoColor = getColor(color_alpha, (quint32)0x00ff0033);
                     echoclutterColor = getColor(color_alpha, (quint32)0x00ff0066);
                     break;
                 default:
                     break;
                 }
             }
    
};

#endif // RADARITEM_H

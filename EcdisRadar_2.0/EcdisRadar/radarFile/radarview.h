#ifndef RADARVIEW_H
#define RADARVIEW_H

#include "transform_global.h"
#include "transform.h"

#include <QPainter>



#define MAXDEGREE 360



class CRadarView : public CTransform
{
public:
    CRadarView();
    virtual ~CRadarView();


    //显示模式
    enum {
        NULL_SCAN = 0,
        A_SCAN,
        B_SCAN,
        NORMAL_SCAN,
        EMPTY_SCAN,
        DELAY_SCAN
    };

public:

    // 设置显示模式
    void setScanMode(quint8 mode)
    {
           m_scanMode = mode;
    }

    quint8 scanMode() const
    {    return m_scanMode;   }

    //设置量程
    void setRange(float rng)
    {  CTransform::setRange(rng);    }




    // 设置当前距离刻度线
    void setCurrentRangeScale(float rngscale)
    {   CurrentRangeScale = rngscale;   }
    // 设置当前方位刻度线
    void setCurrentAzimuthScale(float aziscale)
    {   CurrentAzimuthScale = aziscale;   }
    // 设置距离刻度线的颜色
    void setKm100Color(quint32 color)
    {   Km100Color = color; }
    void setKm50Color(quint32 color)
    {   Km50Color = color; }
    void setKm10Color(quint32 color)
    {   Km10Color = color; }
    // 设置方位刻度线的颜色
    void setDegree30Color(quint32 color)
    {   Degree30Color = color; }
    void setDegree10Color(quint32 color)
    {   Degree10Color = color; }
    void setDegree5Color(quint32 color)
    {   Degree5Color = color; }
    // 获取当前距离刻度线
    float currentRangeScale() const
    {	return CurrentRangeScale;	}
    // 获取当前方位刻度线
    float currentAzimuthScale() const
    {	return CurrentAzimuthScale;	}


    /*屏幕坐标转换成直角坐标*/
    SQUARE_POINT screentosquare_view(const SCREEN_POINT& screen_point);
    /*直角坐标转换成屏幕坐标*/
    SCREEN_POINT squaretoscreen_view(const SQUARE_POINT& square_point);
    /*计算扇形区域*/
    QPainterPath calculateFanPath (const RTHETA_POINT& rtheta1, const RTHETA_POINT& rtheta2);



    //绘制方位刻度线
    void paint(QPainter *p);

    //设置裁剪区域
    void setClipPath(QPainter *p);



protected:
    //画p显刻度线
    void drawScanLineP(QPainter *p);

    // 画外围的方位圈
    void drawOuterAzimuthCircle(QPainter* p0);
    // 计算雷达中心到视图的最大、最小距离
    void radarCenterToViewRange(float& rmin, float& rmax);
    // 计算指定点到圆周每度的距离
    void calculateCenterToOutCircleRange();

    // 像素距离比变化,虚函数，必须在该子类中实现才行
    virtual void rationChanged(bool changed=true);

private:

    quint8  m_scanMode; // 显示模式



    //当前方位.距离刻度线
    float CurrentRangeScale;
    float CurrentAzimuthScale;
    // 距离刻度线颜色
    quint32 Km100Color, Km50Color, Km10Color;
     // 方位刻度线颜色
    quint32 Degree30Color, Degree10Color, Degree5Color;


    //B显参数
    quint16 m_bscanRangeStart;   //B显起始距离
    quint16 m_bscanRangeWidth;   //B显距离宽度
    quint16 m_bscanAzimuthStart;  //B显起始方位（度）
    quint16 m_bscanAzimuthWidth;  //B显方位宽度（度）
    float   m_bscanRangeRation; // B显距离轴上,象素/距离比
    float   m_bscanAzimuthRation;//B显方位轴上,象素/方位比

    quint16 m_bscanRangeStop;  //B显结束距离
    quint16 m_bscanAzimuthStop;  //B显结束方位


     // P显显示控制
     quint8  m_offsetFlag:1;   // 偏心标志
     quint8  m_scrollFlag:1;   // 漫游标志
     quint8  m_zoomFlag  :1;   // 缩放标志
     quint8  m_arscrollFlag:2; // AR显漫游标志 0:无 1：A显漫游 2：R显漫游
     quint8  m_reserved  :3;

     float   m_scanKiloRange;
     qint16  m_scanPixelRange;
     quint8  m_offsetMode;
     int     m_offsetX, m_offsetY;
     float   m_offsetPixel;



     // 用于记录正常状态时的量程
     int   MaxRangeForNormalScan;

     float CurrentRange[MAXDEGREE];  //每个方位的量程

private:
     //保存当前的量程
     void saveMaxRange()
     {
         if(MaxRangeForNormalScan == 0)
             MaxRangeForNormalScan = range();
     }
     //恢复到原来的量程
     void restoreMaxRange()
     {
         if(MaxRangeForNormalScan) {
             CTransform::setRange(MaxRangeForNormalScan);
             MaxRangeForNormalScan = 0;
         }
     }

};


#endif // RADARVIEW_H

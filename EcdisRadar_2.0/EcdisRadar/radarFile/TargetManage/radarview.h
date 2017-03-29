#ifndef RADARVIEW_H
#define RADARVIEW_H

#include "TargetManage_global.h"
#include "transform.h"

#include <QtGui/QPainter>

#define MAXDEGREE 360

class TARGETMANAGESHARED_EXPORT CRadarView : public CTransform
{
public:
    // 显示模式
    enum {
        NULL_SCAN = 0,
        A_SCAN,
        B_SCAN,
        NORMAL_SCAN,
        EMPTY_SCAN,
        DELAY_SCAN,
    };

    CRadarView();
    virtual ~CRadarView();

public:
    // 视图索引号
    quint8 viewIndex() const
    {   return m_viewIndex; }

    // 设置裁减区域
    void setClipPath(QPainter* p);

    // 设置显示模式
    void setScanMode(quint8 mode)
    {
        // 从偏心放大漫游状态恢复到正常状态
        if(m_offsetFlag || m_scrollFlag || m_zoomFlag)
            restoreToPPINormal();

        m_scanMode = mode;

        if(m_scanMode == EMPTY_SCAN)
            setScanKiloRange(range()/5);
        else if(m_scanMode == A_SCAN)
        {
            setRScanRange(0, range());
            setAScanRange(0, range()/10);
        }
        else if(m_scanMode == B_SCAN)
        {
            updateBScanParam();
        }
    }
	
	/*设置量程*/
    void setRange (float rng);

    // 设置延迟距离
    void setDelayRange(float rng)
    {
        if(m_scanMode == DELAY_SCAN)
            setScanKiloRange(rng);
    }

    // 设置偏心模式
    void setOffsetMode(int mode)
    {   m_offsetMode = mode;    }

    // 偏心处理
    void offsetProcess(int x, int y);
    // 区域放大
    void zoomArea(const QRect& rc);
    // 漫游处理
    bool viewMove(int dx, int dy);

    quint8 scanMode() const
    {   return m_scanMode;		}
	bool isOffset() const
	{	return m_offsetFlag;	}
	bool isZoom() const
	{	return m_zoomFlag;		}
	bool isScroll() const
	{	return m_scrollFlag;	}
    float scanKiloRange() const
    {   return m_scanKiloRange; }
    int scanPixelRange() const
    {   return m_scanPixelRange;    }

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
    
    // 设置B显距离参数
    void setBScanRange(quint16 rngStart, quint16 rngWidth=0);
    // 设置B显方位参数
    void setBScanAzimuth(quint16 aziStart, quint16 aziWidth=0);

    // 更新B显参数
    void updateBScanParam()
    {
        setBScanRange(bscanRangeStart(), range());
        m_bscanRangeRation = ((float)m_screenEnvelope.height() / (float)m_bscanRangeWidth);
        m_bscanAzimuthRation = ((float)m_screenEnvelope.width() / (float)m_bscanAzimuthWidth);
    }

    // 获取B显参数
    quint16 bscanRangeStart() const
    {   return m_bscanRangeStart;   }
    quint16 bscanRangeWidth() const
    {   return m_bscanRangeWidth;   }
    quint16 bscanAzimuthStart() const
    {   return m_bscanAzimuthStart; }
    quint16 bscanAzimuthWidth() const
    {   return m_bscanAzimuthWidth; }
    float bscanRangeRation() const
    {   return m_bscanRangeRation;  }
    float bscanAzimuthRation() const
    {   return m_bscanAzimuthRation;    }

    // 设置AR显示标志
    void setARScanFlag(int flag)
    {   m_arflag = flag;    }
	int ARScanFlag() const
	{	return m_arflag;	}

    // 设置AR显区域
    void setARScanRect(const QRect& arect, const QRect& rrect)
    {
        m_rscanRect = rrect;
        m_ascanRect = arect;
    }

    // 获取A显区域
    QRect AScanRect() const
    {   return m_ascanRect; }
    // 获取R显区域
    QRect RScanRect() const
    {   return m_rscanRect; }

    // 设置R显距离范围
    void setRScanRange(int start, int width)
    {
        m_rscanRangeStart = start;
        m_rscanRangeWidth = width;
    }
    // 获取R显距离范围
    void getRScanRange(int& start, int& width)
    {
        start = m_rscanRangeStart;
        width = m_rscanRangeWidth;
    }
    // 设置A显距离范围
    void setAScanRange(int start, int width)
    {
        m_ascanRangeStart = start;
        m_ascanRangeWidth = width;
    }
    // 获取A显距离范围
    void getAScanRange(int& start, int& width)
    {
        start = m_ascanRangeStart;
        width = m_ascanRangeWidth;
    }

    void startAScanScroll(const QPoint& pt);

    bool isPointDisplay(const PLOTPOSITION& position);

public:
    virtual void paint(QPainter* p);

protected:
    // 像素距离比变化
    virtual void rationChanged(bool changed=true);

    // 设置需要延迟的距离
    void setScanKiloRange(float rng)
    {
        m_scanKiloRange = rng;
        m_scanPixelRange = rng * ration();
    }
    // 画A显刻度线
    void drawScanLineA(QPainter* p);
    // 画B显刻度线
    void drawScanLineB(QPainter* p);
    // 画P显刻度线
    void drawScanLineP(QPainter* p);
    // 辅助函数，用于绘制A显刻度线
    void drawScaleLineAScan (QPainter* p0, const QRect& rect, int rngstart, int rngwidth, bool rngbase);
    // 画外围的方位圈
    void drawOuterAzimuthCircle(QPainter* p0);
    // 计算雷达中心到视图的最大、最小距离
    void radarCenterToViewRange(float& rmin, float& rmax);
    // 计算指定点到圆周每度的距离
    void calculateCenterToOutCircleRange();

    // 恢复到PPI正常状态
    void restoreToPPINormal();

private:
    static quint8  m_totalViews;
    quint8  m_viewIndex;

    quint8  m_scanMode; // 显示模式

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

    // B显参数
    quint16 m_bscanRangeStart;  // B显起始距离
    quint16 m_bscanRangeWidth;  // B显距离宽度
    quint16 m_bscanAzimuthStart;// B显起始方位(度)
    quint16 m_bscanAzimuthWidth;// B显方位宽度(度)
    float   m_bscanRangeRation; // B显距离轴上,象素/距离比
    float   m_bscanAzimuthRation;//B显方位轴上,象素/方位比

    quint16 m_bscanRangeStop;  // B显结束距离
    quint16 m_bscanAzimuthStop;// B显结束方位

    // AR显参数
    int     m_arflag;   // AR显标志,bit1:显示A显,bit2:显示R显
    int     m_rscanRangeStart, m_rscanRangeWidth;
    int     m_ascanRangeStart, m_ascanRangeWidth;
    QRect   m_rscanRect, m_ascanRect;

    // 当前方位、距离刻度线
    float CurrentRangeScale;
    float CurrentAzimuthScale;
    // 距离刻度线颜色
    quint32 Km100Color, Km50Color, Km10Color;
    // 方位刻度线颜色
    quint32 Degree30Color, Degree10Color, Degree5Color;

private:
    // 保存当前量程
    void saveMaxRange()
    {
        if(MaxRangeForNormalScan == 0)
            MaxRangeForNormalScan = range();
    }

    // 恢复到原来的量程
    void restoreMaxRange()
    {
        if(MaxRangeForNormalScan)
        {
            CTransform::setRange(MaxRangeForNormalScan);
            MaxRangeForNormalScan = 0;
        }
    }

    // 用于记录正常状态时的量程
    int   MaxRangeForNormalScan;

    float CurrentRange[MAXDEGREE];
};

#endif // RADARVIEW_H

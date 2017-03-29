#ifndef RADARVIEW_H
#define RADARVIEW_H

#include "TargetManage_global.h"
#include "transform.h"

#include <QtGui/QPainter>

#define MAXDEGREE 360

class TARGETMANAGESHARED_EXPORT CRadarView : public CTransform
{
public:
    // ��ʾģʽ
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
    // ��ͼ������
    quint8 viewIndex() const
    {   return m_viewIndex; }

    // ���òü�����
    void setClipPath(QPainter* p);

    // ������ʾģʽ
    void setScanMode(quint8 mode)
    {
        // ��ƫ�ķŴ�����״̬�ָ�������״̬
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
	
	/*��������*/
    void setRange (float rng);

    // �����ӳپ���
    void setDelayRange(float rng)
    {
        if(m_scanMode == DELAY_SCAN)
            setScanKiloRange(rng);
    }

    // ����ƫ��ģʽ
    void setOffsetMode(int mode)
    {   m_offsetMode = mode;    }

    // ƫ�Ĵ���
    void offsetProcess(int x, int y);
    // ����Ŵ�
    void zoomArea(const QRect& rc);
    // ���δ���
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

    // ���õ�ǰ����̶���
    void setCurrentRangeScale(float rngscale)
    {   CurrentRangeScale = rngscale;   }
    // ���õ�ǰ��λ�̶���
    void setCurrentAzimuthScale(float aziscale)
    {   CurrentAzimuthScale = aziscale;   }
    // ���þ���̶��ߵ���ɫ
    void setKm100Color(quint32 color)
    {   Km100Color = color; }
    void setKm50Color(quint32 color)
    {   Km50Color = color; }
    void setKm10Color(quint32 color)
    {   Km10Color = color; }    
    // ���÷�λ�̶��ߵ���ɫ
    void setDegree30Color(quint32 color)
    {   Degree30Color = color; }
    void setDegree10Color(quint32 color)
    {   Degree10Color = color; }
    void setDegree5Color(quint32 color)
    {   Degree5Color = color; }

    // ��ȡ��ǰ����̶���
    float currentRangeScale() const
    {	return CurrentRangeScale;	}
    // ��ȡ��ǰ��λ�̶���
    float currentAzimuthScale() const
    {	return CurrentAzimuthScale;	}

    /*��Ļ����ת����ֱ������*/
    SQUARE_POINT screentosquare_view(const SCREEN_POINT& screen_point);
    /*ֱ������ת������Ļ����*/
    SCREEN_POINT squaretoscreen_view(const SQUARE_POINT& square_point);
    /*������������*/
    QPainterPath calculateFanPath (const RTHETA_POINT& rtheta1, const RTHETA_POINT& rtheta2);
    
    // ����B�Ծ������
    void setBScanRange(quint16 rngStart, quint16 rngWidth=0);
    // ����B�Է�λ����
    void setBScanAzimuth(quint16 aziStart, quint16 aziWidth=0);

    // ����B�Բ���
    void updateBScanParam()
    {
        setBScanRange(bscanRangeStart(), range());
        m_bscanRangeRation = ((float)m_screenEnvelope.height() / (float)m_bscanRangeWidth);
        m_bscanAzimuthRation = ((float)m_screenEnvelope.width() / (float)m_bscanAzimuthWidth);
    }

    // ��ȡB�Բ���
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

    // ����AR��ʾ��־
    void setARScanFlag(int flag)
    {   m_arflag = flag;    }
	int ARScanFlag() const
	{	return m_arflag;	}

    // ����AR������
    void setARScanRect(const QRect& arect, const QRect& rrect)
    {
        m_rscanRect = rrect;
        m_ascanRect = arect;
    }

    // ��ȡA������
    QRect AScanRect() const
    {   return m_ascanRect; }
    // ��ȡR������
    QRect RScanRect() const
    {   return m_rscanRect; }

    // ����R�Ծ��뷶Χ
    void setRScanRange(int start, int width)
    {
        m_rscanRangeStart = start;
        m_rscanRangeWidth = width;
    }
    // ��ȡR�Ծ��뷶Χ
    void getRScanRange(int& start, int& width)
    {
        start = m_rscanRangeStart;
        width = m_rscanRangeWidth;
    }
    // ����A�Ծ��뷶Χ
    void setAScanRange(int start, int width)
    {
        m_ascanRangeStart = start;
        m_ascanRangeWidth = width;
    }
    // ��ȡA�Ծ��뷶Χ
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
    // ���ؾ���ȱ仯
    virtual void rationChanged(bool changed=true);

    // ������Ҫ�ӳٵľ���
    void setScanKiloRange(float rng)
    {
        m_scanKiloRange = rng;
        m_scanPixelRange = rng * ration();
    }
    // ��A�Կ̶���
    void drawScanLineA(QPainter* p);
    // ��B�Կ̶���
    void drawScanLineB(QPainter* p);
    // ��P�Կ̶���
    void drawScanLineP(QPainter* p);
    // �������������ڻ���A�Կ̶���
    void drawScaleLineAScan (QPainter* p0, const QRect& rect, int rngstart, int rngwidth, bool rngbase);
    // ����Χ�ķ�λȦ
    void drawOuterAzimuthCircle(QPainter* p0);
    // �����״����ĵ���ͼ�������С����
    void radarCenterToViewRange(float& rmin, float& rmax);
    // ����ָ���㵽Բ��ÿ�ȵľ���
    void calculateCenterToOutCircleRange();

    // �ָ���PPI����״̬
    void restoreToPPINormal();

private:
    static quint8  m_totalViews;
    quint8  m_viewIndex;

    quint8  m_scanMode; // ��ʾģʽ

    // P����ʾ����
    quint8  m_offsetFlag:1;   // ƫ�ı�־
    quint8  m_scrollFlag:1;   // ���α�־
    quint8  m_zoomFlag  :1;   // ���ű�־
    quint8  m_arscrollFlag:2; // AR�����α�־ 0:�� 1��A������ 2��R������
    quint8  m_reserved  :3;

    float   m_scanKiloRange;
    qint16  m_scanPixelRange;
    quint8  m_offsetMode;
    int     m_offsetX, m_offsetY;
    float   m_offsetPixel;

    // B�Բ���
    quint16 m_bscanRangeStart;  // B����ʼ����
    quint16 m_bscanRangeWidth;  // B�Ծ�����
    quint16 m_bscanAzimuthStart;// B����ʼ��λ(��)
    quint16 m_bscanAzimuthWidth;// B�Է�λ���(��)
    float   m_bscanRangeRation; // B�Ծ�������,����/�����
    float   m_bscanAzimuthRation;//B�Է�λ����,����/��λ��

    quint16 m_bscanRangeStop;  // B�Խ�������
    quint16 m_bscanAzimuthStop;// B�Խ�����λ

    // AR�Բ���
    int     m_arflag;   // AR�Ա�־,bit1:��ʾA��,bit2:��ʾR��
    int     m_rscanRangeStart, m_rscanRangeWidth;
    int     m_ascanRangeStart, m_ascanRangeWidth;
    QRect   m_rscanRect, m_ascanRect;

    // ��ǰ��λ������̶���
    float CurrentRangeScale;
    float CurrentAzimuthScale;
    // ����̶�����ɫ
    quint32 Km100Color, Km50Color, Km10Color;
    // ��λ�̶�����ɫ
    quint32 Degree30Color, Degree10Color, Degree5Color;

private:
    // ���浱ǰ����
    void saveMaxRange()
    {
        if(MaxRangeForNormalScan == 0)
            MaxRangeForNormalScan = range();
    }

    // �ָ���ԭ��������
    void restoreMaxRange()
    {
        if(MaxRangeForNormalScan)
        {
            CTransform::setRange(MaxRangeForNormalScan);
            MaxRangeForNormalScan = 0;
        }
    }

    // ���ڼ�¼����״̬ʱ������
    int   MaxRangeForNormalScan;

    float CurrentRange[MAXDEGREE];
};

#endif // RADARVIEW_H

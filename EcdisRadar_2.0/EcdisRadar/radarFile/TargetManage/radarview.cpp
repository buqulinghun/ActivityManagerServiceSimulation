#include "radarview.h"

#include <math.h>
#include <QtCore/QtDebug>

float EachDegreeSin[360], EachDegreeCos[360];

#define ASCAN_ASCROLL 1
#define ASCAN_RSCROLL 2
#define NAVI_RADAR	1

#if NAVI_RADAR
#define SEQUENTIAL_OFFSET 1
#else
#define SEQUENTIAL_OFFSET 0
#endif

///////////////////////////////////////////////////////////////////////////////////
//
// class CRadarView implement
// author : mo flying
// date: 201004012
//
//////////////////////////////////////////////////////////////////////////////////
quint8 CRadarView::m_totalViews = 0;

CRadarView::CRadarView() :
    CTransform()
{
    m_viewIndex = m_totalViews;
    m_totalViews ++;

    m_scanMode = NULL_SCAN;
	m_arflag = 0x0;

    m_offsetFlag = 0;
    m_scrollFlag = 0;
    m_zoomFlag = 0;

    m_scanKiloRange = 0;
    m_scanPixelRange = 0;

    m_offsetMode = 0;
    m_offsetX = m_offsetY = 0;

    m_bscanRangeStart = 0;
    m_bscanRangeWidth = 0;
    m_bscanAzimuthStart = 0;
    m_bscanAzimuthWidth = 0;
    m_bscanRangeRation = 0;
    m_bscanAzimuthRation = 1;

    m_rscanRangeStart = m_rscanRangeWidth = 0;
    m_ascanRangeStart = m_ascanRangeWidth = 0;
    m_arscrollFlag = 0;

    CurrentRangeScale = 10;
    CurrentAzimuthScale = 5;

    Km100Color = QRGB(96,96,96);
    Km50Color = QRGB(72,72,72);
    Km10Color = QRGB(48,48,48);

    Degree30Color = QRGB(96,96,96);
    Degree10Color = QRGB(72,72,72);
    Degree5Color = QRGB(48,48,48);

    MaxRangeForNormalScan = 0;

    for(int i=0; i<360; i++)
    {
        const float radian = DEGREETORADIAN(i);
        EachDegreeSin[i] = SIN(radian);
        EachDegreeCos[i] = COS(radian);
    }
}

CRadarView::~CRadarView()
{
    m_totalViews --;
}

void CRadarView::rationChanged(bool changed)
{  
	if(!changed)	return;

    switch(m_scanMode)
    {
		case NORMAL_SCAN:
		{
			// ���½���ƫ�Ĳ���
			if(m_offsetFlag)
				offsetProcess(m_offsetX, m_offsetY);
			break;
		}

        case EMPTY_SCAN:
        {
            setScanKiloRange(range()/5);
            break;
        }
        case DELAY_SCAN:
        {
            // ���¼����ӳٵ�����
            m_scanPixelRange = m_scanKiloRange * ration();
            break;
        }
        case A_SCAN:
        {
            m_rscanRangeWidth = range();
            break;
        }
        case B_SCAN:
        {
            updateBScanParam();
            break;
        }
    }
}

/*��������*/
void CRadarView::setRange (float rng)
{
	//CTransform::setRange(m_offsetFlag ? rng/2 : rng);
	CTransform::setRange(rng);
}

// ƫ�Ĵ���
void CRadarView::offsetProcess(int x, int y)
{
    if(m_scanMode == A_SCAN || m_scanMode == B_SCAN)
        return;

    const bool offsetFlag = m_offsetFlag;
	const float rotate_angle = rotation();

    // �ָ���PPI����״̬
    restoreToPPINormal();
	setRotation(0);

#if (!SEQUENTIAL_OFFSET)
    if(!offsetFlag)
#endif
    {
        m_offsetX = x;
        m_offsetY = y;

        const QPoint pt0 = screenEnvelope().center();
        const int dx = m_offsetX - pt0.x(), dy = m_offsetY - pt0.y();
        m_offsetPixel = sqrt((double)(dx*dx+dy*dy));

        if(m_offsetMode)
        {
            x = 2*pt0.x() - x;
            y = 2*pt0.y() - y;
        }

#if (!NAVI_RADAR)
        saveMaxRange();
        CTransform::setRange(range()/2);
#endif
        SCREEN_POINT sc = {x, y};
        setCenterSquare(screen_to_square(sc));
        //setCenterScreen(QPoint(x,y));

		setRotation(rotate_angle);
        m_offsetFlag = true;

        calculateCenterToOutCircleRange();
    }
}

// ����Ŵ�
void CRadarView::zoomArea(const QRect& rc)
{
    if(m_scanMode == A_SCAN || m_scanMode == B_SCAN)
        return;

    // �ָ���PPI����״̬
    if(m_scanMode == DELAY_SCAN || m_scanMode == EMPTY_SCAN || m_offsetFlag)
        restoreToPPINormal();

    saveMaxRange();

    const QPoint sc0 = rc.center();
    SCREEN_POINT sc = {sc0.x(), sc0.y()};
    setCenterSquare(screen_to_square (sc));

    // �������������̺�ת��ϵ��
    const int r = qMax (rc.width(), rc.height()) / 2;
    CTransform::setRange(r / ration());

    m_zoomFlag = true;
}

void CRadarView::startAScanScroll(const QPoint& pt)
{
    if(scanMode() != A_SCAN)
        return;

    if(m_ascanRect.contains(pt))
        m_arscrollFlag = ASCAN_ASCROLL;
    else if(m_rscanRect.contains(pt))
        m_arscrollFlag = ASCAN_RSCROLL;
    else
        m_arscrollFlag = 0;
}

// ���δ���
bool CRadarView::viewMove(int dx, int dy)
{
    bool result = false;
    switch(m_scanMode)
    {
    case NORMAL_SCAN:
    {
        SQUARE_POINT sq = squareCenter();
        const float sqx = sq.x() - (float)dx / ration();
        const float sqy = sq.y() + (float)dy / ration();
        if (sqrt(sqx*sqx+sqy*sqy) < m_rangeMax - range())
        {
            sq.setPoint(sqx, sqy);
            setCenterSquare(sq);
            m_scrollFlag = 1;
            result = true;
        }
        break;
    }

    case A_SCAN:
    {
        // �ƶ�A����ʼ����
        if(m_arscrollFlag == ASCAN_ASCROLL)
        {   // A������
            const int dr = dx * m_ascanRangeWidth / m_ascanRect.width();
            if(qAbs(dr) > 0)
            {
            m_ascanRangeStart = (m_ascanRangeStart > dr) ? m_ascanRangeStart-dr : 0;
            result = true;
            }
        }
        else if(m_arscrollFlag == ASCAN_RSCROLL)
        {   // R������
            const int dr = dx * m_rscanRangeWidth / m_rscanRect.width();
            if(qAbs(dr) > 0)
            {
            m_rscanRangeStart = (m_rscanRangeStart > dr) ? m_rscanRangeStart-dr : 0;
            result = true;
            }
        }
        break;
    }

    case B_SCAN:
    {
        // �ƶ�B����ʼ����
        int rng = m_bscanRangeStart + dy / m_bscanRangeRation;
        if(rng < 0)
            rng = 0;
        else if(rng > m_rangeMax - m_bscanRangeWidth)
            rng = m_rangeMax - m_bscanRangeWidth;
        if(rng != m_bscanRangeStart)
        {
            setBScanRange(rng);
            result = true;
        }
        // �ƶ�B����ʼ��λ
        int azi = m_bscanAzimuthStart - dx / m_bscanAzimuthRation;
        if(azi < 0)
            azi += MAXDEGREE;
        else if(azi > MAXDEGREE)
            azi -= MAXDEGREE;
        if(azi != m_bscanAzimuthStart)
        {
            setBScanAzimuth(azi);
            result = true;
        }
        break;
    }

    default:
        break;
    }

    return result;
}

// �ָ���PPI����״̬
void CRadarView::restoreToPPINormal()
{
    // �ָ�����
    restoreMaxRange();
    // ��ֱ�����������ƶ�����Ļ����
    setCenterScreen(screenEnvelope().center());

	m_scanMode = NORMAL_SCAN;

    m_offsetFlag = 0;
    m_scrollFlag = 0;
    m_zoomFlag = 0;

    m_scanKiloRange = 0;
    m_scanPixelRange = 0;
}

// ����B�Ծ������
void CRadarView::setBScanRange(quint16 start, quint16 width)
{
    bool flag = false;
    if(m_bscanRangeStart != start)
    {
        m_bscanRangeStart = start;
        flag = true;
    }

    if(width && m_bscanRangeWidth != width)
    {
        m_bscanRangeWidth = width;
        m_bscanRangeRation = ((float)m_screenEnvelope.height() / (float)width);
        flag = true;
    }

    if(flag)
    {
        m_bscanRangeStop = m_bscanRangeStart + m_bscanRangeWidth;
    }
}

// ����B�Է�λ����
void CRadarView::setBScanAzimuth(quint16 start, quint16 width)
{
    bool flag = false;
    if(m_bscanAzimuthStart != start)
    {
        m_bscanAzimuthStart = start;
        flag = true;
    }

    if(width && m_bscanAzimuthWidth != width)
    {
        m_bscanAzimuthWidth = width;
        m_bscanAzimuthRation = ((float)m_screenEnvelope.width() / (float)width);
        flag = true;
    }

    if(flag)
    {
        m_bscanAzimuthStop = m_bscanAzimuthStart + m_bscanAzimuthWidth;
    }
}

/*��Ļ����ת����ֱ������*/
SQUARE_POINT CRadarView::screentosquare_view(const SCREEN_POINT& sc)
{
    SQUARE_POINT sq = {0, 0};

    switch(m_scanMode)
    {
    case A_SCAN:
    {
        break;
    }
    case B_SCAN:
    {
        //���㷽λ��Ϣ
        float Azimuth0 = m_bscanAzimuthStart + (float)sc.x()/m_bscanAzimuthRation;
        //���С��0
        if(Azimuth0<0)
            Azimuth0 += 360;
        else if (Azimuth0 >= 360)
            Azimuth0 -= 360;
		const float theta = DEGREETORADIAN(Azimuth0);
        // ���㼫����(��λ����)
        RTHETA_POINT rtheta;
        rtheta.theta() = theta;
        rtheta.r() = m_bscanRangeStart + (float)(m_screenEnvelope.bottom()-sc.y())/m_bscanRangeRation;
        // ����ֱ������
        sq = rtheta_to_square(rtheta);
        break;
    }
    case EMPTY_SCAN:
    {
        //����˵��ֱ������
        sq = screen_to_square(sc);
        //ת���ɼ�����
        RTHETA_POINT rtheta = square_to_rtheta(sq);
        //��������
        if(rtheta.r() < m_scanKiloRange)
            sq = m_coordinateCenter.squarePoint;
        else
        {
            rtheta.r() -= m_scanKiloRange;
            sq = rtheta_to_square(rtheta);
        }
        break;
    }
    case DELAY_SCAN:
    {
        //����˵��ֱ������
        sq = screen_to_square(sc);
        //ת���ɼ�����
        RTHETA_POINT rtheta = square_to_rtheta(sq);
        //��������
        rtheta.r() += m_scanKiloRange;
        sq = rtheta_to_square(rtheta);
        break;
    }
    case NORMAL_SCAN:
    {
        //����˵��ֱ������
        sq = screen_to_square(sc);
        break;
    }
    }   // end switch(m_scanMode)

    return sq;
}

/*ֱ������ת������Ļ����*/
SCREEN_POINT CRadarView::squaretoscreen_view(const SQUARE_POINT& sq)
{
    SCREEN_POINT sc = {0,0};

    switch (m_scanMode)
    {
    //�������A��״̬
    case A_SCAN:
    {
        break;
    }
    //�����B��״̬��
    case B_SCAN:
    {
        RTHETA_POINT rtheta = square_to_rtheta(sq);
        int azi = (int)RADIANTODEGREE(rtheta.theta()), rng = rtheta.r();
        if ((azi < m_bscanAzimuthStart) && (m_bscanAzimuthStart - azi > 180))
            azi += 360;
        sc.rx() = m_screenEnvelope.left() + (int)((azi - m_bscanAzimuthStart) * m_bscanAzimuthRation+0.5);
        sc.ry() = m_screenEnvelope.bottom() - (int)((rng - m_bscanRangeStart) * m_bscanRangeRation+0.5);
        break;
    }
    //������ڿ���״̬
    case EMPTY_SCAN:
    {
        //ת���ɼ�����
        RTHETA_POINT rtheta = square_to_rtheta(sq);
        //��������
        rtheta.r() += m_scanKiloRange;
        sc = square_to_screen(rtheta_to_square(rtheta));
        break;
    }
    //�����ӳ�״̬
    case DELAY_SCAN:
    {
        //ת���ɼ�����
        RTHETA_POINT rtheta = square_to_rtheta(sq);
        //��������
        if (rtheta.r() < m_scanKiloRange)
            sc = m_coordinateCenter.screenPoint;
        else
        {
            rtheta.r() -= m_scanKiloRange;
            sc = square_to_screen(rtheta_to_square(rtheta));
        }
        break;
    }
    // ����״̬
    case NORMAL_SCAN:
    {
        sc = square_to_screen(sq);
        break;
    }

    default:
        break;
    }

    return sc;
}

void CRadarView::paint(QPainter* p)
{
/*
    p->setBrush(QBrush(Qt::black));
    p->drawRect(screenEnvelope());*/


    switch(m_scanMode)
    {
    case A_SCAN:
        drawScanLineA(p);
        break;
    case B_SCAN:
        drawScanLineB(p);
        break;
    case NORMAL_SCAN:
    case EMPTY_SCAN:
    case DELAY_SCAN:
        drawScanLineP(p);
        break;
    default:
        break;
    }
}

// ��A�Կ̶���
void CRadarView::drawScanLineA(QPainter* p)
{
    QPen pen = p->pen();
    pen.setColor (QColor(72, 72, 72));
    p->setPen (pen);

    int rngstart, rngwidth;

    // ����A�Կ̶���
	if(m_arflag & 0x01)
	{
    getAScanRange(rngstart, rngwidth);
    drawScaleLineAScan (p, m_ascanRect, rngstart, rngwidth, false);
	}

    // ����R�Կ̶���
	if(m_arflag & 0x02)
	{
    getRScanRange(rngstart, rngwidth);
    drawScaleLineAScan (p, m_rscanRect, rngstart, rngwidth, true);
	}
    //qDebug() << "RScan:" << m_rscanRect << rngstart << rngwidth;
    //qDebug() << "AScan:" << m_ascanRect << rngstart << rngwidth;
}

// ��B�Կ̶���
void CRadarView::drawScanLineB(QPainter* p)
{
    const int left = m_screenEnvelope.left();
    const int right = m_screenEnvelope.right();
    const int top = m_screenEnvelope.top();
    const int bottom = m_screenEnvelope.bottom();

    QPen pen = p->pen();

    // ���ƾ���̶���
    if (BIGZERO(CurrentRangeScale))
    {
        const int count = (int)(bscanRangeWidth() / CurrentRangeScale)+1;
        int startrng = ((int)(bscanRangeStart() / CurrentRangeScale))*CurrentRangeScale;
        if (startrng < bscanRangeStart())
            startrng += CurrentRangeScale;
        for (int i=0; i<count; i++)
        {
            // set line Color
            if (startrng % 100 == 0)
                pen.setColor (QColor(Km100Color));
            else if (startrng % 50 == 0)
                pen.setColor (QColor(Km50Color));
            else
                pen.setColor (QColor(Km10Color));

            const int y = bottom - (int)((startrng - m_bscanRangeStart) * m_bscanRangeRation);
            p->setPen (pen);
            p->drawLine (left, y, right, y);

            QString rngText = QString::number(startrng);
            p->drawText (left+3, y-1, rngText);
            p->drawText (right-30, y-1, rngText);

            startrng += CurrentRangeScale;
        }
    }

    // ���Ʒ�λ�̶���
    if (CurrentAzimuthScale)
    {
        // ������ʼ�Ƕ�
        const int count = (int)(m_bscanAzimuthWidth / CurrentAzimuthScale) + 1;
        int startazi = ((int)(m_bscanAzimuthStart / CurrentAzimuthScale))*CurrentAzimuthScale;
        if (startazi < m_bscanAzimuthStart)
            startazi += CurrentAzimuthScale;

        //qDebug() << count << startazi << m_bscanAzimuthStart << m_bscanAzimuthRation;

        for (int i=0; i<count; i++)
        {
            // set line Color
            if (startazi % 30 == 0)
                pen.setColor (QColor(Degree30Color));
            else if (startazi % 10 == 0)
                pen.setColor (QColor(Degree10Color));
            else
                pen.setColor (QColor(Degree5Color));

            const int x = left + (int)((startazi - m_bscanAzimuthStart) * m_bscanAzimuthRation+0.5);
            p->setPen (pen);
            p->drawLine (x, top, x, bottom);

            QString aziText = QString::number(startazi%360);
            p->drawText (x, top+20, aziText);
            p->drawText (x, bottom-20, aziText);

            startazi += CurrentAzimuthScale;
        }
    }
}

// ���òü�����
void CRadarView::setClipPath(QPainter* p)
{
    QPainterPath path;
	QRect rc_scan = screenEnvelope();
	// �����״�ʹ��Բ�βü�����
#if (!NAVI_RADAR)
    if(m_scrollFlag || m_zoomFlag || m_offsetFlag)
        path.addRect(rc_scan.adjusted(-1,-1,1,1));
    else
#endif
    {
        int rpmin = qMin(rc_scan.width(), rc_scan.height())/2;
        QPoint ptcenter = rc_scan.center();
        rc_scan.setRect(ptcenter.x()-rpmin, ptcenter.y()-rpmin, 2*rpmin, 2*rpmin);

        path.addEllipse(rc_scan.adjusted(-2,-2,2,2));
    }
    p->setClipPath(path);
    p->setClipping(true);
}

// ��P�Կ̶���
void CRadarView::drawScanLineP(QPainter* p0)
{
    QPainter& p = *p0;
    QPen pen = p.pen();
    QBrush brush = p.brush();

    // ���òü�����
    setClipPath(p0);

	p.save();

    // �����״����ĵ�����Ļ��ͼ�ľ��뷶Χ
    float rmin, rmax;
    radarCenterToViewRange(rmin, rmax);

    // �ӳ�ʱ�����뷶Χ�����ƶ�
    if (m_scanMode == DELAY_SCAN)
    {
        rmax += m_scanKiloRange;
        rmin += m_scanKiloRange;
    }
    // ����ʱ���뷶Χ���仯
    //else if (m_scanMode == EMPTY_SCAN)
    //{
    //}

    // ���������С����ֵ
    //const int MinRngViewToRadarCenter = rmin;
    //const int MaxRngViewToRadarCenter = rmax;

    const float  rngScale = CurrentRangeScale;
    const float  aziScale = CurrentAzimuthScale;

    // �����״�����ԭ����Դ������ĵ�����λ��
    // �õ���Ϊ����̶��ߺͷ�λ�̶��ߵ���ʼλ��
    const   QPoint sc_pt = square_to_screen(QPointF(0,0));

	pen.setWidth(1);
    p.setBrush(Qt::NoBrush);

    //qDebug() << rngScale << range() << rmin;

    // ���ƾ���̶���
    if (BIGZERO(rngScale))
    {
        // ������ʼ����Ϊdr��������
        const float dr = rngScale;
        const float dp = dr * ration();
        int n = (int)(rmin / dr);
        if ((n*dr < rmin) || (n == 0))
            n ++;

        const float rpmax = ration() * rmax;
        float r = dr * n, rp0 = ration() * r;
        for (; rp0 <= rpmax; rp0+=dp,r+=dr)
        {
			float rp = rp0;
			/*
            // ���û�����ɫ
            if (((int)r) % 100 == 0)
                pen.setColor (Km100Color);
            else if (((int)r) % 50 == 0)
                pen.setColor (Km50Color);
            else
                pen.setColor (Km10Color);
            p.setPen(pen);
			*/

            if (m_scanMode == DELAY_SCAN)
            {
                rp -= m_scanPixelRange;
                if (rp < 0)
                    continue;
            }
            else if (m_scanMode == EMPTY_SCAN)
            {
                rp += m_scanPixelRange;
                if (rp > rpmax)
                    break;
            }

            const int x0 = sc_pt.x()-rp, y0 = sc_pt.y()-rp;
            const int diameter =(int)(rp+rp);
            p.drawEllipse (x0, y0, diameter, diameter);

			/*
            // ��ʾ�����ַ���־
            if(r > 10.f && ((int)r) % 50 == 0)
            {
                for (quint8 i=0; i<4; i++)
                {
                    RTHETA_POINT rtheta;
                    rtheta.setPoint(rp, i*M_PI/2.0);
                    SQUARE_POINT sqpt = rtheta_to_square (rtheta);
                    QPoint ptText = sc_pt + sqpt.toPoint().toPoint();
                    p.drawText(ptText, QString("%1").arg(r));
                }
            }
			*/
        }
    }

    // ���Ʒ�λ�̶���
    if (BIGZERO(aziScale))
    {
        float rmax4aziscal = rmax - (m_scanMode == EMPTY_SCAN ? m_scanKiloRange : 0);
        float fixedrng[MAXDEGREE];
		/*
        if(m_offsetFlag)
        {
            for(int i=0;i<MAXDEGREE; i++)
                fixedrng[i] = CurrentRange[i] / ration();
        }
        else*/
        {
            for(int i=0;i<MAXDEGREE; i++)
                fixedrng[i] = rmax4aziscal;
        }

        RTHETA_POINT rtheta;
        rtheta.setX (rmax4aziscal);

        // �Ȼ�30��
        pen.setColor (Degree30Color);
        p.setPen (pen);
        for (int a=0; a<360; a+=30)
        {
            rtheta.setX(fixedrng[a]);
            rtheta.setY(DEGREETORADIAN(a));
            SCREEN_POINT sc0 = squaretoscreen_view(rtheta_to_square(rtheta));
            QPoint sc1 = sc_pt;
            if (m_scanMode == EMPTY_SCAN)
                sc1 = square_to_screen(QPointF(m_scanKiloRange*EachDegreeSin[a], m_scanKiloRange*EachDegreeCos[a]));
            p.drawLine (sc1, sc0.toPoint());
        }
        // �ٻ�10��
        if (aziScale < 30)
        {
            pen.setColor (Degree10Color);
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
                    sc1 = square_to_screen(QPointF(m_scanKiloRange*EachDegreeSin[a], m_scanKiloRange*EachDegreeCos[a]));
                p.drawLine (sc1, sc0.toPoint());
            }
        }
        // �ٻ�5��
        if (aziScale == 5)
        {
            pen.setColor (Degree5Color);
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
                    sc1 = square_to_screen(QPointF(m_scanKiloRange*EachDegreeSin[a], m_scanKiloRange*EachDegreeCos[a]));
                p.drawLine (sc1, sc0.toPoint());
            }
        }
    }

    //p.setClipping(false);

	// �����״���ƫ�����ηŴ����ʱ��Ȼ��ʾ��Χ��Ȧ, mofi 2012-06-28
    // ��������Χ�ľ���Ȧ
#if (!NAVI_RADAR)
    if (!(m_scrollFlag || m_zoomFlag || m_offsetFlag))
#endif
        drawOuterAzimuthCircle(&p);

    // ����ʱ�������Ŀհ�
    if (m_scanMode == EMPTY_SCAN)
    {
        p.setPen (Qt::NoPen);
        p.setBrush(QColor(98, 98, 98));
        const double rp = m_scanPixelRange;
        p.drawEllipse (sc_pt.x()-rp, sc_pt.y()-rp, 2*rp, 2*rp);
    }

	p.restore();
}

// �����״����ĵ���Ļ��ͼ�������С����
void CRadarView::radarCenterToViewRange(float& rmin, float& rmax)
{
    if(m_scrollFlag || m_zoomFlag || m_offsetFlag)
    {
    // ���״�����Ϊԭ�㣬���Ʒ�λ����̶���
    const ENVELOPE square_envelope = squareEnvelope ();
    const float x1 = square_envelope._MinX;
    const float x2 = square_envelope._MaxX;
    const float y1 = square_envelope._MinY;
    const float y2 = square_envelope._MaxY;
    //const float x3 = (x1+x2)/2.0;
    //const float y3 = (y1+y2)/2.0;

    const float absx1 = qAbs(x1);
    const float absx2 = qAbs(x2);
    const float absy1 = qAbs(y1);
    const float absy2 = qAbs(y2);

    const float x11 = x1 * x1;
    const float x22 = x2 * x2;
    //const float x33 = x3 * x3;
    const float y11 = y1 * y1;
    const float y22 = y2 * y2;
    //const float y33 = y3 * y3;

    const float r1 = sqrt(x11 + y11);
    const float r2 = sqrt(x11 + y22);
    const float r3 = sqrt(x22 + y11);
    const float r4 = sqrt(x22 + y22);
    //const float r5 = sqrt(x11 + y33);
    //const float r6 = sqrt(x33 + y22);
    //const float r7 = sqrt(x22 + y33);
    //const float r8 = sqrt(x33 + y11);

    QRectF envelope (x1, y1, x2-x1, y2-y1);

    // ����Ŵ�����ʾ�������״�����ԭ��������С����
    if (envelope.contains (0,0))
    {	// �״�����ԭ���ڷŴ󴰵���ʾ������
        rmin = 0;
    }
    else
    {	// �״�����ԭ���ڷŴ󴰵���ʾ������
        if ((x1 < 0) && (x2 > 0))
        {	// X����������ʾ������
            rmin = qMin(absy1, absy2);
        }
        else if ((y1 < 0) && (y2 > 0))
        {	// Y����������ʾ������
            rmin = qMin(absx1, absx2);
        }
        else
        {
            rmin = qMin(qMin(r1, r2), qMin(r3, r4));
        }
    }

//	rmax = qMax(qMax(absx1, absx2), qMax(absy1, absy2));
    rmax = qMax(qMax(r1, r2), qMax(r3, r4));
    }
    else
    {
        rmin = 0;
        rmax = m_displayRadius / ration();
        if (m_offsetFlag)
            rmax += m_offsetPixel / ration();
    }
}

// ����Χ�ķ�λȦ
void CRadarView::drawOuterAzimuthCircle(QPainter* p0)
{
    QPainter& p = *p0;
	const int border = 0;
    int radius = m_displayRadius - border;

	p.setClipping(false);

    //��������
    //QPen pen = p.pen();
    //pen.setColor(QColor(Km100Color));
    //p.setPen (pen);

    // ����Ȧ(����Ļ����ΪԲ��)
	const QPoint sc_center = screenEnvelope().center();
    p.drawEllipse (sc_center.x()-radius, sc_center.y()-radius, 2*radius, 2*radius);

    // �״����ĵ�����Ӧ����Ļ����(���״�ΪԲ��)
    const QPoint center = square_to_screen(QPointF(0,0));

	//p.drawLine(center.x(), center.y()-radius, center.x(), center.y()+radius);
	//p.drawLine(center.x()-radius, center.y(), center.x()+radius, center.y());

    QFont font = p.font();
//    font.setPointSize(13);	// ���ﲻ�����������С���ɵ��ô�����
//    p.setFont(font);
    const int left = QFontMetrics(font).boundingRect(("456")).width() / 2;

    // ����λ�̶���
    for(int i=0; i<360; i++)
    {
        int idx = i - RADIANTODEGREE(rotation());
        if(idx >= 360)
            idx -= 360;
        else if(idx < 0)
            idx += 360;

        if(m_offsetFlag)
            radius = CurrentRange[idx] - border;

        //�̶���5�ı���ʱ���̶��߼ӳ�һ��
        qint16 radius0 = radius - 4;
        if((i%5)==0)
            radius0 = radius - 8;
        else if((i%10)==0)
            radius0 = radius - 12;

		if(radius0 < 0)	radius0 = 0;
        int x1 = center.x() + radius * EachDegreeSin[idx];
        int y1 = center.y() - radius * EachDegreeCos[idx];
        int x2 = center.x() + radius0 * EachDegreeSin[idx];
        int y2 = center.y() - radius0 * EachDegreeCos[idx];
        p.drawLine(x1, y1, x2, y2);

        if(i % 10 == 0)
        {
            //radius0 -= 15;
			radius0 = radius + 3;
            int x3 = center.x() + radius0 * EachDegreeSin[idx];
            int y3 = center.y() - radius0 * EachDegreeCos[idx];

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

	p.setClipping(true);
}

void CRadarView::drawScaleLineAScan (QPainter* p0, const QRect& rect, int rngstart, int rngwidth, bool rngbase)
{
    QPainter& p = *p0;

    QFont font = p.font();
    font.setPointSize(10);
    p.setFont (font);
    QFontMetrics fm(font);

    int i=0;
    const int y_r_top = rect.top(), y_r_bottom = rect.bottom();
    const int y_r_left = rect.left(), y_r_right = rect.right();
    const int width = rect.width(), height = rect.height();

    // ����ˮƽ��
    const int dh = height / 5;
    p.drawLine (y_r_left, y_r_bottom, y_r_right, y_r_bottom);
    for(i=1; i<6; i++)
    {
        const int y = (i==5?y_r_top:y_r_bottom-i*dh);
        p.drawLine (y_r_left, y, y_r_right, y);
        p.drawText(y_r_left-15, y+5, QString("%1V").arg(i));
    }

    //x����
    quint16 x;
    int x1=y_r_left, x2=y_r_right;
    QString text1, text2;

    p.drawLine(y_r_left, y_r_top, y_r_left, y_r_bottom);
    p.drawLine(y_r_right, y_r_top, y_r_right, y_r_bottom);

    const int dtext = fm.width("1234");
    const int mindd = width / dtext;

    // ���ƿ̶���(��-ʱ��us)
    const qint32 tmWidth = rngwidth*20/3 ;
    double tm2Pixel = double(width) / double(tmWidth);
    double dtm;
    if (tmWidth <= 20)
        dtm = 1;
    else if (tmWidth <= 40)
        dtm = 2;
    else if (tmWidth <= 100)
        dtm = 5;
    else if (tmWidth <= 200)
        dtm = 10;
    else if (tmWidth <= 400)
        dtm = 20;
    else if (tmWidth <= 1000)
        dtm = 50;
    else if (tmWidth <= 2000)
        dtm = 100;
    else if (tmWidth <= 4000)
        dtm = 200;
    else if (tmWidth <= 10000)
        dtm = 500;
    else
        dtm = 1000;

    while(tmWidth / dtm  > mindd)
        dtm *= 2;

    //CSize szText1, szText2, szText;
    double dtm10 = dtm/10.0;
    double tm = 0;

    // ʱ��̶���ʾ��Yλ��
    const int text_y_tm = y_r_top-15;
    text1 = QString("%1us").arg(0);
    x1 = y_r_left - fm.width(text1)/2;
    p.drawText(x1, text_y_tm, text1);
    text2 = QString("%1us").arg(tmWidth);
    x2 = y_r_right - fm.width(text2)/2;
    p.drawText(x2, text_y_tm, text2);

    const int y_tm_1 = y_r_top - 10;
    const int y_tm_2 = rngbase ? y_r_top-1 : y_r_bottom;
    const int y_tm_3 = y_r_top - 5;
    const int y_tm_4 = y_r_top-1;

    while (tm < tmWidth)
    {
        x = y_r_left + (int)(tm * tm2Pixel + 0.5);
        const int cnt = (int)((tm+INFINITESIMAL) / dtm);
        double f = tm - dtm * cnt;
        if(cnt>0 && EQUALZERO(f))
        {
            p.drawLine (x, y_tm_1, x, y_tm_2);
            QString text = QString("%1").arg((qint32)tm);
            int w = fm.width(text);
            x -= w/2;
            if ((x > x1+2) && (x < x2-2-w))
                p.drawText(x, text_y_tm, text);
        }
        else
            p.drawLine (x, y_tm_3, x, y_tm_4);
        tm += dtm10;
    }

    // ���ƿ̶���(��-����km)
    qint32 rng1 = rngstart;
    qint32 rng2 = rngstart + rngwidth;
    qint32 rngWidth = rng2 - rng1;
    double rng2Pixel = double(width) / double(rngWidth);
    double drng;
    if (rngWidth <= 2)
        drng = 0.1;
    else if (rngWidth <= 4)
        drng = 0.2;
    else if (rngWidth <= 10)
        drng = 0.5;
    else if (rngWidth <= 20)
        drng = 1;
    else if (rngWidth <= 40)
        drng = 2;
    else if (rngWidth <= 100)
        drng = 5;
    else if (rngWidth <= 200)
        drng = 10;
    else if (rngWidth <= 500)
        drng = 20;
    else if (rngWidth <= 1000)
        drng = 50;
    else
        drng = 100;

    while(rngWidth / drng  > mindd)
        drng *= 2;

    //CSize szText1, szText2, szText;
    double drng10 = drng/10.0;
    double rng = (double)(rng1);

    // �����ʼ�ͽ�������
    const int text_y = y_r_bottom+22;
    text1 = QString("%1km").arg(rng1);
    x1 = y_r_left - fm.width(text1)/2;
    p.drawText(x1, text_y, text1);
    x1 += fm.width(text1);
    text2 = QString("%1km").arg(rng2);
    x2 = y_r_right - fm.width(text2)/2;
    p.drawText(x2, text_y, text2);

    const int y_rng_1 = rngbase ? y_r_top : y_r_bottom;
    const int y_rng_2 = y_r_bottom+10;
    const int y_rng_3 = y_r_bottom;
    const int y_rng_4 = y_r_bottom + 5;

    if (drng >= 1)
    {
        // ��������С��drng10��������
        const double dd = fmod (rng1, drng10);
        if (!EQUALZERO(dd))
            rng = rng1 + drng10 - dd;

        // ������ʼֵ
        if (EQUAL(rng, rng1))
            rng += drng10;

        while (rng < rng2)
        {
            x = y_r_left + (int)((rng-rng1) * rng2Pixel + 0.5);
            const int cnt = (int)((rng+INFINITESIMAL) / drng);
            if(EQUAL(rng, drng*cnt))
            {
                // ���¼������
                rng = drng * cnt;
                // ����x��λ��
                if(x > y_r_right)
                    x = y_r_right;
                // ��������
                p.drawLine (x, y_rng_1, x, y_rng_2);
                // ��ע����
                QString text = QString("%1").arg((qint32)(rng+INFINITESIMAL));
                int w = fm.width(text);
                x -= w/2;
                if ((x > x1+2) && (x < x2-2-w))
                    p.drawText(x, text_y, text);
            }
            else
                p.drawLine (x, y_rng_3, x, y_rng_4);
            rng += drng10;
        }
    }
    else
    {
        if (EQUAL(rng, rng1))
            rng += drng10;

        qint32 i=1;
        while (rng < rng2)
        {
            x = y_r_left + (int)((rng-rng1) * rng2Pixel + 0.5);
            //if (fmod (rng*100, drng*100) == 0)
            if (i%10 == 0)
            {
                p.drawLine (x, y_rng_1,x, y_rng_2);
                QString text = QString::number(rng, 'f', 1);
                int w = fm.width(text);
                x -= w/2;
                if ((x > x1+2) && (x < x2-2-w))
                    p.drawText(x, text_y, text);
            }
            else
                p.drawLine (x,y_rng_3,x, y_rng_4);

            i++;
            rng += drng10;
        }
    }
}

// ����ָ���㵽Բ��ÿ�ȵľ���
void CRadarView::calculateCenterToOutCircleRange()
{
    const int x = m_offsetX, y = m_offsetY;
    const QPoint center = screenEnvelope().center();
    //���㵱ǰ���ĵ�Բ�ܵľ���
    //���ݽǶȺ͵�ǰ��λ�ã�����д��ֱ�߷���
    //�ȼ������ǰ�������ԭ���ĵ�����
    const qint32 TempX = -(center.x() - x);
    const qint32 TempY = -(y - center.y());
	const qint32 Sign = (TempY >= 0 ? -1 : 1);
    double TempRange = fabs((double)TempX);
    //����ԭ���ĵ��������ĵľ���
    double Range2 = sqrt((double)(TempX*TempX + TempY*TempY));
    //�ȼ���Ƕ�Ϊ0��180�ȵİ뾶
    //���ƫ��������y����
    if(TempX==0)
    {
        //�����0�Ⱥ�180�ȵľ���
        CurrentRange[0] = m_displayRadius - TempY;
        CurrentRange[MAXDEGREE/2] = m_displayRadius + TempY;
    }
    //����ƫ�ĵ㲻��y����
    else
    {
        //�����ҵİ뾶����
        quint32 Range1 = (quint32)sqrt((double)(m_displayRadius*m_displayRadius - TempX*TempX));
        CurrentRange[0] = Range1-TempY;
        CurrentRange[MAXDEGREE/2] = Range1+TempY;
    }

    //���������Ƕȵľ���
    //������ĵ���ԭ�����2��4����
    if(((TempX <= 0) && (TempY >= 0)) ||
        ((TempX > 0) && (TempY < 0)) )
    {
        //�Ӵ���1��ʼ����
        for(uint kk=1;kk<MAXDEGREE/2;kk++)
        {
            //����ֱ�ߵ�б��
            double LineK = tan(M_PI/2-2*M_PI*kk/MAXDEGREE);
            //ֱ�ߵķ����ǣ�k*x-y+TempY-k*TempX=0;
            //����ԭ���ĵ���ֱ�ߵľ���
            double Range1 = fabs(TempY-LineK*TempX)/sqrt(LineK*LineK+1);
            //�����ҵ����ĵ㵽�������ĵľ���
            quint32 Range3 = (quint32)sqrt(Range2*Range2-Range1*Range1);
            //�����ҵİ뾶
            quint32 Range4 = (quint32)sqrt(m_displayRadius*m_displayRadius-Range1*Range1);
            //����˴��������ĵ㵽Բ���ľ���
            if(TempRange<=Range1)
            {
                TempRange = Range1;
                CurrentRange[kk] = Range4+Sign*Range3;
                CurrentRange[kk+MAXDEGREE/2] = Range4-Sign*Range3;
            }
            else
            {
                CurrentRange[kk] = Range4-Sign*Range3;
                CurrentRange[kk+MAXDEGREE/2] = Range4+Sign*Range3;
            }
        }
    }
    //������ĵ���Զ�����1��3����
    else
    {
        //�Ӵ���MAX_TRIGES/2-1��ʼ����
        for(uint kk=MAXDEGREE/2-1;kk>0;kk--)
        {
            //����ֱ�ߵ�б��
            double LineK = tan(M_PI/2-2*M_PI*kk/MAXDEGREE);
            //ֱ�ߵķ����ǣ�k*x-y+TempY-k*TempX=0;
            //����ԭ���ĵ���ֱ�ߵľ���
            double Range1 = fabs(TempY-LineK*TempX)/sqrt(LineK*LineK+1);
            //�����ҵ����ĵ㵽�������ĵľ���
            quint32 Range3 = (quint32)sqrt(Range2*Range2-Range1*Range1);
            //�����ҵİ뾶
            quint32 Range4 = (quint32)sqrt(m_displayRadius*m_displayRadius-Range1*Range1);
            //����˴��������ĵ㵽Բ���ľ���
            if(TempRange<=Range1)
            {
                TempRange = Range1;
                CurrentRange[kk] = Range4-Sign*Range3;
                CurrentRange[kk+MAXDEGREE/2] = Range4+Sign*Range3;
            }
            else
            {
                CurrentRange[kk] = Range4+Sign*Range3;
                CurrentRange[kk+MAXDEGREE/2] = Range4-Sign*Range3;
            }
        }
    }
}

// �ж�ָ��λ�õ��Ƿ�����ʾ������
bool CRadarView::isPointDisplay(const PLOTPOSITION& position)
{
    if(m_scanMode == A_SCAN)
    {
        return false;
    }
    else if(m_scanMode == B_SCAN)
    {
        const float r = position.rtheta_point.r();
        float a = RADIANTODEGREE(position.rtheta_point.theta());
        if(a < m_bscanAzimuthStart && m_bscanAzimuthStop > MAXDEGREE)
            a += MAXDEGREE;

        return ((r >= m_bscanRangeStart && r <= m_bscanRangeStop) &&
                (a >= m_bscanAzimuthStart && a <= m_bscanAzimuthStop));
    }
    else
    {
        if(m_scrollFlag || m_zoomFlag)
            return squareEnvelope().contains(position.square_point);
        else
        {
            const float r = position.rtheta_point.r();
            if(m_scanMode == DELAY_SCAN)
                return (r > m_scanKiloRange && r < range()+m_scanKiloRange);
            else if(m_scanMode == EMPTY_SCAN)
                return (r < range() - m_scanKiloRange);
            else if(m_offsetFlag)
            {
                const int a = RADIANTODEGREE(position.rtheta_point.theta());
                return r < CurrentRange[a];
            }
            else
                return r < range();
        }
    }
    return false;
}

// ������������
QPainterPath CRadarView::calculateFanPath (const RTHETA_POINT& pt1, const RTHETA_POINT& pt2)
{
    double r01 = pt1.r(), a01 = pt1.theta() - rotation();
    double r02 = pt2.r(), a02 = pt2.theta() - rotation();

    if (r01 > r02)
    {
        double r = r01;
        r01 = r02;
        r02 = r;
    }

    const double rmin = r01;
    const double rmax = r02;
    const double abgn = a01, aend = a02;
    const double da = (aend < abgn ? aend-abgn+M_2PI : aend-abgn);

    //const RTHETA_POINT rt1={rmin, amin}, rt2={rmax, amin}, rt3={rmax, amax}, rt4={rmin, amax};

    RTHETA_POINT rtheta;
    rtheta.setPoint(rmin, abgn);
    SCREEN_POINT sc1 = rtheta_to_screen(rtheta);
    rtheta.setPoint(rmax, abgn);
    SCREEN_POINT sc2 = rtheta_to_screen(rtheta);
    rtheta.setPoint(rmax, aend);
    SCREEN_POINT sc3 = rtheta_to_screen(rtheta);
    rtheta.setPoint(rmin, aend);
    SCREEN_POINT sc4 = rtheta_to_screen(rtheta);

    //const SCREEN_POINT center = SystemInfo.screen_point;
    const SQUARE_POINT sq0 = {0, 0};
    const SCREEN_POINT center = squaretoscreen_view(sq0);
    QLineF line1 (center.x(), center.y(), sc1.x(), sc1.y());
    QLineF line2 (center.x(), center.y(), sc2.x(), sc2.y());
    const int r1 = line1.length();
    const int r2 = line2.length();
    QRect rc1 (center.x()-r1, center.y()-r1, 2*r1, 2*r1);
    QRect rc2 (center.x()-r2, center.y()-r2, 2*r2, 2*r2);

    QPainterPath path;
    path.moveTo(sc1.x(), sc1.y());
    path.lineTo(sc2.x(), sc2.y());
	/*
	path.lineTo(sc3.x(), sc3.y());
	path.lineTo(sc4.x(), sc4.y());
	path.lineTo(sc1.x(), sc1.y());*/
	
    path.arcTo(rc2, 90-abgn*360.0/M_2PI, -da*360.0/M_2PI);
	path.moveTo(sc3.x(), sc3.y());
    path.lineTo(sc4.x(), sc4.y());
    path.arcTo(rc1, 90-aend*360.0/M_2PI, da*360.0/M_2PI);

	qDebug() << sc1.toPoint() << sc2.toPoint() << sc4.toPoint();
    return path;
}

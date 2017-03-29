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
			// 重新进行偏心操作
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
            // 重新计算延迟的象素
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

/*设置量程*/
void CRadarView::setRange (float rng)
{
	//CTransform::setRange(m_offsetFlag ? rng/2 : rng);
	CTransform::setRange(rng);
}

// 偏心处理
void CRadarView::offsetProcess(int x, int y)
{
    if(m_scanMode == A_SCAN || m_scanMode == B_SCAN)
        return;

    const bool offsetFlag = m_offsetFlag;
	const float rotate_angle = rotation();

    // 恢复到PPI正常状态
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

// 区域放大
void CRadarView::zoomArea(const QRect& rc)
{
    if(m_scanMode == A_SCAN || m_scanMode == B_SCAN)
        return;

    // 恢复到PPI正常状态
    if(m_scanMode == DELAY_SCAN || m_scanMode == EMPTY_SCAN || m_offsetFlag)
        restoreToPPINormal();

    saveMaxRange();

    const QPoint sc0 = rc.center();
    SCREEN_POINT sc = {sc0.x(), sc0.y()};
    setCenterSquare(screen_to_square (sc));

    // 更改主界面量程和转换系数
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

// 漫游处理
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
        // 移动A显起始距离
        if(m_arscrollFlag == ASCAN_ASCROLL)
        {   // A显漫游
            const int dr = dx * m_ascanRangeWidth / m_ascanRect.width();
            if(qAbs(dr) > 0)
            {
            m_ascanRangeStart = (m_ascanRangeStart > dr) ? m_ascanRangeStart-dr : 0;
            result = true;
            }
        }
        else if(m_arscrollFlag == ASCAN_RSCROLL)
        {   // R显漫游
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
        // 移动B显起始距离
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
        // 移动B显起始方位
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

// 恢复到PPI正常状态
void CRadarView::restoreToPPINormal()
{
    // 恢复量程
    restoreMaxRange();
    // 将直角坐标中心移动到屏幕中心
    setCenterScreen(screenEnvelope().center());

	m_scanMode = NORMAL_SCAN;

    m_offsetFlag = 0;
    m_scrollFlag = 0;
    m_zoomFlag = 0;

    m_scanKiloRange = 0;
    m_scanPixelRange = 0;
}

// 设置B显距离参数
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

// 设置B显方位参数
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

/*屏幕坐标转换成直角坐标*/
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
        //计算方位信息
        float Azimuth0 = m_bscanAzimuthStart + (float)sc.x()/m_bscanAzimuthRation;
        //如果小于0
        if(Azimuth0<0)
            Azimuth0 += 360;
        else if (Azimuth0 >= 360)
            Azimuth0 -= 360;
		const float theta = DEGREETORADIAN(Azimuth0);
        // 计算极坐标(方位距离)
        RTHETA_POINT rtheta;
        rtheta.theta() = theta;
        rtheta.r() = m_bscanRangeStart + (float)(m_screenEnvelope.bottom()-sc.y())/m_bscanRangeRation;
        // 计算直角坐标
        sq = rtheta_to_square(rtheta);
        break;
    }
    case EMPTY_SCAN:
    {
        //计算此点的直角坐标
        sq = screen_to_square(sc);
        //转换成极坐标
        RTHETA_POINT rtheta = square_to_rtheta(sq);
        //调整距离
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
        //计算此点的直角坐标
        sq = screen_to_square(sc);
        //转换成极坐标
        RTHETA_POINT rtheta = square_to_rtheta(sq);
        //调整距离
        rtheta.r() += m_scanKiloRange;
        sq = rtheta_to_square(rtheta);
        break;
    }
    case NORMAL_SCAN:
    {
        //计算此点的直角坐标
        sq = screen_to_square(sc);
        break;
    }
    }   // end switch(m_scanMode)

    return sq;
}

/*直角坐标转换成屏幕坐标*/
SCREEN_POINT CRadarView::squaretoscreen_view(const SQUARE_POINT& sq)
{
    SCREEN_POINT sc = {0,0};

    switch (m_scanMode)
    {
    //如果处于A显状态
    case A_SCAN:
    {
        break;
    }
    //如果在B显状态下
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
    //如果处于空心状态
    case EMPTY_SCAN:
    {
        //转换成极坐标
        RTHETA_POINT rtheta = square_to_rtheta(sq);
        //调整距离
        rtheta.r() += m_scanKiloRange;
        sc = square_to_screen(rtheta_to_square(rtheta));
        break;
    }
    //处于延迟状态
    case DELAY_SCAN:
    {
        //转换成极坐标
        RTHETA_POINT rtheta = square_to_rtheta(sq);
        //调整距离
        if (rtheta.r() < m_scanKiloRange)
            sc = m_coordinateCenter.screenPoint;
        else
        {
            rtheta.r() -= m_scanKiloRange;
            sc = square_to_screen(rtheta_to_square(rtheta));
        }
        break;
    }
    // 正常状态
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

// 画A显刻度线
void CRadarView::drawScanLineA(QPainter* p)
{
    QPen pen = p->pen();
    pen.setColor (QColor(72, 72, 72));
    p->setPen (pen);

    int rngstart, rngwidth;

    // 绘制A显刻度线
	if(m_arflag & 0x01)
	{
    getAScanRange(rngstart, rngwidth);
    drawScaleLineAScan (p, m_ascanRect, rngstart, rngwidth, false);
	}

    // 绘制R显刻度线
	if(m_arflag & 0x02)
	{
    getRScanRange(rngstart, rngwidth);
    drawScaleLineAScan (p, m_rscanRect, rngstart, rngwidth, true);
	}
    //qDebug() << "RScan:" << m_rscanRect << rngstart << rngwidth;
    //qDebug() << "AScan:" << m_ascanRect << rngstart << rngwidth;
}

// 画B显刻度线
void CRadarView::drawScanLineB(QPainter* p)
{
    const int left = m_screenEnvelope.left();
    const int right = m_screenEnvelope.right();
    const int top = m_screenEnvelope.top();
    const int bottom = m_screenEnvelope.bottom();

    QPen pen = p->pen();

    // 绘制距离刻度线
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

    // 绘制方位刻度线
    if (CurrentAzimuthScale)
    {
        // 计算起始角度
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

// 设置裁减区域
void CRadarView::setClipPath(QPainter* p)
{
    QPainterPath path;
	QRect rc_scan = screenEnvelope();
	// 导航雷达使用圆形裁减区域
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

// 画P显刻度线
void CRadarView::drawScanLineP(QPainter* p0)
{
    QPainter& p = *p0;
    QPen pen = p.pen();
    QBrush brush = p.brush();

    // 设置裁减区域
    setClipPath(p0);

	p.save();

    // 计算雷达中心到达屏幕视图的距离范围
    float rmin, rmax;
    radarCenterToViewRange(rmin, rmax);

    // 延迟时将距离范围往外移动
    if (m_scanMode == DELAY_SCAN)
    {
        rmax += m_scanKiloRange;
        rmin += m_scanKiloRange;
    }
    // 空心时距离范围不变化
    //else if (m_scanMode == EMPTY_SCAN)
    //{
    //}

    // 保存最大最小距离值
    //const int MinRngViewToRadarCenter = rmin;
    //const int MaxRngViewToRadarCenter = rmax;

    const float  rngScale = CurrentRangeScale;
    const float  aziScale = CurrentAzimuthScale;

    // 计算雷达中心原点相对窗口中心的坐标位置
    // 该点作为距离刻度线和方位刻度线的起始位置
    const   QPoint sc_pt = square_to_screen(QPointF(0,0));

	pen.setWidth(1);
    p.setBrush(Qt::NoBrush);

    //qDebug() << rngScale << range() << rmin;

    // 绘制距离刻度线
    if (BIGZERO(rngScale))
    {
        // 调整起始距离为dr的整倍数
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
            // 设置画笔颜色
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
            // 显示距离字符标志
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

    // 绘制方位刻度线
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

        // 先画30度
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
        // 再画10度
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
        // 再画5度
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

	// 导航雷达在偏心漫游放大操作时仍然显示外围的圈, mofi 2012-06-28
    // 绘制最外围的距离圈
#if (!NAVI_RADAR)
    if (!(m_scrollFlag || m_zoomFlag || m_offsetFlag))
#endif
        drawOuterAzimuthCircle(&p);

    // 空心时填充里面的空白
    if (m_scanMode == EMPTY_SCAN)
    {
        p.setPen (Qt::NoPen);
        p.setBrush(QColor(98, 98, 98));
        const double rp = m_scanPixelRange;
        p.drawEllipse (sc_pt.x()-rp, sc_pt.y()-rp, 2*rp, 2*rp);
    }

	p.restore();
}

// 计算雷达中心到屏幕视图的最大、最小距离
void CRadarView::radarCenterToViewRange(float& rmin, float& rmax)
{
    if(m_scrollFlag || m_zoomFlag || m_offsetFlag)
    {
    // 以雷达中心为原点，绘制方位距离刻度线
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

    // 计算放大窗所显示的区域到雷达中心原点的最大、最小距离
    if (envelope.contains (0,0))
    {	// 雷达中心原点在放大窗的显示区域内
        rmin = 0;
    }
    else
    {	// 雷达中心原点在放大窗的显示区域外
        if ((x1 < 0) && (x2 > 0))
        {	// X轴中心在显示区域内
            rmin = qMin(absy1, absy2);
        }
        else if ((y1 < 0) && (y2 > 0))
        {	// Y轴中心在显示区域内
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

// 画外围的方位圈
void CRadarView::drawOuterAzimuthCircle(QPainter* p0)
{
    QPainter& p = *p0;
	const int border = 0;
    int radius = m_displayRadius - border;

	p.setClipping(false);

    //创建画笔
    //QPen pen = p.pen();
    //pen.setColor(QColor(Km100Color));
    //p.setPen (pen);

    // 画外圈(以屏幕中心为圆心)
	const QPoint sc_center = screenEnvelope().center();
    p.drawEllipse (sc_center.x()-radius, sc_center.y()-radius, 2*radius, 2*radius);

    // 雷达中心点所对应的屏幕坐标(以雷达为圆心)
    const QPoint center = square_to_screen(QPointF(0,0));

	//p.drawLine(center.x(), center.y()-radius, center.x(), center.y()+radius);
	//p.drawLine(center.x()-radius, center.y(), center.x()+radius, center.y());

    QFont font = p.font();
//    font.setPointSize(13);	// 这里不再设置字体大小，由调用处设置
//    p.setFont(font);
    const int left = QFontMetrics(font).boundingRect(("456")).width() / 2;

    // 画方位刻度线
    for(int i=0; i<360; i++)
    {
        int idx = i - RADIANTODEGREE(rotation());
        if(idx >= 360)
            idx -= 360;
        else if(idx < 0)
            idx += 360;

        if(m_offsetFlag)
            radius = CurrentRange[idx] - border;

        //刻度是5的倍数时，刻度线加长一点
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

    // 绘制水平线
    const int dh = height / 5;
    p.drawLine (y_r_left, y_r_bottom, y_r_right, y_r_bottom);
    for(i=1; i<6; i++)
    {
        const int y = (i==5?y_r_top:y_r_bottom-i*dh);
        p.drawLine (y_r_left, y, y_r_right, y);
        p.drawText(y_r_left-15, y+5, QString("%1V").arg(i));
    }

    //x坐标
    quint16 x;
    int x1=y_r_left, x2=y_r_right;
    QString text1, text2;

    p.drawLine(y_r_left, y_r_top, y_r_left, y_r_bottom);
    p.drawLine(y_r_right, y_r_top, y_r_right, y_r_bottom);

    const int dtext = fm.width("1234");
    const int mindd = width / dtext;

    // 绘制刻度线(上-时间us)
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

    // 时间刻度显示的Y位置
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

    // 绘制刻度线(下-距离km)
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

    // 输出起始和结束距离
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
        // 调整到最小的drng10的整数倍
        const double dd = fmod (rng1, drng10);
        if (!EQUALZERO(dd))
            rng = rng1 + drng10 - dd;

        // 跳过开始值
        if (EQUAL(rng, rng1))
            rng += drng10;

        while (rng < rng2)
        {
            x = y_r_left + (int)((rng-rng1) * rng2Pixel + 0.5);
            const int cnt = (int)((rng+INFINITESIMAL) / drng);
            if(EQUAL(rng, drng*cnt))
            {
                // 重新计算距离
                rng = drng * cnt;
                // 调整x的位置
                if(x > y_r_right)
                    x = y_r_right;
                // 画竖真线
                p.drawLine (x, y_rng_1, x, y_rng_2);
                // 标注距离
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

// 计算指定点到圆周每度的距离
void CRadarView::calculateCenterToOutCircleRange()
{
    const int x = m_offsetX, y = m_offsetY;
    const QPoint center = screenEnvelope().center();
    //计算当前中心到圆周的距离
    //根据角度和当前的位置，可以写出直线方程
    //先计算出当前中心相对原中心的坐标
    const qint32 TempX = -(center.x() - x);
    const qint32 TempY = -(y - center.y());
	const qint32 Sign = (TempY >= 0 ? -1 : 1);
    double TempRange = fabs((double)TempX);
    //计算原中心到现在中心的距离
    double Range2 = sqrt((double)(TempX*TempX + TempY*TempY));
    //先计算角度为0和180度的半径
    //如果偏心中心在y轴上
    if(TempX==0)
    {
        //计算出0度和180度的距离
        CurrentRange[0] = m_displayRadius - TempY;
        CurrentRange[MAXDEGREE/2] = m_displayRadius + TempY;
    }
    //否则偏心点不在y轴上
    else
    {
        //计算弦的半径长度
        quint32 Range1 = (quint32)sqrt((double)(m_displayRadius*m_displayRadius - TempX*TempX));
        CurrentRange[0] = Range1-TempY;
        CurrentRange[MAXDEGREE/2] = Range1+TempY;
    }

    //计算其它角度的距离
    //如果中心点在原坐标的2，4象限
    if(((TempX <= 0) && (TempY >= 0)) ||
        ((TempX > 0) && (TempY < 0)) )
    {
        //从触发1开始计算
        for(uint kk=1;kk<MAXDEGREE/2;kk++)
        {
            //计算直线的斜率
            double LineK = tan(M_PI/2-2*M_PI*kk/MAXDEGREE);
            //直线的方程是：k*x-y+TempY-k*TempX=0;
            //计算原中心到此直线的距离
            double Range1 = fabs(TempY-LineK*TempX)/sqrt(LineK*LineK+1);
            //计算弦的中心点到现在中心的距离
            quint32 Range3 = (quint32)sqrt(Range2*Range2-Range1*Range1);
            //计算弦的半径
            quint32 Range4 = (quint32)sqrt(m_displayRadius*m_displayRadius-Range1*Range1);
            //计算此触发的中心点到圆弧的距离
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
    //如果中心点在远坐标的1，3象限
    else
    {
        //从触发MAX_TRIGES/2-1开始计算
        for(uint kk=MAXDEGREE/2-1;kk>0;kk--)
        {
            //计算直线的斜率
            double LineK = tan(M_PI/2-2*M_PI*kk/MAXDEGREE);
            //直线的方程是：k*x-y+TempY-k*TempX=0;
            //计算原中心到此直线的距离
            double Range1 = fabs(TempY-LineK*TempX)/sqrt(LineK*LineK+1);
            //计算弦的中心点到现在中心的距离
            quint32 Range3 = (quint32)sqrt(Range2*Range2-Range1*Range1);
            //计算弦的半径
            quint32 Range4 = (quint32)sqrt(m_displayRadius*m_displayRadius-Range1*Range1);
            //计算此触发的中心点到圆弧的距离
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

// 判断指定位置点是否在显示区域内
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

// 计算扇形区域
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

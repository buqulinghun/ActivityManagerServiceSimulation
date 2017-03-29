#include "transform.h"
#include <math.h>
#include <QtCore/QtGlobal>
#include <QtCore/QtDebug>

const int screen_interval = 0;
extern void initializeMathLib ();

///////////////////////////////////////////////////////////////////////////////////
//
// class LatitudeTransform implement
// author : mo flying
// date: 20080109
//
//////////////////////////////////////////////////////////////////////////////////
LatitudeTransform::LatitudeTransform()
{
    m_coefficientToKm = 1.852;//1.0;
    initializeMathLib();
}

/*设置中心点经纬度坐标*/
void LatitudeTransform::setCenterLatitude (const LATITUDE_POINT& latitude)
{
//    if (latitudePoint == latitude)
//        return;
	qDebug() << "----------LatitudeTransform::setCenterLatitude" << latitude.x() << latitude.y();
    latitudePoint = latitude;

    //由地面经纬度坐标获取地心经纬度坐标
    //对地球的椭球体进行修正
    float Lat_Geocent = ATAN(TAN(latitudePoint.latitude()) / (1 + EARTH_EXCENT2));

    float sin_Long = SIN(latitudePoint.longitude());
    float cos_Long = COS(latitudePoint.longitude());
    float sin_Lat  = SIN(Lat_Geocent);
    float cos_Lat  = COS(Lat_Geocent);
    float Earth_Radius = EARTH_RADIUS / sqrt(1 + EARTH_EXCENT2 * sin_Lat*sin_Lat);


    //地球坐标系(ECEF)和地理坐标系之间的变换矩阵,参考网上的教程编写
    GEOCENTRIC_MATRIX_[0][0] = -sin_Long;
    GEOCENTRIC_MATRIX_[0][1] =	-sin_Lat * cos_Long;
    GEOCENTRIC_MATRIX_[0][2] =  cos_Lat * cos_Long;

    GEOCENTRIC_MATRIX_[1][0] = cos_Long;
    GEOCENTRIC_MATRIX_[1][1] =	-sin_Lat * sin_Long;
    GEOCENTRIC_MATRIX_[1][2] =  cos_Lat * sin_Long;

    GEOCENTRIC_MATRIX_[2][0] =  0;
    GEOCENTRIC_MATRIX_[2][1] =  cos_Lat;
    GEOCENTRIC_MATRIX_[2][2] =  sin_Lat;

    /* the vector - radios from coa to center of earth    计算点到地球中心的矢量,xyz */
    GEOCENTRIC_POLAR_[0] = Earth_Radius * GEOCENTRIC_MATRIX_[0][2];
    GEOCENTRIC_POLAR_[1] = Earth_Radius * GEOCENTRIC_MATRIX_[1][2];
    GEOCENTRIC_POLAR_[2] = Earth_Radius * GEOCENTRIC_MATRIX_[2][2];

    float Gnom_Lat_Geocent = ATAN(TAN(latitudePoint.latitude()) / (1 + EARTH_EXCENT2));

    _SIN_TP_Latitude_ = SIN(Gnom_Lat_Geocent);
    _COS_TP_Latitude = COS(Gnom_Lat_Geocent);
    //切点处地球半径
    _RADIUS_TP_ = EARTH_RADIUS /
              sqrt(1 + EARTH_EXCENT2 * _SIN_TP_Latitude_*_SIN_TP_Latitude_);
    //投影距离
    _DISTANCE_TP_POLE_ = _RADIUS_TP_ / TAN(Gnom_Lat_Geocent) ;
    //切点处经度
    _TP_Longitude_ = latitudePoint.longitude() ;
}

/************************************/
/*函数名称： latitude_to_square		*/
/*功能：	地理坐标到直角坐标的转换*/
/*输入参数：latitude_point:地理坐标	*/
/*输出参数：直角坐标				*/
/************************************/
SQUARE_POINT LatitudeTransform::latitude_to_square(const LATITUDE_POINT& latitude_point,float Altitude)
{
    const float rate2km = coefficientToKm();

    float Lat_Geocent = ATAN(TAN(latitude_point.latitude()) / (1 + EARTH_EXCENT2));
    float sin_Long = SIN(latitude_point.longitude());
    float cos_Long = COS(latitude_point.longitude());
    float sin_Lat  = SIN(Lat_Geocent);
    float cos_Lat  = COS(Lat_Geocent);
    float Earth_Radius = EARTH_RADIUS /
                            sqrt(1 + EARTH_EXCENT2 * sin_Lat*sin_Lat) + Altitude;

    /* x , y , z - geocentric */
    float x_gcs = Earth_Radius * cos_Lat * cos_Long;
    float y_gcs = Earth_Radius * cos_Lat * sin_Long;
    float z_gcs = Earth_Radius * sin_Lat;

    SQUARE_POINT square_point;
    float x,y;
    /*square_point.rx()*/x = (float)((GEOCENTRIC_MATRIX_[0][0] * (x_gcs - GEOCENTRIC_POLAR_[0]) +
                GEOCENTRIC_MATRIX_[1][0] * (y_gcs - GEOCENTRIC_POLAR_[1]) +
                GEOCENTRIC_MATRIX_[2][0] * (z_gcs - GEOCENTRIC_POLAR_[2]))/1000.0) / rate2km;

    /*square_point.ry()*/y = (float)((GEOCENTRIC_MATRIX_[0][1] * (x_gcs - GEOCENTRIC_POLAR_[0]) +
                GEOCENTRIC_MATRIX_[1][1] * (y_gcs - GEOCENTRIC_POLAR_[1]) +
                GEOCENTRIC_MATRIX_[2][1] * (z_gcs - GEOCENTRIC_POLAR_[2]))/1000.0) / rate2km;

    //square_point.zrange = GEOCENTRIC_MATRIX_[0][2] * (x_gcs - GEOCENTRIC_POLAR_[0]) +
    //			GEOCENTRIC_MATRIX_[1][2] * (y_gcs - GEOCENTRIC_POLAR_[1]) +
    //			GEOCENTRIC_MATRIX_[2][2] * (z_gcs - GEOCENTRIC_POLAR_[2]);

    square_point.setPoint(x,y);
    return square_point;
}
QPointF LatitudeTransform::latitude_to_square(const QPointF& latitude_point,float Altitude)
{
    const float rate2km = coefficientToKm();

    float Lat_Geocent = ATAN(TAN(latitude_point.y()) / (1 + EARTH_EXCENT2));
    float sin_Long = SIN(latitude_point.x());
    float cos_Long = COS(latitude_point.x());
    float sin_Lat  = SIN(Lat_Geocent);
    float cos_Lat  = COS(Lat_Geocent);
    float Earth_Radius = EARTH_RADIUS /
                            sqrt(1 + EARTH_EXCENT2 * sin_Lat*sin_Lat) + Altitude;

    /* x , y , z - geocentric */
    float x_gcs = Earth_Radius * cos_Lat * cos_Long;
    float y_gcs = Earth_Radius * cos_Lat * sin_Long;
    float z_gcs = Earth_Radius * sin_Lat;

    QPointF square_point;
    square_point.rx() = (float)((GEOCENTRIC_MATRIX_[0][0] * (x_gcs - GEOCENTRIC_POLAR_[0]) +
                GEOCENTRIC_MATRIX_[1][0] * (y_gcs - GEOCENTRIC_POLAR_[1]) +
                GEOCENTRIC_MATRIX_[2][0] * (z_gcs - GEOCENTRIC_POLAR_[2]))/1000.0) / rate2km;

    square_point.ry() = (float)((GEOCENTRIC_MATRIX_[0][1] * (x_gcs - GEOCENTRIC_POLAR_[0]) +
                GEOCENTRIC_MATRIX_[1][1] * (y_gcs - GEOCENTRIC_POLAR_[1]) +
                GEOCENTRIC_MATRIX_[2][1] * (z_gcs - GEOCENTRIC_POLAR_[2]))/1000.0) / rate2km;

    //square_point.zrange = GEOCENTRIC_MATRIX_[0][2] * (x_gcs - GEOCENTRIC_POLAR_[0]) +
    //			GEOCENTRIC_MATRIX_[1][2] * (y_gcs - GEOCENTRIC_POLAR_[1]) +
    //			GEOCENTRIC_MATRIX_[2][2] * (z_gcs - GEOCENTRIC_POLAR_[2]);

    return square_point;
}
QPolygonF LatitudeTransform::latitude_to_square(const QPolygonF& latitude_point, QRectF* boundingRect, float Altitude)
{
    const float rate2km = coefficientToKm();

    QPolygonF scpolygon;
    QPointF sc;

    QPolygonF::const_iterator it1 = latitude_point.begin(), it2 = latitude_point.end();
    if(boundingRect)
    {
        float x1 = 65536, x2 = -65536, y1 = 65536, y2 = -65536;
        for (; it1 != it2; ++it1)
        {
            float x = ((*it1).x());
            float y = ((*it1).y());
            float Lat_Geocent = ATAN(TAN(y) / (1 + EARTH_EXCENT2));
            float sin_Long = SIN(x);
            float cos_Long = COS(x);
            float sin_Lat  = SIN(Lat_Geocent);
            float cos_Lat  = COS(Lat_Geocent);
            float Earth_Radius = EARTH_RADIUS /
                                    sqrt(1 + EARTH_EXCENT2 * sin_Lat*sin_Lat) + Altitude;

            /* x , y , z - geocentric */
            float x_gcs = Earth_Radius * cos_Lat * cos_Long;
            float y_gcs = Earth_Radius * cos_Lat * sin_Long;
            float z_gcs = Earth_Radius * sin_Lat;

            x = (float)((GEOCENTRIC_MATRIX_[0][0] * (x_gcs - GEOCENTRIC_POLAR_[0]) +
                    GEOCENTRIC_MATRIX_[1][0] * (y_gcs - GEOCENTRIC_POLAR_[1]) +
                    GEOCENTRIC_MATRIX_[2][0] * (z_gcs - GEOCENTRIC_POLAR_[2]))/1000.0) / rate2km;

            y = (float)((GEOCENTRIC_MATRIX_[0][1] * (x_gcs - GEOCENTRIC_POLAR_[0]) +
                    GEOCENTRIC_MATRIX_[1][1] * (y_gcs - GEOCENTRIC_POLAR_[1]) +
                    GEOCENTRIC_MATRIX_[2][1] * (z_gcs - GEOCENTRIC_POLAR_[2]))/1000.0) / rate2km;

            sc.rx() = x, sc.ry() = y;
            scpolygon << sc;

            x1 = qMin (x1, x), y1 = qMin (y1, y);
            x2 = qMax (x2, x), y2 = qMax (y2, y);
        }

        boundingRect->setRect(x1, y1, x2-x1, y2-y1);
    }
    else
    {
        for (; it1 != it2; ++it1)
        {
            float x = ((*it1).x());
            float y = ((*it1).y());
            float Lat_Geocent = ATAN(TAN(y) / (1 + EARTH_EXCENT2));
            float sin_Long = SIN(x);
            float cos_Long = COS(x);
            float sin_Lat  = SIN(Lat_Geocent);
            float cos_Lat  = COS(Lat_Geocent);
            float Earth_Radius = EARTH_RADIUS /
                                    sqrt(1 + EARTH_EXCENT2 * sin_Lat*sin_Lat) + Altitude;

            /* x , y , z - geocentric */
            float x_gcs = Earth_Radius * cos_Lat * cos_Long;
            float y_gcs = Earth_Radius * cos_Lat * sin_Long;
            float z_gcs = Earth_Radius * sin_Lat;

            x = (float)((GEOCENTRIC_MATRIX_[0][0] * (x_gcs - GEOCENTRIC_POLAR_[0]) +
                    GEOCENTRIC_MATRIX_[1][0] * (y_gcs - GEOCENTRIC_POLAR_[1]) +
                    GEOCENTRIC_MATRIX_[2][0] * (z_gcs - GEOCENTRIC_POLAR_[2]))/1000.0) / rate2km;

            y = (float)((GEOCENTRIC_MATRIX_[0][1] * (x_gcs - GEOCENTRIC_POLAR_[0]) +
                    GEOCENTRIC_MATRIX_[1][1] * (y_gcs - GEOCENTRIC_POLAR_[1]) +
                    GEOCENTRIC_MATRIX_[2][1] * (z_gcs - GEOCENTRIC_POLAR_[2]))/1000.0) / rate2km;

            sc.rx() = x, sc.ry() = y;
            scpolygon << sc;
        }
    }

    return scpolygon;
}


/************************************/
/*函数名称： square_to_latitude		*/
/*功能：	直角坐标到地理坐标的转换*/
/*输入参数：square_point:直角坐标	*/
/*输出参数：地理坐标				*/
/************************************/
LATITUDE_POINT LatitudeTransform::square_to_latitude(const SQUARE_POINT& sq, float *Altitude)
{
    const float rate2km = coefficientToKm();

    float x_gcs, y_gcs, z_gcs;
    float r_xy, r_xyz, Earth_Radius;
    float r2_xy, r2_xyz;
    float sin2_Lat;

    // 转换为米
    const float x = sq.x() * rate2km * 1000.0f;
    const float y = sq.y() * rate2km * 1000.0f;


    //求得相对地心位置，然后计算其经纬度
    x_gcs = GEOCENTRIC_MATRIX_[0][0] * x +
        GEOCENTRIC_MATRIX_[0][1] * y +
        //_ROC_GEOCENTRIC_MATRIX_[0][2] * sq.zrange +
        GEOCENTRIC_POLAR_[0];

    y_gcs = GEOCENTRIC_MATRIX_[1][0] * x +
        GEOCENTRIC_MATRIX_[1][1] * y +
        //GEOCENTRIC_MATRIX_[1][2] * sq.zrange +
        GEOCENTRIC_POLAR_[1];

    z_gcs = GEOCENTRIC_MATRIX_[2][0] * x +
        GEOCENTRIC_MATRIX_[2][1] * y +
        //GEOCENTRIC_MATRIX_[2][2] * sq.zrange +
        GEOCENTRIC_POLAR_[2];

    r2_xy = x_gcs*x_gcs + y_gcs*y_gcs;
    r2_xyz = r2_xy + z_gcs*z_gcs;
    r_xy = sqrt(r2_xy);
    r_xyz = sqrt(r2_xyz);

    sin2_Lat = z_gcs*z_gcs / r2_xyz;
    Earth_Radius = EARTH_RADIUS / sqrt(1 + EARTH_EXCENT2 * sin2_Lat);  //切点处半径

    //经纬度
    LATITUDE_POINT latitude_point;
    latitude_point.ry() = ATAN((1 + EARTH_EXCENT2) * (z_gcs / r_xy));
    latitude_point.rx() = ATAN2(y_gcs, x_gcs);

    //高度
    if(Altitude)
    {
        *Altitude = r_xyz - Earth_Radius;
    }

    return latitude_point;
}
QPointF LatitudeTransform::square_to_latitude(const QPointF& sq,float *Altitude)
{
    const float rate2km = coefficientToKm();

    float x_gcs, y_gcs, z_gcs;
    float r_xy, r_xyz, Earth_Radius;
    float r2_xy, r2_xyz;
    float sin2_Lat;

    // 转换为米
    const float x = sq.x() * rate2km * 1000.0f;
    const float y = sq.y() * rate2km * 1000.0f;

    x_gcs = GEOCENTRIC_MATRIX_[0][0] * x +
        GEOCENTRIC_MATRIX_[0][1] * y +
        //_ROC_GEOCENTRIC_MATRIX_[0][2] * sq.zrange +
        GEOCENTRIC_POLAR_[0];

    y_gcs = GEOCENTRIC_MATRIX_[1][0] * x +
        GEOCENTRIC_MATRIX_[1][1] * y +
        //GEOCENTRIC_MATRIX_[1][2] * sq.zrange +
        GEOCENTRIC_POLAR_[1];

    z_gcs = GEOCENTRIC_MATRIX_[2][0] * x +
        GEOCENTRIC_MATRIX_[2][1] * y +
        //GEOCENTRIC_MATRIX_[2][2] * sq.zrange +
        GEOCENTRIC_POLAR_[2];

    r2_xy = x_gcs*x_gcs + y_gcs*y_gcs;
    r2_xyz = r2_xy + z_gcs*z_gcs;
    r_xy = sqrt(r2_xy);
    r_xyz = sqrt(r2_xyz);

    sin2_Lat = z_gcs*z_gcs / r2_xyz;
    Earth_Radius = EARTH_RADIUS / sqrt(1 + EARTH_EXCENT2 * sin2_Lat);

    //经纬度
    QPointF latitude_point;
    latitude_point.ry() = ATAN((1 + EARTH_EXCENT2) * (z_gcs / r_xy));
    latitude_point.rx() = ATAN2(y_gcs, x_gcs);

    //高度
    if(Altitude)
    {
        *Altitude = r_xyz - Earth_Radius;
    }

    return latitude_point;
}


///////////////////////////////////////////////////////////////////////////////////
//
// class CTransform implement
// author : mo flying
// date: 20080109
//
//////////////////////////////////////////////////////////////////////////////////
LatitudeTransform CTransform::m_latitudeTransform;

CTransform::CTransform()
{
    m_rangeMin = 0.125;
    m_rangeMax = 1000;
    m_range = 100;
    m_ration = 1;
    m_rotation = 0.0;

    m_displayRadius = 100;

    m_coordinateCenter.latitudePoint.setPoint(0.0f, 0.0f);
    m_coordinateCenter.squarePoint.setPoint(0.0f, 0.0f);
    m_coordinateCenter.screenPoint.setPoint(0.0f, 0.0f);
}

/*设置中心点参数：中心点的经纬度坐标、中心点在窗口中的屏幕坐标、象素转换比率、正北方向的旋转角度*/
void CTransform::setCoordinateCenter (const LATITUDE_POINT& latitude, const SCREEN_POINT& screen, float range, quint16 rotate)
{
    if(LESSZERO(range))
        return;

    /* 计算相关的参数 */
    setCenterLatitude (latitude);
    /* 设置中心点所在的屏幕位置 */
    setCenterScreen(screen);
    /*设置量程*/
    setRange(range);
    /* 保存参数 */
    m_rotation = rotate;
}

/*设置中心点*/
void CTransform::setCenterPosition(const LATITUDE_POINT& latitude, const SCREEN_POINT& screen)
{
    setCenterLatitude(latitude);
    setCenterScreen(screen);
}

/*设置中心点经纬度坐标*/
void CTransform::setCenterLatitude (const LATITUDE_POINT& latitude)
{
    if (m_coordinateCenter.latitudePoint == latitude)
        return;

    m_coordinateCenter.latitudePoint = latitude;
    m_latitudeTransform.setCenterLatitude(latitude);
}

/*设置中心点屏幕坐标*/
void CTransform::setCenterScreen (const SCREEN_POINT& screen)
{
    m_coordinateCenter.screenPoint = screen;
    m_coordinateCenter.squarePoint.setPoint(0.0f, 0.0f);
    updateScreenEnvelope ();
}
void CTransform::setCenterScreen (const QPoint& screen)
{
    SCREEN_POINT sc;
    sc.fromPoint(screen);
    setCenterScreen(sc);
}

/*设置屏幕中心点对应的直角坐标*/
void CTransform::setCenterSquare(const SQUARE_POINT& square)
{
    m_coordinateCenter.squarePoint = square;
    updateScreenEnvelope ();
}

/*设置窗口大小*/
void CTransform::setScreenRect (const QRect& rect, const QPoint& screen)
{    
    m_screenEnvelope = rect;
    m_coordinateCenter.screenPoint.fromPoint(screen);
    m_coordinateCenter.squarePoint.setPoint(0.0f, 0.0f);

    m_displayRadius = qMin(m_screenEnvelope.width(), m_screenEnvelope.height())/2;

    updateScreenEnvelope ();
}

/*设置量程*/
void CTransform::setRange (float rng)
{
    if(LESSZERO(rng))
        return;

    if(rng < m_rangeMin)
        rng = m_rangeMin;
    else if(rng > m_rangeMax)
        rng = m_rangeMax;

    if(!EQUAL(m_range, rng))
    {
        m_range = rng;
        updateScreenEnvelope ();
    }
}

/* 由当前直角坐标的中心点和窗口大小确定相应的直角坐标和经纬度坐标的可显示区域*/
void CTransform::updateScreenEnvelope ()
{
    if (screenEnvelope().isEmpty())
        return;

	const float ration1 = ((float)m_displayRadius) / m_range ;
	const bool rationChangedFlag = !EQUAL(m_ration, ration1);
	if(rationChangedFlag)
		m_ration = ration1;

    const QRect scRect = screenEnvelope();
    const QPoint scCenter = screenCenter().toPoint();
    const QPointF sqCenter = squareCenter().toPoint();
    //const quint32 w = scRect.width(), h = scRect.height();

    // 计算窗口所显示的区域的直角坐标
    const float sqx1 = sqCenter.x() - (float)(scCenter.x()-scRect.left()) / m_ration;
    const float sqy1 = sqCenter.y() - (float)(scCenter.y()-scRect.top()) / m_ration;
    const float sqx2 = sqCenter.x() + (float)(scRect.right()-scCenter.x()) / m_ration;
    const float sqy2 = sqCenter.y() + (float)(scRect.bottom()-scCenter.y()) / m_ration;
    m_squareEnvelope.setEnvelope(
        qMin (sqx1, sqx2), qMin (sqy1, sqy2),\
        qMax (sqx1, sqx2), qMax (sqy1, sqy2));

    // 计算经纬度坐标的边界区域
    SQUARE_POINT sq_pt;
    sq_pt.setPoint(sqx1, sqy1);
    LATITUDE_POINT ld_pt1 = square_to_latitude(sq_pt);
    sq_pt.setPoint(sqx2, sqy2);
    LATITUDE_POINT ld_pt2 = square_to_latitude(sq_pt);

    const float ldx1 = ld_pt1.x(), ldx2 = ld_pt2.x();
    const float ldy1 = ld_pt1.y(), ldy2 = ld_pt2.y();
    m_latitudeEnvelope.setEnvelope(
        RADIANTODEGREE(qMin(ldx1, ldx2)), RADIANTODEGREE(qMin(ldy1, ldy2)),\
        RADIANTODEGREE(qMax(ldx1, ldx2)), RADIANTODEGREE(qMax(ldy1, ldy2)));

	if(rationChangedFlag)
		rationChanged(rationChangedFlag);
}

/************************************/
/*函数名称：screen_to_square         */
/*功能：	屏幕坐标到直角坐标的转换        */
/*输入参数：1、sc:屏幕坐标             */
/*输出参数：直角坐标                   */
/************************************/
SQUARE_POINT CTransform::screen_to_square(const SCREEN_POINT& sc)
{
    SQUARE_POINT sq;
    if(EQUALZERO(m_ration))
    {
        sq.rx() = m_coordinateCenter.squarePoint.x();//+0.0F;
        sq.ry() = m_coordinateCenter.squarePoint.y();//+0.0F;
    }
    else
    {
        sq.rx() = m_coordinateCenter.squarePoint.x()+((float)(sc.x() - m_coordinateCenter.screenPoint.x())/m_ration);
        sq.ry() = m_coordinateCenter.squarePoint.y()+((float)(m_coordinateCenter.screenPoint.y() - sc.y())/m_ration);
    }

    if(!EQUALZERO(m_rotation))
        sq.rotate(-m_rotation);

    return sq;
}

QPointF CTransform::screen_to_square(const QPoint& sc)
{
    SQUARE_POINT sq;
    if(EQUALZERO(m_ration))
    {
        sq.rx() = m_coordinateCenter.squarePoint.x();//+0.0F;
        sq.ry() = m_coordinateCenter.squarePoint.y();//+0.0F;
    }
    else
    {
        sq.rx() = m_coordinateCenter.squarePoint.x() + (float)((sc.x() - m_coordinateCenter.screenPoint.x())/m_ration);
        sq.ry() = m_coordinateCenter.squarePoint.y() + (float)((m_coordinateCenter.screenPoint.y() - sc.y())/m_ration);
    }

    if(!EQUALZERO(m_rotation))
        sq.rotate(-m_rotation);

    return sq.toPoint();
}


/************************************/
/*函数名称：square_to_screen		*/
/*功能：	直角坐标到屏幕坐标的转换*/
/*输入参数：1、square_point:屏幕坐标*/
/*输出参数：屏幕坐标				*/
/************************************/
SCREEN_POINT CTransform::square_to_screen(const SQUARE_POINT& sq0)
{
    SQUARE_POINT sq = sq0;
    if(!EQUALZERO(m_rotation))
        sq.rotate(m_rotation);

    SCREEN_POINT sc;
        sc.rx() = (int)(m_coordinateCenter.screenPoint.x()
               +((sq.x()-m_coordinateCenter.squarePoint.x())*m_ration+0.5) );
        sc.ry() = (int)(m_coordinateCenter.screenPoint.y()
               -((sq.y()-m_coordinateCenter.squarePoint.y())*m_ration-0.5) );

    return (sc);
}

QPoint CTransform::square_to_screen(const QPointF& sq0)
{
    SQUARE_POINT sq = {sq0.x(), sq0.y()};
    if(!EQUALZERO(m_rotation))
        sq.rotate(m_rotation);

    QPoint sc;
        sc.rx() = (int)(m_coordinateCenter.screenPoint.x()
               +((sq.x()-m_coordinateCenter.squarePoint.x())*m_ration+0.5) );
        sc.ry() = (int)(m_coordinateCenter.screenPoint.y()
               -((sq.y()-m_coordinateCenter.squarePoint.y())*m_ration-0.5) );

    return (sc);
}

QRect CTransform::square_to_screen (const QRectF& sq)
{
    int x1, x2, y1, y2;
        x1 = (int)(m_coordinateCenter.screenPoint.x()
               +((sq.left()-m_coordinateCenter.squarePoint.x())*m_ration+0.5) );
        y1 = (int)(m_coordinateCenter.screenPoint.y()
               -((sq.top()-m_coordinateCenter.squarePoint.y())*m_ration-0.5) );
        x2 = (int)(m_coordinateCenter.screenPoint.x()
               +((sq.right()-m_coordinateCenter.squarePoint.x())*m_ration+0.5) );
        y2 = (int)(m_coordinateCenter.screenPoint.y()
               -((sq.bottom()-m_coordinateCenter.squarePoint.y())*m_ration-0.5) );

    if (x1 > x2)
    {
        int x = x1;
        x1 = x2;
        x2 = x;
    }
    if (y1 > y2)
    {
        int y = y1;
        y1 = y2;
        y2 = y;
    }

    return QRect (x1, y1, x2-x1, y2-y1);
}

QPolygonF CTransform::square_to_screen (const QPolygonF& sqpolygon, QRect* boundingRect)
{
    QPolygonF scpolygon;
    QPointF sc, sc0(65536, 65536);
    QPolygonF::const_iterator it1 = sqpolygon.begin(), it2 = sqpolygon.end();

    if (boundingRect)
    {	// calculate bounding rect
        int xmin = 65536, xmax = -65536, ymin = 65536, ymax = -65536;
        for (; it1 != it2; ++it1)
        {
            sc.rx() = (int)(m_coordinateCenter.screenPoint.x()
                   +(((*it1).x()-m_coordinateCenter.squarePoint.x())*m_ration+0.5) );
            sc.ry() = (int)(m_coordinateCenter.screenPoint.y()
                   -(((*it1).y()-m_coordinateCenter.squarePoint.y())*m_ration-0.5) );
            float dx = sc.x() - sc0.x(), dy = sc.y() - sc0.y();
            if ((dx > screen_interval) || (dx < -screen_interval) || \
                (dy > screen_interval) || (dy < -screen_interval))
            {
                scpolygon << sc;
                sc0 = sc;

                xmin = qMin(xmin, (int)sc0.x());
                xmax = qMax(xmax, (int)sc0.x());
                ymin = qMin(ymin, (int)sc0.y());
                ymax = qMax(ymax, (int)sc0.y());
            }
        }
        boundingRect->setRect(xmin, ymin, xmax-xmin, ymax-ymin);
    }
    else
    {	// not calculate bounding rect
        for (; it1 != it2; ++it1)
        {
            sc.rx() = (int)(m_coordinateCenter.screenPoint.x()
                   +(((*it1).x()-m_coordinateCenter.squarePoint.x())*m_ration+0.5) );
            sc.ry() = (int)(m_coordinateCenter.screenPoint.y()
                   -(((*it1).y()-m_coordinateCenter.squarePoint.y())*m_ration-0.5) );
            float dx = sc.x() - sc0.x(), dy = sc.y() - sc0.y();
            if ((dx > screen_interval) || (dx < -screen_interval) || \
                (dy > screen_interval) || (dy < -screen_interval))
            {
                scpolygon << sc;
                sc0 = sc;
            }
        }
    }

    // to save last point
    if (sc != sc0)
    {
        scpolygon << sc;
    }

    return scpolygon;
}


/************************************/
/*函数名称： latitude_to_square		*/
/*功能：	地理坐标到直角坐标的转换*/
/*输入参数：latitude_point:地理坐标	*/
/*输出参数：直角坐标				*/
/************************************/
SQUARE_POINT CTransform::latitude_to_square(const LATITUDE_POINT& latitude_point,float Altitude)
{
    return m_latitudeTransform.latitude_to_square(latitude_point, Altitude);
}
QPointF CTransform::latitude_to_square(const QPointF& latitude_point,float Altitude)
{
    return m_latitudeTransform.latitude_to_square(latitude_point, Altitude);
}
QPolygonF CTransform::latitude_to_square(const QPolygonF& latitude_point, QRectF* boundingRect, float Altitude)
{
    return m_latitudeTransform.latitude_to_square(latitude_point, boundingRect, Altitude);
}

// 地理坐标转换成屏幕坐标
SCREEN_POINT CTransform::latitude_to_screen(const LATITUDE_POINT& ld)
{
    return square_to_screen(m_latitudeTransform.latitude_to_square(ld, 0));
}

QPoint CTransform::latitude_to_screen(const QPointF& ld)
{
    return square_to_screen(m_latitudeTransform.latitude_to_square(ld, 0));
}

// 屏幕坐标转换成地理坐标
QPointF CTransform::screen_to_latitude(const QPoint& sc)
{
    return square_to_latitude(screen_to_square(sc));
}

LATITUDE_POINT CTransform::screen_to_latitude(const SCREEN_POINT& sc)
{
    return square_to_latitude(screen_to_square(sc));
}

/************************************/
/*函数名称： square_to_latitude		*/
/*功能：	直角坐标到地理坐标的转换*/
/*输入参数：square_point:直角坐标	*/
/*输出参数：地理坐标				*/
/************************************/
LATITUDE_POINT CTransform::square_to_latitude(const SQUARE_POINT& sq,float *Altitude)
{
    return m_latitudeTransform.square_to_latitude(sq, Altitude);
}
QPointF CTransform::square_to_latitude(const QPointF& sq,float *Altitude)
{
    return m_latitudeTransform.square_to_latitude(sq, Altitude);
}

/************************************/
/*函数名称： square_to_rtheta		*/
/*功能：	直角坐标到极坐标的转换*/
/*输入参数：square_point:直角坐标	*/
/*输出参数：极坐标				*/
/************************************/
RTHETA_POINT CTransform::square_to_rtheta(const SQUARE_POINT& square_point)
{
    RTHETA_POINT rtheta;
    rtheta.r() = (float)sqrt(square_point.x()*square_point.x()+
          square_point.y()*square_point.y());
    rtheta.theta() = (float)(M_HALF_PI - ATAN2(square_point.y(),square_point.x()));
    if(rtheta.theta() < 0)
    {
        rtheta.theta() += M_2PI;
    }
    if(rtheta.theta() >= M_2PI)
    {
        rtheta.theta() -= M_2PI;
    }
    return rtheta;
}

QPointF CTransform::square_to_rtheta(const QPointF& square_point)
{
    QPointF rtheta;
    rtheta.rx() = (float)sqrt(square_point.x()*square_point.x()+
          square_point.y()*square_point.y());
    float y = (float)(M_HALF_PI - ATAN2(square_point.y(),square_point.x()));
    if(y < 0)
    {
        y += M_2PI;
    }
    if(y >= M_2PI)
    {
        y -= M_2PI;
    }
    rtheta.ry() = y;

    return rtheta;
}

// 计算两直角坐标之间的相对方位和相对距离
RTHETA_POINT CTransform::calculate_rtheta(const SQUARE_POINT& sq1, const SQUARE_POINT& sq2)
{
    double dx = sq1.x() - sq2.x();
    double dy = sq1.y() - sq2.y();

    RTHETA_POINT rtheta;

    rtheta.r() = (float)sqrt(dx*dx + dy*dy);
    double y = (float)(1.5*M_PI-atan2(dy,dx));
    //double y = (float)(M_PI+atan2(dy,dx));
    if(y < 0)
    {
        y += M_2PI;
    }
    if(y >= M_2PI)
    {
        y -= M_2PI;
    }
    rtheta.theta() = y;

    return rtheta;
}

/************************************/
/*函数名称： rtheta_to_square		*/
/*功能：	极坐标到直角坐标的转换*/
/*输入参数：rtheta:极坐标	*/
/*输出参数：直角坐标				*/
/************************************/
SQUARE_POINT CTransform::rtheta_to_square(const RTHETA_POINT& rtheta)
{
    SQUARE_POINT sq;
    const float theta = rtheta.theta();
    //计算x坐标
    sq.rx() = (float)(rtheta.r()*SIN(theta));
    //计算y坐标
    sq.ry() = (float)(rtheta.r()*COS(theta));
    //返回直角坐标
    return sq;
}


/************************************/
/*函数名称： screen_to_rtheta		*/
/*功能：	屏幕坐标转换成极坐标        */
/*输入参数：sc:屏幕坐标	*/
/*输出参数：极坐标				    */
/************************************/
RTHETA_POINT CTransform::screen_to_rtheta(const SCREEN_POINT& sc)
{
    RTHETA_POINT rtheta;
    //求极坐标的距离
    qint32 TempX = m_coordinateCenter.screenPoint.x();
    qint32 TempY = m_coordinateCenter.screenPoint.y();

    const qint32 dx = sc.x() - TempX, dy = sc.y() - TempY;

    rtheta.r() = (float)((sqrt((double)(dx*dx+dy*dy)))/ration());
    if(rtheta.r() < 0)
    {
        rtheta.r() = 0;
    }

    //求极坐标的角度
    rtheta.theta() = (float)(M_HALF_PI - ATAN2((double)-dy, (double)dx));
    if(rtheta.theta() < 0)
    {
        rtheta.theta() += M_2PI;
    }
    if(rtheta.theta() >= M_2PI)
    {
        rtheta.theta() -= M_2PI;
    }

    //返回极坐标
    return rtheta;
}

RTHETA_POINT CTransform::screen_to_rtheta(int x1,int y1,int x2,int y2)
{
    const int dx = x1 - x2, dy = y1 - y2;

    RTHETA_POINT rtheta;
    //求极坐标的距离
    rtheta.r() = (float)(sqrt((double)(dx*dx+dy*dy))/ration());
    //求极坐标的角度
    rtheta.theta() = (float)(0.5*M_PI-atan2((double)-dy,(double)dx));
    if(rtheta.theta() < 0)
    {
        rtheta.theta() += M_2PI;
    }
    if(rtheta.theta() >= M_2PI)
    {
        rtheta.theta() -= M_2PI;
    }

    //返回极坐标
    return rtheta;
}

/************************************/
/*函数名称： rtheta_to_screen		*/
/*功能：	极坐标转换成屏幕坐标*/
/*输入参数：rtheta:极坐标	*/
/*输出参数：屏幕坐标				*/
/************************************/
SCREEN_POINT CTransform::rtheta_to_screen(const RTHETA_POINT& rtheta)
{
    return square_to_screen(rtheta_to_square(rtheta));
}



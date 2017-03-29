#include "transform.h"
#include <math.h>
#include <QtCore/QtGlobal>
#include <QtCore/QtDebug>

const int screen_interval = 0;
extern void initializeMathLib ();


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
    float Lat_Geocent = ATAN(TAN(latitudePoint.latitude()) / (1 + EARTH_EXCENT2));
    float sin_Long = SIN(latitudePoint.longitude());
    float cos_Long = COS(latitudePoint.longitude());
    float sin_Lat  = SIN(Lat_Geocent);
    float cos_Lat  = COS(Lat_Geocent);
    float Earth_Radius = EARTH_RADIUS / sqrt(1 + EARTH_EXCENT2 * sin_Lat*sin_Lat);

    GEOCENTRIC_MATRIX_[0][0] = -sin_Long;
    GEOCENTRIC_MATRIX_[0][1] =	-sin_Lat * cos_Long;
    GEOCENTRIC_MATRIX_[0][2] =  cos_Lat * cos_Long;

    GEOCENTRIC_MATRIX_[1][0] = cos_Long;
    GEOCENTRIC_MATRIX_[1][1] =	-sin_Lat * sin_Long;
    GEOCENTRIC_MATRIX_[1][2] =  cos_Lat * sin_Long;

    GEOCENTRIC_MATRIX_[2][0] =  0;
    GEOCENTRIC_MATRIX_[2][1] =  cos_Lat;
    GEOCENTRIC_MATRIX_[2][2] =  sin_Lat;

    /* the vector - radios from coa to center of earth */
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

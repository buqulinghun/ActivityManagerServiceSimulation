#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "TargetManage_global.h"
#include <QtCore/QPoint>
#include <QtCore/QPointF>
#include <QtGui/QPolygonF>
#include <QtCore/QRect>
#include <QtCore/QRectF>


#define		EARTH_RADIUS 	6378135.0	/* Radius of earth at equator [m] */
#define		EARTH_EXCENT2	0.0067394	/* Excentricity of earth elipsoid */

class LatitudeTransform
{
public:
    LatitudeTransform();
    /*设置中心点经纬度坐标*/
    void setCenterLatitude (const LATITUDE_POINT& latitude);

    /*地理坐标(经纬度)转换成直角坐标*/
    SQUARE_POINT latitude_to_square(const LATITUDE_POINT& latitude_point,float Altitude=0);
    QPointF latitude_to_square(const QPointF& latitude_point,float Altitude=0);
    QPolygonF latitude_to_square(const QPolygonF& latitude_point,QRectF* boundingRect=NULL,float Altitude=0);

    /*直角坐标转换成地理坐标(经纬度)*/
    LATITUDE_POINT square_to_latitude(const SQUARE_POINT& square_point,float *Altitude=NULL);
    QPointF square_to_latitude(const QPointF& square_point,float *Altitude=NULL);

    /* 中心点经纬度坐标*/
    LATITUDE_POINT center() const
    {   return latitudePoint;   }

    /*设置与公里的转换系数*/
    void setCoefficientToKm(float rate)
    {   m_coefficientToKm = rate;   }
    float coefficientToKm() const
    {   return m_coefficientToKm;   }

private:
    float	GEOCENTRIC_MATRIX_[3][3];	// 地面直角坐标到地心直角坐标的转移矩阵
    float	GEOCENTRIC_POLAR_[3];		// 地面极坐标到地心极坐标的转移向量

    float  _TP_Longitude_;				//切点处经度 Tangency point Longitude
    float  _SIN_TP_Latitude_;			//切点处纬度的正弦值 Sin of tangency point Latitude
    float  _COS_TP_Latitude;			//切点处纬度的余弦值 Cos of tangency point Latitude
    float  _RADIUS_TP_ ;				//切点处地球半径 Earth radius at tang. point
    float  _DISTANCE_TP_POLE_ ;		//切点到投影点的距离 CV, distance from tang. point to projection of pole

    // 中心点经纬度坐标
    LATITUDE_POINT  latitudePoint;
    //与公里的转换系数
    float m_coefficientToKm;
};

#endif // TRANSFORM_H

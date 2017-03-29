#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "TargetManage_global.h"
#include <QtCore/QPoint>
#include <QtCore/QPointF>
#include <QtGui/QPolygonF>
#include <QtCore/QRect>
#include <QtCore/QRectF>


#define		EARTH_RADIUS 	6378135.0	/* Radius of earth at equator [m] 赤道的半径*/
#define		EARTH_EXCENT2	0.0067394	/* Excentricity of earth elipsoid  椭圆体的偏心率 */

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

class TARGETMANAGESHARED_EXPORT CTransform
{
public:
    CTransform();

    /*设置中心点参数：中心点的经纬度坐标、中心点在窗口中的屏幕坐标、正北方向的旋转角度*/
    void setCoordinateCenter (const LATITUDE_POINT& latitude, const SCREEN_POINT& screen, float range, quint16 rotate=0);
    /*设置中心点(中心点经纬度坐标和中心点屏幕坐标)*/
    void setCenterPosition(const LATITUDE_POINT& latitude, const SCREEN_POINT& screen);
    /*设置中心点经纬度坐标*/
    void setCenterLatitude (const LATITUDE_POINT& latitude);
    /*设置中心点屏幕坐标*/
    void setCenterScreen (const SCREEN_POINT& screen);
    void setCenterScreen (const QPoint& screen);
    /*设置屏幕中心点对应的直角坐标*/
    void setCenterSquare(const SQUARE_POINT& square);
    /*设置窗口区域*/
    void setScreenRect (const QRect& rect, const QPoint& screen);
    /*设置量程*/
    void setRange (float rng);
    /* 设置旋转角度*/
    void setRotation (float rotation=0.0)
    {   m_rotation = rotation;      }

    /*设置与公里的转换系数*/
    void setCoefficientToKm(float rate)
    {   m_latitudeTransform.setCoefficientToKm(rate);;   }
    float coefficientToKm() const
    {   return m_latitudeTransform.coefficientToKm();   }

    /*屏幕坐标转换成直角坐标*/
    SQUARE_POINT screen_to_square(const SCREEN_POINT& screen_point);
    QPointF screen_to_square(const QPoint& screen_point);

    /*直角坐标转换成屏幕坐标*/
    SCREEN_POINT square_to_screen(const SQUARE_POINT& square_point);
    QPoint square_to_screen(const QPointF& square_point);
    QPolygonF square_to_screen (const QPolygonF& square_point, QRect* boundingRect = NULL);
    QRect square_to_screen (const QRectF& square_point);

    /*地理坐标(经纬度)转换成直角坐标*/
    SQUARE_POINT latitude_to_square(const LATITUDE_POINT& latitude_point,float Altitude=0);
    QPointF latitude_to_square(const QPointF& latitude_point,float Altitude=0);
    QPolygonF latitude_to_square(const QPolygonF& latitude_point, QRectF* boundingRect=NULL, float Altitude=0);

    // 地理坐标转换成屏幕坐标
    SCREEN_POINT latitude_to_screen(const LATITUDE_POINT& ld);
    QPoint latitude_to_screen(const QPointF& ld);
    // 屏幕坐标转换成地理坐标
    QPointF screen_to_latitude(const QPoint& sc);
    LATITUDE_POINT screen_to_latitude(const SCREEN_POINT& sc);

    /*直角坐标转换成地理坐标(经纬度)*/
    LATITUDE_POINT square_to_latitude(const SQUARE_POINT& square_point,float *Altitude=NULL);
    QPointF square_to_latitude(const QPointF& square_point,float *Altitude=NULL);

    //直角坐标转换成极坐标
    RTHETA_POINT square_to_rtheta(const SQUARE_POINT& square_point);
    QPointF square_to_rtheta(const QPointF& square_point);
    //极坐标转换成直角坐标
    SQUARE_POINT rtheta_to_square(const RTHETA_POINT& rtheta);

    //屏幕坐标转换成极坐标
    RTHETA_POINT screen_to_rtheta(const SCREEN_POINT& screen_point);
    RTHETA_POINT screen_to_rtheta(int x1,int y1,int x2,int y2);
    //极坐标转换成屏幕坐标
    SCREEN_POINT rtheta_to_screen(const RTHETA_POINT& rtheta);

    LATITUDE_POINT latitudeCenter() const
    {   return m_coordinateCenter.latitudePoint;   }
    SCREEN_POINT screenCenter() const
    {   return m_coordinateCenter.screenPoint;   }
    SQUARE_POINT squareCenter() const
    {   return m_coordinateCenter.squarePoint;   }
    QRect screenEnvelope () const
    {   return m_screenEnvelope;    }
    ENVELOPE squareEnvelope () const
    {   return m_squareEnvelope;    }
    ENVELOPE latitudeEnvelope() const
    {   return m_latitudeEnvelope;  }

    // 返回量程
    float range() const
    {   return m_range;     }
    // 返回象素与距离比
    float ration () const
    {   return m_ration;    }
    // 返回旋转角度
    float rotation() const
    {   return m_rotation;  }

public:
    // 计算两直角坐标之间的相对方位和相对距离
    static RTHETA_POINT calculate_rtheta(const SQUARE_POINT& sq1, const SQUARE_POINT& sq2);


protected:
    /* 由当前直角坐标的中心点和窗口大小确定相应的直角坐标和经纬度坐标的可显示区域*/
    void updateScreenEnvelope ();

    virtual void rationChanged(bool changed=true)=0;

protected:
    /*中心点参数，.latitudePoint为中心点经纬度坐标，.screenPoint为中心点在窗口中的屏幕坐标
    .rthetaPoint为中心点在窗口中心坐标系中的极坐标，.squarePoint为中心点在窗口中心坐标系中的直角坐标*/
    COORDINATE  m_coordinateCenter;
    /*量程和缩放比例*/
    float       m_range;    // 距离
    float       m_ration;   // 高度象素/距离

    // 距离的有效范围
    float   m_rangeMin, m_rangeMax;
    // 显示半径
    int     m_displayRadius;

    /**/
    // 屏幕旋转角度
    float       m_rotation;

    ENVELOPE    m_latitudeEnvelope;
    ENVELOPE    m_squareEnvelope;
    QRect       m_screenEnvelope;

    static LatitudeTransform    m_latitudeTransform;
};


#endif // TRANSFORM_H

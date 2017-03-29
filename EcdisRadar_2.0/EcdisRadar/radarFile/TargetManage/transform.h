#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "TargetManage_global.h"
#include <QtCore/QPoint>
#include <QtCore/QPointF>
#include <QtGui/QPolygonF>
#include <QtCore/QRect>
#include <QtCore/QRectF>


#define		EARTH_RADIUS 	6378135.0	/* Radius of earth at equator [m] ����İ뾶*/
#define		EARTH_EXCENT2	0.0067394	/* Excentricity of earth elipsoid  ��Բ���ƫ���� */

class LatitudeTransform
{
public:
    LatitudeTransform();
    /*�������ĵ㾭γ������*/
    void setCenterLatitude (const LATITUDE_POINT& latitude);

    /*��������(��γ��)ת����ֱ������*/
    SQUARE_POINT latitude_to_square(const LATITUDE_POINT& latitude_point,float Altitude=0);
    QPointF latitude_to_square(const QPointF& latitude_point,float Altitude=0);
    QPolygonF latitude_to_square(const QPolygonF& latitude_point,QRectF* boundingRect=NULL,float Altitude=0);

    /*ֱ������ת���ɵ�������(��γ��)*/
    LATITUDE_POINT square_to_latitude(const SQUARE_POINT& square_point,float *Altitude=NULL);
    QPointF square_to_latitude(const QPointF& square_point,float *Altitude=NULL);

    /* ���ĵ㾭γ������*/
    LATITUDE_POINT center() const
    {   return latitudePoint;   }

    /*�����빫���ת��ϵ��*/
    void setCoefficientToKm(float rate)
    {   m_coefficientToKm = rate;   }
    float coefficientToKm() const
    {   return m_coefficientToKm;   }

private:
    float	GEOCENTRIC_MATRIX_[3][3];	// ����ֱ�����굽����ֱ�������ת�ƾ���
    float	GEOCENTRIC_POLAR_[3];		// ���漫���굽���ļ������ת������

    float  _TP_Longitude_;				//�е㴦���� Tangency point Longitude
    float  _SIN_TP_Latitude_;			//�е㴦γ�ȵ�����ֵ Sin of tangency point Latitude
    float  _COS_TP_Latitude;			//�е㴦γ�ȵ�����ֵ Cos of tangency point Latitude
    float  _RADIUS_TP_ ;				//�е㴦����뾶 Earth radius at tang. point
    float  _DISTANCE_TP_POLE_ ;		//�е㵽ͶӰ��ľ��� CV, distance from tang. point to projection of pole

    // ���ĵ㾭γ������
    LATITUDE_POINT  latitudePoint;
    //�빫���ת��ϵ��
    float m_coefficientToKm;
};

class TARGETMANAGESHARED_EXPORT CTransform
{
public:
    CTransform();

    /*�������ĵ���������ĵ�ľ�γ�����ꡢ���ĵ��ڴ����е���Ļ���ꡢ�����������ת�Ƕ�*/
    void setCoordinateCenter (const LATITUDE_POINT& latitude, const SCREEN_POINT& screen, float range, quint16 rotate=0);
    /*�������ĵ�(���ĵ㾭γ����������ĵ���Ļ����)*/
    void setCenterPosition(const LATITUDE_POINT& latitude, const SCREEN_POINT& screen);
    /*�������ĵ㾭γ������*/
    void setCenterLatitude (const LATITUDE_POINT& latitude);
    /*�������ĵ���Ļ����*/
    void setCenterScreen (const SCREEN_POINT& screen);
    void setCenterScreen (const QPoint& screen);
    /*������Ļ���ĵ��Ӧ��ֱ������*/
    void setCenterSquare(const SQUARE_POINT& square);
    /*���ô�������*/
    void setScreenRect (const QRect& rect, const QPoint& screen);
    /*��������*/
    void setRange (float rng);
    /* ������ת�Ƕ�*/
    void setRotation (float rotation=0.0)
    {   m_rotation = rotation;      }

    /*�����빫���ת��ϵ��*/
    void setCoefficientToKm(float rate)
    {   m_latitudeTransform.setCoefficientToKm(rate);;   }
    float coefficientToKm() const
    {   return m_latitudeTransform.coefficientToKm();   }

    /*��Ļ����ת����ֱ������*/
    SQUARE_POINT screen_to_square(const SCREEN_POINT& screen_point);
    QPointF screen_to_square(const QPoint& screen_point);

    /*ֱ������ת������Ļ����*/
    SCREEN_POINT square_to_screen(const SQUARE_POINT& square_point);
    QPoint square_to_screen(const QPointF& square_point);
    QPolygonF square_to_screen (const QPolygonF& square_point, QRect* boundingRect = NULL);
    QRect square_to_screen (const QRectF& square_point);

    /*��������(��γ��)ת����ֱ������*/
    SQUARE_POINT latitude_to_square(const LATITUDE_POINT& latitude_point,float Altitude=0);
    QPointF latitude_to_square(const QPointF& latitude_point,float Altitude=0);
    QPolygonF latitude_to_square(const QPolygonF& latitude_point, QRectF* boundingRect=NULL, float Altitude=0);

    // ��������ת������Ļ����
    SCREEN_POINT latitude_to_screen(const LATITUDE_POINT& ld);
    QPoint latitude_to_screen(const QPointF& ld);
    // ��Ļ����ת���ɵ�������
    QPointF screen_to_latitude(const QPoint& sc);
    LATITUDE_POINT screen_to_latitude(const SCREEN_POINT& sc);

    /*ֱ������ת���ɵ�������(��γ��)*/
    LATITUDE_POINT square_to_latitude(const SQUARE_POINT& square_point,float *Altitude=NULL);
    QPointF square_to_latitude(const QPointF& square_point,float *Altitude=NULL);

    //ֱ������ת���ɼ�����
    RTHETA_POINT square_to_rtheta(const SQUARE_POINT& square_point);
    QPointF square_to_rtheta(const QPointF& square_point);
    //������ת����ֱ������
    SQUARE_POINT rtheta_to_square(const RTHETA_POINT& rtheta);

    //��Ļ����ת���ɼ�����
    RTHETA_POINT screen_to_rtheta(const SCREEN_POINT& screen_point);
    RTHETA_POINT screen_to_rtheta(int x1,int y1,int x2,int y2);
    //������ת������Ļ����
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

    // ��������
    float range() const
    {   return m_range;     }
    // ��������������
    float ration () const
    {   return m_ration;    }
    // ������ת�Ƕ�
    float rotation() const
    {   return m_rotation;  }

public:
    // ������ֱ������֮�����Է�λ����Ծ���
    static RTHETA_POINT calculate_rtheta(const SQUARE_POINT& sq1, const SQUARE_POINT& sq2);


protected:
    /* �ɵ�ǰֱ����������ĵ�ʹ��ڴ�Сȷ����Ӧ��ֱ������;�γ������Ŀ���ʾ����*/
    void updateScreenEnvelope ();

    virtual void rationChanged(bool changed=true)=0;

protected:
    /*���ĵ������.latitudePointΪ���ĵ㾭γ�����꣬.screenPointΪ���ĵ��ڴ����е���Ļ����
    .rthetaPointΪ���ĵ��ڴ�����������ϵ�еļ����꣬.squarePointΪ���ĵ��ڴ�����������ϵ�е�ֱ������*/
    COORDINATE  m_coordinateCenter;
    /*���̺����ű���*/
    float       m_range;    // ����
    float       m_ration;   // �߶�����/����

    // �������Ч��Χ
    float   m_rangeMin, m_rangeMax;
    // ��ʾ�뾶
    int     m_displayRadius;

    /**/
    // ��Ļ��ת�Ƕ�
    float       m_rotation;

    ENVELOPE    m_latitudeEnvelope;
    ENVELOPE    m_squareEnvelope;
    QRect       m_screenEnvelope;

    static LatitudeTransform    m_latitudeTransform;
};


#endif // TRANSFORM_H

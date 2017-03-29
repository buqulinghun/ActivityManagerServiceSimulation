#ifndef TARGETMANAGE_GLOBAL_H
#define TARGETMANAGE_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QtCore/QPoint>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <math.h>

#ifdef _MSVC_

#  define TARGETMANAGESHARED_EXPORT

#else

#if defined(TARGETMANAGE_LIBRARY)
#  define TARGETMANAGESHARED_EXPORT Q_DECL_EXPORT
#else
#  define TARGETMANAGESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif

#ifndef M_PI
#define M_PI	3.14159265359
#endif

#ifndef M_2PI
#define	M_2PI	6.28318530718
#endif

#ifndef M_HALF_PI
#define M_HALF_PI 1.57079632679
#endif

#define MAXVIEWCOUNT 4
//#define

#define QRGB(r, g, b)		(((quint32(r)) << 16) | ((quint32(g)) << 8) | (b))
#define QRGBA(r, g, b, a)	(quint32)(((a)<<24)|((r)<<16)|((g)<<8)|(b))

// 符号索引
enum {
    SYMBOL_NONE = 0,
    SYMBOL_POINT,           // 小圆点
    SYMBOL_CIRCLE,			// 小圆圈
    SYMBOL_TRIANGLE_UP,     // 正三角形
    SYMBOL_TRIANGLE_DOWN,   // 倒三角形
    SYMBOL_DIAMOND,         // 菱形
    SYMBOL_CROSS,           // 十字
    SYMBOL_EMPTYCROSS,      // 中空十字
    SYMBOL_SQUARE,          // 正方形
    SYMBOL_SQUARE_CROSS,    // 正方形+十字
    SYMBOL_DIAG_CROSS,      // X形
    SYMBOL_BIAS,            // 斜线
    SYMBOL_ASTERISK,        // 星号
	SYMBOL_ARROW_UP,
	SYMBOL_ARROW_DN,
	SYMBOL_HLINE,
	SYMBOL_AIS,
	SYMBOL_AIS_BORDER,
	SYMBOL_ATA_3,
	SYMBOL_ATA_4,
	SYMBOL_ATA_6,
	SYMBOL_ATA_7,
	SYMBOL_ATA_8,
	SYMBOL_ATA_9,
	SYMBOL_ATA_12,
    SYMBOL_MAX,
};


/*角度与弧度相互转换*/
#define COEF_RADIANTODEGREE   57.2957795131  // 360/M_2PI
#define COEF_DEGREETORADIAN   0.0174532925  // M_2PI/360
/*角度转换为弧度*/
#define DEGREETORADIAN(a) ((float(a))*((float)(COEF_DEGREETORADIAN)))
/*弧度转换为角度*/
#define RADIANTODEGREE(a) ((float(a))*((float)(COEF_RADIANTODEGREE)))

/*用于判定浮点数的等于操作*/
#define INFINITESIMAL   (1.0e-4)
#define EQUAL(a,b) (qAbs((a)-(b)) < INFINITESIMAL)
#define EQUALZERO(a)    (qAbs(a) < INFINITESIMAL)
#define LESSZERO(a)    ((a) < INFINITESIMAL)
#define BIGZERO(a)     ((a) > INFINITESIMAL)

/*自定义的数学函数*/
extern float Sin(float x);
extern float Cos(float x);
extern float Tan(float x);
extern float Atan (float x);
extern float Atan2(float x, float y);

#if 1
#define SIN     Sin
#define COS     Cos
#define TAN     Tan
#define ATAN    atan
#define ATAN2   atan2
#else
#define SIN     sin
#define COS     cos
#define TAN     tan
#define ATAN    atan
#define ATAN2   atan2
#endif

// 删除容器中包含的对象
#define DeleteObjectInContainer(ObjType,Container)\
{\
    ObjType::const_iterator it1=Container.begin(), it2=Container.end(), it;\
    for(it=it1; it!=it2; ++it)   \
        delete(*it);\
    Container.clear();\
}

/*点结构定义*/
class SCREEN_POINT
{
public:
    qint16 __x, __y;

    void setPoint (qint16 x, qint16 y)
    {	__x = x, __y = y;	}
    void setX (qint16 x)
    {	__x = x;	}
    void setY (qint16 y)
    {	__y = y;	}
    qint16 x () const
    {	return __x;	}
    qint16 y () const
    {	return __y;	}
    qint16& rx ()
    {	return __x;	}
    qint16& ry ()
    {	return __y;	}

    SCREEN_POINT operator+(const SCREEN_POINT& pt) const
    {
        SCREEN_POINT pt0;
        pt0.rx() = x() + pt.x();
        pt0.ry() = y() + pt.y();
        return pt0;
    }

    SCREEN_POINT operator-(const SCREEN_POINT& pt) const
    {
        SCREEN_POINT pt0;
        pt0.rx() = x() - pt.x();
        pt0.ry() = y() - pt.y();
        return pt0;
    }

    SCREEN_POINT& operator+=(const SCREEN_POINT& pt)
    {
        rx() = x() + pt.x();
        ry() = y() + pt.y();
        return *this;
    }

    SCREEN_POINT& operator+=(const QPoint& pt)
    {
        rx() = x() + pt.x();
        ry() = y() + pt.y();
        return *this;
    }

    void fromPoint (const QPoint& pt)
    {
        __x = pt.x(), __y = pt.y();
    }
    QPoint toPoint () const
    {
        QPoint pt(__x, __y);
        return pt;
    }

    /*坐标旋转*/
    void rotate (float angle)
    {
        const float sinx = SIN(angle), cosx = COS(angle);
        const float x0 = x(), y0 = y();
       // rx() = (qint16)(x0 * cosx - y0 * sinx);
      //  ry() = (qint16)(x0 * sinx + y0 * cosx);
        rx() = (qint16)(x0 * cosx + y0 * sinx);
        ry() = (qint16)( y0 * cosx - x0 * sinx );
    }
    SCREEN_POINT rotated(float angle)
    {
        SCREEN_POINT pt;
        const float sinx = SIN(angle), cosx = COS(angle);
        const float x0 = x(), y0 = y();
        pt.rx() = (qint16)(x0 * cosx - y0 * sinx);
        pt.ry() = (qint16)(x0 * sinx + y0 * cosx);
        return pt;
    }

friend bool operator==(const SCREEN_POINT& sc1, const SCREEN_POINT& sc2)
{	return sc1.x() == sc2.x() && sc1.y() == sc2.y();	}
friend bool operator!=(const SCREEN_POINT& sc1, const SCREEN_POINT& sc2)
{	return !(sc1 == sc2);								}

};

class SQUARE_POINT
{
public:
    float __x, __y;

    void setPoint (float x, float y)
    {	__x = x, __y = y;	}
    void setX (float x)
    {	__x = x;	}
    void setY (float y)
    {	__y = y;	}
    float x () const
    {	return __x;	}
    float y () const
    {	return __y;	}
    float& rx ()
    {	return __x;	}
    float& ry ()
    {	return __y;	}

    SQUARE_POINT operator+(const SQUARE_POINT& pt) const
    {
        SQUARE_POINT pt0;
        pt0.rx() = x() + pt.x();
        pt0.ry() = y() + pt.y();
        return pt0;
    }

    SQUARE_POINT operator-(const SQUARE_POINT& pt) const
    {
        SQUARE_POINT pt0;
        pt0.rx() = x() - pt.x();
        pt0.ry() = y() - pt.y();
        return pt0;
    }

    SQUARE_POINT& operator+=(const SQUARE_POINT& pt)
    {
        rx() = x() + pt.x();
        ry() = y() + pt.y();
        return *this;
    }

    SQUARE_POINT& operator+=(const QPointF& pt)
    {
        rx() = x() + pt.x();
        ry() = y() + pt.y();
        return *this;
    }

    void multiply(float v)
    {
        rx() = rx() * v;
        ry() = ry() * v;
    }

    void fromPoint (const QPointF& pt)
    {
        __x = pt.x(), __y = pt.y();
    }
    QPointF toPoint () const
    {
        QPointF pt(__x, __y);
        return pt;
    }

    /*坐标旋转*/
    void rotate (float angle)
    {
        const float sinx = SIN(angle), cosx = COS(angle);
        const float x0 = x(), y0 = y();
        __x = x0 * cosx - y0 * sinx;
        __y = x0 * sinx + y0 * cosx;
    }
    SQUARE_POINT rotated(float angle)
    {
        SQUARE_POINT pt;
        const float sinx = SIN(angle), cosx = COS(angle);
        pt.rx() = x() * cosx - y() * sinx;
        pt.ry() = x() * sinx + y() * cosx;
        return pt;
    }

#if 0
	bool operator==(const SQUARE_POINT& sc1) const
	{	return (EQUAL(sc1.x(), x()) && EQUAL(sc1.y(), y()));	}
	bool operator!=(const SQUARE_POINT& sc1) const
	{	return (!(EQUAL(sc1.x(), x()) && EQUAL(sc1.y(), y())));	}
	bool operator<(const SQUARE_POINT& sc1) const
	{	return (EQUAL(sc1.y(), y()) ? x()<sc1.x() : y()<sc1.y());}
	bool operator<=(const SQUARE_POINT& sc1) const
	{	return (EQUAL(sc1.y(), y()) ? x()<=sc1.x() : y()<sc1.y());}
#else
    friend bool operator==(const SQUARE_POINT& sc1, const SQUARE_POINT& sc2) 
    {	return (EQUAL(sc1.x(),sc2.x()) && EQUAL(sc1.y(),sc2.y()));	}
    friend bool operator!=(const SQUARE_POINT& sc1, const SQUARE_POINT& sc2) 
    {	return !(sc1 == sc2);								}
#endif

    /*
    friend SQUARE_POINT operator-(const SQUARE_POINT& pt1, const SQUARE_POINT& pt2)
    {
        SQUARE_POINT pt0;
        pt0.rx() = pt1.x() - pt2.x();
        pt0.ry() = pt1.y() - pt2.y();
        return pt0;
    }*/
};

class LATITUDE_POINT : public SQUARE_POINT
{
public:
    //LATITUDE_POINT () : SQUARE_POINT() {}
    //LATITUDE_POINT (double l, double ld) : __x((l, ld) {}

    float longitude () const
    {	return x();	}
    float latitude () const
    {	return y();	}
    float& longitude ()
    {	return rx();	}
    float& latitude ()
    {	return ry();	}
};

class RTHETA_POINT : public SQUARE_POINT
{
public:
    //RTHETA_POINT () : SQUARE_POINT() {}
    //RTHETA_POINT (double r, double theta) : SQUARE_POINT(r, theta) {}

    float r () const
    {	return x();	}
    float theta () const
    {	return y();	}
    float& r ()
    {	return rx();	}
    float& theta ()
    {	return ry();	}
};

typedef struct tagCoordinate
{
    LATITUDE_POINT  latitudePoint;  // 经纬度坐标
    RTHETA_POINT    rthetaPoint;    // 极坐标
    SQUARE_POINT    squarePoint;    // 直角坐标坐标
    SCREEN_POINT    screenPoint;    // 屏幕坐标
}COORDINATE;

/*点迹位置*/
typedef struct tagPlotPosition
{
    // 经纬度坐标
    LATITUDE_POINT  latitude_point;
    // 直角坐标
    SQUARE_POINT	square_point;
    // 极坐标
    RTHETA_POINT	rtheta_point;
    // 屏幕坐标
    SCREEN_POINT    screen_point[MAXVIEWCOUNT];
    // 显示标志(其位数需要与最大视图数相关)
    quint8  displayFlag : 4;
    // 更新标志
    quint8  updateFlag  : 4;
}PLOTPOSITION, *LPPLOTPOSITION;

typedef struct tagEnvelope{
    float	_MinX;
    float       _MinY;
    float	_MaxX;
    float	_MaxY;

	tagEnvelope()
		: _MinX(0), _MinY(0), _MaxX(0), _MaxY(0)
	{}
	
    void setEnvelope(float x1, float y1, float x2, float y2)
    {
		if(x1 < x2)
			_MinX = x1, _MaxX = x2;
		else
			_MinX = x2, _MaxX = x1;
		if(y1 < y2)
			_MinY = y1, _MaxY = y2;
		else
			_MinY = y2, _MaxY = y1;
    }

    float width () const
    {	return _MaxX - _MinX;	}
    float height () const
    {	return _MaxY - _MinY;	}

    bool IsInit () const
    {
        return (!(EQUALZERO(_MinX) && EQUALZERO(_MinY) && \
                  EQUALZERO(_MaxX) && EQUALZERO(_MaxY)));
    }

    void Merge (const tagEnvelope& env)
    {
        if (!IsInit())
        {
            *this = env;
        }
        else
        {
            _MinX = qMin (_MinX, env._MinX);
            _MinY = qMin (_MinY, env._MinY);
            _MaxX = qMax (_MaxX, env._MaxX);
            _MaxY = qMax (_MaxY, env._MaxY);
        }
    }

    bool intersected (const tagEnvelope& env)
    {
        return !(_MaxX  < env._MinX || _MaxY < env._MinY || \
            _MinX > env._MaxX || _MinY > env._MaxY);
    }

    bool contains (const SQUARE_POINT& pt) const
    {
        return (pt.x() > _MinX && pt.x() < _MaxX && \
                pt.y() > _MinY && pt.y() < _MaxY);
    }

    bool contains (const SCREEN_POINT& pt) const
    {
        return (pt.x() > _MinX && pt.x() < _MaxX && \
                pt.y() > _MinY && pt.y() < _MaxY);
    }
	
    bool contains (const tagEnvelope& enp) const
    {
        return (enp._MinX >= _MinX && enp._MaxX <= _MaxX && \
			enp._MinY >= _MinY && enp._MaxY <= _MaxY);
    }

    QRectF toRect() const
    {
        return QRectF(_MinX, _MinY, _MaxX-_MinX, _MaxY-_MinY);
    }

}ENVELOPE;

#endif // TARGETMANAGE_GLOBAL_H

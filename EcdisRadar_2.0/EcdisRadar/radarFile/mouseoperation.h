#ifndef MOUSEOPERATION_H
#define MOUSEOPERATION_H


#include "define.h"
#include "TargetManage/transform.h"
#include "TargetManage/TargetManage_global.h"

#include <QCursor>

class MouseOperation
{
public:
    MouseOperation();

////////////////鼠标位置处理///////////////
public:

    // 设置鼠标操作状态
    void setMouseProcessID (MOUSEPROCESSID id);
    // 返回当前鼠标操作状态
    MOUSEPROCESSID mouseProcessID() const
    {   return MouseProcessID;      }


    //鼠标移动处理
    void mouseMoved(const QPoint& sc, bool forceUpdate = false);
    //更新鼠标位置处理
    void updateMousePosition();
    //显示鼠标位置信息
    void mousePositionDisplay();


    //直角坐标转换为极坐标
    QPointF square_to_rtheta(const QPoint& square_point);

    //直角坐标转换为经纬度坐标
    QPointF square_to_latitude(const QPointF& square_point,float *Altitude=NULL);
    //地理坐标为直角坐标
    SQUARE_POINT latitude_to_square(const LATITUDE_POINT& latitude_point,float Altitude=0);

    /*直角坐标转换成屏幕坐标*/
    SCREEN_POINT square_to_screen(const SQUARE_POINT& square_point);
    QPoint square_to_screen(const QPointF& square_point);

    //直角坐标转换成极坐标
    RTHETA_POINT square_to_rtheta(const SQUARE_POINT& square_point);
   // QPointF square_to_rtheta(const QPointF& square_point);

    // 地理坐标转换成屏幕坐标
    SCREEN_POINT latitude_to_screen(const LATITUDE_POINT& ld);
    QPoint latitude_to_screen(const QPointF& ld);

    //屏幕坐标与直角坐标的转换
    QPointF screen_to_square(const QPoint& screen_point);
    //屏幕坐标与经纬度坐标转换
    QPointF screen_to_latitude(const QPoint& sc);

    //极坐标转换为屏幕坐标
    SCREEN_POINT rtheta_to_screen(const RTHETA_POINT& rtheta);
    //极坐标转换为直角坐标
    SQUARE_POINT rtheta_to_square(const RTHETA_POINT& rtheta);

    //地理坐标转换为极坐标
    RTHETA_POINT latitude_to_rtheta(const LATITUDE_POINT& ld)
    {   return square_to_rtheta(latitude_to_square(ld));   }




    //设置中心点的屏幕坐标
    void setCrenterScreenPoint(const QPoint &center)
    {   m_coordinateCenter.screenPoint.setPoint(center.x(), center.y());   }


    QPoint mouseScreenPoint() const
    {   return m_mouseScreenPoint;  }
    QPointF mouseRThetaPoint() const
    {   return m_mouseRThetaPoint;  }
    QPointF mouseLatitudePoint() const
    {   return m_mouseLatitudePoint;    }


    /*设置与公里的转换系数*/
    void setCoefficientToKm(float rate)
    {  m_latitudeTransform.setCoefficientToKm(rate);  }
    float coefficientToKm() const
    {   return m_latitudeTransform.coefficientToKm();   }

    /*设置中心点经纬度坐标*/
    void setCenterLatitude (const LATITUDE_POINT& latitude);


    //用来调用里面的转换函数
    static LatitudeTransform m_latitudeTransform;








private:
    //记录鼠标当前位置
    quint8 m_mouseMovedFlag;

    QPoint m_mouseScreenPoint;
    QPointF m_mouseRThetaPoint;  
    QPointF  m_mouseLatitudePoint;




    COORDINATE  m_coordinateCenter;



private:
    MOUSEPROCESSID MouseProcessID;

};

#endif // MOUSEOPERATION_H

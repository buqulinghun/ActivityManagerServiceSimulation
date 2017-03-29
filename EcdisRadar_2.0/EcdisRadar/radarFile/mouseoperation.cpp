#include "mouseoperation.h"
#include "boatinfo.h"
#include "math.h"
#include "TargetManage/TargetManage_global.h"
#include "glwidget.h"


extern SYSTEMINFO SystemInfo;
extern GLWidget *pView;

//当变量设置为static时，该变量只能在声明它的文件中引用，其他文件中使用需要声明，利用这种特性可以实现模块封装
LatitudeTransform MouseOperation::m_latitudeTransform;

MouseOperation::MouseOperation()
{

    m_coordinateCenter.latitudePoint.setPoint(0.0f, 0.0f);
    m_coordinateCenter.squarePoint.setPoint(0.0f, 0.0f);

    m_coordinateCenter.screenPoint.setPoint(512.0f, 512.0f);

    LATITUDE_POINT ld;
    ld.setPoint(121.57*COEF_DEGREETORADIAN, 29.17*COEF_DEGREETORADIAN);  //经纬度计算为弧度
    setCenterLatitude(ld);
}

void MouseOperation::mouseMoved(const QPoint &sc, bool forceUpdate)
{
    m_mouseScreenPoint = sc;
    m_mouseMovedFlag = 1;
    if(forceUpdate)
        updateMousePosition();
    SystemInfo.iNeedFlag.toUpdateMousePosition = 1;
}

void MouseOperation::updateMousePosition()
{
    if(m_mouseMovedFlag) {
        m_mouseMovedFlag = 0;

        m_mouseRThetaPoint = square_to_rtheta(m_mouseScreenPoint);

        //具体的转换算法要研究下,计算经纬度/////////////////////////////
        QPointF ld = screen_to_square(m_mouseScreenPoint);  //转换为直角坐标，跟旋转向无关
        m_mouseLatitudePoint = m_latitudeTransform.square_to_latitude(ld);

    }
}


void MouseOperation::mousePositionDisplay()
{
    updateMousePosition();

}
//这个直角坐标实际是屏幕坐标
QPointF MouseOperation::square_to_rtheta(const QPoint &square_point)
{
    //x表示半径， y表示角度
    QPointF rtheta;
    QPoint sp;
    float m_rotation = pView->rotation();  //图像选装的角度，需转化为弧度
    //偏心显示时坐标改变,相对角度没有变化
    if(SystemInfo.ViewDisplay.offset)
        sp = QPoint(square_point.x() - pView->m_offsetPoint.x(), square_point.y() - pView->m_offsetPoint.y());
    else
        sp = QPoint(square_point.x() - 512, square_point.y() - 512);

    rtheta.setX((float)sqrt(sp.x()*sp.x() + sp.y()*sp.y()));

    rtheta.setY((float)(M_HALF_PI + atan2((double)sp.y(), (double)sp.x())) - DEGREETORADIAN(m_rotation));
    if(rtheta.y() < 0)
    {
        rtheta.setY(rtheta.y() + M_2PI);
    }
    if(rtheta.y() >= M_2PI)
    {
        rtheta.setY(rtheta.y() - M_2PI);
    }
    return rtheta;
}

/*设置中心点经纬度坐标*/
void MouseOperation::setCenterLatitude (const LATITUDE_POINT& latitude)
{
    if (m_coordinateCenter.latitudePoint == latitude)
        return;

    m_coordinateCenter.latitudePoint = latitude;
    m_latitudeTransform.setCenterLatitude(latitude);
}

QPointF MouseOperation::square_to_latitude(const QPointF &square_point, float *Altitude)
{
    //设置中心经纬度
      return m_latitudeTransform.square_to_latitude(square_point, Altitude);
}

QPointF MouseOperation::screen_to_square(const QPoint &sc)
{
    SQUARE_POINT sq;
    float m_ration = pView->ration();
    float m_rotation = DEGREETORADIAN(pView->rotation());  //图像选装的角度
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

SQUARE_POINT MouseOperation::latitude_to_square(const LATITUDE_POINT &latitude_point, float Altitude)
{
    return m_latitudeTransform.latitude_to_square(latitude_point, Altitude);
}








/************************************
函数名称：square_to_screen
功能：	直角坐标到屏幕坐标的转换
输入参数：1、square_point:屏幕坐标
输出参数：屏幕坐标
***********************************/
SCREEN_POINT MouseOperation::square_to_screen(const SQUARE_POINT& sq0)
{
    SQUARE_POINT sq = sq0;
    float m_ration = pView->ration();
    float m_rotation = DEGREETORADIAN(pView->rotation());   //回波旋转角度
    if(!EQUALZERO(m_rotation))
        sq.rotate(-m_rotation);

    SCREEN_POINT sc;
        sc.rx() = (int)(m_coordinateCenter.screenPoint.x()
               +((sq.x()-m_coordinateCenter.squarePoint.x())*m_ration+0.5) );
        sc.ry() = (int)(m_coordinateCenter.screenPoint.y()
               -((sq.y()-m_coordinateCenter.squarePoint.y())*m_ration-0.5) );

    return (sc);
}

QPoint MouseOperation::square_to_screen(const QPointF& sq0)
{
    SQUARE_POINT sq = {sq0.x(), sq0.y()};
    float m_ration = pView->ration();
    float m_rotation = DEGREETORADIAN(pView->rotation());
    if(!EQUALZERO(m_rotation))
        sq.rotate(m_rotation);

    QPoint sc;
        sc.rx() = (int)(m_coordinateCenter.screenPoint.x()
               +((sq.x()-m_coordinateCenter.squarePoint.x())*m_ration+0.5) );
        sc.ry() = (int)(m_coordinateCenter.screenPoint.y()
               -((sq.y()-m_coordinateCenter.squarePoint.y())*m_ration-0.5) );

    return (sc);
}

/************************************/
/*函数名称： square_to_rtheta		*/
/*功能：	直角坐标到极坐标的转换*/
/*输入参数：square_point:直角坐标	*/
/*输出参数：极坐标				*/
/************************************/
RTHETA_POINT MouseOperation::square_to_rtheta(const SQUARE_POINT& square_point)
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
/*
QPointF MouseOperation::square_to_rtheta(const QPointF& square_point)
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
} */


// 地理坐标转换成屏幕坐标
SCREEN_POINT MouseOperation::latitude_to_screen(const LATITUDE_POINT& ld)
{
    return square_to_screen(m_latitudeTransform.latitude_to_square(ld, 0));
}

QPoint MouseOperation::latitude_to_screen(const QPointF& ld)
{
    return square_to_screen(m_latitudeTransform.latitude_to_square(ld, 0));
}


// 屏幕坐标转换成地理坐标
QPointF MouseOperation::screen_to_latitude(const QPoint& sc)
{
    return square_to_latitude(screen_to_square(sc));
}


SQUARE_POINT MouseOperation::rtheta_to_square(const RTHETA_POINT& rtheta)
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
//极坐标转换为屏幕坐标
SCREEN_POINT MouseOperation::rtheta_to_screen(const RTHETA_POINT& rtheta)
{
    return square_to_screen(rtheta_to_square(rtheta));
}




// 设置鼠标操作状态
void MouseOperation::setMouseProcessID (MOUSEPROCESSID id)
{
    if ((MouseProcessID != id))
    {
        //if(MouseProcessID != id && id == NULL_PROCESS)
          //  ClearOperate();

        MouseProcessID = id;

       // SetCursorByProcessID(id);
    }
}






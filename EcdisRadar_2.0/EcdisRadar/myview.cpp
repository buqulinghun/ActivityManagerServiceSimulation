#include "myview.h"
#include "mainwindow.h"
#include "ecdis.h"

#include <QWheelEvent>

extern MainWindow* lpMainWindow;
extern DataBase *dataBase;
extern MARINERSELECT MarinerSelect;



MyView::MyView(QWidget *parent) :
    QGraphicsView(parent),center(0,0),old_center(0,0)
{

}

void MyView::drawForeground(QPainter *painter, const QRectF &rect)
{
 /*   QColor color = QColor(Qt::red);
    painter->setPen(color);

    QString text;
    if(number % 2)
        text = "CQUPT";
    else
        text = "WANGMINGXIAO";
    QFont font  = painter->font();
    font.setPixelSize(100);
    QFontMetrics fm(font);
    const int width1 = fm.width(text);
    const int width = this->width();
    painter->drawText((width-width1)/2, height()/2, text);  */
}

void MyView::wheelEvent(QWheelEvent *event)
{
    //比例尺匹配后不能使用放大缩小
    if(MarinerSelect.scaleMatch)    return;
    int numSteps = event->delta() / 120;
    if(numSteps > 0)   //如果鼠标滚轮远离使用者，则delta()返回正值，当值大于120时再执行
    {
       // center = this->mapToScene(this->viewport()->size().width() / 2,this->viewport()->size().height() / 2);
       // center /= 1.2;
        emit signal_scale(1/1.2);
    }else if(numSteps < 0) {
      //  center = this->mapToScene(this->viewport()->size().width() / 2,this->viewport()->size().height() / 2);
       // center *= 1.2;
        emit signal_scale(1.2);
    }
}


void MyView::paintEvent(QPaintEvent *event)
{
    center = QPointF(centerLongScreen, centerLatScreen);
    if(center != old_center){
        old_center = center;
        this->centerOn(center);
   }
    QGraphicsView::paintEvent(event);

}

void MyView::mouseMoveEvent(QMouseEvent *event)
{  
    //因为y轴是向下增长，所以需要对其进行转换，变为向上增长才行
    QPointF scenePoint = mapToScene(QPoint(event->x(), event->y()));
    double hei  = scene()->height();
    QPointF lonlat = dataBase->screenToLatitude(scenePoint.x(), -(scenePoint.y()));
    lpMainWindow->setCoordinateDisp(event->x(), event->y(), lonlat.x(), lonlat.y());

    QGraphicsView::mouseMoveEvent(event);
}



















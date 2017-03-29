#ifndef MYVIEW_H
#define MYVIEW_H

#include <QGraphicsView>

class MyView : public QGraphicsView
{
Q_OBJECT
public:
    explicit MyView(QWidget *parent = 0);

protected:
    void wheelEvent(QWheelEvent *event);
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);


    void drawForeground(QPainter *painter, const QRectF &rect);

public slots:

signals:
    void signal_scale(float);

private:
    QPointF center,old_center;

};

#endif // MYVIEW_H

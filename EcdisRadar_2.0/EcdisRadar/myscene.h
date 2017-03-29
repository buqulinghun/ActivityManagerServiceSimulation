#ifndef MYSCENE_H
#define MYSCENE_H

#include <QGraphicsScene>
#include <QGraphicsItemGroup>
#include <QDateTime>

#include "chart.h"
#include "symbols.h"
#include "conditionalitem.h"




class MyScene : public QGraphicsScene
{
Q_OBJECT
public:
    MyScene(QObject *parent = 0);

    void initData(QVector<Chart *> *data);   //初始化绘图数据

    QString currentChartName() const
    {  return nowChartName;    }

    void createRadarImage(void);   //创建雷达图像
    //@todo  设置雷达图像的位置，在这里可以对雷达图像的位置进行矫正
    void moveRadarToCenter(void);   //将雷达作为视图中心显示
    void calculateMatchScale(void);    //根据雷达量程计算海图比例尺


protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

    //绘制单幅海图
    void Render(Chart* chart);
    //得到每个物标的符号化指令
    void GenerateInstruction(Chart* chart, bool area, bool line, bool point);
    //解析符号化指令
    void parseLookupTable(const std::vector<AreaObject>::iterator &object, const std::vector<sg2d_t> &sg2ds, const QMap<QString, Rgb> &colorTable);
    void parseLookupTable(const std::vector<LineObject>::iterator &object, const std::vector<sg2d_t> &sg2ds, const QMap<QString, Rgb> &colorTable);
    void parseLookupTable(const std::vector<PointObject>::iterator &object, const std::vector<sg2d_t> &sg2ds, const QMap<QString, Rgb> &colorTable);

    bool comparePertime(const QString &persta, const QString &perend);

public slots:
    void renderChart(std::string chart_description);   //第一次绘制海图
    void refreshScreen(QString nowChart, bool area, bool line, bool point);  //配置改变刷新海图
    void slot_scale(float);  //比例尺缩放

private:
    ConditionalItem *conditionalItems;   //条件物标处理

    QVector<Chart *> *openChartData;   //指向需要显示的海图数据


    QDateTime nowTime;   //当前时间
    QString nowChartName;   //当前显示的海图名字
    Chart *nowChart;  //当前绘制海图


    //各类物标分组
    QGraphicsItemGroup *radarAllItems;
    QGraphicsItemGroup *chartAllItemsUnderRadar;     //雷达图像下面
    QGraphicsItemGroup *chartAllItemsOverRadar;   //雷达图像上面


};

#endif // MYSCENE_H

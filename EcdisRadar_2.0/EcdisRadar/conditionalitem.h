

/******************************************
void fun() const;
必须是类的成员函数，不能是单独的类外部函数
其次，如果为类成员函数，他表示
（1）不能修改成员变量；
（2）不能调用非const函数；
（3）其他const函数可以调用它；（其他非const函数当然也可以调用它）
（4）只能从const对象（或引用用指针）上调用，比如：const   A   a;   a.f();
*******************************************/

#ifndef CONDITIONALITEM_H
#define CONDITIONALITEM_H

#include "chart.h"
#include "configuration.h"


#include <QString>
#include <QPainterPath>

//在对应的查询表中DATCVR02, DEPARE02 WRECKS04 DEPCNT03变为DATCVR01, DEPARE01 WRECKS02  DEPCNT02??
//RESARE03--RESARE02   OBSTRN06--OBSTRN04
enum conditionalName{    //条件符号枚举
    CLRLIN01  = 1,
    DATCVR01, DEPARE01, DEPCNT02, DEPVAL02, LEGLIN03,
    LIGHTS05, LITDSN01, OBSTRN04, OWNSHP02, PASTRK01,
    QUAPOS01, QUALIN01, QUAPNT02, RESARE02, RESTRN01,
    RESCSP02, SAFCON01, SLCONS03, SEABED01, SNDFRM03,
    SOUNDG02, SYMINSnn, TOPMAR01, UDWHAZ04, VESSEL02,
    VRMEBL02, WRECKS02
};



class ConditionalItem
{
public:
    ConditionalItem();

    void setNowChart(Chart *chart)
    {    nowChart = chart;       }


    //条件处理外部接口
    QStringList dealConditionalAreaItem(const QString &name, std::vector<AreaObject>::iterator &object) const;
    QStringList dealConditionalLineItem(const QString &name, std::vector<LineObject>::iterator &object) const;
    QStringList dealConditionalPointItem(const QString &name, std::vector<PointObject>::iterator &object) const;




    //区域型
    QStringList setup_DATCVR01(const std::vector<AreaObject>::iterator &object) const;
    QStringList setup_DEPARE01(const std::vector<AreaObject>::iterator &object) const;
    QStringList setup_RESARE02(const std::vector<AreaObject>::iterator &object) const;
    QStringList setup_RESTRN01(const std::vector<AreaObject>::iterator &object) const;
    QStringList setup_WRECKS02_area(std::vector<AreaObject>::iterator &object) const;
    QStringList setup_OBSTRN04_area(std::vector<AreaObject>::iterator &object) const;

    //线型
    QStringList setup_DATCVR01(const std::vector<LineObject>::iterator &object) const;
    QStringList setup_DEPCNT02(const std::vector<LineObject>::iterator &object) const;
    QStringList setup_QUAPOS01_Line(const std::vector<LineObject>::iterator &object) const;
    QStringList setup_OBSTRN04_line(std::vector<LineObject>::iterator &object) const;

    //点物标
    QStringList setup_DATCVR01(const std::vector<PointObject>::iterator &object) const;
    QStringList setup_QUAPOS01_Point(const std::vector<PointObject>::iterator &object) const;
    QStringList setup_LIGHTS05(const std::vector<PointObject>::iterator &object) const;
    QStringList setup_WRECKS02_point(std::vector<PointObject>::iterator &object) const;
    QStringList setup_TOPMAR01(const std::vector<PointObject>::iterator &object) const;
    QStringList setup_SOUNDG02(const std::vector<SoundObject>::iterator &object) const;
    QStringList setup_OBSTRN04_point(std::vector<PointObject>::iterator &object) const;

    //被外部水深点调用
    QStringList setup_SNDFRM03(float depthValue, const int &quapos, std::vector<attf_t> attfs) const;   //不是独立的，返回几个深度数字图像名称

protected:
    QString setup_SEABED01(float &drval1, float &drval2, bool &fill) const;   //被setup_DEPARE01()函数调用的子程序，不是独立的
    QString setup_RESCSP02(const QString &restrn) const;   ////被setup_DEPARE01()函数调用的子程序，不是独立的


    //下面四个函数都没有处理完全，因为有面与面物标交叉的问题没解决************************************
    bool setup_UDWHAZ04_point(std::vector<PointObject>::iterator &object, const float &depthValue, const int &watlev) const;   //不是独立的,返回是否为独立的危险区域
    bool setup_UDWHAZ04_area(std::vector<AreaObject>::iterator &object, const float &depthValue, const int &watlev) const;
    void setup_DEPVAL02_point(const std::vector<PointObject>::iterator &object, const int &expsou, const int &watlev, float &leastDepth, float &seabedDepth) const;   //不是独立程序，返回后面两个参数
    void setup_DEPVAL02_area(const std::vector<AreaObject>::iterator &object, const int &expsou, const int &watlev, float &leastDepth, float &seabedDepth) const;
    void setup_DEPVAL02_line(const std::vector<LineObject>::iterator &object, const int &expsou, const int &watlev, float &leastDepth, float &seabedDepth) const;
    
    //点处理与setup_QUAPOS01_Point一样
    QStringList setup_QUAPNT02_area(const std::vector<AreaObject>::iterator &object) const;

    QStringList setup_SAFCON01(const float &depthValue) const;

private:
    void setupConditionalNameMap();   //建立函数名对应表
    QMap<QString, int> conditional_name_num_map;   //存储条件物标对应的名称

    Chart *nowChart;   //当前处理的海图
};

#endif // CONDITIONALITEM_H

#ifndef MERCATORPROJ_H
#define MERCATORPROJ_H


#include <stdio.h>
#include <math.h>

#include "chart.h"



//圆周率
const double PI = 3.14159265359;
const double PI_4 = 0.7853981633975;
const double PI_2 = 1.570796326795;


#ifndef M_2PI
#define M_2PI 6.28318530718
#endif

//角度于弧度转换
#define COEF_RADIANTODRGREE 57.2957795131
#define COEF_DEGREETORADIAN 0.0174532925   //M_2PI/360
#define DEGREETORADIAN(a)  (((double)a) * ((double)COEF_DEGREETORADIAN))
#define RADIANTODEGREE(a)  (((double)a) * ((double)COEF_RADIANTODRGREE))






class MercatorProj
{
public:
    MercatorProj();


    /* 设定各类参数  */
    void SetAB(double a, double b);
    void SetLat_base(double b);

    /*转换程序  */
    int ToProjection(double B,double L,double &X,double &Y);
    int FromProjection(double X,double Y,double &B,double &L);

    //转换一幅海图所有坐标，暂时不考虑三维坐标
    bool ChartMercatorProj(Chart *chart);
    //根据平面直角坐标得到屏幕坐标
    void ConvertToScreen(Chart *chart);


    QPointF screenToLatitude(double x, double y, Chart *chart);     //直角坐标为场景的坐标,不经过放大缩小的，否则需要根据缩放因子求得第一次转换后的坐标


private:
    //需要更改的参数只有参考纬度，经度一直为0，椭球体采用我国采用的半径长度米6378137,米6356752.31414

    int IterativeTimes;	//反向转换程序中的迭代次数
    double IterativeValue;	//反向转换程序中的迭代初始值
    double A_;	//椭球体长半轴,
    double B_;	//椭球体短半轴,
    double Lat_base; //标准纬度,弧度


    double e/*第一偏心率*/;
    double e_/*第二偏心率*/;
    double K;   //转换成平面坐标参数

};

#endif // MERCATORPROJ_H

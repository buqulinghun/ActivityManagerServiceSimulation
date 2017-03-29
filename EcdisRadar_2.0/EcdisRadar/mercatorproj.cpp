#include "mercatorproj.h"
#include "ecdis.h"

#include <QPointF>

extern MARINERSELECT MarinerSelect;


MercatorProj::MercatorProj()
{
    IterativeTimes = 3;	                 //迭代次数为10
    IterativeValue = 0.0;		//迭代初始值

    SetLat_base(DEGREETORADIAN(30.0));  //特殊时刻更改参考纬度

    SetAB(6378137, 6356752.31414);      //设定长半径短半径,先修改参考纬度

}


void MercatorProj::SetAB(double a, double b)
{
    double dtemp;
    double NB0;    /*卯酉圈曲率半径*/

    if(a<=0||b<=0)
    {    return;    }
    A_ = a;
    B_ = b;

    dtemp =1-(B_/A_)*(B_/A_);
    if(dtemp<0)
    {
        return;
    }
    e = sqrt(dtemp);   //求得第一偏心率

    dtemp=(A_/B_)*(A_/B_)-1;
    if(dtemp<0)
    {
        return;
    }
    e_ = sqrt(dtemp);    //求得第二偏心率

   // NB0 = ((A_*A_)/B_)/sqrt(1+e_*e_*cos(Lat_base)*cos(Lat_base));
    NB0 = A_ / sqrt(1 - e*e*sin(Lat_base)*sin(Lat_base));

    K = NB0 * cos(Lat_base);

}

void MercatorProj::SetLat_base(double b)
{
    if(b < -PI/2 || b > PI/2)
     {
        return;
      }
     Lat_base = b;
}


/*******************************************
投影正向转换程序
double B:	维度,弧度
double L:	经度,弧度
double& X:	纵向直角坐
double& Y:	横向直角坐
*******************************************/
int MercatorProj::ToProjection(double B,double L,double &X,double &Y)
{

        if(L<-PI||L>PI||B<-PI/2||B>PI/2)
        {
            return 1;
        }


        Y = K * L;

        //这里可以增加sin(B)的值的判定
        const double esinb = e * sin(B);
        X = K * log(tan(PI_4 + B/2) * pow((1 - esinb) / (1 + esinb), e/2));

        return 0;
}
/*******************************************
投影反向转换程序
double X:	纵向直角坐
double Y:	横向直角坐
double& B:	维度,弧度
double& L:	经度,弧度
*******************************************/
int MercatorProj::FromProjection(double X,double Y,double &B,double &L)
{
    double E=exp(1);

    L = Y / K;

    B = IterativeValue;
    for(int i=0; i<IterativeTimes; i++)
    {
        const double esinb = e * sin(B);
        B = PI_2 - 2 * atan(pow(E, (-X / K)) * pow(E, (e/2) * log((1-esinb)/(1+esinb))));
    }

    return 0;
}

/*墨卡托投影变换，得到是以米为单位的全球坐标*/
bool MercatorProj::ChartMercatorProj(Chart *chart)
{

    std::vector<sg2d_t>::iterator sg2dit;
    std::vector<sg2d_t>::iterator sg2dend = chart->sg2ds.end();
    for (sg2dit = chart->sg2ds.begin(); sg2dit != sg2dend; ++sg2dit) {
        //先纬度后经度,先纵坐标后横坐标
        const QPointF longlat(sg2dit->long_lat[0].minutes, sg2dit->long_lat[1].minutes);
        ToProjection(DEGREETORADIAN(longlat.y()), DEGREETORADIAN(longlat.x()), sg2dit->long_lat[1].minutes, sg2dit->long_lat[0].minutes);
      //  sg2dit->long_lat[0] = (sg2dit->long_lat[0] * factorToScreen);
      //  sg2dit->long_lat[1] = (sg2dit->long_lat[1] * factorToScreen);
    }

    //转换水深点经纬度
    std::vector<SoundObject>::iterator so_it;
    std::vector<SoundObject>::iterator so_end = chart->sound_object_vector.end();
    for(so_it = chart->sound_object_vector.begin(); so_it != so_end; ++so_it) {
        std::vector<sg3d_t>::iterator it;
        std::vector<sg3d_t>::iterator end = so_it->sg3ds.end();
        for(it = so_it->sg3ds.begin(); it != end; ++it) {
            const QPointF longlat(it->long_lat[0], it->long_lat[1]);
            ToProjection(DEGREETORADIAN(longlat.y()), DEGREETORADIAN(longlat.x()), it->long_lat[1], it->long_lat[0]);
          //  it->long_lat[0] = (it->long_lat[0] * factorToScreen);
           // it->long_lat[1] = (it->long_lat[1] * factorToScreen);
        }

    }
    //转换边缘坐标
    const QPointF left_longlat(chart->wlon.minutes, chart->slat.minutes);
    ToProjection(DEGREETORADIAN(left_longlat.y()), DEGREETORADIAN(left_longlat.x()), chart->Mslat, chart->Mwlon);
    const QPointF right_longlat(chart->elon.minutes, chart->nlat.minutes);
    ToProjection(DEGREETORADIAN(right_longlat.y()), DEGREETORADIAN(right_longlat.x()), chart->Mnlat, chart->Melon);

    return true;
}


/*将投影结果转换为像素，初始显示比例尺设为100000,1英寸等于0.0254米,dpi为每英寸96像素,即每厘米的像素值为96/2.54 */
/*一米包含的像素值为 1*100*(96/2.54) = 3779.5275552   */
/*根据具体的比例尺来得到海图经纬度坐标的屏幕显示坐标*/
void MercatorProj::ConvertToScreen(Chart *chart)
{   
    const int chartScale = MarinerSelect.chartScaleNum;
    const double factorToScreen = (double)3779.527552 / chartScale;

    std::vector<sg2d_t>::iterator sg2dit;
    std::vector<sg2d_t>::iterator sg2dend = chart->sg2ds.end();
    for (sg2dit = chart->sg2ds.begin(); sg2dit != sg2dend; ++sg2dit) {
            sg2dit->long_lat[0] = (sg2dit->long_lat[0].minutes - chart->Mwlon) * factorToScreen;
            sg2dit->long_lat[1] = -(sg2dit->long_lat[1].minutes - chart->Mnlat) * factorToScreen;
        }

    std::vector<SoundObject>::iterator so_it;
    std::vector<SoundObject>::iterator so_end = chart->sound_object_vector.end();
    for(so_it = chart->sound_object_vector.begin(); so_it != so_end; ++so_it) {
        std::vector<sg3d_t>::iterator it;
        std::vector<sg3d_t>::iterator end = so_it->sg3ds.end();
        for(it = so_it->sg3ds.begin(); it != end; ++it) {
            it->long_lat[0] = (it->long_lat[0] - chart->Mwlon) * factorToScreen;
            it->long_lat[1] = -(it->long_lat[1] - chart->Mnlat) * factorToScreen;
        }
    }

    //边缘经纬度转换为屏幕坐标
    chart->Mslat *= factorToScreen;
    chart->Mwlon *= factorToScreen;
    chart->Mnlat *= factorToScreen;
    chart->Melon *= factorToScreen;
}

QPointF MercatorProj::screenToLatitude(double x, double y, Chart *chart)
{
    if(!chart)  return QPointF(0,0);
    //场景中的直角坐标转化为墨卡托转换之后的地球统一直角坐标
    const int chartScale = MarinerSelect.chartScaleNum;
    const double factorToScreen = (double)chartScale / 3779.527552;

    x /= chart->absolute_zoom_factor;
    y /= chart->absolute_zoom_factor;

    x *= factorToScreen;
    y *= factorToScreen;

    x += chart->Mwlon;
    y += chart->Mnlat;

    double lon,lat;
    FromProjection(y, x, lat, lon);   //转换出来是弧度

    lon = RADIANTODEGREE(lon);
    lat = RADIANTODEGREE(lat);

    return QPointF(lon, lat);
}



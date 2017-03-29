/********************************************************************
 *日期: 2016-02-24
 *作者: 王名孝
 *作用: 全局设置变量定义
 *修改: 默认显示低精度位置标记...
 ********************************************************************/
#ifndef ECDIS_H
#define ECDIS_H

#include "configuration.h"


//#define SOFTWAREPATH QString("/home/Ecdis/Ecdis_2/")
#define SOFTWAREPATH QString("/usr/project/EcdisRadar_2.0/EcdisRadar/")


extern double centerLong;   //海图中心经纬度
extern double centerLat;
extern float centerLongScreen;
extern float centerLatScreen;



typedef struct marinerSlect{
    float SHALLOW_CONTOUR;
    float DEEP_CONTOUR;
    float SAFETY_CONTOUR;
    bool TWO_SHADES;       //海上区域显示四层颜色
    bool SHALLOW_PATTERN;  //浅滩加图案标记
    bool FULL_LIGHTS;    //灯光线长全部显示
    bool SHOW_ISOLATEDDANGE;   //显示孤立危险点

    bool simplePoint;   //简单符号，传统纸海图符号
    bool plainArea;     //简单化区域边，符号化区域边
    bool languageSelect;   //语言选择
    bool radarShow;   //雷达图像是否显示
    bool scaleMatch;   //雷达与海图比例尺锁定匹配

    int chartScaleNum;    //海图比例尺
    int dayState;        //颜色环境选择
    int dispCategory;    //显示类别选择



    //初始化
    marinerSlect()
    {
        SHALLOW_CONTOUR = 2.0;
        DEEP_CONTOUR   = 30.0;
        SAFETY_CONTOUR  = 5.0;
        TWO_SHADES  = false;
        SHALLOW_PATTERN =  false;
        FULL_LIGHTS = false;
        SHOW_ISOLATEDDANGE = true;

        simplePoint = true;
        plainArea = false;
        languageSelect = false;  //中文
        radarShow = false;
        scaleMatch = false;
        chartScaleNum = 100000;
        dayState = DAY_BRIGHT;
        dispCategory = STANDARD;
    }

}MARINERSELECT;



//距离制式
enum {
    UNITS_KM = 0,  //公里
    UNITS_NM,     //海里
    UNITS_SM,    //英里
    UNITS_MAX
};

//量程和刻度线相关信息
typedef struct tagRangeScale
{
#define MAXRNGIDX 19

    float maxRange[UNITS_MAX][MAXRNGIDX];  //最大量程
    float scaleLine[UNITS_MAX][MAXRNGIDX];  //刻度线   每种距离制式都保留
    float rateToKm[UNITS_MAX];   //对公里对转换率
    quint8 units;   //单位：0 公里  1 海里 2 英里
    quint8 rngIndex1;  //量程索引
    quint8 rngIndex2;  //量程索引

    tagRangeScale()  //结构体的构造函数
    {
        //量程和刻度线设置
        float rng[MAXRNGIDX][2] = {{0.125,0.025}, {0.25,0.05}, {0.5,0.1}, {0.75,0.15}, {1.0,0.2}, {1.5,0.25}, {2,0.5}, \
        {3,0.5}, {4,1.0}, {6,1.0}, {8,2.0}, {12,2.0}, {16,4.0}, {24,4.0}, {36,6.0}, {48,8.0}, {64.0,10.0}, {72,12.0}, {96.0,16.0}};
        for(quint8 i=0; i<MAXRNGIDX; i++) {
            maxRange[0][i] = rng[i][0];
            scaleLine[0][i] = rng[i][1];
            maxRange[1][i] = rng[i][0];
            scaleLine[1][i] = rng[i][1];
            maxRange[2][i] = rng[i][0];
            scaleLine[2][i] = rng[i][1];

        }

        rateToKm[0] = 1.0;
        rateToKm[1] = 1.852;  //1海里=1.852km
        rateToKm[2] = 1.609;  //1英里=1.609km

        units = 1;
        rngIndex1 = rngIndex2 = 5;
    }

    quint8 rngIndex(bool flag = true) const
    {  return (flag ? rngIndex1 : rngIndex2);  }

    //设置距离单位,如果变化返回真，没有变化返回假
    bool setUnits(quint8 val)
    {
        if((val < UNITS_MAX) && (val != units)) {
            units = val;
            return true;
        }
        return false;
    }
    //距离单位
   /* QString unitsText() const
    {   return unitsDisplayText(units);   }  */

    //距离单位对应公里的转换系数
    float coefficientToKm() const
    {  return rateToKm[units];  }
    //获取最大量程
    float range(bool flag = true) const
    {  return flag ? maxRange[units][rngIndex1] : maxRange[units][rngIndex2];  }
    float scale(bool flag = true) const
    {  return flag ? scaleLine[units][rngIndex1] : scaleLine[units][rngIndex2];  }

    //更改量程
    bool changRange(bool dir, bool flag = true) {
        return dir ? incRange(flag) : decRange(flag);
    }
    bool setRange(quint8 index) {
        rngIndex1 = rngIndex2 = index;
        return true;
    }

    //增加量程，量程变化返回真，没有变化返回假
    bool incRange(bool flag = true)
    {
        if(flag) {
            if(rngIndex1 < MAXRNGIDX-1) {
                rngIndex1++;
                qDebug()<<"range inc";
                return true;
            }
        }else {
            if(rngIndex2 < MAXRNGIDX-1) {
                rngIndex2++;
                return true;
            }
        }
        return false;
    }
    //减少量程，量程变化返回真，没有变化返回假
    bool decRange(bool flag = true)
    {
        if(flag) {
            if(rngIndex1 > 0) {
                rngIndex1--;
                qDebug()<<"range dec";
                return true;
            }
        }else {
            if(rngIndex2 > 0) {
                rngIndex2--;
                return true;
            }
        }
        return false;
    }

#undef MAXRNGIDX

}RANGESCALE;


//雷达系统信息
typedef struct SYSTEMINFO
{
    bool transmite;   //发射标志
    bool offset;     //偏心
    RANGESCALE RangeScale;  //量程设置
    quint8 antennaSelect;  //每次开机都要发送

    quint8 gainNum;   //增益值
    quint8 restrainNum;  //雨雪值
    quint8 clutterNum;  //海浪值
    quint32 bestTuneValue[19];  //调谐的最佳值
    quint16 bestTuneAdd[19];   //增加的值
    quint16 tuneManValue;   //手动调谐值
    int  rngAdjust;   //距离调节值


    SYSTEMINFO(){
        transmite = false;
        offset = false;
       // memset(&RangeScale, 0, sizeof(RangeScale));
        antennaSelect = 0;
        gainNum = 80;
        restrainNum = 0;
        clutterNum = 0;
        memset(bestTuneValue, 0, 19*sizeof(quint32));
        memset(bestTuneAdd, 0, 19*sizeof(quint16));
        tuneManValue = 490;
        rngAdjust = 50;
    }

}SYSTEMINFO;


































#endif // ECDIS_H

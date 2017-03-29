#ifndef DATAPROCESS_H
#define DATAPROCESS_H


#include <QByteArray>
#include "define.h"

//信号点迹结构
typedef struct SIGNAL_PLOT
{
     quint16 Range;                          /*目标距离*/
    quint16 Azimuth;                        /*目标角度*/
    quint16 RangeWidth;             /*目标距离跨度*/
    quint16 AziWidth;                    /*目标角度跨度*/
}SIGNAL_PLOT;
//接收信号点迹帧结构
typedef struct RI_SIGNAL_PLOT
{
    quint32 plotLength;                          /*点迹数量*/
    SIGNAL_PLOT PlotData[10];            /*目标数据*/
}RI_SIGNAL_PLOT;



class DataProcess
{
public:
    DataProcess();

    //网络数据处理
    void dataProcess(const QByteArray& data);




protected:
    void addSimulatePlot(int flag);

private:
        quint16 old_tune;  //用来判断是否需要刷新显示，减少函数跳转次数

        ECHODATA m_data;  //回波数据结构
        RI_SIGNAL_PLOT target_data;   //目标数据结构

        quint32 maxNum;

};

#endif // DATAPROCESS_H

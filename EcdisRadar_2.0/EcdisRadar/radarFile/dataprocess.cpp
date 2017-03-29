#include "dataprocess.h"
#include "radaritem.h"
#include "../ecdis.h"
#include "interact.h"



#define  fusion(a, b)  ((quint16)(a<<8) + (quint8)b)
#define NmToKm 1.852



extern RadarItem *lpRadarItem;
extern DataProcess *lpDataProcess;
extern SYSTEMINFO RadarSysinfo;
extern Interact * lpInteract;


DataProcess::DataProcess() : old_tune(0)
{
    maxNum = 0;
}


void DataProcess::dataProcess(const QByteArray &data)
{


    const quint8 flag = data.at(0);

    if(flag == 0x0f){   //标志0x0f
        //记录原始数据
        //if(lpRecplay && lpRecplay->isRecording())
          //  lpRecplay->recording(data.constData(), data.length(), Device_FPGA);

        //回波数据处理
        const int m_length = data.length()-10;    //长度减去量程和角度的长度得到真实数据的长度
        const quint16 range_tune = quint16(data[0+1]<<8)+quint8(data[1+1]); //量程和调谐值
        const quint16 m_angle = quint16(data[4+1]<<8)+quint8(data[5+1]); //角度

        const quint16 m_range = ((range_tune>>11) & 0x001f);
        const quint16 m_tune = (range_tune & 0x07ff);  //调谐值
        const quint16 m_packetNum = quint16(data[6+1]<<8)+quint8(data[7+1]);  //发送包数标志
     //   const quint8  m_warning = quint8(data[8+1]);
    /*    if((quint8)(m_warning & 0x80)) {
            if(m_warning != antennaWarning)
                antennaWarning = m_warning;
        } */
        //调谐值格式：2位标志，9位数值，最大512   11表示全程最佳值，10表示单程最佳值，00正常显示值
        //全程扫描的三个量程0.5，2，8，对应的索引：2，6，10
        const quint16 flag_tune = (m_tune & (quint16)0x0600);
        switch(flag_tune) {
            case 0x0600://全程自动扫描最佳值，保存
                {
                   quint16 tuneAdd = quint16(data[2+1]<<8)+quint8(data[3+1]);
                    quint16 tune = (m_tune & (quint16)0x01ff);
                    if(m_range == 2) {  //0.125-1    0-4
                        RadarSysinfo.bestTuneValue[0] = tune;
                        RadarSysinfo.bestTuneAdd[0] = tuneAdd;
                        RadarSysinfo.bestTuneValue[1] = tune;
                        RadarSysinfo.bestTuneAdd[1] = tuneAdd;
                        RadarSysinfo.bestTuneValue[2] = tune;
                        RadarSysinfo.bestTuneAdd[2] = tuneAdd;
                        RadarSysinfo.bestTuneValue[3] = tune;
                        RadarSysinfo.bestTuneAdd[3] = tuneAdd;
                        RadarSysinfo.bestTuneValue[4] = tune;
                        RadarSysinfo.bestTuneAdd[4] = tuneAdd;
                        lpInteract->change_tune(1, 10);  //发送关闭命令，就是正常自动扫描的命令
                    }else if(m_range == 6) {  //1.5-3   5-7
                        RadarSysinfo.bestTuneValue[5] = tune;
                        RadarSysinfo.bestTuneAdd[5] = tuneAdd;
                        RadarSysinfo.bestTuneValue[6] = tune;
                        RadarSysinfo.bestTuneAdd[6] = tuneAdd;
                        RadarSysinfo.bestTuneValue[7] = tune;
                        RadarSysinfo.bestTuneAdd[7] = tuneAdd;
                    }else if(m_range == 10) {  //4-96  8-18
                        for(int i=8; i<19; i++){
                            RadarSysinfo.bestTuneValue[i] = tune;
                            RadarSysinfo.bestTuneAdd[i] = tuneAdd;
                        }
                    }
                    qDebug()<< "range and best tune"<< m_range<< RadarSysinfo.bestTuneValue[m_range]<< RadarSysinfo.bestTuneAdd[m_range];

                }
                break;
            case 0x0400:  //单量程最佳值,保存并显示
                {
                  //  lpMainWindow->setManTuneValue(m_tune & (quint16)0x01ff);
                }
                break;
            default:  //正常显示值,00
                {
                  //  if(m_tune != old_tune)
                 //       pView->change_tune_disp(m_tune);
                //    old_tune = m_tune;
                }
                break;
        }


        //对结构体赋值
        m_data.length = m_length;
        m_data.range = m_range;
        m_data.angle = m_angle;
        m_data.packetNum = m_packetNum;
        if(m_packetNum > maxNum)
            maxNum = m_packetNum;

        for(int i=0; i<m_length; i++) {
            m_data.echo[i] = quint8(data[i+9+1]);
        }

        //将数据赋值显示
        if(lpRadarItem)
            lpRadarItem->setEchoData(m_data);

    }else {   //标志为0xf0

        //当显示ARPA目标时再处理
      /*  extern quint8 gAtaAisSwitch;
        if((!gAtaAisSwitch) || (!MenuConfig.dispMenu.showArpa))
            return;

        //点迹目标处理
        //每个量程的径向数据
        const float rnguints[19] = {78, 155, 310, 465, 310, 465, 414, 465, 496, 465, 496, 496, 496, 496, 496, 496, 496, 485, 496};
        const float rngnum[19] = {0.125, 0.25, 0.5, 0.75, 1, 1.5, 2, 3, 4, 6, 8, 12, 16, 24, 36,48, 64, 72, 96 };

        target_data.plotLength = data[1];   //后8位为目标位数

        PLOTINFO plotInfo;
        const quint8 x = SystemInfo.RangeScale.rngIndex();
        const int aziAdjust = pView->getAziAdjustValue();

        for(int i=0; i < target_data.plotLength; i++) {
            target_data.PlotData[i].Azimuth = fusion(data[4*i + 4], data[4*i + 5]);
            target_data.PlotData[i].Range = fusion(data[4*i + 2], data[4*i + 3]);
             plotInfo.type = PLOT_TYPE_PRIMARY;
             plotInfo.aziWidth = 1;
             plotInfo.rngWidth = 1;



             RTHETA_POINT rtheta;
             rtheta.rx() =  target_data.PlotData[i].Range / rnguints[x] * rngnum[x] * NmToKm;   //需要更改，根据每个量程计算,单位为km
             rtheta.ry() = 2 * M_PI * (target_data.PlotData[i].Azimuth + aziAdjust)  / 3600;
             plotInfo.rtheta_point = rtheta;
             plotInfo.extraData = 0;
             setTargetPlot(plotInfo);

        }

   */
    }




}



void DataProcess::addSimulatePlot(int flag)
{
 /*   PLOTINFO plotInfo;

    for(int i=0; i < flag; i++) {
         plotInfo.type = PLOT_TYPE_PRIMARY;
         plotInfo.aziWidth = 1;
         plotInfo.rngWidth = 1;
         RTHETA_POINT rtheta;
         rtheta.rx() = 0.4 * i;   //需要更改，根据每个量程计算
         rtheta.ry() = 2 * M_PI *(500 + i * 100) / 3600;
         plotInfo.rtheta_point = rtheta;
         plotInfo.extraData = 0;
         setTargetPlot(plotInfo);


         const QString text = QString("primary  :  flat:%1,R:%2,A:%3").arg(i).arg(plotInfo.rtheta_point.r()).arg(plotInfo.rtheta_point.theta() * 180.0 / M_PI);
         qDebug() << text;
    }

    for(int i=0; i < flag; i++) {
         plotInfo.type = PLOT_TYPE_FILTER;
         plotInfo.aziWidth = 1;
         plotInfo.rngWidth = 1;
         RTHETA_POINT rtheta;
         rtheta.rx() = 0.3 * i;   //需要更改，根据每个量程计算
         rtheta.ry() = 2 * M_PI *(1000 + i * 100) / 3600;
         plotInfo.rtheta_point = rtheta;
         plotInfo.extraData = 0;
         setTargetPlot(plotInfo);


         const QString text = QString("filter  :  flat:%1,R:%2,A:%3").arg(i).arg(plotInfo.rtheta_point.r()).arg(plotInfo.rtheta_point.theta() * 180.0 / M_PI);
         qDebug() << text;
    }
*/

}


void ReplayDataProcess(const QByteArray &data, quint16 srcid)
{
    if(lpDataProcess)
        lpDataProcess->dataProcess(data);
}





































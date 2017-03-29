#include "parsedevice.h"
#include "define.h"
#include "boatinfo.h"
#include "boatalarm.h"
#include "mainwindow.h"
#include "mouseoperation.h"
#include "TargetManage/TargetManage_global.h"
#include "aismanage.h"

#include <QString>
#include <QStringList>

extern SYSTEMINFO SystemInfo;
extern boatalarm* m_boatAlarm;
extern MainWindow *lpMainWindow;
extern MouseOperation *lpMouseOpetarion;
extern SYSTEM_PARA g_systemPara;
extern AisManage* lpAisManage;
extern ParseDevice *lpParseDevice;


ParseDevice::ParseDevice():m_data_buffer_AIS(NULL),m_data_buffer_GPS(NULL),m_data_buffer_COMP(NULL),m_data_buffer_LOG(NULL)
{
    AisObj = new Ais;
    GpsObj = new CGPS;

    m_data_buffer_AIS = new char[2500];  //开大点
    m_data_buffer_GPS = new char[2500];
    m_data_buffer_COMP = new char[2500];
    m_data_buffer_LOG = new char[2500];
    m_data_format_AIS = WAIT_START;
    m_data_format_GPS = WAIT_START;
    m_data_format_COMP = WAIT_START;
    m_data_format_LOG = WAIT_START;
    m_data_index_w_AIS = 0;
    m_data_index_w_GPS = 0;
    m_data_index_w_COMP = 0;
    m_data_index_w_LOG = 0;

    m_check_code = 0;
    m_max_datasize = 2500;
}

void ParseDevice::dataProcess(const char *data, quint16 length, quint32 deviceid)
{
    //航海仪器输出数据都时按照NEMA0183规则编码，GPS输出$GP,罗经输出$HE,AIS输出！AI

    QString text = QString::fromAscii(data, length);
    QStringList textlist = text.split(",");

    if(textlist[0].startsWith("$")) {  //GPS或者罗经
        QByteArray byteArray = text.toLatin1();
        packetProcess(byteArray.data(), byteArray.size(), deviceid);
        return;

    }

    //AIS数据，检测是否满足要求及是否为多个数据包合成的,最大分割成9包，每包包含7个字段
    //AIS encapsulation sentences  !aaccc,x,y,z,A/B,c--c,x4*hh<CR><LF>
    if(textlist.size() < 7)
        return;
    //x这一信息需要的句子总数，y本句的句子序数,z连续信息的识别,都不超过9
    const quint8 x = textlist[1].toInt();  //const在C++中用来替换#define预处理指令，使用了预编译器进行值替代，并不需要为这些常量分配存储空间，所以执行效率较高
    const quint8 y = textlist[2].toInt();
    const quint8 z = textlist[3].toInt();

    if(z > 9)
        return;

    //合成包数据是第一包将前面的信息加上，后面来的信息只提取其中的数据报文,同时报文右边把逗号去掉
    const int left = text.indexOf(textlist[5]);
    const int right = text.indexOf(textlist[textlist.size()-1]) - 1;
    QByteArray byteArray0 = text.mid(0, right).toLatin1();
   // QByteArray byteArray1 = text.mid(left, right).toLatin1();
    QByteArray byteArray1 = textlist[5].toLatin1();

    //一个包直接处理
    //!AIVDM,1,1,,B,177KQJ5000G?tO`K>RA1wUbN0TKH,0*5C
    if(x == 1) {
        packetProcess(byteArray0.data(), byteArray0.size(), deviceid);
        return;
    }

    //多个包合并
    //!AIVDM,2,1,3,B,55P5TL01VIaAL@7WKO@mBplU@<PDhh000000001S;AJ::4A80?4i@E53,0*3E
    //!AIVDM,2,2,3,B,1@0000000000000,2*55
    QHash<int, RESERVEPACKAGE>::iterator it = m_reservePackage.find(deviceid);
    //第一包
    if(it == m_reservePackage.end()){
        RESERVEPACKAGE package;
        for(int i=0; i<10; i++) {
            package.totalPackageSize[i] = 0;  //清零数据
        }
        package.totalPackageSize[z] = x;
        package.packageList[z] = byteArray0;
        m_reservePackage.insert(deviceid, package);
    }else {
        RESERVEPACKAGE &package = it.value();
        //插入数据部分
        if(package.totalPackageSize[z] !=x || y==1) {
            //错误数据,重新开始
            package.totalPackageSize[z] = x;
            package.packageList[z] = byteArray0;

        }else {
            package.packageList[z].append(byteArray1);   //去掉校验之后的合成数据包
            //当包收齐之后处理
            if(y == x) {
                packetProcess(package.packageList[z].data(), package.packageList[z].size(), deviceid);
                package.totalPackageSize[z] = 0;
                package.packageList[z].clear();
            }
        }
    }


}

//检查是否为字母或数字
bool isLetterOrNumber(char ch)
{
        return ((ch >= 'a' && ch <= 'z') ||
                        (ch >= 'A' && ch <= 'Z') ||
                        (ch >= '0' && ch <= '9'));
}
//当字符串有不是字母或数字的时候将字符串结束符加上
void strim(char* data, quint16 length)
{
    for(int i=0;i<length && data[i];i++)
    {
        if(!isLetterOrNumber(data[i]))
        {
           data[i] = '\0';
           break;
        }
    }

}
//检测字符串中是否出现@，有就将结束符加上，没有在最后面加上
inline void ParseDevice::vCharProcess( char* data, quint16 length)
{
    quint16 i=0, j=0;
    for(i=0;i<length && data[i];i++)
    {
        if(data[i] == '@')
        {
            j = length;
            data[i] = '\0';
            break;
        }
        else if(QChar(data[i]).isLetterOrNumber())
        {
            j = i;
        }
    }

    if(j < i)
        data[j+1] = '\0';
}
//调整数值
#define FloatAdjust(v, precision) \
{				 \
        if( v > 0.0) v += precision;  \
        else if( v < 0.0) v -= precision;\
}

inline void ParseDevice::packetProcess(const char *data, quint16 length, quint32 deviceid)
{
    //此处的数据去掉最后面一部分
    QString text = QString::fromAscii(data, length);
    if(!text.startsWith("!") && !text.startsWith("$")) //
        return;

    bool okflag;  //转换成功标志
    const QString flag = text.mid(3, 3);
    if(flag == "HDT") {
        //电罗经船艏向,$HEHDT,nnn.n,T*hh
        float dm;
        QStringList textlist = text.split(",");
        if(textlist.size() >= 2) {
            dm = (float)textlist[1].toFloat(&okflag);
            if(okflag) {  //转换成功，将船艏向加入
                FloatAdjust(dm, 0.01);  //可以调整数值
                //进行后续操作.........................................

                m_boatAlarm->setOwnshpHeading(dm);
                lpMainWindow->refreshDeviceLink(COMP_DEVICE);

            }
        }

    }else if(flag == "HDG") {
        //磁罗经输出信息， 方向+偏转+变化   意思？？



    }else if(flag == "HRC")
    {	// 舰艏向 $HEHRC09476,-004*79
        double dm;
        QStringList textlist = text.split(",");
        if(textlist.size() >= 2)
        {
            dm = (double)(textlist[0].mid(6).toFloat(&okflag) / 100.0);
            if(okflag){
            FloatAdjust(dm, 0.01);
            //进行后续操作。。。。。。。。。。。。。。。。。。。。。。。。。。。。

            m_boatAlarm->setOwnshpHeading(dm);
            lpMainWindow->refreshDeviceLink(COMP_DEVICE);

            }
        }
    }else if(flag == "VBW") //计程仪输出对水速度等，节
    {
        double dm;
        QStringList textlist = text.split(",");
        if(textlist.size() > 10) {
            dm = (double)(textlist[1].toFloat(&okflag));
            if(okflag) {
                FloatAdjust(dm, 0.01);
                m_boatAlarm->setWaterSpeed(dm);
                lpMainWindow->refreshDeviceLink(LOG_DEVICE);  //刷新状态
            }
        }

    }else if(flag == "VLW")  //计程仪输出的航行里程，海里,不用
    {

    }else if(text.startsWith("!AI")) {  //AIS数据
        int MMSI = 0;
        if(AisObj->AisDataProcess((quint8*)data, length))
        {
            if(AisObj->bGetInfo1_Flag) {  //动态信息
                AIS_DYNAMIC_INFO info;
                info.course = AisObj->m_sInfo1.cog;  //对地航向
                double heading = AisObj-> m_sInfo1.trueHeading;  //船艏真航向
                if(heading > 360 || qAbs(heading - info.course) >20)
                    heading = info.course;
                info.heading = heading;
                info.lat = AisObj->m_sInfo1.latitude;
                info.lon = AisObj->m_sInfo1.longitude;
                info.naviStatus = AisObj->m_sInfo1.naviStatus;
                info.rateOfTurn = AisObj->m_sInfo1.rateOfTurn;
                info.sog = AisObj->m_sInfo1.sog; //对地航速
                info.speed = 102.3;  //对水速度无效
                info.dateTime = SystemInfo.crnt_time;
                MMSI = AisObj->m_sInfo1.userID;

                if (!EQUAL3(info.lon, 181.0, 0.01) && !EQUAL3(info.lat, 91.0, 0.01)){//位置信息有效

                    //qDebug() << "AIS dynamic info:" << piAISObj->m_sInfo1.userID << info.lat << info.lon;

                    if(text.startsWith("!AIVDM"))
                    {	// AIVDM,其他船的动态信息
                        lpAisManage->updateAisDynamicInfo(AisObj->m_sInfo1.userID, &info, AisObj->m_aisClass);
                    }
                    else if(text.startsWith("!AIVDO") && g_systemPara.gpsResource == 2)
                    {
                        // AIVDO, 作为定位源
                        m_boatAlarm->setDynamicInfo(info);
                    }
                 }

                AisObj->bGetInfo1_Flag = false;

            }else if(AisObj->bGetInfo5_Flag) {  //静态信息
                SHIP_STATIC_INFO info;
                info.MMSI = AisObj->m_sInfo5.userID;
                info.maxDraught = AisObj->m_sInfo5.maxDraught;
                info.arriveTime = AisObj->m_sInfo5.eta.time();

                memcpy(info.callCode, (char*)AisObj->m_sInfo5.callSign, 7); //呼号最大为7位
                vCharProcess(info.callCode,7); //在后面加上字符串结束符
                strim(info.callCode, 7); //再次检验
                memcpy(info.destination, (char*)AisObj->m_sInfo5.Destination, 20);
                vCharProcess(info.destination, 20);
                info.IMO = AisObj->m_sInfo5.imoNumber;
                memcpy(info.name, (char*)AisObj->m_sInfo5.shipName, 20);
                vCharProcess(info.name, 20);
                info.shipType = AisObj->m_sInfo5.shipType;
                info.headRange = AisObj->m_sInfo5.refOfPosition.dimA;
                info.tailRange = AisObj->m_sInfo5.refOfPosition.dimB;
                info.leftRange = AisObj->m_sInfo5.refOfPosition.dimC;
                info.rightRange = AisObj->m_sInfo5.refOfPosition.dimD;
                info.version = AisObj->m_sInfo5.versionAis;
                MMSI = info.MMSI;

                //qDebug() << "AIS static info:" << info.MMSI;
                if(text.startsWith("!AIVDM"))
                    lpAisManage->addorUpdateAisShip(info.MMSI, &info, AisObj->m_aisClass);

                else if(text.startsWith("!AIVDO") && g_systemPara.gpsResource == 2)
                        m_boatAlarm->setStaticInfo(info);

                AisObj->bGetInfo5_Flag = false;
            }

            // 刷新AIS链路状态
            lpMainWindow->refreshDeviceLink(AIS_DEVICE);

        }else {
           // qDebug()<<"AIS data error!";

        }

    }else if(text.startsWith("$GP") || text.startsWith("$LC") || text.startsWith("$DE")) {

        GpsObj->GpsDataProcess((quint8*)data);

        int offsetHour = SystemInfo.gps_timeAdjust;  //时间调整值
        DATE_TIME_TYPE_STRUCT datetime = GpsObj->GetAdjustTimeAndDate(offsetHour);
        if((GpsObj->m_bEnableUseTime_Flag)&&(GpsObj->m_bEnableUseDate_Flag))	// 可以使用时间和日期标志
        {
            QDateTime dt = QDateTime::fromTime_t(SystemInfo.crnt_time);
            dt.setDate(QDate(datetime.uhYear, datetime.ucMonth, datetime.ucDay));
            dt.setTime(QTime(datetime.ucHour, datetime.ucMinute, datetime.ucSecond, datetime.uhMillisecond));
            SystemInfo.gps_info.dateTime = dt.toTime_t();	//day month year

            //SystemInfo.crnt_time = SystemInfo.gps_info.dateTime;
           // lpMainWindow->setCrntTime(SystemInfo.gps_info.dateTime, false);

        }
        else if (GpsObj->m_bEnableUseTime_Flag)	//仅可以使用时间标志，日期取当前日期
        {
            QDateTime dt = QDateTime::fromTime_t(SystemInfo.crnt_time);
            dt.setDate(QDate(dt.date().year(),dt.date().month(), dt.date().day()));
            dt.setTime(QTime(datetime.ucHour, datetime.ucMinute, datetime.ucSecond, datetime.uhMillisecond));
            SystemInfo.gps_info.dateTime = dt.toTime_t();//day month year

            //SystemInfo.crnt_time = SystemInfo.gps_info.dateTime;
            //lpMainWindow->setCrntTime(SystemInfo.gps_info.dateTime, false);

        }

        // 时间调整标志，非重演状态下
       /*if(!(IsReplaying()))
        {
        if(GpsObj->m_bEnableUseTime_Flag && gpsTimeAdjust)
        {
            const QDateTime dt = QDateTime::fromTime_t(SystemInfo.gps_info.dateTime);
            const QDate date = dt.date();
            const QTime tm = dt.time();
            SetDateTime(date.year(), date.month(), date.day(), tm.hour(), tm.minute(), tm.second());
            gpsTimeAdjust = false;
        }
        } */

        GpsObj->m_bEnableUseTime_Flag = false;
        GpsObj->m_bEnableUseDate_Flag = false;

    /*  m_uhReceriveGPRMC_Flag
        收到GPRMC报文标志 由各个位来表示数据接收解释状态，
        每个bit位：0 表示未接收到该数据(或者解释出错)，1表示解释成功
        第6位:	<7> 天线移动的速度
        第7位:	<8> 相对地面方向
     */
        if(GpsObj->m_uhReceriveGPRMC_Flag)
        {
            if((GpsObj->m_uhReceriveGPRMC_Flag)&(WORD_BIT_6_GPS))
                    SystemInfo.gps_info.sog = GpsObj->m_fGPRMC_Velocity_knots;
            if((GpsObj->m_uhReceriveGPRMC_Flag)&(WORD_BIT_7_GPS))
            {
                    SystemInfo.gps_info.heading = GpsObj->m_fGPRMC_Velocity_Azimuth;
                    SystemInfo.gps_info.course = GpsObj->m_fGPRMC_Velocity_Azimuth;
            }
            GpsObj->m_uhReceriveGPRMC_Flag = 0;
        }
        if(GpsObj->m_uhReceriveGPVTG_Flag)
        {
            SystemInfo.gps_info.heading = GpsObj->m_fGPVTG_TrueNorth_Azimuth;
            SystemInfo.gps_info.course = GpsObj->m_fGPVTG_TrueNorth_Azimuth;
            SystemInfo.gps_info.sog = GpsObj->m_fGPVTG_Velocity_knots;
            GpsObj->m_uhReceriveGPVTG_Flag = 0;
        }

        if(GpsObj->m_bEnableUseLongLat_Flag)//可以使用经纬度数值标志
        {
            GpsObj->m_bEnableUseLongLat_Flag = false;
            double latitude = GpsObj->m_sLatitude.hDegree + GpsObj->m_sLatitude.hMinute/60.0 + GpsObj->m_sLatitude.dSecond/3600.0;
            double longitude = GpsObj->m_sLongitude.hDegree + GpsObj->m_sLongitude.hMinute/60.0 + GpsObj->m_sLongitude.dSecond/3600.0;
            if(GpsObj->m_ucLatitudeAttribute==1)/* 纬度属性，南北半球，N 0 / S 1  */
                latitude = -latitude;
            if(GpsObj->m_ucLongitudeAttribute==1)/* 经度属性，东西半球，E 0 / W 1  */
                longitude = -longitude;
            SystemInfo.gps_info.lat = latitude;
            SystemInfo.gps_info.lon = longitude;

           //qDebug() << "gps set pos:" << latitude << longitude;

            //设置本船的动态信息
           m_boatAlarm->setDynamicInfo(SystemInfo.gps_info);

           //设置雷达中心点经纬度
         /*  LATITUDE_POINT ld;
           ld.setPoint(longitude*COEF_DEGREETORADIAN, latitude*COEF_DEGREETORADIAN);  //需要转化为弧度才能计算
           lpMouseOpetarion->setCenterLatitude(ld); */

        }

        // 刷新GPS链路状态
       lpMainWindow->refreshDeviceLink(GPS_DEVICE);
    }


}

void ParseDevice::unpacking_AIS(const QByteArray &datagram, quint32 deviceid)
{

    if((!m_data_buffer_AIS) || (!lpParseDevice))
         return;

    QMutexLocker locker(&m_mutex);

    const int length = datagram.length();
    quint8 data;
    for (int i=0; i<length; i++)
    {
        data = quint8(datagram[i]);
        switch (m_data_format_AIS)
        {
        case WAIT_START:	// first ! or $ flag
        {
            if (data == '!' || data == '$')
            {
                m_data_format_AIS = WAIT_CONTENT;
                m_data_index_w_AIS = 0;
                m_data_buffer_AIS[m_data_index_w_AIS++] = data;
                m_check_code = 0;
            }
            break;
        }
        case WAIT_CONTENT:  // content
        {
            if(data == 0x0d)
            {   //数据包结束标志，后面的0x0a作为开始标志
                m_data_format_AIS = WAIT_CHECKCODE;
            }
            else
            {
                m_data_buffer_AIS[m_data_index_w_AIS++] = data;
                if(m_data_index_w_AIS >= m_max_datasize)
                {
                    m_data_format_AIS = WAIT_START;
                   qDebug() << "unpacking data format error : no 0D";
                }
            }
            break;
        }
        case WAIT_CHECKCODE:
            if(data == 0x0a)
            {
                //数据包开始标志
                m_data_format_AIS = WAIT_START;

                if (checkCode(m_data_buffer_AIS, m_data_index_w_AIS) == 0)
                {
                     m_data_buffer_AIS[m_data_index_w_AIS++] = 0x0d;
                     m_data_buffer_AIS[m_data_index_w_AIS++] = 0x0a;
                   // qDebug()<<"checkCode ok";
                    // 数据处理
                    if(lpParseDevice)
                        lpParseDevice->dataProcess(m_data_buffer_AIS, m_data_index_w_AIS, deviceid);
                }
                else{
                    qDebug() << " AIS checkCode error";
                    qDebug() << QString::fromAscii(m_data_buffer_AIS, m_data_index_w_AIS);
                }
            }
            else
            {
                qDebug() << "unpacking data format error : no 0A";
                m_data_format_AIS = WAIT_START;
            }
            break;

        default:
            m_data_format_AIS = WAIT_START;
            break;
        }	// end switch
    }	// end for
}

void ParseDevice::unpacking_GPS(const QByteArray &datagram, quint32 deviceid)
{
    if((!m_data_buffer_GPS) || (!lpParseDevice))
         return;

    QMutexLocker locker(&m_mutex);

    const int length = datagram.length();
    quint8 data;
    for (int i=0; i<length; i++)
    {
        data = quint8(datagram[i]);
        switch (m_data_format_GPS)
        {
        case WAIT_START:	// first ! or $ flag
        {
            if (data == '!' || data == '$')
            {
                m_data_format_GPS = WAIT_CONTENT;
                m_data_index_w_GPS = 0;
                m_data_buffer_GPS[m_data_index_w_GPS++] = data;
                m_check_code = 0;
            }
            break;
        }
        case WAIT_CONTENT:  // content
        {
            if(data == 0x0d)
            {   //数据包结束标志，后面的0x0a作为开始标志
                m_data_format_GPS = WAIT_CHECKCODE;
            }
            else
            {
                m_data_buffer_GPS[m_data_index_w_GPS++] = data;
                if(m_data_index_w_GPS >= m_max_datasize)
                {
                    m_data_format_GPS = WAIT_START;
                   qDebug() << "ending max_datasize unpacking data format error : no 0D";
                }
            }
            break;
        }
        case WAIT_CHECKCODE:
            if(data == 0x0a)
            {
                //数据包开始标志
                m_data_format_GPS = WAIT_START;

                if (checkCode(m_data_buffer_GPS, m_data_index_w_GPS) == 0)
                {
                     m_data_buffer_GPS[m_data_index_w_GPS++] = 0x0d;
                     m_data_buffer_GPS[m_data_index_w_GPS++] = 0x0a;
                   // qDebug()<<"checkCode ok";
                    // 数据处理
                    if(lpParseDevice)
                        lpParseDevice->dataProcess(m_data_buffer_GPS, m_data_index_w_GPS, deviceid);
                }
                else{
                    qDebug() << " GPS checkCode error";
                    qDebug() << QString::fromAscii(m_data_buffer_GPS, m_data_index_w_GPS);
                }
            }
            else
            {
                qDebug() << "unpacking data format error : no 0A";
                m_data_format_GPS = WAIT_START;
            }
            break;

        default:
            m_data_format_GPS = WAIT_START;
            break;
        }	// end switch
    }	// end for
}


void ParseDevice::unpacking_COMP(const QByteArray &datagram, quint32 deviceid)
{
    if((!m_data_buffer_COMP) || (!lpParseDevice))
         return;

    QMutexLocker locker(&m_mutex);

    const int length = datagram.length();
    quint8 data;
    for (int i=0; i<length; i++)
    {
        data = quint8(datagram[i]);
        switch (m_data_format_COMP)
        {
        case WAIT_START:	// first ! or $ flag
        {
            if (data == '!' || data == '$')
            {
                m_data_format_COMP = WAIT_CONTENT;
                m_data_index_w_COMP = 0;
                m_data_buffer_COMP[m_data_index_w_COMP++] = data;
                m_check_code = 0;
            }
            break;
        }
        case WAIT_CONTENT:  // content
        {
            if(data == 0x0d)
            {   //数据包结束标志，后面的0x0a作为开始标志
                m_data_format_COMP = WAIT_CHECKCODE;
            }
            else
            {
                m_data_buffer_COMP[m_data_index_w_COMP++] = data;
                if(m_data_index_w_COMP >= m_max_datasize)
                {
                    m_data_format_COMP = WAIT_START;
                   qDebug() << "unpacking data format error : no 0D";
                }
            }
            break;
        }
        case WAIT_CHECKCODE:
            if(data == 0x0a)
            {
                //数据包开始标志
                m_data_format_COMP = WAIT_START;

                if (checkCode(m_data_buffer_COMP, m_data_index_w_COMP) == 0)
                {
                     m_data_buffer_COMP[m_data_index_w_COMP++] = 0x0d;
                     m_data_buffer_COMP[m_data_index_w_COMP++] = 0x0a;
                   // qDebug()<<"checkCode ok";
                    // 数据处理
                    if(lpParseDevice)
                        lpParseDevice->dataProcess(m_data_buffer_COMP, m_data_index_w_COMP, deviceid);
                }
                else{
                    qDebug() << " COMP checkCode error";
                    qDebug() << QString::fromAscii(m_data_buffer_COMP, m_data_index_w_COMP);
                }
            }
            else
            {
                qDebug() << "unpacking data format error : no 0A";
                m_data_format_COMP = WAIT_START;
            }
            break;

        default:
            m_data_format_COMP = WAIT_START;
            break;
        }	// end switch
    }	// end for
}

void ParseDevice::unpacking_LOG(const QByteArray &datagram, quint32 deviceid)
{
    if((!m_data_buffer_LOG) || (!lpParseDevice))
         return;

    QMutexLocker locker(&m_mutex);

    const int length = datagram.length();
    quint8 data;
    for (int i=0; i<length; i++)
    {
        data = quint8(datagram[i]);
        switch (m_data_format_LOG)
        {
        case WAIT_START:	// first ! or $ flag
        {
            if (data == '!' || data == '$')
            {
                m_data_format_LOG = WAIT_CONTENT;
                m_data_index_w_LOG = 0;
                m_data_buffer_LOG[m_data_index_w_LOG++] = data;
                m_check_code = 0;
            }
            break;
        }
        case WAIT_CONTENT:  // content
        {
            if(data == 0x0d)
            {   //数据包结束标志，后面的0x0a作为开始标志
                m_data_format_LOG = WAIT_CHECKCODE;
            }
            else
            {
                m_data_buffer_LOG[m_data_index_w_LOG++] = data;
                if(m_data_index_w_LOG >= m_max_datasize)
                {
                    m_data_format_LOG = WAIT_START;
                   qDebug() << "unpacking data format error : no 0D";
                }
            }
            break;
        }
        case WAIT_CHECKCODE:
            if(data == 0x0a)
            {
                //数据包开始标志
                m_data_format_LOG = WAIT_START;

                if (checkCode(m_data_buffer_LOG, m_data_index_w_LOG) == 0)
                {
                     m_data_buffer_LOG[m_data_index_w_LOG++] = 0x0d;
                     m_data_buffer_LOG[m_data_index_w_LOG++] = 0x0a;
                   // qDebug()<<"checkCode ok";
                    // 数据处理
                    if(lpParseDevice)
                        lpParseDevice->dataProcess(m_data_buffer_LOG, m_data_index_w_LOG, deviceid);
                }
                else{
                    qDebug() << " LOG checkCode error";
                    qDebug() << QString::fromAscii(m_data_buffer_LOG, m_data_index_w_LOG);
                }
            }
            else
            {
                qDebug() << "unpacking data format error : no 0A";
                m_data_format_LOG = WAIT_START;
            }
            break;

        default:
            m_data_format_LOG = WAIT_START;
            break;
        }	// end switch
    }	// end for
}


// 计算报文的检查和!AIVDM,1,1,,B,169G?I0P007k`vT;BwhP7gv4@d0e,0*0A
              // AIVDM,1,1,,B,169G?I0P007k`vT;BwhP7gv4@d0e,0
quint8 ParseDevice::checkCode(char *m_data_buffer, quint16 m_data_index_w)
{
    // no check sum
    if(m_data_buffer[m_data_index_w-3] != '*')
        return 0;

    //qDebug() << m_data_index_w;
    quint8 checkSum = m_data_buffer[1], checkCode;
    for(int i=2; i<m_data_index_w-3; i++)
    {
        checkSum ^= m_data_buffer[i];
    }
    m_data_buffer[m_data_index_w] = 0;
    checkCode = QString::fromAscii(&m_data_buffer[m_data_index_w-2], 2).toInt(0, 16);
    //qDebug() << checkSum;
    if(checkSum != checkCode)
        return 1;
    return 0;
}







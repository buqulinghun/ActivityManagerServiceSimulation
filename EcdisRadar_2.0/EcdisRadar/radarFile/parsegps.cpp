
/***********************************************************************
 *  模块描述:			GPS数据通用处理程序
 ************************************************************************/
#include "parsegps.h"
#include <stdlib.h>
#include <string.h>

/*-----------------------------------------------------------------------------
 函数名：	CGPS
 描述:		类CGPS的构造函数
 详细描述：	构造函数，完成初始化成员变量
 参数:
 返回值：	无
 注意事项:	无
 -----------------------------------------------------------------------------*/
CGPS::CGPS()
{
    /* 初始化成员变量 */
    CGPSInitPara();
}

/*-----------------------------------------------------------------------------
 函数名：	~CGPS
 描述:		类CGPS的析构函数
 详细描述：
 参数:
 返回值：	无
 注意事项:	无
 -----------------------------------------------------------------------------*/
CGPS::~CGPS()
{

}

/*-----------------------------------------------------------------------------
 函数名：	CGPSInitPara
 描述:		初始化成员变量
 详细描述：	初始化成员变量
 参数:		无
 返回值：	无
 注意事项:	无
 -----------------------------------------------------------------------------*/
void CGPS::CGPSInitPara()
{
    unsigned i = 0;

    /* －－－－－变量初始化－－－－－－－－－ */

    /*  可以使用经纬度数值标志 */
    m_bEnableUseLongLat_Flag	= false;

    /*  可以使用高度数值标志 */
    m_bEnableUseHeight_Flag	= false;

    /*  可以使用日时间标志*/
    m_bEnableUseTime_Flag	= false;

    /*  可以使用日期标志*/
    m_bEnableUseDate_Flag	= false;


    /*  时间信息秒是否带有小数点 */
    m_bSecondWithDot = false;

    /*  NMEA0183协议是否是V3.0版本 */
    m_bVer3 = false;

    /*  是否存在本程序还没有纳入的数据格式 */
    m_bExistUnExplainData = false;

    /*  GPS接收机收到3颗卫星的信号可以输出2D（就是2维）数据，
        只有经纬度，没有高度，如果收到4颗以上的卫星，就输出3D数据，
        可以提供海拔高度。但高度数据有一些误差 ,默认情况下只有达到
        5颗星时才能使用经纬度数据，用户可以根据需要调用函数来设置此值*/
    m_ucEnableUseLongLat_StarNum = 0;

    /*   设置本时区的时差，取值范围 －12～13，默认为北京时区，时差8*/
    m_cOffsetHour = 8;

    /* 本地时区分钟偏移量，00到+/-59,此变量暂时未用于时间校正 */
    m_cOffsetMinute = 0;

    /*----------------------------------GPGGA--------------------------------*/
    /*  收到GPGGA报文标志 */
    m_uhReceriveGPGGA_Flag = 0;

    /* 时、分、秒和毫秒 */
    m_ucHour		= 0;
    m_ucMinute		= 0;
    m_ucSecond		= 0;
    m_uhMillisecond = 0;

    /* 经纬度信息 */
    m_sLatitude.hDegree = 31;
    m_sLatitude.hMinute = 53;
    m_sLatitude.dSecond = 50;

    m_sLongitude.hDegree = 118;
    m_sLongitude.hMinute = 52;
    m_sLongitude.dSecond = 19;

    /* 高度信息 */
    m_fHeight = 180.0f;

    /* 纬度属性，南北半球，N 0 / S 1  */
    m_ucLatitudeAttribute = 0;
    /* 经度属性，东西半球，E 0 / W 1  */
    m_ucLongitudeAttribute = 0;

    /* 接收机定位标志 */
    m_ucReceriverLocationFlag = 0;

    /* GPS收到的卫星颗数，大于等于5颗以上才能使用GPS数据 */
    m_ucStarNum = 0;

    /* 水平精度因子 0.5到99.9*/
    m_fH_PrecisonGene = 0.0f;

    /* 大地水准面高度，－999.9到9999.9 */
    m_fWaterLevelHeight = 0.0f;

    /* 有效数据年龄，最后一次有效差分定位时和现在的时间间隔，
       单位为秒。若不是差分GPS，则置为最大值65535 */
    m_uhValidTime = 65535;

    /* 差分基准站号(0000～1023，不足位数时前面补0)，
       若不是差分GPS，则置为最大值65535 */
    m_uhDifferenceStationNum = 65535;

    /*----------------------------------GPRMC--------------------------------*/
    /*  收到GPRMC报文标志 */
    m_uhReceriveGPRMC_Flag = false;

    /* GPRMC报文定位成功标志 1(A)表示定位成功，(0)V表示目前没有定位*/
    m_ucGP_LocationSucessFlag = 0;

    /* 天线移动的速度，从000.0到999.9节 */
    m_fGPRMC_Velocity_knots = 0.0f;

    /* 相对地面方向，000.0到359.9度,以真北为参考基准 */
    m_fGPRMC_Velocity_Azimuth = 0.0f;

    /* 年、月、日 */
    m_uhYear	= 2009;
    m_ucMonth	= 8;
    m_ucDay		= 13;

    /* 磁偏角，从000.0到180.0度 */
    m_fGPRMC_MagneticAzimuth = 0.0f;

    /* 磁偏方向属性，0/E(东)或1/W(西) */
    m_ucGPRMC_MagneticAttr = 0;

    /* 模式指示（仅NMEA0183 3.00版本输出，
       0/N=数据无效,1/A=自主定位,2/D=差分,3/E=估算） */
    m_ucGPWorkModel = 0;

    /*----------------------------------GPGSV--------------------------------*/
    /*  收到GPGSV报文标志 */
    m_bReceriveGPGSV_Flag = false;

    /* 可见卫星的总数，00 ～ 12 */
    m_ucGPGSV_StarNum = 0;

    /*	GPS可见卫星信息,包括卫星编号、卫星仰角(00～90度)、
        卫星方位角(000～359度)、信号信噪比(00～99dB)和信息是否有效标志，
        目前的接收机最多能收到12颗卫星信息，卫星编号的范围为1～32，为防止出现
        跟踪到12颗卫星后发生卫星编号变化的现象，使用数组下标＋1来表示卫星编号*/
    for(i=0;i<32;i++)
    {
        m_sgStarInfo[i].ucStarIndex		= 0;	/* 卫星编号01～32				*/
        m_sgStarInfo[i].ucStarElevation	= 0;	/* 卫星仰角(00～90度)			*/
        m_sgStarInfo[i].uhStarAzimuth	= 0;	/* 卫星方位角(000～359度)		*/
        m_sgStarInfo[i].ucStarSNR		= 0;	/* 信号信噪比(00～99dB)			*/
        m_sgStarInfo[i].ucStarInfoValid	= 0;	/* 本组信息是否有效,0无效,1有效	*/
    }
    /*----------------------------------GPGSA--------------------------------*/
    /*  收到GPGSA报文标志 */
    m_uhReceriveGPGSA_Flag = 0;

    /* 模式:0/A = 自动,1/M = 手动，*/
    m_ucGPGSA_WorkModel = 0;

    /* 型式 1 = 未定位， 2 = 二维定位， 3 = 三维定位 */
    m_ucGPGSA_PositionModel = 1;

    /* PRN 数字：01～32， 表天空使用中的卫星编号，最多可接收12颗卫星信息 */
    for(i=0;i<12;i++)
        m_ucgGPGSA_StarNum[i] = 0;

    /* PDOP位置精度因子（0.5~99.9），(2.6) */
    m_fGPGSA_PDOP = 0.0f;

    /* HDOP水平精度因子（0.5~99.9），(2.5) */
    m_fGPGSA_HDOP = 0.0f;

    /* VDOP垂直精度因子（0.5~99.9），(1.0) */
    m_fGPGSA_VDOP = 0.0f;
    /*----------------------------------GPGLL--------------------------------*/
    /* 地理定位信息($GPGLL) 经纬度信息和UTC时间 */

    /*  收到GPGLL报文标志 */
    m_uhReceriveGPGLL_Flag = false;
    /*----------------------------------GPVTG--------------------------------*/
    /*  收到GPVTG报文标志 */
    m_uhReceriveGPVTG_Flag = 0;

    /* 相对地面方向，000.0到359.9度,以真北为参考基准 */
    m_fGPVTG_TrueNorth_Azimuth = 0.0f;

    /* 相对地面方向，000.0到359.9度,以磁北为参考基准 */
    m_fGPVTG_MagneticNorth_Azimuth = 0.0f;

    /* 水平运动速度，从000.0到999.9节 */
    m_fGPVTG_Velocity_knots = 0.0f;

    /* 水平运动速度，0000.0～1851.8公里/小时 */
    m_fGPVTG_Velocity_km = 0.0f;
    /*----------------------------------GPZDA--------------------------------*/
    /*  收到GPZDA报文标志 */
    m_uhReceriveGPZDA_Flag = 0;

}
/*-----------------------------------------------------------------------------
 函数名：	CGPSSetPara
 描述:		设置可以使用经纬度信息时的卫星个数和本时区的时差
 详细描述：	设置可以使用经纬度信息时的卫星个数和本时区的时差
 参数:		unsigned char StarNum,  char OffsetHour
 返回值：	无
 注意事项:	无
 -----------------------------------------------------------------------------*/
bool CGPS::GpsDataProcess(quint8 *packet)
{
    return GPS_DataProcess(packet);
}


void CGPS::CGPSSetPara(unsigned char StarNum,char OffsetHour)
{
    /*  GPS接收机收到3颗卫星的信号可以输出2D（就是2维）数据，
        只有经纬度，没有高度，如果收到4颗以上的卫星，就输出3D数据，
        可以提供海拔高度。但高度数据有一些误差 ,默认情况下只有达到
        5颗星时才能使用经纬度数据，用户可以根据需要调用函数来设置此值*/
    if(StarNum >=0 && StarNum <= 12)
        m_ucEnableUseLongLat_StarNum = StarNum;

    /*   设置本时区的时差，取值范围 －12～13，默认为北京时区，时差8*/
    if(OffsetHour >=-12 && OffsetHour <= 13)
        m_cOffsetHour = OffsetHour;
}

/*-----------------------------------------------------------------------------
 函数名：	CGPS::GetAdjustTimeAndDate

 描述:		获取经过校正的年月日时分秒

 详细描述：	获取经过校正的年月日时分秒
            1、时 ＋ 时差；
            2、如果跨日，<1> 日＋1（日－1）
                         <2> 根据闰年以及大小月份，修改年、月、日和时

 参数:	    无

 返回值：	DATE_TIME_TYPE_STRUCT 年年、月、日、时、分和秒

 注意事项:	无
 -----------------------------------------------------------------------------*/
DATE_TIME_TYPE_STRUCT CGPS::GetAdjustTimeAndDate(int offsetHour0)
{
    unsigned char MaxFebDay = 0;
    unsigned short p_Year	= m_uhYear;
    unsigned char p_Month	= m_ucMonth;
    unsigned char p_Day		= m_ucDay;
    unsigned char p_Hour	= m_ucHour;

    DATE_TIME_TYPE_STRUCT TimeStruct;

        int offsetHour = m_cOffsetHour + offsetHour0;

    /*  判断时差正负，如果正，则日期、月和年则可能增加
        如果负，则日期、月和年则可能增加	*/
    if(offsetHour >= 0)
    {
        /* 1、 时 ＋ 时差 */
        p_Hour = m_ucHour + offsetHour;

        /* 2、判断是否跨日 */
        if( p_Hour >= TEWENTYFOUR_HOUR_GPS)
        {
            /*  跨日 时－24 */
            p_Hour	= p_Hour - TEWENTYFOUR_HOUR_GPS;

            /* 日＋1 */
            p_Day += 1;

            /*	－－－－－－－－－－－－－－－－－－－－－－－－－－－－
            闰年,二月 是29天，不是闰年 28天
            不是闰年的判断条件：<1>不能被4整除的不是闰年 ,
            <2>能被100整除但是不能被400整除的不是闰年
            －－－－－－－－－－－－－－－－－－－－－－－－－－－－ */
            if(		((p_Year%4) != 0)
                ||	(((p_Year%100) == 0) && ((p_Year%400) != 0) )  )
                MaxFebDay = 28;
            else
                MaxFebDay = 29;

            if(p_Month == 2)
            {
                if(p_Day > MaxFebDay)
                {
                    p_Day = 1;
                    p_Month += 1;
                }
            }
            else if(	(p_Month == 1) || (p_Month == 3) || (p_Month == 5)
                ||	(p_Month == 7) || (p_Month == 8) || (p_Month == 10))
            {
                /* 大月 31天 */
                if(p_Day > 31)
                {
                    p_Day = 1;
                    p_Month += 1;
                }
            }
            else if(	(p_Month == 4) || (p_Month == 6)
                ||	(p_Month == 9) || (p_Month == 11))
            {
                /* 小月 30天 */
                if(p_Day > 30)
                {
                    p_Day = 1;
                    p_Month += 1;
                }
            }
            else if(p_Month == 12)
            {
                /* 跨年 */
                if(p_Day > 31)
                {
                    p_Day = 1;
                    p_Month = 1;
                    p_Year += 1;
                }
            }
        }
    }
    else if (offsetHour < 0)
    {
        /* 1、 时 ＋ 时差 */
        if( m_ucHour + offsetHour >= 0)
            p_Hour = m_ucHour + offsetHour;
        else
        {
            /* 时+ 24 */
            p_Hour =  m_ucHour + offsetHour + TEWENTYFOUR_HOUR_GPS;

            /* 日-1 */
            p_Day -= 1;
            if(p_Day == 0)
            {
                /* 月-1 */
                p_Month --;

                if(p_Month == 0)
                {
                    /* 1月1号，则日期应该变为12月31号，年份--*/
                    p_Year	= m_uhYear--;
                    p_Month	= 12;
                    p_Day	= 31;
                }
                else if(p_Month == 1 || p_Month == 3 || p_Month == 5 || p_Month == 7 || p_Month == 8 || p_Month == 10)
                {
                    p_Day = 31;
                }
                else if(p_Month == 4 || p_Month == 6 || p_Month == 9 || p_Month == 11)
                {
                    p_Day = 30;
                }
                else if(p_Month == 2)
                {
                    /*	-------------------------------------------
                    闰年,二月 是29天，不是闰年 28天
                    不是闰年的判断条件：<1>不能被4整除的不是闰年 ,
                    <2>能被100整除但是不能被400整除的不是闰年
                        -------------------------------------------*/
                    if(		((p_Year%4) != 0)
                        ||	(((p_Year%100) == 0) && ((p_Year%400) != 0) )  )
                        p_Day = 28;
                    else
                        p_Day = 29;
                }
            }
        }
    }
    /* 保存年月日 时分秒 */
    TimeStruct.uhYear		= p_Year;
    TimeStruct.ucMonth		= p_Month;
    TimeStruct.ucDay		= p_Day;
    TimeStruct.ucHour		= p_Hour;
    TimeStruct.ucMinute		= m_ucMinute;
    TimeStruct.ucSecond		= m_ucSecond;
    TimeStruct.uhMillisecond= m_uhMillisecond;

    return TimeStruct;
}

/*-----------------------------------------------------------------------------
 函数名：	GPS_DataProcess
 描述:		GPS数据处理
 详细描述：
 参数:		unsigned char  *group
 返回值：	bool GPS数据解释成功标志
 注意事项:	无
 -----------------------------------------------------------------------------*/
bool CGPS::GPS_DataProcess(unsigned char *group)
{
    /* GPS数据解释成功标志 */
    bool bGetGPSFlag = false;

    if(		(*(group )	  == '$')
        &&(((*(group + 1) == 'G')&&(*(group + 2) == 'P')) || ((*(group + 1) == 'L')&&(*(group + 2) == 'C')) || ((*(group + 1) == 'D')&&(*(group + 2) == 'E')) )
        &&	(*(group + 3) == 'G')
        &&	(*(group + 4) == 'G')
        &&	(*(group + 5) == 'A'))
    {
        /* 获取GPS数据GPGGA格式数据 (GPS定位信息固定数据输出语句) */
        bGetGPSFlag = GetGPS_GPGGA(group + 6);
    }
    else if(	(*(group )	  == '$')
        &&(((*(group + 1) == 'G')&&(*(group + 2) == 'P')) || ((*(group + 1) == 'L')&&(*(group + 2) == 'C')) || ((*(group + 1) == 'D')&&(*(group + 2) == 'E')) )
            &&	(*(group + 3) == 'R')
            &&	(*(group + 4) == 'M')
            &&	(*(group + 5) == 'C'))
    {
        /* 获取GPS数据GPRMC格式数据 (GPS推荐定位信息)*/
        bGetGPSFlag = GetGPS_GPRMC(group + 6);
    }
    else if(	(*(group )	  == '$')
        &&(((*(group + 1) == 'G')&&(*(group + 2) == 'P')) || ((*(group + 1) == 'L')&&(*(group + 2) == 'C')) || ((*(group + 1) == 'D')&&(*(group + 2) == 'E')) )
            &&	(*(group + 3) == 'G')
            &&	(*(group + 4) == 'S')
            &&	(*(group + 5) == 'V'))
    {
        /* 获取GPS数据GPGSV格式数据 (可视卫星状态输出语句)*/
        bGetGPSFlag = GetGPS_GPGSV(group + 6);
    }
    else if(	(*(group )	  == '$')
            &&	(*(group + 1) == 'G')
            &&	(*(group + 2) == 'P')
            &&	(*(group + 3) == 'G')
            &&	(*(group + 4) == 'S')
            &&	(*(group + 5) == 'A'))
    {
        /* 获取GPS数据GPGSA格式数据 (当前卫星信息) */
        bGetGPSFlag = GetGPS_GPGSA(group + 6);
    }
    else if(	(*(group )	  == '$')
            &&	(*(group + 1) == 'G')
            &&	(*(group + 2) == 'P')
            &&	(*(group + 3) == 'G')
            &&	(*(group + 4) == 'L')
            &&	(*(group + 5) == 'L'))
    {
        /* 获取GPS数据GPGLL格式数据 (地理定位信息) */
        bGetGPSFlag = GetGPS_GPGLL(group + 6);
    }
    else if(	(*(group )	  == '$')
            &&	(*(group + 1) == 'G')
            &&	(*(group + 2) == 'P')
            &&	(*(group + 3) == 'V')
            &&	(*(group + 4) == 'T')
            &&	(*(group + 5) == 'G'))
    {
        /* 获取GPS数据GPVTG格式数据 (地面速度信息) */
        bGetGPSFlag = GetGPS_GPVTG(group + 6);
    }
    else if(	(*(group )	  == '$')
            &&	(*(group + 1) == 'G')
            &&	(*(group + 2) == 'P')
            &&	(*(group + 3) == 'Z')
            &&	(*(group + 4) == 'D')
            &&	(*(group + 5) == 'A'))
    {
        /* 获取GPS数据GPZDA格式数据 (地面速度信息) */
        bGetGPSFlag = GetGPS_GPZDA(group + 6);
    }
    else
        /*  是否存在本程序还没有纳入的数据格式 */
        m_bExistUnExplainData = true;

    return bGetGPSFlag;

}

unsigned char str2char(unsigned char c)
{
        unsigned char res = 0;
    if(c >= '0' &&  c <= '9')
        res = (c - '0');
    else if (c >= 'A' &&  c <= 'F')
        res = (c - 'A' + 10);
    else if (c >= 'a' &&  c <= 'f')
        res = (c - 'a' + 10) ;
        return res;
}

// 判断检查和是否正确
bool CGPS::Check(unsigned char* header, unsigned char* data)
{
        // 数据太短，错误
        const int dataSize = strlen((char*)data);
        if(dataSize < 3)
                return false;

        // 无检查和，返回正常
        if(data[dataSize-3] != '*')
                return true;

        // 已经判断过检查和，不再检查
        return true;

        // 比较检查和
    unsigned char ucCheckSum = *header, ucCheckChar;
        unsigned char* ptemp = header+1;
        for(; *ptemp != '\0'; ++ptemp)
                ucCheckSum ^= *ptemp;

        for(ptemp=data; *ptemp != '*' && *ptemp != '\0'; ++ptemp)
                ucCheckSum ^= *ptemp;

        ucCheckChar = str2char(data[dataSize-2])*16 + str2char(data[dataSize-1]);

        return (ucCheckChar==ucCheckSum);
}

/*-----------------------------------------------------------------------------
 函数名：	CGPS::GetGPS_GPGGA

 描述:		获取GPS数据GPGGA格式数据

 详细描述：	获取GPS数据GPGGA格式数据(GPS定位信息固定数据输出语句)

 例：
        $GPGGA,050901,3931.4449,N,11643.5123,E,1,07,1.4,76.2,M,-7.0,M,,*65
        $GPGGA,121253.000,3937.3090,N,11611.6057,E,1,06,1.0,44.6,M,-5.7,M,,0000*72
        $GPGGA,031300.6,3151.6966,N,11714.4896,E,0,04,29.7,56.9,M,-24.4,M,244,0000*4D
        $GPGGA,191935.767,4738.0172,N,12211.1874,W,1,06,1.4,32.9,M,-17.2,M,0.0,0000*75

 其标准格式为：	 $GPGGA,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,M,<10>,M,<11>,<12>*hh<CR><LF>

        <1> UTC当前时间，格式为hhmmss(时时分分秒秒)；05时09分01秒
        <2> 纬度，格式为ddmm.mmmm (度度分分.分分分分，不足位数时前面补0)；
        <3> 纬度属性，南北半球，N/S；北纬39度31.4449分；
        <4> 经度，格式为dddmm.mmmm(度度度分分.分分分分，不足位数时前面补0)；
        <5> 经度属性，东西半球，E/W；东经116度43.5123分；
        <6> 接收机定位标志， (0=没有定位，1=实时GPS，2=差分GPS，6=正在估算)
        <7> 跟踪到的GPS卫星数，从00到12(不足位数时前面补0)
        <8> 水平精度因子，0.5到99.9；水平精度因子=1.4；
        <9> GPS天线所处海拔高度，－9999.9到99999.9；天线高程=76.2m
        <M> 表示单位米；
        <10> 大地水准面高度，－999.9到9999.9:－7.0m;
        <M> 表示单位米；
        <11> 有效数据年龄，最后一次有效差分定位时和现在的时间间隔，单位为秒。若不是差分GPS，则此信息位为空；
        <12> 差分基准站号(0000～1023，不足位数时前面补0)，若不是差分GPS，则此信息位为空；
        <*> 校验和标志，其后面的一个字节即后面的<hh>表示校验和；总和校验数:65
        (CR)(LF)回车，换行。

 参数:	    UCHAR *pData

 返回值：	bool 获取GPS数据GPGGA格式数据成功标志

 注意事项:	无
 -----------------------------------------------------------------------------*/
bool CGPS::GetGPS_GPGGA(unsigned char *pData)
{
    unsigned char c = '\a';
    bool Result = true;

    /*	成功取得经纬度数据和经纬度属性标志,只有全为true，
        并且卫星个数大于5时，才使用经纬度信息*/
    bool bGetLatData  = false,bGetLongData  = false;
    bool bGetLatAttr  = false,bGetLongAttr  = false;

    char Msg[210];
    unsigned char  Hour = 0,Minute = 0,Second = 0;
    unsigned char count = 0;
    unsigned short l_Degree =0,l_Minute =0;
    unsigned int i =0,Index = 0,l_Second =0;
    unsigned char GetDataCount = 0;

    /*	首先计算检查和，如果检查和错误，则认为本组数据出错，不予处理'
        检查和的计算方法：'$'和'*'之间所有字符进行异或后的值（不包括'$'和'*'）;
        结果与hh 比较，注意：hh是十六进制	*/
        if(!Check((unsigned char*)"GPGGA", pData))
        {
                return false;
        }
        else
        {
        /*-------------清除参数--------------*/
        /*  可以使用经纬度数值标志 */
        m_bEnableUseLongLat_Flag = false;

        /*  可以使用高度数值标志 */
        m_bEnableUseHeight_Flag	= false;

        /*  可以使用日时间标志*/
        m_bEnableUseTime_Flag	= false;

        /*  时间信息秒是否带有小数点 */
        m_bSecondWithDot = false;

        /*  收到GPGGA报文标志 */
        m_uhReceriveGPGGA_Flag = 0;

        /* 置* 检查和 标志位 */
        m_uhReceriveGPGGA_Flag |= WORD_BIT_15_GPS;
        m_uhReceriveGPGGA_Flag |= WORD_BIT_14_GPS;
        i = 0;
        }

    /*-------------依次读取数据，并解释数据--------------*/
    while(c != '\n' && Result && pData[Index] != '*' && pData[Index] != '\0')
    {
        c = pData[Index++];
        if(c == ',')
        {
            GetDataCount ++;
        }

        /* 根据“,”的个数来执行相应字段的解释工作 */
        switch(GetDataCount)
        {
        case 1:
            {
                /* ------------------------------------------------
                    <1>	UTC当前时间，格式为hhmmss(时时分分秒秒)；
                    处理老式GPS，UniStar II型GPS的时间后面带小数点，
                    II A+型不带小数点,本模块兼容这两种结构。
                  ------------------------------------------------*/
                /* 有时间信息 */
                if(pData[Index] != ',')
                {
                    /* 时时 hh */
                    Hour = 0;
                    for(i = Index; i < Index + 2; i++)
                    {
                        if(pData[i] >= '0' && pData[i] <= '9')
                            Hour = Hour * 10 + (pData[i] - '0');
                        else
                        {
                            Result = false;
                            /* 置<1>UTC当前时间标志位 */
                            m_uhReceriveGPGGA_Flag &= (~WORD_BIT_0_GPS);
                            break;
                        }
                    }
                    /* 分分 mm */
                    if(Result && Hour < 24)
                    {
                        /* 保存“时” */
                        m_ucHour = Hour;

                        Minute = 0;
                        Index += 2;
                        for(i = Index; i < Index + 2; i++)
                        {
                            if(pData[i] >= '0' && pData[i] <= '9')
                                Minute = Minute * 10 + (pData[i] - '0');
                            else
                            {
                                Result = false;
                                /* 置<1>UTC当前时间标志位 */
                                m_uhReceriveGPGGA_Flag &= (~WORD_BIT_0_GPS);
                                break;
                            }
                        }

                        /* 秒秒 ss */
                        if(Result && Minute < 60)
                        {
                            /* 保存“分” */
                            m_ucMinute = Minute;
                            Second = 0;
                            Index += 2;
                            for(i = Index; i < Index + 2; i++)
                            {
                                if(pData[i] >= '0' && pData[i] <= '9')
                                    Second = Second * 10 + (pData[i] - '0');
                                else
                                {
                                    Result = false;
                                    /* 置<1>UTC当前时间标志位 */
                                    m_uhReceriveGPGGA_Flag &= (~WORD_BIT_0_GPS);
                                    break;
                                }
                            }
                            if(Result && Second < 60)
                            {
                                Index += 2;
                                m_ucSecond = Second;

                                /* 处理老式GPS，UniStar II型GPS的时间后面带小数点*/
                                if(pData[Index] == '.')
                                {
                                    Index += 1;

                                    /*	秒的小数点后数据位个数不定(现已发现有1位和3位的情况)，
                                        需使用while 循环结构，取出后直接保存为毫秒*/
                                    count = 0;
                                    if(pData[Index] != ',')
                                    {
                                        *Msg = '0';
                                        *(Msg +1) = '.';
                                        count += 2;
                                        while(pData[Index] != ','
                                            && 	(	(pData[Index] >= '0' &&  pData[Index] <= '9')
                                                 || (pData[Index] == '.' )))
                                        {
                                            *(Msg + count) = pData[Index++];
                                            count++;
                                        }
                                        *(Msg + count) = '\0';

                                        if(pData[Index] == ',')
                                        {
                                            m_uhMillisecond = (unsigned short)(atof(Msg) * 1000);

                                            /*  时间信息秒是否带有小数点 */
                                            m_bSecondWithDot = true;
                                        }
                                        else
                                        {
                                            Result = false;
                                            /* 置<1>UTC当前时间标志位 */
                                            m_uhReceriveGPGGA_Flag &= (~WORD_BIT_0_GPS);
                                        }
                                    }
                                    else
                                    {
                                        /* 保存为毫秒 */
                                        m_uhMillisecond = 0;
                                        /* 时间信息秒是否带有小数点 */
                                        m_bSecondWithDot = false;
                                    }
                                }
                                else
                                {
                                    /* 保存为毫秒 */
                                    m_uhMillisecond = 0;
                                    /* 时间信息秒是否带有小数点 */
                                    m_bSecondWithDot = false;
                                }

                                /* 置<1>UTC当前时间标志位 */
                                m_uhReceriveGPGGA_Flag |= WORD_BIT_0_GPS;

                                if(m_ucStarNum >= m_ucEnableUseLongLat_StarNum)
                                {
                                    /* 可以使用日时间标志 */
                                    m_bEnableUseTime_Flag	= true;
                                }
                            }
                            else
                            {
                                Result = false;
                                /* 置<1>UTC当前时间标志位 */
                                m_uhReceriveGPGGA_Flag &= (~WORD_BIT_0_GPS);
                            }
                        }
                        else
                        {
                            Result = false;
                            /* 置<1>UTC当前时间标志位 */
                            m_uhReceriveGPGGA_Flag &= (~WORD_BIT_0_GPS);
                        }
                    }
                    else
                    {
                        Result = false;
                        /* 置<1>UTC当前时间标志位 */
                        m_uhReceriveGPGGA_Flag &= (~WORD_BIT_0_GPS);
                    }
                }
                else
                {
                    Result = false;
                    /* 置<1>UTC当前时间标志位 */
                    m_uhReceriveGPGGA_Flag &= (~WORD_BIT_0_GPS);
                }
            }
            break;
        case 2:
            {
                /* <2> 纬度，格式为ddmm.mmmm (度度分分.分分分分，不足位数时前面补0) */
                /* 有纬度信息 */
                if(pData[Index] != ',')
                {
                    l_Degree = 0;
                    /* 度度 dd */
                    for(i = Index; i < Index + 2; i++)
                    {
                        if(pData[i] >= '0' && pData[i] <= '9')
                            l_Degree = l_Degree * 10 + (pData[i] - '0');
                        else
                        {
                            Result = false;
                            /* 置<2> 纬度标志位 */
                            m_uhReceriveGPGGA_Flag &= (~WORD_BIT_1_GPS);
                            break;
                        }
                    }
                    if(Result && l_Degree <= 90)
                    {
                        Index += 2;
                        m_sLatitude.hDegree = l_Degree;
                        l_Minute = 0;
                        /* 分分 mm */
                        for(i = Index; i < Index + 2; i++)
                        {
                            if(pData[i] >= '0' && pData[i] <= '9')
                                l_Minute = l_Minute * 10 + (pData[i] - '0');
                            else
                            {
                                Result = false;
                                /* 置<2> 纬度标志位 */
                                m_uhReceriveGPGGA_Flag &= (~WORD_BIT_1_GPS);
                                break;
                            }
                        }

                        if(Result && l_Minute < 60)
                        {
                            Index += 2;
                            m_sLatitude.hMinute = l_Minute;

                            /*.分分分分 .mmmm  转化成秒*/
                            l_Second = 0;

                            if(pData[Index] == '.')
                            {
                                Index += 1;

                                /* 因为发现有带4位小数的，有带2位小数的，因此需要使用while 结构 */
                                *Msg = '0';
                                *(Msg + 1) = '.';
                                count = 2;

                                while(	pData[Index] != ','
                                     && pData[Index] >= '0'
                                     && pData[Index] <= '9')
                                {
                                    *(Msg + count) = pData[Index++];
                                    count ++;
                                }
                                *(Msg + count) = '\0';

                                if(pData[Index] == ',')
                                {
                                    m_sLatitude.dSecond = atof(Msg) * 60.0;

                                    /* 置<2> 纬度标志位 */
                                    m_uhReceriveGPGGA_Flag |= WORD_BIT_1_GPS;

                                    /* 成功取得纬度数据 */
                                    bGetLatData = true;
                                }
                                else
                                {
                                    Result = false;
                                    /* 置<2> 纬度标志位 */
                                    m_uhReceriveGPGGA_Flag &= (~WORD_BIT_1_GPS);
                                }
                            }
                            else
                            {
                                Result = false;
                                /* 置<2> 纬度标志位 */
                                m_uhReceriveGPGGA_Flag &= (~WORD_BIT_1_GPS);
                            }
                        }
                        else
                        {
                            Result = false;
                            /* 置<2> 纬度标志位 */
                            m_uhReceriveGPGGA_Flag &= (~WORD_BIT_1_GPS);
                        }
                    }
                    else
                    {
                        Result = false;
                        /* 置<2> 纬度标志位 */
                        m_uhReceriveGPGGA_Flag &= (~WORD_BIT_1_GPS);
                    }
                }
                else
                {
                    Result = false;
                    /* 置<2> 纬度标志位 */
                    m_uhReceriveGPGGA_Flag &= (~WORD_BIT_1_GPS);
                }
            }
            break;
        case 3:
            {
                /* <3> 纬度属性，南北半球，N/S */
                if(pData[Index] != ',')
                {
                    /* 纬度属性，南北半球，N 0 / S 1 */
                    if(pData[Index] == 'N')
                        m_ucLatitudeAttribute = 0;
                    else if (pData[Index] == 'S')
                        m_ucLatitudeAttribute = 1;
                    else
                    {
                        Result = false;
                        /* 置<3> 纬度属性标志位 */
                        m_uhReceriveGPGGA_Flag &= (~WORD_BIT_2_GPS);
                    }

                    if(Result)
                    {
                        Index++;
                        /* 置<3> 纬度属性标志位 */
                        m_uhReceriveGPGGA_Flag |= WORD_BIT_2_GPS;

                        /* 成功取得经纬度属性标志 */
                        bGetLatAttr  = true;
                    }
                }
                else
                {
                    Result = false;
                    /* 置<3> 纬度属性标志位 */
                    m_uhReceriveGPGGA_Flag &= (~WORD_BIT_2_GPS);
                }
            }
            break;
        case 4:
            {
                /* <4> 经度,格式为dddmm.mmmm(度度度分分.分分分分,不足位数时前面补0) */
                if(pData[Index] != ',')
                {
                    /* 度度 dd */
                    l_Degree = 0;
                    for(i = Index; i < Index + 3; i++)
                    {
                        if(pData[i] >= '0' && pData[i] <= '9')
                            l_Degree = l_Degree * 10 + (pData[i] - '0');
                        else
                        {
                            Result = false;
                            /* 置<4> 经度标志位 */
                            m_uhReceriveGPGGA_Flag &= (~WORD_BIT_3_GPS);
                            break;
                        }
                    }
                    if(Result && l_Degree <= 180)
                    {
                        Index += 3;
                        m_sLongitude.hDegree = l_Degree;

                        /* 分分 mm */
                        l_Minute = 0;
                        for(i = Index; i < Index + 2; i++)
                        {
                            if(pData[i] >= '0' && pData[i] <= '9')
                                l_Minute = l_Minute * 10 + (pData[i] - '0');
                            else
                            {
                                Result = false;
                                /* 置<4> 经度标志位 */
                                m_uhReceriveGPGGA_Flag &= (~WORD_BIT_3_GPS);
                                break;
                            }
                        }

                        if(Result && l_Minute < 60)
                        {
                            Index += 2;
                            m_sLongitude.hMinute = l_Minute;

                            l_Second = 0;

                            /*.分分分分 .mmmm  转化成秒*/
                            if(pData[Index] == '.')
                            {
                                Index += 1;

                                /* 因为发现有带4位小数的，有带2位小数的，因此需要使用while 结构 */
                                *Msg = '0';
                                *(Msg + 1) = '.';
                                count = 2;

                                while(	pData[Index] != ','
                                     && pData[Index] >= '0'
                                     && pData[Index] <= '9')
                                {
                                    *(Msg + count) = pData[Index++];
                                    count ++;
                                }
                                *(Msg + count) = '\0';

                                if(pData[Index] == ',')
                                {
                                    m_sLongitude.dSecond = atof(Msg) * 60.0;

                                    /* 置<4> 经度标志位 */
                                    m_uhReceriveGPGGA_Flag |= WORD_BIT_3_GPS;

                                    /* 成功取得经度数据 */
                                    bGetLongData  = true;
                                }
                                else
                                {
                                    Result = false;
                                    /* 置<4> 经度标志位 */
                                    m_uhReceriveGPGGA_Flag &= (~WORD_BIT_3_GPS);
                                }
                            }
                            else
                            {
                                Result = false;
                                /* 置<4> 经度标志位 */
                                m_uhReceriveGPGGA_Flag &= (~WORD_BIT_3_GPS);
                            }
                        }
                        else
                        {
                            Result = false;
                            /* 置<4> 经度标志位 */
                            m_uhReceriveGPGGA_Flag &= (~WORD_BIT_3_GPS);
                        }
                    }
                    else
                    {
                        Result = false;
                        /* 置<4> 经度标志位 */
                        m_uhReceriveGPGGA_Flag &= (~WORD_BIT_3_GPS);
                    }
                }
                else
                {
                    Result = false;
                    /* 置<4> 经度标志位 */
                    m_uhReceriveGPGGA_Flag &= (~WORD_BIT_3_GPS);
                }
            }
            break;
        case 5:
            {
                /* <5> 经度属性，东西半球，E/W */
                if(pData[Index] != ',')
                {
                    /* 经度属性，东西半球，E 0 / W 1  */
                    if(pData[Index] == 'E')
                        m_ucLongitudeAttribute = 0;
                    else if (pData[Index] == 'W')
                        m_ucLongitudeAttribute = 1;
                    else
                    {
                        Result = false;
                        /* 置<5> 经度属性标志位 */
                        m_uhReceriveGPGGA_Flag &= (~WORD_BIT_4_GPS);
                    }

                    if(Result)
                    {
                        Index ++;
                        /* 置<5> 经度属性标志位 */
                        m_uhReceriveGPGGA_Flag |= WORD_BIT_4_GPS;

                        /* 成功取得经度属性标志 */
                        bGetLongAttr  = true;
                    }
                }
                else
                {
                    Result = false;
                    /* 置<5> 经度属性标志位 */
                    m_uhReceriveGPGGA_Flag &= (~WORD_BIT_4_GPS);
                }
            }
            break;
        case 6:
            {
                /* <6> 接收机定位标志，(0=没有定位，1=实时GPS，2=差分GPS，6=正在估算) */
                if(pData[Index] != ',')
                {
                    /* 接收机定位标志，(0=没有定位，1=实时GPS，2=差分GPS，6=正在估算) */
                    m_ucReceriverLocationFlag = pData[Index] - '0';
                    Index ++;

                    /* 置<6> 接收机定位标志标志位 */
                    m_uhReceriveGPGGA_Flag |= WORD_BIT_5_GPS;
                }
                else
                {
                    Result = false;
                    /* 置<6> 接收机定位标志标志位 */
                    m_uhReceriveGPGGA_Flag &= (~WORD_BIT_5_GPS);
                }
            }
            break;
        case 7:
            {
                /* <7> 跟踪到的GPS卫星数，从00到12(不足位数时前面补0) */
                if(pData[Index] != ',')
                {
                    m_ucStarNum = 0;
                    for(i = Index; i < Index + 2 && pData[i] != ','; i++)
                    {
                        if(pData[i] >= '0' && pData[i] <= '9')
                            m_ucStarNum = m_ucStarNum * 10 + (pData[i] - '0');
                        else
                        {
                            Result = false;
                            /* 置<7> 跟踪到的GPS卫星数标志位 */
                            m_uhReceriveGPGGA_Flag &= (~WORD_BIT_6_GPS);
                            break;
                        }
                    }

                    //Index += 2;
                                        Index = i;

                    if(m_ucStarNum <= 12 && Result)
                    {
                        /* 判断是否可以使用经纬度信息 */
                        if(		m_ucStarNum >= m_ucEnableUseLongLat_StarNum
                            &&	bGetLatData
                            &&	bGetLongData
                            &&	bGetLatAttr
                            &&	bGetLongAttr
                            &&  m_ucReceriverLocationFlag)
                        {
                            /*  可以使用经纬度数值标志 */
                            m_bEnableUseLongLat_Flag = true;
                        }
                        else
                            /*  可以使用经纬度数值标志 */
                            m_bEnableUseLongLat_Flag = false;

                        /* 判断是否可以使用日时间 */
                        if(		(m_ucStarNum >= m_ucEnableUseLongLat_StarNum)
                            &&	(m_uhReceriveGPGGA_Flag & WORD_BIT_0_GPS)    )
                        {
                            /* 可以使用日时间标志 */
                            m_bEnableUseTime_Flag = true;
                        }
                        else
                            /* 可以使用日时间标志 */
                            m_bEnableUseTime_Flag = false;

                        /* 置<7> 跟踪到的GPS卫星数标志位 */
                        m_uhReceriveGPGGA_Flag |= WORD_BIT_6_GPS;
                    }
                    else
                    {
                        m_ucStarNum = 0;
                        Result = false;
                        /* 置<7> 跟踪到的GPS卫星数标志位 */
                        m_uhReceriveGPGGA_Flag &= (~WORD_BIT_6_GPS);
                    }
                }
                else
                {
                    m_ucStarNum = 0;
                    Result = false;
                    /* 置<7> 跟踪到的GPS卫星数标志位 */
                    m_uhReceriveGPGGA_Flag &= (~WORD_BIT_6_GPS);
                }
            }
            break;
        case 8:
            {
                /* <8> 水平精度因子，0.5到99.9
                   水平精度因子数据位个数不定，需使用while 循环结构*/
                if(pData[Index] != ',')
                {
                    *Msg = pData[Index++];
                    count = 1;
                    while(	pData[Index] != ','
                        && 	(	(pData[Index] >= '0' &&  pData[Index] <= '9')
                             || (pData[Index] == '.' )))
                    {
                        *(Msg + count) = pData[Index++];
                        count++;
                    }

                    *(Msg + count) = '\0';
                    m_fH_PrecisonGene = (float) atof(Msg);


                    if(pData[Index] != ',' || m_fH_PrecisonGene < 0.5f || m_fH_PrecisonGene > 99.9f)
                    {
                        Result = false;
                        /* 置<8> 水平精度因子标志位 */
                        m_uhReceriveGPGGA_Flag &= (~WORD_BIT_7_GPS);
                        m_fH_PrecisonGene = 0.0f;
                    }
                    else
                        /* 置<8> 水平精度因子标志位 */
                        m_uhReceriveGPGGA_Flag |= WORD_BIT_7_GPS;
                }
                else
                {
                    m_ucStarNum = 0;
                    Result = false;
                    /* 置<8> 水平精度因子标志位 */
                    m_uhReceriveGPGGA_Flag |= WORD_BIT_7_GPS;
                }
            }
            break;
        case 9:
            {
                /* <9> GPS天线所处海拔高度，－9999.9到99999.9
                       GPS天线所处海拔高度数据位个数不定，需使用while 循环结构*/
                if(pData[Index] != ',')
                {
                    *Msg = pData[Index++];
                    count = 1;

                    while(	pData[Index] != ','
                        && 	(	(pData[Index] >= '0' &&  pData[Index] <= '9')
                             || (pData[Index] == '.' )
                             || (pData[Index] == '-' )))
                    {
                        *(Msg + count) = pData[Index++];
                        count++;
                    }
                    *(Msg + count) = '\0';
                    m_fHeight = (float)(atof(Msg));

                    if(pData[Index] != ',' || m_fHeight < -9999.9f ||	m_fHeight > 99999.9f)
                    {
                        Result = false;
                        /* 置<9> GPS天线所处海拔高度标志位 */
                        m_uhReceriveGPGGA_Flag &= (~WORD_BIT_8_GPS);
                        m_fHeight = 0.0f;
                    }
                    else
                    {
                        /* 置<9> GPS天线所处海拔高度标志位 */
                        m_uhReceriveGPGGA_Flag |= WORD_BIT_8_GPS;

                        /*  可以使用高度数值标志 */
                        if( m_ucStarNum >= m_ucEnableUseLongLat_StarNum  && m_ucReceriverLocationFlag)
                            m_bEnableUseHeight_Flag	= true;
                        else
                            m_bEnableUseHeight_Flag	= false;
                    }
                }
                else
                {
                    Result = false;
                    /* 置<9> GPS天线所处海拔高度标志位 */
                    m_uhReceriveGPGGA_Flag &= (~WORD_BIT_8_GPS);
                    m_fHeight = 0.0f;
                }
            }
            break;
        case 10:
            {
                /* <M> 表示GPS天线所处海拔高度单位为米；*/
                if(pData[Index] == 'M')
                {
                    Index++;
                    /* 置 <M> 标志位 */
                    m_uhReceriveGPGGA_Flag |= WORD_BIT_9_GPS;
                }
                else
                {
                    Result = false;
                    /* 置 <M> 标志位 */
                    m_uhReceriveGPGGA_Flag &= (~WORD_BIT_9_GPS);
                }
            }
            break;
        case 11:
            {
                /* <10> 大地水准面高度，－999.9到9999.9
                    大地水准面高度数据位个数不定，需使用while 循环结构 */
                if(pData[Index] != ',')
                {
                    *Msg = pData[Index++];
                    count = 1;

                    while(	pData[Index] != ','
                        && 	(	(pData[Index] >= '0' &&  pData[Index] <= '9')
                             || (pData[Index] == '.' )
                             || (pData[Index] == '-' )))
                    {
                        *(Msg + count) = pData[Index++];
                        count++;
                    }
                    *(Msg + count) = '\0';

                    m_fWaterLevelHeight = (float)(atof(Msg));

                    if(pData[Index] != ',' || m_fWaterLevelHeight < -999.9f || m_fWaterLevelHeight > 9999.9f)
                    {
                        Result = false;
                        m_fWaterLevelHeight = 0;
                        /* 置 <10> 大地水准面高度标志位 */
                        m_uhReceriveGPGGA_Flag &= (~WORD_BIT_10_GPS);
                    }
                    else
                        /* 置 <10> 大地水准面高度标志位 */
                        m_uhReceriveGPGGA_Flag |= WORD_BIT_10_GPS;
                }
                else
                {
                    Result = false;
                    m_fWaterLevelHeight = 0;
                    /* 置 <10> 大地水准面高度标志位 */
                    m_uhReceriveGPGGA_Flag &= (~WORD_BIT_10_GPS);
                }
            }
            break;
        case 12:
            {
                /* <M> 表示大地水准面高度单位为米；*/
                if(pData[Index] == 'M')
                {
                    /* 置 <M> 标志位 */
                    m_uhReceriveGPGGA_Flag |= WORD_BIT_11_GPS;
                    Index++;
                }
                else
                {
                    Result = false;
                    /* 置 <M> 标志位 */
                    m_uhReceriveGPGGA_Flag &= (~WORD_BIT_11_GPS);
                }
            }
            break;
        case 13:
            {
                /*	<11> 有效数据年龄，最后一次有效差分定位时和现在的时间间隔,
                    单位为秒。若不是差分GPS，则此信息位为空；
                    有效数据年龄数据位个数不定，需使用while 循环结构   */
                if(pData[Index] != ',')
                {
                    *Msg = pData[Index++];
                    count = 1;

                    while(pData[Index] != ','
                        && 	(	(pData[Index] >= '0' &&  pData[Index] <= '9')
                             || (pData[Index] == '.' )))
                    {
                        *(Msg + count) = pData[Index++];
                        count++;
                    }
                    *(Msg + count) = '\0';

                    if(pData[Index] == ',')
                    {
                        m_uhValidTime = atoi(Msg);
                        /* 置 <11> 有效数据年龄标志位 */
                        m_uhReceriveGPGGA_Flag |= WORD_BIT_12_GPS;
                    }
                    else
                    {
                        Result = false;
                        /* 置 <11> 有效数据年龄标志位 */
                        m_uhReceriveGPGGA_Flag &= (~WORD_BIT_12_GPS);
                    }
                }
                else
                    /* 置 <11> 有效数据年龄标志位 */
                    m_uhReceriveGPGGA_Flag &= (~WORD_BIT_12_GPS);
            }
            break;
        case 14:
            {
                /*	<12> 差分基准站号(0000～1023，不足位数时前面补0)，
                    若不是差分GPS，则置为最大值65535		*/
                if(pData[Index] != '*')
                {
                    for(i = Index; i<Index + 4; i++)
                    {
                        if(pData[i] != '*' && pData[i] >= '0' && pData[i] <= '9')
                            *(Msg + i -Index ) = pData[i];
                        else
                        {
                            Result = false;
                            /* 置 <12>	差分基准站号标志位 */
                            m_uhReceriveGPGGA_Flag &= (~WORD_BIT_13_GPS);
                            break;
                        }
                    }
                    if(Result)
                    {
                        /* 取得数据 */
                        *(Msg + 4) = '\0';
                        m_uhDifferenceStationNum = (unsigned short)atoi(Msg);
                        if(m_uhDifferenceStationNum >=0 && m_uhDifferenceStationNum <=1023)
                            /* 置 <12>	差分基准站号标志位 */
                            m_uhReceriveGPGGA_Flag |= WORD_BIT_13_GPS;
                        else
                        {
                            m_uhDifferenceStationNum = 65535;
                            /* 置 <12>	差分基准站号标志位 */
                            m_uhReceriveGPGGA_Flag &= (~WORD_BIT_13_GPS);
                        }

                        Index += 4;
                    }
                }
                else
                {
                    m_uhDifferenceStationNum = 65535;
                    /* 置 <12>	差分基准站号标志位 */
                    m_uhReceriveGPGGA_Flag &= (~WORD_BIT_13_GPS);
                }
            }
            break;
        default:
            {
                Result = false;
                break;
            }
        }/* switch 结束 */
    }/* while 结束 */

    return Result;
}

/*-----------------------------------------------------------------------------
 函数名：	CGPS::GetGPS_GPRMC

 描述:		获取GPS数据GPRMC格式数据

 详细描述：	获取GPS数据GPRMC格式数据,GPS定位信息建议使用最小GPS数据格式

 例：
        $GPRMC,031302.6,V,3151.6966,N,11714.4896,E,,,220709,04,W*44
        $GPRMC,062320,V,3537.8333,N,13944.6667,E,000.0,000.0,030222,,*0D
        $GPRMC,030028,A,3957.6874,N,11626.8001,E,000.0,000.0,010305,005.7,W*66
        $GPRMC,121252.000,A,3958.3032,N,11629.6046,E,15.15,359.95,070306,,,A*54

 其标准格式为：	$GPRMC,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>*hh<CR><LF>

        <1> UTC当前时间，格式为hhmmss(时时分分秒秒);
        <2> 状态字，A表示定位成功，V表示目前没有定位;
        <3> 纬度，格式为ddmm.mmmm (度度分分.分分分分，不足位数时前面补0);
        <4> 纬度属性，南北半球，N/S;
        <5> 经度，格式为dddmm.mmmm(度度度分分.分分分分，不足位数时前面补0);
        <6> 经度属性，东西半球，E/W;
        <7> 天线移动的速度，从000.0到999.9节,不足位数时前面补0（也存在0.081611的情况，或者不存在）;
        <8> 相对地面方向，000.0到359.9度,以真北为参考基准，不足位数时前面补0（也存在15.81，或者不存在）;
        <9> 当前日期，格式为ddmmyy（日月年）;
        <10> 磁偏角，从000.0到180.0度，不足位数时前面补0(或者不存在);
        <11> 磁偏方向属性，0/E(东)或1/W(西)(或者不存在);
        <12> 模式指示（仅NMEA0183 3.00版本输出，A=自主定位，D=差分，E=估算，N=数据无效）；
        <*> 校验和标志，其后面的一个字节即后面的<hh>表示校验和
        (CR)(LF)回车，换行。

 参数:	    UCHAR *pData

 返回值：	bool 获取GPS数据GPRMC格式数据成功标志

 注意事项:	无
 -----------------------------------------------------------------------------*/
bool CGPS::GetGPS_GPRMC(unsigned char* pData)
{
    unsigned char c = '\a';
    bool Result = true;

    /*	成功取得经纬度数据和经纬度属性标志,只有全为true，
        并且卫星个数大于5时，才使用经纬度信息*/
    bool bGetLatData  = false,bGetLongData  = false;
    bool bGetLatAttr  = false,bGetLongAttr  = false;

    char Msg[210];
    unsigned char  Hour = 0,Minute = 0,Second = 0;
    unsigned char  Month = 0,Day = 0;
    unsigned short  Year = 0;
    unsigned char count = 0;
    unsigned short l_Degree =0,l_Minute =0;
    unsigned int i =0,Index = 0,l_Second =0;
    unsigned char GetDataCount = 0;

    /*	首先计算检查和，如果检查和错误，则认为本组数据出错，不予处理
        检查和的计算方法：'$'和'*'之间所有字符进行异或后的值（不包括'$'和'*'）;
        结果与hh 比较，注意：hh是十六进制	*/
        if(!Check((unsigned char*)"GPRMC", pData))
        {
                return false;
        }
        else
        {
        /*-------------清除参数--------------*/
        /*  收到GPRMC报文标志 */
        m_uhReceriveGPRMC_Flag = 0;

        /* 置 * 检查和 标志位 */
        m_uhReceriveGPRMC_Flag |= WORD_BIT_15_GPS;
        m_uhReceriveGPRMC_Flag |= WORD_BIT_14_GPS;

        /*  NMEA0183协议是否是V3.0版本 */
        m_bVer3 = false;
        }

    /*-------------依次读取数据，并解释数据--------------*/
    while(c != '\n' && Result && pData[Index] != '*' && pData[Index] != '\0')
    {
        c = pData[Index++];
        if(c == ',')
        {
            GetDataCount ++;
        }

        /* 根据“,”的个数来执行相应字段的解释工作 */
        switch(GetDataCount)
        {
        case 1:
            {
                /* ------------------------------------------------
                    <1>	UTC当前时间，格式为hhmmss(时时分分秒秒);
                    处理老式GPS，UniStar II型GPS的时间后面带小数点,
                    II A+型不带小数点,本模块兼容这两种结构。
                  ------------------------------------------------*/
                /* 有时间信息 */
                if(pData[Index] != ',')
                {
                    /* 时时 hh */
                    Hour = 0;
                    for(i = Index; i < Index + 2; i++)
                    {
                        if(pData[i] >= '0' && pData[i] <= '9')
                            Hour = Hour * 10 + (pData[i] - '0');
                        else
                        {
                            Result = false;
                            /* 置<1>UTC当前时间标志位 */
                            m_uhReceriveGPRMC_Flag &= (~WORD_BIT_0_GPS);
                            break;
                        }
                    }
                    /* 分分 mm */
                    if(Result && Hour < 24)
                    {
                        /* 保存“时” */
                        m_ucHour = Hour;

                        Minute = 0;
                        Index += 2;
                        for(i = Index; i < Index + 2; i++)
                        {
                            if(pData[i] >= '0' && pData[i] <= '9')
                                Minute = Minute * 10 + (pData[i] - '0');
                            else
                            {
                                Result = false;
                                /* 置<1>UTC当前时间标志位 */
                                m_uhReceriveGPRMC_Flag &= (~WORD_BIT_0_GPS);
                                break;
                            }
                        }

                        /* 秒秒 ss */
                        if(Result && Minute < 60)
                        {
                            /* 保存“分” */
                            m_ucMinute = Minute;
                            Second = 0;
                            Index += 2;
                            for(i = Index; i < Index + 2; i++)
                            {
                                if(pData[i] >= '0' && pData[i] <= '9')
                                    Second = Second * 10 + (pData[i] - '0');
                                else
                                {
                                    Result = false;
                                    /* 置<1>UTC当前时间标志位 */
                                    m_uhReceriveGPRMC_Flag &= (~WORD_BIT_0_GPS);
                                    break;
                                }
                            }
                            if(Result && Second < 60)
                            {
                                Index += 2;
                                m_ucSecond = Second;

                                /* 处理老式GPS，UniStar II型GPS的时间后面带小数点*/
                                if(pData[Index] == '.')
                                {
                                    Index += 1;

                                    /*	秒的小数点后数据位个数不定(现已发现有1位和3位的情况)，
                                        需使用while 循环结构，取出后直接保存为毫秒*/
                                    count = 0;
                                    if(pData[Index] != ',')
                                    {
                                        *Msg = '0';
                                        *(Msg +1) = '.';
                                        count += 2;
                                        while(pData[Index] != ','
                                            && 	(	(pData[Index] >= '0' &&  pData[Index] <= '9')
                                                 || (pData[Index] == '.' )))
                                        {
                                            *(Msg + count) = pData[Index++];
                                            count++;
                                        }
                                        *(Msg + count) = '\0';

                                        if(pData[Index] == ',')
                                        {
                                            m_uhMillisecond = (unsigned short)(atof(Msg) * 1000);

                                            /*  时间信息秒是否带有小数点 */
                                            m_bSecondWithDot = true;
                                        }
                                        else
                                        {
                                            Result = false;
                                            /* 置<1>UTC当前时间标志位 */
                                            m_uhReceriveGPRMC_Flag &= (~WORD_BIT_0_GPS);
                                        }
                                    }
                                    else
                                    {
                                        /*  时间信息秒是否带有小数点 */
                                        m_bSecondWithDot = false;
                                    }
                                }
                                else
                                {
                                    /*  时间信息秒是否带有小数点 */
                                    m_bSecondWithDot = false;
                                }

                                /* 置<1>UTC当前时间标志位 */
                                m_uhReceriveGPRMC_Flag |= WORD_BIT_0_GPS;

                                if(m_ucStarNum >= m_ucEnableUseLongLat_StarNum)
                                {
                                    /* 可以使用日时间标志 */
                                    m_bEnableUseTime_Flag	= true;
                                }
                            }
                            else
                            {
                                Result = false;
                                /* 置<1>UTC当前时间标志位 */
                                m_uhReceriveGPRMC_Flag &= (~WORD_BIT_0_GPS);
                            }
                        }
                        else
                        {
                            Result = false;
                            /* 置<1>UTC当前时间标志位 */
                            m_uhReceriveGPRMC_Flag &= (~WORD_BIT_0_GPS);
                        }
                    }
                    else
                    {
                        Result = false;
                        /* 置<1>UTC当前时间标志位 */
                        m_uhReceriveGPRMC_Flag &= (~WORD_BIT_0_GPS);
                    }
                }
                else
                {
                    Result = false;
                    /* 置<1>UTC当前时间标志位 */
                    m_uhReceriveGPRMC_Flag &= (~WORD_BIT_0_GPS);
                }
            }
            break;
        case 2:
            {
                /* <2> 状态字，A表示定位成功，V表示目前没有定位 */
                if(pData[Index] != ',')
                {
                    /* 状态字，A表示定位成功，V表示目前没有定位 */
                    if(pData[Index] == 'A')
                        m_ucGP_LocationSucessFlag = 1;
                    else if (pData[Index] == 'V')
                        m_ucGP_LocationSucessFlag = 0;
                    else
                    {
                        Result = false;
                        /* 置<2> 状态字 */
                        m_uhReceriveGPRMC_Flag &= (~WORD_BIT_1_GPS);
                    }
                    if(Result)
                    {
                        Index++;
                        /* 置<2> 状态字标志位 */
                        m_uhReceriveGPRMC_Flag |= WORD_BIT_1_GPS;
                    }
                }
                else
                {
                    Result = false;
                    /* 置<2> 状态字 */
                    m_uhReceriveGPRMC_Flag &= (~WORD_BIT_1_GPS);
                }
            }
            break;
        case 3:
            {
                /* <3> 纬度，格式为ddmm.mmmm (度度分分.分分分分，不足位数时前面补0) */
                /* 有纬度信息 */
                if(pData[Index] != ',')
                {
                    l_Degree = 0;
                    /* 度度 dd */
                    for(i = Index; i < Index + 2; i++)
                    {
                        if(pData[i] >= '0' && pData[i] <= '9')
                            l_Degree = l_Degree * 10 + (pData[i] - '0');
                        else
                        {
                            Result = false;
                            /* 置<3> 纬度标志位 */
                            m_uhReceriveGPRMC_Flag &= (~WORD_BIT_2_GPS);
                            break;
                        }
                    }

                    if(Result && l_Degree <= 90)
                    {
                        Index += 2;
                        m_sLatitude.hDegree = l_Degree;
                        l_Minute = 0;
                        /* 分分 mm */
                        for(i = Index; i < Index + 2; i++)
                        {
                            if(pData[i] >= '0' && pData[i] <= '9')
                                l_Minute = l_Minute * 10 + (pData[i] - '0');
                            else
                            {
                                Result = false;
                                /* 置<3> 纬度标志位 */
                                m_uhReceriveGPRMC_Flag &= (~WORD_BIT_2_GPS);
                                break;
                            }
                        }

                        if(Result && l_Minute < 60)
                        {
                            Index += 2;
                            m_sLatitude.hMinute = l_Minute;

                            /*.分分分分 .mmmm  转化成秒*/
                            l_Second = 0;

                            if(pData[Index] == '.')
                            {
                                Index += 1;

                                /* 因为发现有带4位小数的，有带2位小数的，因此需要使用while 结构 */
                                *Msg = '0';
                                *(Msg + 1) = '.';
                                count = 2;

                                while(	pData[Index] != ','
                                     && pData[Index] >= '0'
                                     && pData[Index] <= '9')
                                {
                                    *(Msg + count) = pData[Index++];
                                    count ++;
                                }
                                *(Msg + count) = '\0';

                                if(pData[Index] == ',')
                                {
                                    m_sLatitude.dSecond = atof(Msg) * 60.0;

                                    /* 置<3> 纬度标志位 */
                                    m_uhReceriveGPRMC_Flag |= WORD_BIT_2_GPS;

                                    /* 成功取得纬度数据 */
                                    bGetLatData = true;
                                }
                                else
                                {
                                    Result = false;
                                    /* 置<3> 纬度标志位 */
                                    m_uhReceriveGPRMC_Flag &= (~WORD_BIT_2_GPS);
                                }
                            }
                            else
                            {
                                Result = false;
                                /* 置<3> 纬度标志位 */
                                m_uhReceriveGPRMC_Flag &= (~WORD_BIT_2_GPS);
                            }
                        }
                        else
                        {
                            Result = false;
                            /* 置<3> 纬度标志位 */
                            m_uhReceriveGPRMC_Flag &= (~WORD_BIT_2_GPS);
                        }
                    }
                    else
                    {
                        Result = false;
                        /* 置<3> 纬度标志位 */
                        m_uhReceriveGPRMC_Flag &= (~WORD_BIT_2_GPS);
                    }
                }
                else
                {
                    Result = false;
                    /* 置<3> 纬度标志位 */
                    m_uhReceriveGPRMC_Flag &= (~WORD_BIT_2_GPS);
                }
            }
            break;
        case 4:
            {
                /* <4> 纬度属性，南北半球，N/S */
                if(pData[Index] != ',')
                {
                    /* 纬度属性，南北半球，N 0 / S 1 */
                    if(pData[Index] == 'N')
                        m_ucLatitudeAttribute = 0;
                    else if (pData[Index] == 'S')
                        m_ucLatitudeAttribute = 1;
                    else
                    {
                        Result = false;
                        /* 置<4> 纬度属性标志位 */
                        m_uhReceriveGPRMC_Flag &= (~WORD_BIT_3_GPS);
                    }

                    if(Result)
                    {
                        Index++;
                        /* 置<4> 纬度属性标志位 */
                        m_uhReceriveGPRMC_Flag |= WORD_BIT_3_GPS;

                        /* 成功取得经纬度属性标志 */
                        bGetLatAttr  = true;
                    }
                }
                else
                {
                    Result = false;
                    /* 置<4> 纬度属性标志位 */
                    m_uhReceriveGPRMC_Flag &= (~WORD_BIT_3_GPS);
                }
            }
            break;
        case 5:
            {
                /* <5> 经度,格式为dddmm.mmmm(度度度分分.分分分分,不足位数时前面补0) */
                if(pData[Index] != ',')
                {
                    /* 度度 dd */
                    l_Degree = 0;
                    for(i = Index; i < Index + 3; i++)
                    {
                        if(pData[i] >= '0' && pData[i] <= '9')
                            l_Degree = l_Degree * 10 + (pData[i] - '0');
                        else
                        {
                            Result = false;
                            /* 置<5> 经度标志位 */
                            m_uhReceriveGPRMC_Flag &= (~WORD_BIT_4_GPS);
                            break;
                        }
                    }
                    if(Result && l_Degree <= 180)
                    {
                        Index += 3;
                        m_sLongitude.hDegree = l_Degree;

                        /* 分分 mm */
                        l_Minute = 0;
                        for(i = Index; i < Index + 2; i++)
                        {
                            if(pData[i] >= '0' && pData[i] <= '9')
                                l_Minute = l_Minute * 10 + (pData[i] - '0');
                            else
                            {
                                Result = false;
                                /* 置<5> 经度标志位 */
                                m_uhReceriveGPRMC_Flag &= (~WORD_BIT_4_GPS);
                                break;
                            }
                        }

                        if(Result && l_Minute < 60)
                        {
                            Index += 2;
                            m_sLongitude.hMinute = l_Minute;

                            l_Second = 0;

                            /*.分分分分 .mmmm  转化成秒*/
                            if(pData[Index] == '.')
                            {
                                Index += 1;
                                /* 因为发现有带4位小数的，有带2位小数的，因此需要使用while 结构 */
                                *Msg = '0';
                                *(Msg + 1) = '.';
                                count = 2;

                                while(	pData[Index] != ','
                                     && pData[Index] >= '0'
                                     && pData[Index] <= '9')
                                {
                                    *(Msg + count) = pData[Index++];
                                    count ++;
                                }
                                *(Msg + count) = '\0';

                                if(pData[Index] == ',')
                                {
                                    m_sLongitude.dSecond = atof(Msg) * 60.0;

                                    /* 置<5> 经度标志位 */
                                    m_uhReceriveGPRMC_Flag |= WORD_BIT_4_GPS;

                                    /* 成功取得经度数据 */
                                    bGetLongData  = true;
                                }
                                else
                                {
                                    Result = false;
                                    /* 置<5> 经度标志位 */
                                    m_uhReceriveGPRMC_Flag &= (~WORD_BIT_4_GPS);
                                }
                            }
                            else
                            {
                                Result = false;
                                /* 置<5> 经度标志位 */
                                m_uhReceriveGPRMC_Flag &= (~WORD_BIT_4_GPS);
                            }
                        }
                        else
                        {
                            Result = false;
                            /* 置<5> 经度标志位 */
                            m_uhReceriveGPRMC_Flag &= (~WORD_BIT_4_GPS);
                        }
                    }
                    else
                    {
                        Result = false;
                        /* 置<5> 经度标志位 */
                        m_uhReceriveGPRMC_Flag &= (~WORD_BIT_4_GPS);
                    }
                }
                else
                {
                    Result = false;
                    /* 置<5> 经度标志位 */
                    m_uhReceriveGPRMC_Flag &= (~WORD_BIT_4_GPS);
                }
            }
            break;
        case 6:
            {
                /* <6> 经度属性，东西半球，E/W */
                if(pData[Index] != ',')
                {
                    /* 经度属性，东西半球，E 0 / W 1  */
                    if(pData[Index] == 'E')
                        m_ucLongitudeAttribute = 0;
                    else if (pData[Index] == 'W')
                        m_ucLongitudeAttribute = 1;
                    else
                    {
                        Result = false;
                        /* 置<6> 经度属性标志位 */
                        m_uhReceriveGPRMC_Flag &= (~WORD_BIT_5_GPS);
                    }

                    if(Result)
                    {
                        Index ++;
                        /* 置<6> 经度属性标志位 */
                        m_uhReceriveGPRMC_Flag |= WORD_BIT_5_GPS;

                        /* 成功取得经度属性标志 */
                        bGetLongAttr  = true;

                        /* 判断是否可以使用经纬度信息 */
                        if(		m_ucStarNum >= m_ucEnableUseLongLat_StarNum
                            &&	bGetLatData
                            &&	bGetLongData
                            &&	bGetLatAttr
                            &&	bGetLongAttr
                            &&  m_ucGP_LocationSucessFlag)
                        {
                            /*  可以使用经纬度数值标志 */
                            m_bEnableUseLongLat_Flag = true;
                        }
                        else
                            /*  可以使用经纬度数值标志 */
                            m_bEnableUseLongLat_Flag = false;
                    }
                }
                else
                {
                    Result = false;
                    /* 置<6> 经度属性标志位 */
                    m_uhReceriveGPRMC_Flag &= (~WORD_BIT_5_GPS);
                }
            }
            break;
        case 7:
            {
                /* <7> 天线移动的速度，从000.0到999.9节,不足位数时前面补0(也存在0.081611的情况，或者不存在) */
                if(pData[Index] != ',')
                {
                    count = 0;
                    *Msg  = pData[Index++];
                    count++;
                    while(pData[Index] != ','
                        && 	(	(pData[Index] >= '0' &&  pData[Index] <= '9')
                             || (pData[Index] == '.' )))
                    {
                        *(Msg + count) = pData[Index++];
                        count++;
                    }

                    *(Msg + count) = '\0';
                    /* 天线移动的速度，从000.0到999.9节 */
                    m_fGPRMC_Velocity_knots = (float) atof(Msg);

                    if((pData[Index] != ',') || ((m_fGPRMC_Velocity_knots < 0.0f) && (m_fGPRMC_Velocity_knots > 999.9f)))
                    {
                        Result = false;
                        /* 置<7> 天线移动的速度标志位 */
                        m_uhReceriveGPRMC_Flag &= (~WORD_BIT_6_GPS);
                    }
                    else
                    {
                        /* 置<7> 天线移动的速度标志位 */
                        m_uhReceriveGPRMC_Flag |= WORD_BIT_6_GPS;
                    }
                }
                else
                {
                    /* 置<7> 天线移动的速度标志位 */
                    m_uhReceriveGPRMC_Flag &= (~WORD_BIT_6_GPS);
                }
            }
            break;
        case 8:
            {
                /* <8> 相对地面方向，000.0到359.9度,以真北为参考基准，不足位数时前面补0(也存在15.81，或者不存在) */
                if(pData[Index] != ',')
                {
                    count = 0;
                    *Msg  = pData[Index++];
                    count++;
                    while(pData[Index] != ','
                        && 	(	(pData[Index] >= '0' &&  pData[Index] <= '9')
                             || (pData[Index] == '.' )))
                    {
                        *(Msg + count) = pData[Index++];
                        count++;
                    }

                    *(Msg + count) = '\0';

                    /* 相对地面方向，000.0到359.9度,以真北为参考基准 */
                    m_fGPRMC_Velocity_Azimuth = (float) atof(Msg);

                    if(pData[Index] != ',' || m_fGPRMC_Velocity_Azimuth < 0.0f || m_fGPRMC_Velocity_Azimuth > 360.0f)
                    {
                        Result = false;
                        /* 置<8>相对地面方向标志位 */
                        m_uhReceriveGPRMC_Flag &= (~WORD_BIT_7_GPS);
                    }
                    else
                    {
                        /* 置<8>相对地面方向标志位 */
                        m_uhReceriveGPRMC_Flag |= WORD_BIT_7_GPS;
                    }
                }
                else
                {
                    /* 置<8>相对地面方向标志位 */
                    m_uhReceriveGPRMC_Flag &= (~WORD_BIT_7_GPS);
                }
            }
            break;
        case 9:
            {
                /* <9> 当前日期，格式为ddmmyy(日月年) */
                /* 有日期信息 */
                if(pData[Index] != ',')
                {
                    /* 日 dd */
                    Day = 0;
                    for(i = Index; i < Index + 2; i++)
                    {
                        if(pData[i] >= '0' && pData[i] <= '9')
                            Day = Day * 10 + (pData[i] - '0');
                        else
                        {
                            Result = false;
                            /* 置<9>当前日期标志位 */
                            m_uhReceriveGPRMC_Flag &= (~WORD_BIT_8_GPS);
                            break;
                        }
                    }
                    /* 月 mm */
                    if(Result && Day <= 31)
                    {
                        /* 保存“日*/
                        m_ucDay= Day;

                        Month = 0;
                        Index += 2;
                        for(i = Index; i < Index + 2; i++)
                        {
                            if(pData[i] >= '0' && pData[i] <= '9')
                                Month = Month * 10 + (pData[i] - '0');
                            else
                            {
                                Result = false;
                                /* 置<9>当前日期标志位 */
                                m_uhReceriveGPRMC_Flag &= (~WORD_BIT_8_GPS);
                                break;
                            }
                        }

                        /* 年 yy */
                        if(Result && Month <= 12)
                        {
                            /* 保存“月” */
                            m_ucMonth = Month;
                            Year = 0;
                            Index += 2;
                            for(i = Index; i < Index + 2; i++)
                            {
                                if(pData[i] >= '0' && pData[i] <= '9')
                                    Year = Year * 10 + (pData[i] - '0');
                                else
                                {
                                    Result = false;
                                    /* 置<9>当前日期标志位 */
                                    m_uhReceriveGPRMC_Flag &= (~WORD_BIT_8_GPS);
                                    break;
                                }
                            }

                            if(Result)
                            {
                                Index += 2;
                                /* 年保存为完整的格式 有效在2100年前有效 */
                                m_uhYear = Year + 2000;

                                /* 置<9>当前日期标志位 */
                                m_uhReceriveGPRMC_Flag |= WORD_BIT_8_GPS;

                                if(m_ucStarNum >= m_ucEnableUseLongLat_StarNum)
                                {
                                    /* 可以使用日时间标志 */
                                    m_bEnableUseDate_Flag = true;
                                }
                            }
                        }
                        else
                        {
                            Result = false;
                            /* 置<9>当前日期标志位 */
                            m_uhReceriveGPRMC_Flag &= (~WORD_BIT_8_GPS);
                        }
                    }
                    else
                    {
                        Result = false;
                        /* 置<9>当前日期标志位 */
                        m_uhReceriveGPRMC_Flag &= (~WORD_BIT_8_GPS);
                    }
                }
                else
                {
                    Result = false;
                    /* 置<9>当前日期标志位 */
                    m_uhReceriveGPRMC_Flag &= (~WORD_BIT_8_GPS);
                }
            }
            break;
        case 10:
            {
                /* <10> 磁偏角，从000.0到180.0度，不足位数时前面补0 */
                if(pData[Index] != ',')
                {
                    count = 0;
                    *Msg  = pData[Index++];
                    count++;
                    while(pData[Index] != ','
                        && 	(	(pData[Index] >= '0' &&  pData[Index] <= '9')
                             || (pData[Index] == '.' )))
                    {
                        *(Msg + count) = pData[Index++];
                        count++;
                    }

                    *(Msg + count) = '\0';

                    /* 磁偏角，从000.0到180.0度，不足位数时前面补0 */
                    m_fGPRMC_MagneticAzimuth = (float) atof(Msg);

                    if(pData[Index] != ',' || m_fGPRMC_MagneticAzimuth < 0.0f || m_fGPRMC_MagneticAzimuth > 180.0f)
                    {
                        Result = false;
                        /* 置<10> 磁偏角标志位 */
                        m_uhReceriveGPRMC_Flag &= (~WORD_BIT_9_GPS);
                    }
                    else
                    {
                        /* 置<10> 磁偏角标志位 */
                        m_uhReceriveGPRMC_Flag |= WORD_BIT_9_GPS;
                    }
                }
                else
                {
                    /* 置<10> 磁偏角标志位 */
                    m_uhReceriveGPRMC_Flag &= (~WORD_BIT_9_GPS);
                }
            }
            break;
        case 11:
            {
                /* <11> 磁偏方向属性，0/E(东)或1/W(西) */
                if(pData[Index] != ',')
                {
                    /* 磁偏方向属性 */
                    if(pData[Index] == 'E')
                        m_ucGPRMC_MagneticAttr = 0;
                    else if (pData[Index] == 'W')
                        m_ucGPRMC_MagneticAttr = 1;
                    /*	要判断当磁偏方向属性 为空，并且不存在模式指示的情况*/
                    else if (pData[Index] == '*')
                    {
                        /* 置<11> 磁偏方向属性标志位 */
                        m_uhReceriveGPRMC_Flag &= (~WORD_BIT_10_GPS);

                        /* 置<*>  标志位 */
                        m_uhReceriveGPRMC_Flag |= WORD_BIT_14_GPS;
                        break;
                    }
                    else
                    {
                        Result = false;
                        /* 置<11> 磁偏方向属性标志位 */
                        m_uhReceriveGPRMC_Flag &= (~WORD_BIT_10_GPS);
                    }

                    if(Result)
                    {
                        /* 置<11> 磁偏方向属性标志位 */
                        m_uhReceriveGPRMC_Flag |= WORD_BIT_10_GPS;
                        Index ++;

                        /*	要判断当磁偏方向属性正确，并且不存在模式指示的情况*/
                        if (pData[Index] == '*')
                        {
                            /* 置<*>标志位 */
                            m_uhReceriveGPRMC_Flag |= WORD_BIT_14_GPS;
                        }
                        else if (pData[Index] != ',')
                        {
                            /* 数据错误，退出 */
                            Result = false;
                        }
                    }
                }
                else
                {
                    /*	磁偏方向属性为空，并且存在模式指示 */
                    /* 置<11> 磁偏方向属性标志位 */
                    m_uhReceriveGPRMC_Flag &= (~WORD_BIT_10_GPS);
                }
            }
            break;
        case 12:
            {
                /* <12> 模式指示(仅NMEA0183 3.00版本输出，0/N=数据无效,1/A=自主定位,2/D=差分,3/E=估算) */
                if(pData[Index] != '*')
                {
                    /* 模式指示 */
                    if (pData[Index] == 'A')
                        m_ucGPWorkModel = 1;
                    else if(pData[Index] == 'N')
                        m_ucGPWorkModel = 0;
                    else if(pData[Index] == 'D')
                        m_ucGPWorkModel = 2;
                    else if (pData[Index] == 'E')
                        m_ucGPWorkModel = 3;
                    else
                    {
                        Result = false;
                        /* 置<12> 模式指示标志位 */
                        m_uhReceriveGPRMC_Flag &= (~WORD_BIT_11_GPS);
                    }

                    if(Result)
                    {
                        Index ++;
                        /* 置<12> 模式指示标志位 */
                        m_uhReceriveGPRMC_Flag |= WORD_BIT_11_GPS;

                        /*  NMEA0183协议是否是V3.0版本 */
                        m_bVer3 = true;
                    }
                }
                else
                {
                    /* 置<*> 标志位 */
                    m_uhReceriveGPRMC_Flag |= WORD_BIT_14_GPS;
                }
            }
            break;
        default:
            {
                Result = false;
                break;
            }
        }/* switch 结束 */
    }/* while 结束 */

    return Result;
}

/*-----------------------------------------------------------------------------
 函数名：	CGPS::GetGPS_GPGSV

 描述:		获取GPS数据GPGSV格式数据

 详细描述：	获取GPS数据GPGSVA格式数据(GPS Satellites in View可视卫星状态输出语句)

 例：
        $GPGSV,3,1,10,18,84,067,23,09,67,067,27,22,49,312,28,15,47,231,30*70
        $GPGSV,3,2,10,21,32,199,23,14,25,272,24,05,21,140,32,26,14,070,20*7E
        $GPGSV,3,3,10,29,07,074,,30,07,163,28*7D

        $GPGSV,3,1,11,03,03,111,00,04,15,270,00,06,01,010,00,13,06,292,00*74
        $GPGSV,3,2,11,14,25,170,00,16,57,208,39,18,67,296,40,19,40,246,00*74
        $GPGSV,3,3,11,22,42,067,42,24,14,311,43,27,05,244,00,,,,*4D

其标准格式为：	$GPGSV,<1>,<2>,<3>,<4>,<5>,<6>,<7>,…<4>,<5>,<6>,<7>*hh<CR><LF>

        <1> GSV语句的总数；3
        <2> 本句GSV的编号；1
        <3> 可见卫星的总数，00 至 12；10
        <4> 卫星编号，01～32；18
        <5> 卫星仰角(00～90度)；84度
        <6> 卫星方位角(000～359度)；067度
        <7> 信号信噪比(00～99dB)；23dB，无表示未接收到讯号。(后面依次为第09，22，15号卫星的信息)
        <*> 校验和标志，其后面的一个字节即后面的<hh>表示校验和；总和校验数；70
        (CR)(LF)回车，换行

  注：	每条语句最多包括四颗卫星的信息，每颗卫星的信息有四个数据项，
        即：(4)卫星编号，(5)卫星仰角，(6)卫星方位角，(7)信号信噪比。
        其余卫星信息会于次一行出现，若未使用，这些字段会空白,但是有的
        输出语句则直接把这些空白字段不输出，因此解释时应予以考虑,
        现在不能确定是否有：空字段后又有数据段的情况，因此在解释过程中，
        遇到空的情况不退出，依然往后面解释。

 参数:	    UCHAR *pData

 返回值：	bool 获取GPS数据GPGSV格式数据成功标志

 注意事项:	无
 -----------------------------------------------------------------------------*/
bool CGPS::GetGPS_GPGSV(unsigned char *pData)
{
    unsigned char c = '\a';
    bool Result = true;

    unsigned char StarNum = 0,StarElevation = 0;
    unsigned char StarSNR = 0;
    unsigned short StarAzimuth = 0;
    unsigned char TotalGSV = 0,CurrentGSV = 0;
    unsigned char TotalStar = 0;
    unsigned char GetDataCount = 0;
    unsigned char i = 0,Index = 0;

    /*	首先计算检查和，如果检查和错误，则认为本组数据出错，不予处理'
        检查和的计算方法：'$'和'*'之间所有字符进行异或后的值（不包括'$'和'*'）;
        结果与hh 比较，注意：hh是十六进制	*/
        if(!Check((unsigned char*)"GPGSV", pData))
        {
                return false;
        }
        else
        {
        /*-------------清除参数--------------*/
        /* 置 检查和 标志位 */
        m_bReceriveGPGSV_Flag = true;
        }

    /*-------------依次读取数据，并解释数据--------------*/
    while(c != '\n' && Result && pData[Index] != '*' && pData[Index] != '\0')
    {
        c = pData[Index++];
        if(c == ',')
        {
            GetDataCount ++;
        }

        /* 根据“,”的个数来执行相应字段的解释工作 */
        switch(GetDataCount)
        {
        case 1:
            {
                /* <1> GSV语句的总数，最大3组并且应该大于0 */
                if(pData[Index] != ',')
                {
                    if(pData[Index] > '0' && pData[Index] <= '3')
                        TotalGSV = pData[Index++] - '0';
                    else
                    {
                        Result = false;
                        /* 置 标志位 */
                        m_bReceriveGPGSV_Flag = false;
                    }
                }
                else
                {
                    Result = false;
                    /* 置 标志位 */
                    m_bReceriveGPGSV_Flag = false;
                }
            }
            break;
        case 2:
            {
                /*  <2> 本句GSV的编号，最大3组并且应该大于0 */
                if(pData[Index] != ',')
                {
                    if(pData[Index] > '0' && pData[Index] <= '3')
                        CurrentGSV = pData[Index++] - '0';
                    else
                    {
                        Result = false;
                        /* 置标志位 */
                        m_bReceriveGPGSV_Flag = false;
                    }
                    /* 当前语句编号必须小于总语句数 */
                    if(Result && CurrentGSV > TotalGSV)
                    {
                        Result = false;
                        /* 置标志位 */
                        m_bReceriveGPGSV_Flag = false;
                    }
                }
                else
                {
                    Result = false;
                    /* 置标志位 */
                    m_bReceriveGPGSV_Flag = false;
                }
            }
            break;
        case 3:
            {
                /* <3> 可见卫星的总数, 00 至 12；10*/
                if(pData[Index] != ',')
                {
                    /* 可见卫星的总数 */
                    for(i = Index; i < Index + 2; i++)
                    {
                        if(pData[i] >= '0' && pData[i] <= '9')
                            TotalStar = TotalStar * 10 + (pData[i] - '0');
                        else
                        {
                            Result = false;
                            /* 置标志位 */
                            m_bReceriveGPGSV_Flag = false;
                            break;
                        }
                    }

                    if(Result && TotalStar >= 0 && TotalStar <= 12)
                    {
                        Index += 2;
                        m_ucGPGSV_StarNum = TotalStar;
                    }
                    else
                    {
                        Result = false;
                        /* 置标志位 */
                        m_bReceriveGPGSV_Flag = false;
                    }
                }
                else
                {
                    Result = false;
                    /* 置标志位 */
                    m_bReceriveGPGSV_Flag = false;
                }
            }
            break;
        case 4:
        case 8:
        case 12:
        case 16:
            {
                /* <4> 卫星编号，01～32；18 */
                if(pData[Index] != ',')
                {
                    /* 卫星编号 */
                    StarNum = 0;
                    for(i = Index; i < Index + 2; i++)
                    {
                        if(pData[i] >= '0' && pData[i] <= '9')
                            StarNum = StarNum * 10 + (pData[i] - '0');
                        else
                        {
                            Result = false;
                            /* 置标志位 */
                            m_bReceriveGPGSV_Flag = false;
                            break;
                        }
                    }

                    if(Result &&  StarNum <= 32 && StarNum  > 0 )
                    {
                        Index += 2;
                        m_sgStarInfo[StarNum - 1].ucStarIndex = StarNum;
                        m_sgStarInfo[StarNum - 1].ucStarInfoValid |= WORD_BIT_0_GPS;
                    }
                    else
                    {
                        Result = false;
                        /* 置标志位 */
                        m_bReceriveGPGSV_Flag = false;
                    }
                }
                else
                {
                    /* 如果“卫星编号”字段为空，则应该把StarNum置为0 */
                    StarNum = 0;
                }
            }
            break;
        case 5:
        case 9:
        case 13:
        case 17:
            {
                /* <5> 卫星仰角(00～90度)；84度 */
                if(pData[Index] != ',')
                {
                    /* 卫星仰角 */
                    StarElevation = 0;
                    for(i = Index; i < Index + 2; i++)
                    {
                        if(pData[i] >= '0' && pData[i] <= '9')
                            StarElevation = StarElevation * 10 + (pData[i] - '0');
                        else
                        {
                            if( StarNum <= 32 && StarNum  > 0 )
                                m_sgStarInfo[StarNum - 1].ucStarInfoValid &= (~WORD_BIT_1_GPS);
                            Result = false;
                            /* 置标志位 */
                            m_bReceriveGPGSV_Flag = false;
                            break;
                        }
                    }

                    if(Result && StarElevation >= 0 && StarElevation <= 90)
                    {
                        Index += 2;
                        if( StarNum <= 32 && StarNum  > 0 )
                        {
                            m_sgStarInfo[StarNum - 1].ucStarElevation = StarElevation;
                            m_sgStarInfo[StarNum - 1].ucStarInfoValid |= WORD_BIT_1_GPS;
                        }
                    }
                    else
                    {
                        /* 解释数据错误*/
                        if( StarNum <= 32 && StarNum  > 0 )
                        {
                            m_sgStarInfo[StarNum - 1].ucStarInfoValid &= (~WORD_BIT_1_GPS);
                        }

                        Result = false;
                        /* 置标志位 */
                        m_bReceriveGPGSV_Flag = false;
                    }
                }
            }
            break;
        case 6:
        case 10:
        case 14:
        case 18:
            {
                /* <6> 卫星方位角(000～359度)；067度 */
                if(pData[Index] != ',')
                {
                    /* 卫星方位角 */
                    StarAzimuth = 0;
                    for(i = Index; i < Index + 3; i++)
                    {
                        if(pData[i] >= '0' && pData[i] <= '9')
                            StarAzimuth = StarAzimuth * 10 + (pData[i] - '0');
                        else
                        {
                            if( StarNum <= 32 && StarNum  > 0 )
                                m_sgStarInfo[StarNum - 1].ucStarInfoValid &= (~WORD_BIT_2_GPS);

                            Result = false;
                            /* 置标志位 */
                            m_bReceriveGPGSV_Flag = false;
                            break;
                        }
                    }

                    if( Result && StarAzimuth >= 0 && StarAzimuth <= 359)
                    {
                        Index += 3;
                        if( StarNum <= 32 && StarNum  > 0 )
                        {
                            m_sgStarInfo[StarNum - 1].uhStarAzimuth = StarAzimuth;
                            m_sgStarInfo[StarNum - 1].ucStarInfoValid |= WORD_BIT_2_GPS;
                        }
                    }
                    else
                    {
                        /* 解释数据错误*/
                        if( StarNum <= 32 && StarNum  > 0 )
                        {
                            m_sgStarInfo[StarNum - 1].ucStarInfoValid &= (~WORD_BIT_2_GPS);
                        }

                        Result = false;
                        /* 置标志位 */
                        m_bReceriveGPGSV_Flag = false;
                    }
                }
            }
            break;
        case 7:
        case 11:
        case 15:
        case 19:
            {
                /* <7> 信号信噪比(00～99dB)；23dB，无表示未接收到讯号 */
                if(pData[Index] != ',')
                {
                    if(pData[Index] == '*')
                    {
                        break;
                    }

                    /* 信号信噪比 */
                    StarSNR = 0;
                    for(i = Index; i < Index + 2; i++)
                    {
                        if(pData[i] >= '0' && pData[i] <= '9')
                            StarSNR = StarSNR * 10 + (pData[i] - '0');
                        else
                        {
                            if( StarNum <= 32 && StarNum  > 0 )
                                m_sgStarInfo[StarNum - 1].ucStarInfoValid &= (~WORD_BIT_3_GPS);

                            Result = false;
                            /* 置标志位 */
                            m_bReceriveGPGSV_Flag = false;
                            break;
                        }
                    }

                    if(Result && StarSNR >= 0 && StarSNR <= 99)
                    {
                        Index += 2;
                        if( StarNum <= 32 && StarNum  > 0 )
                        {
                            m_sgStarInfo[StarNum - 1].ucStarSNR = StarSNR;
                            m_sgStarInfo[StarNum - 1].ucStarInfoValid |= WORD_BIT_3_GPS;
                        }
                    }
                    else
                    {
                        /* 解释数据错误*/
                        if( StarNum <= 32 && StarNum  > 0 )
                        {
                            m_sgStarInfo[StarNum - 1].ucStarInfoValid &= (~WORD_BIT_3_GPS);
                        }

                        Result = false;
                        /* 置标志位 */
                        m_bReceriveGPGSV_Flag = false;
                    }
                }
            }
            break;
        default:
            {
                Result = false;
                /* 置标志位 */
                m_bReceriveGPGSV_Flag = false;
                break;
            }
        }/* switch 结束 */
    }/* while 结束 */

    return Result;
}

/*-----------------------------------------------------------------------------
 函数名：	CGPS::GetGPS_GPGSA

 描述:		获取GPS数据GPGSA格式数据

 详细描述：	获取GPS数据GPGSA格式数据(当前卫星信息)
            本语句表明GPS精度及使用卫星格式，表示DOP（精度衰减因子）以及观测到的卫星编号
 例：
        $GPGSA,A,3,01,03,14,20,,,,,,,,,2.6,2.5,1.0*35
        $GPGSA,A,1,,,,,,,,,,,,,99.9,99.9,99.9*09
        $GPGSA,A,1,,,,,,,,,,,,,,,*1E

 其标准格式为：	$GPGSA,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>,<13>,<14>,<15>,<16>,<17>*hh<CR><LF>

        <1> 模式 ：M = 手动， A = 自动；
        <2> 定位型式 1 = 未定位， 2 = 二维定位， 3 = 三维定位
        <3> 第1信道正在使用的卫星PRN码编号(Pseudo Random Noise,伪随机噪声码),
            01至32(前导位数不足则补0),表示天空使用中的卫星编号
        <4> 第2信道正在使用的卫星PRN码编号
        <5> 第3信道正在使用的卫星PRN码编号
        <6> 第4信道正在使用的卫星PRN码编号
        <7> 第5信道正在使用的卫星PRN码编号
        <8> 第6信道正在使用的卫星PRN码编号
        <9> 第7信道正在使用的卫星PRN码编号
        <10> 第8信道正在使用的卫星PRN码编号
        <11> 第9信道正在使用的卫星PRN码编号
        <12> 第10信道正在使用的卫星PRN码编号
        <13> 第11信道正在使用的卫星PRN码编号
        <14> 第12信道正在使用的卫星PRN码编号
        <15> PDOP位置精度因子（0.5~99.9），(2.6)
        <16> HDOP水平精度因子（0.5~99.9），(2.5)
        <17> VDOP垂直精度因子（0.5~99.9），(1.0)
        <*> 校验和标志，其后面的一个字节即后面的<hh>表示校验和；总和校验数；35
        (CR)(LF)回车，换行。

 参数:	    UCHAR *pData

 返回值：	bool 获取GPS数据GPGSA格式数据成功标志

 注意事项:	无
 -----------------------------------------------------------------------------*/
bool CGPS::GetGPS_GPGSA(unsigned char *pData)
{
    unsigned char c = '\a';
    bool Result = true;
    char Msg[210];
    unsigned char count = 0;
    unsigned char StarNum = 0;
    float GPGSA_PDOP = 0,GPGSA_HDOP = 0,GPGSA_VDOP = 0;
    unsigned int i =0,Index = 0;
    unsigned int GetDataCount = 0;

    /*	首先计算检查和，如果检查和错误，则认为本组数据出错，不予处理'
        检查和的计算方法：'$'和'*'之间所有字符进行异或后的值(不包括'$'和'*');
        结果与hh 比较，注意：hh是十六进制	*/
        if(!Check((unsigned char*)"GPGSA", pData))
        {
                return false;
        }
        else
        {
        /*-------------清除参数--------------*/
        m_uhReceriveGPGSA_Flag  = 0;

        /* 置* 检查和 标志位 */
        m_uhReceriveGPGSA_Flag |= WORD_BIT_15_GPS;
        m_uhReceriveGPGSA_Flag |= WORD_BIT_14_GPS;
        }

    /*-------------依次读取数据，并解释数据--------------*/
    while(c != '\n' && Result && pData[Index] != '*' && pData[Index] != '\0')
    {
        c = pData[Index++];
        if(c == ',')
        {
            GetDataCount ++;
        }

        /* 根据“,”的个数来执行相应字段的解释工作 */
        switch(GetDataCount)
        {
        case 1:
            {
                /* <1> 模式 ：0/A = 自动,1/M = 手动 */
                if(pData[Index] != ',')
                {
                    /* 模式*/
                    if(pData[Index] == 'A')
                        m_ucGPGSA_WorkModel = 0;
                    else if (pData[Index] == 'M')
                        m_ucGPGSA_WorkModel = 1;
                    else
                    {
                        Result = false;
                        /* 置<1> 模式标志位 */
                        m_uhReceriveGPGSA_Flag &= (~WORD_BIT_0_GPS);
                    }

                    if(Result)
                    {
                        Index++;
                        /* 置<1> 模式标志位 */
                        m_uhReceriveGPGSA_Flag |= WORD_BIT_0_GPS;
                    }
                }
                else
                {
                    Result = false;
                    /* 置<1> 模式标志位 */
                    m_uhReceriveGPGSA_Flag &= (~WORD_BIT_0_GPS);
                }
            }
            break;
        case 2:
            {
                /* <2> 定位型式 1 = 未定位， 2 = 二维定位， 3 = 三维定位 */
                if(pData[Index] != ',')
                {
                    /* 模式*/
                    if(pData[Index] == '1' || pData[Index] == '2' || pData[Index] == '3')
                    {
                        m_ucGPGSA_PositionModel = pData[Index] - '0';
                        /* 置<2> 定位型式标志位 */
                        m_uhReceriveGPGSA_Flag |= WORD_BIT_1_GPS;

                        Index++;
                    }
                    else
                    {
                        Result = false;
                        /* 置<2> 定位型式标志位 */
                        m_uhReceriveGPGSA_Flag &= (~WORD_BIT_1_GPS);
                    }
                }
                else
                {
                    Result = false;
                    /* 置<2> 定位型式标志位 */
                    m_uhReceriveGPGSA_Flag &= (~WORD_BIT_1_GPS);
                }
            }
            break;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
            {
                /* <3>	第1信道正在使用的卫星PRN码编号(Pseudo Random Noise,伪随机噪声码),
                        01至32(前导位数不足则补0),表示天空使用中的卫星编号 */
                StarNum = 0;
                if(pData[Index] != ',')
                {
                    for(i = Index; i < Index + 2; i++)
                    {

                        if(pData[i] >= '0' && pData[i] <= '9' )
                            StarNum = StarNum * 10 + (pData[i] - '0');
                        else
                        {
                            m_ucgGPGSA_StarNum[GetDataCount - 3] = 0;
                            Result = false;
                            break;
                        }
                    }
                    if(Result)
                    {
                        m_ucgGPGSA_StarNum[GetDataCount - 3] = StarNum;
                        Index += 2;
                    }
                }
                else
                {
                    m_ucgGPGSA_StarNum[GetDataCount - 3] = 0;
                }
            }
            break;
        case 15:
            {
                /* <15> PDOP位置精度因子(0.5~99.9),(2.6) */
                if(pData[Index] != ',')
                {
                    *Msg = pData[Index++];
                    count  = 1;
                    while(	pData[Index] != ','
                        && 	(	(pData[Index] >= '0' &&  pData[Index] <= '9')
                             || (pData[Index] == '.' )))
                    {
                        *(Msg + count) = pData[Index++];
                        count ++;
                    }

                    *(Msg + count) = '\0';
                    GPGSA_PDOP = (float)atof(Msg);

                    if(pData[Index] != ',' || GPGSA_PDOP < 0.0f || GPGSA_PDOP > 99.9f )
                    {
                        /* 置<15> PDOP位置精度因子标志位 */
                        m_uhReceriveGPGSA_Flag &= (~WORD_BIT_11_GPS);
                        Result = false;
                    }
                    else
                    {
                        /* 置<15> PDOP位置精度因子标志位 */
                        m_uhReceriveGPGSA_Flag |= WORD_BIT_11_GPS;
                        m_fGPGSA_PDOP = GPGSA_PDOP;
                    }
                }
                else
                {
                    /* 置<15> PDOP位置精度因子标志位 */
                    m_uhReceriveGPGSA_Flag &= (~WORD_BIT_11_GPS);
                }
            }
            break;
        case 16:
            {
                /* 	<16> HDOP水平精度因子(0.5~99.9),(2.5) */
                if(pData[Index] != ',')
                {
                    *Msg = pData[Index++];
                    count  = 1;
                    while(	pData[Index] != ','
                        && 	(	(pData[Index] >= '0' &&  pData[Index] <= '9')
                             || (pData[Index] == '.' )))
                    {
                        *(Msg + count) = pData[Index++];
                        count ++;
                    }

                    *(Msg + count) = '\0';
                    GPGSA_HDOP = (float)atof(Msg);

                    if(pData[Index] != ',' || GPGSA_HDOP < 0.0f || GPGSA_HDOP > 99.9f )
                    {
                        /* 置<16> HDOP水平精度因子标志位 */
                        m_uhReceriveGPGSA_Flag &= (~WORD_BIT_12_GPS);
                        Result = false;
                    }
                    else
                    {
                        /* 置<16> HDOP水平精度因子标志位 */
                        m_uhReceriveGPGSA_Flag |= WORD_BIT_12_GPS;
                        m_fGPGSA_HDOP = GPGSA_HDOP;
                    }
                }
                else
                {
                    /* 置<16> HDOP水平精度因子标志位 */
                    m_uhReceriveGPGSA_Flag &= (~WORD_BIT_12_GPS);
                }
            }
            break;
        case 17:
            {
                /*	<17> VDOP垂直精度因子(0.5~99.9),(1.0) */
                if(pData[Index] != ',' && pData[Index] != '*')
                {
                    *Msg = pData[Index++];
                    count  = 1;
                    while(	pData[Index] != '*'
                        && 	(	(pData[Index] >= '0' &&  pData[Index] <= '9')
                        || (pData[Index] == '.' )))
                    {
                        *(Msg + count) = pData[Index++];
                        count ++;
                    }

                    *(Msg + count) = '\0';
                    GPGSA_VDOP = (float)atof(Msg);

                    if(pData[Index] != '*' || GPGSA_VDOP < 0.0f || GPGSA_VDOP > 99.9f )
                    {
                        /* 置<17> VDOP垂直精度因子标志位 */
                        m_uhReceriveGPGSA_Flag &= (~WORD_BIT_13_GPS);
                        Result = false;
                    }
                    else
                    {
                        /* 置<17> VDOP垂直精度因子标志位 */
                        m_uhReceriveGPGSA_Flag |= WORD_BIT_13_GPS;
                        m_fGPGSA_VDOP = GPGSA_VDOP;
                    }
                }
                else
                {
                    /* 置<17> VDOP垂直精度因子标志位 */
                    m_uhReceriveGPGSA_Flag &= (~WORD_BIT_13_GPS);
                }
            }
            break;
        default:
            {
                Result = false;
                break;
            }
        }/* switch 结束 */
    }/* while 结束 */

    return Result;
}

/*-----------------------------------------------------------------------------
 函数名：	CGPS::GetGPS_GPGLL

 描述:		获取GPS数据GPGLL格式数据

 详细描述：	获取GPS数据GPGLL格式数据，本语句表明GPS地理定位信息和时间信息。

 例：
        $GPGLL,4738.0173,N,12211.1874,W,191934.767,A*21
        $GPGLL,3537.8333,N,13944.6667,E,062320,V*3B
        $GPGLL,3537.8333,N,13944.6667,E,062322,V*39

 其标准格式为：	 $GPGLL,<1>,<2>,<3>,<4>,<5>,<6>*hh<CR><LF>

        <1> 纬度，格式为ddmm.mmmm (度度分分.分分分分，不足位数时前面补0)；
        <2> 纬度属性，南北半球，N/S；南纬42度50.5589分；
        <3> 经度，格式为dddmm.mmmm(度度度分分.分分分分，不足位数时前面补0)；
        <4> 经度属性，东西半球，E/W；东经147度18.5048分；
        <5> UTC当前时间，格式为hhmmss(时时分分秒秒)；09时22分04秒
        <6> 接收机定位标志A=定位，V=未定位
        <*> 校验和标志，其后面的一个字节即后面的<hh>表示校验和；总和校验数:2D
        (CR)(LF)回车，换行。


 参数:	    UCHAR *pData

 返回值：	bool 获取GPS数据GPGLL格式数据成功标志

 注意事项:	无
 -----------------------------------------------------------------------------*/
bool CGPS::GetGPS_GPGLL(unsigned char *pData)
{
    unsigned char c = '\a';
    bool Result = true;

    /*	成功取得经纬度数据和经纬度属性标志 */
    bool bGetLatData  = false,bGetLongData  = false;
    bool bGetLatAttr  = false,bGetLongAttr  = false;

    char Msg[210];
    unsigned char  Hour = 0,Minute = 0,Second = 0;
    unsigned char count = 0;
    unsigned short l_Degree =0,l_Minute =0;
    unsigned int i =0,Index = 0,l_Second =0;
    unsigned char GetDataCount = 0;

    /*	首先计算检查和，如果检查和错误，则认为本组数据出错，不予处理'
        检查和的计算方法：'$'和'*'之间所有字符进行异或后的值（不包括'$'和'*'）;
        结果与hh 比较，注意：hh是十六进制	*/
        if(!Check((unsigned char*)"GPGLL", pData))
        {
                return false;
        }
        else
        {
        /*-------------清除参数--------------*/
        /*  可以使用经纬度数值标志 */
        m_bEnableUseLongLat_Flag = false;

        /*  可以使用高度数值标志 */
        m_bEnableUseHeight_Flag	= false;

        /*  可以使用日时间标志*/
        m_bEnableUseTime_Flag	= false;

        /*  时间信息秒是否带有小数点 */
        m_bSecondWithDot = false;

        /*  收到GPGLL报文标志 */
        m_uhReceriveGPGLL_Flag = 0;

        /* 置 检查和 * 标志位 */
        m_uhReceriveGPGLL_Flag |= WORD_BIT_15_GPS;
        m_uhReceriveGPGLL_Flag |= WORD_BIT_14_GPS;
        }

    /*-------------依次读取数据，并解释数据--------------*/
    while(c != '\n' && Result && pData[Index] != '*' && pData[Index] != '\0')
    {
        c = pData[Index++];
        if(c == ',')
        {
            GetDataCount ++;
        }

        /* 根据“,”的个数来执行相应字段的解释工作 */
        switch(GetDataCount)
        {
        case 1:
            {
                /* <1> 纬度，格式为ddmm.mmmm (度度分分.分分分分，不足位数时前面补0) */
                /* 有纬度信息 */
                if(pData[Index] != ',')
                {
                    l_Degree = 0;
                    /* 度度 dd */
                    for(i = Index; i < Index + 2; i++)
                    {
                        if(pData[i] >= '0' && pData[i] <= '9')
                            l_Degree = l_Degree * 10 + (pData[i] - '0');
                        else
                        {
                            Result = false;
                            /* 置<1> 纬度标志位 */
                            m_uhReceriveGPGLL_Flag &= (~WORD_BIT_0_GPS);
                            break;
                        }
                    }
                    if(Result && l_Degree <= 90)
                    {
                        Index += 2;
                        m_sLatitude.hDegree = l_Degree;
                        l_Minute = 0;
                        /* 分分 mm */
                        for(i = Index; i < Index + 2; i++)
                        {
                            if(pData[i] >= '0' && pData[i] <= '9')
                                l_Minute = l_Minute * 10 + (pData[i] - '0');
                            else
                            {
                                Result = false;
                                /* 置<1> 纬度标志位 */
                                m_uhReceriveGPGLL_Flag &= (~WORD_BIT_0_GPS);
                                break;
                            }
                        }

                        if(Result && l_Minute < 60)
                        {
                            Index += 2;
                            m_sLatitude.hMinute = l_Minute;

                            /*.分分分分 .mmmm  转化成秒*/
                            if(pData[Index] == '.')
                            {
                                Index += 1;

                                /* 因为发现有带4位小数的，有带2位小数的，因此需要使用while 结构 */
                                *Msg = '0';
                                *(Msg + 1) = '.';
                                count = 2;

                                while(	pData[Index] != ','
                                     && pData[Index] >= '0'
                                     && pData[Index] <= '9')
                                {
                                    *(Msg + count) = pData[Index++];
                                    count ++;
                                }
                                *(Msg + count) = '\0';

                                if(pData[Index] == ',')
                                {
                                    m_sLatitude.dSecond = atof(Msg) * 60.0;

                                    /* 置<1> 纬度标志位 */
                                    m_uhReceriveGPGLL_Flag |= WORD_BIT_0_GPS;

                                    /* 成功取得纬度数据 */
                                    bGetLatData = true;
                                }
                                else
                                {
                                    Result = false;
                                    /* 置<1> 纬度标志位 */
                                    m_uhReceriveGPGLL_Flag &= (~WORD_BIT_0_GPS);
                                }
                            }
                            else
                            {
                                Result = false;
                                /* 置<1> 纬度标志位 */
                                m_uhReceriveGPGLL_Flag &= (~WORD_BIT_0_GPS);
                            }
                        }
                        else
                        {
                            Result = false;
                            /* 置<1> 纬度标志位 */
                            m_uhReceriveGPGLL_Flag &= (~WORD_BIT_0_GPS);
                        }
                    }
                    else
                    {
                        Result = false;
                        /* 置<1> 纬度标志位 */
                        m_uhReceriveGPGLL_Flag &= (~WORD_BIT_0_GPS);
                    }
                }
                else
                {
                    Result = false;
                    /* 置<1> 纬度标志位 */
                    m_uhReceriveGPGLL_Flag &= (~WORD_BIT_0_GPS);
                }
            }
            break;
        case 2:
            {
                /* <2> 纬度属性，南北半球，N/S */
                if(pData[Index] != ',')
                {
                    /* 纬度属性，南北半球，N 0 / S 1 */
                    if(pData[Index] == 'N')
                        m_ucLatitudeAttribute = 0;
                    else if (pData[Index] == 'S')
                        m_ucLatitudeAttribute = 1;
                    else
                    {
                        Result = false;
                        /* 置<2> 纬度属性标志位 */
                        m_uhReceriveGPGLL_Flag &= (~WORD_BIT_1_GPS);
                    }

                    if(Result)
                    {
                        Index++;
                        /* 置<2> 纬度属性标志位 */
                        m_uhReceriveGPGLL_Flag |= WORD_BIT_1_GPS;

                        /* 成功取得经纬度属性标志 */
                        bGetLatAttr  = true;
                    }
                }
                else
                {
                    Result = false;
                    /* 置<2> 纬度属性标志位 */
                    m_uhReceriveGPGLL_Flag &= (~WORD_BIT_1_GPS);
                }
            }
            break;
        case 3:
            {
                /* <3> 经度,格式为dddmm.mmmm(度度度分分.分分分分,不足位数时前面补0) */
                if(pData[Index] != ',')
                {
                    /* 度度 dd */
                    l_Degree = 0;
                    for(i = Index; i < Index + 3; i++)
                    {
                        if(pData[i] >= '0' && pData[i] <= '9')
                            l_Degree = l_Degree * 10 + (pData[i] - '0');
                        else
                        {
                            Result = false;
                            /* 置<3> 经度标志位 */
                            m_uhReceriveGPGLL_Flag &= (~WORD_BIT_2_GPS);
                            break;
                        }
                    }
                    if(Result && l_Degree <= 180)
                    {
                        Index += 3;
                        m_sLongitude.hDegree = l_Degree;

                        /* 分分 mm */
                        l_Minute = 0;
                        for(i = Index; i < Index + 2; i++)
                        {
                            if(pData[i] >= '0' && pData[i] <= '9')
                                l_Minute = l_Minute * 10 + (pData[i] - '0');
                            else
                            {
                                Result = false;
                                /* 置<3> 经度标志位 */
                                m_uhReceriveGPGLL_Flag &= (~WORD_BIT_2_GPS);
                                break;
                            }
                        }

                        if(Result && l_Minute < 60)
                        {
                            Index += 2;
                            m_sLongitude.hMinute = l_Minute;

                            l_Second = 0;

                            /*.分分分分 .mmmm  转化成秒*/
                            if(pData[Index] == '.')
                            {
                                Index += 1;

                                /* 因为发现有带4位小数的，有带2位小数的，因此需要使用while 结构 */
                                *Msg = '0';
                                *(Msg + 1) = '.';
                                count = 2;

                                while(	pData[Index] != ','
                                     && pData[Index] >= '0'
                                     && pData[Index] <= '9')
                                {
                                    *(Msg + count) = pData[Index++];
                                    count ++;
                                }
                                *(Msg + count) = '\0';

                                if(pData[Index] == ',')
                                {
                                    m_sLongitude.dSecond = atof(Msg) * 60.0;

                                    /* 置<3> 经度标志位 */
                                    m_uhReceriveGPGLL_Flag |= WORD_BIT_2_GPS;

                                    /* 成功取得经度数据 */
                                    bGetLongData  = true;
                                }
                                else
                                {
                                    Result = false;
                                    /* 置<3> 经度标志位 */
                                    m_uhReceriveGPGLL_Flag &= (~WORD_BIT_2_GPS);
                                }
                            }
                            else
                            {
                                Result = false;
                                /* 置<3> 经度标志位 */
                                m_uhReceriveGPGLL_Flag &= (~WORD_BIT_2_GPS);
                            }
                        }
                        else
                        {
                            Result = false;
                            /* 置<3> 经度标志位 */
                            m_uhReceriveGPGLL_Flag &= (~WORD_BIT_2_GPS);
                        }
                    }
                    else
                    {
                        Result = false;
                        /* 置<3> 经度标志位 */
                        m_uhReceriveGPGLL_Flag &= (~WORD_BIT_2_GPS);
                    }
                }
                else
                {
                    Result = false;
                    /* 置<3> 经度标志位 */
                    m_uhReceriveGPGLL_Flag &= (~WORD_BIT_2_GPS);
                }
            }
            break;
        case 4:
            {
                /* <4> 经度属性，东西半球，E/W */
                if(pData[Index] != ',')
                {
                    /* 经度属性，东西半球，E 0 / W 1  */
                    if(pData[Index] == 'E')
                        m_ucLongitudeAttribute = 0;
                    else if (pData[Index] == 'W')
                        m_ucLongitudeAttribute = 1;
                    else
                    {
                        Result = false;
                        /* 置<4> 经度属性标志位 */
                        m_uhReceriveGPGLL_Flag &= (~WORD_BIT_3_GPS);
                    }
                    if(Result)
                    {
                        Index ++;
                        /* 置<4> 经度属性标志位 */
                        m_uhReceriveGPGLL_Flag |= WORD_BIT_3_GPS;

                        /* 成功取得经度属性标志 */
                        bGetLongAttr  = true;
                    }
                }
                else
                {
                    Result = false;
                    /* 置<4> 经度属性标志位 */
                    m_uhReceriveGPGLL_Flag &= (~WORD_BIT_3_GPS);
                }
            }
            break;
        case 5:
            {
                /* ------------------------------------------------
                    <5>	UTC当前时间，格式为hhmmss(时时分分秒秒)；
                    处理老式GPS，UniStar II型GPS的时间后面带小数点，
                    II A+型不带小数点,本模块兼容这两种结构。
                  ------------------------------------------------*/
                /* 有时间信息 */
                if(pData[Index] != ',')
                {
                    /* 时时 hh */
                    Hour = 0;
                    for(i = Index; i < Index + 2; i++)
                    {
                        if(pData[i] >= '0' && pData[i] <= '9')
                            Hour = Hour * 10 + (pData[i] - '0');
                        else
                        {
                            Result = false;
                            /* 置<5>UTC当前时间标志位 */
                            m_uhReceriveGPGLL_Flag &= (~WORD_BIT_4_GPS);
                            break;
                        }
                    }
                    /* 分分 mm */
                    if(Result && Hour < 24)
                    {
                        /* 保存“时” */
                        m_ucHour = Hour;

                        Minute = 0;
                        Index += 2;
                        for(i = Index; i < Index + 2; i++)
                        {
                            if(pData[i] >= '0' && pData[i] <= '9')
                                Minute = Minute * 10 + (pData[i] - '0');
                            else
                            {
                                Result = false;
                                /* 置<5>UTC当前时间标志位 */
                                m_uhReceriveGPGLL_Flag &= (~WORD_BIT_4_GPS);
                                break;
                            }
                        }

                        /* 秒秒 ss */
                        if(Result && Minute < 60)
                        {
                            /* 保存“分” */
                            m_ucMinute = Minute;
                            Second = 0;
                            Index += 2;
                            for(i = Index; i < Index + 2; i++)
                            {
                                if(pData[i] >= '0' && pData[i] <= '9')
                                    Second = Second * 10 + (pData[i] - '0');
                                else
                                {
                                    Result = false;
                                    /* 置<5>UTC当前时间标志位 */
                                    m_uhReceriveGPGLL_Flag &= (~WORD_BIT_4_GPS);
                                    break;
                                }
                            }
                            if(Result && Second < 60)
                            {
                                Index += 2;
                                m_ucSecond = Second;

                                /* 处理老式GPS，UniStar II型GPS的时间后面带小数点*/
                                if(pData[Index] == '.')
                                {
                                    Index += 1;

                                    /*	秒的小数点后数据位个数不定(现已发现有1位和3位的情况)，
                                        需使用while 循环结构，取出后直接保存为毫秒*/
                                    count = 0;
                                    if(pData[Index] != ',')
                                    {
                                        *Msg = '0';
                                        *(Msg +1) = '.';
                                        count += 2;
                                        while(pData[Index] != ','
                                            && 	(	(pData[Index] >= '0' &&  pData[Index] <= '9')
                                                 || (pData[Index] == '.' )))
                                        {
                                            *(Msg + count) = pData[Index++];
                                            count++;
                                        }
                                        *(Msg + count) = '\0';

                                        if(pData[Index] == ',')
                                        {
                                            m_uhMillisecond = (unsigned short)(atof(Msg) * 1000);

                                            /*  时间信息秒是否带有小数点 */
                                            m_bSecondWithDot = true;
                                        }
                                        else
                                        {
                                            Result = false;
                                            /* 置<5>UTC当前时间标志位 */
                                            m_uhReceriveGPGLL_Flag &= (~WORD_BIT_4_GPS);
                                        }
                                    }
                                    else
                                    {
                                        /* 保存为毫秒 */
                                        m_uhMillisecond = 0;
                                        /* 时间信息秒是否带有小数点 */
                                        m_bSecondWithDot = false;
                                    }
                                }
                                else
                                {
                                    /* 保存为毫秒 */
                                    m_uhMillisecond = 0;
                                    /* 时间信息秒是否带有小数点 */
                                    m_bSecondWithDot = false;
                                }

                                /* 置<5>UTC当前时间标志位 */
                                m_uhReceriveGPGLL_Flag |= WORD_BIT_4_GPS;

                                /* 注：本语句没有定位卫星个数，故不判断
                                if(m_ucStarNum >= m_ucEnableUseLongLat_StarNum)
                                */
                                {
                                    /* 可以使用日时间标志 */
                                    m_bEnableUseTime_Flag	= true;
                                }
                            }
                            else
                            {
                                Result = false;
                                /* 置<5>UTC当前时间标志位 */
                                m_uhReceriveGPGLL_Flag &= (~WORD_BIT_4_GPS);
                            }
                        }
                        else
                        {
                            Result = false;
                            /* 置<5>UTC当前时间标志位 */
                            m_uhReceriveGPGLL_Flag &= (~WORD_BIT_4_GPS);
                        }
                    }
                    else
                    {
                        Result = false;
                        /* 置<5>UTC当前时间标志位 */
                        m_uhReceriveGPGLL_Flag &= (~WORD_BIT_4_GPS);
                    }
                }
                else
                {
                    Result = false;
                    /* 置<5>UTC当前时间标志位 */
                    m_uhReceriveGPGLL_Flag &= (~WORD_BIT_4_GPS);
                }
            }
            break;
        case 6:
            {
                /* <6> 状态字，A表示定位成功，V表示目前没有定位 */
                if(pData[Index] != ',')
                {
                    /* 状态字，A表示定位成功，V表示目前没有定位 */
                    if(pData[Index] == 'A')
                        m_ucGP_LocationSucessFlag = 1;
                    else if (pData[Index] == 'V')
                        m_ucGP_LocationSucessFlag = 0;
                    else
                    {
                        Result = false;
                        /* 置<6> 状态字 */
                        m_uhReceriveGPGLL_Flag &= (~WORD_BIT_5_GPS);
                    }
                    if(Result)
                    {
                        Index++;
                        /* 置<6> 状态字标志位 */
                        m_uhReceriveGPGLL_Flag |= WORD_BIT_5_GPS;

                        if(bGetLatData && bGetLongData && bGetLatAttr && bGetLongAttr)
                            /*  可以使用经纬度数值标志 */
                            m_bEnableUseLongLat_Flag = true;
                    }
                }
                else
                {
                    Result = false;
                    /* 置<6> 状态字 */
                    m_uhReceriveGPGLL_Flag &= (~WORD_BIT_5_GPS);
                }
            }
            break;

                case 7:
                        {
                                /* Mode indicator */
                                if(pData[Index] != ',')
                {
                    /* 状态字，N:Data not valid */
                    if(pData[Index] == 'N')
                                        {
                                                m_bEnableUseTime_Flag = 0;
                                                m_bEnableUseLongLat_Flag = 0;
                                        }
                                }
                        }
                        break;

        default:
            {
                Result = false;
                break;
            }
        }/* switch 结束 */
    }/* while 结束 */

    return Result;
}

/*-----------------------------------------------------------------------------
 函数名：	CGPS::GetGPS_GPVTG

 描述:		获取GPS数据GPVTG格式数据

 详细描述：	获取GPS数据GPVTG格式数据(地面速度信息)

  例：	$GPVTG,89.68,T,,M,000.0,N,000.0,K*6F
        $GPVTG,000.0,T,,M,000.0,N,000.0,K*60
        $GPVTG,318.37,T,,M,2.87,N,5.3,K*65
        $GPVTG,359.9,T,,M,015.15,N,028.0,K,A*31

 其标准格式为：	 $GPVTG,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>*hh<CR><LF>

        <1> 运动角度(真北),000.0～359.9度,(不足位数时前面补0)
        <2> T=真北参照系
        <3> 运动角度(磁北),000～359度,(不足位数时前面补0)
        <4> M=磁北参照系
        <5> 水平运动速度(000.0～999.9节),(不足位数时前面补0)
        <6> N=节，Knots
        <7> 水平运动速度(0000.0～1851.8公里/小时),(不足位数时前面补0)
        <8> K=公里/时,km/h
        <9> 模式指示(仅NMEA0183 3.00版本输出，A=自主定位，D=差分，E=估算，N=数据无效)
        <*> 校验和标志，其后面的一个字节即后面的<hh>表示校验和；总和校验数:65
        (CR)(LF)回车，换行。


 参数:	    UCHAR *pData

 返回值：	bool 获取GPS数据GPVTG格式数据成功标志

 注意事项:	无
 -----------------------------------------------------------------------------*/
bool CGPS::GetGPS_GPVTG(unsigned char *pData)
{
    unsigned char c = '\a';
    bool Result = true;
    char Msg[210];
    unsigned char count = 0;
    unsigned int i =0,Index = 0;
    unsigned int GetDataCount = 0;
    float GPVTG_TrueNorth_Azimuth = 0.0f;
    float GPVTG_Velocity_knots = 0.0f;
    float GPVTG_Velocity_km = 0.0f;
    float GPVTG_MagneticNorth_Azimuth = 0.0f;

    /*	首先计算检查和，如果检查和错误，则认为本组数据出错，不予处理'
        检查和的计算方法：'$'和'*'之间所有字符进行异或后的值（不包括'$'和'*'）;
        结果与hh 比较，注意：hh是十六进制	*/
        if(!Check((unsigned char*)"GPVTG", pData))
        {
                return false;
        }
        else
        {
        /*-------------清除参数--------------*/
        /*  收到GPGLL报文标志 */
        m_uhReceriveGPVTG_Flag = 0;

        /* 置 检查和 * 标志位 */
        m_uhReceriveGPVTG_Flag |= WORD_BIT_15_GPS;
        m_uhReceriveGPVTG_Flag |= WORD_BIT_14_GPS;
        }

    /*-------------依次读取数据，并解释数据--------------*/
    while(c != '\n' && Result && pData[Index] != '*' && pData[Index] != '\0')
    {
        c = pData[Index++];
        if(c == ',')
        {
            GetDataCount ++;
        }

        /* 根据“,”的个数来执行相应字段的解释工作 */
        switch(GetDataCount)
        {
        case 1:
            {
                /* <1> 运动角度(真北),000.0～359.9度,(不足位数时前面补0) */
                if(pData[Index] != ',')
                {
                    *Msg = pData[Index++];
                    count = 1;
                    while(pData[Index] != ','
                        && 	(	(pData[Index] >= '0' &&  pData[Index] <= '9')
                        || (pData[Index] == '.' )))
                    {
                        *(Msg + count) = pData[Index++];
                        count++;
                    }
                    *(Msg + count) = '\0';

                    GPVTG_TrueNorth_Azimuth = (float)atof(Msg);

                    if(		pData[Index] != ','
                        ||	GPVTG_TrueNorth_Azimuth < 0.0f
                        ||	GPVTG_TrueNorth_Azimuth > 360.0f )
                    {
                        /* 置<1> 运动角度(真北)标志位 */
                        m_uhReceriveGPVTG_Flag &= (~WORD_BIT_0_GPS);
                        Result = false;
                    }
                    else
                    {
                        /* 置<1> 运动角度(真北)标志位 */
                        m_uhReceriveGPVTG_Flag |= WORD_BIT_0_GPS;
                        m_fGPVTG_TrueNorth_Azimuth = GPVTG_TrueNorth_Azimuth;
                    }
                }
                else
                {
                    /* 置<1> 运动角度(真北)标志位 */
                    m_uhReceriveGPVTG_Flag &= (~WORD_BIT_0_GPS);
                }
            }
            break;
        case 2:
            {
                /* <2> T=真北参照系 */
                if(pData[Index] != ',')
                {
                    /* T */
                    if(pData[Index] == 'T')
                    {
                        /* 置<2>T=真北参照系标志位 */
                        m_uhReceriveGPVTG_Flag |= WORD_BIT_1_GPS;

                        Index ++;
                    }
                    else
                    {
                        Result = false;
                        /* 置<2>T=真北参照系标志位 */
                        m_uhReceriveGPVTG_Flag &= (~WORD_BIT_1_GPS);
                    }
                }
                else
                {
                    /* 置<2>T=真北参照系标志位 */
                    m_uhReceriveGPVTG_Flag &= (~WORD_BIT_1_GPS);
                }
            }
            break;
        case 3:
            {
                /* <3> 运动角度(磁北),000～359度,(不足位数时前面补0) */
                if(pData[Index] != ',')
                {
                    *Msg = pData[Index++];
                    count = 1;
                    while(pData[Index] != ','
                        && 	(	(pData[Index] >= '0' &&  pData[Index] <= '9')
                        || (pData[Index] == '.' )))
                    {
                        *(Msg + count) = pData[Index++];
                        count++;
                    }
                    *(Msg + count) = '\0';

                    GPVTG_MagneticNorth_Azimuth = (float)atof(Msg);

                    if(		pData[Index] != ','
                        ||	GPVTG_MagneticNorth_Azimuth < 0.0f
                        ||	GPVTG_MagneticNorth_Azimuth > 360.0f )
                    {
                        /* 置<3> 运动角度(磁北)标志位 */
                        m_uhReceriveGPVTG_Flag &= (~WORD_BIT_2_GPS);
                        Result = false;
                    }
                    else
                    {
                        /* 置<3> 运动角度(磁北)标志位 */
                        m_uhReceriveGPVTG_Flag |= WORD_BIT_2_GPS;
                        m_fGPVTG_MagneticNorth_Azimuth = GPVTG_MagneticNorth_Azimuth;
                    }
                }
                else
                {
                    /* 置<3> 运动角度(磁北)标志位 */
                    m_uhReceriveGPVTG_Flag &= (~WORD_BIT_2_GPS);
                }
            }
            break;
        case 4:
            {
                /* <4> M=磁北参照系 */
                if(pData[Index] != ',')
                {
                    /* M */
                    if(pData[Index] == 'M')
                    {
                        /* 置<4>M=磁北参照系标志位 */
                        m_uhReceriveGPVTG_Flag |= WORD_BIT_3_GPS;

                        Index ++;
                    }
                    else
                    {
                        Result = false;
                        /* 置<4>M=磁北参照系标志位 */
                        m_uhReceriveGPVTG_Flag &= (~WORD_BIT_3_GPS);
                    }
                }
                else
                {
                    /* 置<4>M=磁北参照系标志位 */
                    m_uhReceriveGPVTG_Flag &= (~WORD_BIT_3_GPS);
                }
            }
            break;
        case 5:
            {
                /* <5> 水平运动速度(000.0～999.9节),(不足位数时前面补0) */
                if(pData[Index] != ',')
                {
                    *Msg = pData[Index++];
                    count = 1;
                    while(pData[Index] != ','
                        && 	(	(pData[Index] >= '0' &&  pData[Index] <= '9')
                        || (pData[Index] == '.' )))
                    {
                        *(Msg + count) = pData[Index++];
                        count++;
                    }
                    *(Msg + count) = '\0';

                    GPVTG_Velocity_knots = (float)atof(Msg);

                    if(		pData[Index] != ','
                        ||	GPVTG_Velocity_knots < 0.0f
                        ||	GPVTG_Velocity_knots > 999.9f )
                    {
                        /* 置<5> 水平运动速度(N)标志位 */
                        m_uhReceriveGPVTG_Flag &= (~WORD_BIT_4_GPS);
                        Result = false;
                    }
                    else
                    {
                        /* 置<5> 水平运动速度(N)标志位 */
                        m_uhReceriveGPVTG_Flag |= WORD_BIT_4_GPS;
                        m_fGPVTG_Velocity_knots = GPVTG_Velocity_knots;
                    }
                }
                else
                {
                    /* 置<5> 水平运动速度(N)标志位 */
                    m_uhReceriveGPVTG_Flag &= (~WORD_BIT_4_GPS);
                }
            }
            break;
        case 6:
            {
                /* <6> N=节 */
                if(pData[Index] != ',')
                {
                    /* N */
                    if(pData[Index] == 'N')
                    {
                        /* 置<6> N=节标志位 */
                        m_uhReceriveGPVTG_Flag |= WORD_BIT_5_GPS;

                        Index ++;
                    }
                    else
                    {
                        Result = false;
                        /* 置<6> N=节标志位 */
                        m_uhReceriveGPVTG_Flag &= (~WORD_BIT_5_GPS);
                    }
                }
                else
                {
                    /* 置<6> N=节标志位 */
                    m_uhReceriveGPVTG_Flag &= (~WORD_BIT_5_GPS);
                }
            }
            break;
        case 7:
            {
                /* <7> 水平运动速度(0000.0～1851.8公里/小时),(不足位数时前面补0) */
                if(pData[Index] != ',')
                {
                    *Msg = pData[Index++];
                    count = 1;
                    while(pData[Index] != ','
                        && 	(	(pData[Index] >= '0' &&  pData[Index] <= '9')
                        || (pData[Index] == '.' )))
                    {
                        *(Msg + count) = pData[Index++];
                        count++;
                    }
                    *(Msg + count) = '\0';

                    GPVTG_Velocity_km = (float)atof(Msg);

                    if(		pData[Index] != ','
                        ||	GPVTG_Velocity_km < 0.0f
                        ||	GPVTG_Velocity_km > 999.9f )
                    {
                        /* 置<7> 水平运动速度(K)标志位 */
                        m_uhReceriveGPVTG_Flag &= (~WORD_BIT_6_GPS);
                        Result = false;
                    }
                    else
                    {
                        /* 置<7> 水平运动速度(K)标志位 */
                        m_uhReceriveGPVTG_Flag |= WORD_BIT_6_GPS;
                        m_fGPVTG_Velocity_km = GPVTG_Velocity_km;
                    }
                }
                else
                {
                    /* 置<7> 水平运动速度(K)标志位 */
                    m_uhReceriveGPVTG_Flag &= (~WORD_BIT_6_GPS);
                }
            }
            break;
        case 8:
            {
                /* <8>  K=公里/时,km/h */
                if(pData[Index] != ',')
                {
                    /* K */
                    if(pData[Index] == 'K')
                    {
                        /* 置<8> K=公里/时 标志位 */
                        m_uhReceriveGPVTG_Flag |= WORD_BIT_7_GPS;

                        Index ++;
                    }
                    else
                    {
                        Result = false;
                        /* 置<8> K=公里/时标志位 */
                        m_uhReceriveGPVTG_Flag &= (~WORD_BIT_7_GPS);
                    }
                }
                else
                {
                    /* 置<8> K=公里/时标志位 */
                    m_uhReceriveGPVTG_Flag &= (~WORD_BIT_7_GPS);
                }
            }
            break;
        case 9:
            {
                /* <9> 模式指示(仅NMEA0183 3.00版本输出，0/N=数据无效,1/A=自主定位,2/D=差分,3/E=估算) */
                if(pData[Index] != '*')
                {
                    /* 模式指示 */
                    if (pData[Index] == 'A')
                        m_ucGPWorkModel = 1;
                    else if(pData[Index] == 'N')
                        m_ucGPWorkModel = 0;
                    else if(pData[Index] == 'D')
                        m_ucGPWorkModel = 2;
                    else if (pData[Index] == 'E')
                        m_ucGPWorkModel = 3;
                    else
                    {
                        Result = false;
                        /* 置<9> 模式指示标志位 */
                        m_uhReceriveGPVTG_Flag &= (~WORD_BIT_8_GPS);
                    }
                    if(Result)
                    {
                        Index ++;
                        /* 置<9> 模式指示标志位 */
                        m_uhReceriveGPVTG_Flag |= WORD_BIT_8_GPS;

                        /*  NMEA0183协议是否是V3.0版本 */
                        m_bVer3 = true;
                    }
                }
            }
            break;
        default:
            {
                Result = false;
                break;
            }
        }/* switch 结束 */
    }/* while 结束 */

    return Result;
}

/*-----------------------------------------------------------------------------
 函数名：	CGPS::GetGPS_GPZDA

 描述:		获取GPS数据GPZDA格式数据

 详细描述：	获取GPS数据GPZDA格式数据,年月日、时分秒以及时区和分钟的偏移量

 例：
        $GPZDA,062321,03,02,2022,,*4F
        $GPZDA,024611.08,25,03,2002,00,00*6A
        $GPZDA,024613.008,13,08,2009,-01,58*6A

 其标准格式为：	$GPZDA,<1>,<2>,<3>,<4>,<5>,<6>*hh<CR><LF>

        <1> UTC时间，格式hhmmss（前导位数不足则补0,可能有.ss）
        <2> UTC日期，01到31（前导位数不足则补0）
        <3> UTC月份，01到12（前导位数不足则补0）
        <4> UTC年，格式yyyy
        <5> 本地时区小时偏移量，00到+/-13（正常为00到+13或00到-12，前导位数不足则补0）
        <6> 本地时区分钟偏移量，00到+/-59（前导位数不足则补0）。
        <*> 校验和标志，其后面的一个字节即后面的<hh>表示校验和；总和校验数:65
        (CR)(LF)回车，换行。

 参数:	    UCHAR *pData

 返回值：	bool 获取GPS数据GPZDA格式数据成功标志

 注意事项:	无
 -----------------------------------------------------------------------------*/
bool CGPS::GetGPS_GPZDA(unsigned char* pData)
{
    unsigned char c = '\a';
    bool Result = true;

    char Msg[210];
    unsigned char  Hour = 0,Minute = 0,Second = 0;
    unsigned char  Month = 0,Day = 0;
    unsigned short  Year = 0;
    unsigned char count = 0;
    char OffsetHour = 0,OffsetMinute = 0;
    unsigned int i =0,Index = 0;
    unsigned char GetDataCount = 0;

    /*	首先计算检查和，如果检查和错误，则认为本组数据出错，不予处理
        检查和的计算方法：'$'和'*'之间所有字符进行异或后的值（不包括'$'和'*'）;
        结果与hh 比较，注意：hh是十六进制	*/
        if(!Check((unsigned char*)"GPZDA", pData))
        {
                return false;
        }
        else
        {
        /*-------------清除参数--------------*/
        /*  收到GPZDA报文标志 */
        m_uhReceriveGPZDA_Flag = 0;

        /* 置 检查和 标志位 */
        m_uhReceriveGPZDA_Flag |= WORD_BIT_15_GPS;
        m_uhReceriveGPZDA_Flag |= WORD_BIT_14_GPS;
        }

    /*-------------依次读取数据，并解释数据--------------*/
    while(c != '\n' && Result && pData[Index] != '*' && pData[Index] != '\0')
    {
        c = pData[Index++];
        if(c == ',')
        {
            GetDataCount ++;
        }

        /* 根据“,”的个数来执行相应字段的解释工作 */
        switch(GetDataCount)
        {
        case 1:
            {
                /* ------------------------------------------------
                    <1>	UTC当前时间，格式为hhmmss(时时分分秒秒)；
                    处理老式GPS，UniStar II型GPS的时间后面带小数点，
                    II A+型不带小数点,本模块兼容这两种结构。
                  ------------------------------------------------*/
                /* 有时间信息 */
                if(pData[Index] != ',')
                {
                    /* 时时 hh */
                    Hour = 0;
                    for(i = Index; i < Index + 2; i++)
                    {
                        if(pData[i] >= '0' && pData[i] <= '9')
                            Hour = Hour * 10 + (pData[i] - '0');
                        else
                        {
                            Result = false;
                            /* 置<1>UTC当前时间标志位 */
                            m_uhReceriveGPZDA_Flag &= (~WORD_BIT_0_GPS);
                            break;
                        }
                    }
                    /* 分分 mm */
                    if(Result && Hour < 24)
                    {
                        /* 保存“时” */
                        m_ucHour = Hour;

                        Minute = 0;
                        Index += 2;
                        for(i = Index; i < Index + 2; i++)
                        {
                            if(pData[i] >= '0' && pData[i] <= '9')
                                Minute = Minute * 10 + (pData[i] - '0');
                            else
                            {
                                Result = false;
                                /* 置<1>UTC当前时间标志位 */
                                m_uhReceriveGPZDA_Flag &= (~WORD_BIT_0_GPS);
                                break;
                            }
                        }

                        /* 秒秒 ss */
                        if(Result && Minute < 60)
                        {
                            /* 保存“分” */
                            m_ucMinute = Minute;
                            Second = 0;
                            Index += 2;
                            for(i = Index; i < Index + 2; i++)
                            {
                                if(pData[i] >= '0' && pData[i] <= '9')
                                    Second = Second * 10 + (pData[i] - '0');
                                else
                                {
                                    Result = false;
                                    /* 置<1>UTC当前时间标志位 */
                                    m_uhReceriveGPZDA_Flag &= (~WORD_BIT_0_GPS);
                                    break;
                                }
                            }
                            if(Result && Second < 60)
                            {
                                Index += 2;
                                m_ucSecond = Second;

                                /* 处理老式GPS，UniStar II型GPS的时间后面带小数点*/
                                if(pData[Index] == '.')
                                {
                                    Index += 1;

                                    /*	秒的小数点后数据位个数不定(现已发现有1位和3位的情况)，
                                        需使用while 循环结构，取出后直接保存为毫秒*/
                                    count = 0;
                                    if(pData[Index] != ',')
                                    {
                                        *Msg = '0';
                                        *(Msg +1) = '.';
                                        count += 2;
                                        while(pData[Index] != ','
                                            && 	(	(pData[Index] >= '0' &&  pData[Index] <= '9')
                                                 || (pData[Index] == '.' )))
                                        {
                                            *(Msg + count) = pData[Index++];
                                            count++;
                                        }
                                        *(Msg + count) = '\0';

                                        if(pData[Index] == ',')
                                        {
                                            m_uhMillisecond = (unsigned short)(atof(Msg) * 1000);

                                            /*  时间信息秒是否带有小数点 */
                                            m_bSecondWithDot = true;
                                        }
                                        else
                                        {
                                            Result = false;
                                            /* 置<1>UTC当前时间标志位 */
                                            m_uhReceriveGPZDA_Flag &= (~WORD_BIT_0_GPS);
                                        }
                                    }
                                    else
                                    {
                                        /*  时间信息秒是否带有小数点 */
                                        m_bSecondWithDot = false;
                                    }
                                }
                                else
                                {
                                    /*  时间信息秒是否带有小数点 */
                                    m_bSecondWithDot = false;
                                }

                                /* 置<1>UTC当前时间标志位 */
                                m_uhReceriveGPZDA_Flag |= WORD_BIT_0_GPS;

                                /* 这里不做判断
                                if(m_ucStarNum >= m_ucEnableUseLongLat_StarNum) */
                                {
                                    /* 可以使用日时间标志 */
                                    m_bEnableUseTime_Flag	= true;
                                }
                            }
                            else
                            {
                                Result = false;
                                /* 置<1>UTC当前时间标志位 */
                                m_uhReceriveGPZDA_Flag &= (~WORD_BIT_0_GPS);
                            }
                        }
                        else
                        {
                            Result = false;
                            /* 置<1>UTC当前时间标志位 */
                            m_uhReceriveGPZDA_Flag &= (~WORD_BIT_0_GPS);
                        }
                    }
                    else
                    {
                        Result = false;
                        /* 置<1>UTC当前时间标志位 */
                        m_uhReceriveGPZDA_Flag &= (~WORD_BIT_0_GPS);
                    }
                }
                else
                {
                    Result = false;
                    /* 置<1>UTC当前时间标志位 */
                    m_uhReceriveGPZDA_Flag &= (~WORD_BIT_0_GPS);
                }
            }
            break;
        case 2:
            {
                /* <2> UTC日期，01到31(前导位数不足则补0) */
                /* 有日期信息 */
                if(pData[Index] != ',')
                {
                    /* 日 dd */
                    Day = 0;
                    for(i = Index; i < Index + 2; i++)
                    {
                        if(pData[i] >= '0' && pData[i] <= '9')
                            Day = Day * 10 + (pData[i] - '0');
                        else
                        {
                            Result = false;
                            /* 置<2> UTC日期标志位 */
                            m_uhReceriveGPZDA_Flag &= (~WORD_BIT_1_GPS);
                            break;
                        }
                    }

                    if(Result && Day <= 31)
                    {
                        Index += 2;
                        /* 保存 日*/
                        m_ucDay = Day;
                        /* 置<2> UTC日期标志位 */
                        m_uhReceriveGPZDA_Flag |= WORD_BIT_1_GPS;
                    }
                    else
                    {
                        Result = false;
                        /* 置<2> UTC日期标志位 */
                        m_uhReceriveGPZDA_Flag &= (~WORD_BIT_1_GPS);
                    }
                }
                else
                {
                    Result = false;
                    /* 置<2> UTC日期标志位 */
                    m_uhReceriveGPZDA_Flag &= (~WORD_BIT_1_GPS);
                }
            }
            break;
        case 3:
            {
                /* <3> UTC月份，01到12 (前导位数不足则补0) */
                /* 有月份信息 */
                if(pData[Index] != ',')
                {
                    /* 月 */
                    Month = 0;
                    for(i = Index; i < Index + 2; i++)
                    {
                        if(pData[i] >= '0' && pData[i] <= '9')
                            Month = Month * 10 + (pData[i] - '0');
                        else
                        {
                            Result = false;
                            /* 置<3> UTC月份标志位 */
                            m_uhReceriveGPZDA_Flag &= (~WORD_BIT_2_GPS);
                            break;
                        }
                    }

                    if(Result && Month <= 12)
                    {
                        Index += 2;
                        /* 保存 月 */
                        m_ucMonth = Month;
                        /* 置<3> UTC月份标志位 */
                        m_uhReceriveGPZDA_Flag |= WORD_BIT_2_GPS;
                    }
                    else
                    {
                        Result = false;
                        /* 置<3> UTC月份标志位 */
                        m_uhReceriveGPZDA_Flag &= (~WORD_BIT_2_GPS);
                    }
                }
                else
                {
                    Result = false;
                    /* 置<3> UTC月份标志位 */
                    m_uhReceriveGPZDA_Flag &= (~WORD_BIT_2_GPS);
                }
            }
            break;
        case 4:
            {
                /* <4> UTC年，格式yyyy */
                /* 有 年 信息 */
                if(pData[Index] != ',')
                {
                    /* 年 */
                    Year = 0;
                    for(i = Index; i < Index + 4; i++)
                    {
                        if(pData[i] >= '0' && pData[i] <= '9')
                            Year = Year * 10 + (pData[i] - '0');
                        else
                        {
                            Result = false;
                            /* 置<4> UTC年标志位 */
                            m_uhReceriveGPZDA_Flag &= (~WORD_BIT_3_GPS);
                            break;
                        }
                    }

                    if(Result)
                    {
                        Index += 4;
                        /* 保存 年 */
                        m_uhYear = Year;
                        /* 置<4> UTC年标志位 */
                        m_uhReceriveGPZDA_Flag |= WORD_BIT_3_GPS ;
                    }
                    else
                    {
                        Result = false;
                        /* 置<4> UTC年标志位 */
                        m_uhReceriveGPZDA_Flag &= (~WORD_BIT_3_GPS);
                    }
                }
                else
                {
                    Result = false;
                    /* 置<4> UTC年标志位 */
                    m_uhReceriveGPZDA_Flag &= (~WORD_BIT_3_GPS);
                }
            }
            break;
        case 5:
            {
                /* <5> 本地时区小时偏移量，00到+/-13 (正常为00到+13或00到-12，前导位数不足则补0) */
                if(pData[Index] != ',')
                {
                    *Msg = pData[Index++];
                    count = 1;
                    while(pData[Index] != ','
                        && 	(	(pData[Index] >= '0' &&  pData[Index] <= '9')
                        || (pData[Index] == '-' )))
                    {
                        *(Msg + count) = pData[Index++];
                        count++;
                    }
                    *(Msg + count) = '\0';
                    OffsetHour = (char)(atoi(Msg));

                                        // 测试FURUNO GP 90，设置时区为+8，实际输出为-8，故调整之, mofi 2011-03-12
                                        OffsetHour = OffsetHour;

                    if(pData[Index] == ',' && OffsetHour >= -12 && OffsetHour <= 13)
                    {
                        /* 本地时区小时偏移量 */
                        m_cOffsetHour = OffsetHour;

                        /* 置<5> 本地时区小时偏移量标志位 */
                        m_uhReceriveGPZDA_Flag |= WORD_BIT_4_GPS;
                    }
                    else
                    {
                        Result = false;
                        /* 置<5> 本地时区小时偏移量标志位 */
                        m_uhReceriveGPZDA_Flag &= (~WORD_BIT_4_GPS);
                    }
                }
                else
                {
                    /* 置<5> 本地时区小时偏移量标志位 */
                    m_uhReceriveGPZDA_Flag &= (~WORD_BIT_4_GPS);
                }
            }
            break;
        case 6:
            {
                /* <6> 本地时区分钟偏移量，00到+/-59 */
                if(pData[Index] != '*')
                {
                    *Msg = pData[Index++];
                    count = 1;
                    while(pData[Index] != ','
                        && 	(	(pData[Index] >= '0' &&  pData[Index] <= '9')
                        || (pData[Index] == '-' )))
                    {
                        *(Msg + count) = pData[Index++];
                        count++;
                    }
                    *(Msg + count) = '\0';
                    OffsetMinute = (char)(atoi(Msg));

                    if(pData[Index] == '*' && OffsetMinute >= -59 && OffsetMinute <= 59)
                    {
                        /*本地时区分钟偏移量 */
                        m_cOffsetMinute = OffsetMinute ;

                        /* 置<6> 本地时区分钟偏移量标志位 */
                        m_uhReceriveGPZDA_Flag |= WORD_BIT_5_GPS;
                    }
                    else
                    {
                        Result = false;
                        /* 置<6> 本地时区分钟偏移量标志位 */
                        m_uhReceriveGPZDA_Flag &= (~WORD_BIT_5_GPS);
                    }
                }
                else
                {
                    /* 置<6> 本地时区分钟偏移量标志位 */
                    m_uhReceriveGPZDA_Flag &= (~WORD_BIT_5_GPS);
                }
            }
            break;
        default:
            {
                Result = false;
                break;
            }
        }/* switch 结束 */
    }/* while 结束 */

        if(m_uhReceriveGPZDA_Flag & WORD_BIT_0_GPS)
                m_bEnableUseTime_Flag = true;

        const unsigned short dateFlag = (WORD_BIT_1_GPS | WORD_BIT_2_GPS | WORD_BIT_3_GPS);
        if((m_uhReceriveGPZDA_Flag & dateFlag) == dateFlag)
                m_bEnableUseDate_Flag = true;

    return Result;
}


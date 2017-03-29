#ifndef PARSEGPS_H
#define PARSEGPS_H


/********************************************************************************************************
 *	主题：			GPS数据处理模块
 *	内容：			GPS数据处理，包括日期、时间和经纬度、高度等信息
*********************************************************************************************************/

#include <QtCore/QtGlobal>

/* 操作系统平台 */
#define		SYSTEM_LINUX
#undef		SYSTEM_WIN32
#undef		SYSTEM_QNX
#undef		SYSTEM_VXWORKS

#ifdef   SYSTEM_LINUX
/*LINUX系统需要的头文件*/
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#endif

#ifdef SYSTEM_QNX
/*qnx系统需要的头文件*/
#include <sys/irqinfo.h>
#include <sys/types.h>
#include <sys/kernel.h>
#endif

#ifdef SYSTEM_VXWORKS
/*使用的VxWorks头文件*/
#include "vxWorks.h"
#include "semaphore.h"
#include "taskLib.h"
#include "string.h"
#include "math.h"
#include "sys/socket.h"
#include "socket.h"
#include "sockLib.h"
#include "iolib.h"
#include "in.h"
#include "inetLib.h"
#include "netinet/in_systm.h"
#include "netinet/ip.h"
#include "netinet/in.h"
#include "stdlib.h"
#include "stdio.h"
#include "semLib.h"
#include "timers.h"
#include "logLib.h"
#include "signal.h"
#include "iostream.h"
#include "intLib.h"
#include "tickLib.h"
#include "iv.h"
#include "sysLib.h"
#include "kernelLib.h"
#include "sysLib.h"
#endif


/*－－－－－－－－－－－－－－－－－－－－－宏定义－－－－－－－－－－－－－－－－－－－－－－－－－－*/
/* 24小时 */
#define TEWENTYFOUR_HOUR_GPS	24

/* 定义一个字的各个数据位 */
#define WORD_BIT_0_GPS		0x0001
#define WORD_BIT_1_GPS		0x0002
#define WORD_BIT_2_GPS		0x0004
#define WORD_BIT_3_GPS		0x0008
#define WORD_BIT_4_GPS		0x0010
#define WORD_BIT_5_GPS		0x0020
#define WORD_BIT_6_GPS		0x0040
#define WORD_BIT_7_GPS		0x0080
#define WORD_BIT_8_GPS		0x0100
#define WORD_BIT_9_GPS		0x0200
#define WORD_BIT_10_GPS		0x0400
#define WORD_BIT_11_GPS		0x0800
#define WORD_BIT_12_GPS		0x1000
#define WORD_BIT_13_GPS		0x2000
#define WORD_BIT_14_GPS		0x4000
#define WORD_BIT_15_GPS		0x8000
/*－－－－－－－－－－－－－－－－－－－－－数据结构定义－－－－－－－－－－－－－－－－－－－－－－－*/

/* 经纬度结构体定义*/
typedef struct LATITUDE_TYPE_STRUCT
{
    short  hDegree;
    short  hMinute;
    double dSecond;
}LATITUDE_TYPE_STRUCT;

/* 年月日时分秒结构体定义*/
typedef struct DATE_TIME_TYPE_STRUCT
{
    unsigned short	uhYear;
    unsigned char	ucMonth;
    unsigned char	ucDay;
    unsigned char	ucHour;
    unsigned char	ucMinute;
    unsigned char	ucSecond;
    unsigned short	uhMillisecond;
}DATE_TIME_TYPE_STRUCT;

/* GPS可见卫星信息结构体定义*/
typedef struct STAR_INFO_TYPE_STRUCT
{
    unsigned char	ucStarIndex;		/* 卫星编号01～32				*/
    unsigned char	ucStarElevation;	/* 卫星仰角(00～90度)			*/
    unsigned short	uhStarAzimuth;		/* 卫星方位角(000～359度)		*/
    unsigned char	ucStarSNR;			/* 信号信噪比(00～99dB)			*/
    unsigned char	ucStarInfoValid;	/* 本组信息是否有效,  0 无效,
                                           第0位为1 卫星编号有效
                                           第1位为1 卫星仰角有效
                                           第2位为1 卫星方位角有效
                                           第3位为1 信号信噪比有效
                                           其它位无定义					*/
}STAR_INFO_TYPE_STRUCT;

/////////////////////////////////////////////////////////////////////////////
// CGPS command target

/*－－－－－－－－－－－－－－－－－－－－－类CGPS 定义－－－－－－－－－－－－－－－－－－－－－－－*/

class CGPS
{
/* －－－－－变量定义－－－－－－－－－ */
public:

    /*  可以使用经纬度数值标志 */
    bool m_bEnableUseLongLat_Flag;

    /*  可以使用高度数值标志 */
    bool m_bEnableUseHeight_Flag;

    /*  可以使用日时间标志*/
    bool m_bEnableUseTime_Flag;

    /*  可以使用日期标志*/
    bool m_bEnableUseDate_Flag;


    /*  GPS接收机收到3颗卫星的信号可以输出2D（就是2维）数据，
        只有经纬度，没有高度，如果收到4颗以上的卫星，就输出3D数据，
        可以提供海拔高度。但高度数据有一些误差 ,默认情况下只有达到
        5颗星时才能使用经纬度数据，用户可以根据需要调用函数来设置此值*/
    unsigned char m_ucEnableUseLongLat_StarNum;

    /*   设置本时区的时差，取值范围 －12～13，默认为北京时区，时差8*/
    char m_cOffsetHour;

    /* 本地时区分钟偏移量，00到+/-59,此变量暂时未用于时间校正 */
    char m_cOffsetMinute;

    /*  时间信息秒是否带有小数点 */
    bool m_bSecondWithDot;

    /*  NMEA0183协议是否是V3.0版本 */
    bool m_bVer3;

    /*  是否存在本程序还没有纳入的数据格式 */
    bool m_bExistUnExplainData;

    /*----------------------------------GPGGA--------------------------------*/
    /*  收到GPGGA报文标志 由各个位来表示数据接收解释状态，
        每个bit位：0 表示未接收到该数据(或者解释出错)，1表示解释成功

        第0位:	<1> UTC当前时间
        第1位:	<2> 纬度
        第2位:	<3> 纬度属性
        第3位:	<4> 经度
        第4位;	<5> 经度属性
        第5位:	<6> 接收机定位标志
        第6位:	<7> 跟踪到的GPS卫星数
        第7位:	<8> 水平精度因子
        第8位:	<9> GPS天线所处海拔高度
        第9位:	<M> 表示单位米；
        第10位:	<10> 大地水准面高度
        第11位:	<M> 表示单位米；
        第12位:	<11> 有效数据年龄
        第13位:	<12> 差分基准站号
        第14位:	*
        第15位:	检查和		*/
    unsigned short m_uhReceriveGPGGA_Flag;

    /* 时、分、秒和毫秒 */
    unsigned char	m_ucHour;
    unsigned char	m_ucMinute;
    unsigned char	m_ucSecond;
    unsigned short	m_uhMillisecond;

    /* 经纬度和高度信息 */
    LATITUDE_TYPE_STRUCT m_sLatitude;
    LATITUDE_TYPE_STRUCT m_sLongitude;

    /* 纬度属性，南北半球，N 0 / S 1  */
    unsigned char	m_ucLatitudeAttribute;
    /* 经度属性，东西半球，E 0 / W 1  */
    unsigned char	m_ucLongitudeAttribute;

    /* 接收机定位标志，(0=没有定位，1=实时GPS（单点定位），2=差分GPS，6=正在估算) */
    unsigned char	m_ucReceriverLocationFlag;

    /* GPS收到的卫星颗数，大于等于5颗以上才能使用GPS数据 */
    unsigned char	m_ucStarNum;

    /* 水平精度因子 0.5～99.9*/
    float m_fH_PrecisonGene;

    /* GPS天线所处海拔高度，－9999.9到99999.9*/
    float m_fHeight;

    /* 大地水准面高度，－999.9到9999.9 */
    float m_fWaterLevelHeight;

    /* 有效数据年龄，最后一次有效差分定位时和现在的时间间隔，单位为秒。若不是差分GPS，则置为最大值65535 */
    unsigned short m_uhValidTime;

    /* 差分基准站号(0000～1023，不足位数时前面补0)，若不是差分GPS，则置为最大值65535 */
    unsigned short m_uhDifferenceStationNum;

    /*----------------------------------GPRMC--------------------------------*/
    /*  收到GPRMC报文标志 由各个位来表示数据接收解释状态，
        每个bit位：0 表示未接收到该数据(或者解释出错)，1表示解释成功

        第0位:	<1> UTC当前时间
        第1位:	<2> 状态字
        第2位:	<3> 纬度
        第3位:	<4> 纬度属性
        第4位:	<5> 经度
        第5位;	<6> 经度属性
        第6位:	<7> 天线移动的速度
        第7位:	<8> 相对地面方向
        第8位:	<9> 当前日期
        第9位:	<10> 磁偏角
        第10位:	<11> 磁偏方向
        第11位:	<12> 模式指示
        第12位: 无定义
        第13位: 无定义
        第14位:	*
        第15位:	检查和		*/
    unsigned short m_uhReceriveGPRMC_Flag;

    /* GPRMC 和GPGLL 报文定位成功标志 1(A)表示定位成功，(0)V表示目前没有定位*/
    unsigned char m_ucGP_LocationSucessFlag;

    /* 天线移动的速度，从000.0到999.9节 */
    float m_fGPRMC_Velocity_knots;

    /* 相对地面方向，000.0到359.9度,以真北为参考基准 */
    float m_fGPRMC_Velocity_Azimuth;

    /*	年、月、日 ，年是完整的4个数字，
        如果取到两位的年份，则直接增加2000（90年代的早已经过去了，
        一百年以后的事情我也管不了啦）	*/
    unsigned short	m_uhYear;
    unsigned char	m_ucMonth;
    unsigned char	m_ucDay;

    /* 磁偏角，从000.0到180.0度 */
    float m_fGPRMC_MagneticAzimuth;

    /* 磁偏方向属性，0/E(东)或1/W(西) */
    unsigned char m_ucGPRMC_MagneticAttr;

    /* 模式指示（仅NMEA0183 3.00版本输出，0/N=数据无效,1/A=自主定位,2/D=差分,3/E=估算）； */
    unsigned char m_ucGPWorkModel;

    /*----------------------------------GPGSV--------------------------------*/
    /*  收到GPGSV报文标志 由STAR_INFO_TYPE_STRUCT中ucStarInfoValid各个位来
        表示数据接收解释状态（原因：GPGSV报文可能分多组发送）*/
    bool m_bReceriveGPGSV_Flag;

    /* 可见卫星的总数，00 ～ 12 */
    unsigned char	m_ucGPGSV_StarNum;

    /*	GPS可见卫星信息,包括卫星编号、卫星仰角(00～90度)、卫星方位角(000～359度)、
        信号信噪比(00～99dB)和信息是否有效标志，目前的接收机最多能收到12颗卫星信息，
        卫星编号的范围为1～32，为防止出现跟踪到12颗卫星后发生卫星编号变化的现象，
        把数组元素设为32个使用数组下标＋1来表示卫星编号							  */
    STAR_INFO_TYPE_STRUCT m_sgStarInfo[32];

    /*----------------------------------GPGSA--------------------------------*/
    /* 收到GPGSA报文标志 由各个位来表示数据接收解释状态，
        每个bit位：0 表示未接收到该数据(或者解释出错)，1表示解释成功

        第0位:	<1> 模式 ：M = 手动， A = 自动；
        第1位:	<2> 定位型式 1 = 未定位， 2 = 二维定位， 3 = 三维定位
        第2位～第10位:无定义，第1信道到第12信道的信息有m_ucgGPGSA_StarNum来表征
        第11位: <15> PDOP位置精度因子(0.5~99.9)
        第12位: <16> HDOP水平精度因子(0.5~99.9)
        第13位: <17> VDOP垂直精度因子(0.5~99.9)
        第14位:	*
        第15位:	检查和		*/
    unsigned short m_uhReceriveGPGSA_Flag ;

    /* 模式:0/A = 自动(Automatic, 3D/2D),1/M = 手动(Manual, forced to operate in 2D or 3D)*/
    unsigned char m_ucGPGSA_WorkModel;

    /* 型式 1 = 未定位， 2 = 二维定位， 3 = 三维定位 */
    unsigned char m_ucGPGSA_PositionModel;

    /* PRN 数字：01～32， 表天空使用中的卫星编号,默认为0，表示该信道没有卫星信息 */
    unsigned char m_ucgGPGSA_StarNum[12];

    /* PDOP位置精度因子(0.5~99.9),(2.6) */
    float m_fGPGSA_PDOP;

    /* HDOP水平精度因子(0.5~99.9),(2.5) */
    float m_fGPGSA_HDOP;

    /* VDOP垂直精度因子(0.5~99.9),(1.0) */
    float m_fGPGSA_VDOP;
    /*----------------------------------GPGLL--------------------------------*/
    /* 地理定位信息($GPGLL) 经纬度信息和UTC时间 */

    /*  收到GPGLL报文标志 由各个位来表示数据接收解释状态，
        每个bit位：0 表示未接收到该数据(或者解释出错)，1表示解释成功

        第0位:	<1> 纬度
        第1位:	<2> 纬度属性
        第2位:	<3> 经度
        第3位;	<4> 经度属性
        第4位:	<5> UTC当前时间
        第5位:	<6> 接收机定位标志
        第6位~第13位: 无定义
        第14位:	*
        第15位:	检查和		*/
    unsigned short m_uhReceriveGPGLL_Flag ;

    /*----------------------------------GPVTG--------------------------------*/
    /*  收到GPVTG报文标志 由各个位来表示数据接收解释状态，
        每个bit位：0 表示未接收到该数据(或者解释出错)，1表示解释成功

        第0位:	<1> 运动角度(真北)
        第1位:	<2> T=真北参照系
        第2位:	<3> 运动角度(磁北)
        第3位:	<4> M=磁北参照系
        第4位:	<5> 水平运动速度(N)
        第5位;	<6> N=节，Knots
        第6位:	<7> 水平运动速度(K)
        第7位:	<8> K=公里/时，km/h
        第8位:	<9> 模式指示
        第9位～13位:	无定义
        第14位:	*
        第15位:	检查和		*/
    unsigned short m_uhReceriveGPVTG_Flag;


    /* 相对地面方向，000.0到359.9度,以真北为参考基准 */
    float m_fGPVTG_TrueNorth_Azimuth;

    /* 相对地面方向，000.0到359.9度,以磁北为参考基准 */
    float m_fGPVTG_MagneticNorth_Azimuth;

    /* 水平运动速度，从000.0到999.9节 */
    float m_fGPVTG_Velocity_knots;

    /* 水平运动速度，0000.0～1851.8公里/小时 */
    float m_fGPVTG_Velocity_km;

    /*----------------------------------GPZDA--------------------------------*/
    /*  收到GPZDA报文标志 由各个位来表示数据接收解释状态，
        每个bit位：0 表示未接收到该数据(或者解释出错)，1表示解释成功

        第0位:	<1> UTC时间
        第1位:	<2> UTC日期
        第2位:	<3> UTC月份
        第3位:	<4> UTC年
        第4位:	<5> 本地时区小时偏移量
        第5位;	<6> 本地时区分钟偏移量
        第6位～13位:无定义
        第14位:	*
        第15位:	检查和		*/
    unsigned short m_uhReceriveGPZDA_Flag;



/* －－－－－构造函数和析构函数－－－－ */
public:
    CGPS();
    virtual ~CGPS();

/* －－－－－函数定义－－－－－－－－－ */
public:
    /* GPS数据处理 */
    bool GpsDataProcess(quint8 *packet);

    /* 初始化成员变量 */
    void CGPSInitPara();

    /*	设置可以使用经纬度信息时的卫星个数和本时区的时差*/
    void CGPSSetPara(unsigned char StarNum,char OffsetHour);

    /* 获取经过校正的年月日时分秒  ucAdjustHour  时差，单位小时*/
    DATE_TIME_TYPE_STRUCT GetAdjustTimeAndDate(int offsetHour=0);

    /* GPS数据处理 */
    bool GPS_DataProcess(unsigned char *group);

    /* 获取GPS数据GPGGA格式数据 (GPS定位信息固定数据输出语句) */
    bool GetGPS_GPGGA(unsigned char *pData);

    /* 获取GPS数据GPRMC格式数据 (GPS推荐定位信息) */
    bool GetGPS_GPRMC(unsigned char* pData);

    /* 获取GPS数据GPGSV格式数据 (可视卫星状态输出语句) */
    bool GetGPS_GPGSV(unsigned char* pData);

    /* 获取GPS数据GPGSA格式数据 (当前卫星信息) */
    bool GetGPS_GPGSA(unsigned char *pData);

    /* 获取GPS数据GPGLL格式数据 (地理定位信息) */
    bool GetGPS_GPGLL(unsigned char* pData);

    /* 获取GPS数据GPVTG格式数据 (地面速度信息) */
    bool GetGPS_GPVTG(unsigned char* pData);

    /* 获取GPS数据GPZDA格式数据,年月日、时分秒以及时区和分钟的偏移量*/
    bool GetGPS_GPZDA(unsigned char* pData);

protected:
        // 判断检查和是否正确
        bool Check(unsigned char* header, unsigned char* data);
};


#endif // PARSEGPS_H

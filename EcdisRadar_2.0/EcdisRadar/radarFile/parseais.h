#ifndef PARSEAIS_H
#define PARSEAIS_H


/********************************************************************************************************
 *	主题：			AIS数据处理模块
 *	内容：			AIS数据处理，包括精确船位、航向、航速（矢量线）、转向速度和最近船舶会遇距离等动态信息和船名、呼号、船型、船长与船宽等静态信息
*********************************************************************************************************/

#include <QtCore/QtGlobal>
#include <QtCore/QDateTime>

/*Dimension/reference for position
船舶尺寸相对于参考点的距离，a,b,c,d*/
typedef struct tagRefOfPosition
{
    quint16 dimA;//9
    quint16 dimB;//9
    quint8  dimC;//6
    quint8  dimD;//6
}REFOFPOSITION;
typedef enum AisMsgType
{
    /*信息识别码，指明消息类型*/
    ShipDynmicPosInfo1 = 1, //计划中的位置报告
    ShipDynmicPosInfo2 = 2, //指配的计划中的位置报告
    ShipDynmicPosInfo3 = 3, //特定位置报告，对查询的响应
    BaseInfo = 4,           //基站的位置、UTC、数据和当前时隙编号
    ShipStaticInfo = 5,     //计划中的静态和航行相关船只数据报告
    BinaryAddresInfo = 6,   //寻址通信的二进制数据
    BinaryConfirmInfo = 7,  //确认收到寻址的二进制数据
    BinaryBroadcastInfo = 8,//广播通信的二进制数据
    TypeNum = 26            //类型数量
}AisMsgType;

typedef enum RepeatIndicator
{
    Repeat0 = 0,    //指示应该重发的次数
    Repeat1 = 1,
    Repeat2 = 2,
    NonRepeat = 3,	//表示不再重发
        UnUsedRi = 4	//表示不可用
}RepeatIndicator;

typedef enum NaviStatus
{
    Navigating = 0,	//机航中
    Achoring = 1,   //锚泊
    NonConrtol = 2, //未操纵
    SemiControl = 3,//有限操纵性
    Constrained = 4,//受船舶吃水限制
    Moored = 5,		//系泊
    Agound = 6,     //搁浅
    Fishing = 7,	//从事捕捞
    Sailing = 8,    //帆航中
    Reserved = 9,   //留着将来使用9-14
    Default = 15,   //默认值
        UnUsedNs = 16
}NaviStatus;


typedef struct tagShipDynamicInfo
{
    /*******消息类型1的内容:船舶动态信息（内容 位置 所占位数 取值范围 说明）*********/
    AisMsgType  msgType;  //信息识别码 1
    RepeatIndicator  repeatIndicator;//重复次数指示
    quint32	userID;     //用户识别码 8-37 30   MMSI号码

    NaviStatus  naviStatus; //航行状态 38-41 4 0-15
    qint8   rateOfTurn; //转向率 42-49 8 -127 - +127
    double sog;        //对地航速 50-59 10 0-1022 以1/10节距为单位，1023=无；1022= 102.2节。
    bool    positionAccuracy;//船位精确度 60 1 0,1 1=高精度（差分式）；0= 低精度。
    double  longitude;  //经度 61-88 28 -180 - +180 用1/10000分表示的经度（东= +，西= －）
    double  latitude;   //纬度 89-115 27 -90 - +90 用1/10000分表示的纬度（北= +，南= －）
    double  cog;        //对地航向 116-127 12 0-3599 以1/10度表示的航向
    quint16 trueHeading;//船首真航向 128-136 9 0-359
    quint8  timeStamp;  //时间标记 137-142 6

    quint8  manIndicator;// Maneuver Indicator  143-144 2 特定操纵指示符（即关于内陆水道的区域性通行安排）
    //Spare 备用 145-147  3
    bool    raimFlag;   //RAIM Flag   148 1  RAIM（接收机自主整体检测）电子定位装置的标志；0 = RAIM 未使用 = 默认值；1 = RAIM 正在使用，
    quint32 commState;  //通信状态Communication state   149-167 19
}SHIPDYNAMICINFO;

typedef enum EPFD
{
    DefaultEPFD = 0,
    GPS = 1,
    GNSS = 2, //GLONASS
    GPSandGNSS = 3,
    LoranC = 4,
    Chayka = 5,
    IntegratedNaviSys = 6, //综合导航系统
    Surveyed = 7, //正在研究
    Galileo = 8,
    UnUsedEp = 15
}EPFD;

typedef enum ShipType{
    defaultShip,//0
    futureUseShip,//1-19
    WIG,//2X, Wing in ground (WIG), 25 26 27 28 29 留做将来使用
    fishingShip, //30
    towingShip, //31
    bigTowingShip, //32 Towing: length exceeds 200m or breadth exceeds 25m
    dredgingShip,//33 Dredging or underwater ops
    DivingShip,//34 Diving ops
    militaryShip,//35 Military ops
    sailingShip,//36 Sailing
    pleasureCraftShip, //37 Pleasure Craft
    reserved, //38 39 Reserved
    HSC, //High speed craft (HSC),4X
    pilotVessel,//50 Pilot Vessel,引航船舶
    searchVessel,//51 Search and Rescue vessel, 搜救船舶
    tugShip,//52 Tug,拖轮
    portTenderShip,//53 Port Tender, 港口补给船
    antiPollutionShip,//54 Anti-pollution equipment, 安装有防污染设施或设备的船舶
    lawShip,//55, Law Enforcement, 执法船舶
    localVessel,//56 57 Spare - Local Vessel,备用 – 当地船舶指配使用
    medicalShip,//58, Medical Transport, 医疗运送船
    mob83Ship,//59 符合RR 第18 号决议（Mob-83）的船舶
    passengerShip, //6X 客轮,66留做将来使用
    cargoShip,//7X 货轮, 75 76 77 78 留做将来使用
    tankerShip,//8X 油轮, 85 86 87 88 留做将来使用
    otherTypeShip,//9X 其他类型的船舶 95 96 97 98 留做将来使用
    reservedTypeShip //100-199 = 保留，用于区域性用途
                    //200-255 = 保留，将来用
}ShipType;

/*估计到达时间；MMDDHHMM UTC
比特19-16：月；1-12；0 = 不可用 = 默认值
比特15-11：天；1-31；0 = 不可用 = 默认值
比特10-6：时；0-23；24 = 不可用 = 默认值
比特5-0：分；0-59；60 = 不可用 = 默认值*/
typedef struct tagDateTime
{
    quint8  month;
    quint8  day;
    quint8  hour;
    quint8  minute;

        bool isValid() const
        {
                return (!((month==0||month>12) && (day==0) && (hour==0||hour==24) && (minute==0||minute==60)));
        }

        quint32 time() const
        {
                if(!isValid())
                        return 0;

                QDate date0 = QDate::currentDate();
                QDate date(date0.year(), month, day);
                QTime tm(hour, minute, 0);
                QDateTime datetime(date, tm);
                if(!datetime.isValid())
                        return 0;

                return datetime.toTime_t();
        }

}DATETIME;


typedef struct tagShipStaticInfo
{
    /*******消息类型5的内容:船舶静态信息（内容 位置 所占位数 取值范围 说明）*********/
    AisMsgType  msgType;  //信息识别码 5
    RepeatIndicator  repeatIndicator;//重复次数指示
    quint32	userID;     //用户识别码 8-37 30   MMSI号码

    quint8  versionAis; //AIS version indicator AIS版本 38-39 2 0-3
    quint32	imoNumber;  //IMO编号 40-69 30 1-999999999，0 = 不可用 = 默认值
    quint8	callSign[7];   //呼号 70-111 42，7 × 6 比特ASCII 字符，@@@@@@@ = 不可用 = 默认值
    quint8	shipName[20];   //船名 112-231 120，最长20 字符的6 比特ASCII 码，
    quint8	shipType; //船舶和货物类型 232-239 8
    REFOFPOSITION	refOfPosition;// 船舶尺寸以及定位设备位置 240-269 30，已报告位置的参考点
    EPFD	epfd;// Type of electronic position fixing device 270-273 4 0-15，
    DATETIME	eta;        //预计到达时间 274-293 20;    MMDDHHMM UTC
    float  maxDraught; // 最大吃水深度 294-301 8 0-255,  以1/10 m 为单位
    quint8  Destination[20];  //目的地 302-421 120
    bool    dte;        //数据终端就绪 422 1
}SHIPSTATICINFO;



class Ais
{
public:
    Ais();
    virtual ~Ais();

    /* AIS数据处理 */
    bool AisDataProcess(quint8 *packet, quint16 length);
    bool cleanDynamicInfo();
    bool cleanStaticInfo();

protected:
    /* 初始化成员变量 */
    void AisInit();

    /* AIS数据处理 */
    bool GetAIS_AIVDM(quint8 *packet, quint16 length);

    /* 获取AIS消息数据  */
    bool GetAisInfo(quint8 *pData, quint16 length);

    /* 解析AIS消息类型1格式数据(A类动态信息)  */
    bool GetAisInfo1(quint8 *pData);
    /* 解析AIS消息类型5格式数据(A类静态信息) */
    bool GetAisInfo5(quint8 *pData);
    /* 解析AIS消息类型18格式数据(B类动态信息)  */
    bool GetAisInfo18(quint8 *pData);
    /* 解析AIS消息类型24格式数据(B类静态信息)  */
    bool GetAisInfo24(quint8 *pData);
    /* 解析AIS消息类型19格式数据(B类扩展位置信息)  */
    bool GetAisInfo19(quint8 *pData);

    /*单个字符解码，将8bit表示的字符解码成6bit的二进制序列*/
    bool EightByteToSix(quint8 inEight,quint8& outSix);

    /*解码，将整个8bit表示的字符串解码成6bit的二进制序列*/
    bool EightStrToSix(quint8* inEight, quint8* outSix, quint16 length);



    /*获取1bits 到 8bits的整数数据
    lpInfo—调用EightStrToSix获取的解码数据
    dwBitStart –需要获取的数据的起始位. dwBitStart 从0开始
    byLen –数据所占位数*/
    quint8 GetByteValFromInfo(quint8* lpInfo,quint32 dwBitStart,quint8 byLen);


    /*获取9bits 到 16bits的整数数据*/
    quint16 GetWordValFromInfo(quint8* lpInfo,quint32 dwBitStart,quint8 wLen);

    /*获取17bits 到 32bits的整数数据*/
    quint32 GetDwordValFromInfo(quint8* lpInfo,quint32 dwBitStart,quint8 dwLen);

    /*获取字符串类型数据*/
    quint8* GetStringValFromInfo(quint8* lpInfo, quint32 dwBitStart, quint32 dwLen, quint8* info);

    //int 4, short 2, char 1, long 4,
public:
    bool bGetAisFlag;//AIS包成功解析标识；
    bool bGetInfo1_Flag;//消息类型1
    bool bGetInfo5_Flag;//消息类型5
    quint8 m_aisClass;
    SHIPDYNAMICINFO m_sInfo1;
    SHIPSTATICINFO m_sInfo5;

};


#endif // PARSEAIS_H


#include "parseais.h"
#include <stdlib.h>
#include <string.h>
#include <QtCore/QString>

// to include file for ntohl
#ifdef Q_OS_WIN32
#include <winsock2.h>
#endif
#ifdef Q_OS_LINUX
#include <netinet/in.h>
#endif

Ais::Ais()
{
    AisInit();

}


void Ais::AisInit()
{
    bGetAisFlag = false;
    bGetInfo1_Flag = false;
    bGetInfo5_Flag = false;

}

/* AIS数据处理 */
bool Ais::AisDataProcess(quint8 *packet, quint16 length)
{
    if(		(*(packet )	  == '!')
        &&	(*(packet + 1) == 'A')
        &&	(*(packet + 2) == 'I')
        &&	(*(packet + 3) == 'V')
        &&	(*(packet + 4) == 'D')
        &&	(*(packet + 5) == 'M'))
    {
        /* 获取AIS数据AIVDM格式数据 (船舶动态信息和静态信息) */
        return GetAIS_AIVDM(packet + 6, length-6);
    }
    else if(		(*(packet )	  == '!')
        &&	(*(packet + 1) == 'A')
        &&	(*(packet + 2) == 'I')
        &&	(*(packet + 3) == 'V')
        &&	(*(packet + 4) == 'D')
        &&	(*(packet + 5) == 'O'))
    {
        /* 获取AIS数据AIVDO格式数据 (本船的动态信息和静态信息) */
        return GetAIS_AIVDM(packet + 6, length-6);//同样的处理方式
    }

        return false;
}

bool Ais::GetAIS_AIVDM(quint8* packet, quint16 length)
{
    quint8* cur;
    cur = packet;
    int j = 0;
    //得到封装的消息码文，在5个逗号之后
    for(int i=0; i<5;){
        if(*cur == ',') i++;
        cur++;
        j++;
    }
    return GetAisInfo(packet + j, length-j);
}

bool Ais::GetAisInfo(quint8 *pData1, quint16 length)
{
    bool flag = false;

    quint8 pData[100];
    memset(pData, 0x00, 100);

        // 将8Bit字符转换为6Bit流
    if(!EightStrToSix(pData1, pData, length))
        return false;

    int msgID = GetByteValFromInfo(pData, 0, 6);

    if(msgID == 1 || msgID == 2 || msgID == 3)
    {	// A类动态信息
        bGetInfo1_Flag = GetAisInfo1(pData);
        flag = bGetInfo1_Flag;
        m_aisClass = 1;
    }
    else if(msgID == 4)
    {	// AIS基站信息
        flag = true;
        bGetAisFlag = false;
    }
    else if(msgID == 5)
    {	// A类静态信息
        bGetInfo5_Flag = GetAisInfo5(pData);
        flag = bGetInfo5_Flag;
        m_aisClass = 1;
    }
    else if(msgID == 18)
    {	// B类动态信息
        bGetInfo1_Flag = GetAisInfo18(pData);
        flag = bGetInfo1_Flag;
        m_aisClass = 2;
    }
    else if(msgID == 24)
    {	// B类静态信息
        bGetInfo5_Flag = GetAisInfo24(pData);
        flag = bGetInfo5_Flag;
        m_aisClass = 2;
    }
    else if(msgID == 19)
    {  //B级设别扩展位置报告
        bGetInfo5_Flag = GetAisInfo19(pData);
        flag = bGetInfo5_Flag;
        m_aisClass = 2;
    }
    else
    {
        m_aisClass = 0;
        bGetAisFlag = false;

    }
    return flag;

}

/*消息123，A类动态消息
|==============================================================================
|Field   |Len |Description              |Member   |Units
|0-5     | 6  |Message Type             |type     |Unsigned integer: 1-3
|6-7     | 2  |Repeat Indicator         |repeat   |See below...
|8-37    |30  |MMSI                     |mmsi     |Unsigned integer: 9 digits
|38-41   | 4  |Navigation Status        |status   |See table below
|42-49   | 8  |Rate of Turn (ROT)       |turn     |Signed integer: see below
|50-59   |10  |Speed Over Ground (SOG)  |speed    |Unsigned integer: see below
|60-60   | 1  |Position Accuracy        |accuracy |See below
|61-88   |28  |Longitude                |lon      |Minutes/10000 (see below)
|89-115  |27  |Latitude                 |lat      |Minutes/10000 (see below)
|116-127 |12  |Course Over Ground (COG) |course   |Relative to true north,
|        |    |                         |         |to 0.1 degree precision
|128-136 | 9  |True Heading (HDG)       |heading  |0 to 359 degrees,
|        |    |                         |         |511 = not available
|137-142 | 6  |Time Stamp               |second   |Second of UTC timestamp
|143-144 | 2  |Maneuver Indicator       |maneuver |See below
|145-147 | 3  |Spare                    |         |Not used
|148-148 | 1  |RAIM flag                |raim     |See below
|149-167 |19  |Radio status             |radio    |See below
|==============================================================================
*/
bool Ais::GetAisInfo1(quint8* pData)
{
        cleanDynamicInfo();
    m_sInfo1.msgType = (AisMsgType)GetByteValFromInfo(pData, 0, 6);
    m_sInfo1.repeatIndicator = (RepeatIndicator)GetByteValFromInfo(pData, 6, 2);

    m_sInfo1.userID = GetDwordValFromInfo(pData, 8, 30);
        if(m_sInfo1.userID == 0)
                return false;

    const int naviStatusValue = GetByteValFromInfo(pData, 38, 4);
    if  ((naviStatusValue >= 9)&&(naviStatusValue <= 14))
        m_sInfo1.naviStatus = Reserved;
    else
        m_sInfo1.naviStatus = (NaviStatus)naviStatusValue;

    m_sInfo1.rateOfTurn = (char)GetByteValFromInfo(pData, 42, 8);
    m_sInfo1.sog = GetWordValFromInfo(pData, 50, 10)/10.0;
    m_sInfo1.positionAccuracy = GetByteValFromInfo(pData, 60, 1);
    m_sInfo1.longitude = (int)GetDwordValFromInfo(pData, 61, 28)/(10000*60.0);
    m_sInfo1.latitude = (int)GetDwordValFromInfo(pData, 89, 27)/(10000*60.0);
    m_sInfo1.cog = GetWordValFromInfo(pData, 116, 12)/10.0;
    m_sInfo1.trueHeading = GetWordValFromInfo(pData, 128, 9);
    m_sInfo1.timeStamp = GetByteValFromInfo(pData, 137, 6);

    m_sInfo1.manIndicator = GetByteValFromInfo(pData, 143, 2);
    m_sInfo1.raimFlag = GetByteValFromInfo(pData, 148, 1);
    m_sInfo1.commState = GetDwordValFromInfo(pData, 149, 19);

    return true;
}

/* === Type 5: Ship static and voyage related data ===
|==============================================================================
|Field   |Len |Description            |Member       |Encoding
|0-5     |  6 |Message Type           |type         |Unsigned integer: 5
|6-7     |  2 |Repeat Indicator       |repeat       |Message repeat count
|8-37    | 30 |MMSI                   |mmsi         |Unsigned integer: 9 digits
|38-39   |  2 |AIS Version            |ais_version  |0=<<ITU1371>>,
|        |    |                       |             |1-3 = future editions
|40-69   | 30 |IMO Number             |imo          |Unsigned IMO ship ID number
|70-111  | 42 |Call Sign              |callsign     |7 six-bit characters
|112-231 |120 |Vessel Name            |shipname     |20 six-bit characters
|232-239 |  8 |Ship Type              |shiptype     |See table below
|240-248 |  9 |Dimension to Bow       |to_bow       |Unsigned integer: Meters
|249-257 |  9 |Dimension to Stern     |to_stern     |Unsigned integer: Meters
|258-263 |  6 |Dimension to Port      |to_port      |Unsigned integer: Meters
|264-269 |  6 |Dimension to Starboard |to_starboard |Unsigned integer: Meters
|270-273 |  4 |Position Fix Type      |epfd         |As in Type 4 EPSD codes
|274-277 |  4 |ETA month              |month        |1-12, 0=N/A (default)
|278-282 |  5 |ETA day                |day          |1-31, 0=N/A (default)
|283-287 |  5 |ETA hour               |hour         |0-23, 24=N/A (default)
|288-293 |  6 |ETA minute             |minute       |0-59, 60=N/A (default)
|294-301 |  8 |Draught                |draught      |Unsigned integer: Meters/10
|302-421 |120 |Destination            |destination  |20 6-bit characters
|422-422 |  1 |DTE                    |dte          |0=Data terminal ready,
|        |    |                       |             |1=Not ready (default)
|423-423 |  1 |Spare                  |             |Not used
|==============================================================================
*/
bool Ais::GetAisInfo5(quint8* pData)
{
    cleanStaticInfo();
    m_sInfo5.msgType = (AisMsgType)GetByteValFromInfo(pData, 0, 6);
    m_sInfo5.repeatIndicator = (RepeatIndicator)GetByteValFromInfo(pData, 6, 2);

    m_sInfo5.userID = GetDwordValFromInfo(pData, 8, 30);
        if(m_sInfo1.userID == 0)
                return false;

    m_sInfo5.versionAis = GetByteValFromInfo(pData, 38, 2);
    m_sInfo5.imoNumber = GetDwordValFromInfo(pData, 40, 30);

    GetStringValFromInfo(pData, 70, 42, m_sInfo5.callSign);//@@@@表示不可用，应该去除
    GetStringValFromInfo(pData, 112, 120, m_sInfo5.shipName);

    m_sInfo5.shipType = GetByteValFromInfo(pData, 232, 8);

    m_sInfo5.refOfPosition.dimA = GetWordValFromInfo(pData, 240, 9);
    m_sInfo5.refOfPosition.dimB = GetWordValFromInfo(pData, 249, 9);
    m_sInfo5.refOfPosition.dimC = GetByteValFromInfo(pData, 258, 6);
    m_sInfo5.refOfPosition.dimD = GetByteValFromInfo(pData, 264, 6);

    const int epfdValue = GetByteValFromInfo(pData, 270, 4);
    if  ((epfdValue >= 0)&&(epfdValue <= 8))
         m_sInfo5.epfd = (EPFD)epfdValue;
    else    m_sInfo5.epfd = UnUsedEp;

    //m_sInfo5.eta.month =  GetByteValFromInfo(pData, 290, 4);
    //m_sInfo5.eta.day =  GetByteValFromInfo(pData, 285, 5);
    //m_sInfo5.eta.hour =  GetByteValFromInfo(pData, 280, 5);
    //m_sInfo5.eta.minute =  GetByteValFromInfo(pData, 274, 6);
    m_sInfo5.eta.month =  GetByteValFromInfo(pData, 274, 4);
    m_sInfo5.eta.day =  GetByteValFromInfo(pData, 278, 5);
    m_sInfo5.eta.hour =  GetByteValFromInfo(pData, 283, 5);
    m_sInfo5.eta.minute =  GetByteValFromInfo(pData, 288, 6);

    m_sInfo5.maxDraught = GetByteValFromInfo(pData, 294, 8)/10.0;
    GetStringValFromInfo(pData, 302, 120, m_sInfo5.Destination);
    m_sInfo5.dte = GetByteValFromInfo(pData, 422, 1);

    return true;
}

/* 解析AIS消息类型18格式数据  */
/*
|==============================================================================
|Field   |Len |Description        |Member    |Units
|0-5     | 6  |Message Type       |type      |Unsigned integer: 18
|6-7     | 2  |Repeat Indicator   |repeat    |As in Common Navigation Block
|8-37    |30  |MMSI               |mmsi      |Unsigned integer: 9 digits
|38-45   | 8  |Regional Reserved  |reserved
|
|46-55   |10  |Speed Over Ground  |speed     |As in common navigation block
|56-56   | 1  |Position Accuracy  |accuracy  |See below
|57-84   |28  |Longitude          |lon       |Minutes/10000 (as in CNB)
|85-111  |27  |Latitude           |lat       |Minutes/10000 (as in CNB)
|112-123 |12  |Course Over Ground |course    |Relative to true north to 0.1 degree
|124-132 | 9  |True Heading       |heading   |0 to 359 degrees, 511 = N/A
|133-138 | 6  |Time Stamp         |second    |Second of UTC timestamp.
|139-140 | 2  |Regional reserved  |regional
|
|141-141 | 1  |CS Unit            |cs        |0=Class B SOTDMA unit
|        |    |                   |          |1=Class B CS (Carrier Sense) unit
|142-142 | 1  |Display flag       |display   |0=No visual display, 1=Has display
|        |    |                   |          |(Probably not reliable.)
|143-143 | 1  |DSC Flag           |dsc       |If 1, unit is attached to a VHF
|        |    |                   |          |voice radio with DSC capability.
|144-144 | 1  |Band flag          |band      |Base stations can command units
|        |    |                   |          |to switch frequency.  If this flag
|        |    |                   |          |is 1, the unit can use any part
|        |    |                   |          |of the marine channel.
|145-145 | 1  |Message 22 flag    |msg22     |If 1, unit can accept a channel
|        |    |                   |          |assignment via Message Type 22.
|146-146 | 1  |Assigned           |assigned  |Assigned-mode flag
|147-147 | 1  |RAIM flag          |raim      |As for common navigation block
|148-167 |20  |Radio status       |radio     |See <<IALA>> for details.
|==============================================================================
*/
bool Ais::GetAisInfo18(quint8 *pData)
{
        cleanDynamicInfo();
        m_sInfo1.msgType = (AisMsgType)GetByteValFromInfo(pData, 0, 6);
        m_sInfo1.repeatIndicator = (RepeatIndicator)GetByteValFromInfo(pData, 6, 2);
        m_sInfo1.userID = GetDwordValFromInfo(pData, 8, 30);
        if(m_sInfo1.userID == 0)
                return false;
        //|38-45   | 8  |Regional Reserved  |reserved
        m_sInfo1.sog = GetWordValFromInfo(pData, 46, 10)/10.0;
        m_sInfo1.positionAccuracy = GetByteValFromInfo(pData, 56, 1);
        m_sInfo1.longitude = GetDwordValFromInfo(pData, 57, 28)/(10000*60.0);
        m_sInfo1.latitude = GetDwordValFromInfo(pData, 85, 27)/(10000*60.0);
        m_sInfo1.cog = GetWordValFromInfo(pData, 112, 12)/10.0;
        m_sInfo1.trueHeading = GetWordValFromInfo(pData, 124, 9);
        m_sInfo1.timeStamp = GetByteValFromInfo(pData, 133, 6);

        m_sInfo1.naviStatus = Default;
        m_sInfo1.rateOfTurn = 0;
        m_sInfo1.manIndicator = 0;//GetByteValFromInfo(pData, 143, 2);
        m_sInfo1.raimFlag = 0;//GetByteValFromInfo(pData, 148, 1);
        m_sInfo1.commState = 0;//GetDwordValFromInfo(pData, 149, 19);
        return true;
}

/*
|==============================================================================
|Field   |Len |Description            | Member         | Units
|0-5     |  6 | Message Type          | type           | Unsigned integer: 19
|6-7     |  2 | Repeat Indicator      | repeat         | As in CNB
|8-37    | 30 | MMSI                  | mmsi           | Unsigned integer: 9 digits
|38-45   |  8 | Area Reserved         | reserve        | Unsigned integer: 0-1
|46-55   | 10 | speed                 | speed          | (Part A) 20 six-bit chars
|56      |  1 | 精确度                 |                | (Part A) Not used
|57-84   | 28 | 经度                   | shiptype       | (Part B) As in Message Type 5
|85-111  | 27 | 纬度                   | vendorid       | (Part B) 7 six-bit chars
|112-123 | 12 | 对地航向                | callsign       | (Part B) As in Message Type 5
|124-132 |  9 | 真航向                  | to_bow         | (Part B) Unsigned int: Meters
|133-138 |  6 | 时间标记                | to_stern       | (Part B) Unsigned int: Meters
|139-258 |120 | 船名                    | to_port        | (Part B) Unsigned int: Meters
|259-266 |  8 | 船舶及载货类型            | to_starboard   | (Part B) Unsigned int: Meters
|267-296 | 30 | 船舶尺度/位置参考         | mothership_mmsi| (Part B) See below
|297-300 |  4 | 电子定位装置类型          |                | (Part B) Not used
|301     |  1 | RAIM标志                |                | (Part B) Not used
|302     |  1 | 数据终端                 |                | (Part B) Not used
|303-307 |  5 | 备用位                  |                | (Part B) Not used
|===============================================================================
*/

bool Ais::GetAisInfo19(quint8 *pData)
{
    cleanStaticInfo();
    m_sInfo5.msgType = (AisMsgType)GetByteValFromInfo(pData, 0, 6);
    m_sInfo5.repeatIndicator = (RepeatIndicator)GetByteValFromInfo(pData, 6, 2);

    m_sInfo5.userID = GetDwordValFromInfo(pData, 8, 30);
        if(m_sInfo1.userID == 0)
                return false;


    GetStringValFromInfo(pData, 143, 120, m_sInfo5.shipName);

    m_sInfo5.shipType = GetByteValFromInfo(pData, 263, 8);

    m_sInfo5.refOfPosition.dimA = GetWordValFromInfo(pData, 271, 9);
    m_sInfo5.refOfPosition.dimB = GetWordValFromInfo(pData, 280, 9);
    m_sInfo5.refOfPosition.dimC = GetByteValFromInfo(pData, 289, 6);
    m_sInfo5.refOfPosition.dimD = GetByteValFromInfo(pData, 295, 6);

    const int epfdValue = GetByteValFromInfo(pData, 301, 4);
    if  ((epfdValue >= 0)&&(epfdValue <= 8))
         m_sInfo5.epfd = (EPFD)epfdValue;
    else    m_sInfo5.epfd = UnUsedEp;


    return true;

}

/*
|==============================================================================
|Field   |Len |Description            | Member         | Units
|0-5     |  6 | Message Type          | type           | Unsigned integer: 19
|6-7     |  2 | Repeat Indicator      | repeat         | As in CNB
|8-37    | 30 | MMSI                  | mmsi           | Unsigned integer: 9 digits
|38-39   |  2 | Part Number           | partno         | Unsigned integer: 0-1
|40-159  |120 | Vessel Name           | shipname       | (Part A) 20 six-bit chars
|160-167 |  8 | Spare                 |                | (Part A) Not used
|40-47   |  8 | Ship Type             | shiptype       | (Part B) As in Message Type 5
|48-89   | 42 | Vendor ID             | vendorid       | (Part B) 7 six-bit chars
|90-131  | 42 | Call Sign             | callsign       | (Part B) As in Message Type 5
|132-140 |  9 | Dimension to Bow      | to_bow         | (Part B) Unsigned int: Meters
|141-149 |  9 | Dimension to Stern    | to_stern       | (Part B) Unsigned int: Meters
|150-155 |  6 | Dimension to Port     | to_port        | (Part B) Unsigned int: Meters
|156-161 |  6 | Dimension to Starboard| to_starboard   | (Part B) Unsigned int: Meters
|132-161 | 30 | Mothership MMSI       | mothership_mmsi| (Part B) See below
|162-167 |  6 | Spare                 |                | (Part B) Not used
|===============================================================================
*/
bool Ais::GetAisInfo24(quint8 *pData)
{
        if (m_aisClass !=2)//需要区分A/B部分报文，保留上次解析的数据
                cleanStaticInfo();
    m_sInfo5.msgType = (AisMsgType)GetByteValFromInfo(pData, 0, 6);
    m_sInfo5.repeatIndicator = (RepeatIndicator)GetByteValFromInfo(pData, 6, 2);

    m_sInfo5.userID = GetDwordValFromInfo(pData, 8, 30);
        if(m_sInfo5.userID == 0)
                return false;

    quint8 partno = GetByteValFromInfo(pData, 38, 2);
        if(partno == 0)
        {	// part A
                GetStringValFromInfo(pData, 40, 120, m_sInfo5.shipName);//8 not used
        }
        else if(partno == 1)
        {	// part B
                m_sInfo5.shipType = GetByteValFromInfo(pData, 40, 8);//Vendor ID 42
                GetStringValFromInfo(pData, 90, 42, m_sInfo5.callSign);
                m_sInfo5.refOfPosition.dimA = GetWordValFromInfo(pData, 132, 9);
                m_sInfo5.refOfPosition.dimB = GetWordValFromInfo(pData, 141, 9);
                m_sInfo5.refOfPosition.dimC = GetByteValFromInfo(pData, 150, 6);
                m_sInfo5.refOfPosition.dimD = GetByteValFromInfo(pData, 156, 6);
        }
    return true;
}

bool Ais::cleanDynamicInfo()
{	//设置为不可用值
        memset(&m_sInfo1, 0x00, sizeof(SHIPDYNAMICINFO));
        m_sInfo1.repeatIndicator = UnUsedRi;
        m_sInfo1.naviStatus = UnUsedNs;
        m_sInfo1.rateOfTurn = -128;
        m_sInfo1.sog = 102.3;
        m_sInfo1.positionAccuracy = 2;//0,1, 2 mean unused
        m_sInfo1.longitude = 181.0;
        m_sInfo1.latitude = 91.0;
        m_sInfo1.cog = 360.1;//3601-4095 unused
        m_sInfo1.trueHeading = 511;
        m_sInfo1.userID = 0;
        return TRUE;
}

bool Ais::cleanStaticInfo()
{	//设置为不可用值
        memset(&m_sInfo5, 0x00, sizeof(SHIPSTATICINFO));
        m_sInfo5.repeatIndicator = UnUsedRi;
        m_sInfo5.versionAis = 4;//38-39 2 0-3, 4 means unused
        m_sInfo5.imoNumber = 0;
        //memcpy(&m_sInfo5.callSign, "----", 4);
        m_sInfo5.callSign[0] = 0;
        //memcpy(&m_sInfo5.shipName, "----", 4);
        m_sInfo5.shipName[0] = 0;
        m_sInfo5.shipType = 0;
        m_sInfo5.epfd = UnUsedEp;
        m_sInfo5.eta.hour = 24;
        m_sInfo5.eta.minute = 60;
        m_sInfo5.eta.day = 0;
        m_sInfo5.eta.month = 0;
        //memcpy(&m_sInfo5.Destination, "----", 4);
        m_sInfo5.Destination[0] = 0;
        m_sInfo5.dte = 1;
        m_sInfo5.maxDraught = 0;
        m_sInfo5.userID = 0;
        return TRUE;
}

/*单个字符解码，将8bit表示的字符解码成6bit的二进制序列*/
bool Ais::EightByteToSix(quint8 inEight,quint8& outSix)
{
    //以下两个判断用于检测所输入的ASCII码是否有效
    if(inEight < 0x30 || inEight > 0x77)
        return false;
    if(inEight > 0x57 && inEight < 0x60)
        return false;
    //检查结束
    outSix = inEight + 0x28;    //加上101000
    if(outSix > 0x80)          //如果SUM>10000000
        outSix += 0x20;       //加上100000
    else
        outSix += 0x28;       //加上101000
    outSix = outSix<<2;       //右移两位，获取LSB，取高六位
    return true ;
}

/*解码，将整个8bit表示的字符串解码成6bit的二进制序列*/
bool Ais::EightStrToSix(quint8* inEight, quint8* outSix, quint16 length)
{
    quint8* pin = inEight;

    quint8 nowBt = 0x00;//用于记录当前未用完的记录6Bits码的字节
    quint8 midBt;//中间转换记录字节
    quint8 six;
    quint8 outLen = 0;
    quint16 i = 0;

    //for(int i=0;(*(pin+i)!=0x0d&&*(pin+i+1)!=0x0a);i++)//处理所有ASCII码 0A=LF,0D=CR
    for(i=0; i<length; ++i, ++pin)
    {
        quint8 bt = *pin;
        if(EightByteToSix(bt,six)==false)//将当前ASCII码转换成6 bits码
        {
            return false;
        }

        int res = i%4;//因为4个ASCII码转换成3个字节的6bits码，因此将是每4个ASCII码的转换成为一个循环过程
        switch(res)
        {
            //当是第一个ASCII码时，不能直接完成一个6bits码的字节转换，因为还有两个bits没有填入数据。用nowBt暂先保存6bits码正在记录数据的字节
        case 0:
            nowBt = six;
            break;
            //当处理第二个ASCII码时，显然，加上第一个ASCII码的转换，2个6bits码将是12bits，因此可以完成一个字节的数据，另外余下的4bits记录到nowBt中，等待下一个ASCII码的处理。
        case 1:
            midBt = six >>6; //将最高的两位移到末尾，以便将高二位保存到nowBt的低二位中
            nowBt = nowBt | midBt;//完成6bits码的第一个字节
            outSix[outLen] = nowBt;//保存到输出的字节数组中
            outLen ++;
            //以下两步移位是将当前6bits码的中间4位移动到高四位中。并记录到nowBt中。
            nowBt = six >>2;
            nowBt = nowBt <<4;
            break;
            //当处理第三个ASCII码时，nowBt中的有效位是高四位，因此需要将新的6bits码的高四位放到nowBt的低四位中，然后保存nowBt到输出数组，再将新的6bits码的第5，6位移到高二位后记录到nowBt中
        case 2:
            midBt = six >>4;//将高四位移到低四位
            nowBt = nowBt | midBt;//完成6bits码的第二个字节
            outSix[outLen] = nowBt;//保存到输出的字节数组
            outLen ++;
            //将新的6bits码的第5，6位移到高二位后记录到nowBt中
            nowBt = six >>2;
            nowBt = nowBt <<6;
            break;
            //当处理第四个ASCII码时，nowBt中的有效位是高二位，因此将新的6bits码的高6位移到nowBt的低6位，正好完成一个循环
        case 3:
            midBt = six >>2;//将高六位移到低六位
            nowBt = nowBt | midBt;//完成6bits码的第三个字节
            outSix[outLen] = nowBt;//保存到输出的字节数组
            outLen ++;
            nowBt = 0x00;//nowBt复位
            break;
        }
    }

        // 如果不是第一个ASCII码，则保留前面的。
        if(i%4 != 0)
                outSix[outLen] = nowBt;

    return true;
}


/*获取1bits 到 8bits的整数数据
lpInfo—调用EightStrToSix获取的解码数据
dwBitStart –需要获取的数据的起始位. dwBitStart 从0开始
byLen –数据所占位数*/
quint8 Ais::GetByteValFromInfo(quint8* lpInfo,quint32 dwBitStart,quint8 byLen)
{
    quint32 byStartByte = dwBitStart/8;//获取起始字节序号
    quint8 byStartBit = dwBitStart%8;//获取起始字节中的起始位数
    quint8 byInfo = 0x00;
    if(8-byStartBit < byLen)//要获取的数据跨两个字节
    {
          quint8 by1 = *(lpInfo + byStartByte);//获取第一个字节数据
          quint8 by2 = *(lpInfo + byStartByte + 1);//获取第二个字节数据
          //完成两个字节中位的拼接
          by1 = (by1 << byStartBit);
          by1 = (by1 >> (8 - byLen));   //byLen - (8 - byStartBit)
          by2 = (by2 >> (16 - byLen - byStartBit)); //8 - (byLen - (8 - byStartBit))
          byInfo = (by1 | by2);
   }
   else
   {
          byInfo = *(lpInfo + byStartByte);
          byInfo = (byInfo << byStartBit);
          byInfo = (byInfo >> (8 - byLen));  //左移右移将高位置0
   }
   return byInfo;
}


/*获取9bits 到 16bits的整数数据
lpInfo—调用EightStrToSix获取的解码数据
dwBitStart –需要获取的数据的起始位. dwBitStart 从0开始
wyLen –数据所占位数*/
quint16 Ais::GetWordValFromInfo(quint8* lpInfo,quint32 dwBitStart,quint8 wLen)
{

    quint32 byStartByte = dwBitStart/8;
    quint8 byStartBit = dwBitStart%8;
    quint16 wInfo = 0x0000;
    if(16 - byStartBit < wLen)//要获取的数据跨三个字节
       {
              wInfo = *(quint16*)(lpInfo + byStartByte);
              wInfo = (wInfo << byStartBit);
              wInfo = (wInfo >> (16 - wLen));
              quint8 by1 = *(lpInfo + byStartByte + 2);
              by1 = (by1 >> (24- wLen - byStartBit));
              wInfo = wInfo | by1;
       }
       else
       {
              wInfo = *(quint16*)(lpInfo + byStartByte);
              wInfo = ntohs(wInfo);
              wInfo = (wInfo << byStartBit);
              wInfo = (wInfo >> (16 - wLen));
       }
       return wInfo;

}

/*获取17bits 到 32bits的整数数据
lpInfo—调用EightStrToSix获取的解码数据
dwBitStart –需要获取的数据的起始位. dwBitStart 从0开始
byLen –数据所占位数*/
quint32 Ais::GetDwordValFromInfo(quint8* lpInfo,quint32 dwBitStart,quint8 dwLen)
{
       quint32 byStartByte = dwBitStart/8;
       quint8 byStartBit = dwBitStart%8;
       quint32 dwInfo = 0x00000000;
       if(dwLen <= 24)//占三个或者四个字节
       {
              if(24 - byStartBit < dwLen)//占四字节
              {
                     dwInfo = *(quint32*)(lpInfo + byStartByte);
                     dwInfo = ntohl(dwInfo);
                     dwInfo = (dwInfo << byStartBit);
                     dwInfo = (dwInfo >> (32 - dwLen));
              }
              else //占三字节
              {
                     quint32 dwInfo = 0x00FFFFFF;
                     dwInfo = dwInfo & *(quint32*)(lpInfo + byStartByte - 1);
                     dwInfo = ntohl(dwInfo);
                     dwInfo = (dwInfo << byStartBit);
                     dwInfo = (dwInfo >> (24 - dwLen));
              }
       }
       else//占四个或者五个字节
       {
              if( (32 - byStartBit) < dwLen)//占五个字节
              {
                     dwInfo = *(quint32*)(lpInfo + byStartByte);
                     dwInfo = ntohl(dwInfo);
                     dwInfo = (dwInfo << byStartBit);
                     dwInfo = (dwInfo >> (32 - dwLen));
                     quint8 by1 = *(lpInfo + byStartByte + 4);
                     by1 = (by1 >> (40 - dwLen - byStartBit));
                     dwInfo = dwInfo | by1;
              }
              else//占四个字节
              {
                     dwInfo = *(quint32*)(lpInfo + byStartByte);
                     dwInfo = ntohl(dwInfo);
                     dwInfo = (dwInfo << byStartBit);
                     dwInfo = (dwInfo >> (32 - dwLen));
              }
       }
       return dwInfo;

}

/*获取字符串类型数据
lpInfo—调用EightStrToSix获取的解码数据
dwBitStart –需要获取的数据的起始位. dwBitStart 从0开始
dwLen –数据所占位数*/
quint8* Ais::GetStringValFromInfo(quint8* lpInfo, quint32 dwBitStart, quint32 dwLen, quint8* sRtnStr)
{
       quint32 wChars = dwLen/6;
       quint32 byStartByte = dwBitStart/8;
       quint8 byStartBit = dwBitStart%8;
       //QString
       memset(sRtnStr, 0x00, wChars);
       for(int wChar = 0; wChar < wChars; wChar++)
       {
              quint8 byChar = *(lpInfo + byStartByte);
              byChar = (byChar << byStartBit);
              byChar = (byChar >> 2);
              if(byStartBit > 2)//表示该数据跨字节了
              {
                     quint8 by1 = *(lpInfo + byStartByte + 1);
                     by1 = (by1 >> (16 - 6 - byStartBit));
                     byChar = (byChar | by1);
                     byStartByte ++;
                     byStartBit = byStartBit - 2;
              }
              else
              {
                     byStartBit += 6;
                     if(byStartBit == 8)
                     {
                            byStartBit = 0;
                            byStartByte ++;
                     }
              }
              //字符串解码，如果小于0x20，需要加上0x40
              //具体规则参考ITU-R M-1371-1文件第41，42页
              if(byChar < 0x20)
                 byChar += 0x40;
              sRtnStr[wChar]= byChar;
              if(byChar == '@')
              {
                 sRtnStr[wChar] = 0;
                 break;
             }
       }
       return sRtnStr;

}



Ais::~Ais()
{

}





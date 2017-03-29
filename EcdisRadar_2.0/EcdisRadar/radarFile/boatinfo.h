#ifndef BOATINFO_H
#define BOATINFO_H

#include <QString>
#include <QPointF>
#include <QDateTime>
#include <QDebug>

#include "define.h"

/*获取字符文本*/
extern QString upmodeDisplayText(quint8 index);
extern QString movingDisplayText(quint8 index);
extern QString offsetDisplayText(quint8 index);
extern QString unitsDisplayText(quint8 index);

/*向上方位*/
enum {
    N_UP = 0,   // 真北向上
    H_UP,       // 舰艏向上
    C_UP,       // 航向向上
    MAXUPMODE
};

/*运动方式*/
enum {
    MOVE_TM = 0,    // 真运动
    MOVE_RM,        // 相对运动
    MAXMOVE
};

/*偏心模式*/
enum {
    OFFSET_0 = 0,   // 无偏心
    OFFSET_1,       // 1/3偏心
    OFFSET_2,       // 2/3偏心
    MAXOFFSET
};

//距离制式
enum {
    UNITS_KM = 0,  //公里
    UNITS_NM,     //海里
    UNITS_SM,    //英里
    UNITS_MAX
};


//需要更新的状态标志,使用这种方式更容易让人理解程序？
typedef struct INEEDFLAG
{
    union {
        struct {
            quint32 toUpdateDateTIme : 1;     //更新时间显示
            quint32 toUpdateSystemInfo : 1;   //更新系统信息
            quint32 toUpdateMousePosition : 1;  //更新鼠标位置
            quint32 toUpdateTerminalState : 1;  //更新各终端状态
            quint32 toUpdateDispCtrl : 1;  //更新显示控制状态
            quint32 reserved : 27;  //保留
        };
        quint32 toUpdateValue;
    };
}INEEDFLAG;


//视图显示控制参数
typedef struct tagViewDisplay
{
    quint16 upmode : 2;  //向上方式
    quint16 offset : 2;  //偏心方式
    quint16 moving : 1;  //运动方式:TM/RM
    quint16 rngring : 1;  //距标圈显示标志
    quint16 guildline : 1;  //指引线显示标志
    quint16 dispmode : 2;   //显示模式：白天/黄昏/黑夜
    quint16 reserve : 9;  //保留
}VIEWDISPLAY;


//VRM/EBL控制
typedef struct tagVrmEbl
{
    float vrm;
    float ebl;  //绝对方位（正北）
    QPointF offsetPos;
    quint8 offset : 1;
    quint8 show : 2;
    quint8 reserve : 5;

    //对变量进行初始化,构造函数
    tagVrmEbl():offsetPos(0,0)
    {
        vrm = ebl = offset = show = 0;
    }
}VRMEBL;
typedef struct tagVrmEblCtrl
{
    VRMEBL vrmebl[2];
    quint8 eblReference;  //ebl方位参考，0：真方位（相对真北） 1：相对方位(相对舰首)
}VRMEBLCTRL;

// 船相关信息
typedef struct tagShipInfo
{
    QPointF   position; // 船位置
    float     head;     // 船艏向
    float     course;   // 船航向
    float     speed;    // 船速度
    float     sog;     // 对地速度
    float     vcourse[3]; //记录每种来源的航向
    float     vspeed[3];
}SHIPINFO;


//碰撞告警控制
typedef struct tagCollisionAlarm
{
    quint8   Range;     // 告警距离(单位公里)
    quint8   Enable:1;  // 告警使能
    quint8   VoiceAlarm:1;  // 语音告警使能
    quint8   Flicker:1;     // 目标闪烁使能
    quint8   ShowRing:1;    // 显示警告圈
    quint8   Reserve:4;
}COLLISIONALARM;



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
    QString unitsText() const
    {   return unitsDisplayText(units);   }

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


//系统信息
typedef struct SYSTEMINFO
{
    INEEDFLAG iNeedFlag;   //更新标志
    quint8 ScanMode;   //显示模式
    int DelayRange;
    QDateTime currentDateTime;  //当前日期时间
    float rotation;    //转速
    quint16 plotCount;  //点迹数量
    quint16 trackCount;  //航迹数量

    quint8 fullauto;  //0:半自动  1:全自动

    VIEWDISPLAY ViewDisplay;  //视图显示控制
    VRMEBLCTRL VrmEblCtrl;  //VRM/EBL显示控制
    RANGESCALE RangeScale;  //量程设置
    SHIPINFO    ShipInfo;       // 船相关信息
    COLLISIONALARM  CollisionAlarm; // 碰撞告警控制


    quint32 gps_time;
    quint32 crnt_time;  //系统时间
    quint8 showShipName : 1;  //显示船名
    quint8 chineseName : 1;  //中文船名
    quint8 reserved : 6;

    AIS_DYNAMIC_INFO gps_info;
    int gps_timeAdjust;
}SYSTEMINFO;


/************************操作界面的配置设置**********************************/

// 主要显示菜单配置
typedef struct tagDispMenu
{
    quint8 upmode : 4;  // 向上模式
    quint8 motion : 4;
    quint8 dayNight : 4;
    quint8 offset   : 4;
    quint8 echoTrail:4;
    quint8 vecterLength : 4;
    quint8 trackTime : 4;
    quint8 symbolSize : 4;
    quint8 colorSelect : 4;
    quint8 showAis : 1;
    quint8 showArpa : 1;
    quint8 showHeadLine : 1;
    quint8 showRngRing : 1;
    quint8 screenBright;
    quint8 kbdBright;
    quint8 fixlineBright;
    quint8 varlineBright;
    quint8 reserved[4];
}DISPMENU, *LPDISPMENU;

// 安装菜单配置
typedef struct tagInstallMenu
{
    quint8 dataOut;
    quint8 radarSelect;
    quint8 langSelect;
    quint32 dateTime;
    quint32 timeUsed;
    quint32 timeTran;
    float  aziAdjust;
    float  rngAdjust;
    quint8 mbsAdjust;
    quint8  tuneMan;
    quint16  tuneValue;
    quint32 rangeChecked;
    quint32 bestTuneValue[19];  //调谐的最佳值
    quint16 bestTuneAdd[19];   //增加的值
    char producer[15];  //cpu生产商信息
    char serialId[20];  //cpu序列号
    quint8 bandSelect;  //绑定选择按钮
    quint8 antennaSelect;  //每次开机都要发送
    quint8 firstSatrt;  //第一次启动标志
    quint8 reserved[4];
}INSTALLMENU, * LPINSTALLMENU;

// 功能菜单配置
typedef struct tagOtherMenu
{
    quint8 corseSelect : 4;
    quint8 speedSelect : 4;
    quint8 samefreqJam : 4;
    quint8 transmite : 4;
    quint8 echoExpand;
    quint8 vrmeblEnable; // bit0:vrm1, bit1:vrm2, bit2:ebl1, bit3:ebl2
    quint8 plotLimit;
    qint8 timeAdjust;
    float manCorse;
    float manSpeed;
    float vrmebl[4];
    quint8 audibleWarningEnable : 1;
    quint8 cpaEnable : 1;
    quint8 dcpaEnable : 1;
    quint8 tcpaEnable : 1;
    quint8 guardZoneEnable : 1;
    quint8 guardSelect : 1;
    quint8 reserved1 : 2;
    quint8 tcpaValue;
    quint8 tuneMan;
    quint8 tuneAuto;
    quint16 tuneValue;
    float cpaValue;
    float dcpaValue;
    float guardZoneValue[4];	// 0:起始距离 1:起始方位 2:结束距离 3:结束方位
    quint8 reserved2[4];
}OTHERMENU, * LPOTHERMENU;

//串口配置菜单配置
typedef struct tagSerialConfigMenu
{
    quint8 SerialConfig : 4;
    char devicename;
    int bautrate;
    int databit;
    int stopbit;
    int parity;
    quint8 reserved[4];
}SERIALCONFIGMENU,*LPSERIALCONFIG;


// 系统菜单配置
typedef struct tagMenuConfig
{
    DISPMENU    dispMenu;
    INSTALLMENU installMenu;
    OTHERMENU   otherMenu;
    SERIALCONFIGMENU SerialConfigMenu;

    tagMenuConfig()
    {
        memset(&dispMenu, 0, sizeof(dispMenu));
        memset(&installMenu, 0, sizeof(installMenu));
        memset(&otherMenu, 0, sizeof(otherMenu));
        memset(&SerialConfigMenu, 0, sizeof(SerialConfigMenu));
    }
}MENUCONFIG, *LPMENUCONFIG;


#endif // BOATINFO_H

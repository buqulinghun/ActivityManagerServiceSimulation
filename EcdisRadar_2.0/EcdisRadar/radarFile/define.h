#ifndef DEFINE_H
#define DEFINE_H

#include <QString>
#include <QDebug>
#include <QProgressBar>

#include "TargetManage/track.h"



#define DPRINT(str) qDebug() << __FILE__ << __FUNCTION__ << str;



#ifndef EQUAL3
#define EQUAL3(a, b, pre) (fabs((a)-(b))<(pre))
#endif

#define IsDataValid0(var, vmin, vmax) ((var)>(vmin) && (var)<(vmax))
#define IsDataValid1(var, vmin, vmax) ((var)>=(vmin) && (var)<(vmax))


#define REALMOVING 0
#define RELATIVEMOVING 1


#define CURRENTPATH QString("/home/Ecdis/ECDISRADAR/EcdisRadar/radarFile")






// AIS最长消息间隔时间
#define AIS_HALT_INTERVAL   300            // AIS消息中断的时间为5分钟(300秒)
#define AIS_MISS_INTERVAL   600   // AIS船失踪的时间为10分钟(600秒)



// 判断方位是否有效
#define IsAzimuthValid(azi) IsDataValid1(azi, 0.0, 360.0)


typedef void* S52_objHandle;

//定义各个设备的ID
enum {
    Device_NULL = 0,
    Device_FPGA,   //FPGA
    Device_KDB,   //键盘串口
    Device_GPS,   //GPS串口
    Device_COMP,  //罗经串口
    Device_AIS,   //AIS串口
    Device_LOG,   //计程仪串口

};

extern QString Longitude2String(double lon, bool enflag);
extern QString Latitude2String(double lat, bool enflag);


//回波数据结构
typedef struct tagEchoData
{
    quint16 range;  //量程
    quint16 angle;   //角度
    quint16 length;   //真实回波数据长度
    quint16 packetNum;  //包数标志
    quint8 echo[512];   //真实数据

}ECHODATA;



// 颜色配置
struct COLORCONFIG
{
        quint32 color_eblvrm[6];	// ebl vrm
        quint32 color_rngring[6];	// range ring
        quint32 color_headl[6];		// head line
        quint32 color_back[6];		// back color
        quint32 color_fore[6];		// fore color
        quint32 color_last[6];		// last color
        quint32 color_signalplot[6];	// signal plot color
        quint32 color_filterplot[6];	// filter plot color
        quint32 color_ata[6];		// ata color
        quint32 color_ais[6];		// ais color
};

// AIS动态信息
struct AIS_DYNAMIC_INFO
{
    unsigned char   naviStatus;             // 航行状态
    unsigned int    dateTime;               // 日期时间
    double          lon;                    // 位置经度（度）
    double          lat;                    // 位置纬度（度）
    double          heading;                // 艏向（度）
    double          course;                 // 航向（度）
    double          speed;                  // 航速（节）
    double	    sog;                  //对地速度
    quint8	    rateOfTurn;		   // ROT,转向率
};

// 船舶静态信息
struct SHIP_STATIC_INFO
{
    unsigned long   MMSI;                   // MMSI
    unsigned long   IMO;                    // IMO
    unsigned char   version;                // 版本号
    char            callCode[10];           // 呼号
    char            name[20];               // 船名
    char            destination[20];        // 目的地
    unsigned char   shipType;               // 船舶类型
    double          headRange;              // GPS距船艏（米）
    double          tailRange;              // GPS距船尾（米）
    double          leftRange;              // GPS距左舷（米）
    double          rightRange;             // GPS距右舷（米）
    double          maxDraught;             // 最大吃水深度（米）
    unsigned int    arriveTime;             // 预计到达时间

        SHIP_STATIC_INFO& operator=(const SHIP_STATIC_INFO& info)
        {
                MMSI = info.MMSI;
                IMO = info.IMO;
                version = info.version;
                shipType = info.shipType;
                headRange = info.headRange;
                tailRange = info.tailRange;
                leftRange = info.leftRange;
                rightRange = info.rightRange;
                maxDraught = info.maxDraught;
                arriveTime = info.arriveTime;

                if(strlen(info.callCode) > 0)
                        strncpy(callCode, info.callCode, 10);
                if(strlen(info.name) > 0)
                        strncpy(name, info.name, 20);
                if(strlen(info.destination) > 0)
                        strncpy(destination, info.destination, 20);

                return *this;
        }
};


// AIS导航信息
struct AIS_NAVIGATION_INFO
{
    double          range;           // 和本船距离（海里）
    double          azimuth;         // 和本船方位（度）
    double          DCPA;	  // 最小会遇距离(海里)
    unsigned int    TCPA;         // 最小会遇时间(分钟)
};

typedef struct tagPoint2 {
    double x, y;
}POINT2;

struct AisPoints {
    TRACKPOINT  crntPoint;
    TRACKPOINT  points[MAXTRACKPOINTS]; // 航迹历史点
    quint16  curPointPos;   // 当前点索引位置
    quint16  maxPoints;     // 最大历史点数(设置的最大尾点数量)
    quint16  curPoints;     // 当前历史点数(从航迹产生时开始计数，最大值为maxPoints)，控制尾点显示
    quint16  totalPoints;   // 总的历史点数(从航迹产生时开始计数，最大值为MAXTRACKPOINTS)

     AisPoints()
     {
        curPointPos = curPoints = totalPoints = 0;
        maxPoints = 6;
    }
    // 更新航迹当前点迹
    void enterPoint()
    {
        //调整点迹位置，如果当前点迹位置超过最大点迹数，位置回头
        //curPointPos = NextTrackPointIndex(curPointPos);
        //如果当前的点迹数比最大数要小，当前点迹数增加1
        if(curPoints < maxPoints)
            curPoints++;
        //如果点迹总的个数没有到达最大允许个数，总个数加1
        if(totalPoints < MAXTRACKPOINTS)
            totalPoints++;
    }
};

// AIS船舶信息
struct AIS_SHIP
{


    SHIP_STATIC_INFO    staticInfo;         // 静态信息
    AIS_DYNAMIC_INFO    dynamicInfo;        // 动态信息
    AIS_NAVIGATION_INFO navigationInfo;     // 导航信息
    AisPoints	points;
    S52_objHandle       vesselObj;          // 对应的S52对象
    unsigned int	updateTime;         // 更新时间
    unsigned char       aisClass;	   // AIS类别  0:无效，1:A类，2:B类
    unsigned char	msgStatic;          // 消息状态 0:正常 1:中断 2:消失
    unsigned char	boxIndex;
    unsigned char   flag;		// bit0:静态信息有效，bit1:动态信息有效, bit2:导航信息有效,bit3:S52对象有效

    QRect    screenRect;


    AIS_SHIP():points(),vesselObj(0),updateTime(0),msgStatic(0),flag(0)
    {
        memset(&staticInfo, 0, sizeof(staticInfo));
        memset(&dynamicInfo, 0, sizeof(dynamicInfo));
        boxIndex = 0;
    }

};


// 报警参数
struct ALARM_PARA
{
    unsigned char   shallowAlarm;           // 进入浅水区报警标志
    unsigned char   forbiddenSailAlarm;     // 进入禁航区报警标志
    unsigned char   specialAreaAlarm;       // 进入特殊水域报警标志
    unsigned char   dangerAlarm;            // 水下危险物预警标志
    unsigned char   noChartAlarm;           // 船位处无海图报警标志
    unsigned char   yawingAlarm;            // 偏航报警标志
    unsigned char   sailPointAlarm;         // 航点预警标志
    unsigned char   cpaAlarm;               // CPA报警标志
    unsigned char   dcpaAlarm;              // DCPA报警标志
    unsigned char   tcpaAlarm;              // TCPA报警标志
    unsigned char   anchorAlarm;            // 移锚报警

    unsigned char   shallowAlarmLevel;      // 进入浅水区报警等级
    unsigned char   forbiddenSailAlarmLevel;// 进入禁航区报警等级
    unsigned char   specialAreaAlarmLevel;  // 进入特殊水域报警等级
    unsigned char   dangerAlarmLevel;       // 水下危险物预警等级
    unsigned char   noChartAlarmLevel;      // 船位处无海图报警等级
    unsigned char   yawingAlarmLevel;       // 偏航报警等级
    unsigned char   sailPointAlarmLevel;    // 航点预警等级
    unsigned char   cpaAlarmLevel;          // CPA报警等级
    unsigned char   signalLostLevel;         // 信号丢失报警等级

    unsigned short  shallowAlarmTime;       // 进入浅水区报警提前时间(分钟)
    unsigned short  forbiddenSailAlarmTime; // 进入禁航区报警提前时间(分钟)
    unsigned short  specialAreaAlarmTime;   // 进入特殊水域报警提前时间(分钟)
    unsigned short  dangerAlarmTime;        // 水下危险物报警提前时间(分钟)
    unsigned short  noChartAlarmTime;       // 船位处无海图报警提前时间(分钟)
    double          yawingAlarmRange;       // 偏航报警提前距离(海里)
    double          sailPointAlarmRange;    // 航点预警提前距离(海里)
    double          cpaAlarmRange;          // CPA报警提前距离(海里)
    double		dcpaAlarmRange;			// DCPA
    unsigned char	tcpaAlarmTime;			// TCPA
    double		anchorAlarmRange;		// 移锚报警
    double		anchorLat, anchorLon;
};

// 系统参数
struct SYSTEM_PARA
{
    unsigned char   dispCategory;            // 海图显示内容
    unsigned char   backMode;                // 海图背景模式
    unsigned char   chartOrientation;        // 海图显示方向
    unsigned char   autoCenter;              // 本船自动居中标志
    unsigned char   dispCurRoute;            // 是否显示当前航线
    unsigned char   dispPastrk;              // 是否显示本船航迹
    unsigned char   recordSailTrail;         // 是否记录航行轨迹
    unsigned short  pastrkTime;              // 本船轨迹显示时间跨度(分钟)
    unsigned short  pastrkInterval;          // 本船轨迹船位时间间隔(分钟)
    unsigned short  ownshpRefreshInterval;   // 本船船位刷新时间间隔(秒)
    unsigned short  ownshpSaveInterval;      // 本船轨迹保存时间间隔(秒)
    unsigned char   sailRecordInterval;      // 航行记录保存时间间隔(分钟)
    unsigned char   ownshpSaveTime;          // 本船轨迹保存时间跨度(小时)
    unsigned char   gpsResource;			 // 定位源选择 0:外置GPS，1:内置GPS，2:AIS内GPS
    ALARM_PARA      alarmPara;               // 报警参数
    unsigned int    nextGeoId;
};



extern double g_ownshpLon;                     // 本船经度
extern double g_ownshpLat;                     // 本船纬度
extern double g_cLon;                          // 海图中心经度
extern double g_cLat;                          // 海图中心纬度


//////////////////////////////////////////////////////////////////
//定义各种创建标签显示的函数
/*********创建一个QGroupBox，其中设置n个layout,为水平layout ****/
#define CreateGroupBox(gboxout, layoutin, n)  \
{        \
    QBoxLayout *vlayout = new QVBoxLayout; \
    vlayout->setContentsMargins(0,0,0,0);  \
    vlayout->setSpacing(1);           \
    for(int i=0; i<n; i++)              \
        vlayout->addLayout(layoutin[i]); \
    gboxout = new QGroupBox(this);        \
    gboxout->setLayout(vlayout);           \
    gboxout->setAutoFillBackground(true);      \
    gboxout->clearFocus();                  \
}
/***********创建一个QLineEdit部件并设置*******/
#define CreateEdit(edit) {  \
    edit = new QLineEdit(this);  \
    edit->setReadOnly(true);  \
    edit->setFrame(true);     \
    edit->setContextMenuPolicy(Qt::NoContextMenu);   \
    edit->setObjectName("MainViewEdit");  \
    edit->installEventFilter(this);  \
}
/**********创建一个QProgressBar部件并设置*************/
#define CreateProgressbar(bar) {  \
    bar = new QProgressBar(this);  \
    bar->setFormat("%v"); /*只显示真实数字*/   \
    bar->setFixedHeight(20);  \
}

/**********创建一个QToolButton部件*******************/
#define CreateToolButton(button, name) { \
    button = new QToolButton(this);   \
    button->setText(name);   \
    button->setObjectName("MainViewButton");  \
    button->installEventFilter(this);   \
}
/**********创建一个QLabel部件*******************/
#define CreateLabel(name, label) {   \
    label = new QLabel(name,this);   \
    label->setAutoFillBackground(false);   \
    label->setObjectName("MainViewLabel");   \
}
/**********创建一个QHBoxLayout部件*******************/
#define CreateHBoxLayout(layout) {  \
    QBoxLayout *layouttmp = new QHBoxLayout;  \
    layout = layouttmp;   \
    layout->setContentsMargins(0,0,0,0);  \
    layout->setSpacing(1);  \
}
/**********创建一个QHBoxLayout,并添加一个widget部件*******************/
#define CreateHBoxLayout_v1(layout, widget1) { \
    CreateHBoxLayout(layout);  \
    layout->addWidget(widget1);   \
}
/**********创建一个QHBoxLayout,并添加二个widget部件*******************/
#define CreateHBoxLayout_v2(layout, widget1, widget2) { \
    CreateHBoxLayout(layout);  \
    layout->addWidget(widget1);   \
    layout->addWidget(widget2);   \
}
/**********创建一个QHBoxLayout,并添加三个widget部件*******************/
#define CreateHBoxLayout_v3(layout, widget1, widget2, widget3) { \
    CreateHBoxLayout(layout);  \
    layout->addWidget(widget1);   \
    layout->addWidget(widget2);   \
    layout->addWidget(widget3);    \
}
/**********创建一个QHBoxLayout,并添加label和LineEdit部件*******************/
#define CreateHLayout_v1(lout, name, edit, label) { \
    CreateLabel(name, label);  \
    label->setFixedSize(50,30);  \
    label->setAlignment(Qt::AlignCenter); \
    CreateEdit(edit);\
    CreateHBoxLayout_v2(lout, label, edit);\
}
/**********创建一个QHBoxLayout，并添加label和processbar部件***************/
#define CreateHLayout_p1(lout, name, bar, label) {  \
    CreateLabel(name, label);  \
    label->setFixedSize(50,30);   \
    label->setAlignment(Qt::AlignCenter);   \
    CreateProgressbar(bar);   \
    CreateHBoxLayout_v2(lout, label, bar);  \
}

/**********创建一个QHBoxLayout,并添加label和两个LineEdit部件**************/
#define CreateHLayout_v2(lout, name, edit1, edit2) { \
    QLabel *label; \
    CreateLabel(name, label);  \
    CreateEdit(edit1);\
    CreateEdit(edit2); \
    CreateHBoxLayout_v3(lout, label, edit1, edit2);\
}
/**********创建一个QHBoxLayout,并添加button和一个LineEdit部件**************/
#define CreateHLayout_v3(lout, name, button, edit) { \
    CreateToolButton(button, name); \
    CreateEdit(edit);  \
    CreateHBoxLayout_v2(lout, button, edit);  \
}
/**********创建一个QHBoxLayout,并添加两个button部件**************/
#define CreateHLayout_v4(lout, name1, button1, name2, button2) { \
    CreateToolButton(button1, name1); \
    CreateToolButton(button2, name2);  \
    CreateHBoxLayout_v2(lout, button1, button2);  \
}
//////////////////////////////////////////////////////////////////////////////////



//添加部件到链表

#define addObject(object) {  \
QPoint pos = this->mapToGlobal(object->pos());  \
QRect rect = QRect(pos, object->size());  \
addWidget(object, rect);  \
}
#define addObject2(object1, object2) {  \
addObject(object1);  \
addObject(object2);  \
}
#define addObject3(object1, object2, object3) {  \
addObject(object1);  \
addObject(object2);  \
addObject(object3); \
}
#define addObject6(object1, object2, object3, object4, object5, object6) { \
addObject3(object1, object2, object3);  \
addObject3(object4, object5, object6); \
}




#endif // DEFINE_H

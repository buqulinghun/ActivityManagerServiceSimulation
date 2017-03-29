#ifndef TRACK_H
#define TRACK_H

#include "TargetManage_global.h"

#include <QtCore/QHash>
#include <QtCore/QMultiMap>
#include <QtCore/QMutex>
#include <QtGui/QPainter>

class CRadarView;

#ifdef Q_OS_WIN32
#define _ATA_ 1
#else
#define _ATA_ 0
#endif

#define MAXTRACKS   9999
// 最大航迹历史点数
#define MAXTRACKPOINTS  10 //1001
#define MAXBOARDMODE    7
#define MANTRACKPOINTMODE 255

#define NextTrackPointIndex(i) ((i)<MAXTRACKPOINTS-1?(i)+1:0)
#define PrevTrackPointIndex(i) ((i)>0?(i)-1:MAXTRACKPOINTS-1)

typedef qint16 TRACKINDEX;
typedef qint16 TRACKNO;

// 常用航迹类型定义
enum {
    TRACK_TYPE_UNKNOWN=0,		// 未知类型航迹
    TRACK_TYPE_PRIMARY,			// 一次航迹
    TRACK_TYPE_SECONDARY,		// 二次航迹
    TRACK_TYPE_IIIREQUEST,		// III型询问机航迹
    TRACK_TYPE_PRIMARYSEC,		// 一二次配对航迹
    TRACK_TYPE_PRIMARYIII,		// 一次、III型配对航迹
    TRACK_TYPE_IIIPRIMARY,		// III型、一次配对航迹
    TRACK_TYPE_FILTER,			// 补点外推航迹
    TRACK_TYPE_USER1,
    TRACK_TYPE_USER2,
	TRACK_TYPE_USER3,
    MAX_TRACK_TYPE,
};

// 高度来源
enum {
    HSOURCE_UNKNOWN = 0,
    HSOURCE_PRIMARY,	// 一次高度
    HSOURCE_SECONDARY,	// 二次高度
    HSOURCE_IFF,		// 询问高度
    HSOURCE_USER,
};

//航迹跟踪类型
typedef struct{
    //航迹跟踪方式
    quint8 Mode:	1;  // 0:自动跟踪 1:人工跟踪
    //航迹控制方式
    quint8 Control:	1;  // 0:非控 1:受控
    //航迹跟踪状态
    quint8 Trace:	3;
    //航迹目标状态 0:一般 1:模拟 2:重要
    quint8 Target: 	3;
}TRACK_TRACE;

//航迹类型标志
typedef struct {
    quint8 value:7; // 数值
    quint8 flag:1;  // 有效标志
}VALUE8BIT;

// 航迹跟踪参数
typedef struct tagTrackFilterParam
{

}TRACKFILTERPARAM, *LPTRACKFILTERPARAM;

// 航迹点结构
typedef struct tagTrackPoint
{
    quint8  source;     // 点迹来源
    quint32 updateTime; // 更新时间
    quint16 height:13;  // 航迹高度(可用于高度分析(高度曲线))
    quint16 heightSrc:3;// 高度来源
    quint16 speed;      // 航迹速度(可用于机动分析(速度曲线和加速度曲线))    
    quint16	doplSpeed : 15; // 航迹多普勒速度
    quint16	doplSpeedFlag : 1;
    quint8	fadeing;		// 外推标志
    PLOTPOSITION position;  // 航迹位置    
    TRACKFILTERPARAM filter;     // 航迹跟踪参数

    // 额外的数据,应用时需要使用 new qint8[]申请内存,以保证析构函数中能够正确删除该内存
    qint8*   extraData;

    // 设置额外数据
    void setExtraData(qint8* _data)
    {
        if(extraData)
            delete[] extraData;
        extraData = _data;
    }

}TRACKPOINT, *LPTRACKPOINT;

// 标牌指引线显示控制
typedef struct tagCfgBoardDisp
{
    // 标牌显示内容索引
    enum {
        LINE_NULL = 0,
        LINE_TRACKNO,    // 批号
        LINE_HEIGHT,     // 高度
        LINE_SPEED,      // 速度
        LINE_DOPLSPEED,  // 多普勒速度
        LINE_COURSE,     // 航向
        LINE_ALIAS,      // 名称
        LINE_TYPE,       // 类型
        LINE_ATTRI,      // 属性
        LINE_QUANTITY,   // 数量
        LINE_SSRCODE,    // 机批（民航二次雷达目标代码）
        LINE_IFFCODE,    // IFF代码
        LINE_MODE1,      // 模式1代码
        LINE_MODE2,      // 模式2代码
        LINE_MODE3,      // 模式3代码
        LINE_USER1, LINE_USER2, LINE_USER3, LINE_USER4,LINE_USER5,
        LINE_MAXBOARD,
    };

    // 指引线准则说明：
    // 如果ManLine为1，则使用固定线长(ManLineLength)和固定方位(ManLineCourse)；
    // 否则，如果FixedLength为1，则线长由LineLength计算，否则由速度计算;
    // 如果FixedCourse为1，则方向由LineCourse计算，否则由航向计算。

    quint16	BoardMode  :3;		// 标牌显示模式：0:无 1:单标 2:标准 3:全标
    quint16	FixedCourse:1;		// 固定指引线方位 0:指引线方向由航向计算 1:使用固定指向
    quint16 FixedLength:1;      // 固定线长
    quint16	ManLine	   :1;		// 人工修改指引线标志
    quint16	ManLineCourse : 10;	// 人工指引线方向(单位为360/1024)
    quint16	ManLineLength;		// 人工指引线长度

    quint8	LineLength	: 3;	// 固定指引线长度 0-7级可调
    quint8	LineCourse	: 3;	// 指引线方向 8个方向
    quint8	FontSize	: 2;	// 字体大小 0:较小 1:正常 2:较大 3：很大
    quint8  ShowBorder   :1;   // 显示边框
    quint8  FillBackground:1;   // 填充背景标志
    quint8	FillPersent   :6;	// 背景透明度
    quint32	FillColor;			// 背景颜色
    quint32	BorderColor;		// 边框颜色

    // 指定标牌每行显示的内容(其值对应标牌显示内容索引)，标牌最多显示5行
    union {
        struct{
            quint32 Line1:5;    // 指定标牌第1行显示内容
            quint32 Line2:5;    // 指定标牌第2行显示内容
            quint32 Line3:5;    // 指定标牌第3行显示内容
            quint32 Line4:5;    // 指定标牌第4行显示内容
            quint32 Line5:5;    // 指定标牌第5行显示内容
            quint32 MaxLines:7;
        };
        quint32 LineValue;
    };

    tagCfgBoardDisp (quint8 mode=2)
    {
        BoardMode = mode;
        FixedCourse = 0;
        FixedLength = 0;

        ManLine = 0;
        ManLineCourse = 0;
        ManLineLength = 0;

        LineLength = 4;
        LineCourse = 0;
        FontSize = 1;

        ShowBorder = 0;
        FillBackground = 0;
        FillPersent = 10;
        FillColor = 0xffffcc;
        BorderColor = 0xccffff;

        LineValue = 0;
    }

    // 获取标牌某一行的显示内容索引值
    quint8 boardLineValue(quint8 line)
    {
        if(line < 5)
            return ((LineValue >> (line*5)) & 0x1f);
        else
            return 0;
    }

    // 设置标牌某一行的显示内容索引
    void setBoardLineValue(quint8 line, quint8 value)
    {
        if(line < 5)
        {
            LineValue &= (~(0x1f<<(line*5)));
            LineValue |= (value << (line*5));
        }
    }

    // 判断指定内容索引值是否在标牌内
    bool isBoardContainsLineValue(const quint8 linev)
    {
        if(Line1 == linev || Line2 == linev ||
           Line3 == linev || Line4 == linev ||
           Line5 == linev)
            return true;
        else
            return false;
    }

    // 自动旋转标牌
    void autoRotate()
    {
        if (ManLine)
        {
            ManLine = 0;
            LineCourse = ((ManLineCourse >> 7) & 0x7);	// 10位-->3位
        }
        FixedCourse = 1;
        LineCourse = (LineCourse+1) % 8;
    }

    // 设置为固定线长模式
    void setLineLength(quint8 length)
    {
        // 取消人工指引线标志
        if (ManLine)
        {
            ManLine = 0;
            LineCourse = ((ManLineCourse >> 7) & 0x7);	// 10位-->3位
            FixedCourse = 1;
        }
        FixedLength = 1;
        LineLength = length;
    }

    // 人工设置指引线, 长度和方向
    void manGuildLine (int length, int course)
    {
        ManLineCourse = course;
        ManLineLength = length;
        ManLine = 1;
        FixedCourse = 0;
        FixedLength = 0;
    }

}BOARDDISP, *LPBOARDDISP;

// 航迹尾点结构
typedef struct tagTrackPointMode
{
    quint8  mode;   // 模式索引
    QString name;   // 模式名称
    quint16 pointCount;// 尾点数量
}TRACKPOINTMODE;

// 二次航迹参数
typedef struct tagSecondaryTrack
{

}SECONDARYTRACK;

// IFF参数
typedef struct tagIFFTrack
{

}IFFTRACK;

// 组网参数
typedef struct tagNetworkTrack
{

}NETWORKTRACK;


typedef struct tagAtaParam
{
	ushort cpa;
	ushort dcpa;	// 1/100ni
	ushort tcpa;// min
	ushort relativeSpeed;	// 相对速度 1/100 kn
	ushort relativeCourse;	// 相对航向 1/10 度
	ushort trueSpeed;		// 真速度   1/100 kn
	ushort trueCourse;		// 真航向   1/10 度
	ushort relativeRng;		// 相对距离 1/100 mi
	ushort relativeAzi;		// 相对方位 1/10度

	ushort  predict_x, predict_y;

	union{
		struct{
	quint8 cpaFlag : 1;		// cpa/tcpa报警标志
	quint8 dcpaFlag : 1;		// cpa/tcpa报警标志
	quint8 tcpaFlag : 1;		// cpa/tcpa报警标志
	quint8 guardZoneFlag : 1;	// 区域报警标志
	quint8 targetLost : 1;		// 目标丢失标志
	quint8 targetInit : 1;		// 目标起始标志
	quint8 reserve   : 2;
		};
	quint8 flag;
	};
}ATAPARAM, * LPATAPARAM;

// 航迹结构
typedef struct tagTrack
{
    TRACKNO      no;    // 航迹批号
    TRACKINDEX   index; // 航迹索引号

	TRACKPOINT  crntPoint;
    TRACKPOINT  points[MAXTRACKPOINTS]; // 航迹历史点
    quint16  curPointPos;   // 当前点索引位置
    quint16  maxPoints;     // 最大历史点数(设置的最大尾点数量)
    quint16  curPoints;     // 当前历史点数(从航迹产生时开始计数，最大值为maxPoints)，控制尾点显示
    quint16  totalPoints;   // 总的历史点数(从航迹产生时开始计数，最大值为MAXTRACKPOINTS)
    quint8   hpointMode;    // 尾点模式

    quint8  source;     // 点迹来源
    float speed;      // 速度
    float course;     // 航向
    quint16 doplspeed;  // 多普勒速度

    //外推点
    RTHETA_POINT	extrapolatePoint;

    TRACK_TRACE trace; // 跟踪状态
    VALUE8BIT  type;   // 航迹类型
    VALUE8BIT  model;  // 航迹属性
    VALUE8BIT  number; // 航迹数量
    char	name[6];   // 航迹别名

    // 航迹控制标志
    union{
        struct{
            quint8  valid : 1;  // 有效标志
            quint8  updateBoardText:1;// 更新标牌字符串
            quint8  reserve:6;
        };
        quint8  flags;
    };

    // 标牌指引线显示控制
    BOARDDISP   boardDisp;
    // 标牌每行字符串
    QList<QString>  boardLineTexts;
    // 标牌显示区域
    QRect       boardRect[MAXVIEWCOUNT];

	quint8  specifySymb;	// 指定符号
	quint32 specifyColor;	// 指定颜色 

// 如果有这些参数，则作为额外数据来处理
//    // 二次航迹参数
//    SECONDARYTRACK secondaryTrack;
//    // IFF参数
//    IFFTRACK iffTrack;
//    // 组网参数
//    NETWORKTRACK networkTrack;

    // 额外的数据,应用时需要使用 new qint8[]申请内存,以保证析构函数中能够正确删除该内存
    qint8*   extraData;

    ~tagTrack()
    {
        clearExtraData();
    }

    // 设置额外数据
    void setExtraData(qint8* _data)
    {
        if(extraData)
            delete[] extraData;
        extraData = _data;
    }

    // 清除所有的外部数据
    void clearExtraData()
    {
        setExtraData(NULL);
        for(quint16  pos = curPointPos, i=0; i<totalPoints; i++)
        {
            points[pos].setExtraData(NULL);
            pos = PrevTrackPointIndex(pos);
        }
    }

    // 更新航迹当前点迹
    void enterTrackPoint()
    {
        //调整点迹位置，如果当前点迹位置超过最大点迹数，位置回头
        curPointPos = NextTrackPointIndex(curPointPos);
        //如果当前的点迹数比最大数要小，当前点迹数增加1
        if(curPoints < maxPoints)
            curPoints++;
        //如果点迹总的个数没有到达最大允许个数，总个数加1
        if(totalPoints < MAXTRACKPOINTS)
            totalPoints++;
    }
}TRACK, *LPTRACK;

// 点迹类型控制结构
typedef struct tagTrackTypeInfo
{
    quint8		type;   // 点迹类型
    QString     name;   // 类型名称
    union{
        struct{
            quint16	displayFlag	:1;	// 显示标志
            quint16 tailDispMode:1; // 尾点显示模式 0:显示圆点 1:显示符号
            quint16	reserve		:14;// 保留
        };
        quint16 flags;
    };
    quint32	color;		// 航迹显示颜色
    quint8  symbol;     // 航迹符号索引
    quint8  symbolSize; // 符号大小
}TRACKTYPEINFO, *LPTRACKTYPEINFO;

typedef QHash<quint8, LPTRACKTYPEINFO> TRACKTYPEINFOHASH;


/***********************************************************/
// 航迹信息
typedef struct tagSetTrackInfo
{
    TRACKNO         trackNo;
    RTHETA_POINT    current_point;    // 当前点
    RTHETA_POINT	extrapolatePoint; // 外推点

    quint8  source;     // 点迹来源
    quint16 velocity;   // 速度
    quint16 course;     // 航向
    quint16 height;     // 高度

    TRACK_TRACE trace; // 跟踪状态
    VALUE8BIT  type;   // 航迹类型
    VALUE8BIT  model;  // 航迹属性
    VALUE8BIT  number; // 航迹数量
    char	name[6];   // 航迹别名

    // 额外数据
    qint8*  trackExtraData;    // 航迹额外数据
    qint8*  pointExtraData;    // 点迹额外数据
}SETTRACKINFO;

// 航迹状态
typedef struct tagSetTrackStatus
{
    quint8	trackStatus:3;		// 跟踪状态
    quint8	trackStatusFlag :1;
    quint8	targetStatus:3;		// 目标状态
    quint8	targetStatusFlag :1;
    quint8	captureStatus : 3;	// 目标截获状态
    quint8	captureStatusFlag :1;
    quint8	measureHeight	:1;	// 测高
    quint8	measureHeightFlag : 1;	// 测高标志
    quint8	trackMode:1;			// 跟踪方式(0自动,1手动)
    quint8	trackModeFlag :1;		// 方标
    quint8	coruscate:1;		// 闪烁
    quint8	coruscateFlag:1;	// 闪标
    quint8	hPrecision : 3;		// 高度精度
    quint8	hPrecisionFlag:1;	// 高度精度标志
    quint8	jumpFlag:1;			// 跳
    quint8	ctrlFlag:1;			// 控制
    quint16	trackNo;		// 批号
    quint8	beamDispNo;		// 波束号/显示席位号
    quint16	height;			// 高度
    quint8	trackAlias[6];	// 航迹别名
}SETTRACKSTATUS;

// 人工航迹起始
typedef struct tagSetTrackManInit
{
    quint16	trackNo;		// 批号
    SQUARE_POINT    square_point;    // 当前点
    quint8	trackMode : 1;	// 跟踪方式(0:自动跟踪,1:人工跟踪)
    quint8	control : 1;	// 控制
    quint8	start	: 1;	// 起始
    quint8	reserve1: 5;	//
    quint8	beamDispNo;		// 波束号/显示席位号
    quint8	source;			// 来源
}SETTRACKMANINIT;

// 类型属性
typedef struct tagSetTrackTypeAttr
{
    quint16	trackNo;   // 批号
    VALUE8BIT  type;   // 航迹类型
    VALUE8BIT  model;  // 航迹属性
    VALUE8BIT  number; // 航迹数量
}SETTRACKTYPEATTR;

// 改换批
typedef struct tagSetChgTrackNo
{
    quint16	trackNo1;
    quint16	trackNo2;
}SETCHGTRACKNO;

// 目标删除
typedef struct tagSetTrackDelete
{
    quint16	trackNo;    // 批号
    quint8	frames;     // 帧数
}SETTRACKDELETE;

/***********************************************************/
/*
 * class: Track
 * author: moflying
 * time: 2010-04-15
 * description:
 *
 */
class TARGETMANAGESHARED_EXPORT Track
{
public:
    explicit Track(quint16 maxTracks);
    virtual ~Track();

// 航迹表
public:
    // 判断航迹索引号是否有效
    bool isIndexValid(const TRACKINDEX i) const
    {   return m_trackTable.contains(i);   }
    // 判断航迹是否有效
    bool isTrackValid(const TRACKINDEX i) const
    {   return (isIndexValid(i) && m_trackTable[i]->valid);    }
    // 判断批号是否有效
    bool isTrackNoValid(const TRACKNO no) const
    {   return isTrackValid(indexByNo(no)); }

    /*查找航迹批号相对应的索引号，如果该批号不存在，则返回-1*/
    TRACKINDEX indexByNo(const TRACKNO no) const
    {
        return m_no_index.value(no, TRACKINDEX(-1));
    }

    /*获取航迹对象*/
    LPTRACK getTrack(const TRACKINDEX i)
    {
        return (isTrackValid(i) ? m_trackTable[i] : NULL);
    }

    /*如果位置点在航迹标牌范围内，则返回该航迹索引号*/
    TRACKINDEX trackBoardSelect(const QPoint& pt, quint8 view);

    // 获取航迹批号
    TRACKNO getTrackNo(const TRACKINDEX i)
    {   return (isTrackValid(i) ? m_trackTable[i]->no : -1);}

    // 获取航迹批号字符串
    QString getTrackNoString (TRACKINDEX i, quint8 length=3)
    {
        LPTRACK track = getTrack(i);
        if (track)
            return QString("%1").arg(track->no, length, 10, QLatin1Char('0'));
        else
            return "";
    }

    // 删除所有航迹
    void clear (bool deleteFlag=false);

    /*接收到航迹信息，则跟踪单元航迹滤波后(航迹输出时)调用*/
    void setTrackInfo (const SETTRACKINFO& info);
    // 设置航迹状态
    void setTrackStatus (const SETTRACKSTATUS& trackStatus);
    // 人工航迹起始
    void setTrackManInit (const SETTRACKMANINIT& trackManInit);
    // 设置目标类型属性
    void setTrackTypeAttr (const SETTRACKTYPEATTR& trackTypeAttr);
    // 设置改换批
    void setChgTrackNo (const SETCHGTRACKNO& chgTrackNo);
    // 设置目标删除
    void setTrackDelete (const SETTRACKDELETE& trackDelete);

    //////////////////////////////////////////////////////////////////////
    // 目标位置更新处理
    //更新所有目标的屏幕坐标
    void updateScreenPoint();
    void updateScreenPoint(quint8 index);
    //更新所有目标的直角坐标和屏幕坐标，距离量纲变化时调用
    void updateSquareAndScreenPoint();
    //目标位置移动处理
    void move (const QPointF& sq, const QList<QPoint>& sc, quint8 index);

	void cpaAlarm();

protected:
    // 更新航迹当前点迹
    inline void enterTrackPoint(TRACK& track, const SQUARE_POINT& sq);
    inline void updateTrackScreenPoint(LPTRACK track0);

    // 创建指定数量的航迹对象
    inline void createTracks (quint16 maxTracks);
    // 删除所有的航迹对象
    inline void deleteAllTracks ();
    // 删除指定航迹对象
    inline void deleteTrack(TRACKINDEX);
    // 分配一个航迹对象
    inline TRACKINDEX applyTrack ();
    // 初始化指定航迹对象
    inline void initTrack (TRACK& track0);

	inline void cpaAlarm(LPTRACK track);
// 航迹类型管理
public:
    // 注册航迹类型(建议在程序初始化时一次注册完所有的航迹类型)
    bool registerType(quint8 type, const QString& name, quint32 color, quint8 symbol=SYMBOL_CROSS, quint8 symsize = 3);
    // 删除航迹类型
    void unregisterType(quint8 type);
    // 设置航迹类型标志
    void setTrackTypeFlag(quint8 type, quint8 flags);
    // 判断航迹类型是否有效
    bool isTypeValid(quint8 type) const
    {   return m_typeInfo.contains(type);   }
    // 航迹类型颜色
    void setTrackTypeColor(quint8 type, quint32 color);
    quint32 trackTypeColor(quint8 type) const;
    // 航迹类型名称
    void setTrackTypeName(quint8 type, const QString& name);
    QString trackTypeName(quint8 type) const;
    // 航迹类型符号
    void setTrackTypeSymbol(quint8 type, quint8 symbol);
    quint8 trackTypeSymbol(quint8 type) const;
    // 航迹符号大小
    void setTrackTypeSymbolSize(quint8 type, quint8 size);
    quint8 trackTypeSymbolSize(quint8 type) const;

protected:
    // 删除航迹类型
    inline void unregisterType();
    inline quint8 getTrackPointSource(quint8 src0) const
    {   return isTypeValid(src0) ? src0 : TRACK_TYPE_UNKNOWN;    }

public:
    // 设置坐标变换对象
    void setView(CRadarView* view, quint8 index)
    {
        if(index < MAXVIEWCOUNT)
        {
            m_radarView[index] = view;
            m_radarViewCount = index+1;
            if(!m_radarView0)
                m_radarView0 = view;
        }
    }

    // 设置是否显示航迹
    void setTrackShow(bool show)
    {   m_defaultTrackShow = show;  }
    // 判断是否显示航迹标志
    bool isTrackShow () const
    {   return m_defaultTrackShow;  }
    // 判断某一类型的航迹是否可显示
    bool isTrackShow (quint8 trackType) const;
    bool isTrackShow (LPTRACK ptrack) const;

    quint32 formBoardLineValue(quint8 line1, quint8 line2=0, quint8 line3=0, quint8 line4=0, quint8 line5=0)
    {
        quint32 value = 0;
        quint32 maxlines = 0;
        quint32 lines[5] = {line1 & 0x1f, line2 & 0x1f, line3 & 0x1f, line4 & 0x1f, line5 & 0x1f};
        for(quint8 i=0; i<5; i++)
        {
            if(lines[i])
            {
            value += (lines[i] << (i*5));
            maxlines ++;
            }
        }
        value += (maxlines << 25);
        return value;
    }

    // 设置指定模式默认标牌每行显示内容
    void setDefaultBoardLineValue(quint8 mode, quint32 value)
    {
        if(mode < MAXBOARDMODE)
            m_defaultBoardLineValue[mode] = value;
    }

    // 注册可用的标牌显示内容索引值
    void registerBoardLineValue(quint8 linev)
    {
        if(!m_usableBoardLines.contains(linev))
            m_usableBoardLines.append(linev);
    }

    // 设置航迹标牌显示模式(改变标牌显示模式)
    void setTrackBoardMode(quint8 mode, LPTRACK track=NULL);
    // 获取航迹标牌模式（或系统标牌模式）
    quint8 trackBoardMode(LPTRACK track=NULL)
    {
        BOARDDISP &board = (track ? track->boardDisp : m_defaultBoardDisp);
        return board.BoardMode;
    }

    // 切换标牌最后一行显示的内容
    void switchBoardLastLine(LPTRACK track);    
    // 旋转指引线的方向
    void rotateBoardCourse (LPTRACK track);
    // 设置航迹指引线线长
    void setLineLength (LPTRACK track, quint8 length);
    // 人工设置指引线, 长度和方向
    void manGuildLine (LPTRACK track, int length, int course);
    // 设置航迹标牌字体大小
    void setBoardFontSize(LPTRACK track, quint8 fontsize);
    quint8 boardFontSize(LPTRACK track);
    // 显示标牌边框
    void showBoardBorder(LPTRACK track, bool show=true);

    // 更新指定航迹标牌
    void updateBoard (LPTRACK track);

    // 注册航迹尾点模式
    void registerTrackPointMode(quint8 mode, const QString& name, quint16 pointcount);
    // 设置尾点模式
    void setTrackPointMode(quint8 mode, LPTRACK track=NULL);
    // 人工设置尾点数量
    void setManTrackPointSize(quint16 size, LPTRACK track=NULL);
    // 切换尾点模式
    void switchTrackPointMode(LPTRACK track=NULL);
    // 获取尾点模式
    quint8 trackPointMode(LPTRACK track=NULL) const;
    QString trackPointModeName(LPTRACK track=NULL) const;

    // 设置显示可见尾点
    void showVisiblePoint(bool show)
    {   m_showVisiblePoint = show;  }

    // 设置重要航迹显示颜色
    void setImportantColor(quint32 color)
    {   m_importantColor = color;   }
    // 设置衰弱航迹显示颜色
    void setFadeColor(quint32 color)
    {   m_fadeColor = color;        }

	// 设置航迹的符号
	void setTrackSymb(LPTRACK track, quint8 symb)
	{
		if(track)
			track->specifySymb = symb;
	}
	quint8 trackSymb(LPTRACK track) const
	{
		return (track?track->specifySymb:0);
	}
	// 设置航迹的颜色(黑色即为清除颜色)
	void setTrackColor(LPTRACK track, quint32 color)
	{
		if(track)
			track->specifyColor = color;
	}
	quint32 trackColor(LPTRACK track)
	{
		return (track?track->specifyColor:0);
	}

	// 设置显示指引线
	void setGuidLine(quint8 show, quint8 fmt, quint8 length = 0)
	{
		m_showGuidLine = show;
		m_guidLineFmt = fmt;
		m_guidLineLength = length;
	}

    // 绘制所有的航迹对象
    void paint(QPainter* painter, quint8 view);

protected:
    // 根据标牌内容索引值，将航迹相应的参数转换为字符串输出
    virtual QString formBoardText(LPTRACK track, quint8 linevalue);

    // 更新航迹标牌字符串
    inline void updateBoardText (LPTRACK track);
    // 更新航迹标牌指引线
    inline void updateBoardGuildLine(LPTRACK track);
    // 计算航迹指引线的方向
    inline float guildLineCourse (TRACK* track);
    // 计算航迹指引线的长度
    inline int guildLineLength (LPTRACK track, quint8 view);
    // 设置字体大小
    inline void setFontSize (QPainter* p, quint8 size);
    // 计算航迹标牌所在的显示区域
    QRect getBoardRect (QPainter* p, LPTRACK track, int x2, int y2);

    // 设置航迹尾点数量
    inline void setTrackPointModeAndSize(LPTRACK track, quint8 mode, quint16 size);
    inline void setTrackPointModeAndSize(quint8 mode, quint16 size);
    // 根据航迹尾点模式，获取对应的尾点数量
    inline quint16 trackPointSize(quint8 mode);

    void implUpdateTrackScreenPoint(LPTRACK track0);

    // 绘制一批航迹对象
    virtual void paintTrack(QPainter* painter, quint8 view, LPTRACK track);
    // 1.绘制航迹标牌
    void drawBoard (QPainter* p, LPTRACK track, const QRect& rc);
    // 2.绘制指引线
    void drawGuidLine (QPainter* p, int x1, int y1, int x2, int y2);
    // 3.显示历史点迹
    void drawFirstPoint (QPainter* painter, const int symbol, const QPoint& pt, char ptsize=3);
    // 4.显示尾点
    void drawHistoryPoint (QPainter* painter, const QPoint& pt);

	// 显示ATA模式目标
	void paintAta(QPainter* painter, quint8 view, LPTRACK track);
	// 更新ATA参数
	void updateAta(LPTRACK track, int view=0);

// 手录处理
public:
    // 当前批前跳
    void JumpPrev (bool updateCursorPos=false);
    // 当前批后跳
    void JumpNext (bool updateCursorPos=false);
    // 对当前批进行手录处理
    void ManExtractProcess ();
    // 更新手录航迹
    void UpdateManTrack (TRACKINDEX i);
    // 删除手录航迹
    void DeleteManTrack (TRACKINDEX i);
    // 判断是否当前手录航迹
    bool IsCurrentManTrack (TRACKINDEX i);

protected:
    //LPTRACK m_trackHeader;    // 航迹对象
    quint16 m_maxTracks;      // 最大航迹数量
    QHash<TRACKINDEX, LPTRACK> m_trackTable;
    QHash<TRACKNO, TRACKINDEX> m_no_index;  // 航迹批号与索引号表
    QMutex  m_mutex;

    // 可以分配的航迹索引号表,(只在用户线程中访问)
    QList<TRACKINDEX>		m_freeIndex;
    // 当前存在的航迹索引表
    QList<TRACKINDEX>       m_trackIndex;

    // 鼠标经过航迹索引
    TRACKINDEX  m_mouseOverTrackIndex;
    // 当前选择航迹索引
    TRACKINDEX  m_selectedTrackIndex;

    // 人工航迹管理
    // key:方位8192, value:航迹索引号
    QMultiMap<quint16, TRACKINDEX>	ManTrackLists;
    quint16 	ManTrackIndex;	// 当前手录航迹索引号
    QMutex		LockForManTrack;

    // 航迹类型信息
    TRACKTYPEINFOHASH m_typeInfo;

    // 坐标转换对象
    CRadarView* m_radarView0;
    CRadarView*   m_radarView[MAXVIEWCOUNT];
    quint8  m_radarViewCount;

    // 显示航迹标牌标志
    quint8  m_defaultTrackShow;
    // 标牌自动避让标志
    quint8  m_autoBoardAvoidFlag;

    // 默认标牌显示控制
    BOARDDISP   m_defaultBoardDisp;
    // 对应每种模式默认标牌每行显示内容(航迹初始化时使用该值来初始化标牌)
    quint32     m_defaultBoardLineValue[MAXBOARDMODE];
    // 可用的标牌显示内容索引值
    QList<quint8>   m_usableBoardLines;
    // 尾点模式
    QHash<quint8, TRACKPOINTMODE> m_trackPointMode;
    quint8  m_defaultTrackPointMode;

    // 重要航迹颜色
    quint32 m_importantColor;
    // 衷落航迹颜色
    quint32 m_fadeColor;

    // 显示可见历史点，指航迹当前点已经不在视图范围内时，
    // 是否显示在视图范围内的尾点
    quint8  m_showVisiblePoint;

	// 显示指引线
	quint8 m_showGuidLine;
	quint8 m_guidLineFmt;
	quint8 m_guidLineLength;
};

#endif // TRACK_H

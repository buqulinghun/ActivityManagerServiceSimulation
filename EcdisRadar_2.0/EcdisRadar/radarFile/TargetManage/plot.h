#ifndef PLOT_H
#define PLOT_H

#include "TargetManage_global.h"

#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtGui/QPainter>
#include <QtCore/QHash>

class CRadarView;

// 点迹类型定义
enum {
    PLOT_TYPE_UNKNOWN = 0,		// 未知点迹
    PLOT_TYPE_PRIMARY,			// 一次点迹
    PLOT_TYPE_SECONDARY,		// 二次点迹
    PLOT_TYPE_IIIREQUEST,		// III型询问机点迹
    PLOT_TYPE_PRIMARYSEC,		// 一二次相关点迹
    PLOT_TYPE_PRIMARYIII,		// 一次III型询问机相关点迹
    PLOT_TYPE_MULFUSE,			// 多站融合点迹
    PLOT_TYPE_FILTER,			// 滤波点迹
    PLOT_TYPE_USER1,
    PLOT_TYPE_USER2,
	PLOT_TYPE_USER3,
	PLOT_TYPE_USER4,
	PLOT_TYPE_USER5,
    MAX_PLOT_TYPE,
};

// 点迹数据结构
typedef struct tagPlot
{
    quint8  type;   // 点迹类型
    PLOTPOSITION  position; // 点迹位置
    quint8  aziWidth;   // 方位宽度
    quint8  rngWidth;   // 距离宽度
    qint8*   extraData;  // 额外的数据,应用时需要使用 new qint8[]申请内存,以保证析构函数中能够正确删除该内存

    tagPlot(quint8 __type=PLOT_TYPE_UNKNOWN) :
            type(__type), extraData(NULL)
    {}
    ~tagPlot()
    {
        if(extraData)
            delete[] extraData;
    }
}PLOT, *LPPLOT;

// 点迹链表
typedef QList<LPPLOT> PLOTLIST;

// 一组点迹
typedef struct tagPlotGroup
{
    quint8      keepTime;
    PLOTLIST    plotList;
}PLOTGROUP, *LPPLOTGROUP;

// 一组点迹链
typedef QList<LPPLOTGROUP> PLOTGROUPLIST;
typedef QHash<quint8, LPPLOTGROUP> PLOTGROUPHASH;


// 一帧点迹
typedef struct tagPlotFrame
{
    quint8   keepingTime; // 保留时间
    quint16  plotCount;   // 点迹数量
    PLOTGROUPHASH plotGroup; // 按类型分组管理
}PLOTFRAME, *LPPLOTFRAME;

// 一帧点迹链表
typedef QList<LPPLOTFRAME> PLOTFRAMELIST;

// 点迹类型控制结构
typedef struct tagPlotTypeInfo
{
    quint8		type;   // 点迹类型
    QString     name;   // 类型名称
    union{
        struct{
            quint16	displayFlag	:1;		// 显示标志
            quint16	displayBrow :1;		// 显示眉毛标志
            quint16	displaySymb :1;		// 显示符号标志
            quint16	colorEvade	:1;		// 颜色渐变标志
            quint16	sizeEvade	:1;		// 大小渐变标志
            quint16	reserve		:11;	// 保留
        };
        quint16 flags;  // 类型控制标志
    };
    quint8  keepTime;	// 保留时间
	quint16 symbol;		// 符号id
	quint8  symbolSize;
    quint32	color;		// 点迹显示颜色
}PLOTTYPEINFO, *LPPLOTTYPEINFO;

typedef QHash<quint8, LPPLOTTYPEINFO> PLOTTYPEINFOHASH;

// 外面传入的点迹的信息
typedef struct tagPlotInfo
{
    // 点迹类型
    quint8  type;
    // 直角坐标
    RTHETA_POINT	rtheta_point;
    quint8  aziWidth;   // 方位宽度(单位为度)
    quint8  rngWidth;   // 距离宽度(单位为米)
    // 额外数据
    qint8*   extraData;
}PLOTINFO, *LPPLOTINFO;

// 点迹数据链表
typedef QList<LPPLOTINFO> PLOTINFOLIST;

/*
 * class: Plot
 * author: moflying
 * time: 2010-04-07
 * description:
 *
 */
class TARGETMANAGESHARED_EXPORT Plot
{
public:
    explicit Plot();
    virtual ~Plot();

// 公用访问接口
public:
    // 清除所有点迹
    void clear(bool toDelete=false);

    // 设置点迹保留时间
    void setDefaultKeepTime(quint8 tm)
    {   m_defaultKeepTime = tm;    }
    quint8 defaultKeepTime() const
    {   return m_defaultKeepTime;   }

    // 新的点迹处理
    void enterPlot(LPPLOTINFO info);
    void enterPlot(const PLOTINFOLIST& info);

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

    // 设置32位的用户标志信息
    void setUserFlag(quint32 flag)
    {   m_userFlag = flag;  }
    quint32 userFlag() const
    {   return m_userFlag;  }

    // 设置点迹大小
    void setPlotSize(quint8 sz)
    {   m_plotSize = sz;    }
    quint8 plotSize() const
    {   return m_plotSize;  }

    // 按时间更新点迹显示
    void updatePlotKeepTime(const quint32 time0);
    //更新所有目标的屏幕坐标
    void updateScreenPoint();
    void updateScreenPoint(quint8 index);
    //更新所有目标的直角坐标和屏幕坐标，距离量纲变化时调用
    void updateSquareAndScreenPoint();
    //目标位置移动处理
    void move (const QPointF& sq, const QList<QPoint>& sc, quint8 index);

    // 判断当前类型的点迹是否显示
    bool isPlotTypeShow(quint8 type) const;
    // 判断是否显示点迹符号
    bool isPlotSymbolShow(quint8 type) const;
    // 判断是否显示点迹眉毛
    bool isPlotBrowShow(quint8 type) const;
    // 获取指定类型点迹显示颜色
    quint32  plotColor(quint8 type) const;

    // 设置帧时间宽度(有效范围为1到4)
    void setFrameTimeWidth(quint8 width)
    {   m_frameTimeWidth = (width>4 ? 4 : (width<1?1:width));  }
    quint8 frameTimeWidth() const
    {   return m_frameTimeWidth;    }

// 点迹类型管理
public:
    // 注册点迹类型(建议在程序初始化时一次注册完所有的点迹类型)
    bool registerType(quint8 type, const QString& name, quint32 color, quint32 symbol = SYMBOL_CROSS, quint8 symbolSize = 5);
    // 删除点迹类型
    void unregisterType(quint8 type);
    // 设置点迹类型标志
    void setTypeFlag(quint8 type, quint8 flags);
    // 设置点迹的保留时间
    void setTypeKeepTime(quint8 type, quint8 keeptime);
	// 设置点迹颜色
	void setTypeColor(quint8 type, quint32 color);
    // 判断点迹类型是否有效
    bool isTypeValid(quint8 type) const
    {   return m_typeInfo.contains(type);   }

	quint8 typeFlag(quint8 type) const;
	quint8 typeKeeptime(quint8 type) const;

private:
    // 删除点迹类型
    inline void unregisterType();

// 通过可重载的虚函数,让用户处理自己的特定需求.
public:
    // 绘制函数
    virtual void paint(QPainter* painter, quint8 idx=0);

protected:
    // 添加额外数据到点迹中
    virtual void addExtraData(LPPLOT plot, qint8* extraData)
    {   plot->extraData = extraData;      }

protected:
    // 申请一个点迹对象
    inline LPPLOT applyPlot();
    // 申请一个点迹组对象
    inline LPPLOTGROUP applyPlotGroup();
    // 申请一个点迹帧对象
    inline LPPLOTFRAME applyPlotFrame();
    // 访问当前帧对象
    inline LPPLOTFRAME accessCurrentPlotFrame();
    // 释放一个点迹
    inline void freePlot(LPPLOT plot);
    // 释放一组点迹
    inline void freePlot(PLOTLIST& plotList);
    // 释放一组点迹
    void freePlot(PLOTGROUPHASH& groupHash);
    // 释放一组点迹
    inline void freePlotGroup(LPPLOTGROUP plotGroup);
    inline void freePlotGroup(PLOTGROUPHASH& groupHash);
    // 释放帧点迹对象
    inline void freePlotFrame(bool freeGroup=false);
    inline void freePlotFrame(LPPLOTFRAME plotFrame, bool freeGroup=false);
    // 添加点迹到指定帧
    inline void insertPlotToFrame(LPPLOTFRAME frame, LPPLOTINFO info);

    // 绘制点迹符号
    inline void drawPlotSymbol(QPainter* painter, const int type, const QPoint& pt, char ptsize);
    // 绘制点迹眉毛
    inline void drawPlotBrow(QPainter* painter, LPPLOT plot, quint8 viewindex);
    // 绘制点迹标牌
    inline void drawPlotBoard(QPainter* painter, LPPLOT plot, quint8 viewindex);

    // 判断是否显示点迹标牌
    inline bool isPlotBoardShow(LPPLOT plot);

    // 判断当前帧是否发生改变
    inline bool frameChanged();

protected:
    // 数据保护
    QMutex  m_mutex;
    // 当前每帧点迹列表
    PLOTFRAMELIST   m_plotFrameList;
    // 使用自由对象链表,来管理需要释放的对象,从而减少调用new和delete的开销和内存碎片的产生.
    // 缺点是可能会占用更多的内存,如果内存比较紧张的话,可以适当的进行内存回收.
    // 自由点迹链表
    PLOTLIST        m_freePlotList;
    // 自由点迹帧结构链表
    PLOTFRAMELIST   m_freePlotFrame;
    // 自由点迹组链
    PLOTGROUPLIST   m_freePlotGroup;

    // 点迹类型信息
    PLOTTYPEINFOHASH m_typeInfo;

    // 坐标转换对象
    CRadarView* m_radarView0;
    CRadarView* m_radarView[MAXVIEWCOUNT];
    quint8  m_radarViewCount;

    // 点迹保留时间
    quint8  m_defaultKeepTime;
    // 点迹大小
    quint8  m_plotSize;
    // 当前点迹消失时间
    quint32 m_disappearTime;
    // 当前点迹出现时间
    quint32 m_appearTime;

    quint32 m_lastFrameTime;
    quint8  m_frameTimeWidth;

    // 32位的用户标志信息,用于保留用户的特殊信息
    quint32 m_userFlag;    
};

#endif // PLOT_H

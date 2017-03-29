#ifndef PLOT_H
#define PLOT_H

#include "TargetManage_global.h"

#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtGui/QPainter>
#include <QtCore/QHash>

class CRadarView;

// �㼣���Ͷ���
enum {
    PLOT_TYPE_UNKNOWN = 0,		// δ֪�㼣
    PLOT_TYPE_PRIMARY,			// һ�ε㼣
    PLOT_TYPE_SECONDARY,		// ���ε㼣
    PLOT_TYPE_IIIREQUEST,		// III��ѯ�ʻ��㼣
    PLOT_TYPE_PRIMARYSEC,		// һ������ص㼣
    PLOT_TYPE_PRIMARYIII,		// һ��III��ѯ�ʻ���ص㼣
    PLOT_TYPE_MULFUSE,			// ��վ�ںϵ㼣
    PLOT_TYPE_FILTER,			// �˲��㼣
    PLOT_TYPE_USER1,
    PLOT_TYPE_USER2,
	PLOT_TYPE_USER3,
	PLOT_TYPE_USER4,
	PLOT_TYPE_USER5,
    MAX_PLOT_TYPE,
};

// �㼣���ݽṹ
typedef struct tagPlot
{
    quint8  type;   // �㼣����
    PLOTPOSITION  position; // �㼣λ��
    quint8  aziWidth;   // ��λ���
    quint8  rngWidth;   // ������
    qint8*   extraData;  // ���������,Ӧ��ʱ��Ҫʹ�� new qint8[]�����ڴ�,�Ա�֤�����������ܹ���ȷɾ�����ڴ�

    tagPlot(quint8 __type=PLOT_TYPE_UNKNOWN) :
            type(__type), extraData(NULL)
    {}
    ~tagPlot()
    {
        if(extraData)
            delete[] extraData;
    }
}PLOT, *LPPLOT;

// �㼣����
typedef QList<LPPLOT> PLOTLIST;

// һ��㼣
typedef struct tagPlotGroup
{
    quint8      keepTime;
    PLOTLIST    plotList;
}PLOTGROUP, *LPPLOTGROUP;

// һ��㼣��
typedef QList<LPPLOTGROUP> PLOTGROUPLIST;
typedef QHash<quint8, LPPLOTGROUP> PLOTGROUPHASH;


// һ֡�㼣
typedef struct tagPlotFrame
{
    quint8   keepingTime; // ����ʱ��
    quint16  plotCount;   // �㼣����
    PLOTGROUPHASH plotGroup; // �����ͷ������
}PLOTFRAME, *LPPLOTFRAME;

// һ֡�㼣����
typedef QList<LPPLOTFRAME> PLOTFRAMELIST;

// �㼣���Ϳ��ƽṹ
typedef struct tagPlotTypeInfo
{
    quint8		type;   // �㼣����
    QString     name;   // ��������
    union{
        struct{
            quint16	displayFlag	:1;		// ��ʾ��־
            quint16	displayBrow :1;		// ��ʾüë��־
            quint16	displaySymb :1;		// ��ʾ���ű�־
            quint16	colorEvade	:1;		// ��ɫ�����־
            quint16	sizeEvade	:1;		// ��С�����־
            quint16	reserve		:11;	// ����
        };
        quint16 flags;  // ���Ϳ��Ʊ�־
    };
    quint8  keepTime;	// ����ʱ��
	quint16 symbol;		// ����id
	quint8  symbolSize;
    quint32	color;		// �㼣��ʾ��ɫ
}PLOTTYPEINFO, *LPPLOTTYPEINFO;

typedef QHash<quint8, LPPLOTTYPEINFO> PLOTTYPEINFOHASH;

// ���洫��ĵ㼣����Ϣ
typedef struct tagPlotInfo
{
    // �㼣����
    quint8  type;
    // ֱ������
    RTHETA_POINT	rtheta_point;
    quint8  aziWidth;   // ��λ���(��λΪ��)
    quint8  rngWidth;   // ������(��λΪ��)
    // ��������
    qint8*   extraData;
}PLOTINFO, *LPPLOTINFO;

// �㼣��������
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

// ���÷��ʽӿ�
public:
    // ������е㼣
    void clear(bool toDelete=false);

    // ���õ㼣����ʱ��
    void setDefaultKeepTime(quint8 tm)
    {   m_defaultKeepTime = tm;    }
    quint8 defaultKeepTime() const
    {   return m_defaultKeepTime;   }

    // �µĵ㼣����
    void enterPlot(LPPLOTINFO info);
    void enterPlot(const PLOTINFOLIST& info);

    // ��������任����
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

    // ����32λ���û���־��Ϣ
    void setUserFlag(quint32 flag)
    {   m_userFlag = flag;  }
    quint32 userFlag() const
    {   return m_userFlag;  }

    // ���õ㼣��С
    void setPlotSize(quint8 sz)
    {   m_plotSize = sz;    }
    quint8 plotSize() const
    {   return m_plotSize;  }

    // ��ʱ����µ㼣��ʾ
    void updatePlotKeepTime(const quint32 time0);
    //��������Ŀ�����Ļ����
    void updateScreenPoint();
    void updateScreenPoint(quint8 index);
    //��������Ŀ���ֱ���������Ļ���꣬�������ٱ仯ʱ����
    void updateSquareAndScreenPoint();
    //Ŀ��λ���ƶ�����
    void move (const QPointF& sq, const QList<QPoint>& sc, quint8 index);

    // �жϵ�ǰ���͵ĵ㼣�Ƿ���ʾ
    bool isPlotTypeShow(quint8 type) const;
    // �ж��Ƿ���ʾ�㼣����
    bool isPlotSymbolShow(quint8 type) const;
    // �ж��Ƿ���ʾ�㼣üë
    bool isPlotBrowShow(quint8 type) const;
    // ��ȡָ�����͵㼣��ʾ��ɫ
    quint32  plotColor(quint8 type) const;

    // ����֡ʱ����(��Ч��ΧΪ1��4)
    void setFrameTimeWidth(quint8 width)
    {   m_frameTimeWidth = (width>4 ? 4 : (width<1?1:width));  }
    quint8 frameTimeWidth() const
    {   return m_frameTimeWidth;    }

// �㼣���͹���
public:
    // ע��㼣����(�����ڳ����ʼ��ʱһ��ע�������еĵ㼣����)
    bool registerType(quint8 type, const QString& name, quint32 color, quint32 symbol = SYMBOL_CROSS, quint8 symbolSize = 5);
    // ɾ���㼣����
    void unregisterType(quint8 type);
    // ���õ㼣���ͱ�־
    void setTypeFlag(quint8 type, quint8 flags);
    // ���õ㼣�ı���ʱ��
    void setTypeKeepTime(quint8 type, quint8 keeptime);
	// ���õ㼣��ɫ
	void setTypeColor(quint8 type, quint32 color);
    // �жϵ㼣�����Ƿ���Ч
    bool isTypeValid(quint8 type) const
    {   return m_typeInfo.contains(type);   }

	quint8 typeFlag(quint8 type) const;
	quint8 typeKeeptime(quint8 type) const;

private:
    // ɾ���㼣����
    inline void unregisterType();

// ͨ�������ص��麯��,���û������Լ����ض�����.
public:
    // ���ƺ���
    virtual void paint(QPainter* painter, quint8 idx=0);

protected:
    // ��Ӷ������ݵ��㼣��
    virtual void addExtraData(LPPLOT plot, qint8* extraData)
    {   plot->extraData = extraData;      }

protected:
    // ����һ���㼣����
    inline LPPLOT applyPlot();
    // ����һ���㼣�����
    inline LPPLOTGROUP applyPlotGroup();
    // ����һ���㼣֡����
    inline LPPLOTFRAME applyPlotFrame();
    // ���ʵ�ǰ֡����
    inline LPPLOTFRAME accessCurrentPlotFrame();
    // �ͷ�һ���㼣
    inline void freePlot(LPPLOT plot);
    // �ͷ�һ��㼣
    inline void freePlot(PLOTLIST& plotList);
    // �ͷ�һ��㼣
    void freePlot(PLOTGROUPHASH& groupHash);
    // �ͷ�һ��㼣
    inline void freePlotGroup(LPPLOTGROUP plotGroup);
    inline void freePlotGroup(PLOTGROUPHASH& groupHash);
    // �ͷ�֡�㼣����
    inline void freePlotFrame(bool freeGroup=false);
    inline void freePlotFrame(LPPLOTFRAME plotFrame, bool freeGroup=false);
    // ��ӵ㼣��ָ��֡
    inline void insertPlotToFrame(LPPLOTFRAME frame, LPPLOTINFO info);

    // ���Ƶ㼣����
    inline void drawPlotSymbol(QPainter* painter, const int type, const QPoint& pt, char ptsize);
    // ���Ƶ㼣üë
    inline void drawPlotBrow(QPainter* painter, LPPLOT plot, quint8 viewindex);
    // ���Ƶ㼣����
    inline void drawPlotBoard(QPainter* painter, LPPLOT plot, quint8 viewindex);

    // �ж��Ƿ���ʾ�㼣����
    inline bool isPlotBoardShow(LPPLOT plot);

    // �жϵ�ǰ֡�Ƿ����ı�
    inline bool frameChanged();

protected:
    // ���ݱ���
    QMutex  m_mutex;
    // ��ǰÿ֡�㼣�б�
    PLOTFRAMELIST   m_plotFrameList;
    // ʹ�����ɶ�������,��������Ҫ�ͷŵĶ���,�Ӷ����ٵ���new��delete�Ŀ������ڴ���Ƭ�Ĳ���.
    // ȱ���ǿ��ܻ�ռ�ø�����ڴ�,����ڴ�ȽϽ��ŵĻ�,�����ʵ��Ľ����ڴ����.
    // ���ɵ㼣����
    PLOTLIST        m_freePlotList;
    // ���ɵ㼣֡�ṹ����
    PLOTFRAMELIST   m_freePlotFrame;
    // ���ɵ㼣����
    PLOTGROUPLIST   m_freePlotGroup;

    // �㼣������Ϣ
    PLOTTYPEINFOHASH m_typeInfo;

    // ����ת������
    CRadarView* m_radarView0;
    CRadarView* m_radarView[MAXVIEWCOUNT];
    quint8  m_radarViewCount;

    // �㼣����ʱ��
    quint8  m_defaultKeepTime;
    // �㼣��С
    quint8  m_plotSize;
    // ��ǰ�㼣��ʧʱ��
    quint32 m_disappearTime;
    // ��ǰ�㼣����ʱ��
    quint32 m_appearTime;

    quint32 m_lastFrameTime;
    quint8  m_frameTimeWidth;

    // 32λ���û���־��Ϣ,���ڱ����û���������Ϣ
    quint32 m_userFlag;    
};

#endif // PLOT_H

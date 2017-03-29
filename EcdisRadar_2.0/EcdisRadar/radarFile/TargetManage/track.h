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
// ��󺽼���ʷ����
#define MAXTRACKPOINTS  10 //1001
#define MAXBOARDMODE    7
#define MANTRACKPOINTMODE 255

#define NextTrackPointIndex(i) ((i)<MAXTRACKPOINTS-1?(i)+1:0)
#define PrevTrackPointIndex(i) ((i)>0?(i)-1:MAXTRACKPOINTS-1)

typedef qint16 TRACKINDEX;
typedef qint16 TRACKNO;

// ���ú������Ͷ���
enum {
    TRACK_TYPE_UNKNOWN=0,		// δ֪���ͺ���
    TRACK_TYPE_PRIMARY,			// һ�κ���
    TRACK_TYPE_SECONDARY,		// ���κ���
    TRACK_TYPE_IIIREQUEST,		// III��ѯ�ʻ�����
    TRACK_TYPE_PRIMARYSEC,		// һ������Ժ���
    TRACK_TYPE_PRIMARYIII,		// һ�Ρ�III����Ժ���
    TRACK_TYPE_IIIPRIMARY,		// III�͡�һ����Ժ���
    TRACK_TYPE_FILTER,			// �������ƺ���
    TRACK_TYPE_USER1,
    TRACK_TYPE_USER2,
	TRACK_TYPE_USER3,
    MAX_TRACK_TYPE,
};

// �߶���Դ
enum {
    HSOURCE_UNKNOWN = 0,
    HSOURCE_PRIMARY,	// һ�θ߶�
    HSOURCE_SECONDARY,	// ���θ߶�
    HSOURCE_IFF,		// ѯ�ʸ߶�
    HSOURCE_USER,
};

//������������
typedef struct{
    //�������ٷ�ʽ
    quint8 Mode:	1;  // 0:�Զ����� 1:�˹�����
    //�������Ʒ�ʽ
    quint8 Control:	1;  // 0:�ǿ� 1:�ܿ�
    //��������״̬
    quint8 Trace:	3;
    //����Ŀ��״̬ 0:һ�� 1:ģ�� 2:��Ҫ
    quint8 Target: 	3;
}TRACK_TRACE;

//�������ͱ�־
typedef struct {
    quint8 value:7; // ��ֵ
    quint8 flag:1;  // ��Ч��־
}VALUE8BIT;

// �������ٲ���
typedef struct tagTrackFilterParam
{

}TRACKFILTERPARAM, *LPTRACKFILTERPARAM;

// ������ṹ
typedef struct tagTrackPoint
{
    quint8  source;     // �㼣��Դ
    quint32 updateTime; // ����ʱ��
    quint16 height:13;  // �����߶�(�����ڸ߶ȷ���(�߶�����))
    quint16 heightSrc:3;// �߶���Դ
    quint16 speed;      // �����ٶ�(�����ڻ�������(�ٶ����ߺͼ��ٶ�����))    
    quint16	doplSpeed : 15; // �����������ٶ�
    quint16	doplSpeedFlag : 1;
    quint8	fadeing;		// ���Ʊ�־
    PLOTPOSITION position;  // ����λ��    
    TRACKFILTERPARAM filter;     // �������ٲ���

    // ���������,Ӧ��ʱ��Ҫʹ�� new qint8[]�����ڴ�,�Ա�֤�����������ܹ���ȷɾ�����ڴ�
    qint8*   extraData;

    // ���ö�������
    void setExtraData(qint8* _data)
    {
        if(extraData)
            delete[] extraData;
        extraData = _data;
    }

}TRACKPOINT, *LPTRACKPOINT;

// ����ָ������ʾ����
typedef struct tagCfgBoardDisp
{
    // ������ʾ��������
    enum {
        LINE_NULL = 0,
        LINE_TRACKNO,    // ����
        LINE_HEIGHT,     // �߶�
        LINE_SPEED,      // �ٶ�
        LINE_DOPLSPEED,  // �������ٶ�
        LINE_COURSE,     // ����
        LINE_ALIAS,      // ����
        LINE_TYPE,       // ����
        LINE_ATTRI,      // ����
        LINE_QUANTITY,   // ����
        LINE_SSRCODE,    // �������񺽶����״�Ŀ����룩
        LINE_IFFCODE,    // IFF����
        LINE_MODE1,      // ģʽ1����
        LINE_MODE2,      // ģʽ2����
        LINE_MODE3,      // ģʽ3����
        LINE_USER1, LINE_USER2, LINE_USER3, LINE_USER4,LINE_USER5,
        LINE_MAXBOARD,
    };

    // ָ����׼��˵����
    // ���ManLineΪ1����ʹ�ù̶��߳�(ManLineLength)�͹̶���λ(ManLineCourse)��
    // �������FixedLengthΪ1�����߳���LineLength���㣬�������ٶȼ���;
    // ���FixedCourseΪ1��������LineCourse���㣬�����ɺ�����㡣

    quint16	BoardMode  :3;		// ������ʾģʽ��0:�� 1:���� 2:��׼ 3:ȫ��
    quint16	FixedCourse:1;		// �̶�ָ���߷�λ 0:ָ���߷����ɺ������ 1:ʹ�ù̶�ָ��
    quint16 FixedLength:1;      // �̶��߳�
    quint16	ManLine	   :1;		// �˹��޸�ָ���߱�־
    quint16	ManLineCourse : 10;	// �˹�ָ���߷���(��λΪ360/1024)
    quint16	ManLineLength;		// �˹�ָ���߳���

    quint8	LineLength	: 3;	// �̶�ָ���߳��� 0-7���ɵ�
    quint8	LineCourse	: 3;	// ָ���߷��� 8������
    quint8	FontSize	: 2;	// �����С 0:��С 1:���� 2:�ϴ� 3���ܴ�
    quint8  ShowBorder   :1;   // ��ʾ�߿�
    quint8  FillBackground:1;   // ��䱳����־
    quint8	FillPersent   :6;	// ����͸����
    quint32	FillColor;			// ������ɫ
    quint32	BorderColor;		// �߿���ɫ

    // ָ������ÿ����ʾ������(��ֵ��Ӧ������ʾ��������)�����������ʾ5��
    union {
        struct{
            quint32 Line1:5;    // ָ�����Ƶ�1����ʾ����
            quint32 Line2:5;    // ָ�����Ƶ�2����ʾ����
            quint32 Line3:5;    // ָ�����Ƶ�3����ʾ����
            quint32 Line4:5;    // ָ�����Ƶ�4����ʾ����
            quint32 Line5:5;    // ָ�����Ƶ�5����ʾ����
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

    // ��ȡ����ĳһ�е���ʾ��������ֵ
    quint8 boardLineValue(quint8 line)
    {
        if(line < 5)
            return ((LineValue >> (line*5)) & 0x1f);
        else
            return 0;
    }

    // ���ñ���ĳһ�е���ʾ��������
    void setBoardLineValue(quint8 line, quint8 value)
    {
        if(line < 5)
        {
            LineValue &= (~(0x1f<<(line*5)));
            LineValue |= (value << (line*5));
        }
    }

    // �ж�ָ����������ֵ�Ƿ��ڱ�����
    bool isBoardContainsLineValue(const quint8 linev)
    {
        if(Line1 == linev || Line2 == linev ||
           Line3 == linev || Line4 == linev ||
           Line5 == linev)
            return true;
        else
            return false;
    }

    // �Զ���ת����
    void autoRotate()
    {
        if (ManLine)
        {
            ManLine = 0;
            LineCourse = ((ManLineCourse >> 7) & 0x7);	// 10λ-->3λ
        }
        FixedCourse = 1;
        LineCourse = (LineCourse+1) % 8;
    }

    // ����Ϊ�̶��߳�ģʽ
    void setLineLength(quint8 length)
    {
        // ȡ���˹�ָ���߱�־
        if (ManLine)
        {
            ManLine = 0;
            LineCourse = ((ManLineCourse >> 7) & 0x7);	// 10λ-->3λ
            FixedCourse = 1;
        }
        FixedLength = 1;
        LineLength = length;
    }

    // �˹�����ָ����, ���Ⱥͷ���
    void manGuildLine (int length, int course)
    {
        ManLineCourse = course;
        ManLineLength = length;
        ManLine = 1;
        FixedCourse = 0;
        FixedLength = 0;
    }

}BOARDDISP, *LPBOARDDISP;

// ����β��ṹ
typedef struct tagTrackPointMode
{
    quint8  mode;   // ģʽ����
    QString name;   // ģʽ����
    quint16 pointCount;// β������
}TRACKPOINTMODE;

// ���κ�������
typedef struct tagSecondaryTrack
{

}SECONDARYTRACK;

// IFF����
typedef struct tagIFFTrack
{

}IFFTRACK;

// ��������
typedef struct tagNetworkTrack
{

}NETWORKTRACK;


typedef struct tagAtaParam
{
	ushort cpa;
	ushort dcpa;	// 1/100ni
	ushort tcpa;// min
	ushort relativeSpeed;	// ����ٶ� 1/100 kn
	ushort relativeCourse;	// ��Ժ��� 1/10 ��
	ushort trueSpeed;		// ���ٶ�   1/100 kn
	ushort trueCourse;		// �溽��   1/10 ��
	ushort relativeRng;		// ��Ծ��� 1/100 mi
	ushort relativeAzi;		// ��Է�λ 1/10��

	ushort  predict_x, predict_y;

	union{
		struct{
	quint8 cpaFlag : 1;		// cpa/tcpa������־
	quint8 dcpaFlag : 1;		// cpa/tcpa������־
	quint8 tcpaFlag : 1;		// cpa/tcpa������־
	quint8 guardZoneFlag : 1;	// ���򱨾���־
	quint8 targetLost : 1;		// Ŀ�궪ʧ��־
	quint8 targetInit : 1;		// Ŀ����ʼ��־
	quint8 reserve   : 2;
		};
	quint8 flag;
	};
}ATAPARAM, * LPATAPARAM;

// �����ṹ
typedef struct tagTrack
{
    TRACKNO      no;    // ��������
    TRACKINDEX   index; // ����������

	TRACKPOINT  crntPoint;
    TRACKPOINT  points[MAXTRACKPOINTS]; // ������ʷ��
    quint16  curPointPos;   // ��ǰ������λ��
    quint16  maxPoints;     // �����ʷ����(���õ����β������)
    quint16  curPoints;     // ��ǰ��ʷ����(�Ӻ�������ʱ��ʼ���������ֵΪmaxPoints)������β����ʾ
    quint16  totalPoints;   // �ܵ���ʷ����(�Ӻ�������ʱ��ʼ���������ֵΪMAXTRACKPOINTS)
    quint8   hpointMode;    // β��ģʽ

    quint8  source;     // �㼣��Դ
    float speed;      // �ٶ�
    float course;     // ����
    quint16 doplspeed;  // �������ٶ�

    //���Ƶ�
    RTHETA_POINT	extrapolatePoint;

    TRACK_TRACE trace; // ����״̬
    VALUE8BIT  type;   // ��������
    VALUE8BIT  model;  // ��������
    VALUE8BIT  number; // ��������
    char	name[6];   // ��������

    // �������Ʊ�־
    union{
        struct{
            quint8  valid : 1;  // ��Ч��־
            quint8  updateBoardText:1;// ���±����ַ���
            quint8  reserve:6;
        };
        quint8  flags;
    };

    // ����ָ������ʾ����
    BOARDDISP   boardDisp;
    // ����ÿ���ַ���
    QList<QString>  boardLineTexts;
    // ������ʾ����
    QRect       boardRect[MAXVIEWCOUNT];

	quint8  specifySymb;	// ָ������
	quint32 specifyColor;	// ָ����ɫ 

// �������Щ����������Ϊ��������������
//    // ���κ�������
//    SECONDARYTRACK secondaryTrack;
//    // IFF����
//    IFFTRACK iffTrack;
//    // ��������
//    NETWORKTRACK networkTrack;

    // ���������,Ӧ��ʱ��Ҫʹ�� new qint8[]�����ڴ�,�Ա�֤�����������ܹ���ȷɾ�����ڴ�
    qint8*   extraData;

    ~tagTrack()
    {
        clearExtraData();
    }

    // ���ö�������
    void setExtraData(qint8* _data)
    {
        if(extraData)
            delete[] extraData;
        extraData = _data;
    }

    // ������е��ⲿ����
    void clearExtraData()
    {
        setExtraData(NULL);
        for(quint16  pos = curPointPos, i=0; i<totalPoints; i++)
        {
            points[pos].setExtraData(NULL);
            pos = PrevTrackPointIndex(pos);
        }
    }

    // ���º�����ǰ�㼣
    void enterTrackPoint()
    {
        //�����㼣λ�ã������ǰ�㼣λ�ó������㼣����λ�û�ͷ
        curPointPos = NextTrackPointIndex(curPointPos);
        //�����ǰ�ĵ㼣���������ҪС����ǰ�㼣������1
        if(curPoints < maxPoints)
            curPoints++;
        //����㼣�ܵĸ���û�е����������������ܸ�����1
        if(totalPoints < MAXTRACKPOINTS)
            totalPoints++;
    }
}TRACK, *LPTRACK;

// �㼣���Ϳ��ƽṹ
typedef struct tagTrackTypeInfo
{
    quint8		type;   // �㼣����
    QString     name;   // ��������
    union{
        struct{
            quint16	displayFlag	:1;	// ��ʾ��־
            quint16 tailDispMode:1; // β����ʾģʽ 0:��ʾԲ�� 1:��ʾ����
            quint16	reserve		:14;// ����
        };
        quint16 flags;
    };
    quint32	color;		// ������ʾ��ɫ
    quint8  symbol;     // ������������
    quint8  symbolSize; // ���Ŵ�С
}TRACKTYPEINFO, *LPTRACKTYPEINFO;

typedef QHash<quint8, LPTRACKTYPEINFO> TRACKTYPEINFOHASH;


/***********************************************************/
// ������Ϣ
typedef struct tagSetTrackInfo
{
    TRACKNO         trackNo;
    RTHETA_POINT    current_point;    // ��ǰ��
    RTHETA_POINT	extrapolatePoint; // ���Ƶ�

    quint8  source;     // �㼣��Դ
    quint16 velocity;   // �ٶ�
    quint16 course;     // ����
    quint16 height;     // �߶�

    TRACK_TRACE trace; // ����״̬
    VALUE8BIT  type;   // ��������
    VALUE8BIT  model;  // ��������
    VALUE8BIT  number; // ��������
    char	name[6];   // ��������

    // ��������
    qint8*  trackExtraData;    // ������������
    qint8*  pointExtraData;    // �㼣��������
}SETTRACKINFO;

// ����״̬
typedef struct tagSetTrackStatus
{
    quint8	trackStatus:3;		// ����״̬
    quint8	trackStatusFlag :1;
    quint8	targetStatus:3;		// Ŀ��״̬
    quint8	targetStatusFlag :1;
    quint8	captureStatus : 3;	// Ŀ��ػ�״̬
    quint8	captureStatusFlag :1;
    quint8	measureHeight	:1;	// ���
    quint8	measureHeightFlag : 1;	// ��߱�־
    quint8	trackMode:1;			// ���ٷ�ʽ(0�Զ�,1�ֶ�)
    quint8	trackModeFlag :1;		// ����
    quint8	coruscate:1;		// ��˸
    quint8	coruscateFlag:1;	// ����
    quint8	hPrecision : 3;		// �߶Ⱦ���
    quint8	hPrecisionFlag:1;	// �߶Ⱦ��ȱ�־
    quint8	jumpFlag:1;			// ��
    quint8	ctrlFlag:1;			// ����
    quint16	trackNo;		// ����
    quint8	beamDispNo;		// ������/��ʾϯλ��
    quint16	height;			// �߶�
    quint8	trackAlias[6];	// ��������
}SETTRACKSTATUS;

// �˹�������ʼ
typedef struct tagSetTrackManInit
{
    quint16	trackNo;		// ����
    SQUARE_POINT    square_point;    // ��ǰ��
    quint8	trackMode : 1;	// ���ٷ�ʽ(0:�Զ�����,1:�˹�����)
    quint8	control : 1;	// ����
    quint8	start	: 1;	// ��ʼ
    quint8	reserve1: 5;	//
    quint8	beamDispNo;		// ������/��ʾϯλ��
    quint8	source;			// ��Դ
}SETTRACKMANINIT;

// ��������
typedef struct tagSetTrackTypeAttr
{
    quint16	trackNo;   // ����
    VALUE8BIT  type;   // ��������
    VALUE8BIT  model;  // ��������
    VALUE8BIT  number; // ��������
}SETTRACKTYPEATTR;

// �Ļ���
typedef struct tagSetChgTrackNo
{
    quint16	trackNo1;
    quint16	trackNo2;
}SETCHGTRACKNO;

// Ŀ��ɾ��
typedef struct tagSetTrackDelete
{
    quint16	trackNo;    // ����
    quint8	frames;     // ֡��
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

// ������
public:
    // �жϺ����������Ƿ���Ч
    bool isIndexValid(const TRACKINDEX i) const
    {   return m_trackTable.contains(i);   }
    // �жϺ����Ƿ���Ч
    bool isTrackValid(const TRACKINDEX i) const
    {   return (isIndexValid(i) && m_trackTable[i]->valid);    }
    // �ж������Ƿ���Ч
    bool isTrackNoValid(const TRACKNO no) const
    {   return isTrackValid(indexByNo(no)); }

    /*���Һ����������Ӧ�������ţ���������Ų����ڣ��򷵻�-1*/
    TRACKINDEX indexByNo(const TRACKNO no) const
    {
        return m_no_index.value(no, TRACKINDEX(-1));
    }

    /*��ȡ��������*/
    LPTRACK getTrack(const TRACKINDEX i)
    {
        return (isTrackValid(i) ? m_trackTable[i] : NULL);
    }

    /*���λ�õ��ں������Ʒ�Χ�ڣ��򷵻ظú���������*/
    TRACKINDEX trackBoardSelect(const QPoint& pt, quint8 view);

    // ��ȡ��������
    TRACKNO getTrackNo(const TRACKINDEX i)
    {   return (isTrackValid(i) ? m_trackTable[i]->no : -1);}

    // ��ȡ���������ַ���
    QString getTrackNoString (TRACKINDEX i, quint8 length=3)
    {
        LPTRACK track = getTrack(i);
        if (track)
            return QString("%1").arg(track->no, length, 10, QLatin1Char('0'));
        else
            return "";
    }

    // ɾ�����к���
    void clear (bool deleteFlag=false);

    /*���յ�������Ϣ������ٵ�Ԫ�����˲���(�������ʱ)����*/
    void setTrackInfo (const SETTRACKINFO& info);
    // ���ú���״̬
    void setTrackStatus (const SETTRACKSTATUS& trackStatus);
    // �˹�������ʼ
    void setTrackManInit (const SETTRACKMANINIT& trackManInit);
    // ����Ŀ����������
    void setTrackTypeAttr (const SETTRACKTYPEATTR& trackTypeAttr);
    // ���øĻ���
    void setChgTrackNo (const SETCHGTRACKNO& chgTrackNo);
    // ����Ŀ��ɾ��
    void setTrackDelete (const SETTRACKDELETE& trackDelete);

    //////////////////////////////////////////////////////////////////////
    // Ŀ��λ�ø��´���
    //��������Ŀ�����Ļ����
    void updateScreenPoint();
    void updateScreenPoint(quint8 index);
    //��������Ŀ���ֱ���������Ļ���꣬�������ٱ仯ʱ����
    void updateSquareAndScreenPoint();
    //Ŀ��λ���ƶ�����
    void move (const QPointF& sq, const QList<QPoint>& sc, quint8 index);

	void cpaAlarm();

protected:
    // ���º�����ǰ�㼣
    inline void enterTrackPoint(TRACK& track, const SQUARE_POINT& sq);
    inline void updateTrackScreenPoint(LPTRACK track0);

    // ����ָ�������ĺ�������
    inline void createTracks (quint16 maxTracks);
    // ɾ�����еĺ�������
    inline void deleteAllTracks ();
    // ɾ��ָ����������
    inline void deleteTrack(TRACKINDEX);
    // ����һ����������
    inline TRACKINDEX applyTrack ();
    // ��ʼ��ָ����������
    inline void initTrack (TRACK& track0);

	inline void cpaAlarm(LPTRACK track);
// �������͹���
public:
    // ע�ẽ������(�����ڳ����ʼ��ʱһ��ע�������еĺ�������)
    bool registerType(quint8 type, const QString& name, quint32 color, quint8 symbol=SYMBOL_CROSS, quint8 symsize = 3);
    // ɾ����������
    void unregisterType(quint8 type);
    // ���ú������ͱ�־
    void setTrackTypeFlag(quint8 type, quint8 flags);
    // �жϺ��������Ƿ���Ч
    bool isTypeValid(quint8 type) const
    {   return m_typeInfo.contains(type);   }
    // ����������ɫ
    void setTrackTypeColor(quint8 type, quint32 color);
    quint32 trackTypeColor(quint8 type) const;
    // ������������
    void setTrackTypeName(quint8 type, const QString& name);
    QString trackTypeName(quint8 type) const;
    // �������ͷ���
    void setTrackTypeSymbol(quint8 type, quint8 symbol);
    quint8 trackTypeSymbol(quint8 type) const;
    // �������Ŵ�С
    void setTrackTypeSymbolSize(quint8 type, quint8 size);
    quint8 trackTypeSymbolSize(quint8 type) const;

protected:
    // ɾ����������
    inline void unregisterType();
    inline quint8 getTrackPointSource(quint8 src0) const
    {   return isTypeValid(src0) ? src0 : TRACK_TYPE_UNKNOWN;    }

public:
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

    // �����Ƿ���ʾ����
    void setTrackShow(bool show)
    {   m_defaultTrackShow = show;  }
    // �ж��Ƿ���ʾ������־
    bool isTrackShow () const
    {   return m_defaultTrackShow;  }
    // �ж�ĳһ���͵ĺ����Ƿ����ʾ
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

    // ����ָ��ģʽĬ�ϱ���ÿ����ʾ����
    void setDefaultBoardLineValue(quint8 mode, quint32 value)
    {
        if(mode < MAXBOARDMODE)
            m_defaultBoardLineValue[mode] = value;
    }

    // ע����õı�����ʾ��������ֵ
    void registerBoardLineValue(quint8 linev)
    {
        if(!m_usableBoardLines.contains(linev))
            m_usableBoardLines.append(linev);
    }

    // ���ú���������ʾģʽ(�ı������ʾģʽ)
    void setTrackBoardMode(quint8 mode, LPTRACK track=NULL);
    // ��ȡ��������ģʽ����ϵͳ����ģʽ��
    quint8 trackBoardMode(LPTRACK track=NULL)
    {
        BOARDDISP &board = (track ? track->boardDisp : m_defaultBoardDisp);
        return board.BoardMode;
    }

    // �л��������һ����ʾ������
    void switchBoardLastLine(LPTRACK track);    
    // ��תָ���ߵķ���
    void rotateBoardCourse (LPTRACK track);
    // ���ú���ָ�����߳�
    void setLineLength (LPTRACK track, quint8 length);
    // �˹�����ָ����, ���Ⱥͷ���
    void manGuildLine (LPTRACK track, int length, int course);
    // ���ú������������С
    void setBoardFontSize(LPTRACK track, quint8 fontsize);
    quint8 boardFontSize(LPTRACK track);
    // ��ʾ���Ʊ߿�
    void showBoardBorder(LPTRACK track, bool show=true);

    // ����ָ����������
    void updateBoard (LPTRACK track);

    // ע�ẽ��β��ģʽ
    void registerTrackPointMode(quint8 mode, const QString& name, quint16 pointcount);
    // ����β��ģʽ
    void setTrackPointMode(quint8 mode, LPTRACK track=NULL);
    // �˹�����β������
    void setManTrackPointSize(quint16 size, LPTRACK track=NULL);
    // �л�β��ģʽ
    void switchTrackPointMode(LPTRACK track=NULL);
    // ��ȡβ��ģʽ
    quint8 trackPointMode(LPTRACK track=NULL) const;
    QString trackPointModeName(LPTRACK track=NULL) const;

    // ������ʾ�ɼ�β��
    void showVisiblePoint(bool show)
    {   m_showVisiblePoint = show;  }

    // ������Ҫ������ʾ��ɫ
    void setImportantColor(quint32 color)
    {   m_importantColor = color;   }
    // ����˥��������ʾ��ɫ
    void setFadeColor(quint32 color)
    {   m_fadeColor = color;        }

	// ���ú����ķ���
	void setTrackSymb(LPTRACK track, quint8 symb)
	{
		if(track)
			track->specifySymb = symb;
	}
	quint8 trackSymb(LPTRACK track) const
	{
		return (track?track->specifySymb:0);
	}
	// ���ú�������ɫ(��ɫ��Ϊ�����ɫ)
	void setTrackColor(LPTRACK track, quint32 color)
	{
		if(track)
			track->specifyColor = color;
	}
	quint32 trackColor(LPTRACK track)
	{
		return (track?track->specifyColor:0);
	}

	// ������ʾָ����
	void setGuidLine(quint8 show, quint8 fmt, quint8 length = 0)
	{
		m_showGuidLine = show;
		m_guidLineFmt = fmt;
		m_guidLineLength = length;
	}

    // �������еĺ�������
    void paint(QPainter* painter, quint8 view);

protected:
    // ���ݱ�����������ֵ����������Ӧ�Ĳ���ת��Ϊ�ַ������
    virtual QString formBoardText(LPTRACK track, quint8 linevalue);

    // ���º��������ַ���
    inline void updateBoardText (LPTRACK track);
    // ���º�������ָ����
    inline void updateBoardGuildLine(LPTRACK track);
    // ���㺽��ָ���ߵķ���
    inline float guildLineCourse (TRACK* track);
    // ���㺽��ָ���ߵĳ���
    inline int guildLineLength (LPTRACK track, quint8 view);
    // ���������С
    inline void setFontSize (QPainter* p, quint8 size);
    // ���㺽���������ڵ���ʾ����
    QRect getBoardRect (QPainter* p, LPTRACK track, int x2, int y2);

    // ���ú���β������
    inline void setTrackPointModeAndSize(LPTRACK track, quint8 mode, quint16 size);
    inline void setTrackPointModeAndSize(quint8 mode, quint16 size);
    // ���ݺ���β��ģʽ����ȡ��Ӧ��β������
    inline quint16 trackPointSize(quint8 mode);

    void implUpdateTrackScreenPoint(LPTRACK track0);

    // ����һ����������
    virtual void paintTrack(QPainter* painter, quint8 view, LPTRACK track);
    // 1.���ƺ�������
    void drawBoard (QPainter* p, LPTRACK track, const QRect& rc);
    // 2.����ָ����
    void drawGuidLine (QPainter* p, int x1, int y1, int x2, int y2);
    // 3.��ʾ��ʷ�㼣
    void drawFirstPoint (QPainter* painter, const int symbol, const QPoint& pt, char ptsize=3);
    // 4.��ʾβ��
    void drawHistoryPoint (QPainter* painter, const QPoint& pt);

	// ��ʾATAģʽĿ��
	void paintAta(QPainter* painter, quint8 view, LPTRACK track);
	// ����ATA����
	void updateAta(LPTRACK track, int view=0);

// ��¼����
public:
    // ��ǰ��ǰ��
    void JumpPrev (bool updateCursorPos=false);
    // ��ǰ������
    void JumpNext (bool updateCursorPos=false);
    // �Ե�ǰ��������¼����
    void ManExtractProcess ();
    // ������¼����
    void UpdateManTrack (TRACKINDEX i);
    // ɾ����¼����
    void DeleteManTrack (TRACKINDEX i);
    // �ж��Ƿ�ǰ��¼����
    bool IsCurrentManTrack (TRACKINDEX i);

protected:
    //LPTRACK m_trackHeader;    // ��������
    quint16 m_maxTracks;      // ��󺽼�����
    QHash<TRACKINDEX, LPTRACK> m_trackTable;
    QHash<TRACKNO, TRACKINDEX> m_no_index;  // ���������������ű�
    QMutex  m_mutex;

    // ���Է���ĺ��������ű�,(ֻ���û��߳��з���)
    QList<TRACKINDEX>		m_freeIndex;
    // ��ǰ���ڵĺ���������
    QList<TRACKINDEX>       m_trackIndex;

    // ��꾭����������
    TRACKINDEX  m_mouseOverTrackIndex;
    // ��ǰѡ�񺽼�����
    TRACKINDEX  m_selectedTrackIndex;

    // �˹���������
    // key:��λ8192, value:����������
    QMultiMap<quint16, TRACKINDEX>	ManTrackLists;
    quint16 	ManTrackIndex;	// ��ǰ��¼����������
    QMutex		LockForManTrack;

    // ����������Ϣ
    TRACKTYPEINFOHASH m_typeInfo;

    // ����ת������
    CRadarView* m_radarView0;
    CRadarView*   m_radarView[MAXVIEWCOUNT];
    quint8  m_radarViewCount;

    // ��ʾ�������Ʊ�־
    quint8  m_defaultTrackShow;
    // �����Զ����ñ�־
    quint8  m_autoBoardAvoidFlag;

    // Ĭ�ϱ�����ʾ����
    BOARDDISP   m_defaultBoardDisp;
    // ��Ӧÿ��ģʽĬ�ϱ���ÿ����ʾ����(������ʼ��ʱʹ�ø�ֵ����ʼ������)
    quint32     m_defaultBoardLineValue[MAXBOARDMODE];
    // ���õı�����ʾ��������ֵ
    QList<quint8>   m_usableBoardLines;
    // β��ģʽ
    QHash<quint8, TRACKPOINTMODE> m_trackPointMode;
    quint8  m_defaultTrackPointMode;

    // ��Ҫ������ɫ
    quint32 m_importantColor;
    // ���亽����ɫ
    quint32 m_fadeColor;

    // ��ʾ�ɼ���ʷ�㣬ָ������ǰ���Ѿ�������ͼ��Χ��ʱ��
    // �Ƿ���ʾ����ͼ��Χ�ڵ�β��
    quint8  m_showVisiblePoint;

	// ��ʾָ����
	quint8 m_showGuidLine;
	quint8 m_guidLineFmt;
	quint8 m_guidLineLength;
};

#endif // TRACK_H

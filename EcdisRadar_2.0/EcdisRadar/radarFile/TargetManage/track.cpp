#include "track.h"
#include "radarview.h"
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtCore/QtDebug>
#include <math.h>
#include <time.h>

extern void drawSymbol(QPainter* painter, quint8 type, const QPoint& pt, quint8 sz);

#ifdef _MSVC_
template<class obj>
void listAppend(QList<obj> list1, QList<obj> list2)
{
	QList<obj>::iterator it = list2.begin(), it2 = list2.end();
	for(; it != it2; ++ it)
		list1.append(*it);
}
#endif

#if _ATA_
extern double g_ownshpLon;                     // ��������
extern double g_ownshpLat;                     // ����γ��
// ������������Է�λ�;��룬����ֵ��λ��azi�ȣ�rng����
extern void calRelativeAziRng(double lon1, double lat1, double lon2, double lat2, double &azi, double &rng);
extern void calArrivePos(double startLon, double startLat, double speed, double course,
                  unsigned int time, double& arriveLon, double& arriveLat);
// ��ȡ�������ٶȺ���
extern bool GetOwnShipCourseSpeed(float* course, float* speed);
// ��ȡcpa/tcpa����
extern quint8 GetCpaTcpaParam(float *cpa, float *dcpa, float *tcpa);
// ��ȡ����������
extern quint8 GetGuardZoneParam(float *zone);
extern bool GuardZoneAlarm(float azi, float rng);
// ��������
extern void StartAlarm(int level, const QString& msg);
// ��ȡ����ʱ�䳤��
extern int GetVecterLength();
// ��ȡ����β����ʱ��
extern int GetTrackIntervalTime();
#endif

/*
 * class Plot
 * author:moflying
 * time:2010-04-08
 */
Track::Track(quint16 maxTracks)
{
    m_maxTracks = 0;
    m_defaultBoardDisp = 0;

    m_radarView0 = 0;
    m_radarViewCount = 0;

    m_showVisiblePoint = 1;

    m_defaultTrackShow = 1;
    m_autoBoardAvoidFlag = 0;
    m_defaultTrackPointMode = 0;
    m_importantColor = QRGB(200,50,50);
    m_fadeColor = QRGB(48,48,48);

	m_showGuidLine = 1;
	m_guidLineFmt = 0;
	m_guidLineLength = 5;

    for(int i=0; i<MAXVIEWCOUNT; i++)
        m_radarView[i] = NULL;

    for(int i=0; i<MAXBOARDMODE; i++)
        m_defaultBoardLineValue[i] = 0;

    registerType(TRACK_TYPE_UNKNOWN, "unknown", QRGB(0xff, 0xff, 0xff));
    registerTrackPointMode(MANTRACKPOINTMODE, "manual", MAXTRACKPOINTS);

    if(maxTracks)
        createTracks(maxTracks);
}

Track::~Track()
{
    // ɾ�����еĺ�������
    clear(true);

    // �ͷź������Ͷ���
    unregisterType();
}

// ����ָ�������ĺ�������
void Track::createTracks (quint16 maxTracks)
{    
    if(m_maxTracks >= MAXTRACKS)
        return;
    if(m_maxTracks + maxTracks > MAXTRACKS)
        maxTracks = MAXTRACKS - m_maxTracks;

    for(quint16 i=0; i<maxTracks; i++)
    {
        const TRACKINDEX index = m_maxTracks+i;
        TRACK& track = *(new TRACK);
        track.no = 0;
        track.index = index;
        //������ʾ״̬
        track.valid = 0;	// �ɷ���״̬
        // ��������
        track.curPointPos = 0;
        track.maxPoints = 0;
        track.curPoints = 0;
        track.totalPoints = 0;
        track.hpointMode = 0;
		track.extraData = NULL;

		for(quint16 j=0; j<MAXTRACKPOINTS; j++){
            track.points[j].extraData = NULL;
		}

        // ��ӵ��ɷ�������
        m_freeIndex << index;
        m_trackTable.insert(index, &track);
    }

	m_maxTracks += maxTracks;
}

// ɾ�����к���
void Track::clear (bool deleteFlag)
{
    QMutexLocker locker(&m_mutex);
    deleteAllTracks ();

    if(deleteFlag)
    {
        qDeleteAll(m_trackTable);
        m_trackTable.clear();
        m_no_index.clear();
        m_freeIndex.clear();
        m_trackIndex.clear();
        m_mouseOverTrackIndex = -1;
        m_selectedTrackIndex = -1;
        m_maxTracks = 0;

        ManTrackLists.clear();
        ManTrackIndex = -1;
    }
}

// ��ʼ��ָ����������
void Track::initTrack (TRACK& track)
{
    track.valid = 0;
    // β��
    track.curPoints = 0;
	track.curPointPos = 0;
	track.totalPoints = 0;
    track.hpointMode = m_defaultTrackPointMode;
    track.maxPoints = trackPointSize(track.hpointMode);
    // ʹ��ϵͳ��������
    track.boardDisp = m_defaultBoardDisp;
    track.boardLineTexts.clear();

    track.source = 0;
    track.speed = 0;
    track.doplspeed = 0;
    track.course = 0;

    track.type.flag = 0;
    track.model.flag = 0;
    track.number.flag = 0;
    track.name[0] = 0;

	track.specifyColor = 0;
	track.specifySymb = 0;
	
#if _ATA_
	if(!track.extraData)
		track.extraData = (qint8*)(new ATAPARAM);
	memset(track.extraData, 0, sizeof(ATAPARAM));
#endif
}

// ɾ�����еĺ�������
void Track::deleteAllTracks ()
{
    QList<TRACKINDEX>::const_iterator it1 = m_trackIndex.constBegin(), it2 = m_trackIndex.constEnd(), it;
    for(it=it1; it!=it2; ++it)
    {
        TRACKINDEX i = *it;
        if(!isIndexValid(i))
            continue;

        TRACK& track = *(m_trackTable[i]);
        track.clearExtraData();
        track.valid = 0;
        m_no_index.remove(track.no);
    }

#ifndef _MSVC_
    m_freeIndex.append(m_trackIndex);
#else
	listAppend<qint16>(m_freeIndex, m_trackIndex);
#endif
    m_trackIndex.clear();
}

// ɾ��ָ����������
void Track::deleteTrack(TRACKINDEX i)
{
    if(!isIndexValid(i))
        return;

    TRACK& track = *(m_trackTable[i]);
    track.clearExtraData();
    track.valid = 0;
    m_no_index.remove(track.no);
    m_trackIndex.removeAll(i);
    m_freeIndex.append(i);
}

// ����һ����������
TRACKINDEX Track::applyTrack ()
{
    TRACKINDEX index = -1;

    if(m_freeIndex.empty())
    {
        createTracks(10);
    }

    if(!m_freeIndex.empty())
    {
        index = m_freeIndex.takeFirst();
        if (isIndexValid(index))
        {
            TRACK& track = *(m_trackTable[index]);
            // ��ʼ����������
            initTrack(track);
            // ������ʾ��־Ϊ��ʾ״̬
            track.valid = 1;
        }
    }
    return index;
}

/*���յ�������Ϣ������ٵ�Ԫ�����˲���(�������ʱ)����*/
void Track::setTrackInfo (const SETTRACKINFO& info)
{
    QMutexLocker locker(&m_mutex);

    // ���Ҷ�Ӧ�ĺ����������û�У��������µ�
    TRACKINDEX index = indexByNo(info.trackNo);
    if(!isIndexValid(index))
    {
        index = applyTrack();
        if(!isIndexValid(index))
            return;

        // ��������������
        m_no_index[info.trackNo] = index;
        // ���浽����������
        m_trackIndex.append(index);
    }

    TRACK& track = *(m_trackTable[index]);

    // �ӹ���ת������ǰϵͳ��������
    const float rate2km = m_radarView0->coefficientToKm();

    // ���º�������
    track.no = info.trackNo;
    track.source = getTrackPointSource(info.source);
    track.course = info.course;						// 360��
    track.speed = (int)(info.velocity / rate2km);	// ����/Сʱ
    track.extrapolatePoint = info.extrapolatePoint;

    track.trace = info.trace;   // ����״̬
    track.type = info.type;     // ��������
    track.model = info.model;   // ��������
    track.number = info.number; // ��������
    memcpy(track.name, info.name, 6);   // ��������

    // ���º����㼣λ��(�ӹ���ת������ǰϵͳ��������)
    SQUARE_POINT square_point = m_radarView0->rtheta_to_square(info.current_point);
    square_point.rx() = square_point.x() / rate2km;	// ����
    square_point.ry() = square_point.y() / rate2km;

    // �ɺ������ָ���߷���
    if(!track.boardDisp.FixedCourse)
        track.boardDisp.LineCourse = track.course / 45; // * 8 / 360;

    // ��ǰ����´���
    enterTrackPoint(track, square_point);

    // ���ú�����������
	if(info.trackExtraData)
    track.setExtraData(info.trackExtraData);
    // ���õ㼣��������
	if(info.pointExtraData)
    track.points[track.curPointPos].setExtraData(info.pointExtraData);

	//qDebug() << info.course << info.velocity << info.current_point.toPoint() << square_point.toPoint();

#if _ATA_
	updateAta(&track);
	if(3 == track.trace.Trace)
	qDebug() << __FUNCTION__ << track.no << track.trace.Trace;
#endif
    //qDebug() << "setTrackInfo ok" << track.no << track.index << square_point.toPoint() << track.source;
}

// ���ú���״̬
void Track::setTrackStatus (const SETTRACKSTATUS& trackStatus)
{
    //ȡ������
    const TRACKNO p = trackStatus.trackNo;

    QMutexLocker locker(&m_mutex);

    //Ѱ�Һ�����������
    TRACKINDEX index = indexByNo(p);
    if(!isIndexValid(index))
        return;

    TRACK& track = *(m_trackTable[index]);

    //�жϺ����ĸ���״̬
    if(trackStatus.trackStatusFlag)
        track.trace.Trace = trackStatus.trackStatus;

    //�жϺ�����Ŀ��״̬
    if(trackStatus.targetStatusFlag)
        track.trace.Target = trackStatus.targetStatus;
	
#if _ATA_
	//if(3 == track.trace.Trace)
	//qDebug() << __FUNCTION__ << track.no << track.trace.Trace;
	// ������ر�־λ
	LPATAPARAM param = (LPATAPARAM)(track.extraData);
	if(param){
	param->targetInit = (track.trace.Trace == 1 ? 1 : 0);
	param->targetLost = (track.trace.Trace == 3 ? 1 : 0);
	if(param->targetLost)
		StartAlarm(2, "Ŀ�궪ʧ");
	}
	else
		qDebug() << "track ata param is null" << track.no;
#endif
}

// �˹�������ʼ
void Track::setTrackManInit (const SETTRACKMANINIT& trackManInit)
{
    const TRACKNO p = trackManInit.trackNo;

    QMutexLocker locker(&m_mutex);

    // ���Ҷ�Ӧ�ĺ����������û�У��������µ�
    TRACKINDEX index = indexByNo(p);
    if(!isIndexValid(index))
    {
        index = applyTrack();
        if(!isIndexValid(index))
            return;

        // ��������������
        m_no_index[p] = index;
        // ���浽����������
        m_trackIndex.append(index);
    }

    TRACK& track = *(m_trackTable[index]);

    // �ӹ���ת������ǰϵͳ��������
    const float rate2km = m_radarView0->coefficientToKm();

    // ���º�������
    track.no = p;
    track.source = getTrackPointSource(trackManInit.source);
    track.trace.Mode = trackManInit.trackMode;  // ���ٷ�ʽ
    track.trace.Control = trackManInit.control; // ���Ʊ�־

    // ���º����㼣λ��(�ӹ���ת������ǰϵͳ��������)
    SQUARE_POINT square_point;
    square_point.rx() = trackManInit.square_point.x() / rate2km;
    square_point.ry() = trackManInit.square_point.y() / rate2km;

    track.extrapolatePoint = m_radarView0->square_to_rtheta(square_point);

    // �ɺ������ָ���߷���
    if(!track.boardDisp.FixedCourse)
        track.boardDisp.LineCourse = track.course / 45; // * 8 / 360;

    // ��ǰ����´���
    enterTrackPoint(track, square_point);

    // ���ú�����������
    track.setExtraData(NULL);
    // ���õ㼣��������
    track.points[track.curPointPos].setExtraData(NULL);

    // �˹���������
    if(track.trace.Mode)
    {

    }

    //�ж���ʼ��־
    if(trackManInit.start)
    {
        //���ø���״̬-������ʼ
        track.trace.Trace = 1;
        //����Ŀ��״̬
        track.trace.Target = 0;
    }
}

// ����Ŀ����������
void Track::setTrackTypeAttr (const SETTRACKTYPEATTR& trackTypeAttr)
{
    //ȡ������
    TRACKNO p = trackTypeAttr.trackNo;
    TRACKINDEX index = indexByNo(p);
    if(!isIndexValid(index))
        return;

    QMutexLocker locker(&m_mutex);

    TRACK& track = *(m_trackTable[index]);

    // ����
    if(trackTypeAttr.type.flag)
    {
        track.type.flag = 1;
        track.type.value = trackTypeAttr.type.value;
    }

    // ����
    if(trackTypeAttr.model.flag)
    {
        track.model.flag = 1;
        track.model.value = trackTypeAttr.model.value;
    }

    // ����
    if(trackTypeAttr.number.flag)
    {
        track.number.flag = 1;
        track.number.value = trackTypeAttr.number.value;
    }
}

// ���øĻ���
void Track::setChgTrackNo (const SETCHGTRACKNO& chgTrackNo)
{
    QMutexLocker locker(&m_mutex);

    //ȡ��Ŀ��1������
    TRACKNO p1 = chgTrackNo.trackNo1;
    //ȡ��Ŀ���������
    TRACKINDEX index1 = indexByNo(p1);
    //���Ŀ�겻����
    if(!isIndexValid(index1))
        return;

    //ȡ��Ŀ��2������
    TRACKNO p2 = chgTrackNo.trackNo2;
    //ȡ��Ŀ��2��������
    TRACKINDEX index2 = indexByNo(p2);
    //���Ŀ��2����(����)
    if(isIndexValid(index2))
    {   //���л�������
        m_trackTable[index2]->no = p1;
        m_no_index[p1] = index2;
    }
    else
    {   // ����,��ʱ��Ҫɾ��ԭ����������
        m_no_index.remove(p1);
    }

    m_trackTable[index1]->no = p2;
    m_no_index[p2] = index1;

    LPTRACK track;
    if(track = getTrack(index1))
        track->updateBoardText = 1;
    if(track = getTrack(index2))
        track->updateBoardText = 1;
}

// ����Ŀ��ɾ��
void Track::setTrackDelete (const SETTRACKDELETE& trackDelete)
{
    QMutexLocker locker(&m_mutex);

    //ȡ������
    TRACKNO p = trackDelete.trackNo;
    if(p==0)
    {   // �������Ϊ0ɾ��������
        deleteAllTracks();
    }
    else
    {   // ɾ��Ŀ��
        deleteTrack(indexByNo(p));
    }
}

// ���º�����ǰ�㼣
void Track::enterTrackPoint(TRACK& track, const SQUARE_POINT& sq)
{
	const int crnttime = time(0);

	TRACKPOINT& trackpoint = track.crntPoint;//track.points[track.curPointPos];
    trackpoint.source = track.source;
	trackpoint.fadeing = (track.trace.Trace == 3 ? 1 : 0);
	trackpoint.updateTime = crnttime;

    // ���µ�ǰ��λ��
    PLOTPOSITION& position = trackpoint.position;
    position.updateFlag = 0;
    position.square_point = sq;
    position.latitude_point = m_radarView0->square_to_latitude(position.square_point);
    position.rtheta_point = m_radarView0->square_to_rtheta(position.square_point);
    for(int i=0;i<m_radarViewCount; i++)
    {
        position.screen_point[i] = m_radarView[i]->squaretoscreen_view(position.square_point);
        //if(m_radarView[i]->screenEnvelope().contains(position.screen_point[i].toPoint()))
        if(m_radarView[i]->isPointDisplay(position))
            position.displayFlag |= (1 << i);
    }

#if _ATA_
	// ����㼣����ʱ����С��30�룬�򲻼�¼��ǰ�㼣
	const int trackInterval = GetTrackIntervalTime();
	if(track.curPoints == 0 || (trackInterval > 0 && crnttime - track.points[track.curPointPos].updateTime >= trackInterval))
	{
		track.enterTrackPoint();
		track.points[track.curPointPos] = trackpoint;
	}
#endif

    // ������Ҫ���±��Ʊ�־
    track.updateBoardText = 1;
}

/*��������Ŀ�����Ļ����*/
void Track::updateScreenPoint()
{
    QMutexLocker lock(&m_mutex);

    quint32 displayflag[MAXVIEWCOUNT];
    for(int i=0;i<MAXVIEWCOUNT; i++)
        displayflag[i] = (1<<i);

    // ��ÿһ���㼣����,��������Ļ����
    QList<TRACKINDEX>::const_iterator it1 = m_trackIndex.constBegin(), it2 = m_trackIndex.constEnd(), it;
    for(it=it1; it!=it2; ++it)
    {
        if(!isTrackValid(*it))
            continue;

        TRACK& track = *(m_trackTable[*it]);

        const int count = track.curPoints;
        int pos = track.curPointPos;
        for (int cnt=0; cnt<count; cnt++, pos = PrevTrackPointIndex(pos))
        {            
            PLOTPOSITION& position = track.points[pos].position;
            position.displayFlag = 0;
            for(int i=0; i<m_radarViewCount; i++)
            {
                position.screen_point[i] = m_radarView[i]->squaretoscreen_view(position.square_point);
                //if(m_radarView[i]->screenEnvelope().contains(position.screen_point[i].toPoint()))
                if(m_radarView[i]->isPointDisplay(position))
                    position.displayFlag |= displayflag[i];
            }
        }
        
        PLOTPOSITION& position = track.crntPoint.position;
        position.displayFlag = 0;
        for(int i=0; i<m_radarViewCount; i++)
        {
            position.screen_point[i] = m_radarView[i]->squaretoscreen_view(position.square_point);
            //if(m_radarView[i]->screenEnvelope().contains(position.screen_point[i].toPoint()))
            if(m_radarView[i]->isPointDisplay(position))
                position.displayFlag |= displayflag[i];
        }
    }
}

/*��������Ŀ�����Ļ����*/
void Track::updateScreenPoint(quint8 idx)
{
    if(idx >= m_radarViewCount)
        return;

    //qDebug() << "Track::updateScreenPoint" << idx;

    QMutexLocker lock(&m_mutex);

    quint32 displayflag = (1 << idx);

    // ��ÿһ���㼣����,��������Ļ����
    QList<TRACKINDEX>::const_iterator it1 = m_trackIndex.constBegin(), it2 = m_trackIndex.constEnd(), it;
    for(it=it1; it!=it2; ++it)
    {
        if(!isTrackValid(*it))
            continue;

        TRACK& track = *(m_trackTable[*it]);

        const int count = track.curPoints;
        int pos = track.curPointPos;
        for (int cnt=0; cnt<count; cnt++, pos = PrevTrackPointIndex(pos))
        {
            PLOTPOSITION& position = track.points[pos].position;
			//position.square_point = m_radarView[idx]->latitude_to_square(position.latitude_point);
			//position.rtheta_point = m_radarView[idx]->square_to_rtheta(position.square_point);
            position.screen_point[idx] = m_radarView[idx]->squaretoscreen_view(position.square_point);
            //if(m_radarView[idx]->screenEnvelope().contains(position.screen_point[idx].toPoint()))
            if(m_radarView[idx]->isPointDisplay(position))
                position.displayFlag |= displayflag;
            else
                position.displayFlag &= (~displayflag);
        }

		PLOTPOSITION& position = track.crntPoint.position;
        position.screen_point[idx] = m_radarView[idx]->squaretoscreen_view(position.square_point);
        if(m_radarView[idx]->isPointDisplay(position))
            position.displayFlag |= displayflag;
        else
            position.displayFlag &= (~displayflag);

		//updateAta(&track, idx);
    }
}

/*����ָ����������Ļ����*/
void Track::updateTrackScreenPoint(LPTRACK track0)
{
    QMutexLocker lock(&m_mutex);
    if(track0)
    implUpdateTrackScreenPoint(track0);
}
void Track::implUpdateTrackScreenPoint(LPTRACK track0)
{
    quint32 displayflag[MAXVIEWCOUNT];
    for(int i=0;i<MAXVIEWCOUNT; i++)
        displayflag[i] = (1<<i);

    TRACK& track = *track0;
    const int count = track.curPoints;
    int pos = track.curPointPos;
    for (int cnt=0; cnt<count; cnt++, pos = PrevTrackPointIndex(pos))
    {
        PLOTPOSITION& position = track.points[pos].position;
        position.displayFlag = 0;
        for(int i=0; i<m_radarViewCount; i++)
        {
            position.screen_point[i] = m_radarView[i]->squaretoscreen_view(position.square_point);
            //if(m_radarView[i]->screenEnvelope().contains(position.screen_point[i].toPoint()))
            if(m_radarView[i]->isPointDisplay(position))
                position.displayFlag |= displayflag[i];
        }
    }
	updateAta(&track, 0);
}

/*��������Ŀ���ֱ���������Ļ���꣬�������ٱ仯ʱ����*/
void Track::updateSquareAndScreenPoint()
{
    QMutexLocker lock(&m_mutex);

    quint32 displayflag[MAXVIEWCOUNT];
    for(int i=0;i<MAXVIEWCOUNT; i++)
        displayflag[i] = (1<<i);

    // ��ÿһ���㼣����,������ֱ�����ꡢ�����ꡢ��Ļ����
    QList<TRACKINDEX>::const_iterator it1 = m_trackIndex.constBegin(), it2 = m_trackIndex.constEnd(), it;
    for(it=it1; it!=it2; ++it)
    {
        if(!isTrackValid(*it))
            continue;

        TRACK& track = *(m_trackTable[*it]);

        const int count = track.curPoints;
        int pos = track.curPointPos;
        for (int cnt=0; cnt<count; cnt++, pos = PrevTrackPointIndex(pos))
        {
            PLOTPOSITION& position = track.points[pos].position;
            position.displayFlag = 0;
            for(int i=0; i<m_radarViewCount; i++)
            {
                position.square_point = m_radarView[i]->latitude_to_square(position.latitude_point);
                position.rtheta_point = m_radarView[i]->square_to_rtheta(position.square_point);
                position.screen_point[i] = m_radarView[i]->squaretoscreen_view(position.square_point);
                //if(m_radarView[i]->screenEnvelope().contains(position.screen_point[i].toPoint()))
                if(m_radarView[i]->isPointDisplay(position))
                    position.displayFlag |= displayflag[i];
            }
        }

		updateAta(&track, 0);
    }
}

/*Ŀ��λ���ƶ�����*/
void Track::move (const QPointF& sq, const QList<QPoint>& sc, quint8 index)
{
    QMutexLocker lock(&m_mutex);

    quint32 displayflag[MAXVIEWCOUNT];
    for(int i=0;i<MAXVIEWCOUNT; i++)
        displayflag[i] = (1<<i);

    // ��ÿһ���㼣����,�����ƶ���ֱ�����ꡢ��Ļ����
    QList<TRACKINDEX>::const_iterator it1 = m_trackIndex.constBegin(), it2 = m_trackIndex.constEnd(), it;
    for(it=it1; it!=it2; ++it)
    {
        if(!isTrackValid(*it))
            continue;

        TRACK& track = *(m_trackTable[*it]);

        const int count = track.curPoints;
        int pos = track.curPointPos;
        for (int cnt=0; cnt<count; cnt++, pos = PrevTrackPointIndex(pos))
        {
            PLOTPOSITION& position = track.points[pos].position;
            position.square_point += sq;
            position.rtheta_point = m_radarView0->square_to_rtheta(position.square_point);
            for(int i=0,j=0; i<m_radarViewCount; i++)
            {
                if(index & displayflag[i])
                {
                    position.screen_point[i] += sc[j];
                    //if(m_radarView[i]->screenEnvelope().contains(position.screen_point[i].toPoint()))
                    if(m_radarView[i]->isPointDisplay(position))
                        position.displayFlag |= displayflag[i];
                    else
                        position.displayFlag &= (~displayflag[i]);
                    j++;
                }
            }
        }
    }
}

// ע�ẽ������(�����ڳ����ʼ��ʱһ��ע�������еĺ�������)
bool Track::registerType(quint8 type, const QString& name, quint32 color, quint8 symbol, quint8 symsize)
{
    LPTRACKTYPEINFO info = m_typeInfo.value(type, NULL);
    if(!info)
    {
        info = new TRACKTYPEINFO;
        info->flags = 0xff;
        info->type = type;
        info->name = name;
        info->color = color;
        info->symbol = symbol;
        info->symbolSize = symsize;

        m_typeInfo.insert(type, info);
        //qDebug() << "registerType:" << type << name << color;
        return 1;
    }

    return 0;
}

// ���ú������ͱ�־
void Track::setTrackTypeFlag(quint8 type, quint8 flags)
{
    if(LPTRACKTYPEINFO info = m_typeInfo.value(type, NULL))
    {
        info->flags = flags;
    }
}

// ���ú���������ɫ
void Track::setTrackTypeColor(quint8 type, quint32 color)
{
    if(LPTRACKTYPEINFO info = m_typeInfo.value(type, NULL))
    {
        info->color = color;
    }
}

// ��ȡ����������ɫ
quint32 Track::trackTypeColor(quint8 type) const
{
    if(LPTRACKTYPEINFO info = m_typeInfo.value(type, NULL))
        return info->color;
    else
        return QRGB(0xff, 0xff, 0xff);
}

// ���ú�����������
void Track::setTrackTypeName(quint8 type, const QString& name)
{
    if(LPTRACKTYPEINFO info = m_typeInfo.value(type, NULL))
    {
        info->name = name;
    }
}

// ��ȡ������������
QString Track::trackTypeName(quint8 type) const
{
    if(LPTRACKTYPEINFO info = m_typeInfo.value(type, NULL))
        return info->name;
    else
        return QString("Unknown");
}

// ���ú������ͷ���
void Track::setTrackTypeSymbol(quint8 type, quint8 symbol)
{
    if(LPTRACKTYPEINFO info = m_typeInfo.value(type, NULL))
    {
        info->symbol = symbol;
    }
}

// ��ȡ�������ͷ���
quint8 Track::trackTypeSymbol(quint8 type) const
{
    if(LPTRACKTYPEINFO info = m_typeInfo.value(type, NULL))
        return info->symbol;
    else
        return SYMBOL_NONE;
}

// ���ú������Ŵ�С
void Track::setTrackTypeSymbolSize(quint8 type, quint8 size)
{
    if(LPTRACKTYPEINFO info = m_typeInfo.value(type, NULL))
    {
        info->symbolSize = size;
    }
}

// ��ȡ�������Ŵ�С
quint8 Track::trackTypeSymbolSize(quint8 type) const
{
    if(LPTRACKTYPEINFO info = m_typeInfo.value(type, NULL))
        return info->symbolSize;
    else
        return 0;
}

// ɾ����������
void Track::unregisterType(quint8 type)
{
    if(type != TRACK_TYPE_UNKNOWN && m_typeInfo.contains(type))
    {
        LPTRACKTYPEINFO info = m_typeInfo.take(type);
        delete info;
    }
}

// ɾ���������Ͷ���
void Track::unregisterType()
{
    DeleteObjectInContainer(TRACKTYPEINFOHASH, m_typeInfo);
}

// �ж�ĳһ���͵ĺ����Ƿ����ʾ
bool Track::isTrackShow (quint8 type) const
{
    if(LPTRACKTYPEINFO info = m_typeInfo.value(type, NULL))
        return info->displayFlag;
    else
        return false;
}

bool Track::isTrackShow (LPTRACK track) const
{
    return isTrackShow(track->source);
}

// ����ָ����������
void Track::updateBoard (LPTRACK track)
{
    // ���º��������ַ���
    updateBoardText (track);
    // ���º�������ָ����
    updateBoardGuildLine(track);
}

// ���ú���������ʾģʽ(�ı������ʾģʽ)
void Track::setTrackBoardMode(quint8 mode, LPTRACK track)
{
    QMutexLocker locker(&m_mutex);
    BOARDDISP &board = (track ? (track->boardDisp) : m_defaultBoardDisp);
    if(mode < MAXBOARDMODE && board.BoardMode != mode)
    {
        board.BoardMode = mode;
        board.LineValue = m_defaultBoardLineValue[mode];
        // ������޸�ϵͳ����ģʽ,����Ҫ�޸ĵ�ǰ���к����ı���ģʽ
        if(!track)
        {
            QList<TRACKINDEX>::const_iterator it1 = m_trackIndex.constBegin(), it2 = m_trackIndex.constEnd(), it;
            for(it=it1; it!=it2; ++it)
            {
                TRACK& track0 = *(m_trackTable[*it]);
                BOARDDISP &board0 = track0.boardDisp;
                board0.BoardMode = mode;
                board0.LineValue = m_defaultBoardLineValue[mode];
                track0.updateBoardText = 1;
            }
        }
        else
        {
            track->updateBoardText = 1;
        }
    }
}

// �л��������һ����ʾ������
void Track::switchBoardLastLine(LPTRACK track)
{
    LPBOARDDISP boardDisp = &(track->boardDisp);
    if(boardDisp->MaxLines == 0 || boardDisp->MaxLines >= m_usableBoardLines.size())
        return;

    const int lastLine = boardDisp->MaxLines - 1;
    quint8 linev0 = boardDisp->boardLineValue(lastLine);

    QList<quint8>::const_iterator it_bgn = m_usableBoardLines.constBegin();
    QList<quint8>::const_iterator it_end = m_usableBoardLines.constEnd();
    QList<quint8>::const_iterator it_0, it;
    it_0 = qFind(it_bgn, it_end, linev0);

    if(it_0 == it_end)
        return;

    for(it=it_0+1; it!=it_end; ++it)
    {
        if(!boardDisp->isBoardContainsLineValue(*it))
        {
            boardDisp->setBoardLineValue(lastLine, (*it));
            track->updateBoardText = 1;
            return;
        }
    }

    for(it=it_bgn; it!=it_0; ++it)
    {
        if(!boardDisp->isBoardContainsLineValue(*it))
        {
            boardDisp->setBoardLineValue(lastLine, (*it));
            track->updateBoardText = 1;
            return;
        }
    }
}

// ���±����ַ���
void Track::updateBoardText (LPTRACK track)
{
    if(track->updateBoardText)
    {        
        track->boardLineTexts.clear();
        BOARDDISP& board = track->boardDisp;
        const quint8 lines = board.MaxLines;
        QString lineText;
        for(quint8 i=0; i<lines; i++)
        {
            lineText = formBoardText(track, board.boardLineValue(i));
            if(!lineText.isEmpty())
                track->boardLineTexts.append(lineText);
        }
        track->updateBoardText = 0;
        //qDebug() << "updateBoardText" << board.LineValue << board.MaxLines << track->boardLineTexts;
    }
}

QString Track::formBoardText(LPTRACK track, quint8 linevalue)
{
    QString text;
    switch(linevalue)
    {
    case BOARDDISP::LINE_TRACKNO:  // ��������
    {
        text = QString("%1%2%3%4")\
            .arg(track->trace.Mode ? "M" : "T")\
            .arg(track->no, 3, 10, QLatin1Char('0'))\
            .arg("")\
            .arg("");
        break;
    }
    case BOARDDISP::LINE_HEIGHT:
    {
        const int height = track->points[track->curPointPos].height;
        if(height)
            text = QString("H%1").arg(height, 3, 10, QLatin1Char('0'));
        break;
    }
    case BOARDDISP::LINE_SPEED:
    {
        text = QString("V%1").arg((int)track->speed, 3, 10, QLatin1Char('0'));
        break;
    }
    case BOARDDISP::LINE_COURSE:
    {
        text = QString("K%1").arg((int)track->course, 3, 10, QLatin1Char('0'));
        break;
    }
    case BOARDDISP::LINE_ALIAS:
    {
        //text = QString("K%1").arg(track->);
        break;
    }
    }

    return text;
}

// ��תָ���ߵķ���
void Track::rotateBoardCourse (LPTRACK track)
{
    track->boardDisp.autoRotate();
}

// ���ú���ָ�����߳�
void Track::setLineLength (LPTRACK track, quint8 mode)
{
    track->boardDisp.setLineLength(mode);
}

// �˹�����ָ����, ���Ⱥͷ���
void Track::manGuildLine (LPTRACK track, int length, int course)
{
    track->boardDisp.manGuildLine(length, course);
}

// ���㺽��ָ���ߵĳ���
int Track::guildLineLength (LPTRACK track, quint8 view)
{
    if (track)
    {
        BOARDDISP& board = track->boardDisp;
        if (board.ManLine)
            // �˹��߳�
            return board.ManLineLength;
        else if(board.FixedLength)
            // �̶��߳�
            return (board.LineLength) * 20;
        else
            // �Զ������߳�
		    {   
#if _ATA_
                const float rationTep = m_radarView[view]->ration();
		        const int time = GetVecterLength();
				return track->speed / 3.6 / m_radarView[view]->coefficientToKm() * time * rationTep;
#endif
				return 40;
		    }
            //return (track->speed / 30);// * m_radarView[view]->ration();
    }
    else
    {
        return 40;
    }
}

// ���㺽��ָ���ߵķ���
float Track::guildLineCourse (LPTRACK track)
{
    if (track)
    {
        BOARDDISP& board = track->boardDisp;
        if (board.ManLine)
            // �˹�ָ���߷���
            return ((float)board.ManLineCourse) * M_2PI / 1024.0;
        else
            // �̶�ָ���߷���
            return ((float)board.LineCourse) * M_2PI / 8.0f;
    }
    else
    {
        return 0.0f;
    }
}

// ���º�������ָ����
void Track::updateBoardGuildLine(LPTRACK track)
{
}
// ���ú������������С
void Track::setBoardFontSize(LPTRACK track, quint8 fontsize)
{
    track->boardDisp.FontSize = fontsize;
}

quint8 Track::boardFontSize(LPTRACK track)
{
    return track->boardDisp.FontSize;
}

// ��ʾ���Ʊ߿�
void Track::showBoardBorder(LPTRACK track, bool show)
{
    track->boardDisp.ShowBorder = (show?1:0);
}

// ע�ẽ��β��ģʽ
void Track::registerTrackPointMode(quint8 mode, const QString& name, quint16 pointcount)
{
    TRACKPOINTMODE trackpoint;
    trackpoint.mode = mode;
    trackpoint.name = name;
    trackpoint.pointCount = pointcount;
    m_trackPointMode[mode] = trackpoint;
}

// ����β��ģʽ
void Track::setTrackPointMode(quint8 mode, LPTRACK track)
{
    QMutexLocker lock(&m_mutex);

    if(!m_trackPointMode.contains(mode))
        return;

    const quint16 pointcount = m_trackPointMode[mode].pointCount;
    if(track)
    {   // ���õ�ǰ����β��ģʽ��β������
        setTrackPointModeAndSize(track, mode, pointcount);
    }
    else
    {   // ����ϵͳβ��ģʽ�����޸����к�����β��ģʽ������
        setTrackPointModeAndSize(mode, pointcount);
    }
}

// �˹�����β������
void Track::setManTrackPointSize(quint16 size, LPTRACK track)
{
    QMutexLocker lock(&m_mutex);

    if(track)
        setTrackPointModeAndSize(track, MANTRACKPOINTMODE, size);
    else
        setTrackPointModeAndSize(MANTRACKPOINTMODE, size);
}

// �л�β��ģʽ
void Track::switchTrackPointMode(LPTRACK track)
{
    QMutexLocker lock(&m_mutex);

    // Ѱ����һ��ģʽ
    if(m_trackPointMode.size() <= 2)
        return;
    QHash<quint8, TRACKPOINTMODE>::const_iterator it=m_trackPointMode.find(m_defaultTrackPointMode);
    do {
    if(it == m_trackPointMode.constEnd())
        it = m_trackPointMode.constBegin();
    if(it.key() == MANTRACKPOINTMODE)
        ++it;
    } while(it == m_trackPointMode.constEnd() || it.key() == MANTRACKPOINTMODE);

    // ����ϵͳβ��ģʽ�Ͷ�Ӧ��β������
    TRACKPOINTMODE mode = it.value();
    if(track)
        setTrackPointModeAndSize(track, it.key(), mode.pointCount);
    else
        setTrackPointModeAndSize(it.key(), mode.pointCount);
}

// ��ȡβ��ģʽ
quint8 Track::trackPointMode(LPTRACK track) const
{
    return (track?track->hpointMode:m_defaultTrackPointMode);
}

QString Track::trackPointModeName(LPTRACK track) const
{
    return m_trackPointMode[trackPointMode(track)].name;
}

// ���ݺ���β��ģʽ����ȡ��Ӧ��β������
quint16 Track::trackPointSize(quint8 mode)
{
    QHash<quint8, TRACKPOINTMODE>::const_iterator it=m_trackPointMode.find(mode);
    if(it == m_trackPointMode.constEnd())
        return 0;
    else
        return (*it).pointCount;
}

// ���ú���β������
void Track::setTrackPointModeAndSize(quint8 mode, quint16 pointcount)
{
    m_defaultTrackPointMode = mode;
    QList<TRACKINDEX>::const_iterator it1 = m_trackIndex.constBegin(), it2 = m_trackIndex.constEnd(), it;
    for(it=it1; it!=it2; ++it)
    {
        setTrackPointModeAndSize(m_trackTable[*it], mode, pointcount);
    }
}

// ���ú���β������
void Track::setTrackPointModeAndSize(LPTRACK track, quint8 mode, quint16 size)
{
    if(track->hpointMode != mode || track->maxPoints != size)
    {
        track->hpointMode = mode;

        if (size > MAXTRACKPOINTS)
            track->maxPoints = MAXTRACKPOINTS;
        else
            track->maxPoints = size;

        // ���µ�ǰ��ʷ��������
        if (size > track->totalPoints)
            track->curPoints = track->totalPoints;
        else
            track->curPoints = size;

        implUpdateTrackScreenPoint (track);
    }
}

// ���������С
void Track::setFontSize (QPainter* p, quint8 size)
{
    QFont myfont = p->font();
    switch (size)
    {
    case 0:		// ��С
        myfont.setPointSize (9);
        myfont.setBold(false);
        break;
    case 2:		// �ϴ�
        myfont.setPointSize (14);
        myfont.setBold(true);
        break;
    case 3:		// �ܴ�
        myfont.setPointSize (16);
        myfont.setBold(true);
        break;
    default:	// ����
        myfont.setPointSize (11);
        myfont.setBold(false);
        break;
    }
    p->setFont (myfont);
}

// ���㺽���������ڵ���ʾ����
QRect Track::getBoardRect (QPainter* p, LPTRACK track, int x2, int y2)
{
    // ���±����ı�
    updateBoardText (track);

    // ���������С
    setFontSize (p, track->boardDisp.FontSize);

    QFontMetrics fm (p->font());
    //const int ascent = fm.ascent ();
    const int lineHeight = fm.height ();

    int boardWidth = 0, boardHeight = 0;
    QStringList::iterator it1 = track->boardLineTexts.begin(), it2 = track->boardLineTexts.end();
    for (; it1 != it2; ++ it1)
    {
        boardWidth = qMax (boardWidth, fm.width(*it1));
        boardHeight += lineHeight;
    }

    if(boardWidth == 0 || boardHeight == 0)
    {
        boardWidth = boardHeight = 20;
    }

    //const float lineCourse = track->LineMask1.Course * 360 / 8.0f;
    const float lineCourse = guildLineCourse(track) * 360 / M_2PI;
    const bool upFlag = !(lineCourse >= 90 && lineCourse <= 270);

    if (upFlag)
            y2 -= boardHeight;

    //if (leftFlag)
    //	x2 -= lineWidth;

    x2 -= boardWidth/2;

    //  ��ȡ�߿�
    QRect rc(0, 0, boardWidth, boardHeight);
    rc.translate (x2, y2);

    return rc;
}

// �������еĺ�������
void Track::paint(QPainter* painter, quint8 view)
{
    if(!isTrackShow())
        return;

    if(view >= m_radarViewCount)
        return;

    QMutexLocker lock(&m_mutex);

	painter->setBrush(Qt::NoBrush);
    QList<TRACKINDEX>::const_iterator it1 = m_trackIndex.constBegin(), it2 = m_trackIndex.constEnd(), it;
    for(it=it1; it!=it2; ++it)
    {
        LPTRACK track = (m_trackTable[*it]);
		if(isTrackShow(track)){
#if _ATA_
			paintAta(painter, view, track);
#else
            paintTrack(painter, view, track);
#endif
		}
    }
}

void Track::cpaAlarm()
{
#if _ATA_
    QMutexLocker lock(&m_mutex);

    // ��ÿһ���㼣����,��������Ļ����
    QList<TRACKINDEX>::const_iterator it1 = m_trackIndex.constBegin(), it2 = m_trackIndex.constEnd(), it;
    for(it=it1; it!=it2; ++it)
    {
        if(isTrackValid(*it))
			cpaAlarm(m_trackTable[*it]);
	}
#endif
}

void Track::cpaAlarm(LPTRACK track)
{
#if _ATA_
	LPATAPARAM param = (LPATAPARAM)(track->extraData);
	if(!param)	
		return;

	const int view = 0;
	const RTHETA_POINT track_rtpt = track->crntPoint.position.rtheta_point;
	const float crntMaxRange = m_radarView[view]->range();

	// ������ر�־λ
	param->dcpaFlag = param->tcpaFlag = 0;
	param->guardZoneFlag = 0;
	param->targetInit = (track->trace.Trace == 1 ? 1 : 0);
	param->targetLost = (track->trace.Trace == 3 ? 1 : 0);

	float cpa, dcpa, tcpa;
	const quint8 cpaflag = GetCpaTcpaParam( &cpa, &dcpa, &tcpa);
	if((cpaflag & 0x02) && dcpa > 0.001 && param->dcpa > 0 && param->dcpa < dcpa * 100)
		param->dcpaFlag = 1;
	if((cpaflag & 0x04) && tcpa > 0.001 && param->tcpa > 0 && param->tcpa < tcpa * 100)
		param->tcpaFlag = 1;
	
	if(track_rtpt.r() < crntMaxRange && GuardZoneAlarm(track_rtpt.theta() * 180.0 / M_PI, track_rtpt.r()))
	{
		param->guardZoneFlag = 1;
	}

	// ��������
	if(param->dcpaFlag || param->tcpaFlag)
	{
		StartAlarm(2, param->dcpaFlag ? "DCPAĿ�걨��" : "TCPAĿ�걨��");
	}
	else if(param->guardZoneFlag)
	{
		StartAlarm(2, "������Ŀ�걨��");
	}
	else if(param->targetLost)
	{
		StartAlarm(2, "Ŀ�궪ʧ");
	}
#endif
}

// ����ATA����
void Track::updateAta(LPTRACK track, int view)
{
#if _ATA_
	LPATAPARAM param = (LPATAPARAM)(track->extraData);
	if(!param)	
	{
		qDebug() << __FUNCTION__ << track->no << "track ata param is null";
		return;
		//exit(0);
	}

	const double degree2radian = M_PI/180.0;
	const float rotation = m_radarView[view]->rotation();

	// ������Ա�����λ������
	double azi, rng;
	const PLOTPOSITION& trackPos = track->crntPoint.position;
	const RTHETA_POINT track_rtpt = trackPos.rtheta_point;
	const float trackLon = trackPos.latitude_point.longitude() / degree2radian;	// ��λ����
	const float trackLat = trackPos.latitude_point.latitude() / degree2radian;
	calRelativeAziRng(g_ownshpLon, g_ownshpLat, trackLon, trackLat, azi, rng);
	param->relativeRng = (ushort)(rng * 100);
	param->relativeAzi = (ushort)(azi * 10);

	/* ����DCPA��TCPA */	
	float ownshipCourse, ownshipSpeed;
	const bool ownshipFlag = GetOwnShipCourseSpeed(&ownshipCourse, &ownshipSpeed);
	const bool trackSpeedValid = (track->speed > 0 && track->speed < 100);
	if( ownshipFlag || trackSpeedValid)
	{
		// ����λ�ӽǶ�ת��Ϊ����
		const double dazi = azi * degree2radian;
		//qDebug() << "cal navigate info:" << ownInfo.speed << ownInfo.course << aisInfo.speed << aisInfo.course;
		// �����������ٶȺ���
		const double V0 = ownshipSpeed, C0 = ownshipCourse * degree2radian;
		const double Vt = track->speed, Ct = track->course * degree2radian;
		const double sinc0 = sin(C0), cosc0 = cos(C0), sinct = sin(Ct), cosct = cos(Ct);
		const double Vrx = Vt*sinct - V0*sinc0, Vry = Vt*cosct - V0*cosc0;
		// ��������˶�ʸ���ĽǶ�
		const double Cr = atan2(Vrx, Vry);
		const double Vr = sqrt(Vrx*Vrx + Vry*Vry);

		param->relativeSpeed = (ushort)(Vr * 100);
		param->relativeCourse = (ushort)(Cr / degree2radian * 10);

		if(Vr >= 0.01){
			double theta = Cr - dazi - M_PI;
			param->dcpa = qAbs(rng * sin(theta)) * 100.0;
			param->tcpa = qAbs(rng * cos(theta)/Vr) * 60 + 0.5;
		}
		else{
			param->dcpa = 1.0e10;
			param->tcpa = 0xffffffff;
			param->dcpaFlag = param->tcpaFlag = 0;
		}
		//qDebug() << track->no << Cr / degree2radian << Vr << rng << param->dcpa << param->tcpa;
	}
	else
	{
		param->relativeSpeed = 0;
		param->relativeCourse = 0;
		param->cpa = 1.0e10;
		param->tcpa = 0xffff;
		param->dcpaFlag = param->tcpaFlag = 0;
	}

	//qDebug() << trackLon << trackLat << g_ownshpLon << g_ownshpLat;

	// ���˶��ٶȺͺ���
	param->trueSpeed = (ushort)(track->speed * 100);
	param->trueCourse = (ushort)(track->course * 10);

	// cpa����
	cpaAlarm(track);
#endif
}

// ��ʾATAģʽĿ��
void Track::paintAta(QPainter* painter, quint8 view, LPTRACK track)
{
#if _ATA_
	extern int GetAtaDispIndex(int index);
	extern int GetAtaSymbolSize();

    const quint32 displayFlag = (1<<view);
	TRACKPOINT& trackPoint = track->crntPoint;//points[track->curPointPos];
	const QPoint scpt = trackPoint.position.screen_point[view].toPoint();
    const bool showFirstPoint = (trackPoint.position.displayFlag & displayFlag);
    // ����׵㲻�ɼ������Ҳ���ʾ�ɼ���ʷ�㣬���˳�
    if((!showFirstPoint) && (!m_showVisiblePoint))
        return;

	const float rotation = m_radarView[view]->rotation();
	const bool trackSpeedValid = (track->speed > 0 && track->speed < 100);

	// ��������������ʾ
	extern const quint16 * CurrentRange();
    const quint16 * lpCurrentRange = CurrentRange();
	const int dispAzi = 450+(trackPoint.position.rtheta_point.theta()+rotation) * 450.0 / M_2PI;
	if(trackPoint.position.rtheta_point.r() >= lpCurrentRange[dispAzi%450])
		return;

	// ���û���(��\����)����ɫ
	quint32 trackcolor;
	if(track->specifyColor)
		trackcolor = track->specifyColor;
	else
		trackcolor = trackTypeColor(track->source);
    painter->setPen (QColor(trackcolor));

	// ���÷������� Ŀ��ѡ��(12) > CPA/TCPA(8) > ������(7) > Ŀ�궪ʧ(9) > Ŀ����ʼ(3)
	int symbol = SYMBOL_ATA_4;
	LPATAPARAM param = (LPATAPARAM)(track->extraData);
	const int boxIndex = GetAtaDispIndex(track->index);
	const int symbolSize = GetAtaSymbolSize();
	if(param) {
		if(param->dcpaFlag || param->tcpaFlag) symbol = SYMBOL_ATA_8;
		else if(param->guardZoneFlag) symbol = SYMBOL_ATA_7;
		else if(param->targetLost) symbol = SYMBOL_ATA_9;
		else if(param->targetInit) symbol = SYMBOL_ATA_3;
		else if(boxIndex > 0)	symbol = SYMBOL_ATA_12;
	}
	drawSymbol(painter, symbol, scpt, symbolSize);

	if(boxIndex > 0)
		painter->drawText(scpt.x()+1.5*symbolSize-1, scpt.y()+symbolSize, QString::number(boxIndex));

	// ���������Ķ˵�
	param->predict_x = param->predict_y = 0;
	if(trackSpeedValid)
	{
		const int veclength = GetVecterLength();
		QPoint pt1;
		int dispLength;
		if(veclength < 5)
		{
		const int times[] = {1, 3, 6, 10, 15, 20};
		const int tm = times[veclength];
		dispLength = track->speed * (float)tm / 60.0 * m_radarView[view]->ration();
		}
		else
		{	// ʹ�ù̶��߳�	
			dispLength = 40;
		}
		
		const float course = track->course * M_PI / 180.0 - rotation;
		pt1 = scpt + QPoint(dispLength * sin(course), - dispLength * cos(course));
		
		// ������ٶ�����
		//if(param && param->predict_x>0 && param->predict_y>0)
		painter->drawLine(scpt, pt1);
	}

    // 4.��ʾβ��(�ӵڶ��㿪ʼ)
    const int count = track->curPoints;
    int index = track->curPointPos;//PrevTrackPointIndex(track->curPointPos);

	QPoint lastPoint, crntPoint;
    {   // ��ʾСԲ��
        {
            /*����β��ʹ��ͬһ��ɫ*/
            painter->setBrush (QBrush(trackcolor));
            for (int i=1; i<count; ++i)
            {
                if(track->points[index].position.displayFlag & displayFlag)
                {
                //drawHistoryPoint (painter, track->points[index].position.screen_point[view].toPoint());
				crntPoint = track->points[index].position.screen_point[view].toPoint();
                drawHistoryPoint(painter, crntPoint);
                }
                // ȡǰһ������
                index = PrevTrackPointIndex(index);
            }
        }
    }

	// ���±������ڵ�����
	track->boardRect[view] = QRect(scpt.x()-symbolSize, scpt.y()-symbolSize, 2*symbolSize, 2*symbolSize);
#endif
}

// ����һ����������
void Track::paintTrack(QPainter* painter, quint8 view, LPTRACK track)
{
    const quint32 displayFlag = (1<<view);
    TRACKPOINT& trackPoint = track->points[track->curPointPos];
    const bool showFirstPoint = (trackPoint.position.displayFlag & displayFlag);
    // ����׵㲻�ɼ������Ҳ���ʾ�ɼ���ʷ�㣬���˳�
    if((!showFirstPoint) && (!m_showVisiblePoint))
        return;

    // ��������
    LPTRACKTYPEINFO typeinfo = m_typeInfo.value(track->source, NULL);
    if(!typeinfo)
    {
        typeinfo = m_typeInfo.value(TRACK_TYPE_UNKNOWN, NULL);
        if(!typeinfo)
            return;
    }

    // ���º��������ַ�
    updateBoardText(track);

    // ��ȡ����������ز���
    const quint8 trackSymbol = track->specifySymb ? track->specifySymb : typeinfo->symbol;
    const quint8 trackTrailDispMode = typeinfo->tailDispMode;
    const quint8 trackSymbolSize = typeinfo->symbolSize;

    quint32 trackcolor;
    bool fixedColorFlag = false;
    // �������Ҫ����˥�䣬��ʹ�ù̶���ɫ����������ʷ�����;�����ɫ
    if(track->trace.Target == 2)
    {   // ��Ҫ
        trackcolor = m_importantColor;
        fixedColorFlag = true;
    }
	/*
    else if(track->trace.Trace == 3)
    {   // ˥��
        trackcolor = m_fadeColor;
        fixedColorFlag = true;
    }*/
    else
    {
		if(track->specifyColor)
			trackcolor = track->specifyColor;
		else
			trackcolor = trackTypeColor(track->source);
    }

    // ������(����)����ɫ
    painter->setPen (QColor(trackcolor));

    //qDebug() << "drawTrack:" << hasBoardFlag;

    if(showFirstPoint)
    {
        // ���
        QPoint sc = track->points[track->curPointPos].position.screen_point[view].toPoint();
        // ָ���߲���
        const float lineCourse = guildLineCourse(track);
        const int lineLength = 10;//guildLineLength (track, view);
        // ָ���ߵ����
        const int x1 = sc.x(), y1 = sc.y();
        // ָ���ߵ��յ�
        int x2, y2;
        x2 = x1 + lineLength*SIN(lineCourse);
        y2 = y1 - lineLength*COS(lineCourse);
        QRect rc;
        const bool hasBoardFlag = (!track->boardLineTexts.empty());
        if (hasBoardFlag)
        {
            rc = getBoardRect (painter, track, x2+4, y2);
        }
        else
        {
            rc = QRect(x1-10, y1-10, 20, 20);
        }

        // 1.��ʾ��������
        // �����Զ����ô���
        if(hasBoardFlag && m_autoBoardAvoidFlag)
        {
        }

        track->boardRect[view] = rc;
        if(hasBoardFlag)
		{
            drawBoard(painter, track, rc);
		}
        else
		{
			painter->setBrush(Qt::NoBrush);
            painter->drawRect(rc);
		}

        // 2.��ָ����
        if(hasBoardFlag)
            drawGuidLine(painter, x1, y1, x2, y2);

        // 3.��ʾ�׵�
        drawFirstPoint(painter, trackSymbol, sc, trackSymbolSize);

		/*
		if(m_showGuidLine && track->speed > 0)
		{
			if(m_guidLineFmt == 0 && m_guidLineLength > 0)
			{	// ��ʱ�䳤����ʾ
				SQUARE_POINT sq = track->points[track->curPointPos].position.square_point;
				double r = track->speed * m_guidLineLength / 60.0, da = track->course*M_2PI/360.0;
				double dx = r * sin(da), dy = r * cos(da);
				sq += QPointF(dx, dy);
				SCREEN_POINT sc2 = m_radarView[view]->squaretoscreen_view(sq);
				x2 = sc2.x(), y2 = sc2.y();
				drawGuidLine(painter, x1, y1, x2, y2);
			}
			else if(m_guidLineFmt == 1)
			{	// �ֵ���ʾ
				double da = track->course*M_2PI/360.0;
				double r = 40;
				if(track->speed > 20)
					r = 80;
				else if(track->speed > 10)
					r = 60;
				double dx = r * sin(da), dy = r * cos(da);
				x2 = x1 + dx, y2 = y1 - dy;
				drawGuidLine(painter, x1, y1, x2, y2);
			}
		}*/
    }

    //qDebug() << track->no << track->maxPoints << track->curPoints;
    // 4.��ʾβ��(�ӵڶ��㿪ʼ)
    const int count = track->curPoints;
    int index = PrevTrackPointIndex(track->curPointPos);

	QPoint lastPoint, crntPoint;
    if(trackTrailDispMode)
    {   // ��ʾ����
        char symbolsize;
        painter->setBrush (Qt::NoBrush);
        if (!fixedColorFlag)
        {
            /*ÿһβ��ʹ�õ�����ɫ*/
            for (int i=1; i<count; ++i)
            {
                if(track->points[index].position.displayFlag & displayFlag)
                {
                const float rate = 1.0 - (float)i / (float)count;
                symbolsize = (char)(trackSymbolSize * rate);
                LPTRACKTYPEINFO typeinfo1 = m_typeInfo.value(track->points[index].source, NULL);
                const quint8 symbol = (typeinfo1 ? typeinfo1->symbol : SYMBOL_NONE);//typeinfo->symbol;
                /*�ɺ�������ȷ����ɫ*/
                painter->setPen (QPen(trackTypeColor(track->points[index].source)));
				crntPoint = track->points[index].position.screen_point[view].toPoint();
                drawFirstPoint(painter, symbol, crntPoint, symbolsize);
				/*if(!track->points[index].fadeing)
				{
					if(!lastPoint.isNull())
						painter->drawLine(lastPoint, crntPoint);
					lastPoint = crntPoint;
				}
				else
				{
					lastPoint = QPoint(0,0);
				}*/
                }
                // ȡǰһ������
                index = PrevTrackPointIndex(index);
            }
        }
        else
        {
            /*����β��ʹ��ͬһ��ɫ*/
            painter->setPen (QPen(trackcolor));
            for (int i=1; i<count; ++i)
            {
                if(track->points[index].position.displayFlag & displayFlag)
                {
                const float rate = 1.0 - (float)i / (float)count;
                symbolsize = (char)(trackSymbolSize * rate);
                LPTRACKTYPEINFO typeinfo1 = m_typeInfo.value(track->points[index].source, NULL);
                const quint8 symbol = (typeinfo1 ? typeinfo1->symbol : SYMBOL_NONE);//typeinfo->symbol;
				crntPoint = track->points[index].position.screen_point[view].toPoint();
                drawFirstPoint(painter, symbol, crntPoint, symbolsize);
				/*if(!track->points[index].fadeing)
				{
					if(!lastPoint.isNull())
						painter->drawLine(lastPoint, crntPoint);
					lastPoint = crntPoint;
				}
				else
				{
					lastPoint = QPoint(0,0);
				}*/
                }
                // ȡǰһ������
                index = PrevTrackPointIndex(index);
            }
        }
    }
    else
    {   // ��ʾСԲ��
        //painter->setPen (Qt::NoPen);
        if (!fixedColorFlag)
        {
            /*ÿһβ��ʹ�õ�����ɫ*/
            for (int i=1; i<count; ++i)
            {
                if(track->points[index].position.displayFlag & displayFlag)
                {
                /*�ɺ�������ȷ����ɫ*/
                painter->setBrush (QBrush(trackTypeColor(track->points[index].source)));
                //drawHistoryPoint (painter, track->points[index].position.screen_point[view].toPoint());
				crntPoint = track->points[index].position.screen_point[view].toPoint();
                drawHistoryPoint(painter, crntPoint);
				/*if(!track->points[index].fadeing)
				{
					if(!lastPoint.isNull())
						painter->drawLine(lastPoint, crntPoint);
					lastPoint = crntPoint;
				}
				else
				{
					lastPoint = QPoint(0,0);
				}*/
                }
                // ȡǰһ������
                index = PrevTrackPointIndex(index);
            }
        }
        else
        {
            /*����β��ʹ��ͬһ��ɫ*/
            painter->setBrush (QBrush(trackcolor));
            for (int i=1; i<count; ++i)
            {
                if(track->points[index].position.displayFlag & displayFlag)
                {
                //drawHistoryPoint (painter, track->points[index].position.screen_point[view].toPoint());
				crntPoint = track->points[index].position.screen_point[view].toPoint();
                drawHistoryPoint(painter, crntPoint);
				/*if(!track->points[index].fadeing)
				{
					if(!lastPoint.isNull())
						painter->drawLine(lastPoint, crntPoint);
					lastPoint = crntPoint;
				}
				else
				{
					lastPoint = QPoint(0,0);
				}*/
                }
                // ȡǰһ������
                index = PrevTrackPointIndex(index);
            }
        }
    }
}


// ���ƺ�������
void Track::drawBoard (QPainter* p, LPTRACK track, const QRect& rc)
{
    // ���±����ı�
    updateBoardText (track);

    BOARDDISP& board = track->boardDisp;

    // ���������С
    bool needRestoreFont = false;
    if (IsCurrentManTrack (track->index))
    {
        QFont font = p->font();
        font.setBold (true);
        font.setItalic (true);
        font.setUnderline (true);
        p->setFont (font);
        needRestoreFont = true;
    }
    setFontSize (p, board.FontSize);

    QFontMetrics fm (p->font());
    const int ascent = fm.ascent ();
    const int lineHeight = fm.height ();

    int x = rc.left();
    int y = rc.top() + ascent;

    // �����Ʊ����ͱ߿�
    if(board.FillBackground)
    {
        p->save();
        p->setBrush(QColor(board.FillColor));
        p->setPen(board.BorderColor);
        p->drawRect(rc);
        p->restore();
    }
    else if(board.ShowBorder)
    {
        p->setBrush(Qt::NoBrush);
        //p->setPen(board.BorderColor);
        p->drawRect(rc);
    }

    // ��ʾ��������
    QStringList::iterator it1 = track->boardLineTexts.begin(), it2 = track->boardLineTexts.end();
    for (; it1 != it2; ++ it1)
    {
        p->drawText (x+2, y+1, *it1);
        y += lineHeight;
    }

    if (needRestoreFont)
    {
        QFont font = p->font();
        font.setBold (false);
        font.setItalic (false);
        font.setUnderline (false);
        setFontSize (p, track->boardDisp.FontSize);
        p->setFont (font);
    }
}

// ����ָ����
void Track::drawGuidLine (QPainter* p, int x1, int y1, int x2, int y2)
{
    p->setRenderHint (QPainter::Antialiasing, true);
    p->drawLine (x1, y1, x2, y2);
    p->setRenderHint (QPainter::Antialiasing, false);
}

// ��ʾ��ʷ�㼣
void Track::drawFirstPoint (QPainter* painter, const int symbol, const QPoint& pt, char ptsize)
{
    // ��С��һ����
    if(ptsize <= 0)
    {
        painter->drawPoint(pt);
        return;
    }

    drawSymbol(painter, symbol, pt, ptsize);
}

// ��ʾβ��
void Track::drawHistoryPoint (QPainter* painter, const QPoint& pt)
{
    const int x = pt.x(), y = pt.y();
    //painter->drawRect (QRect(x-1, y-1, 2, 2));
	painter->drawRect (QRect(x, y, 1, 1));
}

/*���λ�õ��ں������Ʒ�Χ�ڣ��򷵻ظú���������*/
TRACKINDEX Track::trackBoardSelect(const QPoint& pt, quint8 view)
{
    if((!isTrackShow()) || (view >= m_radarViewCount))
        return -1;

    QMutexLocker lock(&m_mutex);

    QList<TRACKINDEX>::const_iterator it1 = m_trackIndex.constBegin(), it2 = m_trackIndex.constEnd();
    for(; it1 != it2; ++it1)
    {
        LPTRACK track = (m_trackTable[*it1]);
        if(isTrackShow(track) && (track->boardRect[view].contains(pt)))
        {
            return track->index;
        }
    }

    return -1;
}

/////////////////////////////////////////////////////////////////////////////////
// ��¼����
// ��ǰ��ǰ��
void Track::JumpPrev (bool updateCursorPos)
{

}

// ��ǰ������
void Track::JumpNext (bool updateCursorPos)
{

}

// �Ե�ǰ��������¼����
void Track::ManExtractProcess ()
{

}

// ������¼����
void Track::UpdateManTrack (TRACKINDEX i)
{

}

// ɾ����¼����
void Track::DeleteManTrack (TRACKINDEX i)
{

}

// �ж��Ƿ�ǰ��¼����
bool Track::IsCurrentManTrack (TRACKINDEX i)
{
    return false;
}

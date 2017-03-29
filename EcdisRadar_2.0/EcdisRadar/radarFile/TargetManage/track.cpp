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
extern double g_ownshpLon;                     // 本船经度
extern double g_ownshpLat;                     // 本船纬度
// 计算两点间的相对方位和距离，返回值单位：azi度，rng海里
extern void calRelativeAziRng(double lon1, double lat1, double lon2, double lat2, double &azi, double &rng);
extern void calArrivePos(double startLon, double startLat, double speed, double course,
                  unsigned int time, double& arriveLon, double& arriveLat);
// 获取本船的速度航向
extern bool GetOwnShipCourseSpeed(float* course, float* speed);
// 获取cpa/tcpa参数
extern quint8 GetCpaTcpaParam(float *cpa, float *dcpa, float *tcpa);
// 获取警报区参数
extern quint8 GetGuardZoneParam(float *zone);
extern bool GuardZoneAlarm(float azi, float rng);
// 启动报警
extern void StartAlarm(int level, const QString& msg);
// 获取向量时间长度
extern int GetVecterLength();
// 获取航迹尾点间隔时间
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
    // 删除所有的航迹对象
    clear(true);

    // 释放航迹类型对象
    unregisterType();
}

// 创建指定数量的航迹对象
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
        //航迹显示状态
        track.valid = 0;	// 可分配状态
        // 其它参数
        track.curPointPos = 0;
        track.maxPoints = 0;
        track.curPoints = 0;
        track.totalPoints = 0;
        track.hpointMode = 0;
		track.extraData = NULL;

		for(quint16 j=0; j<MAXTRACKPOINTS; j++){
            track.points[j].extraData = NULL;
		}

        // 添加到可分配链中
        m_freeIndex << index;
        m_trackTable.insert(index, &track);
    }

	m_maxTracks += maxTracks;
}

// 删除所有航迹
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

// 初始化指定航迹对象
void Track::initTrack (TRACK& track)
{
    track.valid = 0;
    // 尾点
    track.curPoints = 0;
	track.curPointPos = 0;
	track.totalPoints = 0;
    track.hpointMode = m_defaultTrackPointMode;
    track.maxPoints = trackPointSize(track.hpointMode);
    // 使用系统标牌设置
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

// 删除所有的航迹对象
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

// 删除指定航迹对象
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

// 分配一个航迹对象
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
            // 初始化航迹参数
            initTrack(track);
            // 设置显示标志为显示状态
            track.valid = 1;
        }
    }
    return index;
}

/*接收到航迹信息，则跟踪单元航迹滤波后(航迹输出时)调用*/
void Track::setTrackInfo (const SETTRACKINFO& info)
{
    QMutexLocker locker(&m_mutex);

    // 查找对应的航迹对象，如果没有，则申请新的
    TRACKINDEX index = indexByNo(info.trackNo);
    if(!isIndexValid(index))
    {
        index = applyTrack();
        if(!isIndexValid(index))
            return;

        // 保存批号索引表
        m_no_index[info.trackNo] = index;
        // 保存到航迹索引表
        m_trackIndex.append(index);
    }

    TRACK& track = *(m_trackTable[index]);

    // 从公里转换到当前系统距离量纲
    const float rate2km = m_radarView0->coefficientToKm();

    // 更新航迹参数
    track.no = info.trackNo;
    track.source = getTrackPointSource(info.source);
    track.course = info.course;						// 360度
    track.speed = (int)(info.velocity / rate2km);	// 海里/小时
    track.extrapolatePoint = info.extrapolatePoint;

    track.trace = info.trace;   // 跟踪状态
    track.type = info.type;     // 航迹类型
    track.model = info.model;   // 航迹属性
    track.number = info.number; // 航迹数量
    memcpy(track.name, info.name, 6);   // 航迹别名

    // 更新航迹点迹位置(从公里转换到当前系统距离量纲)
    SQUARE_POINT square_point = m_radarView0->rtheta_to_square(info.current_point);
    square_point.rx() = square_point.x() / rate2km;	// 海里
    square_point.ry() = square_point.y() / rate2km;

    // 由航向计算指引线方向
    if(!track.boardDisp.FixedCourse)
        track.boardDisp.LineCourse = track.course / 45; // * 8 / 360;

    // 当前点更新处理
    enterTrackPoint(track, square_point);

    // 设置航迹额外数据
	if(info.trackExtraData)
    track.setExtraData(info.trackExtraData);
    // 设置点迹额外数据
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

// 设置航迹状态
void Track::setTrackStatus (const SETTRACKSTATUS& trackStatus)
{
    //取出批号
    const TRACKNO p = trackStatus.trackNo;

    QMutexLocker locker(&m_mutex);

    //寻找航迹的索引号
    TRACKINDEX index = indexByNo(p);
    if(!isIndexValid(index))
        return;

    TRACK& track = *(m_trackTable[index]);

    //判断航迹的跟踪状态
    if(trackStatus.trackStatusFlag)
        track.trace.Trace = trackStatus.trackStatus;

    //判断航迹的目标状态
    if(trackStatus.targetStatusFlag)
        track.trace.Target = trackStatus.targetStatus;
	
#if _ATA_
	//if(3 == track.trace.Trace)
	//qDebug() << __FUNCTION__ << track.no << track.trace.Trace;
	// 设置相关标志位
	LPATAPARAM param = (LPATAPARAM)(track.extraData);
	if(param){
	param->targetInit = (track.trace.Trace == 1 ? 1 : 0);
	param->targetLost = (track.trace.Trace == 3 ? 1 : 0);
	if(param->targetLost)
		StartAlarm(2, "目标丢失");
	}
	else
		qDebug() << "track ata param is null" << track.no;
#endif
}

// 人工航迹起始
void Track::setTrackManInit (const SETTRACKMANINIT& trackManInit)
{
    const TRACKNO p = trackManInit.trackNo;

    QMutexLocker locker(&m_mutex);

    // 查找对应的航迹对象，如果没有，则申请新的
    TRACKINDEX index = indexByNo(p);
    if(!isIndexValid(index))
    {
        index = applyTrack();
        if(!isIndexValid(index))
            return;

        // 保存批号索引表
        m_no_index[p] = index;
        // 保存到航迹索引表
        m_trackIndex.append(index);
    }

    TRACK& track = *(m_trackTable[index]);

    // 从公里转换到当前系统距离量纲
    const float rate2km = m_radarView0->coefficientToKm();

    // 更新航迹参数
    track.no = p;
    track.source = getTrackPointSource(trackManInit.source);
    track.trace.Mode = trackManInit.trackMode;  // 跟踪方式
    track.trace.Control = trackManInit.control; // 控制标志

    // 更新航迹点迹位置(从公里转换到当前系统距离量纲)
    SQUARE_POINT square_point;
    square_point.rx() = trackManInit.square_point.x() / rate2km;
    square_point.ry() = trackManInit.square_point.y() / rate2km;

    track.extrapolatePoint = m_radarView0->square_to_rtheta(square_point);

    // 由航向计算指引线方向
    if(!track.boardDisp.FixedCourse)
        track.boardDisp.LineCourse = track.course / 45; // * 8 / 360;

    // 当前点更新处理
    enterTrackPoint(track, square_point);

    // 设置航迹额外数据
    track.setExtraData(NULL);
    // 设置点迹额外数据
    track.points[track.curPointPos].setExtraData(NULL);

    // 人工航迹处理
    if(track.trace.Mode)
    {

    }

    //判断起始标志
    if(trackManInit.start)
    {
        //设置跟踪状态-航迹起始
        track.trace.Trace = 1;
        //设置目标状态
        track.trace.Target = 0;
    }
}

// 设置目标类型属性
void Track::setTrackTypeAttr (const SETTRACKTYPEATTR& trackTypeAttr)
{
    //取出批号
    TRACKNO p = trackTypeAttr.trackNo;
    TRACKINDEX index = indexByNo(p);
    if(!isIndexValid(index))
        return;

    QMutexLocker locker(&m_mutex);

    TRACK& track = *(m_trackTable[index]);

    // 类型
    if(trackTypeAttr.type.flag)
    {
        track.type.flag = 1;
        track.type.value = trackTypeAttr.type.value;
    }

    // 属性
    if(trackTypeAttr.model.flag)
    {
        track.model.flag = 1;
        track.model.value = trackTypeAttr.model.value;
    }

    // 数量
    if(trackTypeAttr.number.flag)
    {
        track.number.flag = 1;
        track.number.value = trackTypeAttr.number.value;
    }
}

// 设置改换批
void Track::setChgTrackNo (const SETCHGTRACKNO& chgTrackNo)
{
    QMutexLocker locker(&m_mutex);

    //取出目标1的批号
    TRACKNO p1 = chgTrackNo.trackNo1;
    //取出目标的索引号
    TRACKINDEX index1 = indexByNo(p1);
    //如果目标不存在
    if(!isIndexValid(index1))
        return;

    //取出目标2的批号
    TRACKNO p2 = chgTrackNo.trackNo2;
    //取出目标2的索引号
    TRACKINDEX index2 = indexByNo(p2);
    //如果目标2存在(换批)
    if(isIndexValid(index2))
    {   //进行换批操作
        m_trackTable[index2]->no = p1;
        m_no_index[p1] = index2;
    }
    else
    {   // 改批,此时需要删除原来批号索引
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

// 设置目标删除
void Track::setTrackDelete (const SETTRACKDELETE& trackDelete)
{
    QMutexLocker locker(&m_mutex);

    //取出批号
    TRACKNO p = trackDelete.trackNo;
    if(p==0)
    {   // 如果批号为0删除所有批
        deleteAllTracks();
    }
    else
    {   // 删除目标
        deleteTrack(indexByNo(p));
    }
}

// 更新航迹当前点迹
void Track::enterTrackPoint(TRACK& track, const SQUARE_POINT& sq)
{
	const int crnttime = time(0);

	TRACKPOINT& trackpoint = track.crntPoint;//track.points[track.curPointPos];
    trackpoint.source = track.source;
	trackpoint.fadeing = (track.trace.Trace == 3 ? 1 : 0);
	trackpoint.updateTime = crnttime;

    // 更新当前点位置
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
	// 如果点迹更新时间间隔小于30秒，则不记录当前点迹
	const int trackInterval = GetTrackIntervalTime();
	if(track.curPoints == 0 || (trackInterval > 0 && crnttime - track.points[track.curPointPos].updateTime >= trackInterval))
	{
		track.enterTrackPoint();
		track.points[track.curPointPos] = trackpoint;
	}
#endif

    // 设置需要更新标牌标志
    track.updateBoardText = 1;
}

/*更新所有目标的屏幕坐标*/
void Track::updateScreenPoint()
{
    QMutexLocker lock(&m_mutex);

    quint32 displayflag[MAXVIEWCOUNT];
    for(int i=0;i<MAXVIEWCOUNT; i++)
        displayflag[i] = (1<<i);

    // 对每一个点迹对象,计算其屏幕坐标
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

/*更新所有目标的屏幕坐标*/
void Track::updateScreenPoint(quint8 idx)
{
    if(idx >= m_radarViewCount)
        return;

    //qDebug() << "Track::updateScreenPoint" << idx;

    QMutexLocker lock(&m_mutex);

    quint32 displayflag = (1 << idx);

    // 对每一个点迹对象,计算其屏幕坐标
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

/*更新指定航迹的屏幕坐标*/
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

/*更新所有目标的直角坐标和屏幕坐标，距离量纲变化时调用*/
void Track::updateSquareAndScreenPoint()
{
    QMutexLocker lock(&m_mutex);

    quint32 displayflag[MAXVIEWCOUNT];
    for(int i=0;i<MAXVIEWCOUNT; i++)
        displayflag[i] = (1<<i);

    // 对每一个点迹对象,计算其直角坐标、极坐标、屏幕坐标
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

/*目标位置移动处理*/
void Track::move (const QPointF& sq, const QList<QPoint>& sc, quint8 index)
{
    QMutexLocker lock(&m_mutex);

    quint32 displayflag[MAXVIEWCOUNT];
    for(int i=0;i<MAXVIEWCOUNT; i++)
        displayflag[i] = (1<<i);

    // 对每一个点迹对象,计算移动其直角坐标、屏幕坐标
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

// 注册航迹类型(建议在程序初始化时一次注册完所有的航迹类型)
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

// 设置航迹类型标志
void Track::setTrackTypeFlag(quint8 type, quint8 flags)
{
    if(LPTRACKTYPEINFO info = m_typeInfo.value(type, NULL))
    {
        info->flags = flags;
    }
}

// 设置航迹类型颜色
void Track::setTrackTypeColor(quint8 type, quint32 color)
{
    if(LPTRACKTYPEINFO info = m_typeInfo.value(type, NULL))
    {
        info->color = color;
    }
}

// 获取航迹类型颜色
quint32 Track::trackTypeColor(quint8 type) const
{
    if(LPTRACKTYPEINFO info = m_typeInfo.value(type, NULL))
        return info->color;
    else
        return QRGB(0xff, 0xff, 0xff);
}

// 设置航迹类型名称
void Track::setTrackTypeName(quint8 type, const QString& name)
{
    if(LPTRACKTYPEINFO info = m_typeInfo.value(type, NULL))
    {
        info->name = name;
    }
}

// 获取航迹类型名称
QString Track::trackTypeName(quint8 type) const
{
    if(LPTRACKTYPEINFO info = m_typeInfo.value(type, NULL))
        return info->name;
    else
        return QString("Unknown");
}

// 设置航迹类型符号
void Track::setTrackTypeSymbol(quint8 type, quint8 symbol)
{
    if(LPTRACKTYPEINFO info = m_typeInfo.value(type, NULL))
    {
        info->symbol = symbol;
    }
}

// 获取航迹类型符号
quint8 Track::trackTypeSymbol(quint8 type) const
{
    if(LPTRACKTYPEINFO info = m_typeInfo.value(type, NULL))
        return info->symbol;
    else
        return SYMBOL_NONE;
}

// 设置航迹符号大小
void Track::setTrackTypeSymbolSize(quint8 type, quint8 size)
{
    if(LPTRACKTYPEINFO info = m_typeInfo.value(type, NULL))
    {
        info->symbolSize = size;
    }
}

// 获取航迹符号大小
quint8 Track::trackTypeSymbolSize(quint8 type) const
{
    if(LPTRACKTYPEINFO info = m_typeInfo.value(type, NULL))
        return info->symbolSize;
    else
        return 0;
}

// 删除航迹类型
void Track::unregisterType(quint8 type)
{
    if(type != TRACK_TYPE_UNKNOWN && m_typeInfo.contains(type))
    {
        LPTRACKTYPEINFO info = m_typeInfo.take(type);
        delete info;
    }
}

// 删除航迹类型对象
void Track::unregisterType()
{
    DeleteObjectInContainer(TRACKTYPEINFOHASH, m_typeInfo);
}

// 判断某一类型的航迹是否可显示
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

// 更新指定航迹标牌
void Track::updateBoard (LPTRACK track)
{
    // 更新航迹标牌字符串
    updateBoardText (track);
    // 更新航迹标牌指引线
    updateBoardGuildLine(track);
}

// 设置航迹标牌显示模式(改变标牌显示模式)
void Track::setTrackBoardMode(quint8 mode, LPTRACK track)
{
    QMutexLocker locker(&m_mutex);
    BOARDDISP &board = (track ? (track->boardDisp) : m_defaultBoardDisp);
    if(mode < MAXBOARDMODE && board.BoardMode != mode)
    {
        board.BoardMode = mode;
        board.LineValue = m_defaultBoardLineValue[mode];
        // 如果是修改系统标牌模式,则需要修改当前所有航迹的标牌模式
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

// 切换标牌最后一行显示的内容
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

// 更新标牌字符串
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
    case BOARDDISP::LINE_TRACKNO:  // 航迹批号
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

// 旋转指引线的方向
void Track::rotateBoardCourse (LPTRACK track)
{
    track->boardDisp.autoRotate();
}

// 设置航迹指引线线长
void Track::setLineLength (LPTRACK track, quint8 mode)
{
    track->boardDisp.setLineLength(mode);
}

// 人工设置指引线, 长度和方向
void Track::manGuildLine (LPTRACK track, int length, int course)
{
    track->boardDisp.manGuildLine(length, course);
}

// 计算航迹指引线的长度
int Track::guildLineLength (LPTRACK track, quint8 view)
{
    if (track)
    {
        BOARDDISP& board = track->boardDisp;
        if (board.ManLine)
            // 人工线长
            return board.ManLineLength;
        else if(board.FixedLength)
            // 固定线长
            return (board.LineLength) * 20;
        else
            // 自动计算线长
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

// 计算航迹指引线的方向
float Track::guildLineCourse (LPTRACK track)
{
    if (track)
    {
        BOARDDISP& board = track->boardDisp;
        if (board.ManLine)
            // 人工指引线方向
            return ((float)board.ManLineCourse) * M_2PI / 1024.0;
        else
            // 固定指引线方向
            return ((float)board.LineCourse) * M_2PI / 8.0f;
    }
    else
    {
        return 0.0f;
    }
}

// 更新航迹标牌指引线
void Track::updateBoardGuildLine(LPTRACK track)
{
}
// 设置航迹标牌字体大小
void Track::setBoardFontSize(LPTRACK track, quint8 fontsize)
{
    track->boardDisp.FontSize = fontsize;
}

quint8 Track::boardFontSize(LPTRACK track)
{
    return track->boardDisp.FontSize;
}

// 显示标牌边框
void Track::showBoardBorder(LPTRACK track, bool show)
{
    track->boardDisp.ShowBorder = (show?1:0);
}

// 注册航迹尾点模式
void Track::registerTrackPointMode(quint8 mode, const QString& name, quint16 pointcount)
{
    TRACKPOINTMODE trackpoint;
    trackpoint.mode = mode;
    trackpoint.name = name;
    trackpoint.pointCount = pointcount;
    m_trackPointMode[mode] = trackpoint;
}

// 设置尾点模式
void Track::setTrackPointMode(quint8 mode, LPTRACK track)
{
    QMutexLocker lock(&m_mutex);

    if(!m_trackPointMode.contains(mode))
        return;

    const quint16 pointcount = m_trackPointMode[mode].pointCount;
    if(track)
    {   // 设置当前航迹尾点模式和尾点数量
        setTrackPointModeAndSize(track, mode, pointcount);
    }
    else
    {   // 设置系统尾点模式，并修改所有航迹的尾点模式和数量
        setTrackPointModeAndSize(mode, pointcount);
    }
}

// 人工设置尾点数量
void Track::setManTrackPointSize(quint16 size, LPTRACK track)
{
    QMutexLocker lock(&m_mutex);

    if(track)
        setTrackPointModeAndSize(track, MANTRACKPOINTMODE, size);
    else
        setTrackPointModeAndSize(MANTRACKPOINTMODE, size);
}

// 切换尾点模式
void Track::switchTrackPointMode(LPTRACK track)
{
    QMutexLocker lock(&m_mutex);

    // 寻找下一个模式
    if(m_trackPointMode.size() <= 2)
        return;
    QHash<quint8, TRACKPOINTMODE>::const_iterator it=m_trackPointMode.find(m_defaultTrackPointMode);
    do {
    if(it == m_trackPointMode.constEnd())
        it = m_trackPointMode.constBegin();
    if(it.key() == MANTRACKPOINTMODE)
        ++it;
    } while(it == m_trackPointMode.constEnd() || it.key() == MANTRACKPOINTMODE);

    // 设置系统尾点模式和对应的尾点数量
    TRACKPOINTMODE mode = it.value();
    if(track)
        setTrackPointModeAndSize(track, it.key(), mode.pointCount);
    else
        setTrackPointModeAndSize(it.key(), mode.pointCount);
}

// 获取尾点模式
quint8 Track::trackPointMode(LPTRACK track) const
{
    return (track?track->hpointMode:m_defaultTrackPointMode);
}

QString Track::trackPointModeName(LPTRACK track) const
{
    return m_trackPointMode[trackPointMode(track)].name;
}

// 根据航迹尾点模式，获取对应的尾点数量
quint16 Track::trackPointSize(quint8 mode)
{
    QHash<quint8, TRACKPOINTMODE>::const_iterator it=m_trackPointMode.find(mode);
    if(it == m_trackPointMode.constEnd())
        return 0;
    else
        return (*it).pointCount;
}

// 设置航迹尾点数量
void Track::setTrackPointModeAndSize(quint8 mode, quint16 pointcount)
{
    m_defaultTrackPointMode = mode;
    QList<TRACKINDEX>::const_iterator it1 = m_trackIndex.constBegin(), it2 = m_trackIndex.constEnd(), it;
    for(it=it1; it!=it2; ++it)
    {
        setTrackPointModeAndSize(m_trackTable[*it], mode, pointcount);
    }
}

// 设置航迹尾点数量
void Track::setTrackPointModeAndSize(LPTRACK track, quint8 mode, quint16 size)
{
    if(track->hpointMode != mode || track->maxPoints != size)
    {
        track->hpointMode = mode;

        if (size > MAXTRACKPOINTS)
            track->maxPoints = MAXTRACKPOINTS;
        else
            track->maxPoints = size;

        // 更新当前历史航迹点数
        if (size > track->totalPoints)
            track->curPoints = track->totalPoints;
        else
            track->curPoints = size;

        implUpdateTrackScreenPoint (track);
    }
}

// 设置字体大小
void Track::setFontSize (QPainter* p, quint8 size)
{
    QFont myfont = p->font();
    switch (size)
    {
    case 0:		// 较小
        myfont.setPointSize (9);
        myfont.setBold(false);
        break;
    case 2:		// 较大
        myfont.setPointSize (14);
        myfont.setBold(true);
        break;
    case 3:		// 很大
        myfont.setPointSize (16);
        myfont.setBold(true);
        break;
    default:	// 正常
        myfont.setPointSize (11);
        myfont.setBold(false);
        break;
    }
    p->setFont (myfont);
}

// 计算航迹标牌所在的显示区域
QRect Track::getBoardRect (QPainter* p, LPTRACK track, int x2, int y2)
{
    // 更新标牌文本
    updateBoardText (track);

    // 设置字体大小
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

    //  获取边框
    QRect rc(0, 0, boardWidth, boardHeight);
    rc.translate (x2, y2);

    return rc;
}

// 绘制所有的航迹对象
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

    // 对每一个点迹对象,计算其屏幕坐标
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

	// 设置相关标志位
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

	// 报警处理
	if(param->dcpaFlag || param->tcpaFlag)
	{
		StartAlarm(2, param->dcpaFlag ? "DCPA目标报警" : "TCPA目标报警");
	}
	else if(param->guardZoneFlag)
	{
		StartAlarm(2, "警戒区目标报警");
	}
	else if(param->targetLost)
	{
		StartAlarm(2, "目标丢失");
	}
#endif
}

// 更新ATA参数
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

	// 计算相对本船方位、距离
	double azi, rng;
	const PLOTPOSITION& trackPos = track->crntPoint.position;
	const RTHETA_POINT track_rtpt = trackPos.rtheta_point;
	const float trackLon = trackPos.latitude_point.longitude() / degree2radian;	// 单位：度
	const float trackLat = trackPos.latitude_point.latitude() / degree2radian;
	calRelativeAziRng(g_ownshpLon, g_ownshpLat, trackLon, trackLat, azi, rng);
	param->relativeRng = (ushort)(rng * 100);
	param->relativeAzi = (ushort)(azi * 10);

	/* 计算DCPA和TCPA */	
	float ownshipCourse, ownshipSpeed;
	const bool ownshipFlag = GetOwnShipCourseSpeed(&ownshipCourse, &ownshipSpeed);
	const bool trackSpeedValid = (track->speed > 0 && track->speed < 100);
	if( ownshipFlag || trackSpeedValid)
	{
		// 将方位从角度转化为弧度
		const double dazi = azi * degree2radian;
		//qDebug() << "cal navigate info:" << ownInfo.speed << ownInfo.course << aisInfo.speed << aisInfo.course;
		// 本船和他船速度航向
		const double V0 = ownshipSpeed, C0 = ownshipCourse * degree2radian;
		const double Vt = track->speed, Ct = track->course * degree2radian;
		const double sinc0 = sin(C0), cosc0 = cos(C0), sinct = sin(Ct), cosct = cos(Ct);
		const double Vrx = Vt*sinct - V0*sinc0, Vry = Vt*cosct - V0*cosc0;
		// 计算相对运动矢量的角度
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

	// 真运动速度和航向
	param->trueSpeed = (ushort)(track->speed * 100);
	param->trueCourse = (ushort)(track->course * 10);

	// cpa报警
	cpaAlarm(track);
#endif
}

// 显示ATA模式目标
void Track::paintAta(QPainter* painter, quint8 view, LPTRACK track)
{
#if _ATA_
	extern int GetAtaDispIndex(int index);
	extern int GetAtaSymbolSize();

    const quint32 displayFlag = (1<<view);
	TRACKPOINT& trackPoint = track->crntPoint;//points[track->curPointPos];
	const QPoint scpt = trackPoint.position.screen_point[view].toPoint();
    const bool showFirstPoint = (trackPoint.position.displayFlag & displayFlag);
    // 如果首点不可见，并且不显示可见历史点，则退出
    if((!showFirstPoint) && (!m_showVisiblePoint))
        return;

	const float rotation = m_radarView[view]->rotation();
	const bool trackSpeedValid = (track->speed > 0 && track->speed < 100);

	// 如果距离出界则不显示
	extern const quint16 * CurrentRange();
    const quint16 * lpCurrentRange = CurrentRange();
	const int dispAzi = 450+(trackPoint.position.rtheta_point.theta()+rotation) * 450.0 / M_2PI;
	if(trackPoint.position.rtheta_point.r() >= lpCurrentRange[dispAzi%450])
		return;

	// 设置画笔(线\文字)的颜色
	quint32 trackcolor;
	if(track->specifyColor)
		trackcolor = track->specifyColor;
	else
		trackcolor = trackTypeColor(track->source);
    painter->setPen (QColor(trackcolor));

	// 设置符号类型 目标选择(12) > CPA/TCPA(8) > 警报区(7) > 目标丢失(9) > 目标起始(3)
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

	// 计算向量的端点
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
		{	// 使用固定线长	
			dispLength = 40;
		}
		
		const float course = track->course * M_PI / 180.0 - rotation;
		pt1 = scpt + QPoint(dispLength * sin(course), - dispLength * cos(course));
		
		// 航向和速度向量
		//if(param && param->predict_x>0 && param->predict_y>0)
		painter->drawLine(scpt, pt1);
	}

    // 4.显示尾点(从第二点开始)
    const int count = track->curPoints;
    int index = track->curPointPos;//PrevTrackPointIndex(track->curPointPos);

	QPoint lastPoint, crntPoint;
    {   // 显示小圆点
        {
            /*所有尾点使用同一颜色*/
            painter->setBrush (QBrush(trackcolor));
            for (int i=1; i<count; ++i)
            {
                if(track->points[index].position.displayFlag & displayFlag)
                {
                //drawHistoryPoint (painter, track->points[index].position.screen_point[view].toPoint());
				crntPoint = track->points[index].position.screen_point[view].toPoint();
                drawHistoryPoint(painter, crntPoint);
                }
                // 取前一个索引
                index = PrevTrackPointIndex(index);
            }
        }
    }

	// 更新标牌所在的区域
	track->boardRect[view] = QRect(scpt.x()-symbolSize, scpt.y()-symbolSize, 2*symbolSize, 2*symbolSize);
#endif
}

// 绘制一批航迹对象
void Track::paintTrack(QPainter* painter, quint8 view, LPTRACK track)
{
    const quint32 displayFlag = (1<<view);
    TRACKPOINT& trackPoint = track->points[track->curPointPos];
    const bool showFirstPoint = (trackPoint.position.displayFlag & displayFlag);
    // 如果首点不可见，并且不显示可见历史点，则退出
    if((!showFirstPoint) && (!m_showVisiblePoint))
        return;

    // 航迹类型
    LPTRACKTYPEINFO typeinfo = m_typeInfo.value(track->source, NULL);
    if(!typeinfo)
    {
        typeinfo = m_typeInfo.value(TRACK_TYPE_UNKNOWN, NULL);
        if(!typeinfo)
            return;
    }

    // 更新航迹标牌字符
    updateBoardText(track);

    // 获取航迹类型相关参数
    const quint8 trackSymbol = track->specifySymb ? track->specifySymb : typeinfo->symbol;
    const quint8 trackTrailDispMode = typeinfo->tailDispMode;
    const quint8 trackSymbolSize = typeinfo->symbolSize;

    quint32 trackcolor;
    bool fixedColorFlag = false;
    // 如果是重要或是衰落，则使用固定颜色，否则由历史点类型决定颜色
    if(track->trace.Target == 2)
    {   // 重要
        trackcolor = m_importantColor;
        fixedColorFlag = true;
    }
	/*
    else if(track->trace.Trace == 3)
    {   // 衰落
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

    // 设置线(文字)的颜色
    painter->setPen (QColor(trackcolor));

    //qDebug() << "drawTrack:" << hasBoardFlag;

    if(showFirstPoint)
    {
        // 起点
        QPoint sc = track->points[track->curPointPos].position.screen_point[view].toPoint();
        // 指引线参数
        const float lineCourse = guildLineCourse(track);
        const int lineLength = 10;//guildLineLength (track, view);
        // 指引线的起点
        const int x1 = sc.x(), y1 = sc.y();
        // 指引线的终点
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

        // 1.显示航迹标牌
        // 标牌自动避让处理
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

        // 2.画指引线
        if(hasBoardFlag)
            drawGuidLine(painter, x1, y1, x2, y2);

        // 3.显示首点
        drawFirstPoint(painter, trackSymbol, sc, trackSymbolSize);

		/*
		if(m_showGuidLine && track->speed > 0)
		{
			if(m_guidLineFmt == 0 && m_guidLineLength > 0)
			{	// 按时间长度显示
				SQUARE_POINT sq = track->points[track->curPointPos].position.square_point;
				double r = track->speed * m_guidLineLength / 60.0, da = track->course*M_2PI/360.0;
				double dx = r * sin(da), dy = r * cos(da);
				sq += QPointF(dx, dy);
				SCREEN_POINT sc2 = m_radarView[view]->squaretoscreen_view(sq);
				x2 = sc2.x(), y2 = sc2.y();
				drawGuidLine(painter, x1, y1, x2, y2);
			}
			else if(m_guidLineFmt == 1)
			{	// 分档显示
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
    // 4.显示尾点(从第二点开始)
    const int count = track->curPoints;
    int index = PrevTrackPointIndex(track->curPointPos);

	QPoint lastPoint, crntPoint;
    if(trackTrailDispMode)
    {   // 显示符号
        char symbolsize;
        painter->setBrush (Qt::NoBrush);
        if (!fixedColorFlag)
        {
            /*每一尾点使用单独颜色*/
            for (int i=1; i<count; ++i)
            {
                if(track->points[index].position.displayFlag & displayFlag)
                {
                const float rate = 1.0 - (float)i / (float)count;
                symbolsize = (char)(trackSymbolSize * rate);
                LPTRACKTYPEINFO typeinfo1 = m_typeInfo.value(track->points[index].source, NULL);
                const quint8 symbol = (typeinfo1 ? typeinfo1->symbol : SYMBOL_NONE);//typeinfo->symbol;
                /*由航迹类型确定颜色*/
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
                // 取前一个索引
                index = PrevTrackPointIndex(index);
            }
        }
        else
        {
            /*所有尾点使用同一颜色*/
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
                // 取前一个索引
                index = PrevTrackPointIndex(index);
            }
        }
    }
    else
    {   // 显示小圆点
        //painter->setPen (Qt::NoPen);
        if (!fixedColorFlag)
        {
            /*每一尾点使用单独颜色*/
            for (int i=1; i<count; ++i)
            {
                if(track->points[index].position.displayFlag & displayFlag)
                {
                /*由航迹类型确定颜色*/
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
                // 取前一个索引
                index = PrevTrackPointIndex(index);
            }
        }
        else
        {
            /*所有尾点使用同一颜色*/
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
                // 取前一个索引
                index = PrevTrackPointIndex(index);
            }
        }
    }
}


// 绘制航迹标牌
void Track::drawBoard (QPainter* p, LPTRACK track, const QRect& rc)
{
    // 更新标牌文本
    updateBoardText (track);

    BOARDDISP& board = track->boardDisp;

    // 设置字体大小
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

    // 填充标牌背景和边框
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

    // 显示标牌内容
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

// 绘制指引线
void Track::drawGuidLine (QPainter* p, int x1, int y1, int x2, int y2)
{
    p->setRenderHint (QPainter::Antialiasing, true);
    p->drawLine (x1, y1, x2, y2);
    p->setRenderHint (QPainter::Antialiasing, false);
}

// 显示历史点迹
void Track::drawFirstPoint (QPainter* painter, const int symbol, const QPoint& pt, char ptsize)
{
    // 缩小到一个点
    if(ptsize <= 0)
    {
        painter->drawPoint(pt);
        return;
    }

    drawSymbol(painter, symbol, pt, ptsize);
}

// 显示尾点
void Track::drawHistoryPoint (QPainter* painter, const QPoint& pt)
{
    const int x = pt.x(), y = pt.y();
    //painter->drawRect (QRect(x-1, y-1, 2, 2));
	painter->drawRect (QRect(x, y, 1, 1));
}

/*如果位置点在航迹标牌范围内，则返回该航迹索引号*/
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
// 手录处理
// 当前批前跳
void Track::JumpPrev (bool updateCursorPos)
{

}

// 当前批后跳
void Track::JumpNext (bool updateCursorPos)
{

}

// 对当前批进行手录处理
void Track::ManExtractProcess ()
{

}

// 更新手录航迹
void Track::UpdateManTrack (TRACKINDEX i)
{

}

// 删除手录航迹
void Track::DeleteManTrack (TRACKINDEX i)
{

}

// 判断是否当前手录航迹
bool Track::IsCurrentManTrack (TRACKINDEX i)
{
    return false;
}

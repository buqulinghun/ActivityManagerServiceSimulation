#include "plot.h"
#include "radarview.h"
#include "mouseoperation.h"
#include "glwidget.h"
#include <time.h>
#include <QtCore/QtDebug>

extern void drawSymbol(QPainter* painter, quint8 type, const QPoint& pt, quint8 sz);
extern MouseOperation *lpMouseOpetarion;
extern GLWidget *pView;

#ifdef _MSVC_
template<class obj>
void listAppend(QList<obj> list1, QList<obj> list2)
{
	QList<obj>::iterator it = list2.begin(), it2 = list2.end();
	for(; it != it2; ++ it)
		list1.append(*it);
}
#endif

/*
 * ֡�㼣�����б������
 */
class PlotFrameListIterator
{
public:
    PlotFrameListIterator(const PLOTFRAMELIST& plotFrameList)
    {
        frame = plotFrameList.constBegin();
        frame_end = plotFrameList.constEnd();

        // ���ҵ���һ���㼣�����λ��
        for(; frame != frame_end; ++frame)
        {
            plotlist = (*frame)->plotGroup.constBegin();
            plotlist_end = (*frame)->plotGroup.constEnd();
            for(; plotlist != plotlist_end; ++plotlist)
            {
                PLOTLIST& plist = (*plotlist)->plotList;
                plot = plist.constBegin();
                plot_end = plist.constEnd();
                if(plot != plot_end)
                    return;
            }
        }        
    }

    bool hasNexPlot() const
    {   return (frame != frame_end);    }

    LPPLOT nextPlot()
    {
        LPPLOT pplot = *plot++;
        if(plot == plot_end)
        {
            // ����֡��һ���͵㼣�б�
            ++plotlist;
            findNextPlot();
        }
        return pplot;
    }

private:
    void findNextPlot()
    {
        // �ȴ���ǰ֡���ٴ�����һ֡��ȷ��(*plot)��Ч
        while(frame != frame_end)
        {
            for(; plotlist != plotlist_end; ++plotlist)
            {
                PLOTLIST& plist = (*plotlist)->plotList;
                plot = plist.constBegin();
                plot_end = plist.constEnd();
                plot_end = plist.constEnd();
                if(plot != plot_end)
                    return;
            }

            if((++frame) != frame_end)
            {
                plotlist = (*frame)->plotGroup.constBegin();
                plotlist_end = (*frame)->plotGroup.constEnd();
            }
        }
    }

private:
    PLOTFRAMELIST::const_iterator frame_end, frame;
    QHash<quint8, LPPLOTGROUP>::const_iterator plotlist_end, plotlist;
    PLOTLIST::const_iterator plot_end, plot;
};

// ���ڱ���֡�㼣�б��е����е㼣����
#define ForEachPlot(variable,container) \
for(PlotFrameListIterator frameIterator(container);\
    frameIterator.hasNexPlot();\
    )\
    if(variable=frameIterator.nextPlot())

/*
 * class Plot
 * author:moflying
 * time:2010-04-08
 */

Plot::Plot()
{
    m_radarView0 = NULL;
    m_radarViewCount = 1;

    for(int i=0; i<MAXVIEWCOUNT; i++)
        m_radarView[i] = NULL;

    m_defaultKeepTime = 60;
    m_plotSize = 4;

    m_disappearTime = 0;
    m_appearTime = 0;

    m_lastFrameTime = 0;
    m_frameTimeWidth = 1;

    registerType(PLOT_TYPE_UNKNOWN, "Unknow", Qt::white);
    setFrameTimeWidth(2);
}

Plot::~Plot()
{
    // �ͷ����еĵ㼣����
    clear(true);
    // �ͷ����еĵ㼣���Ͷ���
    unregisterType();
}

// �µĵ㼣����
void Plot::enterPlot(LPPLOTINFO info)
{
 //   if(!m_radarView0)
   //     return;

    QMutexLocker lock(&m_mutex);

    // ��ȡ��ǰ֡����
    LPPLOTFRAME plotFrame = accessCurrentPlotFrame();
    if(!plotFrame)
        return;

    // ���㼣��ӵ���ǰ֡
    insertPlotToFrame(plotFrame, info);
}

void Plot::enterPlot(const PLOTINFOLIST& infolist)
{
 //   if(!m_radarView0)
  //      return;

    QMutexLocker lock(&m_mutex);

    // ��ȡ��ǰ֡����
    LPPLOTFRAME plotFrame = accessCurrentPlotFrame();
    if(!plotFrame)
        return;

    // ���㼣��ӵ���ǰ֡
    PLOTINFOLIST::const_iterator it1 = infolist.begin(), it2 = infolist.end(), it;
    for(it=it1; it!=it2; ++it)
        insertPlotToFrame(plotFrame, *it);
}

// ��ӵ㼣��ָ��֡
void Plot::insertPlotToFrame(LPPLOTFRAME frame, LPPLOTINFO info)
{
    // ����㼣����
    LPPLOT plot = applyPlot();
    if(!plot)
        return;

    // �㼣����
    plot->type = (isTypeValid(info->type) ? info->type : PLOT_TYPE_UNKNOWN);

    // �߼����󣬰�����������������һֱ������
    if(!frame->plotGroup.contains(plot->type))
        return;

    // ����㼣λ��
    PLOTPOSITION& position = plot->position;
    // �ӹ���ת������ǰϵͳ��������
    const float rate2km = lpMouseOpetarion->coefficientToKm();
    SQUARE_POINT sq = lpMouseOpetarion->rtheta_to_square(info->rtheta_point);
    position.square_point.rx() = sq.x() / rate2km;
    position.square_point.ry() = sq.y() / rate2km;
    //qDebug() << "square_point:" << position.square_point.toPoint();
    position.rtheta_point = lpMouseOpetarion->square_to_rtheta(position.square_point);
    const QPointF square = QPointF(position.square_point.x(), position.square_point.y());
    position.latitude_point.fromPoint( lpMouseOpetarion->square_to_latitude(square));   //get latitude      much convert
    position.displayFlag = 0;
    for(int i=0;i<m_radarViewCount; i++)
    {
        position.screen_point[i] = lpMouseOpetarion->square_to_screen(position.square_point);

        if(pView->isPointDisplay(position))
            position.displayFlag |= (1 << i);
      //  qDebug() << "screen_point:" << i << position.screen_point[i].toPoint();
    }

    // ����㼣����
    plot->aziWidth = info->aziWidth;
    plot->rngWidth = info->rngWidth;

    // �����������
    if(info->extraData)
        addExtraData(plot, info->extraData);

    // ���㼣����ҵ�֡����ĵ㼣����
    frame->plotGroup[plot->type]->plotList.append(plot);

    frame->plotCount ++;

    //qDebug() << "insertPlotToFrame ok";
}

// ��ʱ����µ㼣��ʾ
void Plot::updatePlotKeepTime(const quint32 time0)
{
    // ����ǰ�����δ����ʱ���,�����浱ǰ����ʱ��.
    const int dtime = time0 - m_disappearTime;
    m_disappearTime = time0;

    // ���ʱ���쳣�仯���򲻴���
    if(dtime <= 0 || dtime >= 5)
        return;

    QMutexLocker lock(&m_mutex);
    PLOTFRAMELIST::iterator it1 = m_plotFrameList.begin(), it2 = m_plotFrameList.end(), it;
    for(it=it1; it!=it2; )
    {
        // ����֡����ʱ��
        LPPLOTFRAME frame = (*it);
        frame->keepingTime += dtime;

        // �ж�֡����ÿһ��㼣�Ƿ����
        const quint8 dt = frame->keepingTime;
        PLOTGROUPHASH& gtable = frame->plotGroup;
        PLOTGROUPHASH::const_iterator it_g1 = gtable.constBegin(), it_g2 = gtable.constEnd(), it_g;
        for(it_g=it_g1; it_g!=it_g2; ++it_g)
        {
            LPPLOTGROUP group = (*it_g);
            // ������ڣ����ͷ�һ��㼣
            if(dt >= group->keepTime)
            {
                (*it)->plotCount -= group->plotList.size();
                freePlot(group->plotList);
            }
        }

        // �����ǰ֡�㼣��Ϊ0�����ͷŵ�ǰ֡����
        if(frame->plotCount <= 0)
        {
            freePlotFrame(frame, false);
            it = m_plotFrameList.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

/*��������Ŀ�����Ļ����(������ͼ)*/
void Plot::updateScreenPoint()
{
 //   if(!m_radarView0)
 //       return;

    QMutexLocker lock(&m_mutex);

    quint32 displayflag[MAXVIEWCOUNT];
    for(int i=0;i<MAXVIEWCOUNT; i++)
        displayflag[i] = (1<<i);

    // ��ÿһ���㼣����,��������Ļ����
    ForEachPlot(LPPLOT plot, m_plotFrameList)
    {
        PLOTPOSITION& position = plot->position;
        position.displayFlag = 0;
        for(int i=0; i<m_radarViewCount; i++)
        {
            position.screen_point[i] = lpMouseOpetarion->square_to_screen(position.square_point);

            if(pView->isPointDisplay(position))
                position.displayFlag |= displayflag[i];
        }
    }
}

/*��������Ŀ�����Ļ���ָ꣨����ͼ��*/
void Plot::updateScreenPoint(quint8 idx)
{
  //  if(!m_radarView0)
    //    return;

    if(idx >= m_radarViewCount)
        return;

    QMutexLocker lock(&m_mutex);

    quint32 displayflag = (1 << idx);

  //  CRadarView* view = m_radarView[idx];
    // ��ÿһ���㼣����,��������Ļ����
    ForEachPlot(LPPLOT plot, m_plotFrameList)
    {
        PLOTPOSITION& position = plot->position;
        position.screen_point[idx] = lpMouseOpetarion->square_to_screen(position.square_point);

        if(pView->isPointDisplay(position))
            position.displayFlag |= displayflag;
        else
            position.displayFlag &= (~displayflag);
    }
}

/*��������Ŀ���ֱ���������Ļ���꣬�������ٱ仯ʱ����*/
void Plot::updateSquareAndScreenPoint()
{
 //   if(!m_radarView0)
   //     return;

    QMutexLocker lock(&m_mutex);

    quint32 displayflag[MAXVIEWCOUNT];
    for(int i=0;i<MAXVIEWCOUNT; i++)
        displayflag[i] = (1<<i);

    // ��ÿһ���㼣����,������ֱ�����ꡢ�����ꡢ��Ļ����
    ForEachPlot(LPPLOT plot, m_plotFrameList)
    {
        PLOTPOSITION& position = plot->position;
        position.displayFlag = 0;
        for(int i=0; i<m_radarViewCount; i++)
        {
        //    CRadarView* view = m_radarView[i];
            position.square_point = lpMouseOpetarion->latitude_to_square(position.latitude_point);
            position.rtheta_point = lpMouseOpetarion->square_to_rtheta(position.square_point);
            position.screen_point[i] = lpMouseOpetarion->square_to_screen(position.square_point);

            if(pView->isPointDisplay(position))
                position.displayFlag |= displayflag[i];
        }
    }
}

/*Ŀ��λ���ƶ�����(ֱ���������Ļ������Ӧ���ƶ�)*/
void Plot::move (const QPointF& sq, const QList<QPoint>& sc, quint8 index)
{
 //   if(!m_radarView0)
    //    return;

    QMutexLocker lock(&m_mutex);

    quint32 displayflag[MAXVIEWCOUNT];
    for(int i=0;i<MAXVIEWCOUNT; i++)
        displayflag[i] = (1<<i);

    // ��ÿһ���㼣����,�����ƶ���ֱ�����ꡢ��Ļ����
    ForEachPlot(LPPLOT plot, m_plotFrameList)
    {
        PLOTPOSITION& position = plot->position;
        position.square_point += sq;
        position.rtheta_point = lpMouseOpetarion->square_to_rtheta(position.square_point);
        for(int i=0,j=0; i<m_radarViewCount; i++)
        {
            if(index & displayflag[i])
            {
             //   CRadarView* view = m_radarView[i];
                position.screen_point[i] += sc[j];

                if(pView->isPointDisplay(position))
                    position.displayFlag |= displayflag[i];
                else
                    position.displayFlag &= (~displayflag[i]);
                j++;
            }
        }
    }
}

// ������е㼣
void Plot::clear(bool toDelete)
{
    QMutexLocker lock(&m_mutex);

    freePlotFrame(toDelete);

    if(toDelete)
    {
        // ɾ���������е㼣֡����
        DeleteObjectInContainer(PLOTFRAMELIST, m_freePlotFrame);
        // ɾ���������еĵ㼣�����
        DeleteObjectInContainer(PLOTGROUPLIST, m_freePlotGroup);
        // ɾ���������еĵ㼣����
        DeleteObjectInContainer(PLOTLIST, m_freePlotList);
    }
}

// ����һ���㼣����
LPPLOT Plot::applyPlot()
{
    if(m_freePlotList.empty())
        return new PLOT;
    else
        return m_freePlotList.takeLast();
}

// ����һ���㼣�����
LPPLOTGROUP Plot::applyPlotGroup()
{
    if(m_freePlotGroup.empty())
        return new PLOTGROUP;
    else
        return m_freePlotGroup.takeLast();
}

// ����һ���㼣֡����
LPPLOTFRAME Plot::applyPlotFrame()
{
    LPPLOTFRAME plotFrame = NULL;
    if(m_freePlotFrame.empty())
    {
        plotFrame = new PLOTFRAME;

        // ���������
        PLOTTYPEINFOHASH::const_iterator it1 = m_typeInfo.constBegin(), it2=m_typeInfo.constEnd(), it;
        for(it=it1; it!=it2; ++it)
        {
            if(LPPLOTGROUP group = applyPlotGroup())
            {
                group->keepTime = (*it)->keepTime;  // ����ָ������ʱ��
                plotFrame->plotGroup.insert((*it)->type, group);
            }
        }
    }
    else
    {
        plotFrame = m_freePlotFrame.takeLast();
    }

    return plotFrame;
}

// ���ʵ�ǰ֡����
LPPLOTFRAME Plot::accessCurrentPlotFrame()
{
    // ����е�ǰ֡����,����û�з���֡�ı�,�򷵻ص�ǰ֡����,
    // �������µ�֡������Ϊ��ǰ֡����
    LPPLOTFRAME plotFrame = NULL;
    if(frameChanged() || m_plotFrameList.empty())
    {
        plotFrame = applyPlotFrame();
        m_plotFrameList.append(plotFrame);
        plotFrame->keepingTime = 0;
        plotFrame->plotCount = 0;
    }
    else
    {
        plotFrame = m_plotFrameList.last();
    }

    return plotFrame;
}

// �жϵ�ǰ֡�Ƿ����ı�
bool Plot::frameChanged()
{
    const quint32 tm0 = time(0);
    if(qAbs(m_lastFrameTime - tm0) > m_frameTimeWidth)
    {
        m_lastFrameTime = tm0;
        return true;
    }
    return false;
}

// �ͷ�һ���㼣
void Plot::freePlot(LPPLOT plot)
{
    m_freePlotList.append(plot);
}

// �ͷ�һ��㼣
void Plot::freePlot(PLOTLIST& plotList)
{
    if(!plotList.empty())
    {
        // �����еĵ㼣�������ӵ����ɵ㼣��β
#ifndef _MSVC_
        m_freePlotList.append(plotList);
#else
		listAppend<LPPLOT>(m_freePlotList, plotList);
#endif
        // ��յ㼣��
        plotList.clear();
    }
}

// �ͷ�һ��㼣
void Plot::freePlot(PLOTGROUPHASH& groupHash)
{
    PLOTGROUPHASH::const_iterator it1 = groupHash.constBegin(), it2 = groupHash.constEnd(), it;
    for(it=it1; it!=it2; ++it)
    {
        freePlot((*it)->plotList);
    }
}

// �ͷ�һ��㼣
void Plot::freePlotGroup(LPPLOTGROUP plotGroup)
{
    if(!plotGroup->plotList.empty())
    {
        // �����еĵ㼣�������ӵ����ɵ㼣��β
#ifndef _MSVC_
        m_freePlotList.append(plotGroup->plotList);
#else
		listAppend<LPPLOT>(m_freePlotList, plotGroup->plotList);
#endif
        // ��յ㼣��
        plotGroup->plotList.clear();
    }

    m_freePlotGroup.append(plotGroup);
}

// �ͷ�һ��㼣
void Plot::freePlotGroup(PLOTGROUPHASH& groupHash)
{
    PLOTGROUPHASH::const_iterator it1 = groupHash.constBegin(), it2 = groupHash.constEnd(), it;
    for(it=it1; it!=it2; ++it)
    {
        PLOTLIST& plotList = (*it)->plotList;
#ifndef _MSVC_
        m_freePlotList.append(plotList);
#else
		listAppend<LPPLOT>(m_freePlotList, plotList);
#endif
        plotList.clear();

        m_freePlotGroup.append(*it);
    }

    groupHash.clear();
}

// �ͷ�֡�㼣����
void Plot::freePlotFrame(LPPLOTFRAME plotFrame, bool freeGroup)
{
    if(!freeGroup)
        freePlot(plotFrame->plotGroup);
    else
        freePlotGroup(plotFrame->plotGroup);
    m_freePlotFrame.append(plotFrame);
}

// �ͷ�֡�㼣����
void Plot::freePlotFrame(bool freeGroup)
{
    PLOTFRAMELIST::const_iterator it1 = m_plotFrameList.begin(), it2 = m_plotFrameList.end(), it;
    for(it=it1; it!=it2; ++it)
    {
        freePlotFrame(*it, freeGroup);
        // ��Ҫע��,�����ͷ�֡����ʱ,��֡����ҵ�������֡��������,�����ﲻ��ɾ��֡����
        // delete (*it);
    }

    m_plotFrameList.clear();
}

// ע��㼣����(�����ڳ����ʼ��ʱһ��ע�������еĵ㼣����)
bool Plot::registerType(quint8 type, const QString& name, quint32 color, quint32 symbol, quint8 symbolSize)
{
    LPPLOTTYPEINFO info = m_typeInfo.value(type, NULL);
    if(!info)
    {
        info = new PLOTTYPEINFO;
        info->flags = 0xff;
        info->type = type;
        info->name = name;
        info->color = color;
        info->keepTime = m_defaultKeepTime;
		info->symbol = symbol;
		info->symbolSize = symbolSize;

        m_typeInfo.insert(type, info);
        return 1;
    }

    return 0;
}

// ���õ㼣���ͱ�־
void Plot::setTypeFlag(quint8 type, quint8 flags)
{
	QMutexLocker locker(&m_mutex);
    if(LPPLOTTYPEINFO info = m_typeInfo.value(type, NULL))
    {
        info->flags = flags;
    }
}

// ���õ㼣�ı���ʱ��
void Plot::setTypeKeepTime(quint8 type, quint8 keeptime)
{
	QMutexLocker locker(&m_mutex);
    if(LPPLOTTYPEINFO info = m_typeInfo.value(type, NULL))
    {
        info->keepTime = keeptime;
    }
}

// ���õ㼣��ɫ
void Plot::setTypeColor(quint8 type, quint32 color)
{
	QMutexLocker locker(&m_mutex);
    if(LPPLOTTYPEINFO info = m_typeInfo.value(type, NULL))
    {
        info->color = color;
    }
}

quint8 Plot::typeFlag(quint8 type) const
{
    if(LPPLOTTYPEINFO info = m_typeInfo.value(type, NULL))
    {
        return info->flags;
    }
	return 0;
}

quint8 Plot::typeKeeptime(quint8 type) const
{
    if(LPPLOTTYPEINFO info = m_typeInfo.value(type, NULL))
    {
        return info->keepTime;
    }
	return 0;
}

// ɾ���㼣����
void Plot::unregisterType(quint8 type)
{
    if(type != PLOT_TYPE_UNKNOWN && m_typeInfo.contains(type))
    {
        LPPLOTTYPEINFO info = m_typeInfo.take(type);
        delete info;
    }
}

// ɾ���㼣���Ͷ���
void Plot::unregisterType()
{
    DeleteObjectInContainer(PLOTTYPEINFOHASH, m_typeInfo);
}

// ���ƺ���
void Plot::paint(QPainter* painter, quint8 idx)
{
    if(idx >= m_radarViewCount)
        return;

    QMutexLocker lock(&m_mutex);

    const quint32 displayFlag = (1<<idx);
    float rate = 1.0;

    PLOTFRAMELIST::const_iterator it1 = m_plotFrameList.begin(), it2 = m_plotFrameList.end(), it;
    for(it=it1; it!=it2; ++it)
    {
        const float keepingTime = (float)((*it)->keepingTime);
        PLOTGROUPHASH::iterator it_g1 = (*it)->plotGroup.begin(), it_g2 = (*it)->plotGroup.end(), it_g;
        for(it_g=it_g1; it_g!=it_g2; ++it_g)
        {
            const quint8 type = it_g.key();
            PLOTLIST& plotlist = (*it_g)->plotList;

            // �ж��Ƿ���ʾ�����͵ĵ㼣
            if((!isPlotTypeShow(type)) || plotlist.empty())
                continue;

            LPPLOTTYPEINFO& plotType = m_typeInfo[type];
			if(keepingTime >= plotType->keepTime)
				continue;

            rate = 1.0 - keepingTime / (float)(plotType->keepTime);

            // ���û�����ɫ
            QColor color = plotColor(type);
            // ���õ㼣��С
            char ptsize = plotType->symbolSize;//m_plotSize;

            // ��ɫ������
            if(plotType->colorEvade)
            {
                int h, s, v;
                color.getHsv(&h, &s, &v);
                color.setHsv(h, s, v*rate);
            }

            // ��С���䴦��
            if(plotType->sizeEvade)
            {
                ptsize = (char)(ptsize*rate);
            }

            //qDebug() << "plot color:" << color << plotColor(type) << rate << type;
            painter->setPen(color);

            // ��ʾ�����͵㼣�������е㼣
            PLOTLIST::const_iterator it_p1 = plotlist.begin(), it_p2 = plotlist.end(), it_p;
            for(it_p = it_p1; it_p != it_p2; ++it_p)
            {
                LPPLOT plot = (*it_p);
                //qDebug() << "paint plot:" << plot->position.screen_point[idx].toPoint();
//                drawPlotSymbol(painter, plot->type, plot->position.screen_point[idx].toPoint(), ptsize);
                if(plot->position.displayFlag & displayFlag)
                {
                    // ���Ƶ㼣üë
                    if(isPlotBrowShow(plot->type))
                        drawPlotBrow(painter, plot, idx);
                    // ���Ƶ㼣����
                    if(isPlotSymbolShow(plot->type))
						drawSymbol(painter, plotType->symbol, plot->position.screen_point[idx].toPoint(), ptsize);
                        //drawPlotSymbol(painter, plot->type, plot->position.screen_point[idx].toPoint(), ptsize);
                    // ���Ƶ㼣����
                    if(isPlotBoardShow(plot))
                        drawPlotBoard(painter, plot, idx);
                }
            }
        }
    }
}

// ����һ���㼣����
void Plot::drawPlotSymbol(QPainter* painter, const int type, const QPoint& pt, char ptsize)
{
    // ��С��һ����
    if(ptsize <= 0)
    {
        painter->drawPoint(pt);
        return;
    }

    switch (type)
    {
    // ����������
    case PLOT_TYPE_PRIMARY:			// һ�ε㼣
    case PLOT_TYPE_MULFUSE:			// ��վ�ںϵ㼣
    case PLOT_TYPE_FILTER:			// �˲��㼣
    {
        drawSymbol(painter, SYMBOL_TRIANGLE_UP, pt, ptsize);
        break;
    }

    // ����������
    case PLOT_TYPE_SECONDARY:		// ���ε㼣
    case PLOT_TYPE_IIIREQUEST:		// III��ѯ�ʻ��㼣
    {
        drawSymbol(painter, SYMBOL_TRIANGLE_DOWN, pt, ptsize);
        break;
    }

    // ������
    case PLOT_TYPE_PRIMARYSEC:		// һ������ص㼣
    case PLOT_TYPE_PRIMARYIII:		// һ��III��ѯ�ʻ���ص㼣
    {
        drawSymbol(painter, SYMBOL_DIAMOND, pt, ptsize);
        break;
    }

    // ��ʮ��
    default:
    {
        drawSymbol(painter, SYMBOL_CROSS, pt, ptsize);
        break;
    }
    }	// end switch
}

// ���Ƶ㼣üë
void Plot::drawPlotBrow(QPainter* painter, LPPLOT plot, quint8 viewindex)
{
    quint8 penWidth = 2;

    painter->setRenderHint (QPainter::Antialiasing, true);

  //  CRadarView* view = m_radarView[viewindex];

	/*
    penWidth = (quint8)((plot->rngWidth / 25.0) * view->ration()+0.5);
    if (penWidth < 1)
        penWidth = 1;*/
    QPen mypen = painter->pen();
    mypen.setWidth(penWidth);
    painter->setPen(mypen);

    if(0)
    {/*
        int w = (int)(plot->aziWidth/10.0 * view->bscanAzimuthRation());
        SCREEN_POINT sc = plot->position.screen_point[viewindex];

        // ��ֱ��
        int x1 = sc.x() - w/2, x2 = sc.x() + w/2, y = sc.y();
        painter->drawLine(x1, y, x2, y);  */
    }
    else
    {
        //const double aziWidth = DEGREETORADIAN(plot->aziWidth/10.0 / 2.0);
		const double aziWidth = DEGREETORADIAN(plot->aziWidth * 0.8);
        RTHETA_POINT rt = plot->position.rtheta_point, rt1, rt2;

        rt1.setPoint(rt.r(), rt.theta()-aziWidth);
        rt2.setPoint(rt.r(), rt.theta()+aziWidth);
        const SCREEN_POINT sc1 = lpMouseOpetarion->square_to_screen(lpMouseOpetarion->rtheta_to_square(rt1));
        const SCREEN_POINT sc2 = lpMouseOpetarion->square_to_screen(lpMouseOpetarion->rtheta_to_square(rt2));
        QPoint pt(sc1.x()-sc2.x(), sc1.y()-sc2.y());
        if (pt.manhattanLength() < 20)
        {	// ��ֱ��
            painter->drawLine(sc1.x(), sc1.y(), sc2.x(), sc2.y());
        }
        else
        {	// ������
            SQUARE_POINT sq0 = {0, 0};
            const SCREEN_POINT sc0 = lpMouseOpetarion->square_to_screen(sq0);
            const int ct_x = sc0.x(), ct_y = sc0.y();
            QLineF line1 (ct_x, ct_y, sc1.x(), sc1.y());
            const int r1 = (int)(line1.length());
            QRect rc1 (ct_x-r1, ct_y-r1, 2*r1, 2*r1);
            const double abgn = rt1.theta(), aend = rt2.theta();
            const double da = (aend < abgn ? aend-abgn+M_2PI : aend-abgn);

            painter->drawArc(rc1, (int)((90-aend*360.0/M_2PI)*16), (int)((da*360.0/M_2PI)*16));
        }
    }

    painter->setRenderHint (QPainter::Antialiasing, false);
    mypen.setWidth(1);
    painter->setPen(mypen);
}

// ���Ƶ㼣����
void Plot::drawPlotBoard(QPainter* painter, LPPLOT plot, quint8 viewindex)
{
}

// ��ȡָ�����͵㼣��ʾ��ɫ
quint32  Plot::plotColor(quint8 type) const
{
    return m_typeInfo.contains(type) ? \
            m_typeInfo[type]->color : 0;
}

// �жϵ�ǰ���͵ĵ㼣�Ƿ���ʾ
bool Plot::isPlotTypeShow(quint8 type) const
{
    return m_typeInfo.contains(type) ? \
            m_typeInfo[type]->displayFlag : false;
}

// �ж��Ƿ���ʾ�㼣����
bool Plot::isPlotSymbolShow(quint8 type) const
{
    return m_typeInfo.contains(type) ? \
            m_typeInfo[type]->displaySymb : false;
}

// �ж��Ƿ���ʾ�㼣üë
bool Plot::isPlotBrowShow(quint8 type) const
{
    return m_typeInfo.contains(type) ? \
            m_typeInfo[type]->displayBrow : false;
}

// �ж��Ƿ���ʾ�㼣����
bool Plot::isPlotBoardShow(LPPLOT plot)
{
    return false;
}

// ��ָ��λ�û�ָ�����͵ķ���
void drawSymbol(QPainter* painter, quint8 type, const QPoint& pt, quint8 ptsize)
{
    const int d0 = -ptsize, d1 = 0, d2 = ptsize;
    const int x = pt.x(), y = pt.y();
    switch (type)
    {
    // СԲ��
    case SYMBOL_POINT:
    {
        //painter->drawRect (QRect(x-1, y-1, 2, 2));
        painter->drawLine(x-1, y, x+1, y);
        painter->drawLine(x, y-1, x, y+1);
        break;
    }

	case SYMBOL_CIRCLE:
	{
        painter->drawArc (QRect(x-ptsize, y-ptsize, ptsize*2, ptsize*2), 0, 5760);//360*16);
        break;
	}

    // ����������
    case SYMBOL_TRIANGLE_UP:
    {
        QPoint pt1 = QPoint(d0, d2);
        QPoint pt2 = QPoint(d1, d0);
        QPoint pt3 = QPoint(d2, d2);
        painter->drawLine (pt1+pt, pt2+pt);
        painter->drawLine (pt2+pt, pt3+pt);
        painter->drawLine (pt3+pt, pt1+pt);
        break;
    }

    // ����������
    case SYMBOL_TRIANGLE_DOWN:
    {
        QPoint pt1 = QPoint(d0, d0);
        QPoint pt2 = QPoint(d1, d2);
        QPoint pt3 = QPoint(d2, d0);
        painter->drawLine (pt1+pt, pt2+pt);
        painter->drawLine (pt2+pt, pt3+pt);
        painter->drawLine (pt3+pt, pt1+pt);
        break;
    }

    // ������
    case SYMBOL_DIAMOND:
    {
        QPoint pt1 = QPoint(d1, d0);
        QPoint pt2 = QPoint(d2, d1);
        QPoint pt3 = QPoint(d1, d2);
        QPoint pt4 = QPoint(d0, d1);
        painter->drawLine (pt1+pt, pt2+pt);
        painter->drawLine (pt2+pt, pt3+pt);
        painter->drawLine (pt3+pt, pt4+pt);
        painter->drawLine (pt4+pt, pt1+pt);
        break;
    }

    // ��ʮ�ּ�
    case SYMBOL_CROSS:
    {
        QPoint pt1 = QPoint(d1, d0);
        QPoint pt2 = QPoint(d2, d1);
        QPoint pt3 = QPoint(d1, d2);
        QPoint pt4 = QPoint(d0, d1);
        painter->drawLine (pt1+pt, pt3+pt);
        painter->drawLine (pt2+pt, pt4+pt);
        break;
    }

    // �п�ʮ��
    case SYMBOL_EMPTYCROSS:
    {
        painter->drawLine(pt+QPoint(0, d0), pt+QPoint(0, -2));
        painter->drawLine(pt+QPoint(0, d2), pt+QPoint(0, 2));
        painter->drawLine(pt+QPoint(d0, 0), pt+QPoint(-2, 0));
        painter->drawLine(pt+QPoint(d2, 0), pt+QPoint(2, 0));
        break;
    }

    // ��������
    case SYMBOL_SQUARE:
    {
        QPoint pt1 = QPoint(d0, d0);
        QPoint pt2 = QPoint(d0, d2);
        QPoint pt3 = QPoint(d2, d2);
        QPoint pt4 = QPoint(d2, d0);
        painter->drawLine (pt1+pt, pt2+pt);
        painter->drawLine (pt2+pt, pt3+pt);
        painter->drawLine (pt3+pt, pt4+pt);
        painter->drawLine (pt4+pt, pt1+pt);
        break;
    }

    // �������μ�ʮ��
    case SYMBOL_SQUARE_CROSS:
    {
        QPoint pt1 = QPoint(d0, d0);
        QPoint pt2 = QPoint(d0, d2);
        QPoint pt3 = QPoint(d2, d2);
        QPoint pt4 = QPoint(d2, d0);

        QPoint pt5 = QPoint(d1, d0);
        QPoint pt6 = QPoint(d2, d1);
        QPoint pt7 = QPoint(d1, d2);
        QPoint pt8 = QPoint(d0, d1);
        {
            // ��������
            painter->drawLine (pt1+pt, pt2+pt);
            painter->drawLine (pt2+pt, pt3+pt);
            painter->drawLine (pt3+pt, pt4+pt);
            painter->drawLine (pt4+pt, pt1+pt);

            // ��ʮ�ּ�
            painter->drawLine (pt5+pt, pt7+pt);
            painter->drawLine (pt6+pt, pt8+pt);
        }
        break;
    }

    // ��X
    case SYMBOL_DIAG_CROSS:
    {
        QPoint pt1 = QPoint(d0, d0);
        QPoint pt2 = QPoint(d0, d2);
        QPoint pt3 = QPoint(d2, d2);
        QPoint pt4 = QPoint(d2, d0);

        painter->drawLine (pt1+pt, pt3+pt);
        painter->drawLine (pt2+pt, pt4+pt);
        break;
    }

    // б��
    case SYMBOL_BIAS:
    {
        QPoint pt1 = QPoint(d2, d0);
        QPoint pt2 = QPoint(d0, d2);
        painter->drawLine (pt1+pt, pt2+pt);
        break;
    }

    // �Ǻ�
    case SYMBOL_ASTERISK:
    {
        painter->drawLine(pt+QPoint(0, d0), pt+QPoint(0, d2));
        painter->drawLine(pt+QPoint(d0, d0), pt+QPoint(d2, d2));
        painter->drawLine(pt+QPoint(d2, d0), pt+QPoint(d0, d2));
        break;
    }

	case SYMBOL_ARROW_UP:
	{
		QPen pen = painter->pen();
		pen.setWidth(2);
		painter->setPen(pen);
        painter->drawLine(pt+QPoint(0, d0), pt+QPoint(0, d2));
        painter->drawLine(pt+QPoint(d0, 0), pt+QPoint(0, d2));
        painter->drawLine(pt+QPoint(d2, 0), pt+QPoint(0, d2));		
		pen.setWidth(1);
		painter->setPen(pen);
		break;
	}

	case SYMBOL_ARROW_DN:
	{
		QPen pen = painter->pen();
		pen.setWidth(2);
		painter->setPen(pen);
        painter->drawLine(pt+QPoint(0, d0), pt+QPoint(0, d2));
        painter->drawLine(pt+QPoint(d0, 0), pt+QPoint(0, d0));
        painter->drawLine(pt+QPoint(d2, 0), pt+QPoint(0, d0));	
		pen.setWidth(1);
		painter->setPen(pen);
		break;
	}

	case SYMBOL_HLINE:
	{
		painter->drawLine(pt+QPoint(d0, 0), pt+QPoint(d2, 0));
		painter->drawLine(pt+QPoint(d0, 1), pt+QPoint(d2, 1));
		break;
	}

	case SYMBOL_AIS:
	{		
		const int x1 = -ptsize, x2 = 0, x3 = ptsize;
		const int y1 = -1.5*ptsize, y2 = 1.5*ptsize, y3 = 2.0*ptsize;
		QPoint points[] = {QPoint(x1, y1), QPoint(x1, y2), QPoint(x2, y3), QPoint(x3, y2), QPoint(x3, y1), QPoint(x1, y1)};
		painter->drawPolyline(points, 6);
		break;
	}

	case SYMBOL_AIS_BORDER:
	{
		const int xy[] = {-1*ptsize, -0.5*ptsize, 0.5*ptsize, ptsize};
		QPoint points[] = {pt+QPoint(xy[0], xy[0]), pt+QPoint(xy[0], xy[1]), pt+QPoint(xy[0], xy[2]), pt+QPoint(xy[0], xy[3]),
			pt+QPoint(xy[0], xy[3]), pt+QPoint(xy[1], xy[3]), pt+QPoint(xy[2], xy[3]),  pt+QPoint(xy[3], xy[3]),
			pt+QPoint(xy[3], xy[3]), pt+QPoint(xy[3], xy[2]), pt+QPoint(xy[3], xy[1]),  pt+QPoint(xy[3], xy[0]),
			pt+QPoint(xy[3], xy[0]), pt+QPoint(xy[2], xy[0]), pt+QPoint(xy[1], xy[0]),  pt+QPoint(xy[0], xy[0]),
		};
		painter->drawLines(points, 8);
		break;
	}

	case SYMBOL_ATA_3:
	{
		// ������ֱ��Ϊ10���ص�ԲȦ
		painter->drawArc (x-5, y-5, 5*2, 5*2, 0, 5760);//360*16);
		// ����Χ�����߿�
		const float dwsize = ptsize * 2.0 / 5.0;
		QVector<qreal> dashes;
		dashes << ptsize << ptsize;
		QPen pen = painter->pen();
		painter->save();
		pen.setDashPattern(dashes);
		painter->setPen(pen);
		painter->setBrush(Qt::NoBrush);
		painter->drawRect(pt.x() - ptsize, pt.y() - ptsize, 2*ptsize, 2*ptsize);
		painter->restore();
		break;
	}
	case SYMBOL_ATA_4:
	{
		// ������ֱ��Ϊ10���ص�ԲȦ
		painter->drawArc (x-5, y-5, 5*2, 5*2, 0, 5760);//360*16);
		break;
	}
	case SYMBOL_ATA_6:
	{
		// ������ֱ��Ϊ10���ص�ԲȦ
		painter->drawArc (x-3, y-3, 3*2, 3*2, 0, 5760);//360*16);
		break;
	}
	case SYMBOL_ATA_7:
	{		
		// ������ֱ��Ϊ10���ص�ԲȦ
		painter->drawArc (x-5, y-5, 5*2, 5*2, 0, 5760);//360*16);
		// ����Χ�ĵ�������
		const int dd = ptsize;
		QPoint points[] = {QPoint(x, y-1.5*ptsize), QPoint(x-dd, y+dd), QPoint(x+dd, y+dd), QPoint(x, y-1.5*ptsize)};
		painter->drawPolyline(points, 4);
		break;
	}
	case SYMBOL_ATA_8:
	{	
		// ������ֱ��Ϊ10���ص�ԲȦ
		painter->drawArc (x-5, y-5, 5*2, 5*2, 0, 5760);//360*16);
		// ����Χ����������
		const int dd = ptsize;
		QPoint points[] = {QPoint(x, y+1.5*ptsize), QPoint(x-dd, y-dd), QPoint(x+dd, y-dd), QPoint(x, y+1.5*ptsize)};
		painter->drawPolyline(points, 4);
		break;
	}
	case SYMBOL_ATA_9:
	{	
		// ������ֱ��Ϊ10���ص�ԲȦ
		painter->drawArc (x-5, y-5, 5*2, 5*2, 0, 5760);//360*16);
		// ����Χ������
		const int dd = 1.5*ptsize;
		QPoint points[] = {QPoint(x-dd, y), QPoint(x, y+dd), QPoint(x+dd, y), QPoint(x, y-dd),QPoint(x-dd, y)};
		painter->drawPolyline(points, 5);
		break;
	}
	case SYMBOL_ATA_12:
	{		
		// ������ֱ��Ϊ10���ص�ԲȦ
		painter->drawArc (x-5, y-5, 5*2, 5*2, 0, 5760);//360*16);
		// ����Χ�ľ���
		QPoint points[] = {QPoint(x-ptsize, y-ptsize), QPoint(x-ptsize, y+ptsize), QPoint(x+ptsize, y+ptsize), QPoint(x+ptsize, y-ptsize),QPoint(x-ptsize, y-ptsize)};
		painter->drawPolyline(points, 5);
		break;
	}
    default:
    {
        break;
    }
    }	// end switch
}

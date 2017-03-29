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
 * 帧点迹对象列表迭代器
 */
class PlotFrameListIterator
{
public:
    PlotFrameListIterator(const PLOTFRAMELIST& plotFrameList)
    {
        frame = plotFrameList.constBegin();
        frame_end = plotFrameList.constEnd();

        // 查找到第一个点迹对象的位置
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
            // 到本帧下一类型点迹列表
            ++plotlist;
            findNextPlot();
        }
        return pplot;
    }

private:
    void findNextPlot()
    {
        // 先处理当前帧，再处理下一帧，确保(*plot)有效
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

// 用于遍列帧点迹列表中的所有点迹对象
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
    // 释放所有的点迹对象
    clear(true);
    // 释放所有的点迹类型对象
    unregisterType();
}

// 新的点迹处理
void Plot::enterPlot(LPPLOTINFO info)
{
 //   if(!m_radarView0)
   //     return;

    QMutexLocker lock(&m_mutex);

    // 获取当前帧对象
    LPPLOTFRAME plotFrame = accessCurrentPlotFrame();
    if(!plotFrame)
        return;

    // 将点迹添加到当前帧
    insertPlotToFrame(plotFrame, info);
}

void Plot::enterPlot(const PLOTINFOLIST& infolist)
{
 //   if(!m_radarView0)
  //      return;

    QMutexLocker lock(&m_mutex);

    // 获取当前帧对象
    LPPLOTFRAME plotFrame = accessCurrentPlotFrame();
    if(!plotFrame)
        return;

    // 将点迹添加到当前帧
    PLOTINFOLIST::const_iterator it1 = infolist.begin(), it2 = infolist.end(), it;
    for(it=it1; it!=it2; ++it)
        insertPlotToFrame(plotFrame, *it);
}

// 添加点迹到指定帧
void Plot::insertPlotToFrame(LPPLOTFRAME frame, LPPLOTINFO info)
{
    // 申请点迹对象
    LPPLOT plot = applyPlot();
    if(!plot)
        return;

    // 点迹类型
    plot->type = (isTypeValid(info->type) ? info->type : PLOT_TYPE_UNKNOWN);

    // 逻辑错误，按正常情况下面的条件一直不成立
    if(!frame->plotGroup.contains(plot->type))
        return;

    // 计算点迹位置
    PLOTPOSITION& position = plot->position;
    // 从公里转换到当前系统距离量纲
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

    // 保存点迹参数
    plot->aziWidth = info->aziWidth;
    plot->rngWidth = info->rngWidth;

    // 保存额外数据
    if(info->extraData)
        addExtraData(plot, info->extraData);

    // 将点迹对象挂到帧对象的点迹链中
    frame->plotGroup[plot->type]->plotList.append(plot);

    frame->plotCount ++;

    //qDebug() << "insertPlotToFrame ok";
}

// 按时间更新点迹显示
void Plot::updatePlotKeepTime(const quint32 time0)
{
    // 计算前后两次处理的时间差,并保存当前处理时间.
    const int dtime = time0 - m_disappearTime;
    m_disappearTime = time0;

    // 如果时间异常变化，则不处理
    if(dtime <= 0 || dtime >= 5)
        return;

    QMutexLocker lock(&m_mutex);
    PLOTFRAMELIST::iterator it1 = m_plotFrameList.begin(), it2 = m_plotFrameList.end(), it;
    for(it=it1; it!=it2; )
    {
        // 更新帧保留时间
        LPPLOTFRAME frame = (*it);
        frame->keepingTime += dtime;

        // 判断帧里面每一组点迹是否过期
        const quint8 dt = frame->keepingTime;
        PLOTGROUPHASH& gtable = frame->plotGroup;
        PLOTGROUPHASH::const_iterator it_g1 = gtable.constBegin(), it_g2 = gtable.constEnd(), it_g;
        for(it_g=it_g1; it_g!=it_g2; ++it_g)
        {
            LPPLOTGROUP group = (*it_g);
            // 如果过期，则释放一组点迹
            if(dt >= group->keepTime)
            {
                (*it)->plotCount -= group->plotList.size();
                freePlot(group->plotList);
            }
        }

        // 如果当前帧点迹数为0，则释放当前帧对象
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

/*更新所有目标的屏幕坐标(所有视图)*/
void Plot::updateScreenPoint()
{
 //   if(!m_radarView0)
 //       return;

    QMutexLocker lock(&m_mutex);

    quint32 displayflag[MAXVIEWCOUNT];
    for(int i=0;i<MAXVIEWCOUNT; i++)
        displayflag[i] = (1<<i);

    // 对每一个点迹对象,计算其屏幕坐标
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

/*更新所有目标的屏幕坐标（指定视图）*/
void Plot::updateScreenPoint(quint8 idx)
{
  //  if(!m_radarView0)
    //    return;

    if(idx >= m_radarViewCount)
        return;

    QMutexLocker lock(&m_mutex);

    quint32 displayflag = (1 << idx);

  //  CRadarView* view = m_radarView[idx];
    // 对每一个点迹对象,计算其屏幕坐标
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

/*更新所有目标的直角坐标和屏幕坐标，距离量纲变化时调用*/
void Plot::updateSquareAndScreenPoint()
{
 //   if(!m_radarView0)
   //     return;

    QMutexLocker lock(&m_mutex);

    quint32 displayflag[MAXVIEWCOUNT];
    for(int i=0;i<MAXVIEWCOUNT; i++)
        displayflag[i] = (1<<i);

    // 对每一个点迹对象,计算其直角坐标、极坐标、屏幕坐标
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

/*目标位置移动处理(直角坐标和屏幕坐标相应的移动)*/
void Plot::move (const QPointF& sq, const QList<QPoint>& sc, quint8 index)
{
 //   if(!m_radarView0)
    //    return;

    QMutexLocker lock(&m_mutex);

    quint32 displayflag[MAXVIEWCOUNT];
    for(int i=0;i<MAXVIEWCOUNT; i++)
        displayflag[i] = (1<<i);

    // 对每一个点迹对象,计算移动其直角坐标、屏幕坐标
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

// 清除所有点迹
void Plot::clear(bool toDelete)
{
    QMutexLocker lock(&m_mutex);

    freePlotFrame(toDelete);

    if(toDelete)
    {
        // 删除自由链中点迹帧对象
        DeleteObjectInContainer(PLOTFRAMELIST, m_freePlotFrame);
        // 删除自由链中的点迹组对象
        DeleteObjectInContainer(PLOTGROUPLIST, m_freePlotGroup);
        // 删除自由链中的点迹对象
        DeleteObjectInContainer(PLOTLIST, m_freePlotList);
    }
}

// 申请一个点迹对象
LPPLOT Plot::applyPlot()
{
    if(m_freePlotList.empty())
        return new PLOT;
    else
        return m_freePlotList.takeLast();
}

// 申请一个点迹组对象
LPPLOTGROUP Plot::applyPlotGroup()
{
    if(m_freePlotGroup.empty())
        return new PLOTGROUP;
    else
        return m_freePlotGroup.takeLast();
}

// 申请一个点迹帧对象
LPPLOTFRAME Plot::applyPlotFrame()
{
    LPPLOTFRAME plotFrame = NULL;
    if(m_freePlotFrame.empty())
    {
        plotFrame = new PLOTFRAME;

        // 申请组对象
        PLOTTYPEINFOHASH::const_iterator it1 = m_typeInfo.constBegin(), it2=m_typeInfo.constEnd(), it;
        for(it=it1; it!=it2; ++it)
        {
            if(LPPLOTGROUP group = applyPlotGroup())
            {
                group->keepTime = (*it)->keepTime;  // 类型指定保留时间
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

// 访问当前帧对象
LPPLOTFRAME Plot::accessCurrentPlotFrame()
{
    // 如果有当前帧对象,并且没有发生帧改变,则返回当前帧对象,
    // 否则创造新的帧对象作为当前帧对象
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

// 判断当前帧是否发生改变
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

// 释放一个点迹
void Plot::freePlot(LPPLOT plot)
{
    m_freePlotList.append(plot);
}

// 释放一组点迹
void Plot::freePlot(PLOTLIST& plotList)
{
    if(!plotList.empty())
    {
        // 将所有的点迹对象链接到自由点迹链尾
#ifndef _MSVC_
        m_freePlotList.append(plotList);
#else
		listAppend<LPPLOT>(m_freePlotList, plotList);
#endif
        // 清空点迹链
        plotList.clear();
    }
}

// 释放一组点迹
void Plot::freePlot(PLOTGROUPHASH& groupHash)
{
    PLOTGROUPHASH::const_iterator it1 = groupHash.constBegin(), it2 = groupHash.constEnd(), it;
    for(it=it1; it!=it2; ++it)
    {
        freePlot((*it)->plotList);
    }
}

// 释放一组点迹
void Plot::freePlotGroup(LPPLOTGROUP plotGroup)
{
    if(!plotGroup->plotList.empty())
    {
        // 将所有的点迹对象链接到自由点迹链尾
#ifndef _MSVC_
        m_freePlotList.append(plotGroup->plotList);
#else
		listAppend<LPPLOT>(m_freePlotList, plotGroup->plotList);
#endif
        // 清空点迹链
        plotGroup->plotList.clear();
    }

    m_freePlotGroup.append(plotGroup);
}

// 释放一组点迹
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

// 释放帧点迹对象
void Plot::freePlotFrame(LPPLOTFRAME plotFrame, bool freeGroup)
{
    if(!freeGroup)
        freePlot(plotFrame->plotGroup);
    else
        freePlotGroup(plotFrame->plotGroup);
    m_freePlotFrame.append(plotFrame);
}

// 释放帧点迹对象
void Plot::freePlotFrame(bool freeGroup)
{
    PLOTFRAMELIST::const_iterator it1 = m_plotFrameList.begin(), it2 = m_plotFrameList.end(), it;
    for(it=it1; it!=it2; ++it)
    {
        freePlotFrame(*it, freeGroup);
        // 需要注意,由于释放帧对象时,将帧对象挂到了自由帧对象链中,故这里不能删除帧对象
        // delete (*it);
    }

    m_plotFrameList.clear();
}

// 注册点迹类型(建议在程序初始化时一次注册完所有的点迹类型)
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

// 设置点迹类型标志
void Plot::setTypeFlag(quint8 type, quint8 flags)
{
	QMutexLocker locker(&m_mutex);
    if(LPPLOTTYPEINFO info = m_typeInfo.value(type, NULL))
    {
        info->flags = flags;
    }
}

// 设置点迹的保留时间
void Plot::setTypeKeepTime(quint8 type, quint8 keeptime)
{
	QMutexLocker locker(&m_mutex);
    if(LPPLOTTYPEINFO info = m_typeInfo.value(type, NULL))
    {
        info->keepTime = keeptime;
    }
}

// 设置点迹颜色
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

// 删除点迹类型
void Plot::unregisterType(quint8 type)
{
    if(type != PLOT_TYPE_UNKNOWN && m_typeInfo.contains(type))
    {
        LPPLOTTYPEINFO info = m_typeInfo.take(type);
        delete info;
    }
}

// 删除点迹类型对象
void Plot::unregisterType()
{
    DeleteObjectInContainer(PLOTTYPEINFOHASH, m_typeInfo);
}

// 绘制函数
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

            // 判断是否显示该类型的点迹
            if((!isPlotTypeShow(type)) || plotlist.empty())
                continue;

            LPPLOTTYPEINFO& plotType = m_typeInfo[type];
			if(keepingTime >= plotType->keepTime)
				continue;

            rate = 1.0 - keepingTime / (float)(plotType->keepTime);

            // 设置画笔颜色
            QColor color = plotColor(type);
            // 设置点迹大小
            char ptsize = plotType->symbolSize;//m_plotSize;

            // 颜色渐处理
            if(plotType->colorEvade)
            {
                int h, s, v;
                color.getHsv(&h, &s, &v);
                color.setHsv(h, s, v*rate);
            }

            // 大小渐变处理
            if(plotType->sizeEvade)
            {
                ptsize = (char)(ptsize*rate);
            }

            //qDebug() << "plot color:" << color << plotColor(type) << rate << type;
            painter->setPen(color);

            // 显示该类型点迹链上所有点迹
            PLOTLIST::const_iterator it_p1 = plotlist.begin(), it_p2 = plotlist.end(), it_p;
            for(it_p = it_p1; it_p != it_p2; ++it_p)
            {
                LPPLOT plot = (*it_p);
                //qDebug() << "paint plot:" << plot->position.screen_point[idx].toPoint();
//                drawPlotSymbol(painter, plot->type, plot->position.screen_point[idx].toPoint(), ptsize);
                if(plot->position.displayFlag & displayFlag)
                {
                    // 绘制点迹眉毛
                    if(isPlotBrowShow(plot->type))
                        drawPlotBrow(painter, plot, idx);
                    // 绘制点迹符号
                    if(isPlotSymbolShow(plot->type))
						drawSymbol(painter, plotType->symbol, plot->position.screen_point[idx].toPoint(), ptsize);
                        //drawPlotSymbol(painter, plot->type, plot->position.screen_point[idx].toPoint(), ptsize);
                    // 绘制点迹标牌
                    if(isPlotBoardShow(plot))
                        drawPlotBoard(painter, plot, idx);
                }
            }
        }
    }
}

// 绘制一个点迹符号
void Plot::drawPlotSymbol(QPainter* painter, const int type, const QPoint& pt, char ptsize)
{
    // 缩小到一个点
    if(ptsize <= 0)
    {
        painter->drawPoint(pt);
        return;
    }

    switch (type)
    {
    // 画正三角形
    case PLOT_TYPE_PRIMARY:			// 一次点迹
    case PLOT_TYPE_MULFUSE:			// 多站融合点迹
    case PLOT_TYPE_FILTER:			// 滤波点迹
    {
        drawSymbol(painter, SYMBOL_TRIANGLE_UP, pt, ptsize);
        break;
    }

    // 画倒三角形
    case PLOT_TYPE_SECONDARY:		// 二次点迹
    case PLOT_TYPE_IIIREQUEST:		// III型询问机点迹
    {
        drawSymbol(painter, SYMBOL_TRIANGLE_DOWN, pt, ptsize);
        break;
    }

    // 画菱形
    case PLOT_TYPE_PRIMARYSEC:		// 一二次相关点迹
    case PLOT_TYPE_PRIMARYIII:		// 一次III型询问机相关点迹
    {
        drawSymbol(painter, SYMBOL_DIAMOND, pt, ptsize);
        break;
    }

    // 画十字
    default:
    {
        drawSymbol(painter, SYMBOL_CROSS, pt, ptsize);
        break;
    }
    }	// end switch
}

// 绘制点迹眉毛
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

        // 画直线
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
        {	// 画直线
            painter->drawLine(sc1.x(), sc1.y(), sc2.x(), sc2.y());
        }
        else
        {	// 画弧线
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

// 绘制点迹标牌
void Plot::drawPlotBoard(QPainter* painter, LPPLOT plot, quint8 viewindex)
{
}

// 获取指定类型点迹显示颜色
quint32  Plot::plotColor(quint8 type) const
{
    return m_typeInfo.contains(type) ? \
            m_typeInfo[type]->color : 0;
}

// 判断当前类型的点迹是否显示
bool Plot::isPlotTypeShow(quint8 type) const
{
    return m_typeInfo.contains(type) ? \
            m_typeInfo[type]->displayFlag : false;
}

// 判断是否显示点迹符号
bool Plot::isPlotSymbolShow(quint8 type) const
{
    return m_typeInfo.contains(type) ? \
            m_typeInfo[type]->displaySymb : false;
}

// 判断是否显示点迹眉毛
bool Plot::isPlotBrowShow(quint8 type) const
{
    return m_typeInfo.contains(type) ? \
            m_typeInfo[type]->displayBrow : false;
}

// 判断是否显示点迹标牌
bool Plot::isPlotBoardShow(LPPLOT plot)
{
    return false;
}

// 在指定位置画指定类型的符号
void drawSymbol(QPainter* painter, quint8 type, const QPoint& pt, quint8 ptsize)
{
    const int d0 = -ptsize, d1 = 0, d2 = ptsize;
    const int x = pt.x(), y = pt.y();
    switch (type)
    {
    // 小圆点
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

    // 画正三角形
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

    // 画倒三角形
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

    // 画菱形
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

    // 画十字架
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

    // 中空十字
    case SYMBOL_EMPTYCROSS:
    {
        painter->drawLine(pt+QPoint(0, d0), pt+QPoint(0, -2));
        painter->drawLine(pt+QPoint(0, d2), pt+QPoint(0, 2));
        painter->drawLine(pt+QPoint(d0, 0), pt+QPoint(-2, 0));
        painter->drawLine(pt+QPoint(d2, 0), pt+QPoint(2, 0));
        break;
    }

    // 画正方形
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

    // 画正方形加十字
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
            // 画正方形
            painter->drawLine (pt1+pt, pt2+pt);
            painter->drawLine (pt2+pt, pt3+pt);
            painter->drawLine (pt3+pt, pt4+pt);
            painter->drawLine (pt4+pt, pt1+pt);

            // 画十字架
            painter->drawLine (pt5+pt, pt7+pt);
            painter->drawLine (pt6+pt, pt8+pt);
        }
        break;
    }

    // 画X
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

    // 斜线
    case SYMBOL_BIAS:
    {
        QPoint pt1 = QPoint(d2, d0);
        QPoint pt2 = QPoint(d0, d2);
        painter->drawLine (pt1+pt, pt2+pt);
        break;
    }

    // 星号
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
		// 画中心直径为10象素的圆圈
		painter->drawArc (x-5, y-5, 5*2, 5*2, 0, 5760);//360*16);
		// 画外围的虚线框
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
		// 画中心直径为10象素的圆圈
		painter->drawArc (x-5, y-5, 5*2, 5*2, 0, 5760);//360*16);
		break;
	}
	case SYMBOL_ATA_6:
	{
		// 画中心直径为10象素的圆圈
		painter->drawArc (x-3, y-3, 3*2, 3*2, 0, 5760);//360*16);
		break;
	}
	case SYMBOL_ATA_7:
	{		
		// 画中心直径为10象素的圆圈
		painter->drawArc (x-5, y-5, 5*2, 5*2, 0, 5760);//360*16);
		// 画外围的倒三角形
		const int dd = ptsize;
		QPoint points[] = {QPoint(x, y-1.5*ptsize), QPoint(x-dd, y+dd), QPoint(x+dd, y+dd), QPoint(x, y-1.5*ptsize)};
		painter->drawPolyline(points, 4);
		break;
	}
	case SYMBOL_ATA_8:
	{	
		// 画中心直径为10象素的圆圈
		painter->drawArc (x-5, y-5, 5*2, 5*2, 0, 5760);//360*16);
		// 画外围的正三角形
		const int dd = ptsize;
		QPoint points[] = {QPoint(x, y+1.5*ptsize), QPoint(x-dd, y-dd), QPoint(x+dd, y-dd), QPoint(x, y+1.5*ptsize)};
		painter->drawPolyline(points, 4);
		break;
	}
	case SYMBOL_ATA_9:
	{	
		// 画中心直径为10象素的圆圈
		painter->drawArc (x-5, y-5, 5*2, 5*2, 0, 5760);//360*16);
		// 画外围的菱形
		const int dd = 1.5*ptsize;
		QPoint points[] = {QPoint(x-dd, y), QPoint(x, y+dd), QPoint(x+dd, y), QPoint(x, y-dd),QPoint(x-dd, y)};
		painter->drawPolyline(points, 5);
		break;
	}
	case SYMBOL_ATA_12:
	{		
		// 画中心直径为10象素的圆圈
		painter->drawArc (x-5, y-5, 5*2, 5*2, 0, 5760);//360*16);
		// 画外围的矩形
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

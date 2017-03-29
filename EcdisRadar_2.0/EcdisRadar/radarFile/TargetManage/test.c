
class plot
{
public:
    // 更新点迹保持时间
    void updatePlotKeepTime(QSHORT delTime);
    // 点迹消失处理(时间单位为100ms)
    void updatePlotDispear(QUINT32 tm);
    // 更新所有点迹的屏幕坐标
    void updatePlotScreenCoor(int windowflag);

    // 获取显示点迹标志
    bool IsPlotShow () const;
    bool IsPlotShow (QBYTE plotType) const;
    bool IsPlotDopplerShow (QBYTE plotType, QBYTE dopplerChn);
    bool IsPlotADShow (QBYTE plotType, QBYTE ad);
    // 获取显示点迹眉毛标志
    bool IsPlotBrowShow(QBYTE plotType) const;
    // 获取显示点迹符号标志
    bool IsPlotSymbShow(QBYTE plotType) const;
    // 获取颜色衰变标志
    bool IsPlotColorEvade(QBYTE plotType) const;
    // 获取点迹颜色模式
    QBYTE PlotColorMode (QBYTE plotType) const;
    // 获取多普勒频道
    QUSHORT PlotDopplerChannel (QBYTE plotType) const;
    // 获取点迹AD值分级显示控制
    QUSHORT PlotADLevelCtrl (QBYTE plotType) const;
    // 获取点迹保持时间
    QSHORT PlotTypeKeepTime(QBYTE plotType) const;

    // 由AD值确定点迹的颜色
    QColor plotColorFromAD (QBYTE advalue);
    // 由多普勒频道号确定点迹的颜色
    QColor plotColorFromDopplerChannel (QBYTE dopplerChn);
    // 由杂波标志获取固定颜色模式下的点迹颜色
    QColor plotColorFromNoiseFlag (QBYTE plotType, QBYTE noiseFlag);

    // 设置点迹显示标志
    void SetPlotDispFlag (QBYTE dispflag);
    void SetPlotBrowShowFlag (QBYTE plotType, bool show = true);
    void SetPlotSymbShowFlag (QBYTE plotType, bool show = true);
    void SetPlotColorEvadeFlag (QBYTE plotType, bool flag = true);
    void SetPlotColorMode (QBYTE plotType, QBYTE colormode);
    void SetPlotDopplerChn (QBYTE plotType, QUSHORT chn);
    void SetPlotADLevelCtrl (QBYTE plotType, QUSHORT levelCtrl);
    // 设置点迹保持时间
    void setPlotKeepTime(QBYTE plotType, QSHORT keepTime);

protected:
    // 由点迹类型获取对应的点迹类型索引
    char  PlotTypeIndex (QBYTE plotType) const;
    QBYTE PlotTypeFromIndex(char index) const;

    // 初始化点迹类型信息
    inline void initPlotTypeInfo();
    // 判断点迹类型是否有效
    inline bool isPlotTypeValid (QBYTE plotType) const
    {
        return (plotType >= POINT_TYPE_PRIMARY && \
            plotType <= POINT_TYPE_FILTER);
    }
    // 申请一个回波点
    inline ECHOPLOT* allocOneEchoPlot();
    // 释放一个回波点
    inline void freeOneEchoPlot (ECHOPLOT* plot);
    // 释放所有的点迹数据
    inline void freeAllPlots();
    // 删除所有的点迹数据
    inline void deleteAllPlots ();

private:
    // 点迹类型信息
    QHash<QBYTE, char>		PlotTypeTable;	// 点迹类型索引表
    PLOTTYPEINFO			PlotTypeInfo[MAX_PLOT_TYPE];
    //QList<ECHOPLOT*>		EchoPlots;		// 当前回波点链(所有的回波点都挂在该链上)
    QList<ECHOPLOT*>		EchoPlots[MAX_PLOT_TYPE];		// 当前回波点链(每一种类型的回波点挂在对应的链上)
    QList<ECHOPLOT*>		FreeEchoPlots;	// 自由回波点链
    QUINT32					EchoPlotMemoryCount;	// 已经申请的回波点数
    QMutex			LockForPlot;
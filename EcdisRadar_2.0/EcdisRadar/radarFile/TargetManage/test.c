
class plot
{
public:
    // ���µ㼣����ʱ��
    void updatePlotKeepTime(QSHORT delTime);
    // �㼣��ʧ����(ʱ�䵥λΪ100ms)
    void updatePlotDispear(QUINT32 tm);
    // �������е㼣����Ļ����
    void updatePlotScreenCoor(int windowflag);

    // ��ȡ��ʾ�㼣��־
    bool IsPlotShow () const;
    bool IsPlotShow (QBYTE plotType) const;
    bool IsPlotDopplerShow (QBYTE plotType, QBYTE dopplerChn);
    bool IsPlotADShow (QBYTE plotType, QBYTE ad);
    // ��ȡ��ʾ�㼣üë��־
    bool IsPlotBrowShow(QBYTE plotType) const;
    // ��ȡ��ʾ�㼣���ű�־
    bool IsPlotSymbShow(QBYTE plotType) const;
    // ��ȡ��ɫ˥���־
    bool IsPlotColorEvade(QBYTE plotType) const;
    // ��ȡ�㼣��ɫģʽ
    QBYTE PlotColorMode (QBYTE plotType) const;
    // ��ȡ������Ƶ��
    QUSHORT PlotDopplerChannel (QBYTE plotType) const;
    // ��ȡ�㼣ADֵ�ּ���ʾ����
    QUSHORT PlotADLevelCtrl (QBYTE plotType) const;
    // ��ȡ�㼣����ʱ��
    QSHORT PlotTypeKeepTime(QBYTE plotType) const;

    // ��ADֵȷ���㼣����ɫ
    QColor plotColorFromAD (QBYTE advalue);
    // �ɶ�����Ƶ����ȷ���㼣����ɫ
    QColor plotColorFromDopplerChannel (QBYTE dopplerChn);
    // ���Ӳ���־��ȡ�̶���ɫģʽ�µĵ㼣��ɫ
    QColor plotColorFromNoiseFlag (QBYTE plotType, QBYTE noiseFlag);

    // ���õ㼣��ʾ��־
    void SetPlotDispFlag (QBYTE dispflag);
    void SetPlotBrowShowFlag (QBYTE plotType, bool show = true);
    void SetPlotSymbShowFlag (QBYTE plotType, bool show = true);
    void SetPlotColorEvadeFlag (QBYTE plotType, bool flag = true);
    void SetPlotColorMode (QBYTE plotType, QBYTE colormode);
    void SetPlotDopplerChn (QBYTE plotType, QUSHORT chn);
    void SetPlotADLevelCtrl (QBYTE plotType, QUSHORT levelCtrl);
    // ���õ㼣����ʱ��
    void setPlotKeepTime(QBYTE plotType, QSHORT keepTime);

protected:
    // �ɵ㼣���ͻ�ȡ��Ӧ�ĵ㼣��������
    char  PlotTypeIndex (QBYTE plotType) const;
    QBYTE PlotTypeFromIndex(char index) const;

    // ��ʼ���㼣������Ϣ
    inline void initPlotTypeInfo();
    // �жϵ㼣�����Ƿ���Ч
    inline bool isPlotTypeValid (QBYTE plotType) const
    {
        return (plotType >= POINT_TYPE_PRIMARY && \
            plotType <= POINT_TYPE_FILTER);
    }
    // ����һ���ز���
    inline ECHOPLOT* allocOneEchoPlot();
    // �ͷ�һ���ز���
    inline void freeOneEchoPlot (ECHOPLOT* plot);
    // �ͷ����еĵ㼣����
    inline void freeAllPlots();
    // ɾ�����еĵ㼣����
    inline void deleteAllPlots ();

private:
    // �㼣������Ϣ
    QHash<QBYTE, char>		PlotTypeTable;	// �㼣����������
    PLOTTYPEINFO			PlotTypeInfo[MAX_PLOT_TYPE];
    //QList<ECHOPLOT*>		EchoPlots;		// ��ǰ�ز�����(���еĻز��㶼���ڸ�����)
    QList<ECHOPLOT*>		EchoPlots[MAX_PLOT_TYPE];		// ��ǰ�ز�����(ÿһ�����͵Ļز�����ڶ�Ӧ������)
    QList<ECHOPLOT*>		FreeEchoPlots;	// ���ɻز�����
    QUINT32					EchoPlotMemoryCount;	// �Ѿ�����Ļز�����
    QMutex			LockForPlot;
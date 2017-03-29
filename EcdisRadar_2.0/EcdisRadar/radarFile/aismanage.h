#ifndef AISMANAGE_H
#define AISMANAGE_H

#include <QObject>
#include <QMutex>

#include "define.h"
#include "TargetManage/TargetManage_global.h"


#define MAXAISNUM 4
#define MAXAISNUM_OVER 3

class AisManage : public QObject
{
public:
    AisManage();
    virtual ~AisManage();

    //更新AIS船动态
    void updateAisShips(quint32 tm);

    //删除所有AIS船显示
    void clearAisShips();

    //获取AIS船数量
    int getAisShipCount() const
    {  return m_aisShips.size();   }

    void setColor(quint32 color)
    {   m_color = color;  }

    // 添加或更新一条AIS船舶静态信息
    void addorUpdateAisShip(unsigned int MMSI, SHIP_STATIC_INFO* pShipStaticInfo, quint8 aisclass);
    // 更新AIS船舶动态信息
    void updateAisDynamicInfo(unsigned int MMSI, AIS_DYNAMIC_INFO* pAisDynamicInfo, quint8 aisclass);
    // 获取AIS船舶信息
    AIS_SHIP* getAisShip(unsigned int MMSI, bool lock=true);
    // 判断MMSI号是否有效
    bool isShipValid(quint32 MMSI);
    // 判断某位置点是否选中AIS船舶，返回船泊MMSI号
    quint32 ptOnAisShip(const QPoint& pt);

    // 添加模拟AIS船舶
    void addSimuAis(int flag);

    // 模拟AIS船舶运动
    void simuAisMove(unsigned int interval);

    void initializeNewShip(unsigned int MMSI);

    // 设置当前选择船泊
    void setSelectedShip(unsigned int MMSI);

    // 更新所有的船名显示
    void updateShipNameDisplay();

    // 本船位置改变
    void ownshipPositionChanged();

    void paint(QPainter* p);

    // CPA报警
    void cpaAlarm();
    void updateScreenPoint();

protected:
    // CPA报警
    void cpaAlarm(const AIS_SHIP& ship);
    void enterPoint(AIS_SHIP* lpShip, const LATITUDE_POINT& pt);

private:
    QMutex m_mutex;
    // AIS船舶列表
    QHash<unsigned int, AIS_SHIP> m_aisShips;
    // 计算AIS导航信息
    void calAisNavigationInfo(AIS_SHIP* pAisShip);

    QHash<quint32, int> m_aisStaticInfos;
    QHash<quint32, int> m_aisDynamicInfos;

    quint32 m_selectedShipMMSI;


    QColor m_color;


    double max[MAXAISNUM_OVER];
    quint32 mmsi[MAXAISNUM_OVER];
};

#endif // AISMANAGE_H

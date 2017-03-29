#ifndef BOATALARM_H
#define BOATALARM_H


#include "define.h"

#include <QMutex>
#include <QStandardItemModel>


//#define SQLITE_USE


typedef enum ALARMLEVEL{
    ALARM_LEVEL0 = 0,
    ALARM_LEVEL1,
    ALARM_LEVEL2,
    ALARM_LEVEL3,
    ALARM_LEVEL4,
    ALARM_LEVEL5,
    ALARM_LEVEL6,
    ALARM_LEVEL7,
}ALARMLEVEL;

class Speaker;
class AlarmRecorder;




class Alarm : public QObject
{
public:
    Alarm();
    virtual ~Alarm();

    // 启动报警，输入参数为报警等级和相关信息
    void startAlarm(ALARMLEVEL level, const QString& information="");
    // 停止报警
    void stopAlarm();
    //清空报警信息
    void clearAlarmInfo();

    void setAlarmRecorder(AlarmRecorder* obj)
    {	m_lpAlarmRecorder = obj;	}

    // 根据报警等级，控制扬声器发声
    void sound(ALARMLEVEL level);

    // 在界面上显示报警信息
    void showMessage(const QString& information);

    // 记录报警信息
    void record(ALARMLEVEL level, const QString& information="");

protected:



private:
    QMutex      m_mutex;
    Speaker*    m_lpSpeaker;
    AlarmRecorder* m_lpAlarmRecorder;

    QHash<QString, quint32> m_alarmInfo;

};



class AlarmRecorder : public QObject
{
public :
    AlarmRecorder(QObject* parent=NULL);
    ~AlarmRecorder();

    // 添加告警信息记录
    bool addRecord(int level, const QString& info);
    // 获取一个时间段的告警信息记录，其结果放到model中
    bool getRecord(quint32 starttime, quint32 stoptime, QStandardItemModel* model);

protected:
    // 打开数据库
    bool openDatabase();
    // 关闭数据库
    void closeDatabase();
     // 根据时间获取对应的表名
    QString getTableName(quint32 datetime);

private:
    QMutex	m_mutex;

    //数据库接口
#ifdef SQLITE_USE
    sqlite3*	m_db;
#endif
};




class boatalarm : public QObject
{
public:
    boatalarm(QObject *parent = NULL);
    virtual ~boatalarm();


    //设置本船静态信息
    void setStaticInfo(SHIP_STATIC_INFO &info);
    //设置本船动态信息
    void setDynamicInfo(AIS_DYNAMIC_INFO &info);
    //设置船艏向
    void setOwnshpHeading(float heading);
    //设置对水速度
    void setWaterSpeed(float speed);
    //设置对地速度,速度来源  0:LOG 1:GPS 2：手动
    void setGroupSpeed(float sog, quint8 src);
    //设置航向 航向来源  0:LOG 1:GPS 2：手动
    void setCourse(float course, quint8 src);


    //更新本船动态信息
    void updateDynamicInfo();

    //获取本船静态信息
    void staticInfo(SHIP_STATIC_INFO &info) const
    {   info = m_staticInfo;   }
    //获取本船动态信息
    void dynamicInfo(AIS_DYNAMIC_INFO &info)  const
    {   info = m_dynamicInfo;  }
    //获取本船位置
    void ownshpPosition(double &lat, double &lon)
    {
        lat = m_crntLat;
        lon = m_crntLon;
    }
    //本船位置是否可用
    quint8 ownShipPositionValid() const
     {  return m_ownshipPositionValid;  }

    //重置更新时间
    void resetUpdateTime();



    //设置本船经纬度，测试用
    void setShipPosition(double lat, double lon);


  protected:
    // 位置发生变化,海图报警、航线报警、目标报警
    void positionChanged(quint8 flag=0);
    // 速度发生变化，如果指定TCPA，海图报警、目标报警
    void speedChanged(quint8 flag=0);
    //对水速度发生变化
    void waterSpeedChanged(quint8 flag=0);
     // 航向发生变化
    void courseChanged(quint8 flag=0);
     // 船艏发生变化
    void headlineChanged(quint8 flag=0);





private:
    QMutex mutex;

    //本船静态信息
    SHIP_STATIC_INFO m_staticInfo;
    //本船动态信息
    AIS_DYNAMIC_INFO m_dynamicInfo;

    double m_crntLat,m_crntLon;  //当前经纬度
    quint8 m_sogsrc,m_coursesrc;   //速度和航向来源

    //上次船位刷新时间
    quint32 m_lastOwnShpRefreshTime;

    //本船变化标志
    quint8 m_changedFlag;

    //本船位置有效标志
    quint8 m_ownshipPositionValid;

};

#endif // BOATALARM_H

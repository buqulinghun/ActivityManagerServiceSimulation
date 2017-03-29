#ifndef RECORDER_H
#define RECORDER_H

#include <QObject>
#include <QMutex>
#include <QStandardItemModel>

#include "sqlite3/sqlite3.h"
#include "mainwindow.h"


class AlarmRecorder : public QObject
{

public:
     AlarmRecorder(QObject *parent = 0);
     ~AlarmRecorder();

     // 添加告警信息记录
     bool addRecord(int level, const QString& info);
     // 获取一个时间段的告警信息记录，其结果放到model中
     bool getRecord(quint32 starttime, quint32 stoptime, QStandardItemModel* model);


protected:
     bool openDatabase();   //打开数据库
     void closeDatabase();   //关闭数据库

     // 根据时间获取对应的表名
     QString getTableName(quint32 datetime);

private:
     /*数据库锁，因为并没有多个同时读取的情况，所以就用这个，考虑使用QReadWriteLock  */
     QMutex mutex;

     sqlite3* alarm_db;
};







// 航行记录信息
typedef struct tagSailInfo
{
        quint32 datetime;//日期时间
        qint32 longitude; //经度
        qint32 latitude;//纬度
        quint32 speed;//速度
        quint32 cource;//航向
        quint32 heading;//船艏向
}SAILRCDINFO, * LPSAILRCDINFO;
/*********************************************************
class:  船位置记录类
date:   2016-03-28
************************************************************/
class SailRecorder : public QObject
{
public :
    SailRecorder(QObject* parent=NULL);
    ~SailRecorder();


    // 添加航行记录
    bool addRecord(const SAILRCDINFO& sailInfo);
    // 获取一个时间段的航行记录，其结果放到model中
    bool getRecord(quint32 starttime, quint32 stoptime, quint32 interval, QStandardItemModel* model);

protected:
    // 打开数据库
    bool openDatabase();
    // 关闭数据库
    void closeDatabase();
    // 根据时间获取对应的表名
    QString getTableName(quint32 datetime);

private:
    QMutex mutex;

    sqlite3* sail_db;
    quint32 m_lastLongitude, m_lastLatitude;   //最新经纬度
};

#endif // RECORDER_H

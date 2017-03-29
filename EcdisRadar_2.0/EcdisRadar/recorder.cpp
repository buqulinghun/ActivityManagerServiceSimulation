#include "recorder.h"

#include <QDateTime>
#include <QDebug>
#include <stdio.h>
#include <math.h>






QString Longitude2String(double lon, bool enflag);
QString Latitude2String(double lat, bool enflag);



AlarmRecorder::AlarmRecorder(QObject* parent): QObject(parent)
{
    alarm_db = NULL;

    if(!openDatabase())
        qDebug() << "cann't open alarmrecord database success!";
}

AlarmRecorder::~AlarmRecorder()
{
    closeDatabase();
}

// 添加告警信息记录
bool AlarmRecorder::addRecord(int level, const QString& info)
{

    QMutexLocker locker(&mutex);

    int result;
    const quint32 time0 = time(0);

    // 创建表格
    const QString tableName = getTableName(time0);
    QString crttbl = QString("CREATE TABLE IF NOT EXISTS '%1' (datetime INTEGER , level INTEGER , message TEXT)").arg(tableName);
    result = sqlite3_exec(alarm_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);

    // 插入记录，每个VALUE必须用''括起来，否则插入不成功
    QString insert = QString("INSERT INTO '%1' VALUES ('%2', '%3', '%4')").arg(tableName).arg(time0).arg(level).arg(info);
    result = sqlite3_exec(alarm_db, insert.toLocal8Bit().constData(), 0, 0, 0);

    return (SQLITE_OK == result);
}

#define SQLSELECT(tbl,tm1,tm2) QString("SELECT * FROM '%1' WHERE datetime >= '%2' AND datetime <= '%3'").arg(tbl).arg(tm1).arg(tm2);
#define SetTableItem(model, row, column, data)	\
{		\
    QStandardItem *item = new QStandardItem;	\
    item->setData(data, Qt::DisplayRole);      \
    model->setItem(row, column, item);		\
}
#define SetTableItem2(model, row, column, disp, data)	\
{							\
    QStandardItem *item = new QStandardItem;	       \
    item->setData(disp, Qt::DisplayRole);		\
    item->setData(data, Qt::UserRole);			\
    model->setItem(row, column, item);			\
}
// 获取一个时间段的告警信息记录，其结果放到model中
bool AlarmRecorder::getRecord(quint32 starttime, quint32 stoptime, QStandardItemModel* model)
{
    QMutexLocker locker(&mutex);

    sqlite3_stmt* stmt;
    int result, row = 0;;

    //model->clear();

    QDateTime dt1 = QDateTime::fromTime_t(starttime);
    QDateTime dt2 = QDateTime::fromTime_t(stoptime);
    for(QDateTime dt = dt1; dt <= dt2; dt = dt.addMonths(1))    //查找一个月之类
    {

        const QString tableName = getTableName(dt.toTime_t());
        const QString select = SQLSELECT(tableName, starttime, stoptime);
        result = sqlite3_prepare_v2(alarm_db, select.toLocal8Bit().constData(), select.size(), &stmt, NULL);
        if( result != SQLITE_OK ){
         //   fprintf(stderr, "SQL error: sqlite3_prepare\n");
            continue;
        }

        do
        {
            result = sqlite3_step(stmt);
            if(SQLITE_ROW == result)
            {
                int datetime = sqlite3_column_int(stmt, 0);   //获取时间
                int level = sqlite3_column_int(stmt, 1);      //获取报警等级
                QString info = QString::fromLocal8Bit((const char*)sqlite3_column_text(stmt, 2));   //获取报警内容
                const QString strDate = QDateTime::fromTime_t(datetime).toString("yyyy-MM-dd hh:mm:ss");
                //SetTableItem(model, row, 0, datetime);
                SetTableItem2(model, row, 0, strDate, datetime);
                SetTableItem(model, row, 1, level);
                SetTableItem(model, row, 2, info);
                row ++;
            }
            else
                break;
        }while(1);

        result = sqlite3_finalize(stmt);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_finalize\n");
        }
    }

    return true;
}


bool AlarmRecorder::openDatabase()
{
    if(alarm_db)
        return true;

    int result;
    result = sqlite3_open_v2("data/alarmRecord.db", &alarm_db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, NULL);
    if( result != SQLITE_OK ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(alarm_db));
        sqlite3_close(alarm_db);
        alarm_db = NULL;
        return false;
    }

    return true;
}

void AlarmRecorder::closeDatabase()
{
    if(!alarm_db)
        return;

    sqlite3_close(alarm_db);
    alarm_db = NULL;
}

// 根据时间获取对应的表名
QString AlarmRecorder::getTableName(quint32 datetime)
{
    //以月份创建表名，即一个月的记录作为一个表
    QDateTime dt = QDateTime::fromTime_t(datetime);
    QDate date = dt.date();
    return QString("%1-%2").arg(date.year()).arg(date.month(), 2, 10, QLatin1Char('0'));
}















/*********************************************************
class:  船位置记录类
date:   2016-03-28
************************************************************/
SailRecorder::SailRecorder(QObject *parent): QObject(parent)
{
    sail_db = NULL;
    if(!openDatabase())
        qDebug() << "cann't open sail database success!";
}
SailRecorder::~SailRecorder()
{
    closeDatabase();
}


// 添加告警信息记录
bool SailRecorder::addRecord(const SAILRCDINFO& info)
{

    // 只有当经纬度变化大于1秒时才记录
    if(abs((long)(m_lastLongitude - info.longitude)) < 1 &&
            abs((long)(m_lastLatitude - info.latitude)) < 1)
        return false;

    QMutexLocker locker(&mutex);

    int result;
    const quint32 time0 = info.datetime;

    // 创建表格
    const QString tableName = getTableName(time0);
    QString crttbl = QString("CREATE TABLE IF NOT EXISTS '%1' (datetime INTEGER, longitude INTEGER, \
                                                     latitude INTEGER, speed INTEGER, cource INTEGER, heading INTEGER)").arg(tableName);
    result = sqlite3_exec(sail_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);

    // 插入记录
    QString insert = QString("INSERT INTO '%1' VALUES ('%2', '%3', '%4', '%5', '%6', '%7')").arg(tableName).arg(info.datetime)\
            .arg(info.longitude).arg(info.latitude).arg(info.speed).arg(info.cource).arg(info.heading);
    result = sqlite3_exec(sail_db, insert.toLocal8Bit().constData(), 0, 0, 0);

    // 保存经纬度
    m_lastLongitude = info.longitude;
    m_lastLatitude = info.latitude;

    return (SQLITE_OK == result);
}

//上面定义过
#undef SQLSELECT
#undef SetTableItem
#undef SetTableItem2

#define SQLSELECT(tbl,tm1,tm2) QString("SELECT * FROM '%1' WHERE datetime >= '%2' AND datetime <= '%3'").arg(tbl).arg(tm1).arg(tm2);
#define SetTableItem(model, row, column, data)	\
{					\
    QStandardItem *item = new QStandardItem;	\
    item->setData(data, Qt::DisplayRole);		\
    model->setItem(row, column, item);			\
}
#define SetTableItem2(model, row, column, disp, data)	\
{							\
    QStandardItem *item = new QStandardItem;	\
    item->setData(disp, Qt::DisplayRole);		\
    item->setData(data, Qt::UserRole);			\
    model->setItem(row, column, item);			\
}
// 获取一个时间段的告警信息记录，其结果放到model中
bool SailRecorder::getRecord(quint32 starttime, quint32 stoptime, quint32 interval, QStandardItemModel* model)
{
    QMutexLocker locker(&mutex);
    //model->clear();

    sqlite3_stmt* stmt;
    int result, row = 0;

    quint32 lastTime = 0;
    QDateTime dt1 = QDateTime::fromTime_t(starttime);
    QDateTime dt2 = QDateTime::fromTime_t(stoptime);
    for(QDateTime dt = dt1; dt <= dt2; dt = dt.addMonths(1))
    {
        const QString tableName = getTableName(dt.toTime_t());
        const QString select = SQLSELECT(tableName, starttime, stoptime);
        result = sqlite3_prepare(sail_db, select.toLocal8Bit().constData(), select.size(), &stmt, 0);
        if( result != SQLITE_OK ){
            //fprintf(stderr, "SQL error: sqlite3_prepare\n");
            continue;
        }

        do
        {
            result = sqlite3_step(stmt);
            if(SQLITE_ROW == result)
            {
                quint32 crntTime = sqlite3_column_int(stmt, 0);
                if(crntTime >= lastTime + interval)   //查询的航行记录之间的时间间隔
                {
                    double lon = sqlite3_column_int(stmt, 1)/3600.0;
                    double lat = sqlite3_column_int(stmt, 2)/3600.0;
                    QString timestr = QDateTime::fromTime_t(crntTime).toString("yyyy-MM-dd hh:mm:ss");
                    SetTableItem2(model, row, 0, timestr, crntTime);
                    SetTableItem2(model, row, 1, Longitude2String(lon, true), lon);
                    SetTableItem2(model, row, 2, Latitude2String(lat, true), lat);
                    SetTableItem(model, row, 3, int(sqlite3_column_int(stmt, 3)));
                    SetTableItem(model, row, 4, int(sqlite3_column_int(stmt, 4)));
                    SetTableItem(model, row, 5, int(sqlite3_column_int(stmt, 5)));
                    row ++;
                    lastTime = crntTime;
                }
            }
            else
                break;
        }while(1);

        result = sqlite3_finalize(stmt);
        if(result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_finalize\n");
        }
    }

    return true;
}


bool SailRecorder::openDatabase()
{
    if(sail_db)
       return true;

    int result;
    result = sqlite3_open_v2("data/sailRecord.db", &sail_db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, NULL);
    if(result != SQLITE_OK ){
       fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(sail_db));
       sqlite3_close(sail_db);
       sail_db = NULL;
       return false;
    }

    return true;
}

void SailRecorder::closeDatabase()
{
    if(!sail_db)
        return;

    sqlite3_close(sail_db);
    sail_db = NULL;
}

// 根据时间获取对应的表名
QString SailRecorder::getTableName(quint32 datetime)
{
    //每月作为表名
    QDateTime dt = QDateTime::fromTime_t(datetime);
    QDate date = dt.date();
    return QString("%1-%2").arg(date.year()).arg(date.month(), 2, 10, QLatin1Char('0'));
}










// 获取经度字符串
QString Longitude2String(double lon, bool enflag)
{
    int degree = (int)fabs(lon);
    double minute = (fabs(lon) - degree) * 60.0;
    QString sLon = QString("%1").arg(degree) + ('°') + QString("%2'").arg(minute, 6, 'f', 3, QLatin1Char('0'));     //°
    if(lon>=0)
    {
        if(enflag)
            sLon += "E";
        else
            sLon = QString::fromLocal8Bit("东经") + sLon;
    }
    else
    {
        if(enflag)
            sLon += "W";
        else
            sLon = QString::fromLocal8Bit("西经") + sLon;
    }
    return sLon;
}

// 获取纬度字符串
QString Latitude2String(double lat, bool enflag)
{
    int degree = (int)fabs(lat);
    double minute = (fabs(lat) - degree) * 60.0;
    QString sLon = QString("%1").arg(degree) + ('°') + QString("%2'").arg(minute, 5, 'f', 3, QLatin1Char('0'));     //°
    if(lat>=0)
    {
        if(enflag)
            sLon += "N";
        else
            sLon = QString::fromLocal8Bit("北纬") + sLon;
    }
    else
    {
        if(enflag)
            sLon += "S";
        else
            sLon = QString::fromLocal8Bit("南纬") + sLon;
    }
    return sLon;
}







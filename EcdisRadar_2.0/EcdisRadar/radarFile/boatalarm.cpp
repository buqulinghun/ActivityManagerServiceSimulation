#include "boatalarm.h"
#include "dialog.h"
#include "boatinfo.h"
#include "speaker.h"
#include "CustomEvent.h"
#include "mainwindow.h"
#include "glwidget.h"
#include "aismanage.h"

#include <QMutexLocker>
#include <math.h>
#include <QApplication>

quint8 updateOwnShipDynamic = 0;

extern Dialog*   m_lpDialog;
extern SYSTEMINFO SystemInfo;
extern MENUCONFIG MenuConfig;
extern Dialog*   m_lpDialog;
extern GLWidget *pView;
extern AisManage* lpAisManage;
extern MainWindow *lpMainWindow;

extern quint8 flag_isSpeaking;



enum {
    POS_CHANGED_FLAG = 0x01,
    SOG_CHANGED_FLAG = 0x02,  //对地速度
    SOW_CHANGED_FLAG = 0x04,  //对水速度
    COURSE_CHANGED_FLAG = 0x08,
    HEADL_CHANGED_FLAG = 0x10,
};

/********************class Alarm****************************/
Alarm::Alarm()
{
    // 创建扬声器对象
    m_lpSpeaker = new Speaker;


    /****************没有记录***********************/
    m_lpAlarmRecorder = NULL;
}

Alarm::~Alarm()
{
    // 删除扬声器对象
    if(m_lpSpeaker)
    {
        delete m_lpSpeaker;
    }
}

// 启动报警，输入参数为报警等级和相关信息
void Alarm::startAlarm(ALARMLEVEL level, const QString& information)
{
    QMutexLocker locer(&m_mutex);

    const quint32 tm0 = SystemInfo.crnt_time;
    //30秒之后再次报警
    if(m_alarmInfo[information] > 0 && qAbs((int)m_alarmInfo[information] - (int)tm0) < 30){
     /*   if(!flag_isSpeaking)
            sound(level);
        else */
            return;

     }
    // 删除所有过期的消息
    const quint32 tm_expired = tm0 - 60;
    QHash<QString, quint32>::iterator it = m_alarmInfo.begin(), it2 = m_alarmInfo.end();
    while( it != it2)
    {
        if(it.value() < tm_expired)
            it = m_alarmInfo.erase(it);  //it会自动向下移动一个
        else
            it ++;
    }

    m_alarmInfo[information] = tm0;//写值入哈希表中去


    // 在界面上显示报警信息
    if(information.length()>0)
    {
        showMessage(information);
    }
    // 根据报警等级，控制扬声器发声
    sound(level);
    // 记录报警信息
    record(level, information);
}

void Alarm::clearAlarmInfo()
{
    m_alarmInfo.clear();
    if(lpMainWindow)
        lpMainWindow->clearAlarmInfo();
}

// 停止报警
void Alarm::stopAlarm()
{
    m_mutex.lock();

    // 停止扬声器发声
    if(m_lpSpeaker)
    {
        m_lpSpeaker->stopSounding();
    }

    m_mutex.unlock();
}

// 记录报警信息
void Alarm::record(ALARMLEVEL level, const QString& information)
{
    /*
    if(m_lpAlarmRecorder)
        m_lpAlarmRecorder->addRecord(level, information); */

    // 以添加方式打开或创建报警记录文件
    QFile file(CURRENTPATH + "/Alarm.dat");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        return;
    }
    // 时间
    QDateTime dt = QDateTime::currentDateTime();
    QDate date = dt.date();
    QTime time = dt.time();
    QString text = QString("%1-%2-%3:%4-%5-%6,%7,%8\n").arg(date.year()).arg(date.month()).arg(date.day()) \
                   .arg(time.hour()).arg(time.minute()).arg(time.second()).arg(level).arg(information);
    // 添加一条报警信息
    QTextStream out(&file);
    out << text;
    file.close();
}

// 根据报警等级，控制扬声器发声
void Alarm::sound(ALARMLEVEL level)
{
    if(!m_lpSpeaker)
        return;

    //菜单不允许报警
    if(!MenuConfig.otherMenu.audibleWarningEnable)
        return;

    // 根据报警等级，设置扬声器发声频率
    // 报警等级越高，发声频率越快
    int timeInterval;
    switch(level)
    {
    case ALARM_LEVEL0:
        timeInterval = 1400;
        break;
    case ALARM_LEVEL1:
        timeInterval = 1200;
        break;
    case ALARM_LEVEL2:
        timeInterval = 1000;
        break;
    case ALARM_LEVEL3:
        timeInterval = 800;
        break;
    case ALARM_LEVEL4:
        timeInterval = 600;
        break;
    case ALARM_LEVEL5:
        timeInterval = 400;
        break;
    case ALARM_LEVEL6:
        timeInterval = 200;
        break;
    case ALARM_LEVEL7:
        timeInterval = 100;
        break;
    default:
        timeInterval = 400;
        break;
    }
    // 设置扬声器发声模式
    QBitArray mode;
    mode.resize(2);
    mode[0] = true;
    mode[1] = false;
    // 设置发声次数
    quint8 repeat = 5;
    // 开始发声
    extern quint8 flag_isSpeaking;
    if(!flag_isSpeaking)
        m_lpSpeaker->startSounding(mode, repeat, timeInterval);
}

// 在界面上显示报警信息  发出报警消息  用户应用程序线程
void Alarm::showMessage(const QString& information)
{
    CustomEvent* evt = new CustomEvent(QEvent::Type(MSGID_AlarmInfo), information);
    QApplication::postEvent(m_lpDialog, evt);//发出evt消息给g_lpDialog,发送事件这个机制还不错

}


/********************************AlarmRecorder***************************************/
AlarmRecorder::AlarmRecorder(QObject* parent)
: QObject(parent)
{
#ifdef SQLITE_USE
    m_db = NULL;
#endif
    openDatabase();
}

AlarmRecorder::~AlarmRecorder()
{
    closeDatabase();
}

// 添加告警信息记录
bool AlarmRecorder::addRecord(int level, const QString& info)
{
    QMutexLocker locker(&m_mutex);

#ifdef SQLITE_USE
    int rc;
    const quint32 time0 = time(0);

    // 创建表格
    const QString tableName = getTableName(time0);
    QString crttbl = QString("CREATE TABLE IF NOT EXISTS '%1' (datetime INTEGER , level INTEGER , message TEXT)").arg(tableName);
    rc = sqlite3_exec(m_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);

    // 插入记录，每个VALUE必须用''括起来，否则插入不成功
    QString insert = QString("INSERT INTO '%1' VALUES ('%2', '%3', '%4')").arg(tableName).arg(time0).arg(level).arg(info);
    rc = sqlite3_exec(m_db, insert.toLocal8Bit().constData(), 0, 0, 0);

    return (SQLITE_OK == rc);

#endif
}

#define SQLSELECT(tbl) QString("SELECT * FROM '%1' WHERE datetime >= '%2' AND datetime <= '%3'").arg(tbl).arg(starttime).arg(stoptime);
#define SetTableItem(model, row, column, data)	\
{												\
        QStandardItem *item = new QStandardItem;	\
        item->setData(data, Qt::DisplayRole);		\
        model->setItem(row, column, item);			\
}
#define SetTableItem2(model, row, column, disp, data)	\
{												\
        QStandardItem *item = new QStandardItem;	\
        item->setData(disp, Qt::DisplayRole);		\
        item->setData(data, Qt::UserRole);			\
        model->setItem(row, column, item);			\
}
// 获取一个时间段的告警信息记录，其结果放到model中
bool AlarmRecorder::getRecord(quint32 starttime, quint32 stoptime, QStandardItemModel* model)
{
        QMutexLocker locker(&m_mutex);

#ifdef SQLITE_USE
        sqlite3_stmt* stmt;
        int rc, row = 0;;

        //model->clear();

        QDateTime dt1 = QDateTime::fromTime_t(starttime);
        QDateTime dt2 = QDateTime::fromTime_t(stoptime);
        for(QDateTime dt = dt1; dt <= dt2; dt = dt.addMonths(1))
        {
                const QString tableName = getTableName(dt.toTime_t());
                const QString select = SQLSELECT(tableName);
                rc = sqlite3_prepare(m_db, select.toLocal8Bit().constData(), select.size(), &stmt, 0);
                if( rc != SQLITE_OK ){
                        //fprintf(stderr, "SQL error: sqlite3_prepare\n");
                        continue;
                }

                do
                {
                        rc = sqlite3_step(stmt);
                        if(SQLITE_ROW == rc)
                        {
                                int datetime = sqlite3_column_int(stmt, 0);
                                int level = sqlite3_column_int(stmt, 1);
                                QString info = QString::fromLocal8Bit((const char*)sqlite3_column_text(stmt, 2));
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

                rc = sqlite3_finalize(stmt);
                if( rc != SQLITE_OK ){
                        fprintf(stderr, "SQL error: sqlite3_finalize\n");
                }
        }

#endif

        return true;
}


bool AlarmRecorder::openDatabase()
{
#ifdef SQLITE_USE
    if(m_db)
        return true;

    int rc;
    rc = sqlite3_open("alarmRecord.db", &m_db);
    if( rc ){
        //fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(m_db);
        m_db = NULL;
        return false;
    }
#endif
    return true;
}

void AlarmRecorder::closeDatabase()
{
#ifdef SQLITE_USE
    if(!m_db)
        return;

    sqlite3_close(m_db);
    m_db = NULL;
#endif
}

// 根据时间获取对应的表名
QString AlarmRecorder::getTableName(quint32 datetime)
{
    QDateTime dt = QDateTime::fromTime_t(datetime);
    QDate date = dt.date();
    return QString("%1-%2").arg(date.year()).arg(date.month(), 2, 10, QLatin1Char('0'));
}







/****************************class boatalarm*********************************/

boatalarm::boatalarm(QObject *parent) : QObject(parent)
{
    m_crntLat = m_crntLon = 181;

    memset(&m_staticInfo, 0, sizeof(SHIP_STATIC_INFO));
    memset(&m_dynamicInfo, 0,sizeof(AIS_DYNAMIC_INFO));

    //初始化上次本船刷新时间
    m_lastOwnShpRefreshTime = 0;


    m_changedFlag = 0;
    m_ownshipPositionValid = 0;

}

boatalarm::~boatalarm()
{

}

//设置本船静态信息
void boatalarm::setStaticInfo(SHIP_STATIC_INFO &info)
{
    QMutexLocker locker(&mutex);

    m_staticInfo = info;
}
//设置本船动态信息
void boatalarm::setDynamicInfo(AIS_DYNAMIC_INFO &info)
{
    QMutexLocker locker(&mutex);

    if(qAbs(info.lat) > 90 || qAbs(info.lon) > 180) {
        qDebug()<<"ship position error:"<< info.lat<< info.lon;
        return;
    }
    //经纬度变化超过一定距离  位置变化(1.0e-3度 = 3.6秒)
    if((!EQUAL3(info.lat, m_crntLat, 0.5e-3)) || (!EQUAL3(info.lon, m_crntLon, 0.5e-3))) {
        //记录当前位置
        m_crntLat = info.lat;
        m_crntLon = info.lon;
        m_changedFlag |= POS_CHANGED_FLAG;
    }

    m_dynamicInfo.dateTime = info.dateTime;
    m_dynamicInfo.lat = info.lat;
    m_dynamicInfo.lon = info.lon;
    setGroupSpeed(info.sog, 1);   //GPS 速度
    setCourse(info.course, 1);   //GPS航向


    updateOwnShipDynamic = 1;  //更新提示


}

// 设置船艏向
void boatalarm::setOwnshpHeading(float heading)
{
    if(!IsDataValid1(heading, 0, 360))
        return;

    //qDebug() << "setOwnshpHeading:" << heading;
    if(!EQUAL3(m_dynamicInfo.heading, heading, 0.1)){
        m_dynamicInfo.heading = heading;
        m_changedFlag |= HEADL_CHANGED_FLAG;
        updateOwnShipDynamic = 1;

       // SystemInfo.ShipInfo.head = heading;
    }
}

// 设置对水速度
void boatalarm::setWaterSpeed(float speed)
{
    //qDebug() << __FUNCTION__ << speed << m_dynamicInfo.speed;
    if(!IsDataValid1(speed, 0, 200))
        return;

    if(!EQUAL3(m_dynamicInfo.speed, speed, 0.1)){
            m_dynamicInfo.speed = speed;
            m_changedFlag |= SOW_CHANGED_FLAG;
            updateOwnShipDynamic = 1;
    }
}

// 设置对地速度 速度来源，0：LOG，1：GPS，2：手动
void boatalarm::setGroupSpeed(float sog, quint8 src)
{
    if(!IsDataValid1(sog, 0, 200))
        return;

    if(src < 3)
    {
        m_sogsrc = src;
        SystemInfo.ShipInfo.vspeed[src] = sog;   //保存速度
       // if(src == MenuConfig.otherMenu.speedSelect)
        {
             m_dynamicInfo.sog = sog;
             m_changedFlag |= SOG_CHANGED_FLAG;
             updateOwnShipDynamic = 1;
        }
    }
}

// 设置航向 航向来源，0：罗经，1：GPS，2：手动
void boatalarm::setCourse(float course, quint8 src)
{
    if(!IsDataValid1(course, 0, 360))
                return;

    if(src < 3)
    {
        m_coursesrc = src;
        SystemInfo.ShipInfo.vcourse[src] = course;
       // if(src == MenuConfig.otherMenu.corseSelect)  //应该跟选择没有关系
        {
             m_dynamicInfo.course = course;
             m_changedFlag |= COURSE_CHANGED_FLAG;
             updateOwnShipDynamic = 1;
        }
    }
}


void boatalarm::updateDynamicInfo()
{
    //qDebug() << "OwnShipMonitor::updateDynamicInfo";
    QMutexLocker locker(&mutex);

    // 保存本船位置
    g_ownshpLon = m_crntLon;
    g_ownshpLat = m_crntLat;

    // 位置更新处理
    if(m_changedFlag & POS_CHANGED_FLAG)
        positionChanged();
    // 速度更新处理
    if(m_changedFlag & SOG_CHANGED_FLAG)
        speedChanged();
    // 对水速度更新处理
    if(m_changedFlag & SOW_CHANGED_FLAG)
        waterSpeedChanged();
    // 航向更新处理
    if(m_changedFlag & COURSE_CHANGED_FLAG)
        courseChanged();
    // 船艏更新处理
    if(m_changedFlag & HEADL_CHANGED_FLAG)
        headlineChanged();

    if(m_changedFlag)
        m_lpDialog->updateBoatInfoDisplay();

    // clear flag
    m_changedFlag = 0;
    updateOwnShipDynamic = 0;
}



// 重置更新时间
void boatalarm::resetUpdateTime()
{
     m_lastOwnShpRefreshTime = 0;
}



// 位置发生变化,海图报警、航线报警、目标报警
void boatalarm::positionChanged(quint8 flag)
{
    const double lon = m_dynamicInfo.lon*M_PI/180, lat = m_dynamicInfo.lat*M_PI/180;
    SystemInfo.ShipInfo.position = QPointF(lon, lat);

    pView->ownshipPositionChanged();
    lpAisManage->ownshipPositionChanged();
}

// 速度发生变化，如果指定TCPA，海图报警、目标报警
void boatalarm::speedChanged(quint8)
{
    SystemInfo.ShipInfo.sog = m_dynamicInfo.sog;
}

//对水速度发生变化
void boatalarm::waterSpeedChanged(quint8)
{
    SystemInfo.ShipInfo.speed = m_dynamicInfo.speed;
}

// 航向发生变化
void boatalarm::courseChanged(quint8)
{
    pView->setBoatCorse(m_dynamicInfo.course);
}

// 船艏发生变化
void boatalarm::headlineChanged(quint8)
{
    pView->setBoatHeading(m_dynamicInfo.heading);
}




//测试用
void boatalarm::setShipPosition(double lat, double lon)
{
    AIS_DYNAMIC_INFO info;
    info.lat = lat;
    info.lon = lon;

    setDynamicInfo(info);

}























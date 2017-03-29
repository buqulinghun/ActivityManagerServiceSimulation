/********************************************************************
 *日期: 2016-01-15
 *作者: 王名孝
 *作用: 电子海图主函数
 *修改:
 ********************************************************************/
#include <QtGui/QApplication>
#include <QTextCodec>
#include <QDebug>
#include <QStandardItemModel>
#include <QTableView>
#include <QString>
#include <QFile>
#include <QDesktopWidget>

#include "mainwindow.h"
#include "configuration.h"
#include "ecdis.h"
#include "recorder.h"
#include "radarFile/interact.h"
#include "radarFile/dataprocess.h"
#include "radarFile/radaritem.h"


double centerLong = 110.25;   //海图中心经纬度,单位为度
double centerLat = 39.65;
float centerLongScreen = 0;   //转换后在场景中的位置
float centerLatScreen = 0;

/****************变量声明************************/
MARINERSELECT MarinerSelect;                 //航海人员选择项
Configuration *configuration =  NULL;        //所需配置文件
DataBase *dataBase = NULL;                 //海图加载数据库
AlarmRecorder* alarmRecorder = NULL;          //数据记录对象
SailRecorder* sailRecorder = NULL;
MainWindow* lpMainWindow = NULL;              //主窗口
Symbols *symbols = NULL;                  //加载物标图形


SYSTEMINFO RadarSysinfo;    //雷达相关设置
Interact * lpInteract = NULL;    //雷达接受线程
DataProcess *lpDataProcess = NULL;   //接受数据处理对象
RadarItem *lpRadarItem = NULL;      //雷达图像绘制对象


/****************函数声明*************************/
void initRecorder();
void loadMenuConifg();
void saveMenuConfig();
void initRadarDisplay();


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    QTextCodec::setCodecForTr(QTextCodec::codecForLocale());  //支持中文

    loadMenuConifg();
    initRecorder();   //初始化记录对象

    //加载配置文件
    configuration = new Configuration;
    configuration->Load();

    //加载雷达设置
    initRadarDisplay();


    MainWindow w;
    w.Init();
    // 暂时屏蔽
    w.showFullScreen();


    return a.exec();
}

void initRecorder()
{
    alarmRecorder = new AlarmRecorder;
    alarmRecorder->addRecord(4, "test the alarm recorder 2!");

    sailRecorder = new SailRecorder;
    SAILRCDINFO sailInfo;
    const QDate dates = QDate(2016, 1, 1);
    sailInfo.datetime = QDateTime(dates).toTime_t();
    sailInfo.longitude = 129.2 * 3600;  //转换为秒
    sailInfo.latitude = 29.32 * 3600;
    sailInfo.speed = 48.3;
    sailInfo.cource = 68.5;
    sailInfo.heading = 65.3;
    sailRecorder->addRecord(sailInfo);

    //qDebug() << "test the record ok!";
}


void saveMenuConfig()
{
    QFile file("menu.cfg");
    if(!file.open(QIODevice::WriteOnly)) {
        qDebug() << "can't open menu.cfg for write.";
        return;
    }
    file.write((char*)&MarinerSelect, sizeof(MarinerSelect));
    file.close();
}

void loadMenuConifg()
{
    QFile file("menu.cfg");
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug() << "can't open menu.cfg for write.";
        return;
    }
    const QByteArray ba = file.readAll();
    file.close();
    if(ba.size() == sizeof(MarinerSelect))
    memcpy((char*)(&MarinerSelect), ba.constData(), sizeof(MarinerSelect));
    MarinerSelect.radarShow = false;
    MarinerSelect.scaleMatch = false;

}


void initRadarDisplay()
{
    lpInteract = new Interact;
    lpInteract->initialize();
    lpDataProcess = new DataProcess;
}

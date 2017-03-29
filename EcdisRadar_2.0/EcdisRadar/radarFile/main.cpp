/****************************************************************************
file name: main.cpp
author: wang ming xiao
date: 2015/07/05
comments:
***************************************************************************/
#include <QtGui/QApplication>
#include <QTextCodec>
#include <QtGui/QDesktopWidget>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QSplashScreen>
#include <QTranslator>
#include <QString>


#include "boatinfo.h"
#include "mainwindow.h"
#include "interact.h"
#include "openglview.h"
#include "mouseoperation.h"
#include "computerinfo.h"
#include "parsedevice.h"
#include "boatalarm.h"
#include "define.h"
#include "aismanage.h"
#include "glwidget.h"
#include "dataprocess.h"
#include "recordreplay/recordreplay.h"



MainWindow *lpMainWindow = NULL;   //主窗口对象
GLWidget *pView = NULL; //显示视图窗口对象
Interact *lpInteract = NULL;    //通信对象，UDP处理线程,并解包处理
MouseOperation *lpMouseOpetarion = NULL;  //鼠标操作对象

ComputerInfo *lpComputerInfo = NULL;  //电脑硬件信息
Dialog*   m_lpDialog = NULL;  //监视窗口对象
ParseDevice *lpParseDevice = NULL;  //串口数据解析对象
boatalarm* m_boatAlarm = NULL;  //本船动态监视对象
AisManage* lpAisManage = NULL;  //AIS监控对象
Alarm* lpAlarm = NULL;   //声音报警对象
DataProcess* lpDataProcess = NULL;  //数据处理接口

Plot*  lpPlot = NULL;     //点迹信息
Recordreplay* lpRecplay = NULL;   //重演对象


SYSTEMINFO SystemInfo;    //系统信息
MENUCONFIG MenuConfig;   //操作菜单配置对象
COLORCONFIG *lpColorConfig = NULL;   //颜色配置对象
SYSTEM_PARA g_systemPara; //系统参数

/********************全局变量定义********************/
int screenWidth, screenHeight;  //屏幕高宽
quint8 initFlag = 0;  //初始化成功标志
char producer[15];   //保存生产商信息，占12个字节
char serialid[20];   //保存序列号

double g_cLon;                          // 海图中心经度
double g_cLat;                          // 海图中心纬度
double g_ownshpLon = 121.57;                     // 本船经度
double g_ownshpLat = 29.17;                     // 本船纬度



/****************函数声明*************************/
void initSystemInfo();
void loadMenuConfig();
void setDefaultMenuConfig();
void saveMenuConfig();
void loadColorConfig();
void parseColor(quint32 *colors, const QString strColor);
void toSetStyleSheet(quint8 flag);
void saveComputerInfo(quint8 flag);
void initTargetObject();
void initRecordReplay();


/****************主函数程序************************/
int main(int argc, char *argv[])
{

    DPRINT("Enter main");
    initFlag = 0;
    QApplication a(argc, argv);
    //设置格式才能正确显示
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    QTextCodec::setCodecForTr(QTextCodec::codecForLocale());


    memset(producer, 0, sizeof(producer));
    memset(serialid, 0, sizeof(serialid));

    //开机画面
  /*  QSplashScreen *splash = new QSplashScreen;
    QPixmap pix("./sea_radar.png");
    splash->setPixmap(pix);
    splash->show();
    splash->showMessage("Setting up the main window......", Qt::AlignCenter, Qt::red); */


    //获取屏幕大小
    QDesktopWidget *desktop = QApplication::desktop();
    screenWidth = desktop->width();
    screenHeight = desktop->height();

    //初始化系统信息
    initSystemInfo();

    //加载语言设置
    QTranslator trans;
    if(MenuConfig.installMenu.langSelect)
        trans.load(CURRENTPATH + "/RadarDisplay_en.qm");
    else
        trans.load(CURRENTPATH + "/RadarDisplay_zh_cn.qm");
    a.installTranslator(&trans);

  //  MenuConfig.installMenu.producer[0] = 'd';
  //  MenuConfig.installMenu.serialId[0] = 4;
    //第一次启动获取电脑信息
    if(MenuConfig.installMenu.firstSatrt == 1) {
        saveComputerInfo(1);
        MenuConfig.installMenu.firstSatrt = 20;
        saveMenuConfig();  //保存信息
    }else{
    //进行电脑信息匹配
        if(MenuConfig.installMenu.bandSelect){
            lpComputerInfo = new ComputerInfo;
            lpComputerInfo->GetCpuProducerInfo(producer);
            lpComputerInfo->GetCpuSerialNumber(serialid);

            if((strcmp(producer, MenuConfig.installMenu.producer) != 0)  \
                || (strcmp(serialid, MenuConfig.installMenu.serialId) != 0)) { //相等返回0
                QMessageBox box;
                box.setWindowTitle(QObject::tr("警告"));
                box.setIcon(QMessageBox::Warning);
                box.setText(QObject::tr("电脑与雷达显示软件不匹配，请与维护人员联系！"));
                box.setStandardButtons(QMessageBox::Yes);
                QTimer::singleShot(5000, &box, SLOT(close()));
               // if(box.exec() == QMessageBox::Yes)
                    return box.exec();

            }
        }
    }


    //初始化主窗口框架
    MainWindow w;
    toSetStyleSheet(0);
    w.showFullScreen();
    w.setColorMode(MenuConfig.dispMenu.colorSelect);  
    lpMainWindow->updateScreen(MenuConfig.dispMenu.screenBright);


    //创建本船报警对象
    if(!lpAlarm)
        lpAlarm = new Alarm;
    //创建本船监视对象
    if(!m_boatAlarm) {
        m_boatAlarm = new boatalarm();
    }
    //创建AIS管理对象
    if(!lpAisManage)
        lpAisManage = new AisManage;
    lpAisManage->setColor(lpColorConfig->color_ais[MenuConfig.dispMenu.colorSelect]);

    //初始化目标
    initTargetObject();
    //初始化重演
    initRecordReplay();

    //初始化各种操作对象
    lpInteract = new Interact;
    lpInteract->initialize();
    lpMouseOpetarion = new MouseOperation;
    lpMouseOpetarion->setMouseProcessID(NULL_PROCESS);
    lpParseDevice = new ParseDevice;   //设备数据解析对象
    lpDataProcess = new DataProcess;



  /*  sleep(3);
    splash->finish(&w);
    delete splash;*/

    initFlag = 1;
    qDebug() << "Initialize ok ......";

    return a.exec();
}

//系统基本信息初始化
void initSystemInfo()
{
    loadMenuConfig();
    loadColorConfig();

    SystemInfo.currentDateTime = QDateTime::currentDateTime();
    SystemInfo.iNeedFlag.toUpdateValue = 0xffffffff;  //全部都需要更新

    SystemInfo.RangeScale.setUnits(UNITS_NM);  //海里

    //VRM/EBL
    SystemInfo.VrmEblCtrl.eblReference = 0;  //相对真北
    SystemInfo.VrmEblCtrl.vrmebl[0].show = (MenuConfig.otherMenu.vrmeblEnable & 0x03);  //占两位
    SystemInfo.VrmEblCtrl.vrmebl[0].vrm = MenuConfig.otherMenu.vrmebl[0];
    SystemInfo.VrmEblCtrl.vrmebl[0].ebl = MenuConfig.otherMenu.vrmebl[1];
    SystemInfo.VrmEblCtrl.vrmebl[1].show = ((MenuConfig.otherMenu.vrmeblEnable>>2) & 0x03);  //占两位
    SystemInfo.VrmEblCtrl.vrmebl[1].vrm = MenuConfig.otherMenu.vrmebl[2];
    SystemInfo.VrmEblCtrl.vrmebl[1].ebl = MenuConfig.otherMenu.vrmebl[3];

    //视图相关信息（向上方式/运动方式/偏心方式）
    MenuConfig.dispMenu.upmode = H_UP;   //船艏向上
    MenuConfig.dispMenu.offset = 0;  //关闭偏心
    SystemInfo.ViewDisplay.offset = MenuConfig.dispMenu.offset;
    SystemInfo.ViewDisplay.moving = MenuConfig.dispMenu.motion;
    SystemInfo.ViewDisplay.upmode = MenuConfig.dispMenu.upmode;

    // 船相关信息
    SystemInfo.ShipInfo.position = QPointF(117.26028*M_PI/180.f, 31.8735*M_PI/180.f);
    SystemInfo.ShipInfo.vcourse[2] = MenuConfig.otherMenu.manCorse;  //手动航向和速度
    SystemInfo.ShipInfo.vspeed[2] = MenuConfig.otherMenu.manSpeed;
    SystemInfo.ShipInfo.course = SystemInfo.ShipInfo.vcourse[MenuConfig.otherMenu.corseSelect];
    SystemInfo.ShipInfo.speed = SystemInfo.ShipInfo.vspeed[MenuConfig.otherMenu.speedSelect];
    SystemInfo.ShipInfo.head = 50;
    SystemInfo.ShipInfo.course = 20;

    // 碰撞告警参数
    SystemInfo.CollisionAlarm.Enable = false;
    SystemInfo.CollisionAlarm.Range = 5;
    SystemInfo.CollisionAlarm.Flicker = 1;
    SystemInfo.CollisionAlarm.VoiceAlarm = 1;

    //系统参数配置
    g_systemPara.gpsResource = 1;
    g_systemPara.alarmPara.cpaAlarmLevel = 2;
    g_systemPara.alarmPara.signalLostLevel = 4;

    //系统当前时间
    SystemInfo.crnt_time = time(0);
    //gps时间调整
    SystemInfo.gps_timeAdjust = MenuConfig.otherMenu.timeAdjust;
    SystemInfo.gps_info.lat = 91;
    SystemInfo.gps_info.lon = 181;


    //默认菜单配置选项
    MenuConfig.dispMenu.showRngRing = 0;  //不显示距离圈
    MenuConfig.dispMenu.dayNight = 0;
    SystemInfo.ViewDisplay.rngring = MenuConfig.dispMenu.showRngRing;
    MenuConfig.dispMenu.showHeadLine = 0;  //显示船艏线
    SystemInfo.ViewDisplay.guildline = MenuConfig.dispMenu.showHeadLine;
    MenuConfig.otherMenu.plotLimit = 10;
    MenuConfig.otherMenu.transmite = 0;  //不允许发射
    MenuConfig.otherMenu.tuneValue = 256;  //默认调谐值
    MenuConfig.otherMenu.tuneMan = 1;  //0手动1自动，默认自动
    MenuConfig.otherMenu.tuneAuto = 1;
    MenuConfig.installMenu.tuneMan = 0; //1:自动调谐
    MenuConfig.installMenu.tuneValue = 1;
    MenuConfig.otherMenu.guardSelect = 1;  //警戒区域自动，关闭
    MenuConfig.otherMenu.guardZoneEnable = 0;
    MenuConfig.installMenu.bandSelect = 1;  //默认绑定
    MenuConfig.otherMenu.audibleWarningEnable = 1;  //语音默认启动
 //   MenuConfig.installMenu.rangeChecked = 0xffffffff;  //量程都选中


    DPRINT("init SystemInfo ok!!!!!");

}

void initRecordReplay()
{
    lpRecplay = new Recordreplay(lpMainWindow);
    lpRecplay->setRecordPath("./record/");
    lpRecplay->setWaitMode(false);

    extern void ReplayDataProcess(const QByteArray &data, quint16 deviceid);
    lpRecplay->setReplayProcessFunc(ReplayDataProcess);

}

//加载菜单基本配置
void loadMenuConfig()
{
    QFile file(CURRENTPATH + "/menu.cfg");
    if(! file.open(QIODevice::ReadOnly))
    {
        qDebug()<< "can't open menu.cfg, set default menu config.";
        setDefaultMenuConfig();
    }else {
        const QByteArray ba = file.readAll();
        file.close();
        if(ba.size() == sizeof(MenuConfig))
            memcpy((char*)(&MenuConfig), ba.constData(), sizeof(MenuConfig));
        else
            setDefaultMenuConfig();
    }
}

void setDefaultMenuConfig()
{
    memset(&MenuConfig, 0, sizeof(MenuConfig));
    MenuConfig.installMenu.dateTime = time(0);
    MenuConfig.dispMenu.kbdBright = 100;
    MenuConfig.dispMenu.screenBright = 75;
    MenuConfig.dispMenu.fixlineBright = 100;
    MenuConfig.dispMenu.varlineBright = 100;
    MenuConfig.installMenu.aziAdjust = 0.0;
    MenuConfig.installMenu.rngAdjust = 0.0;
    MenuConfig.installMenu.mbsAdjust = 0.0;
    MenuConfig.installMenu.firstSatrt = 1;
}

void saveMenuConfig()
{
    QFile file(CURRENTPATH+ "/menu.cfg");
    if(! file.open(QIODevice::WriteOnly)) {
        qDebug()<< "can't open meun.cfg for write.";
        return;
    }
    file.write((char*)(&MenuConfig), sizeof(MenuConfig));
    file.close();
}

//加载颜色配置
void loadColorConfig()
{
    if(lpColorConfig)
        return;
    lpColorConfig = new COLORCONFIG;

    QFile file(CURRENTPATH + "/color.txt");
    if(! file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug()<< "can't open color.txt for read.";
        return;
    }

    QTextStream in(&file);
    while(! in.atEnd()) {
        QString line = in.readLine();
        if(line.isEmpty() || line.startsWith("#")) continue;
        QStringList segement = line.split(":", QString::SkipEmptyParts);
        if(segement.size() != 2) continue;
        const QString str = segement[0].trimmed();  //去掉字符串前面和后面的空格
        if(str == "EBLVRM")
            parseColor(lpColorConfig->color_eblvrm, segement[1]);
        else if(str == "ECHOBACK")
            parseColor(lpColorConfig->color_back, segement[1]);
        else if(str == "RNGRING")
            parseColor(lpColorConfig->color_rngring, segement[1]);
        else if(str == "ECHOFORE")
            parseColor(lpColorConfig->color_fore, segement[1]);
        else if(str == "ECHOLAST")
            parseColor(lpColorConfig->color_last, segement[1]);
        else if(str == "SIGNALPLOT")
            parseColor(lpColorConfig->color_signalplot, segement[1]);
        else if(str == "FILTERPLOT")
            parseColor(lpColorConfig->color_filterplot, segement[1]);
        else if(str == "ATA")
            parseColor(lpColorConfig->color_ata, segement[1]);
        else if(str == "AIS")
            parseColor(lpColorConfig->color_ais, segement[1]);
        else if(str == "HEADL")
            parseColor(lpColorConfig->color_headl, segement[1]);
    }
    file.close();
}

void parseColor(quint32 *colors, const QString strColor)
{
    //颜色格式：#RRGGBB,#RRGGBB,#RRGGBB  设置6种颜色
    QStringList segement = strColor.split(QRegExp("[,; ]"), QString::SkipEmptyParts);
    const quint8 size = qMin(segement.size(), 6);
    int i=0,j=0;
    bool flag;  //最好加上，有检测
    for(; i<size; i++) {
        if(! segement[i].startsWith("#") || segement[i].size() != 7)
            continue;
        colors[j] = segement[i].mid(1).toUInt(&flag, 16);  //以十六进制转化为整数
        if(flag)
            j++;
    }

    //没有这么多自动添加一样的
    for(i=j; i<6; i++)
        colors[i] = colors[j-1];
}

void toSetStyleSheet(quint8 flag)
{

    //0：day  other:backcolor
    QString styleFileName = "";
    if(flag) {
        switch(MenuConfig.dispMenu.colorSelect) {
            case 0:
            styleFileName = CURRENTPATH+ "/qss/color1.qss";
            break;
            case 1:
            styleFileName = CURRENTPATH + "/qss/color2.qss";
            break;
            case 2:
            styleFileName = CURRENTPATH + "/qss/color3.qss";
            break;

            default:
            break;
        }


    }else {
        switch(MenuConfig.dispMenu.dayNight) {
           case 0:
           styleFileName = CURRENTPATH + "/qss/day.qss";
           break;
           case 1:
           styleFileName = CURRENTPATH + "/qss/dusk.qss";
           break;
           case 2:
           styleFileName = CURRENTPATH+ "/qss/night.qss";
           break;

           default:
           break;
        }

    }
        qDebug() << styleFileName;

    //设置样式表
    QFile file(styleFileName);
    if(!file.open(QIODevice::Text | QIODevice::ReadOnly))
        qDebug()<<"can't open style configure file !";
    const QString styleSheet = QLatin1String(file.readAll());
    qApp->setStyleSheet(styleSheet);
    file.close();
}

void saveComputerInfo(quint8 flag)
{
    //1开启0关闭
    if(flag) {
        if(!lpComputerInfo)
            lpComputerInfo = new ComputerInfo;

        lpComputerInfo->GetCpuProducerInfo(producer);
        lpComputerInfo->GetCpuSerialNumber(serialid);
        //将CPU信息存储起来
        memcpy(MenuConfig.installMenu.producer, producer, 15);
        memcpy(MenuConfig.installMenu.serialId, serialid, 20);


       /* QMessageBox box(lpMainWindow);
        QTimer::singleShot(500000, &box, SLOT(close()));
        box.setWindowTitle(QObject::tr("提示"));
        box.setIcon(QMessageBox::Information);
        box.setText(QObject::tr("电脑与雷达显示软件匹配成功!\nCPU信息：%1%2").arg(producer).arg(serialid));
        box.show();*/
      //  QMessageBox::information(lpMainWindow, QString(QObject::tr("提示")), QString(QObject::tr("电脑与雷达显示软件匹配成功!\nCPU信息：%1%2")).arg(producer).arg(serialid), QMessageBox::NoButton);


    }else{
        memset(MenuConfig.installMenu.producer, 0, 15);
        memset(MenuConfig.installMenu.serialId, 0, 20);
      //  QMessageBox::information(lpMainWindow, QString(QObject::tr("提示")), QString(QObject::tr("电脑与雷达显示软件取消匹配成功！")), QMessageBox::NoButton);
    }

}

void initTargetObject()
{
    lpPlot = new Plot;
    lpPlot->setPlotSize(6);
    lpPlot->setFrameTimeWidth(1);
    lpPlot->setDefaultKeepTime(5);

    lpPlot->registerType(PLOT_TYPE_PRIMARY, "primaryplot", QRGB(0x00, 0xff, 0x00));
    lpPlot->registerType(PLOT_TYPE_FILTER, "filterplot", QRGB(0xff, 0x00, 0x00));
    lpPlot->setTypeFlag(PLOT_TYPE_PRIMARY, 0x1f);
    lpPlot->setTypeFlag(PLOT_TYPE_FILTER, 0x1f);

}





















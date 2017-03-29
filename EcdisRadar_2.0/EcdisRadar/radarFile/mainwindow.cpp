/****************************************************************************
file name: mainwindow.cpp
author: wang ming xiao
date: 2015/07/05
comments:
***************************************************************************/
#include "mainwindow.h"
#include "dialog.h"
#include "glwidget.h"
#include "boatinfo.h"
#include "interact.h"
#include "mouseoperation.h"
#include "boatalarm.h"
#include "aismanage.h"
#include "TargetManage/plot.h"
#include "recordreplay/recordreplay.h"


#include <QLabel>
#include <QDebug>
#include <QAction>
#include <QDialog>
#include <QStatusBar>
#include <QGLWidget>
#include <QToolButton>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QEvent>




#define UpdateMenuDlg(dlg) {	if(dlg && dlg->isVisible()) dlg->updateDlgDisplay();	}  //更新对话框

uchar  gXinnengMonitor = 0; //性能监视
quint8  closeSpeaking = 0;  //关闭报警声
quint8 clearScreen = 0;     //清空屏幕
DockWidget *pOperateDock;

quint8 gAtaAisSwitch = 0;	// 0:ais, 1:ata
quint8 FirstWaitTime = 90;   //开机时间

extern COLORCONFIG *lpColorConfig;
extern MENUCONFIG MenuConfig;
extern SYSTEMINFO SystemInfo;
extern MainWindow *lpMainWindow;
extern GLWidget *pView;
extern Interact *lpInteract;
extern MouseOperation *lpMouseOpetarion;
extern Dialog*   m_lpDialog;
extern boatalarm* m_boatAlarm;
extern AisManage* lpAisManage;
extern Alarm* lpAlarm;
extern SYSTEM_PARA g_systemPara;
extern Plot*  lpPlot;
extern Recordreplay* lpRecplay;

extern quint8 updateOwnShipDynamic;
extern quint8 antennaWarning;
extern  bool drawImageFlag;
extern bool isReplaying;

extern void toSetStyleSheet(quint8 flag);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    lpMainWindow = this;
   // setMouseTracking(true);
   // setFixedSize(FULLSCREEN_WIDTH, MAINVIEW_RADIUS);

    //设定默认颜色
    MenuConfig.dispMenu.colorSelect = 0;
    m_colorMode = -1;
  //  setColorMode(MenuConfig.dispMenu.colorSelect); 放在初始化后面
    setAutoFillBackground(true);
   /* QPalette p = this->palette();
    p.setColor(QPalette::Window, QColor(lpColorConfig->color_back[MenuConfig.dispMenu.colorSelect]));
    this->setPalette(p);*/

    //初始化各设备的链路保持状态
    memset(linkKeepTime, 0, sizeof(linkKeepTime));
    for(int i=0;i < MAX_OUTER_DEVICE; i++) {
        m_linkStatus[i].changed = 0;
        m_linkStatus[i].status = BROKEN;
    }

    m_lpMainMenu = NULL;
    m_lpDispMenu = NULL;
    m_lpInstMenu = NULL;
    m_lpOtherMenu = NULL;
    m_lpSerialConfigMenu = NULL;//定义的指针一定要初始化  否则指针中存的数值是随机不确定的   自己验证是会出错的
    m_lpAlarmDispMenu = NULL;
    m_lpRecordDlg = NULL;

/*
    QGLWidget *pWidget = new QGLWidget( QGLFormat(QGL::SampleBuffers), this);
    pWidget->makeCurrent();
    //新建场景
    pScene = new OpenGLSence(this);
    //新建视图
    pView = new OpenGLView(this);
    pView->setViewport(pWidget);
    pView->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    pView->setScene(pScene);
   // pView->setFixedSize(MAINVIEW_RADIUS,MAINVIEW_RADIUS);  */
    pView = new GLWidget(this);
    pView->setFixedSize(MAINVIEW_RADIUS, MAINVIEW_RADIUS);

    pView->enableMouseTracking(true);
    pView->setMouseTracking(true); //跟踪鼠标位置
    setCentralWidget(pView);

    ////////////////////////创建右侧窗口对象//////////////////////////////
    m_lpDialog = new Dialog(this);
    //m_lpDialog->setFixedSize(FULLSCREEN_WIDTH-MAINVIEW_RADIUS-8, MAINVIEW_RADIUS);
    m_lpDialog->move(MAINVIEW_RADIUS, 0);
    m_lpDialog->setDispinfo(0);  //显示AIS

    //DockWidget对象，将dialog添加到里面
    DockOptions opts = 0;
    opts |= AnimatedDocks;
    opts |= AllowNestedDocks;
    opts |= AllowTabbedDocks;
    setDockOptions(opts);
    pOperateDock = new DockWidget(this, 0);
    pOperateDock->setAllowedAreas(Qt::RightDockWidgetArea);
    pOperateDock->setWidget(m_lpDialog);
   // pOperateDock->setFeatures(DockWidget::DockWidgetMovable | DockWidget::DockWidgetFloatable);
    pOperateDock->setFeatures(DockWidget::NoDockWidgetFeatures);
    pOperateDock->setWindowFlags(Qt::FramelessWindowHint);
    //隐藏DockWidget的标题栏
    QWidget *qw = new QWidget(this);
    pOperateDock->setTitleBarWidget(qw);  //界面一下子变好了哈哈，网络太聪明了
    pOperateDock->setFixedSize(FULLSCREEN_WIDTH-MAINVIEW_RADIUS, MAINVIEW_RADIUS);
    addDockWidget(Qt::RightDockWidgetArea, pOperateDock);


    //////////////////////创建四个角落部件//////////////////////////
    createCtrlPanel();
    m_gboxPos[0] = QPoint(4,4);
    m_gboxPos[1] = QPoint(4, pView->height()-m_gbox[1]->height()-4);
    m_gboxPos[2] = QPoint(pView->width()-m_gbox[2]->width()-4, 4);
    m_gboxPos[3] = QPoint(pView->width()-m_gbox[3]->width()-4, pView->height()-m_gbox[3]->height()-4);
    m_gboxPos[4] = QPoint(pView->width()-m_gbox[4]->width()-4, m_gboxPos[3].y()-m_gbox[4]->height()-2);
  //  qDebug()<<"MAIN width is"<<width();
  //  qDebug()<<"MAIN height is"<<height();
    if(m_gbox[0])
        m_gbox[0]->move(m_gboxPos[2]);
    if(m_gbox[1])
        m_gbox[1]->move( m_gboxPos[1]);
    if(m_gbox[2])
        m_gbox[2]->move(m_gboxPos[0]);
    if(m_gbox[3])
        m_gbox[3]->move(m_gboxPos[3]);
    if(m_gbox[4])
        m_gbox[4]->move(m_gboxPos[4]);

    connect(m_menu_btn, SIGNAL(clicked(bool)), this, SLOT(on_m_cancel_btn_clicked(bool))); //其实没必要
    connect(m_tmrm_btn, SIGNAL(clicked(bool)), this, SLOT(on_m_tmrm_btn_clicked(bool)));
    connect(m_up_btn, SIGNAL(clicked(bool)), this, SLOT(on_m_up_btn_clicked(bool)));
    connect(m_trial_btn, SIGNAL(clicked(bool)), this, SLOT(on_m_trial_btn_clicked(bool)));

#ifdef TIMEEVENTFLASH
    m_timerid1 = startTimer(50);  //定时器
#else
    m_timerid1 = startTimer(100);  //定时器
#endif

    updateDispCtrl();
    updateVrmeblDisplay();

    m_switchVrmebl = 0;
    m_vrmeblChanged = 0;
    m_time100ms = 0;
#ifdef TIMEEVENTFLASH
    m_time50ms = 0;
#endif

    m_gaint = 0;
    m_restraint = 0;
    m_cluttert = 0;
    m_tunet = 0;

    m_brightIndex = 0;
    m_lightChanged = 0;
    m_screenLightChanged = 0;

    //最后刷新显示右下角
    updateCtrlPanel(0xffffffff);  //数值最大，能全部刷新
}

MainWindow::~MainWindow()
{
    if(m_timerid1)
         killTimer(m_timerid1);
}


// 设置当前系统时间
void MainWindow::setCrntTime(quint32 tm, bool udpate)
{
    SystemInfo.crnt_time = tm;
    if(udpate)
         m_lpDialog->updateDateTimeDisplay();
}

//设置颜色模式
void MainWindow::setColorMode(int flag)
{
    extern COLORCONFIG *lpColorConfig;
    if(lpColorConfig && m_colorMode != flag) {
        m_colorMode = flag;


        //qDebug()<<"change the mainwindow color";
        //更改窗口背景
      /*  QPalette p = this->palette();
        p.setColor(QPalette::Window, QColor(lpColorConfig->color_back[flag]));
        this->setPalette(p);*/
        toSetStyleSheet(1);

        //更改回波背景
        if(pView) {
            pView->setForeColor(lpColorConfig->color_fore[flag]);  //更改回波颜色,刷新在背景更改后
            pView->setBackColor(lpColorConfig->color_back[flag]);  //回波显示矩形的背景色,即整个背景色
            pView->setLineColor(lpColorConfig->color_headl[flag], lpColorConfig->color_eblvrm[flag]);  //活标线的颜色

/*
            if((m_colorMode == 0) || (m_colorMode == 1))
                //单色模式
                pView->setColorMode(0);
            else if(m_colorMode == 2)
                //多色模式
                pView->setColorMode(1);  */
            pView->setColorMode(m_colorMode);
        }

        if(lpAisManage)
            lpAisManage->setColor(lpColorConfig->color_ais[flag]);

    }
}

//设置情景模式,改变方式如何？
void MainWindow::setDisplayMode(int val)
{
    //选择不同的配置文件
    extern COLORCONFIG *lpColorConfig;
    quint8 flag = val + 3;  //在颜色配置表的后三项
    if(lpColorConfig) {

        toSetStyleSheet(0);

        //更改回波背景
        if(pView) {
            pView->setForeColor(lpColorConfig->color_fore[flag]);  //更改回波颜色,刷新在背景更改后
            pView->setBackColor(lpColorConfig->color_back[flag]);  //回波显示矩形的背景色,即整个背景色
            pView->setLineColor(lpColorConfig->color_headl[flag], lpColorConfig->color_eblvrm[flag]);  //活标线的颜色

/*
            if((m_colorMode == 0) || (m_colorMode == 1))
                //单色模式
                pView->setColorMode(0);
            else if(m_colorMode == 2)
                //多色模式
                pView->setColorMode(1); */
            pView->setColorMode(m_colorMode);
        }

        if(lpAisManage)
            lpAisManage->setColor(lpColorConfig->color_ais[flag]);

    }

}
void MainWindow::setBrightSelect()
{
    m_brightIndex++;
    if(m_brightIndex > 3)  m_brightIndex = 0;
}
void MainWindow::setBrightValue(quint8 flag)
{
    int value = 0;
    //0:down 1:up
    if(flag) {
        switch(m_brightIndex) {
        case 0:
            value = MenuConfig.dispMenu.screenBright;
            if(value >= 50 )
                setBrightMode(m_brightIndex, value+5);
            else
                setBrightMode(m_brightIndex, ++value);
        break;
        case 1:
            value = MenuConfig.dispMenu.kbdBright;
            setBrightMode(m_brightIndex, ++value);
        break;
        case 2:
            value = MenuConfig.dispMenu.fixlineBright;
            setBrightMode(m_brightIndex, ++value);
        break;
        case 3:
            value = MenuConfig.dispMenu.varlineBright;
            setBrightMode(m_brightIndex, ++value);
        break;
        default:
        break;
        }


      }else {
          switch(m_brightIndex) {
          case 0:
              value = MenuConfig.dispMenu.screenBright;
              if(value >= 50 )
                  setBrightMode(m_brightIndex, value-5);
              else
                  setBrightMode(m_brightIndex, --value);
          break;
          case 1:
              value = MenuConfig.dispMenu.kbdBright;
              setBrightMode(m_brightIndex, --value);
          break;
          case 2:
              value = MenuConfig.dispMenu.fixlineBright;
              setBrightMode(m_brightIndex, --value);
          break;
          case 3:
              value = MenuConfig.dispMenu.varlineBright;
              setBrightMode(m_brightIndex, --value);
          break;
          default:
          break;
          }

      }
      //更新显示菜单


}
void MainWindow::setBrightMode(int index, int val)
{
    // index 0:屏幕亮度, 1:键盘亮度, 2:固定标线亮度, 3:活动标线亮度
    if(index < 0 || index >= 4)
            return;

    // 通过index调用相应的set函数
typedef void (MainWindow::* SetBrightFunc)(int val);
#define SETBRIGHTFUNCPTR(func) SetBrightFunc(&MainWindow::func)  //就是将该函数指针赋值

    SetBrightFunc lpFunc[] = {SETBRIGHTFUNCPTR(setScreenBright), SETBRIGHTFUNCPTR(setKbdBright), \
            SETBRIGHTFUNCPTR(setFixLineBright), SETBRIGHTFUNCPTR(setVarLineBright)};

    (this->* lpFunc[index])(val);
#undef SETBRIGHTFUNCPTR

}
// 设置屏幕亮度
void MainWindow::setScreenBright(int val)
{
    if(val < 1) val = 1;
    if(val > 100) val = 100;
    //qDebug() << __FUNCTION__ << val;
    if(MenuConfig.dispMenu.screenBright != val) {
        MenuConfig.dispMenu.screenBright = val;
       //定时器中刷新
        m_screenLightChanged = 1;
        m_lightChanged = 1;
    }

}
// 设置键盘亮度
void MainWindow::setKbdBright(int val)
{
    if(val < 1) val = 1;
    if(val > 100) val = 100;
    //qDebug() << __FUNCTION__ << val;
    if(MenuConfig.dispMenu.kbdBright != val) {
        MenuConfig.dispMenu.kbdBright = val;
        QByteArray light;
        QDataStream in(&light, QIODevice::WriteOnly);
        in<<(quint8)val;
        lpInteract->SeirialWrite(light);  //串口设备号：5
        m_lightChanged = 1;
    }

}
// 设置固定标线亮度
void MainWindow::setFixLineBright(int val)
{
    if(val < 1) val = 1;
    if(val > 100) val = 100;
    //qDebug() << __FUNCTION__ << val;

    if((MenuConfig.dispMenu.fixlineBright != val)) {
        MenuConfig.dispMenu.fixlineBright = val;
        m_lightChanged = 1;
        drawImageFlag = true;
        if((! MenuConfig.otherMenu.transmite))
            pView->update();
    }
}
// 设置活动标线亮度
void MainWindow::setVarLineBright(int val)
{
    if(val < 1) val = 1;
    if(val > 100) val = 100;
   // qDebug() << __FUNCTION__ << val;

    if((MenuConfig.dispMenu.varlineBright != val)) {
        MenuConfig.dispMenu.varlineBright = val;
        m_lightChanged = 1;
        if((! MenuConfig.otherMenu.transmite))
            pView->update();
    }
}


void MainWindow::updateScreen(quint8 val)
{
    //屏幕亮度根据对比度调整和屏幕亮度两者结合来实现理想的效果,对亮度的数值采用合适的区间计算
    //前50加1，后50加5

    //发送指令给FPGA调节屏幕亮度
    QByteArray senddata;
    QDataStream in(&senddata, QIODevice::WriteOnly);

     if(lpInteract) {
         in<<(quint8)170<<(quint8)0<<(quint8)15<<(quint8)0<<(quint8)val;  //AA000f 0001 --- AA000f 0064  1-100
         if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
         // qDebug()<< "FPGA write screenLight success!"<< flag;
     }

    //调整对比度，对比度不能降低到太低，否则会失真
     float num = 30.0 + (float)(val*0.7);
     num = (float)(num / 100.0);
     QString command = QString("xgamma -gamma %1").arg(num);
     char com[18] = {0};
     const quint8 size = command.size();
     if(size <= 18) {
         for(int i=0; i< size; i++) {
             com[i] = command.at(i).toAscii();
         }
     }
     system(com);
}


//距标圈显示
void MainWindow::setRngringMode()
{
    quint8 val = (MenuConfig.dispMenu.showRngRing ? 0 : 1);
    MenuConfig.dispMenu.showRngRing = val;
    pView->setRngringShow(val);
    UpdateMenuDlg(m_lpDispMenu);  //更新显示对话框
}
//船艏线显示
void MainWindow::setHeadlineMode()
{
    quint8 val = (MenuConfig.dispMenu.showHeadLine ? 0 : 1);
    MenuConfig.dispMenu.showHeadLine = val;
    pView->setHeadlineShow(val);
    UpdateMenuDlg(m_lpDispMenu);
}
void MainWindow::updateOtherDlg()
{
    UpdateMenuDlg(m_lpOtherMenu);
}
//VRM/EBL显示
void MainWindow::setVrmeblMode()
{

    if(!m_switchVrmebl) {  //  0:vrm1
        if((MenuConfig.otherMenu.vrmeblEnable & 0x03) == 0x03)  //有显示,关闭
        {
            MenuConfig.otherMenu.vrmeblEnable &= ~(0x03);
        }else {
            MenuConfig.otherMenu.vrmeblEnable |= 0x03;
        }

    }else {  //1:vrm2
        if(((MenuConfig.otherMenu.vrmeblEnable>>2) & 0x03) == 0x03)  //有显示，关闭
        {
            MenuConfig.otherMenu.vrmeblEnable &= ~(0x0c);

        }else {
            MenuConfig.otherMenu.vrmeblEnable |= 0x0c;
        }
    }
    SystemInfo.VrmEblCtrl.vrmebl[0].show = (MenuConfig.otherMenu.vrmeblEnable & 0x03);  //0:vrm1   1:ebl1
    SystemInfo.VrmEblCtrl.vrmebl[1].show = ((MenuConfig.otherMenu.vrmeblEnable>>2) & 0x03);

    m_vrmeblChanged = 1;

   // qDebug()<< "vrmebl enabled"<<MenuConfig.otherMenu.vrmeblEnable;



}
void MainWindow::switchVrmeblMode()
{
    //  0:vrm1  1:vrm2
    m_switchVrmebl = (m_switchVrmebl ? 0 : 1);
   // qDebug()<< "switch vrmebl"<<m_switchVrmebl;

}
void MainWindow::change_EBL(int val)
{
     VRMEBL& vrmebl = SystemInfo.VrmEblCtrl.vrmebl[m_switchVrmebl];
     float e = vrmebl.ebl;
    //0:down 1:up
    if(val) {
        e++;
    } else {
        e--;
    }
    if(e < 0)  e+=360;
    if(e > 359) e=0;
    if(e <= 360) {
        vrmebl.ebl = e;
        m_switchVrmebl ? MenuConfig.otherMenu.vrmebl[3] = e : MenuConfig.otherMenu.vrmebl[1] = e;
        m_vrmeblChanged = 1;
    }

}
void MainWindow::change_Vrm(int val)
{
    VRMEBL& vrmebl = SystemInfo.VrmEblCtrl.vrmebl[m_switchVrmebl];
    float e = vrmebl.vrm;
    float inc = pView->range() /80.0;
   //0:down 1:up
   if(val) {
       if((e * pView->ration()) < (pView->m_offset_maxradius - 5))
           e += inc;
   } else {
       e -= inc;
   }
   if((e > 0)) {
      vrmebl.vrm = e;
      m_switchVrmebl ? MenuConfig.otherMenu.vrmebl[2] = e : MenuConfig.otherMenu.vrmebl[0] = e;
      m_vrmeblChanged = 1;
   }
}
void MainWindow::setTuneMode()
{
    if(MenuConfig.installMenu.tuneMan == 0) {
        MenuConfig.installMenu.tuneMan = 1;
        pView->change_tune(MenuConfig.installMenu.tuneMan, MenuConfig.installMenu.tuneValue);
    } else {
        MenuConfig.installMenu.tuneMan = 0;
        pView->change_tune(MenuConfig.installMenu.tuneMan, MenuConfig.installMenu.tuneValue);
    }
    //更新安装显示菜单
    UpdateMenuDlg(m_lpInstMenu);
}
void MainWindow::setGuildMode()
{
    quint8 mode = SystemInfo.ViewDisplay.upmode;
    if(mode < 2) mode++;
    else mode = 0;
    MenuConfig.dispMenu.upmode = mode;
    pView->setUpMode(mode);
    //lpMainView->updateDispCtrl();
}
//偏心显示处理
void MainWindow::setOffsetMode(bool flag)
{
    if(flag){
        const quint8 mode = (MenuConfig.dispMenu.offset ? 0 : 1);
        MenuConfig.dispMenu.offset = mode;
        pView->setOffset(mode);

        //更新角落显示
        updateCtrlPanel(VIEW_UPDATE_OFST_BIT);
        //更新对话框
        UpdateMenuDlg(m_lpDispMenu);
    }else {
        MenuConfig.dispMenu.offset = 0;
        SystemInfo.ViewDisplay.offset = 0;

        //更新角落显示
        updateCtrlPanel(VIEW_UPDATE_OFST_BIT);
        //更新对话框
        UpdateMenuDlg(m_lpDispMenu);
    }

}

//每100ms时间处理函数
void MainWindow::timerEvent(QTimerEvent *event)
{
#ifdef TIMEEVENTFLASH
    m_time50ms++;
    if(m_time50ms > 99)
        m_time50ms = 0;

    pView->receivedpaint();  //刷新屏幕

    if(m_time50ms % 2 == 0) {   //100ms操作
#endif

    m_time100ms ++;
    if(m_time100ms > 99)
        m_time100ms = 0;


    //清空屏幕
    if(clearScreen)
        pView->update();

    //定时更新VRM和EBL显示
    if(m_vrmeblChanged) {
        m_vrmeblChanged = 0;
        updateVrmeblDisplay();
        //更新菜单显示窗口
        UpdateMenuDlg(m_lpOtherMenu);
        if(! MenuConfig.otherMenu.transmite)
            pView->update();
    }

    //定时刷新增益等显示
    if(m_time100ms % 2 == 0) {
        if(m_gaint != pView->gain() || m_cluttert != pView->clutter() || m_restraint != pView->restrain()) {
            m_gaint = pView->gain();
            m_cluttert = pView->clutter();
            m_restraint = pView->restrain();
            updateDispCtrl();
        }
        //刷新亮度条显示
        if(m_lightChanged) {
            m_lightChanged = 0;
            UpdateMenuDlg(m_lpDispMenu);

        }
        //设置屏幕亮度和对比度
        if(m_screenLightChanged) {
            m_screenLightChanged = 0;
            updateScreen(MenuConfig.dispMenu.screenBright);
        }
        //刷新调谐显示
        if(m_tunet != pView->tune()) {
            m_tunet = pView->tune();
            updateTuneDisp();
        }
        //更新记录重演时间
        if(lpRecplay)
            lpRecplay->updateComputerTime(2);

    }

    //定时更新鼠标位置
    if(m_time100ms % 3 == 0) {
        updateMousePosDisplay();

        //刷新重演进度条和状态
        if(m_lpRecordDlg && ( isReplaying || m_lpRecordDlg->isVisible()))
            m_lpRecordDlg->updateStatus();
    }

    //每秒钟更新时间显示/////////////////////////////////
    if(m_time100ms % 10 == 0) {
        if(FirstWaitTime > 0)
        {
            FirstWaitTime --;
            pView->update();
            lpAisManage-> simuAisMove(50);
        }


       //使用重演时间作为当前时间
       if(lpRecplay && lpRecplay->isReplaying())
           setCrntTime(lpRecplay->crntReplayTime());
      else  if(m_linkStatus[1].status != NORMAL)
          setCrntTime(time(0));


       MenuConfig.installMenu.timeUsed ++;
       if(MenuConfig.otherMenu.transmite)
          MenuConfig.installMenu.timeTran++;

       updateLinkInfo();

       //更新点迹保留时间
       if(lpPlot)
           lpPlot->updatePlotKeepTime(time(0));

       //更新船舶动态信息
       if(updateOwnShipDynamic) {
           m_boatAlarm->updateDynamicInfo();
       }

       //如果警戒区域打开，刷新数据
       if(MenuConfig.otherMenu.guardZoneEnable  && MenuConfig.otherMenu.guardSelect && m_lpOtherMenu && m_lpOtherMenu->isVisible()) {
           refreshGuardZone();
       }

       // 更新AIS船舶对象并报警
       if(lpAisManage) {
           lpAisManage->updateAisShips(SystemInfo.crnt_time);//time(0);
           if(m_linkStatus[GPS_DEVICE].status == NORMAL)  //GPS数据有效才报警
               lpAisManage->cpaAlarm();
       }

       //刷新天线报警信息
       if(antennaWarning) {
           if(pView)
               pView->antennaWarning(antennaWarning);
         //  qDebug()<< "warning"<< antennaWarning;
           antennaWarning = 0;
       }

       //刷新报警标志,11报警
       extern quint8 guardZone_alarm;
       if(guardZone_alarm == 11) {
           //guardZone_alarm = 0;
           //测试报警
           QString msg = QString("目标进入警戒区域报警！");
           lpAlarm->startAlarm((ALARMLEVEL)g_systemPara.alarmPara.cpaAlarmLevel, msg);
       }
       if(closeSpeaking) {
           closeSpeaking = 0;
           extern quint8 flag_isSpeaking;
           if(flag_isSpeaking) {
               lpAlarm->stopAlarm();
           }
       }
    }

    //更新尾迹保留时间
    if(m_time100ms % 50 == 0) {
        pView->updateDelayTime();
    }

#ifdef TIMEEVENTFLASH
    }
#endif
}

void MainWindow::clearAlarmInfo()
{
    m_lpDialog->alarmProcess();
}

void MainWindow::refreshGuardZone()
{
    const QPointF p1 = pView->guardzone()->first_r_point();
    const QPointF p2 = pView->guardzone()->second_r_point();


    if(m_lpOtherMenu)
        m_lpOtherMenu->setGuardZoneText(p1.x(), RADIANTODEGREE(p1.y()), p2.x(), RADIANTODEGREE(p2.y()));
    //UpdateMenuDlg(m_lpOtherMenu);
}

//刷新设备链表
void MainWindow::refreshDeviceLink(int device)
{
    if(device >= MAX_OUTER_DEVICE)
        return;

    linkKeepTime[device] = 10000;  //10秒
    if(m_linkStatus[device].status != NORMAL)
    {
        m_linkStatus[device].changed = 1;
        m_linkStatus[device].status = NORMAL;
        m_lpDialog->setLinkStatusDisp(device, NORMAL);
    }

}
void MainWindow::updateLinkInfo()
{
    quint8 changed = 0;
    // 判断各设备链路状态
    for(int i=0; i<MAX_OUTER_DEVICE; i++)
    {
        if(linkKeepTime[i]>0)
        {
            linkKeepTime[i] -= 1000;
            if(linkKeepTime[i]<=0 && m_linkStatus[i].status != BROKEN)
            {
                m_linkStatus[i].changed = 1;
                m_linkStatus[i].status = BROKEN;
                m_lpDialog->setLinkStatusDisp(i, BROKEN);
            }
        }
        if(m_linkStatus[i].changed)
        {
             changed |= (1<<i);   //将变化标志保存，修改显示
             m_linkStatus[i].changed = 0;
        }
     }

    if(changed);
        m_lpDialog->updateBoatInfoDisplay();

    if(changed & 0x06);
        m_lpDialog->clearTargetBoxDisplay();
}



void MainWindow::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    switch (event->type()) {
    case QEvent::LanguageChange:
        retranslate(this);
        //qDebug()<<"MainWindow page";
        break;
    default:
        break;
    }
}

//翻译使用函数
void MainWindow::retranslate(QWidget *mainwindow)
{
  /*  m_menu_btn->setText(QApplication::translate("mainwindow", "菜单", 0, QApplication::UnicodeUTF8));
    m_up_btn->setText(QApplication::translate("mainwindow", "船艏向上", 0, QApplication::UnicodeUTF8));
    m_tmrm_btn->setText(QApplication::translate("mainwindow", "TM/RM", 0, QApplication::UnicodeUTF8));
    m_trial_btn->setText(QApplication::translate("mainwindow", "尾迹", 0, QApplication::UnicodeUTF8));*/
    m_menu_btn->setText(tr("菜单"));
    m_up_btn->setText(tr("船艏向上"));
    m_tmrm_btn->setText(tr("TM/RM"));
    m_trial_btn->setText(tr("尾迹"));
    label_gain->setText(tr("增益"));
    label_restrain->setText(tr("雨雪"));
    label_clutter->setText(tr("海浪"));
    label_tune->setText(tr("调谐"));
    m_x_btn->setText(tr("X波段"));

}

//按键处理的一些函数
void MainWindow::on_m_menu_btn_clicked(bool)
{
    QPoint pt = QCursor::pos();
    if(btn_confirm(pt))  //先检测左上角的按键
        return;

    //显示主菜单
    if(isMenuShow()) {
       // QPoint pt = lpMouseOpetarion->mouseScreenPoint();
        MenuDlg* crnt = (MenuDlg*)m_menutree.top();
        if(crnt && crnt->kbd_confirm(pt, 1))  { //查询是否包含鼠标坐标点
               return;
           // QPoint pt_wiget = crnt->mapFromGlobal(pt);
           //  QMouseEvent event(QEvent::MouseButtonPress, pt_wiget, pt, Qt::LeftButton,Qt::LeftButton, Qt::NoModifier );
          //   emit dialogMousePressed(&event);


        }else {
            qDebug()<< "error pos on btn";
            //没有选中按钮，画航迹点或者警戒区域
            //画航迹点操作
            QMouseEvent event(QEvent::MouseButtonPress, QCursor::pos(), QCursor::pos(), Qt::LeftButton,Qt::LeftButton, Qt::NoModifier );
            emit mousePressed(&event);
            return;

        }
    }else {
        //ATA和AIS的切换
        if(m_lpDialog && m_lpDialog->kbd_confirm(pt,1))
            return; 


        //选择AIS/ATA目标显示
        QMouseEvent event(QEvent::MouseButtonPress, QCursor::pos(), QCursor::pos(), Qt::LeftButton,Qt::LeftButton, Qt::NoModifier );
        emit mousePressed(&event);
        return;


    }
}

//显示菜单按钮操作
void MainWindow::on_m_cancel_btn_clicked(bool)
{
    //对slider进行数值减少操作
 /*   if(isMenuShow()) {
        QPoint pt = QCursor::pos();
        MenuDlg *crnt = (MenuDlg*)m_menutree.top();
        if(crnt && crnt->kbd_confirm(pt,0))  //相反操作
            return;
        else {
            //画航迹点操作
            QMouseEvent event(QEvent::MouseButtonPress, QCursor::pos(), QCursor::pos(), Qt::LeftButton,Qt::LeftButton, Qt::NoModifier );
            emit mousePressed(&event);
            return;
        }
    } */

    //主菜单切换
   if(isMenuShow()) {
       hideAllMenu();
       pOperateDock->show();
   }else {
       pOperateDock->hide();
       showMainMenu();
   }
}

void MainWindow::on_m_tmrm_btn_clicked(bool)
{
    //qDebug()<<"tmrm btn clicked";
    quint8 val = MenuConfig.dispMenu.motion;
    if(val)
        val = 0;
    else
        val = 1;

    MenuConfig.dispMenu.motion = val;
    UpdateMenuDlg(m_lpDispMenu);

    pView->setMotion(val);
}
void MainWindow::on_m_up_btn_clicked(bool)
{
  //  qDebug()<<"tmrm up clicked";
    quint8 val = MenuConfig.dispMenu.upmode;
    if(val < 2)
        val++;
    else
        val = 0;

    MenuConfig.dispMenu.upmode = val;
    UpdateMenuDlg(m_lpDispMenu);
    pView->setUpMode(val);
}
void MainWindow::on_m_trial_btn_clicked(bool)
{
   // qDebug()<<"tmrm trial clicked";
    quint8 val = MenuConfig.dispMenu.echoTrail;
    if(val < 5)
        val++;
    else
        val = 0;
    MenuConfig.dispMenu.echoTrail = val;
    UpdateMenuDlg(m_lpDispMenu);
    pView->setTrailTime(val);
}


bool MainWindow::btn_confirm(QPoint pt)
{
    //确认处理
    QList<QAbstractButton *> button;
    button << m_tmrm_btn << m_up_btn << m_trial_btn << m_menu_btn;

    foreach(QAbstractButton *b ,button) {
        QRect rect = QRect(mapToGlobal(b->pos()),b->rect().size());
        if(rect.contains(pt)){
            b->click();
            return true;
        }
    }


    return false;
}


//鼠标移动处理
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    //只记录位置，不计算其他数据
   // lpMouseOpetarion->mouseMoved(event->globalPos(), false);
  //  qDebug()<<"mouse"<<event->globalPos().x()<<event->globalPos().y();
}

QDialog* MainWindow::showChangedDispMenu()
{
    //设置背景颜色
    QPalette p = m_lpDispMenu->palette();
    p.setColor(QPalette::Window, QColor(lpColorConfig->color_back[MenuConfig.dispMenu.colorSelect]));
    m_lpDispMenu->setPalette(p);


    m_lpDispMenu->move(MAINVIEW_RADIUS, 0);
    m_lpDispMenu->show();
    m_lpDispMenu->activateWindow();

    return m_lpDispMenu;
}
void MainWindow::displayMenu(QDialog *menu, bool hidelast, bool pushlast)
{
    QDialog* last = NULL;
    if(hidelast && !m_menutree.isEmpty())
    {
        last = m_menutree.top();
    }

    //将显示的窗口隐藏，将新窗口压入堆栈并显示
    if(pushlast)
        m_menutree.push(menu);
    //设置背景颜色
    QPalette p = menu->palette();
    p.setColor(QPalette::Window, QColor(lpColorConfig->color_back[MenuConfig.dispMenu.colorSelect]));
    menu->setPalette(p);

    //QPoint pt = pos();
    menu->move(MAINVIEW_RADIUS, 0);
    menu->show();
   // menu->exec();
    menu->activateWindow();

    if(last)	last->hide();
}
//设置输入菜单在堆栈中的位置,属于局部变量，使用完后需要释放
void MainWindow::setInputStack(QDialog *menu, quint8 flag)
{
    //falg:1  添加   0：释放
    if(flag) {
        m_menutree.push(menu);
    }else {      
        QDialog* input = m_menutree.pop();
        input->hide();
    }

}
// 返回上层菜单窗口
void MainWindow::gobackMenu()
{
    lpMouseOpetarion->setMouseProcessID(NULL_PROCESS);

    if(m_menutree.isEmpty())
        return;

    // 隐藏当前菜单窗口，将其从堆栈取出
    QDialog* crnt = m_menutree.pop();

    // 显示上一菜单窗口
    if(!m_menutree.isEmpty())
    {
        QDialog* last = m_menutree.top();

        //设置背景颜色
        QPalette p = last->palette();
        p.setColor(QPalette::Window, QColor(lpColorConfig->color_back[MenuConfig.dispMenu.colorSelect]));
        last->setPalette(p);

        last->show();
        last->activateWindow();
    }else {
        pOperateDock->show();
    }

     if(crnt) crnt->hide();


}
// 隐藏所有菜单窗口
void MainWindow::hideAllMenu()
{
    if(m_menutree.isEmpty())
        return;

    // 隐藏当前菜单窗口
    QDialog* crnt = m_menutree.top();
    crnt->hide();

    // 清空菜单树
    m_menutree.clear();
}

/* QPalette p = object->palette();
 p.setColor(QPalette::Window, QColor(lpColorConfig->color_back[MenuConfig.dispMenu.colorSelect]));
 object->setPalette(p);  */
#define DisplayMenuDlg(classtype, object, flag) \
{   \
if(!object){    \
    object = new classtype(lpMainWindow);\
    object->setFixedSize(FULLSCREEN_WIDTH-MAINVIEW_RADIUS + 7, MAINVIEW_RADIUS);	\
} \
displayMenu(object, flag);\
object->createOperation(); \
return object;   \
}
// 显示主菜单窗口
QDialog* MainWindow::showMainMenu()
{
    DisplayMenuDlg(MainMenu, m_lpMainMenu, true);
}

// 显示显示菜单窗口
QDialog* MainWindow::showDisplayMenu()
{
    DisplayMenuDlg(DisplayMenu, m_lpDispMenu, true);
}

// 显示安装菜单窗口
QDialog* MainWindow::showInstallMenu()
{
    DisplayMenuDlg(InstallMenu, m_lpInstMenu, true);
}

// 显示其它菜单窗口
QDialog* MainWindow::showOtherMenu()
{
    DisplayMenuDlg(OtherMenu, m_lpOtherMenu, true);
}

//显示串口配置菜单
QDialog* MainWindow::showSerialConfigMenu()
{
    DisplayMenuDlg(SerialConfigMenu, m_lpSerialConfigMenu, true);
}

// 显示输入菜单窗口
QDialog* MainWindow::showInputMenu()
{
    DisplayMenuDlg(InputMenu, m_lpInputMenu, true);
}
QDialog* MainWindow::showAlarmDisplayMenu()
{
    DisplayMenuDlg(AlarmDisplay, m_lpAlarmDispMenu, true);
}
void MainWindow::setAlarmDisplayText(const QString &text)
{
    if(! (m_lpAlarmDispMenu == NULL)) {
        m_lpAlarmDispMenu->setDisplayText(text);
    }
}


//更新窗口右边控制内容
void MainWindow::updateCtrlPanel(quint32 flag)
{
#define ValueCheck(var, value) (((value) & (var)) == var)

    if(ValueCheck(VIEW_UPDATE_TMRM_BIT,flag) && m_tmrm_btn)
    {
        const QString motion[] = {tr("真运动"),tr("相对运动"),tr("")};
        m_tmrm_btn->setText(motion[MenuConfig.dispMenu.motion]);
    }
    if(ValueCheck(VIEW_UPDATE_UP_BIT, flag) && m_up_btn)
    {
        const QString uptext[] = {tr("真北向上"),tr("船艏向上"),tr("航向向上"),tr("")};
        m_up_btn->setText(uptext[MenuConfig.dispMenu.upmode]);
    }
    if(ValueCheck(VIEW_UPDATE_TRAIL_BIT, flag) && m_trial_btn)
    {
        const QString trial[] = {tr("尾迹关闭"),tr("尾迹5秒"),tr("尾迹10秒"),tr("尾迹1分"),tr("尾迹2分"),tr("尾迹连续"),tr(""),tr("")};
        m_trial_btn->setText(trial[MenuConfig.dispMenu.echoTrail]);
    }
    if(ValueCheck(VIEW_UPDATE_JAM_BIT, flag) && m_irLabel)
    {
        const QString irText[] = {"IR-1", "IR-2", "IR-3", ""};
        m_irLabel->setText(irText[MenuConfig.otherMenu.samefreqJam]);
    }
    //回波扩展显示
    if(ValueCheck(VIEW_UPDATE_AUTO_BIT, flag) && m_autoLabel)
    {
        const QString autoText[] = {"", "ES-1", "ES-2", ""};
        m_autoLabel->setText(autoText[MenuConfig.otherMenu.echoExpand]);
    }
    if(ValueCheck(VIEW_UPDATE_OFST_BIT, flag) && m_ofstLabel)
    {
        const QString ofstText[] = {"", "OFST", ""};
        m_ofstLabel->setText(ofstText[SystemInfo.ViewDisplay.offset]);
    }
    if(ValueCheck(VIEW_UPDATE_AIS_BIT, flag) && m_ofstLabel)
    {
        const QString text[] = {"  ", " AIS ", ""};
        m_aisLabel->setText(text[MenuConfig.dispMenu.showAis]);
    }
    if(ValueCheck(VIEW_UPDATE_ATA_BIT, flag) && m_ofstLabel)
    {
        const QString text[] = {"  ", " ATA ", ""};
        m_ataLabel->setText(text[MenuConfig.dispMenu.showArpa]);
    }

    if(flag) update();
}
//切换语言
void MainWindow::toSetLanguage(quint8 flag)
{
    //0：中文  1：英文
    if(flag)
        translator.load("./RadarDisplay_en.qm");
    else
        translator.load("./RadarDisplay_zh_cn.qm");

    qApp->installTranslator(&translator);
}


//更新显示控制
void MainWindow::updateDispCtrl()
{
    m_range->setText(QString::number(SystemInfo.RangeScale.range()) + QString(" NM"));//量程显示
    m_range2->setText(QString::number(SystemInfo.RangeScale.scale()));  //量程刻度显示
   /* m_gain->setText(QString::number(pView->gain()));  //增益显示
    m_restrain->setText(QString::number(pView->restrain()));  //雨雪抑制显示
    m_clutter->setText(QString::number(pView->clutter()));  //杂波显示*/
    m_gain->setValue((pView->gain()));  //增益显示
    m_restrain->setValue((pView->restrain()));  //雨雪抑制显示
    m_clutter->setValue((pView->clutter()));  //杂波显示

    update();
}
void MainWindow::updateTuneDisp()
{
    m_tune->setValue(pView->tune());  //调谐显示
    update();
}
void MainWindow::updateVrmeblDisplay()
{
    if(m_vrm1) {
        if(SystemInfo.VrmEblCtrl.vrmebl[0].show & 0x01)
            m_vrm1->setText(QString::number(SystemInfo.VrmEblCtrl.vrmebl[0].vrm,'f',3)+QString(" Nm"));
        else
            m_vrm1->setText(tr("OFF"));
    }
    if(m_ebl1) {
        if(SystemInfo.VrmEblCtrl.vrmebl[0].show & 0x02)
            m_ebl1->setText(QString::number(SystemInfo.VrmEblCtrl.vrmebl[0].ebl,'f',1)+QString("°"));
        else
            m_ebl1->setText(tr("OFF"));
    }
    if(m_vrm2) {
        if(SystemInfo.VrmEblCtrl.vrmebl[1].show & 0x01)
            m_vrm2->setText(QString::number(SystemInfo.VrmEblCtrl.vrmebl[1].vrm,'f',3)+QString(" Nm"));
        else
            m_vrm2->setText(tr("OFF"));
    }
    if(m_ebl2) {
        if(SystemInfo.VrmEblCtrl.vrmebl[1].show & 0x02)
            m_ebl2->setText(QString::number(SystemInfo.VrmEblCtrl.vrmebl[1].ebl,'f',1)+QString("°"));
        else
            m_ebl2->setText(tr("OFF"));
    }
    update();
}
void MainWindow::updateMousePosDisplay()
{
    QPoint sc = lpMouseOpetarion->mouseScreenPoint();
    QPointF  rt = lpMouseOpetarion->mouseRThetaPoint();
    QPointF  ld = lpMouseOpetarion->mouseLatitudePoint();

    QString strlon("---°--.---'-"), strlat("--°--.---'-");
    //显示目标经纬度的时候需要罗经船艏向和GPS经纬度,计算需要以船艏向计算
   // if((NORMAL == lpMainWindow->linkStatus(GPS_DEVICE).status) && (NORMAL == lpMainWindow->linkStatus(COMP_DEVICE).status))
     {
        strlon = Longitude2String(ld.x()*180.0/M_PI, 1);
        strlat = Latitude2String(ld.y()*180.0/M_PI, 1);
     }
     QString strazi("------"), strrng("------");
     //船艏信号有才能显示
  //   if(NORMAL == lpMainWindow->linkStatus(COMP_DEVICE).status)
     {
         strazi = QString("%1").arg((double)rt.y()/M_PI*180.0, 4, 'f', 1) + QString("°");
         strrng = QString("%2").arg((double)rt.x()/(pView->ration()), 4, 'f', 3) + QString(" Nm");
     }


    QString         m_mousePosition;
    if(MenuConfig.installMenu.langSelect == 0) {  //中文
    m_mousePosition = QString("\
                      经度：%1<br> \
                      纬度：%2<br> \
                      方位：%3<br> \
                      距离：%4")  \
                      .arg(strlon)
                      .arg(strlat)
                      .arg(strazi)
                      .arg(strrng);
}else {   //英文
    m_mousePosition = QString("\
                      Long：%1<br> \
                      Lat：%2<br> \
                      Dir：%3<br> \
                      Dis：%4")  \
                      .arg(strlon)
                      .arg(strlat)
                      .arg(strazi)
                      .arg(strrng);

}


    if(m_mousePositionLabel) {
    QFont ft;
    ft.setPointSize(12);
    m_mousePositionLabel->setFont(ft);

    m_mousePositionLabel->setText(m_mousePosition);
    update(rect());
    }
}

////////////////创建控制面板，就是四个角落/////////////////////////
void MainWindow::createCtrlPanel()
{
    QBoxLayout *layout[5];


    //1.左上角，改为右上角了
    CreateHLayout_v2(layout[0], tr(""), m_range, m_range2); //一个标签和两个量程显示器
   /* CreateHLayout_v1(layout[2], tr("增益"), m_gain, label_gain);
    CreateHLayout_v1(layout[3], tr("雨雪"), m_restrain, label_restrain);
    CreateHLayout_v1(layout[4], tr("海浪"), m_clutter, label_clutter); */
    CreateHLayout_p1(layout[1], tr("调谐"), m_tune, label_tune);
    CreateHLayout_p1(layout[2], tr("增益"), m_gain, label_gain);
    CreateHLayout_p1(layout[3], tr("雨雪"), m_restrain, label_restrain);
    CreateHLayout_p1(layout[4], tr("海浪"), m_clutter, label_clutter);
    m_tune->setRange(0, 512);
    m_gain->setRange(0, 100);
    m_restrain->setRange(0,100);
    m_clutter->setRange(0,100);



    CreateGroupBox(m_gbox[0], layout, 5);  //放在一个QGroupBox中显示
    m_gbox[0]->setFixedSize(160, 120);

    m_range->setAlignment(Qt::AlignHCenter);
    m_range2->setAlignment(Qt::AlignHCenter);
    m_gain->setAlignment(Qt::AlignHCenter);
    m_restrain->setAlignment(Qt::AlignHCenter);
    m_clutter->setAlignment(Qt::AlignHCenter);
    //m_range2->setEnabled(false);

    //2.左下角
    CreateHLayout_v1(layout[0], tr("VRM1"), m_vrm1, label_vrm1);
    CreateHLayout_v1(layout[1], tr("EBL1"), m_ebl1, label_ebl1);
    CreateHLayout_v1(layout[2], tr("VRM2"), m_vrm2, label_vrm2);
    CreateHLayout_v1(layout[3], tr("EBL2"), m_ebl2, label_ebl2);

    CreateGroupBox(m_gbox[1], layout, 4);
    m_gbox[1]->setFixedSize(160, 110);

    //3.右上角，改为左上角了
    CreateHLayout_v4(layout[0], tr("船首向上"), m_up_btn, tr("菜单"), m_menu_btn);
    CreateHLayout_v4(layout[1], tr("TM/RM"), m_tmrm_btn, tr("尾迹"), m_trial_btn);
    CreateToolButton(m_x_btn, "X波段");
    CreateHBoxLayout(layout[2]);
    layout[2]->addWidget(m_x_btn);
    layout[2]->setAlignment(Qt::AlignLeft);
    m_x_btn->setFixedSize(76,32);
    m_tmrm_btn->setFixedSize(76, 32);
    m_up_btn->setFixedSize(76, 32);
    m_menu_btn->setFixedSize(76, 32);
    m_trial_btn->setFixedSize(76, 32);

    CreateGroupBox(m_gbox[2], layout, 3);
    m_gbox[2]->setFixedSize(160, 120);

    //4.右下角
    QLabel *pLabel = new QLabel(this);
    pLabel->setTextFormat(Qt::RichText);
    pLabel->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
   // pLabel->setObjectName(tr("MousePosition"));
    m_mousePositionLabel = pLabel;
    QFont ft;
    ft.setPointSize(12);
    m_mousePositionLabel->setFont(ft);
    CreateHBoxLayout(layout[0]);
   // m_mousePositionLabel->setAlignment(Qt::AlignCenter);
    layout[0]->addWidget(m_mousePositionLabel);

    CreateGroupBox(m_gbox[3], layout, 1);
    m_gbox[3]->setFixedSize(200, 100);



    //5.角落全部显示为空
    CreateLabel(tr(""), m_irLabel);
    m_irLabel->setObjectName(tr("IR"));
    CreateLabel(tr(""), m_autoLabel);
    m_autoLabel->setObjectName(tr("OPINFO"));
    CreateLabel(tr(""), m_ofstLabel);
    m_ofstLabel->setObjectName(tr("OPINFO"));
    CreateLabel(tr(""), m_aisLabel);
    m_aisLabel->setObjectName(tr("OPINFO"));
    CreateLabel(tr(""), m_ataLabel);
    m_ataLabel->setObjectName(tr("OPINFO"));

    m_irLabel->setAlignment(Qt::AlignRight);
    m_ofstLabel->setAlignment(Qt::AlignRight);
    m_autoLabel->setAlignment(Qt::AlignRight);
    m_aisLabel->setAlignment(Qt::AlignRight);
    m_ataLabel->setAlignment(Qt::AlignRight);
    m_ofstLabel->setFixedWidth(50);
    m_autoLabel->setFixedWidth(50);
    m_aisLabel->setFixedWidth(50);
    m_ataLabel->setFixedWidth(50);

    CreateHBoxLayout_v1(layout[0], m_irLabel);
    CreateHBoxLayout_v2(layout[1], m_autoLabel, m_aisLabel);
    CreateHBoxLayout_v2(layout[2], m_ofstLabel, m_ataLabel);
    CreateGroupBox(m_gbox[4], layout, 3);
    m_gbox[4]->setFixedSize(110, 70);


}


//linux下设置系统时间
void MainWindow::setSystemTime(int year, int mon, int day, int hour, int min, int sec)
{
    time_t t;
    struct tm nowtime;
    nowtime.tm_sec = sec;  //(0-60)
    nowtime.tm_min = min;  //(0-59)
    nowtime.tm_hour = hour; //(0-23)
    nowtime.tm_mday = day;  //(1-31)
    nowtime.tm_mon = mon-1;  //(0-11)
    nowtime.tm_year = year-1900;  //(year -1900)
    nowtime.tm_isdst = -1;  //DST[-1.0.1]
    t = mktime(&nowtime);
    stime(&t);

   // m_lpDialog->updateDateTimeDisplay();

}

void MainWindow::setManTuneValue(int val)
{
        MenuConfig.otherMenu.tuneValue = val;
        if(m_lpOtherMenu && m_lpOtherMenu->isVisible())
            m_lpOtherMenu->setManTuneValue(val);
}

void MainWindow::showReplayDlg(bool show)
{
    if(show) {
        if(!m_lpRecordDlg) {
            m_lpRecordDlg = new ReplayDlg(lpMainWindow);
            m_lpRecordDlg->setFixedSize(410,170);
            m_lpRecordDlg->move(860 - m_lpRecordDlg->width(), 0);
            m_lpRecordDlg->setReplayObject(lpRecplay);
        }
        m_lpRecordDlg->show();
       // m_lpRecordDlg->activateWindow();
    }else {
        m_lpRecordDlg->hide();
    }
}
void MainWindow::switchRecord()
{
    if(lpRecplay) {
        if(lpRecplay->isRecording())
            lpRecplay->stopRecord();
        else
            lpRecplay->startRecord();
    }
}



void MainWindow::KbdDataSolve(QByteArray data)
{

    const quint32 size = data.size();
   // qDebug()<<"received serial data size:"<<size;

    for(int i=0; i<(size-2); ) {
    //只操作一次
    if(((quint8)data[i] == 0x23) && ((quint8)data[i+2] == 0x25))   //# = 0x23,  % = 0x25
    {
        switch((quint8)data[i+1]) {
            //////////旋钮长按////////
        case 0x1c:
            //VRM1,VRM2是否显示
            qDebug()<<"VRM EBL display";
            lpMainWindow->setVrmeblMode();
            break;
        case 0x2c:
            //手动调谐开关，但是调节大小用哪个按键？
            // lpMainWindow->setTuneMode();
             break;
        case 0x3c:

             break;
        case 0x4c:

             break;
        case 0x5c:

             break;
        case 0x6c:

             break;
             ////////////旋钮单按////////
         case 0x11:
             //切换vrm1 和 vrm2
             qDebug()<< "convet vrm1 and vrm2";
             lpMainWindow->switchVrmeblMode();
             break;
         case 0x22:
             //改变亮度
              lpMainWindow->setBrightSelect();
              break;
         case 0x33:
              //船艏线显示和关闭
              lpMainWindow->setHeadlineMode();  //主要调用视图里的函数显示刷新
              pView->update();
              break;
         case 0x44:
              //偏心操作
              lpMainWindow->setOffsetMode();

              break;
         case 0x55:
              //消音处理
              {

                  extern quint8 guardZone_alarm;
                  guardZone_alarm = 0;
                  closeSpeaking = 1;

              }
             break;
         case 0x66:
             //距标圈显示和关闭
             lpMainWindow->setRngringMode();
             pView->update();
             break;
        case 0x77:
             //开机/关机,按键的第二排第一个
            lpInteract->BtnOpenClose();
            break;
        case 0x88:
            //增加量程
            pView->rangeChanged(1);
            pView->update();
             break;
        case 0x99:      
             //减少量程
              pView->rangeChanged(0);
              pView->update();
             break;
        case 0xaa:
             //不用船艏向选择
              // lpMainWindow->setGuildMode();
             //ATA AIS切换
             {
             extern quint8 gAtaAisSwitch;
             if(gAtaAisSwitch)
                 m_lpDialog->setDispinfo(0);
             else
                 m_lpDialog->setDispinfo(1);
             }
             break;
        case 0xbb:
             lpMainWindow->on_m_cancel_btn_clicked(true);
            break;
        case 0xcc:
            lpMainWindow->on_m_menu_btn_clicked(true);
            break;
            //////////逆时针减////////
        case 0x01:
            //减少EBL的角度
            lpMainWindow->change_EBL(0);
            break;
        case 0x02:
            //降低亮度
             lpMainWindow->setBrightValue(0);
             break;
        case 0x03:
             //降低海浪抑制
             pView->change_clutter(0);
             break;
        case 0x04:
             //降低雨雪抑制
             pView->change_restrain(0);
             break;
        case 0x05:
             //降低增益
             pView->change_gain(0);
             break;
        case 0x06:
             //减少VRM的角度
             lpMainWindow->change_Vrm(0);
             break;
             //////////顺时针增////////
         case 0x10:
             //增加EBL的角度
             lpMainWindow->change_EBL(1);
             break;
         case 0x20:
             //增加亮度
              lpMainWindow->setBrightValue(1);
              break;
         case 0x30:
              //增加海浪抑制
              pView->change_clutter(1);
              break;
         case 0x40:
              //增加雨雪抑制
              pView->change_restrain(1);
              break;
         case 0x50:
              //增加增益
              pView->change_gain(1);
              break;
         case 0x60:
              //增加VRM的角度
              lpMainWindow->change_Vrm(1);
              break;

          default:
              break;

        }
        i += 3;  //3位一次
    }else { //end if

        i++;
    }
    }//end for

    //处理完后清空数据
//    data.clear();

}


// 获取经度字符串
QString Longitude2String(double lon, bool enflag)
{
    int degree = (int)fabs(lon);
    double minute = (fabs(lon) - degree) * 60.0;
   // int minute2 = (int)minute;
  //  int second = (int)((fabs(minute) - minute2) * 60.0);
    QString sLon = QString("%1°%2'").arg(degree).arg(minute, 6, 'f', 3, QLatin1Char('0'));  //分表示
   // QString sLon = QString("%1°%2'%3''").arg(degree).arg(minute2).arg(second);  //分秒表示
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
   // int minute2 = (int)minute;
   // int second = (int)((fabs(minute) - minute2) * 60.0);
    QString sLon = QString("%1°%2'").arg(degree).arg(minute, 5, 'f', 3, QLatin1Char('0'));
   // QString sLon = QString("%1°%2'%3''").arg(degree).arg(minute2).arg(second);  //分秒表示
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


///////////////////////////////////////////////////////////////////////////////////
//
// class DockWidget implement
// author : wang ming xiao
// date: 20150711
//
//////////////////////////////////////////////////////////////////////////////////
void DockWidget::initialize ()
{
    setObjectName(tr("DockWidget"));

    setFloating(false);
    setFeatures(QDockWidget::NoDockWidgetFeatures);
}

void DockWidget::moveEvent (QMoveEvent* event)
{
}

void DockWidget::leaveEvent(QEvent* event)
{
    QDockWidget::leaveEvent(event);
    clearFocus();
}

void DockWidget::allow(Qt::DockWidgetArea area, bool a)
{
    Qt::DockWidgetAreas areas = allowedAreas();
    areas = a ? areas | area : areas & ~area;
    setAllowedAreas(areas);
}

void DockWidget::place(Qt::DockWidgetArea area, bool p)
{
    if (!p)
    {
        setFloating(true);
    }
    else
    {
        QMainWindow *mainWindow = qobject_cast<QMainWindow *>(parentWidget());
        if (mainWindow)
            mainWindow->addDockWidget(area, this);
    }
}

bool DockWidget::IsVisible () const
{
    if (!isVisible ())
        return false;

    //检测是否在限定的矩形框里面，将widget的左上角位置转化为全局屏幕坐标然后再检测
    const QRect screenRect (0, 0, 1600, 1200);
    QPoint pt = mapToGlobal (QPoint(0, 0));
    return screenRect.contains(pt);
}

void DockWidget::changeClosable(bool on)
{ setFeatures(on ? features() | DockWidgetClosable : features() & ~DockWidgetClosable); }

void DockWidget::changeMovable(bool on)
{ setFeatures(on ? features() | DockWidgetMovable : features() & ~DockWidgetMovable); }

void DockWidget::changeFloatable(bool on)
{ setFeatures(on ? features() | DockWidgetFloatable : features() & ~DockWidgetFloatable); }

void DockWidget::changeFloating(bool floating)
{ setFloating(floating); }

void DockWidget::allowLeft(bool a)
{ allow(Qt::LeftDockWidgetArea, a); }

void DockWidget::allowRight(bool a)
{ allow(Qt::RightDockWidgetArea, a); }

void DockWidget::allowTop(bool a)
{ allow(Qt::TopDockWidgetArea, a); }

void DockWidget::allowBottom(bool a)
{ allow(Qt::BottomDockWidgetArea, a); }

void DockWidget::placeLeft(bool p)
{ place(Qt::LeftDockWidgetArea, p); }

void DockWidget::placeRight(bool p)
{ place(Qt::RightDockWidgetArea, p); }

void DockWidget::placeTop(bool p)
{ place(Qt::TopDockWidgetArea, p); }

void DockWidget::placeBottom(bool p)
{ place(Qt::BottomDockWidgetArea, p); }

void DockWidget::changeVerticalTitleBar(bool on)
{
    setFeatures(on ? features() | DockWidgetVerticalTitleBar
                    : features() & ~DockWidgetVerticalTitleBar);
}

void DockWidget::splitInto(QAction *action)
{
    QMainWindow *mainWindow = qobject_cast<QMainWindow *>(parentWidget());
    QList<DockWidget*> dock_list = qFindChildren<DockWidget*>(mainWindow);
    DockWidget *target = 0;
    foreach (DockWidget *dock, dock_list) {
        if (action->text() == dock->objectName()) {
            target = dock;
            break;
        }
    }
    if (target == 0)
        return;

    // ? Qt::Horizontal : Qt::Vertical;
    Qt::Orientation o = (Qt::Orientation)(action->data().toInt());

    mainWindow->splitDockWidget(target, this, o);
}

void DockWidget::tabInto(QAction *action)
{
    QMainWindow *mainWindow = qobject_cast<QMainWindow *>(parentWidget());
    QList<DockWidget*> dock_list = qFindChildren<DockWidget*>(mainWindow);
    DockWidget *target = 0;
    foreach (DockWidget *dock, dock_list) {
        if (action->text() == dock->objectName()) {
            target = dock;
            break;
        }
    }
    if (target == 0)
        return;

    mainWindow->tabifyDockWidget(target, this);
}






int GetTrackIntervalTime()
{
        const int interval[] = {0, 30, 60, 120, 180, 360, 0};
        return (MenuConfig.dispMenu.trackTime < 6 ? interval[MenuConfig.dispMenu.trackTime] : 0);
}


const quint16 * CurrentRange()
{
        return pView->currentRange();
}

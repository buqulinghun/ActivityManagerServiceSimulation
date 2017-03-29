/********************************************************************
 *日期: 2016-01-22
 *作者: 王名孝
 *作用: 主窗口设置，整体框架
 *修改:
 ********************************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_factorDialog.h"
#include "ui_adjustDialog.h"
#include "configuration.h"S
#include "s57/s57chart.h"
#include "recorder.h"
#include "ecdis.h"
#include "radarFile/interact.h"
#include  "radarFile/radaritem.h"


#include <QMessageBox>
#include <QDebug>
#include <QGLWidget>
#include <QTimer>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QResizeEvent>
#include <QSignalMapper>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QProgressBar>
#include <QDragMoveEvent>


extern SYSTEMINFO RadarSysinfo;
extern Configuration *configuration;
extern MainWindow* lpMainWindow;
extern MARINERSELECT MarinerSelect;
extern DataBase *dataBase;
extern Symbols *symbols;
extern Interact * lpInteract;
extern RadarItem *lpRadarItem;


extern QString Longitude2String(double lon, bool enflag);
extern QString Latitude2String(double lat, bool enflag);
QString screenCoorToString(int x, int y);
extern void saveMenuConfig();



#define SignalMap2(signal,slot,a,b)  \
{  \
    QSignalMapper *mapper = new QSignalMapper(this); \
    QObject::connect(a,SIGNAL(signal),mapper,SLOT(map()));  mapper->setMapping(a,0); \
    QObject::connect(b,SIGNAL(signal),mapper,SLOT(map()));  mapper->setMapping(b,1);\
    QObject::connect(mapper,SIGNAL(mapped(int)),this,SLOT(slot(int)));  \
}
#define SignalMap3(signal,slot,a,b,c)  \
{  \
    QSignalMapper *mapper = new QSignalMapper(this); \
    QObject::connect(a,SIGNAL(signal),mapper,SLOT(map()));  mapper->setMapping(a,0); \
    QObject::connect(b,SIGNAL(signal),mapper,SLOT(map()));  mapper->setMapping(b,1);\
    QObject::connect(c,SIGNAL(signal),mapper,SLOT(map()));  mapper->setMapping(c,2);\
    QObject::connect(mapper,SIGNAL(mapped(int)),this,SLOT(slot(int)));  \
}
#define SignalMap5(signal,slot,a,b,c,d,e)  \
{  \
    QSignalMapper *mapper = new QSignalMapper(this); \
    QObject::connect(a,SIGNAL(signal),mapper,SLOT(map()));  mapper->setMapping(a,1); \
    QObject::connect(b,SIGNAL(signal),mapper,SLOT(map()));  mapper->setMapping(b,2);\
    QObject::connect(c,SIGNAL(signal),mapper,SLOT(map()));  mapper->setMapping(c,3);\
    QObject::connect(d,SIGNAL(signal),mapper,SLOT(map()));  mapper->setMapping(d,4);\
    QObject::connect(e,SIGNAL(signal),mapper,SLOT(map()));  mapper->setMapping(e,5);\
    QObject::connect(mapper,SIGNAL(mapped(int)),this,SLOT(slot(int)));  \
}



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    lpMainWindow = this;
    ui->setupUi(this);
    setStatusBarWidget();

    myDialog = new MyDialog;
    dataBase = new DataBase;
    adjustDlg = NULL;



    //利用图形视图框架来显示
 // QGLWidget *widget = new QGLWidget(QGLFormat(QGL::SampleBuffers), this);  //为什么不能用
    myScene = new MyScene(this);
    const Rgb back = configuration->color_map.find(MarinerSelect.dayState).value().value("NODTA");
    myScene->setBackgroundBrush(QColor(back.r, back.g, back.b));   //为什么没有显示出来
    myView = new MyView(this);
   // myView->setViewport(widget);
    myView->setAutoFillBackground(true);
    myView->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    myView->setDragMode(QGraphicsView::ScrollHandDrag);  //手型拖动
    myView->setRenderHint(QPainter::Antialiasing, false);
    myView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    myView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    myView->setScene(myScene);
    myView->setMouseTracking(true);

    setCentralWidget(myView);

    myScene->createRadarImage();   //创建雷达图像

    //连接处理信号
    QObject::connect(dataBase, SIGNAL(readyToRender(std::string)), myScene, SLOT(renderChart(std::string)));
    QObject::connect(dataBase, SIGNAL(readyToRenderOther(QString,bool,bool,bool)), myScene, SLOT(refreshScreen(QString,bool,bool,bool)));
    SignalMap3(triggered(),display_clicked,ui->displayAll,ui->displayStand,ui->displayBase);
    SignalMap5(triggered(),color_clicked,ui->BRIGHT,ui->WHITEBACK,ui->BLACKBACK,ui->DUSK,ui->NIGHT);
    SignalMap2(triggered(),point_clicked,ui->simplePoint,ui->paperPoint);
    SignalMap2(triggered(),area_clicked,ui->symbolArea,ui->simpleArea);
    SignalMap2(triggered(),language_clicked,ui->chinese,ui->english);
    QObject::connect(myView, SIGNAL(signal_scale(float)), myScene, SLOT(slot_scale(float)));

    refreshDisplayMenu();  //刷新菜单选项
}

MainWindow::~MainWindow()
{
    delete ui;

}

//窗口初始化
void MainWindow::Init()
{
    myScene->initData(&(dataBase->ChartData));   //传递雷达数据


}




void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}



QPointF MainWindow::screenCrenterToScene() const
{
    const int vWidth =240 ;   //视图屏幕中心坐标,这个是雷达物标左上角的位置
    const int vHeight = 112 ;

    const QPointF sceneCenter = myView->mapToScene(vWidth, vHeight);
    return sceneCenter;
}




/****************操作处理************************/
void MainWindow::on_sailRecoder_triggered()
{
    const quint32 endtime = time(0);
    const quint32 starttime = QDateTime::fromTime_t(time(0)).addMonths(-6).toTime_t();  //显示6个月的数据
    myDialog->showSailRecorderWindow(starttime, endtime, 60);   //间隔1分

}

void MainWindow::on_alarmRecoder_triggered()
{
    const quint32 endtime = time(0);
    const quint32 starttime = QDateTime::fromTime_t(time(0)).addMonths(-6).toTime_t();  //显示6个月的数据
     myDialog->showAlarmRecorderWindow(starttime, endtime);
}

void MainWindow::on_chartSelect_triggered()
{
    dataBase->showChartSelectWindow();
}

void MainWindow::on_safeContour_triggered()
{
    //界面选择
    float oldSafety = MarinerSelect.SAFETY_CONTOUR;
    FactorDiaog safetyDialog(lpMainWindow);
    safetyDialog.move(1280 - safetyDialog.width(), 10);
    if(safetyDialog.exec() == QDialog::Accepted)
        if(oldSafety != MarinerSelect.SAFETY_CONTOUR) {
            myScene->refreshScreen(myScene->currentChartName(), true, false, false);   //区域部分重新生成
            saveMenuConfig();
        }
}



void MainWindow::display_clicked(int val)
{
    int displayType;
    switch(val) {
        case 0:  displayType = MARINERS_OTHER; ui->displayAll->setChecked(true);ui->displayStand->setChecked(false);ui->displayBase->setChecked(false); break;
        case 1:  displayType = STANDARD;ui->displayAll->setChecked(false);ui->displayStand->setChecked(true);ui->displayBase->setChecked(false);  break;
        case 2:  displayType = DISPLAY_BASE;ui->displayAll->setChecked(false);ui->displayStand->setChecked(false);ui->displayBase->setChecked(true);  break;
    }
    if(displayType != MarinerSelect.dispCategory){    
        MarinerSelect.dispCategory = displayType;
        myScene->refreshScreen(myScene->currentChartName(), false, false, false);
        saveMenuConfig();
    }
}

void MainWindow::color_clicked(int val)
{
    //12345分别表示
    if(val != MarinerSelect.dayState) {
        ui->BRIGHT->setChecked(val == 1);
        ui->WHITEBACK->setChecked(val == 2);
        ui->BLACKBACK->setChecked(val == 3);
        ui->DUSK->setChecked(val == 4);
        ui->NIGHT->setChecked(val == 5);
        MarinerSelect.dayState = val;
        symbols->Init();   //重新根据颜色生成物标
        myScene->refreshScreen(myScene->currentChartName(), false, false, false);
        const Rgb back = configuration->color_map.find(MarinerSelect.dayState).value().value("NODTA");
        myScene->setBackgroundBrush(QColor(back.r, back.g, back.b));   //为什么没有显示出来
        saveMenuConfig();
    }
}
void MainWindow::point_clicked(int val)
{
    //需要重新生成符号化语句
    //0简单符号  1纸质符号
    if(!val){
        if((!MarinerSelect.simplePoint)) {
            ui->simplePoint->setChecked(true);
            ui->paperPoint->setChecked(false);
            MarinerSelect.simplePoint = true;
            myScene->refreshScreen(myScene->currentChartName(), false, false, true);
        }
    }else{
        if(MarinerSelect.simplePoint) {
            ui->simplePoint->setChecked(false);
            ui->paperPoint->setChecked(true);
            MarinerSelect.simplePoint = false;
            myScene->refreshScreen(myScene->currentChartName(), false, false, true);
        }
    }
    saveMenuConfig();


}
void MainWindow::area_clicked(int val)
{
    //需要重新生成符号化语句
    //0符号化边界1简单化边界
    if((bool)val != MarinerSelect.plainArea) {
        ui->simpleArea->setChecked(val == 1);
        ui->symbolArea->setChecked(val == 0);
        MarinerSelect.plainArea = val;
        myScene->refreshScreen(myScene->currentChartName(), true, false, false);
        saveMenuConfig();
    }
}
void MainWindow::language_clicked(int val)
{
    //0中文1英文
    if((bool)val != MarinerSelect.languageSelect) {
        ui->chinese->setChecked(val == 0);
        ui->english->setChecked(val == 1);
        MarinerSelect.languageSelect = val;
        //刷新显示

        saveMenuConfig();
    }
}

void MainWindow::refreshDisplayMenu()
{
    ui->displayAll->setChecked(MarinerSelect.dispCategory == MARINERS_OTHER);
    ui->displayStand->setChecked(MarinerSelect.dispCategory == STANDARD);
    ui->displayBase->setChecked(MarinerSelect.dispCategory == DISPLAY_BASE);

    ui->BRIGHT->setChecked(MarinerSelect.dayState == 1);
    ui->WHITEBACK->setChecked(MarinerSelect.dayState == 2);
    ui->BLACKBACK->setChecked(MarinerSelect.dayState == 3);
    ui->DUSK->setChecked(MarinerSelect.dayState == 4);
    ui->NIGHT->setChecked(MarinerSelect.dayState == 5);

    ui->simplePoint->setChecked(MarinerSelect.simplePoint);
    ui->paperPoint->setChecked(!MarinerSelect.simplePoint);

    ui->simpleArea->setChecked(MarinerSelect.plainArea);
    ui->symbolArea->setChecked(!MarinerSelect.plainArea);

    ui->chinese->setChecked(!MarinerSelect.languageSelect);
    ui->english->setChecked(MarinerSelect.languageSelect);
}

void MainWindow::setStatusBarWidget()
{
    ui->statusBar->showMessage(tr("欢迎使用电子海图显示系统"), 5000);   //开机显示5秒

    QLabel *screen = new QLabel(this);
    QLabel *longlat = new QLabel(this);
    screenCoord = new QLabel(this);
    longlatCoord = new QLabel(this);
    screenCoord->setFixedWidth(100);
    longlatCoord->setFixedWidth(200);
    screen->setText(tr("屏幕坐标:"));
    longlat->setText(tr("经纬度坐标:"));
    QString long_lat = Longitude2String(centerLong, true) + QString(", ") + Latitude2String(centerLat, true);
    longlatCoord->setText(long_lat);
    QString screen_coor = QString("x:%1, y:%2").arg(640).arg(512);
    screenCoord->setText(screen_coor);
    QLabel *scale = new QLabel(this);
    scaleNumber = new QLabel(this);
    scaleNumber->setFixedWidth(100);
    scale->setText(tr("比例尺:"));
    scaleNumber->setText(QString("1:%1").arg(MarinerSelect.chartScaleNum));
 /*   QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(screen);
    layout->addWidget(screenCoord);
    layout->addWidget(longlat);
    layout->addWidget(longlatCoord); */

    ui->statusBar->addPermanentWidget(screen);
    ui->statusBar->addPermanentWidget(screenCoord);
    ui->statusBar->addPermanentWidget(longlat);
    ui->statusBar->addPermanentWidget(longlatCoord);
    ui->statusBar->addPermanentWidget(scale);
    ui->statusBar->addPermanentWidget(scaleNumber);
}

void MainWindow::setCoordinateDisp(int x, int y, double lon, double lat)
{
    QString screen_coor = QString("x:%1, y:%2").arg(x).arg(y);
    screenCoord->setText(screen_coor);
    QString long_lat = Longitude2String(lon, true) + QString(", ") + Latitude2String(lat, true);
    longlatCoord->setText(long_lat);
}
void MainWindow::setScaleDisp(int scale)
{
    scaleNumber->setText(QString("1:%1").arg(scale));
}

void MainWindow::on_openAndClose_triggered()
{
    if(lpRadarItem && lpInteract)  {
        lpInteract->BtnOpenClose();
        lpRadarItem->initEchoView();
        lpRadarItem->update();
    }
    ui->openAndClose->setChecked(RadarSysinfo.transmite);
}

void MainWindow::on_rangeAdd_triggered()
{
    if(MarinerSelect.radarShow && lpInteract) {
        lpInteract->rangeChanged(1);
        if(MarinerSelect.scaleMatch)
            myScene->calculateMatchScale();
    }
}

void MainWindow::on_rangeSec_triggered()
{
    if(MarinerSelect.radarShow && lpInteract) {
        lpInteract->rangeChanged(0);
        if(MarinerSelect.scaleMatch)
            myScene->calculateMatchScale();
    }
}

void MainWindow::on_offsetOps_triggered()
{
    if(! MarinerSelect.radarShow)   return;
     RadarSysinfo.offset = RadarSysinfo.offset ? false : true;
    if(lpRadarItem)
        lpRadarItem->setOffsetStatus((quint8)RadarSysinfo.offset);

}

void MainWindow::on_gainOps_triggered()
{
    if(! MarinerSelect.radarShow)   return;
    //界面选择
    if(!adjustDlg) {
        adjustDlg = new AdjustDialog(lpMainWindow);
        adjustDlg->move(1280 - adjustDlg->width(), 10);
    }
    adjustDlg->show();
    adjustDlg->setDialogType(AdjustDialog::gainDlg);
}

void MainWindow::on_restrainOps_triggered()
{
    if(! MarinerSelect.radarShow)   return;
    //界面选择
    if(!adjustDlg) {
        adjustDlg = new AdjustDialog(lpMainWindow);
        adjustDlg->move(1280 - adjustDlg->width(), 10);
    }
    adjustDlg->show();
    adjustDlg->setDialogType(AdjustDialog::restrainDlg);
}

void MainWindow::on_clutterOps_triggered()
{
    if(! MarinerSelect.radarShow)   return;
    //界面选择
    if(!adjustDlg) {
        adjustDlg = new AdjustDialog(lpMainWindow);
        adjustDlg->move(1280 - adjustDlg->width(), 10);
    }
    adjustDlg->show();
    adjustDlg->setDialogType(AdjustDialog::clutterDlg);
}

void MainWindow::on_tuneMan_triggered()
{
    if(! MarinerSelect.radarShow)   return;
    //界面选择
    if(!adjustDlg) {
        adjustDlg = new AdjustDialog(lpMainWindow);
        adjustDlg->move(1280 - adjustDlg->width(), 10);
    }
    adjustDlg->show();
    adjustDlg->setDialogType(AdjustDialog::tuneDlg);
}

void MainWindow::on_tuneAuto_triggered()
{
    if(! MarinerSelect.radarShow)   return;
    if(lpInteract)
        lpInteract->change_tune(1, 65500);
}
void MainWindow::on_action_2_triggered()
{
    if(! MarinerSelect.radarShow)   return;
    //后面加界面选择
    if(lpInteract)
        lpInteract->setRngAdjust(50);
}

//比例尺匹配
void MainWindow::on_scaleMatch_triggered()
{
    if(! MarinerSelect.radarShow)   return;
    MarinerSelect.scaleMatch = MarinerSelect.scaleMatch ? false : true;
    if(myScene)
        myScene->calculateMatchScale();

}
//透明度设置
void MainWindow::on_highAlpha_triggered()
{
    if(! MarinerSelect.radarShow)   return;
    if(lpRadarItem)
        lpRadarItem->setColorAlpha(0xbf);
    ui->highAlpha->setChecked(true);
    ui->middleAlpha->setChecked(false);
    ui->lowAlpha->setChecked(false);
}

void MainWindow::on_middleAlpha_triggered()
{
    if(! MarinerSelect.radarShow)   return;
    if(lpRadarItem)
        lpRadarItem->setColorAlpha(0x8f);
    ui->highAlpha->setChecked(false);
    ui->middleAlpha->setChecked(true);
    ui->lowAlpha->setChecked(false);
}

void MainWindow::on_lowAlpha_triggered()
{
    if(! MarinerSelect.radarShow)   return;
    if(lpRadarItem)
        lpRadarItem->setColorAlpha(0x4f);
    ui->highAlpha->setChecked(false);
    ui->middleAlpha->setChecked(false);
    ui->lowAlpha->setChecked(true);
}
//雷达图像颜色设置

void MainWindow::on_green_triggered()
{
    if(! MarinerSelect.radarShow)   return;
    if(lpRadarItem)
        lpRadarItem->setEchoColor(0);
    ui->green->setChecked(true);
    ui->yellow->setChecked(false);
    ui->red->setChecked(false);
}

void MainWindow::on_yellow_triggered()
{
    if(! MarinerSelect.radarShow)   return;
    if(lpRadarItem)
        lpRadarItem->setEchoColor(1);
    ui->green->setChecked(false);
    ui->yellow->setChecked(true);
    ui->red->setChecked(false);
}

void MainWindow::on_red_triggered()
{
    if(! MarinerSelect.radarShow)   return;
    if(lpRadarItem)
        lpRadarItem->setEchoColor(2);
    ui->green->setChecked(false);
    ui->yellow->setChecked(false);
    ui->red->setChecked(true);
}

//关闭雷达图像
void MainWindow::on_closeRadar_triggered()
{
    if(MarinerSelect.radarShow) {
        MarinerSelect.radarShow = false;
        myScene->createRadarImage();
        ui->showRadar->setChecked(false);
        ui->closeRadar->setChecked(true);
    }
}
//显示雷达图像
void MainWindow::on_showRadar_triggered()
{
    if(!MarinerSelect.radarShow) {
        MarinerSelect.radarShow = true;
        myScene->createRadarImage();
        ui->showRadar->setChecked(true);
        ui->closeRadar->setChecked(false);
    }
}



/*****************************************************************
 class FactorDialog
 *****************************************************************/
FactorDiaog::FactorDiaog(QWidget *parent): QDialog(parent), ui(new Ui::factorDialog)
{
   // setWindowFlags(Qt::Tool);
     setWindowFlags(Qt::Widget|Qt::FramelessWindowHint);
    setWindowTitle(tr("安全线深度设置"));
    ui->setupUi(this);

    ui->factorEdit->setText(QString::number(MarinerSelect.SAFETY_CONTOUR));

}
FactorDiaog::~FactorDiaog()
{
    delete ui;
}


void FactorDiaog::on_okButton_clicked()
{
    float setSafety = ui->factorEdit->text().toFloat();
    if(setSafety != MarinerSelect.SAFETY_CONTOUR)
        MarinerSelect.SAFETY_CONTOUR = setSafety;

    accept();
}


/*****************************************************************
 class AdjustDialog
 *****************************************************************/
AdjustDialog::AdjustDialog(QWidget *parent): QDialog(parent), ui(new Ui::adjustDialog)
{
  //  setWindowFlags(Qt::Tool);
    setWindowFlags(Qt::Widget|Qt::FramelessWindowHint);
    ui->setupUi(this);

}

AdjustDialog::~AdjustDialog()
{
    delete ui;
}

void AdjustDialog::setDialogType(dialogType type)
{
    dlgType = type;
    switch(dlgType) {
    case gainDlg:
        setAdjustLabel(tr("增益调节"));
        ui->numBar->setRange(0, 100);
        setNumBarValue(RadarSysinfo.gainNum);
        break;
    case restrainDlg:
        setAdjustLabel(tr("雨雪抑制调节"));
        ui->numBar->setRange(0, 100);
        setNumBarValue(RadarSysinfo.restrainNum);
        break;
    case clutterDlg:
        setAdjustLabel(tr("海浪抑制调节"));
        ui->numBar->setRange(0, 100);
         setNumBarValue(RadarSysinfo.clutterNum);
        break;
    break;
    case tuneDlg:
        setAdjustLabel(tr("手动调谐值调节"));
        ui->numBar->setRange(0, 512);
        setNumBarValue(RadarSysinfo.tuneManValue);
        break;
    }
}

void AdjustDialog::on_addButton_clicked()
{
    switch(dlgType) {
    case gainDlg:
        RadarSysinfo.gainNum++;
        setNumBarValue(RadarSysinfo.gainNum);
        if(lpInteract)  lpInteract->change_gain(1);
        break;
    case restrainDlg:
        RadarSysinfo.restrainNum++;
        setNumBarValue(RadarSysinfo.restrainNum);
         if(lpInteract)  lpInteract->change_restrain(1);
        break;
    case clutterDlg:
        RadarSysinfo.clutterNum++;
        setNumBarValue(RadarSysinfo.clutterNum);
         if(lpInteract)  lpInteract->change_clutter(1);
        break;
    case tuneDlg:
        RadarSysinfo.tuneManValue++;
        setNumBarValue(RadarSysinfo.tuneManValue);
         if(lpInteract)  lpInteract->change_tune(0, RadarSysinfo.tuneManValue);
        break;
    }
}
void AdjustDialog::on_decButton_clicked()
{
    switch(dlgType) {
    case gainDlg:
        RadarSysinfo.gainNum--;
        setNumBarValue(RadarSysinfo.gainNum);
         if(lpInteract)  lpInteract->change_gain(0);
        break;
    case restrainDlg:
        RadarSysinfo.restrainNum--;
        setNumBarValue(RadarSysinfo.restrainNum);
         if(lpInteract)  lpInteract->change_restrain(0);
        break;
    case clutterDlg:
        RadarSysinfo.clutterNum--;
        setNumBarValue(RadarSysinfo.clutterNum);
         if(lpInteract)  lpInteract->change_clutter(0);
        break;
    case tuneDlg:
        RadarSysinfo.tuneManValue--;
        setNumBarValue(RadarSysinfo.tuneManValue);
        if(lpInteract)  lpInteract->change_tune(0, RadarSysinfo.tuneManValue);
        break;
    }
}
void AdjustDialog::setAdjustLabel(QString label)
{
    ui->adjustLabel->setText(label);
}
void AdjustDialog::setNumBarValue(int number)
{
    ui->numBar->setValue(number);
}

void AdjustDialog::on_enterButton_clicked()
{
    close();
}

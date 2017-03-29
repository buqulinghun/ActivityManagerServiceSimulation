/****************************************************************************
file name: dialog.cpp
author: wang ming xiao
date: 2015/07/25
comments:  创建各类对话框，以及一些操作
***************************************************************************/

#include "dialog.h"
#include "mainwindow.h"
#include "define.h"
#include "boatinfo.h"
#include "TargetManage/TargetManage_global.h"
#include "interact.h"
#include "glwidget.h"
#include "CustomEvent.h"
#include "aismanage.h"
#include "boatalarm.h"
#include "mouseoperation.h"
#include "recordreplay/recordreplay.h"


#include "ui_operatedlg.h"
#include "ui_aisbox.h"
#include "ui_atabox.h"
#include "ui_mainmenu.h"
#include "ui_displaymenu.h"
#include "ui_functionmenu.h"
#include "ui_installmenu.h"
#include "ui_serialconfigmenu.h"
#include "ui_inputmenu.h"
#include "ui_alarm.h"
#include"ui_ReplayDlg.h"

#include <QSignalMapper>
#include <QAbstractButton>

extern MainWindow *lpMainWindow;
extern MENUCONFIG MenuConfig;
extern SYSTEMINFO SystemInfo;
extern uchar gXinnengMonitor;
extern Interact *lpInteract;
extern GLWidget *pView;
extern COLORCONFIG *lpColorConfig;
extern AisManage* lpAisManage;
extern Alarm* lpAlarm;
extern MouseOperation *lpMouseOpetarion;
extern boatalarm* m_boatAlarm;

const QString gStrDataInvalid = "----";
#define MMSIstring(mmsi) (QString("%1").arg(mmsi, 9, 10, QLatin1Char('0')))


quint8 flag_editChanged=0;
bool isReplaying = false;

Dialog::Dialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::OperateDlg),m_operateNodeTree(NULL)
{
    setWindowFlags(Qt::Widget);
    ui->setupUi(this);

    createAisAndAtaBox();

    //设置报警字体颜色
    ui->alarmlist->setStyleSheet("color:rgba(255,0,0,180)");

    //测试报警信息
    for(int i=0; i<MAX_OUTER_DEVICE; i++)
        setLinkStatusDisp(i, BROKEN);

}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    switch (event->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Dialog::createAisAndAtaBox()
{
    {
    QBoxLayout *vlayout1 = new QVBoxLayout;
    vlayout1->setContentsMargins(0,0,0,0);
    vlayout1->setSpacing(0);
    m_aisBoxes[0] = new AisBox(this); m_aisBoxes[0]->updateDisplay(0); vlayout1->addWidget(m_aisBoxes[0]);
    m_aisBoxes[1] = new AisBox(this); m_aisBoxes[1]->updateDisplay(0); vlayout1->addWidget(m_aisBoxes[1]);
    m_aisBoxes[2] = new AisBox(this); m_aisBoxes[2]->updateDisplay(0); vlayout1->addWidget(m_aisBoxes[2]);
    m_aisBoxes[3] = new AisBox(this); m_aisBoxes[3]->updateDisplay(0); vlayout1->addWidget(m_aisBoxes[3]);
    m_aisBoxes[4] = new AisBox(this); m_aisBoxes[4]->updateDisplay(0); vlayout1->addWidget(m_aisBoxes[4]);
    ui->ais_box->setLayoutDirection(Qt::LeftToRight);
    ui->ais_box->setLayout(vlayout1);
    }

    {
    QBoxLayout *vlayout2 = new QVBoxLayout;
    vlayout2->setContentsMargins(0,0,0,0);
    vlayout2->setSpacing(0);
    m_ataBoxes[0] = new AtaBox(this); m_ataBoxes[0]->updateDisplay(0); vlayout2->addWidget(m_ataBoxes[0]);
    m_ataBoxes[1] = new AtaBox(this); m_ataBoxes[1]->updateDisplay(0); vlayout2->addWidget(m_ataBoxes[1]);
    m_ataBoxes[2] = new AtaBox(this); m_ataBoxes[2]->updateDisplay(0); vlayout2->addWidget(m_ataBoxes[2]);
    m_ataBoxes[3] = new AtaBox(this); m_ataBoxes[3]->updateDisplay(0); vlayout2->addWidget(m_ataBoxes[3]);
    m_ataBoxes[4] = new AtaBox(this); m_ataBoxes[4]->updateDisplay(0); vlayout2->addWidget(m_ataBoxes[4]);
    ui->ata_box->setLayoutDirection(Qt::LeftToRight);
    ui->ata_box->setLayout(vlayout2);
    }

    ui->ata_box->hide();

//    ui->targetDispBox->setFixedHeight(230);
    m_crntAisIndex = m_crntAtaIndex = 0;
    m_aisIndex[0] = m_aisIndex[1] = m_aisIndex[2] = m_aisIndex[3] = m_aisIndex[4] = 0;
    m_ataIndex[0] = m_ataIndex[1] = m_ataIndex[2] = m_ataIndex[3] = m_ataIndex[4] = 0;


}

void Dialog::on_alarmStatus_clicked(bool)
{
    alarmProcess();
}

void Dialog::alarmProcess()
{
    extern quint8 flag_isSpeaking;
    if(lpAlarm && flag_isSpeaking)
    {
        lpAlarm->stopAlarm();
    }
   // setLinkStatusDisp(4, BROKEN);
    //m_alarmStatusBtn->setStatus(BROKEN);  //不显示
    clearAlarmInfo();
}
void Dialog::clearAlarmInfo()
{
    ui->alarmlist->clear();
}




void Dialog::addOperateWidget(QWidget *widget, QRect rect)
{
    if(m_operateNodeTree == NULL) {

        m_operateNodeTree = new NODE(widget);
        m_operateNodeTree->labelrect = rect;

    }else {
        LPNODE new_node = new NODE(widget);  //使用new分配内存才能调用构造函数
        new_node->labelrect = rect;   //部件区域

        LPNODE p = m_operateNodeTree->next;
        LPNODE q = m_operateNodeTree;
        while(p != NULL) {
            q = p;
            p = p->next;
        }

        q->next = new_node;
        new_node->prev = q;  //前一指针
        new_node = NULL;
    }
}
void Dialog::initOperateWidget()
{
    QPoint pos = ui->toolButton_1->pos() + ui->mainGroupBox->pos();
    pos = this->mapToGlobal(pos);

    QRect rect = QRect(pos, ui->toolButton_1->size());   //主菜单的按钮部件70*25
    addOperateWidget(ui->toolButton_1, rect);

    pos = this->mapToGlobal(ui->toolButton_2->pos() + ui->mainGroupBox->pos());
    rect = QRect(pos, ui->toolButton_2->size());
    addOperateWidget(ui->toolButton_2, rect);


    /*pos = this->mapToGlobal(ui->alarmStatus->pos() + ui->mainGroupBox->pos() + ui->alarmStatusBox->pos());
    rect = QRect(pos, ui->alarmStatus->size());
    addOperateWidget(ui->alarmStatus, rect); */

}
bool Dialog::kbd_confirm(QPoint pt, quint8 flag)
{
    if(flag > 0) {
        //确认处理
        LPNODE root = m_operateNodeTree;
        while(root) {
            if(root->labelrect.contains(pt)){
                root->focusion = true;
                QAbstractButton *btn = qobject_cast<QAbstractButton*>(root->widget);
                if(btn) {
                    btn->click();
                    return true;
                }

            }
            root = root->next;

        }
    }

    return false;
}




void Dialog::showEvent(QShowEvent *)
{
    updateBoatInfoDisplay();
    updateDateTimeDisplay();

}

void Dialog::customEvent(QEvent *event)
{
    //显示报警信息
    if(event->type() == QEvent::Type(MSGID_AlarmInfo))
    {
       CustomEvent* evt = (CustomEvent*)event;
       showAlarmInfo(evt->variant.toString());
      //lpAlarm->record(ALARM_LEVEL3, evt->variant.toString());
    }
}

void Dialog::showAlarmInfo(const QString& info)
{
    ui->alarmlist->insertItem(0,info);

    // 最多保留9行记录
    const int total = ui->alarmlist->count();
    if(total > 9){
        //将第10行删除掉
         QListWidgetItem* item = ui->alarmlist->takeItem(total-1);
         if(item) delete item;
     }
}

void Dialog::on_toolButton_1_clicked(bool)
{
    setDispinfo(0);
}
void Dialog::on_toolButton_2_clicked(bool)
{
    setDispinfo(1);
}
// 0:ais, 1:ata
void Dialog::setDispinfo(int index)
{
    extern quint8 gAtaAisSwitch;
    gAtaAisSwitch = index;

    QToolButton* button[] = {ui->toolButton_1, ui->toolButton_2};
    QWidget* container[] = {ui->ais_box, ui->ata_box};

    button[0]->setChecked(0 == index);
    button[1]->setChecked(1 == index);

    button[0]->setDown(0 == index);
    button[1]->setDown(1 == index);

    if(0 == index)
    {
        container[1]->setVisible(1 == index);
        container[0]->setVisible(0 == index);
    }
    else
    {
        container[0]->setVisible(0 == index);
        container[1]->setVisible(1 == index);


    }

}

// 更新本船信息显示
void Dialog::updateBoatInfoDisplay()
{
        //qDebug() << "<<<<<<<<<<updateBoatInfoDisplay" << SystemInfo.ShipInfo.position;
        const LINKSTATUS * lpStatus = lpMainWindow->linkStatus();
        // 位置
        if(NORMAL == lpStatus[GPS_DEVICE].status)
        {
            const QPointF& pt = SystemInfo.ShipInfo.position;
            ui->lineEdit_1->setText(Longitude2String(pt.x() * 180.0 / M_PI, 1));   //后面参数表示语言选择，1英文0中文
            ui->lineEdit_2->setText(Latitude2String(pt.y() * 180.0 / M_PI, 1));
        }
        else
        {
            ui->lineEdit_1->setText("---°--.---'-");
            ui->lineEdit_2->setText("--°--.---'-");
        }

        // 艏向
        if(NORMAL == lpStatus[COMP_DEVICE].status)
             ui->lineEdit_4->setText(QString::number(SystemInfo.ShipInfo.head,'f',1));
        else
             ui->lineEdit_4->setText("---");

        // 航向,取GPS 的速度。状态正常则输出
        if(NORMAL == lpStatus[GPS_DEVICE].status)  //手动，GPS，罗经
                ui->lineEdit_3->setText(QString::number(SystemInfo.ShipInfo.course,'f',1));
        else
                ui->lineEdit_3->setText("---");

        // 对水速度
        if(NORMAL == lpStatus[LOG_DEVICE].status)
                ui->lineEdit_6->setText(QString::number(SystemInfo.ShipInfo.speed,'f',1));
        else
                ui->lineEdit_6->setText("---");

        // 对地速度  取GPS 的速度。状态正常则输出
        if(NORMAL == lpStatus[GPS_DEVICE].status) //手动，GPS，LOG(计程仪)
                ui->lineEdit_5->setText(QString::number(SystemInfo.ShipInfo.sog,'f',1));
        else
                ui->lineEdit_5->setText("---");
}

// 更新系统日期时间显示
void Dialog::updateDateTimeDisplay()
{
    ui->dateTimeEdit->setDateTime(QDateTime::fromTime_t(SystemInfo.crnt_time));
}

void Dialog::updateAtaBtnState()
{
        /*const int id = lpMouseOperation->mouseProcessID();
        ui->ataAutoBtn->setChecked(auto_flag);
        ui->ataAutoBtn->setText(auto_flag ? "自动" : "手动");
        ui->ataNewBtn->setChecked(ECHO_DOOR_PROCESS == id);
        ui->ataModifyBtn->setChecked(MODIFY_TRACK_PROCESS == id);
        ui->ataDeleteBtn->setChecked(DELETE_TRACK_PROCESS == id);
        ui->clearAllBtn->setChecked(false);*/
}

void Dialog::updateDataSwitch()
{
        //extern quint8 gAtaAisSwitch;
        //setDispinfo(gAtaAisSwitch);
}


// 更新链路状态显示,当为BROKEN时报警列表显示，发声报警
void Dialog::setLinkStatusDisp(quint8 index, quint8 status)
{
    if(status == BROKEN){
        QString msg;
        switch(index) {
            case 0:  //AIS
                msg = QString("AIS数据丢失！");
                if(lpAlarm)
                lpAlarm->startAlarm((ALARMLEVEL)3, msg);
            break;
            case 1:  //GPS
                msg = QString("GPS数据丢失！");
                if(lpAlarm)
                lpAlarm->startAlarm((ALARMLEVEL)3, msg);
            break;
            case 2:  //罗经GYRO COMP
                msg = QString("GYRO数据丢失！");
                if(lpAlarm)
                lpAlarm->startAlarm((ALARMLEVEL)3, msg);
            break;
            case 3:  //LOG记程仪
                msg = QString("LOG数据丢失！");
                if(lpAlarm)
                lpAlarm->startAlarm((ALARMLEVEL)3, msg);
            break;
            case 4:  //其他报警数据再另外显示
                //QString msg = QString("AIS数据丢失报警！");
                //showAlarmInfo(msg);
            break;
        }
    }

}
//参数意义：显示盒子指针，相应盒子的索引（MMSI），几个显示盒子，指定索引MMSI，操作标志（清除更新插入切换）
template<class T> void updateBox(T* boxes[], quint32 indexes[], quint8 count, quint32 idx_specify, quint8 flag)
{
        const int count1 = count-1;
        int index = 0;
        for(; index<count; index++)
        {
                if(idx_specify == (quint32)indexes[index])
                        break;
        }

        if(TgtBox::TGTBOX_CLEAR == flag || (TgtBox::TGTBOX_SWITCH == flag && index < count))
        {	// clear  即将对应的显示清除，将下一个显示移到该区域，如果时最后一个的话将其清空
                for(; index < count1 && indexes[index+1] != 0; index++)
                {
                        indexes[index] = indexes[index+1];
                        boxes[index]->updateDisplay(indexes[index]);  //根据MMSI号更新
                }

                if(index < count){
                        indexes[index] = 0;
                        boxes[index]->updateDisplay(0);
                }
        }
        else if(TgtBox::TGTBOX_UPDATE == flag)
        {	// update，跟新信息显示
                if(index < count) {
                boxes[index]->updateDisplay(indexes[index]);
                }
        }
        else if(TgtBox::TGTBOX_INSERT == flag || (TgtBox::TGTBOX_SWITCH == flag && index >= count))
        {	// insert
                if(index < count)
                {
                        boxes[index]->updateDisplay(indexes[index]);
                }
                else
                {
                    //在最上面插入新的AIS信息
                        for(int i=count1; i>0; i--)
                        {
                                if(indexes[i] != indexes[i-1])
                                {
                                indexes[i] = indexes[i-1];
                                boxes[i]->updateDisplay(indexes[i]);   //将所有的显示信息下移
                                }
                        }

                        indexes[0] = idx_specify;
                        boxes[0]->updateDisplay(indexes[0]);
                }
        }
}

// flag 0:clear, 1:update, 2:insert, 3:switch display
void Dialog::updateAis(quint32 mmsi, quint8 flag)
{
     QMutexLocker lock(&m_mutex);
     updateBox<AisBox>(m_aisBoxes, m_aisIndex, 5, mmsi, flag);
}

void Dialog::updateAta(quint32 index, quint8 flag)
{
     QMutexLocker lock(&m_mutex);
     updateBox<AtaBox>(m_ataBoxes, m_ataIndex, 5, index, flag);
}

// 获取AIS目标显示索引号
int Dialog::getAisDispIndex(quint32 mmsi)
{
    QMutexLocker lock(&m_mutex);
    for(int i=0; i<5; i++)
    {
         if(m_aisIndex[i] == mmsi)
               return i+1;
     }
    return -1;
}

// 获取ATA目标显示索引号
int Dialog::getAtaDispIndex(quint32 index)
{
    QMutexLocker lock(&m_mutex);
    for(int i=0; i<5; i++)
    {
        if(m_ataIndex[i] == index)
            return i+1;
    }
    return -1;
}

// 更新目标Box显示
void Dialog::clearTargetBoxDisplay(quint8 flag)
{
    QMutexLocker lock(&m_mutex);
    if(flag & 0x01)
    {
         for(int i=0; i<5; i++)
         updateBox<AisBox>(m_aisBoxes, m_aisIndex, i, 0, 0);
    }

    if(flag & 0x02)
    {
        for(int i=0; i<5; i++)
        updateBox<AtaBox>(m_ataBoxes, m_ataIndex, i, 0, 0);
    }
}

//////////////////////////////////////////////////////////////
/*************************************************
* class  : AisBox
* author :
* date   :2015-7-15
*************************************************/
AisBox::AisBox(QWidget *parent)
:QWidget(parent), ui(new Ui::AisBox),m_mmsi(0)
{
        ui->setupUi(this);

        ui->groupBox->setObjectName("TgtBox");
        ui->label_1->setObjectName("TgtBoxLabel1");
        ui->label_3->setObjectName("TgtBoxLabel1");
        ui->label_4->setObjectName("TgtBoxLabel1");
        ui->label_5->setObjectName("TgtBoxLabel1");
        ui->label_6->setObjectName("TgtBoxLabel1");
        ui->label_7->setObjectName("TgtBoxLabel1");
        ui->label_13->setObjectName("TgtBoxLabel1");
        ui->label_14->setObjectName("TgtBoxLabel1");
        ui->label_15->setObjectName("TgtBoxLabel1");
        ui->label_16->setObjectName("TgtBoxLabel1");
        ui->label_17->setObjectName("TgtBoxLabel1");

        ui->name->setObjectName("TgtBoxLabel2");
        ui->callCode->setObjectName("TgtBoxLabel2");
        ui->mmsi->setObjectName("TgtBoxLabel2");
        ui->longitude->setObjectName("TgtBoxLabel2");
        ui->latitude->setObjectName("TgtBoxLabel2");
        ui->speed->setObjectName("TgtBoxLabel2");
        ui->course->setObjectName("TgtBoxLabel2");
        ui->azimuth->setObjectName("TgtBoxLabel2");
        ui->range->setObjectName("TgtBoxLabel2");
        ui->cpa->setObjectName("TgtBoxLabel2");
        ui->tcpa->setObjectName("TgtBoxLabel2");
}

AisBox::~AisBox()
{
}

void AisBox::updateDisplay(quint32 mmsi)
{

    if(!mmsi)
    {
         clearDisplay();
            return;
    }

    //qDebug() << __FUNCTION__ << mmsi;
    AIS_SHIP* pShip = lpAisManage->getAisShip(mmsi, false);   //得到显示数据
    if(pShip)
    {
    m_mmsi = mmsi;
    // 1. 船名
    if(strlen(pShip->staticInfo.name))
        ui->name->setText(QString::fromAscii(pShip->staticInfo.name/*, 20*/));
    else
        ui->name->setText(gStrDataInvalid);
     // 2. 呼号
    if(strlen(pShip->staticInfo.callCode))
        ui->callCode->setText(QString::fromAscii(pShip->staticInfo.callCode/*, 10*/));
    else
        ui->callCode->setText(gStrDataInvalid);
       // 3. MMSI
    if (pShip->staticInfo.MMSI !=0)
        ui->mmsi->setText(MMSIstring(pShip->staticInfo.MMSI));
    else
        ui->mmsi->setText(gStrDataInvalid);
       // 4. 航向
    if (IsDataValid1(pShip->dynamicInfo.course, 0, 360))	//!EQUAL3(pShip->dynamicInfo.course, 360.0, 0.01))
        ui->course->setText(QString("%1").arg(pShip->dynamicInfo.course, 0, 'f', 1));
    else
        ui->course->setText(QString("%1").arg(gStrDataInvalid));
        // 5. 速度
    if(IsDataValid1(pShip->dynamicInfo.sog, 0, 100))	//!EQUAL3(pShip->dynamicInfo.sog, 102.3, 0.01) && pShip->dynamicInfo.sog>0)
        ui->speed->setText(QString("%1").arg(pShip->dynamicInfo.sog, 0, 'f', 1));
    else
        ui->speed->setText(QString("%1").arg(gStrDataInvalid));
          // 6. 距离
    if(IsDataValid1(pShip->navigationInfo.range, 0.01, 100))	//!EQUAL3(pShip->navigationInfo.range, 0, 0.01))
        ui->range->setText(QString("%1").arg(pShip->navigationInfo.range, 0, 'f', 3));
    else
        ui->range->setText(QString("%1").arg(gStrDataInvalid));
      // 7. 方位
    if(IsDataValid1(pShip->navigationInfo.azimuth, 0, 360))	//!EQUAL3(pShip->navigationInfo.azimuth, 0, 0.01))
        ui->azimuth->setText(QString("%1").arg(pShip->navigationInfo.azimuth, 0, 'f', 1));
    else
        ui->azimuth->setText(QString("%1").arg(gStrDataInvalid));
         // 8.9. CPA/TCPA
    if(IsDataValid1(pShip->navigationInfo.DCPA,0,999))
        ui->cpa->setText(QString("%1").arg(pShip->navigationInfo.DCPA,0,'f',2));
    else
        ui->cpa->setText(gStrDataInvalid);
    if(IsDataValid1(pShip->navigationInfo.TCPA, 0, 999))
        ui->tcpa->setText(QString("%1").arg(pShip->navigationInfo.TCPA));
    else
        ui->tcpa->setText(gStrDataInvalid);
      // 10.11. 经度/纬度
    const double lon = pShip->dynamicInfo.lon;
    const double lat = pShip->dynamicInfo.lat;
    if(IsDataValid0(lon, -180, 180) && IsDataValid0(lat, -90, 90))
    {
        ui->longitude->setText(Longitude2String(lon, true));		// 经度
        ui->latitude->setText(Latitude2String(lat, true));		// 纬度
    }
    else
    {
        ui->longitude->setText(gStrDataInvalid);		// 经度;
        ui->latitude->setText(gStrDataInvalid);	// 纬度;
    }
    }
    else
    {
            clearDisplay();
    }


}

void AisBox::clearDisplay()
{
        m_mmsi = 0;
        ui->name->setText(gStrDataInvalid);
        ui->callCode->setText(gStrDataInvalid);
        ui->mmsi->setText(gStrDataInvalid);
        ui->longitude->setText(gStrDataInvalid);
        ui->latitude->setText(gStrDataInvalid);
        ui->course->setText(gStrDataInvalid);
        ui->speed->setText(gStrDataInvalid);
        ui->range->setText(gStrDataInvalid);
        ui->azimuth->setText(gStrDataInvalid);
        ui->cpa->setText(gStrDataInvalid);
        ui->tcpa->setText(gStrDataInvalid);
}

void AisBox::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    switch (event->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        updateDisplay(0);
        break;
    default:
        break;
    }
}

/*************************************************
* class  : AtaBox
* author :
* date   : 2015-7-15
*************************************************/
AtaBox::AtaBox(QWidget *parent)
:QWidget(parent), ui(new Ui::AtaBox),m_index(0)
{
        ui->setupUi(this);

        ui->groupBox->setObjectName("TgtBox");
        ui->label_1->setObjectName("TgtBoxLabel1");
        ui->label_3->setObjectName("TgtBoxLabel1");
        ui->label_4->setObjectName("TgtBoxLabel1");
        ui->label_5->setObjectName("TgtBoxLabel1");
        ui->label_6->setObjectName("TgtBoxLabel1");
        ui->label_13->setObjectName("TgtBoxLabel1");
        ui->label_14->setObjectName("TgtBoxLabel1");
        ui->label_15->setObjectName("TgtBoxLabel1");
        ui->label_16->setObjectName("TgtBoxLabel1");

        ui->atano->setObjectName("TgtBoxLabel2");
        ui->azimuth->setObjectName("TgtBoxLabel2");
        ui->range->setObjectName("TgtBoxLabel2");
        ui->speed->setObjectName("TgtBoxLabel2");
        ui->course->setObjectName("TgtBoxLabel2");
        ui->trueSpeed->setObjectName("TgtBoxLabel2");
        ui->trueCourse->setObjectName("TgtBoxLabel2");
        ui->cpa->setObjectName("TgtBoxLabel2");
        ui->tcpa->setObjectName("TgtBoxLabel2");
}

AtaBox::~AtaBox()
{
}

void AtaBox::updateDisplay(quint32 mmsi)
{

    if(!mmsi)
    {
        clearDisplay();
            return;
    }
/*
    {
            m_index = mmsi;
            const int index = mmsi - 1;
            TRACK* track = lpTrack->getTrack(index);
            if(!track || !track->extraData)
            {
                    clearDisplay();
            }
            else
            {
                    LPATAPARAM extraInfo = (LPATAPARAM)(track->extraData);
                    const TRACKPOINT& trackPoint = track->points[track->curPointPos];
                    //RTHETA_POINT rtheta = trackPoint.position.rtheta_point;
                    ui->atano->setText(QString("%1").arg(track->no, 3, 10, QLatin1Char('0')));

                    // 相对方位距离
                    ui->range->setText(QString("%1").arg(((float)(short)extraInfo->relativeRng) / 100.0, 0, 'f', 3));
                    ui->azimuth->setText(QString("%1").arg(((float)(short)extraInfo->relativeAzi) / 10.0, 0, 'f', 1));

                    // 相对速度航向
                    if(IsDataValid1(extraInfo->relativeSpeed, 0, 10000))
                            ui->speed->setText(QString("%1").arg(((float)(short)extraInfo->relativeSpeed) / 100.0, 0, 'f', 1));
                    else
                            ui->speed->setText(gStrDataInvalid);
                    if(IsDataValid1(extraInfo->relativeCourse, 0, 3600))
                            ui->course->setText(QString("%1").arg(((float)(short)extraInfo->relativeCourse) / 10.0, 0, 'f', 1));
                    else
                            ui->course->setText(gStrDataInvalid);

                    // 真速度航向
                    if(IsDataValid1(extraInfo->trueSpeed, 0, 10000))
                            ui->trueSpeed->setText(QString("%1").arg(((float)(short)extraInfo->trueSpeed) / 100.0, 0, 'f', 1));
                    else
                            ui->trueSpeed->setText(gStrDataInvalid);
                    if(IsDataValid1(extraInfo->trueCourse, 0, 3600))
                            ui->trueCourse->setText(QString("%1").arg(((float)(short)extraInfo->trueCourse) / 10.0, 0, 'f', 1));
                    else
                            ui->trueCourse->setText(gStrDataInvalid);
                    // dcpa/tcpa
                    if(IsDataValid1(extraInfo->dcpa, 0, 10000))
                            ui->cpa->setText(QString("%1").arg(((float)(short)extraInfo->dcpa) / 100.0, 0, 'f', 2));
                    else
                            ui->cpa->setText(gStrDataInvalid);
                    if(IsDataValid1(extraInfo->tcpa, 0, 999))
                            ui->tcpa->setText(QString("%1").arg(extraInfo->tcpa));
                    else
                            ui->tcpa->setText(gStrDataInvalid);

                    //qDebug() << track->no << extraInfo->dcpa;
            }
    }  */

}

void AtaBox::clearDisplay()
{
        m_index = 0;
        ui->atano->setText(gStrDataInvalid);
        ui->azimuth->setText(gStrDataInvalid);
        ui->range->setText(gStrDataInvalid);
        ui->course->setText(gStrDataInvalid);
        ui->speed->setText(gStrDataInvalid);
        ui->trueCourse->setText(gStrDataInvalid);
        ui->trueSpeed->setText(gStrDataInvalid);
        ui->cpa->setText(gStrDataInvalid);
        ui->tcpa->setText(gStrDataInvalid);
}
void AtaBox::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    switch (event->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        updateDisplay(0);
        break;
    default:
        break;
    }
}










/*************************************************
* class  : MenuDlg
* author : wangmingxiao
* date   : 2015-8-18
*************************************************/
MenuDlg::MenuDlg(QWidget *parent) : QDialog(parent),m_lpNodeHead(NULL)
{
    setWindowFlags(Qt::Widget|Qt::FramelessWindowHint);
  //  setWindowFlags(Qt::WindowModal);
    this->setMaximumWidth(256);//(200);
    m_changed = false;
    this->setMouseTracking(true);

}
MenuDlg::~MenuDlg()
{
    freeLink(m_lpNodeHead);
}

void MenuDlg::freeLink(LPNODE head)
{
    LPNODE crnt = head , next;
    while(crnt) {
        next = crnt->next;
        delete crnt;
        crnt = next;
    }
}

void MenuDlg::toHideMe()
{
    MainWindow *win = qobject_cast<MainWindow*>(parent());   //返回object向下的转型MainWindow
    if(win)
        win->gobackMenu();
    else
        hide();
}

void MenuDlg::showEvent(QShowEvent *)
{
    adjustSize();
    //stepFirstLine();
   // setCrntNode(m_lpCrntNode);
    updateDisplay();
}

void MenuDlg::hideEvent(QHideEvent *)
{
    if(m_changed)
    {
        //qDebug() << "to saveMenuConfig";
        m_changed = false;
        extern void saveMenuConfig();
        saveMenuConfig();
    }
}
/*
void MenuDlg::mouseMoveEvent(QMouseEvent *event)
{
    //只记录位置，不计算其他数据
    lpMouseOpetarion->mouseMoved(event->globalPos(), false);
   // qDebug()<<"mouse dialog"<<event->globalPos().x()<<event->globalPos().y();
}
*/
//按键确认对控件的控制
bool MenuDlg::kbd_confirm(QPoint pt,quint8 flag)
{
    if(flag > 0) {
        //确认处理
        LPNODE root = m_lpNodeHead;
        while(root) {
          //  if(root->labelrect.contains(pt)){
            QRect rect = QRect(mapToGlobal(root->widget->pos()),root->widget->rect().size());
            if(rect.contains(pt)) {
                root->focusion = true;
                //qobject_cast()返回object向下的转型T，如果转型不成功则返回0，如果传入的Object本身就是0则返回0
                QAbstractButton *btn = qobject_cast<QAbstractButton*>(root->widget);
                if(btn) {
                    btn->click();
                    return true;
                }else {
                    qDebug()<<"pos contain but btn convert false";
                    // 输入框对确认键的处理，弹出输入窗口
                    QLineEdit *edit = qobject_cast<QLineEdit*>(root->widget);
                    if(edit) {
                        InputMenu menu(lpMainWindow);
                        menu.setFixedSize(FULLSCREEN_WIDTH-MAINVIEW_RADIUS + 7, MAINVIEW_RADIUS);
                        menu.move(MAINVIEW_RADIUS, 0);
                        menu.show();
                        menu.activateWindow();
                        lpMainWindow->setInputStack(&menu, 1);  //将其加入堆栈
                        menu.createOperation();

                        if(QDialog::Accepted == menu.exec()){
                            const QValidator *lpValidator = edit->validator();
                            int pos = 0;
                             QString text = menu.getText();
                            if(!lpValidator || lpValidator->validate(text, pos) == QValidator::Acceptable){
                                edit->setText(text);
                                flag_editChanged = 1;
                             }

                          //  lpMainWindow->setInputStack(&menu, 0);  //将其从堆栈中取出
                            if(lpMainWindow)
                                lpMainWindow->gobackMenu();
                        }else {
                           // lpMainWindow->setInputStack(&menu, 0);  //将其从堆栈中取出
                            if(lpMainWindow)
                                lpMainWindow->gobackMenu();
                        }
                        return true;

                    }/*else {
                            //滑动条确认键
                            QSlider *slider = qobject_cast<QSlider*>(root->widget);
                            if(slider ) {
                                if(slider->isEnabled()) {
                                    int value = slider->value();
                                    slider->setValue(value + 32);
                                }
                               // qDebug()<<"slider";
                                return true;
                            }



                    } */

                }

            }
            root = root->next;

        }

    }/*else {
        //取消处理
        LPNODE root = m_lpNodeHead;
        while(root) {
            if(root->labelrect.contains(pt)){
                root->focusion = false;
                //滑动条减少数值
                QSlider *slider = qobject_cast<QSlider*>(root->widget);
                if(slider && slider->isEnabled()) {
                    int value = slider->value();
                    slider->setValue(value - 32);
                   // qDebug()<<"slider";
                    return true;

                }else {
                    return false;
                }
           }
           root = root->next;

       } //end while
    }  //end if  */

    return false;
}


void MenuDlg::addWidget(QWidget *widget, QRect rect)
{

    if(m_lpNodeHead == NULL) {

        m_lpNodeHead = new NODE(widget);
        m_lpNodeHead->labelrect = rect;

    }else {
        LPNODE new_node = new NODE(widget);  //使用new分配内存才能调用构造函数
        new_node->labelrect = rect;   //部件区域

        LPNODE p = m_lpNodeHead->next;
        LPNODE q = m_lpNodeHead;
        while(p != NULL) {
            q = p;
            p = p->next;
        }

        q->next = new_node;
        new_node->prev = q;  //前一指针
        new_node = NULL;
    }
}


/*************************************************
* class  : MainMenu
* author : wangmingxiao
* date   : 2015-8-18
*************************************************/
MainMenu::MainMenu(QWidget *parent)
        :MenuDlg(parent),ui(new Ui::MainMenu)
{
    ui->setupUi(this);

    setWindowTitle(tr("配置菜单"));
    connect(ui->toolButton, SIGNAL(clicked(bool)), this, SLOT(toHideMe()));


}
MainMenu::~MainMenu()
{
    delete ui;
}

void MainMenu::createOperation()
{

   /* QPoint pos = ui->toolButton->pos();
    pos = this->mapToGlobal(pos);

    QRect rect = QRect(pos, ui->toolButton->size());   //主菜单的按钮部件70*25
    addWidget(ui->toolButton, rect);
    pos = mapToGlobal(ui->toolButton_1->pos());
    rect = QRect(pos, ui->toolButton_1->size());
    addWidget(ui->toolButton_1, rect);
    pos = mapToGlobal(ui->toolButton_2->pos());
    rect = QRect(pos, ui->toolButton_2->size());
    addWidget(ui->toolButton_2, rect);
    pos = mapToGlobal(ui->toolButton_3->pos());
    rect = QRect(pos, ui->toolButton_3->size());
    addWidget(ui->toolButton_3, rect); */
    addObject(ui->toolButton);
    addObject3(ui->toolButton_1, ui->toolButton_2, ui->toolButton_3);

}

void MainMenu::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    switch(event->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}
void MainMenu::on_toolButton_1_clicked(bool)
{
    MainWindow *win = qobject_cast<MainWindow*>(parent());
    if(win)
        win->showDisplayMenu();
}
void MainMenu::on_toolButton_2_clicked(bool)
{
    MainWindow *win = qobject_cast<MainWindow*>(parent());
    if(win)
        win->showOtherMenu();
}
void MainMenu::on_toolButton_3_clicked(bool)
{
    InputMenu menu(lpMainWindow);
    menu.setPassword(1);  //显示密码样式
    menu.setFixedSize(FULLSCREEN_WIDTH-MAINVIEW_RADIUS + 7, MAINVIEW_RADIUS);
    menu.move(MAINVIEW_RADIUS, 0);
    menu.show();
    menu.activateWindow();
    lpMainWindow->setInputStack(&menu, 1);  //将其加入堆栈
    menu.createOperation();

        if((QDialog::Accepted == menu.exec()) && (menu.getText() == "654321")){
               /* if(menu.getText() == "654321") */{
                    MainWindow *win = qobject_cast<MainWindow*>(parent());
                    lpMainWindow->setInputStack(&menu, 0);  //从堆栈中取出

                    if(win)  win->showInstallMenu();
                }
        }else{
           MainWindow *win = qobject_cast<MainWindow*>(parent());
         //  lpMainWindow->setInputStack(&menu, 0);

            if(win)  win->gobackMenu();
        }
}


/////SignalMap()信号映射，因为几个信号连接到一个槽
#define SignalMap(signal, slot, list) \
{ \
    QSignalMapper *mapper = new QSignalMapper(this); \
    int i=0; \
    foreach(QAbstractButton *w, list) { \
        connect(w, SIGNAL(signal), mapper, SLOT(map())); \
        mapper->setMapping(w, i++); \
    } \
    connect(mapper, SIGNAL(mapped(int)), this, SLOT(slot(int))); \
}
#define SignalMap2(signal,slot,a,b) \
{   \
QSignalMapper* mapper = new QSignalMapper(this);\
connect(a, SIGNAL(signal), mapper, SLOT(map()));mapper->setMapping(a, 0);\
connect(b, SIGNAL(signal), mapper, SLOT(map()));mapper->setMapping(b, 1);\
connect(mapper, SIGNAL(mapped(int)), this, SLOT(slot(int)));\
}
#define SignalMap3(signal,slot,a,b,c) \
{   \
QSignalMapper* mapper = new QSignalMapper(this);\
connect(a, SIGNAL(signal), mapper, SLOT(map()));mapper->setMapping(a, 0);\
connect(b, SIGNAL(signal), mapper, SLOT(map()));mapper->setMapping(b, 1);\
connect(c, SIGNAL(signal), mapper, SLOT(map()));mapper->setMapping(c, 2);\
connect(mapper, SIGNAL(mapped(int)), this, SLOT(slot(int)));\
}
#define SignalMap4(signal,slot,a,b,c,d) \
{   \
QSignalMapper* mapper = new QSignalMapper(this);\
connect(a, SIGNAL(signal), mapper, SLOT(map()));mapper->setMapping(a, 0);\
connect(b, SIGNAL(signal), mapper, SLOT(map()));mapper->setMapping(b, 1);\
connect(c, SIGNAL(signal), mapper, SLOT(map()));mapper->setMapping(c, 2);\
connect(d, SIGNAL(signal), mapper, SLOT(map()));mapper->setMapping(d, 3);\
connect(mapper, SIGNAL(mapped(int)), this, SLOT(slot(int)));\
}



#define CheckButton(v,list) { \
if(v>=0&&v<list.size())  \
    for(int i=0; i<list.size(); i++) {  \
    list[i]->setChecked(false);  \
    list[i]->setEnabled(true);  \
    }  \
list[v]->setChecked(true);  \
list[v]->setEnabled(false); \
}
//一个选中就中就没法让它无效了
#define CheckButton1(v, a) {   \
switch(v){ \
    case 0:a->setChecked(false); break;  \
    case 1:a->setChecked(true);  break;  \
} }
#define CheckButton2(v,a,b) {\
switch(v){  \
case 0:a->setChecked(true);b->setChecked(false); a->setEnabled(false); b->setEnabled(true); break;\
case 1:b->setChecked(true);a->setChecked(false); b->setEnabled(false); a->setEnabled(true); break;\
}}
#define CheckButton3(v,a,b,c) {\
switch(v){  \
case 0:a->setChecked(true);b->setChecked(false);c->setChecked(false); a->setEnabled(false); b->setEnabled(true);c->setEnabled(true); break;\
case 1:a->setChecked(false);b->setChecked(true);c->setChecked(false); a->setEnabled(true); b->setEnabled(false);c->setEnabled(true);break;\
case 2:a->setChecked(false);b->setChecked(false);c->setChecked(true); a->setEnabled(true); b->setEnabled(true);c->setEnabled(false);break;\
}}
#define CheckButton4(v,a,b,c,d) {\
switch(v){  \
case 0:a->setChecked(true);b->setChecked(false);c->setChecked(false);d->setChecked(false); a->setEnabled(false); b->setEnabled(true);c->setEnabled(true);d->setEnabled(true); break;\
case 1:a->setChecked(false);b->setChecked(true);c->setChecked(false);d->setChecked(false); a->setEnabled(true); b->setEnabled(false);c->setEnabled(true);d->setEnabled(true); break;\
case 2:a->setChecked(false);b->setChecked(false);c->setChecked(true);d->setChecked(false); a->setEnabled(true); b->setEnabled(true);c->setEnabled(false);d->setEnabled(true); break;\
case 3:a->setChecked(false);b->setChecked(false);c->setChecked(false);d->setChecked(true); a->setEnabled(true); b->setEnabled(true);c->setEnabled(true);d->setEnabled(false); break;\
}}

#define SetText(edit, val){edit->setText(QString::number(val));}
#define SetTextNm(edit, val){edit->setText(QString::number(val)+QString(" Nm"));}
#define SetTextAzi(edit, val){edit->setText(QString::number(val)+QString("°"));}
#define SetTextMin(edit, val){edit->setText(QString::number(val)+QString(" min"));}
#define FloatFromText(variable, text, min, max) \
{ \
    bool ok;\
    float val = text.toFloat(&ok);\
    if(ok && val >= min && val <= max)  {variable = val; setChangedFlag(); }\
}



/*************************************************
* class  : DisplayMenu
* author : wangmingxiao
* date   : 2015-8-18
*************************************************/
DisplayMenu::DisplayMenu(QWidget *parent)
        :MenuDlg(parent),ui(new Ui::DisplayMenu)
{
    ui->setupUi(this);

    setWindowTitle(tr("显示配置"));
    connect(ui->toolButton, SIGNAL(clicked(bool)), this, SLOT(toHideMe()));

    QList<QAbstractButton*> trails;
    trails << ui->trail_1 << ui->trail_2 << ui->trail_3 << ui->trail_4 << ui->trail_5 \
             << ui->trail_6;
    QList<QAbstractButton*> veclens;
    veclens << ui->veclen_1 << ui->veclen_2 << ui->veclen_3 << ui->veclen_4 << ui->veclen_5 << ui->veclen_6;
    QList<QAbstractButton*> tracks;
    tracks << ui->track_1 << ui->track_2 << ui->track_3 << ui->track_4 << ui->track_5 << ui->track_6;
    QList<QAbstractButton*> colors;
    colors << ui->color_1 << ui->color_2 << ui->color_3;


    SignalMap3(clicked(bool), upmode_clicked, ui->upmode_1, ui->upmode_2, ui->upmode_3);
    SignalMap2(clicked(bool), motionmode_clicked, ui->motionmode_1, ui->motionmode_2);
    SignalMap3(clicked(bool), daymode_clicked, ui->daymode_1, ui->daymode_2, ui->daymode_3);
    SignalMap2(clicked(bool), offset_clicked, ui->offset_1, ui->offset_2);
    SignalMap2(clicked(bool), headlinedisp_clicked, ui->headlinedisp_1, ui->headlinedisp_2);
    SignalMap2(clicked(bool), rngringdisp_clicked, ui->rngringdisp_1, ui->rngringdisp_2);
    SignalMap(clicked(bool), trail_clicked, trails);
    SignalMap(clicked(bool), veclen_clicked, veclens);
    SignalMap(clicked(bool), track_clicked, tracks);
    SignalMap3(clicked(bool), symbSize_clicked, ui->symbsize_1, ui->symbsize_2, ui->symbsize_3);
    SignalMap(clicked(bool), color_clicked, colors);
    SignalMap2(clicked(bool), aisdisplay_clicked, ui->aisdisplay_1, ui->aisdisplay_2);
    SignalMap2(clicked(bool), arpadisplay_clicked, ui->arpadisplay_1, ui->arpadisplay_2);
    SignalMap4(valueChanged(int), bright_valueChanged, ui->screenBright, ui->kbdBright, ui->lineBright1, ui->lineBright2);


}
DisplayMenu::~DisplayMenu()
{
    delete ui;
}

void DisplayMenu::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    switch (event->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


//保存每个部件的区域。来确定鼠标操作哪个部件
void DisplayMenu::createOperation()
{
    //按顺序添加的
    addObject(ui->toolButton);
    addObject2(ui->motionmode_1, ui->motionmode_2);
    addObject3(ui->upmode_1, ui->upmode_2, ui->upmode_3);
    addObject3(ui->daymode_1, ui->daymode_2, ui->daymode_3);
    addObject2(ui->offset_1, ui->offset_2);
    addObject6(ui->trail_1, ui->trail_2, ui->trail_3, ui->trail_4, ui->trail_5, ui->trail_6);
    addObject6(ui->veclen_1, ui->veclen_2, ui->veclen_3, ui->veclen_4, ui->veclen_5, ui->veclen_6);
    addObject6(ui->track_1, ui->track_2, ui->track_3, ui->track_4, ui->track_5, ui->track_6);
    addObject3(ui->symbsize_1, ui->symbsize_2, ui->symbsize_3);
    addObject3(ui->color_1, ui->color_2, ui->color_3);
    addObject2(ui->headlinedisp_1, ui->headlinedisp_2);
    addObject2(ui->rngringdisp_1, ui->rngringdisp_2);
    addObject2(ui->aisdisplay_1, ui->aisdisplay_2);
    addObject2(ui->arpadisplay_1, ui->arpadisplay_2);


}
//更新面板各个按钮显示
void DisplayMenu::updateDisplay()
{
    QList<QAbstractButton*> trails;
    trails << ui->trail_1 << ui->trail_2 << ui->trail_3 << ui->trail_4 << ui->trail_5 \
             << ui->trail_6;
    QList<QAbstractButton*> veclens;
    veclens << ui->veclen_1 << ui->veclen_2 << ui->veclen_3 << ui->veclen_4 << ui->veclen_5 << ui->veclen_6;
    QList<QAbstractButton*> tracks;
    tracks << ui->track_1 << ui->track_2 << ui->track_3 << ui->track_4 << ui->track_5 << ui->track_6;
    QList<QAbstractButton*> colors;
    colors << ui->color_1 << ui->color_2 << ui->color_3;

    //MenuConfig 根据upmode的值，判断哪个按钮处于check的状态
    CheckButton3(MenuConfig.dispMenu.upmode, ui->upmode_1, ui->upmode_2, ui->upmode_3);
    CheckButton2(MenuConfig.dispMenu.motion, ui->motionmode_1, ui->motionmode_2);
    CheckButton3(MenuConfig.dispMenu.dayNight, ui->daymode_1, ui->daymode_2, ui->daymode_3);
    CheckButton2(MenuConfig.dispMenu.offset, ui->offset_1, ui->offset_2);
    CheckButton2(MenuConfig.dispMenu.showHeadLine, ui->headlinedisp_1, ui->headlinedisp_2);
    CheckButton2(MenuConfig.dispMenu.showRngRing, ui->rngringdisp_1, ui->rngringdisp_2);
    CheckButton(MenuConfig.dispMenu.echoTrail, trails);
    CheckButton(MenuConfig.dispMenu.vecterLength, veclens);
    CheckButton(MenuConfig.dispMenu.trackTime, tracks);
    CheckButton3(MenuConfig.dispMenu.symbolSize, ui->symbsize_1, ui->symbsize_2, ui->symbsize_3);
    CheckButton(MenuConfig.dispMenu.colorSelect, colors);
    CheckButton2(MenuConfig.dispMenu.showAis, ui->aisdisplay_1, ui->aisdisplay_2);
    CheckButton2(MenuConfig.dispMenu.showArpa, ui->arpadisplay_1, ui->arpadisplay_2);

    ui->screenBright->setValue(MenuConfig.dispMenu.screenBright);
    ui->kbdBright->setValue(MenuConfig.dispMenu.kbdBright);
    ui->lineBright1->setValue(MenuConfig.dispMenu.fixlineBright);
    ui->lineBright2->setValue(MenuConfig.dispMenu.varlineBright);
}
// 向上模式选择
void DisplayMenu::upmode_clicked(int val)
{
    if(MenuConfig.dispMenu.upmode != val) {
    MenuConfig.dispMenu.upmode = val;
    CheckButton3(MenuConfig.dispMenu.upmode, ui->upmode_1, ui->upmode_2, ui->upmode_3);
    if(pView)
       pView->setUpMode(val);
    setChangedFlag();
    }
}

// 运动方式选择
void DisplayMenu::motionmode_clicked(int val)
{
    if(MenuConfig.dispMenu.motion != val) {
    MenuConfig.dispMenu.motion = val;
    CheckButton2(MenuConfig.dispMenu.motion, ui->motionmode_1, ui->motionmode_2);
    if(pView)
       pView->setMotion(val);
    setChangedFlag();
    }
}

// 情景模式选择
void DisplayMenu::daymode_clicked(int val)
{
    if(MenuConfig.dispMenu.dayNight != val){
    MenuConfig.dispMenu.dayNight = val;
    CheckButton3(MenuConfig.dispMenu.dayNight, ui->daymode_1, ui->daymode_2, ui->daymode_3);
    if(lpMainWindow)
       lpMainWindow->setDisplayMode(val);
    setChangedFlag();
    }
}

// 偏心显示
void DisplayMenu::offset_clicked(int val)
{
    if(MenuConfig.dispMenu.offset != val){
        MenuConfig.dispMenu.offset = val;
        CheckButton2(MenuConfig.dispMenu.offset, ui->offset_1, ui->offset_2);
       // if(val == 0)  //关闭偏心,打开偏心不起作用
        {
            pView->setOffset(val);
            //if(lpMouseOperation) lpMouseOperation->setMouseProcessID(MenuConfig.dispMenu.offset ? OFFSET_PROCESS : NULL_PROCESS);
        }
        //更新角落显示
        lpMainWindow->updateCtrlPanel(VIEW_UPDATE_OFST_BIT);
        setChangedFlag();
    }
}

// 船艏线显示
void DisplayMenu::headlinedisp_clicked(int val)
{
    if(MenuConfig.dispMenu.showHeadLine != val){
    MenuConfig.dispMenu.showHeadLine = val;
    CheckButton2(MenuConfig.dispMenu.showHeadLine, ui->headlinedisp_1, ui->headlinedisp_2);
    if(pView) {
        pView->setHeadlineShow(val);
        pView->update();
    }
    setChangedFlag();
    }
}

// 距离环显示
void DisplayMenu::rngringdisp_clicked(int val)
{
    if(MenuConfig.dispMenu.showRngRing != val){
    MenuConfig.dispMenu.showRngRing = val;
    CheckButton2(MenuConfig.dispMenu.showRngRing, ui->rngringdisp_1, ui->rngringdisp_2);
    if(pView) {
        pView->setRngringShow(val);
        pView->update();
     }
    setChangedFlag();
    }
}

// 回波尾迹时间
void DisplayMenu::trail_clicked(int val)
{
    if(MenuConfig.dispMenu.echoTrail != val){
    QList<QAbstractButton*> trails;
    trails << ui->trail_1 << ui->trail_2 << ui->trail_3 << ui->trail_4 << ui->trail_5 \
             << ui->trail_6;
    MenuConfig.dispMenu.echoTrail = val;
    CheckButton(MenuConfig.dispMenu.echoTrail, trails);
    if(pView)
       pView->setTrailTime(val);
    setChangedFlag();
    }
}

// 向量长度
void DisplayMenu::veclen_clicked(int val)
{
    if(MenuConfig.dispMenu.vecterLength != val){
    QList<QAbstractButton*> veclens;
    veclens << ui->veclen_1 << ui->veclen_2 << ui->veclen_3 << ui->veclen_4 << ui->veclen_5 << ui->veclen_6;

    MenuConfig.dispMenu.vecterLength = val;
    CheckButton(MenuConfig.dispMenu.vecterLength, veclens);
    setChangedFlag();
    }
}

// 航迹时间
void DisplayMenu::track_clicked(int val)
{
    if(MenuConfig.dispMenu.trackTime != val){
    QList<QAbstractButton*> tracks;
    tracks << ui->track_1 << ui->track_2 << ui->track_3 << ui->track_4 << ui->track_5 << ui->track_6;

    MenuConfig.dispMenu.trackTime = val;
    CheckButton(MenuConfig.dispMenu.trackTime, tracks);
    setChangedFlag();
    }
}

// 符号大小
void DisplayMenu::symbSize_clicked(int val)
{
    if(MenuConfig.dispMenu.symbolSize != val){
    MenuConfig.dispMenu.symbolSize = val;
    CheckButton3(MenuConfig.dispMenu.symbolSize, ui->symbsize_1, ui->symbsize_2, ui->symbsize_3);
    setChangedFlag();
    }
}

// 颜色选择
void DisplayMenu::color_clicked(int val)
{
    //qDebug()<<"color seclect:"<<val;
    if(MenuConfig.dispMenu.colorSelect != val){
    QList<QAbstractButton*> colors;
    colors << ui->color_1 << ui->color_2 << ui->color_3;

    MenuConfig.dispMenu.colorSelect = val;
    CheckButton(MenuConfig.dispMenu.colorSelect, colors);
    if(lpMainWindow) {
       lpMainWindow->setColorMode(val);
       lpMainWindow->showChangedDispMenu();
       }
    setChangedFlag();
    }
}

// AIS显示
void DisplayMenu::aisdisplay_clicked(int val)
{
    if(MenuConfig.dispMenu.showAis != val){
    MenuConfig.dispMenu.showAis = val;
    CheckButton2(MenuConfig.dispMenu.showAis, ui->aisdisplay_1, ui->aisdisplay_2);
    setChangedFlag();
    lpMainWindow->updateCtrlPanel(VIEW_UPDATE_AIS_BIT);
    }
}

// ARPA显示
void DisplayMenu::arpadisplay_clicked(int val)
{
    if(MenuConfig.dispMenu.showArpa != val){
    MenuConfig.dispMenu.showArpa = val;
    CheckButton2(MenuConfig.dispMenu.showArpa, ui->arpadisplay_1, ui->arpadisplay_2);
    setChangedFlag();
    lpMainWindow->updateCtrlPanel(VIEW_UPDATE_ATA_BIT);
    }
}

// 亮度调节
void DisplayMenu::bright_valueChanged(int index)
{
        QProgressBar* bars[] = {ui->screenBright, ui->kbdBright, ui->lineBright1, ui->lineBright2};
        if(index >= 0 && index < 4)
        {
                const int value = bars[index]->value();
                //lpMainView->setBright(index, value);
                //SendBright(index, value);

                //if(0 == index && g_lpDialog)
                        //g_lpDialog->updateBright(value);

                //if(index > 1)
                        //lpMainView->update();
        }
}
/*************************************************
* class  : InstallMenu
* author : wangmingxiao
* date   : 2015-8-18
*************************************************/
InstallMenu::InstallMenu(QWidget *parent)
    :MenuDlg(parent),ui(new Ui::InstallMenu)
{
    ui->setupUi(this);

    setWindowTitle(tr("安装配置"));

    QList<QAbstractButton*> ranges;
    ranges << ui->checkBox_0 << ui->checkBox_1 << ui->checkBox_2 << ui->checkBox_3 << ui->checkBox_4 << ui->checkBox_5 \
            << ui->checkBox_6 << ui->checkBox_7 << ui->checkBox_8 << ui->checkBox_9 << ui->checkBox_10 << ui->checkBox_11 \
            << ui->checkBox_12 << ui->checkBox_13 << ui->checkBox_14 << ui->checkBox_15 << ui->checkBox_16 << ui->checkBox_17 << ui->checkBox_18;

    ui->aziAdjust->setValidator(new QDoubleValidator(-360, 360, 1, ui->aziAdjust));
    ui->rngAdjust->setValidator(new QDoubleValidator(-1, 1, 4, ui->rngAdjust));

    //ui->tuneValue->setEnabled(false); //滑动条不使用


    connect(ui->toolButton, SIGNAL(clicked(bool)), this, SLOT(toHideMe()));

    SignalMap4(clicked(bool), DataOutput_clicked, ui->DataOutput_1,ui->DataOutput_2,ui->DataOutput_3,ui->DataOutput_4);
    SignalMap2(clicked(bool), langSelect_clicked, ui->langSelect_1,ui->langSelect_2);
    SignalMap(stateChanged(int), range_changed, ranges);
  //  SignalMap2(clicked(bool), band_changed, ui->closeBand, ui->openBand);
    SignalMap2(clicked(bool), HL1710_clicked, ui->HL1710_1, ui->HL1710_2);
     SignalMap2(clicked(bool), aziAdjust_Changed, ui->aziAdjust_1, ui->aziAdjust_2);
     SignalMap2(clicked(bool), rngAdjust_Changed, ui->rngAdjust_1, ui->rngAdjust_2);
     SignalMap2(clicked(bool), mbsAdjust_Changed, ui->MBS_1, ui->MBS_2);

}

InstallMenu::~InstallMenu()
{
    delete ui;
}


void InstallMenu::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void InstallMenu::createOperation()
{
    //按顺序添加
    addObject(ui->toolButton);
    addObject2(ui->DataOutput_1, ui->DataOutput_2);
    addObject2(ui->DataOutput_3, ui->DataOutput_4);

    //方位距离调节
    addObject2(ui->aziAdjust_1, ui->aziAdjust_2);
    addObject2(ui->rngAdjust_1, ui->rngAdjust_2);
    addObject2(ui->MBS_1, ui->MBS_2);

    addObject2(ui->langSelect_1, ui->langSelect_2);
    addObject(ui->dateTimeEdit);
    addObject(ui->serialConfig);
    addObject(ui->systemCheck);
    //addObject2(ui->tuneMan, ui->tuneValue);

    addObject(ui->timeTrans);
    addObject(ui->timeUsed);

    //量程选择
    QPoint pos =  ui->checkBox_0->pos();
    pos = this->mapToGlobal(pos);
    QRect rect = QRect(pos, ui->checkBox_0->size());
    addWidget(ui->checkBox_0, rect);

    pos = this->mapToGlobal(ui->checkBox_1->pos());
    rect = QRect(pos/*+QPoint(12,12)*/, ui->checkBox_1->size());
    addWidget(ui->checkBox_1, rect);

    pos = this->mapToGlobal(ui->checkBox_2->pos());
    rect = QRect(pos/*+QPoint(12,12)*/, ui->checkBox_2->size());
    addWidget(ui->checkBox_2, rect);

    addObject3(ui->checkBox_3, ui->checkBox_4, ui->checkBox_5);
    addObject6(ui->checkBox_6, ui->checkBox_7, ui->checkBox_8, ui->checkBox_9, ui->checkBox_10, ui->checkBox_11);
    addObject6(ui->checkBox_12, ui->checkBox_13, ui->checkBox_14, ui->checkBox_15, ui->checkBox_16, ui->checkBox_17);
    addObject(ui->checkBox_18);

  //  addObject2(ui->openBand, ui->closeBand);
    addObject2(ui->HL1710_1, ui->HL1710_2);

    addObject(ui->clearAlarm);
}

void InstallMenu::updateDisplay()
{
    ui->DataOutput_1->setChecked(MenuConfig.installMenu.dataOut & 0x01);
    ui->DataOutput_2->setChecked(MenuConfig.installMenu.dataOut & 0x02);
    ui->DataOutput_3->setChecked(MenuConfig.installMenu.dataOut & 0x04);
    ui->DataOutput_4->setChecked(MenuConfig.installMenu.dataOut & 0x08);

    CheckButton2(MenuConfig.installMenu.langSelect, ui->langSelect_1, ui->langSelect_2);
   // CheckButton1(MenuConfig.installMenu.tuneMan, ui->tuneMan);

    SetText(ui->aziAdjust, MenuConfig.installMenu.aziAdjust);
    SetText(ui->rngAdjust, MenuConfig.installMenu.rngAdjust);
    SetText(ui->MBS, MenuConfig.installMenu.mbsAdjust);
    ui->dateTimeEdit->setDateTime(QDateTime::fromTime_t(MenuConfig.installMenu.dateTime));
    SetTextMin(ui->timeUsed, (int)(MenuConfig.installMenu.timeUsed / 60));
    SetTextMin(ui->timeTrans, (int)(MenuConfig.installMenu.timeTran / 60));

    //显示量程选择
    quint32 range = MenuConfig.installMenu.rangeChecked;
    QList<QAbstractButton*> ranges;
    ranges << ui->checkBox_0 << ui->checkBox_1 << ui->checkBox_2 << ui->checkBox_3 << ui->checkBox_4 << ui->checkBox_5 \
            << ui->checkBox_6 << ui->checkBox_7 << ui->checkBox_8 << ui->checkBox_9 << ui->checkBox_10 << ui->checkBox_11 \
            << ui->checkBox_12 << ui->checkBox_13 << ui->checkBox_14 << ui->checkBox_15 << ui->checkBox_16 << ui->checkBox_17 << ui->checkBox_18;

    for(int i=0; i<19; i++) {
        if((range & ((quint32)1 << i))) {  //需要显示
            ranges[i]->setChecked(true);
        }else {
            ranges[i]->setChecked(false);
        }
    }
   // CheckButton2(MenuConfig.installMenu.bandSelect, ui->closeBand, ui->openBand);
    CheckButton2(MenuConfig.installMenu.antennaSelect, ui->HL1710_1, ui->HL1710_2);

}

void InstallMenu::DataOutput_clicked(int val)
{
    // switch state
    if(MenuConfig.installMenu.dataOut & (1<<val))
        MenuConfig.installMenu.dataOut &= ~(1 << val);
    else
        MenuConfig.installMenu.dataOut |= (1 << val);

    QToolButton* btns[] = {ui->DataOutput_1,ui->DataOutput_2,ui->DataOutput_3,ui->DataOutput_4};
    if(val < 4)
    {
        btns[val]->setChecked(MenuConfig.installMenu.dataOut & (1<<val));
        setChangedFlag();
    }
}

void InstallMenu::langSelect_clicked(int val)
{
    //0:中文   1：英文
    if(MenuConfig.installMenu.langSelect != val){
    MenuConfig.installMenu.langSelect = val;
    CheckButton2(MenuConfig.installMenu.langSelect, ui->langSelect_1, ui->langSelect_2);
    lpMainWindow->toSetLanguage(MenuConfig.installMenu.langSelect);
    setChangedFlag();
    }
}
void InstallMenu::range_changed(int val)
{
    //量程选择，从0-18
    quint32 range = MenuConfig.installMenu.rangeChecked;
    QList<QAbstractButton*> ranges;
    ranges << ui->checkBox_0 << ui->checkBox_1 << ui->checkBox_2 << ui->checkBox_3 << ui->checkBox_4 << ui->checkBox_5 \
            << ui->checkBox_6 << ui->checkBox_7 << ui->checkBox_8 << ui->checkBox_9 << ui->checkBox_10 << ui->checkBox_11 \
            << ui->checkBox_12 << ui->checkBox_13 << ui->checkBox_14 << ui->checkBox_15 << ui->checkBox_16 << ui->checkBox_17 << ui->checkBox_18;

    if(ranges[val]->isChecked()) {  //选中
        range = (range | ((quint32)1 << val));  //将相应位置1
    }else{
        range = (range & (~((quint32)1 << val)));  //将相应位置0
    }
    MenuConfig.installMenu.rangeChecked = range;
    setChangedFlag();

   // qDebug()<<"range check changed"<< val;

}
void InstallMenu::band_changed(int val)
{
    //1开启，0关闭
    if(MenuConfig.installMenu.bandSelect != val) {
        MenuConfig.installMenu.bandSelect = (quint8)val;
      //  CheckButton2(MenuConfig.installMenu.bandSelect, ui->closeBand, ui->openBand);
        extern void saveComputerInfo(quint8 flag);
        saveComputerInfo(val);
        qDebug()<<"computer info number"<<val;
        setChangedFlag();
    }

}
void InstallMenu::HL1710_clicked(int val)
{
    if(val) {
        MenuConfig.installMenu.antennaSelect = 1;
        pView->change_HL1710(1);
        CheckButton2(MenuConfig.installMenu.antennaSelect, ui->HL1710_1, ui->HL1710_2);
        setChangedFlag();
    }else {
        MenuConfig.installMenu.antennaSelect = 0;
        pView->change_HL1710(0);
        setChangedFlag();
        CheckButton2(MenuConfig.installMenu.antennaSelect, ui->HL1710_1, ui->HL1710_2);
    }

}

/*#define ShowMenuDlg(cb)\
{   \
MainWindow* wnd = qobject_cast<MainWindow*>(parent());\
if(wnd) wnd->cb();\
}*/
void InstallMenu::on_serialConfig_clicked(bool)
{
    MainWindow *win = qobject_cast<MainWindow*>(parent());
    if(win)
        win->showSerialConfigMenu();
}

//系统自检
void InstallMenu::on_systemCheck_clicked(bool)
{

}

//清除报警信息
void InstallMenu::on_clearAlarm_clicked(bool)
{
    QFile file(CURRENTPATH + "/Alarm.dat");
    if(! file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {  //打开会将之前的所有信息丢失
        qDebug()<< "clear the Alarm.dat data!";
        return;
    }

    QTextStream out(&file);
    out << ""; //没有写入数据

    file.close();
}

void InstallMenu::aziAdjust_Changed(int val)
{
    //qDebug() << __FUNCTION__ << text;
    float azi = MenuConfig.installMenu.aziAdjust;
    if(val){
        if(azi < 360)
            azi += 0.5;
    }else{
        if(azi > -360)
            azi -= 0.5;
    }
   // MenuConfig.installMenu.aziAdjust = azi;
    if(pView)
        pView->setAziAdjust(azi);
    ui->aziAdjust->setText(QString::number(azi, 'f', 2));

    setChangedFlag();

}

void InstallMenu::rngAdjust_Changed(int val)
{
    //qDebug() << __FUNCTION__ << text;
    float rng = MenuConfig.installMenu.rngAdjust;
    if(val){
        if(rng < 250)
            rng += 1.0;
    }else{
        if(rng > 0)
            rng -= 1.0;
    }
   // MenuConfig.installMenu.rngAdjust = rng;
    if(pView)
        pView->setRngAdjust(rng);
    ui->rngAdjust->setText(QString::number(rng, 'f', 0));
    setChangedFlag();
}
void InstallMenu::mbsAdjust_Changed(int val)
{
    quint8 mbs = MenuConfig.installMenu.mbsAdjust;
    if(val) {
        if(mbs < 250)
            mbs += 5;
    }
    else {
        if(mbs > 0)
            mbs -= 5;
    }
    MenuConfig.installMenu.mbsAdjust = mbs;
    if(pView)
        pView->setMBSAdjust(mbs);

    ui->MBS->setText(QString::number(mbs, 'f', 0));
    setChangedFlag();
}


void InstallMenu::on_dateTimeEdit_dateTimeChanged(const QDateTime & datetime)
{
    if(datetime.isValid()){
        const quint32 tm = datetime.toTime_t();
        if(tm != MenuConfig.installMenu.dateTime){
            MenuConfig.installMenu.dateTime = tm;
            setChangedFlag();
            //设置系统的时间
            if(lpMainWindow) {
                QDate date = datetime.date();
                QTime time = datetime.time();

                //年月日时分秒
                lpMainWindow->setSystemTime(date.year(),date.month(),date.day(),time.hour(),time.minute(),time.second());

            }
        }
    }
}

void InstallMenu::on_tuneMan_clicked(bool checked)
{
    if(0 == MenuConfig.installMenu.tuneMan)  //实际上是自动调谐
    {
        MenuConfig.installMenu.tuneMan = 1;
        //CheckButton1(MenuConfig.installMenu.tuneMan, ui->tuneMan);
       // ui->tuneValue->setEnabled(MenuConfig.installMenu.tuneMan);
        if(pView)
        pView->change_tune(MenuConfig.installMenu.tuneMan, 65500);

    }else {
        MenuConfig.installMenu.tuneMan = 0;
        //CheckButton1(MenuConfig.installMenu.tuneMan, ui->tuneMan);
        //关闭自动调谐
    }
}
//没有使用
void InstallMenu::tuneValue_valueChanged(int value)
{
    //qDebug() << __FUNCTION__ << value;
    if(MenuConfig.installMenu.tuneMan)
    {
        MenuConfig.installMenu.tuneValue = value;
        pView->change_tune(MenuConfig.installMenu.tuneMan, MenuConfig.installMenu.tuneValue);

    }
}

//更改使用时间发射时间
void InstallMenu::on_timeTrans_textChanged(const QString &text)
{
    quint32 timetrans=0;
    if(flag_editChanged) {
        flag_editChanged = 0;
        FloatFromText(timetrans, text, 0, 1024);  //最大到2的10次方
        MenuConfig.installMenu.timeTran = timetrans;
    }


}
void InstallMenu::on_timeUsed_textChanged(const QString &text)
{
    quint32 timeused=0;
    if(flag_editChanged) {
        flag_editChanged = 0;
        FloatFromText(timeused, text, 0, 1024);  //最大到2的10次方
        MenuConfig.installMenu.timeUsed = timeused;
     }
}



/*************************************************
* class  : OtherMenu
* author : wangmingxiao
* date   : 2015-8-19
*************************************************/
OtherMenu::OtherMenu(QWidget *parent)
    :MenuDlg(parent),ui(new Ui::OtherMenu)
{
    ui->setupUi(this);

    setWindowTitle(tr("其它配置"));
    navi_btn = 1;  //默认删除
    alarm_btn = 1;
    record_btn  = 0;


    ui->plotLimit->setRange(0, 60);
    connect(ui->toolButton, SIGNAL(clicked(bool)), this, SLOT(toHideMe()));

    ui->vrm_1->setValidator(new QDoubleValidator(0, 50, 3, ui->vrm_1));
    ui->vrm_2->setValidator(new QDoubleValidator(0, 360, 0, ui->vrm_2));
    ui->vrm_3->setValidator(new QDoubleValidator(0, 50, 3, ui->vrm_3));
    ui->vrm_4->setValidator(new QDoubleValidator(0, 360, 0, ui->vrm_4));

    //信号和槽的连接设置
    SignalMap3(clicked(bool), course_clicked, ui->course_1,ui->course_2,ui->course_3);
    SignalMap3(clicked(bool), speed_clicked, ui->speed_1,ui->speed_2,ui->speed_3);
    SignalMap2(clicked(bool), transmit_clicked, ui->transmit_1,ui->transmit_2);
    SignalMap2(clicked(bool), xinneng_clicked, ui->xinneng_1,ui->xinneng_2);
    SignalMap3(clicked(bool), jam_clicked, ui->jam_1,ui->jam_2, ui->jam_3);
    SignalMap4(clicked(bool), vrm_clicked, ui->vr_enable_1,ui->vr_enable_2,ui->vr_enable_3,ui->vr_enable_4);
    connect(ui->plotLimit, SIGNAL(valueChanged(int)), this, SLOT(plotLimit_valueChanged(int)));
    SignalMap2(clicked(bool), audibleWarning_clicked, ui->audibleWarning_1,ui->audibleWarning_2);
    SignalMap2(clicked(bool), guardZone_clicked, ui->guardZoneEn_1,ui->guardZoneEn_2);
  //  SignalMap2(clicked(bool), guardSelect_clicked, ui->guardZoneEn_3, ui->guardZoneEn_4);  //警戒区域手动自动
    SignalMap3(clicked(bool), echoExpand_Changed, ui->echoExpand_1, ui->echoExpand_2, ui->echoExpand_3);
    SignalMap2(clicked(bool), tuneValue_valueChanged, ui->tuneValue_dec, ui->tuneValue_add);  //调谐值变化
    SignalMap2(clicked(bool), alarm_changed, ui->openAlarm, ui->closeAlarm);  //报警显示

}

OtherMenu::~OtherMenu()
{
    delete ui;
}

void OtherMenu::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void OtherMenu::createOperation()
{
    //按菜单顺序来添加
    addObject(ui->toolButton);
    addObject3(ui->course_1, ui->course_2, ui->course_3);
    addObject3(ui->speed_1, ui->speed_2, ui->speed_3);
    addObject3(ui->jam_1, ui->jam_2, ui->jam_3);
    addObject2(ui->transmit_1, ui->transmit_2);
    addObject2(ui->xinneng_1, ui->xinneng_2);
    addObject2(ui->vr_enable_1, ui->vrm_1);
    addObject2(ui->vr_enable_2, ui->vrm_2);
    addObject2(ui->vr_enable_3, ui->vrm_3);
    addObject2(ui->vr_enable_4, ui->vrm_4);

    addObject3(ui->echoExpand_1, ui->echoExpand_2, ui->echoExpand_3);
    addObject2(ui->audibleWarning_1, ui->audibleWarning_2);
    addObject2(ui->tuneAuto, ui->tuneMan);
    //调谐值变化
    addObject2(ui->tuneValue_dec, ui->tuneValue_add);

    //cpa   tcpa
    addObject2(ui->cpaBtn, ui->cpaEdit);
    addObject2(ui->tcpaBtn, ui->tcpaEdit);
    //警戒区域
    addObject2(ui->guardZoneEn_1, ui->guardZoneEn_2);
  //  addObject2(ui->guardZoneEn_3, ui->guardZoneEn_4);

    //航路点
    addObject2(ui->naviPoint_1, ui->naviPoint_2);
    addObject3(ui->naviPointPos_1, ui->naviPointPos_2, ui->naviPointPos_3);

    //报警信息
    addObject2(ui->openAlarm, ui->closeAlarm);


}


void OtherMenu::updateDisplay()
{
    CheckButton3(MenuConfig.otherMenu.corseSelect, ui->course_1,ui->course_2,ui->course_3);
    CheckButton3(MenuConfig.otherMenu.speedSelect, ui->speed_1,ui->speed_2,ui->speed_3);
    CheckButton2(MenuConfig.otherMenu.transmite, ui->transmit_1,ui->transmit_2);
    CheckButton2(gXinnengMonitor, ui->xinneng_1, ui->xinneng_2);
    CheckButton3(MenuConfig.otherMenu.samefreqJam, ui->jam_1,ui->jam_2, ui->jam_3);
    CheckButton2(MenuConfig.otherMenu.audibleWarningEnable, ui->audibleWarning_1,ui->audibleWarning_2);
    CheckButton2(MenuConfig.otherMenu.guardZoneEnable, ui->guardZoneEn_1,ui->guardZoneEn_2);
//    CheckButton2(MenuConfig.otherMenu.guardSelect, ui->guardZoneEn_3,ui->guardZoneEn_4);
    CheckButton3(MenuConfig.otherMenu.echoExpand, ui->echoExpand_1, ui->echoExpand_2, ui->echoExpand_3);
    CheckButton2(navi_btn, ui->naviPoint_1, ui->naviPoint_2);
  //  CheckButton2(alarm_btn, ui->openAlarm, ui->closeAlarm);
    CheckButton2(MenuConfig.otherMenu.tuneMan, ui->tuneMan, ui->tuneAuto);
    MenuConfig.otherMenu.tuneMan ? ui->tuneValue->setEnabled(false) : ui->tuneValue->setEnabled(true);

    ui->cpaBtn->setText(MenuConfig.otherMenu.dcpaEnable? tr("启用") : tr("禁止"));
    ui->cpaBtn->setChecked(MenuConfig.otherMenu.dcpaEnable);
    ui->tcpaBtn->setText(MenuConfig.otherMenu.tcpaEnable? tr("启用") : tr("禁止"));
    ui->tcpaBtn->setChecked(MenuConfig.otherMenu.tcpaEnable);
    ui->cpaEdit->setText(QString::number(MenuConfig.otherMenu.dcpaValue,'f',3));
    ui->tcpaEdit->setText(QString::number(MenuConfig.otherMenu.tcpaValue));

    ui->guardZone_0->setText(QString::number(MenuConfig.otherMenu.guardZoneValue[0],'f',3)+QString(" Nm"));
    ui->guardZone_1->setText(QString::number(MenuConfig.otherMenu.guardZoneValue[1],'f',3)+QString("°"));
    ui->guardZone_2->setText(QString::number(MenuConfig.otherMenu.guardZoneValue[2],'f',3)+QString(" Nm"));
    ui->guardZone_3->setText(QString::number(MenuConfig.otherMenu.guardZoneValue[3],'f',3)+QString("°"));

    ui->vr_enable_1->setChecked(MenuConfig.otherMenu.vrmeblEnable & 0x01);
    ui->vr_enable_2->setChecked(MenuConfig.otherMenu.vrmeblEnable & 0x02);
    ui->vr_enable_3->setChecked(MenuConfig.otherMenu.vrmeblEnable & 0x04);
    ui->vr_enable_4->setChecked(MenuConfig.otherMenu.vrmeblEnable & 0x08);

    ui->vrm_1->setText(QString::number(MenuConfig.otherMenu.vrmebl[0],'f',3)+QString(" Nm"));
    ui->vrm_2->setText(QString::number(MenuConfig.otherMenu.vrmebl[1],'f',1)+QString("°"));
    ui->vrm_3->setText(QString::number(MenuConfig.otherMenu.vrmebl[2],'f',3)+QString(" Nm"));
    ui->vrm_4->setText(QString::number(MenuConfig.otherMenu.vrmebl[3],'f',1)+QString("°"));

    ui->tuneValue->setText(QString::number(MenuConfig.otherMenu.tuneValue,'f',0));

    updateNaviPointDisplay(0x0F);
}

void OtherMenu::updateNaviPointDisplay(quint8 index)
{
#define IsLatitudeValid(ld) (qAbs((ld).x()) < 180 && qAbs((ld).y())<90)
#define DispLatitude(label, ld) {	\
        QString strlon("---°--.---'-"), strlat("--°--.---'-");	\
        if(IsLatitudeValid(ld)){		\
        strlon = Longitude2String(ld.x()*180.0/M_PI, 1);	\
        strlat = Latitude2String(ld.y()*180.0/M_PI, 1);	}\
        label->setText(strlat+"     "+strlon);	\
        }
        //显示导航点
        if(index & 0x01)
                DispLatitude(ui->naviPointPos_1, pView->getNaviPoint(0));
        if(index & 0x02)
                DispLatitude(ui->naviPointPos_2, pView->getNaviPoint(1));
        if(index & 0x04)
                DispLatitude(ui->naviPointPos_3, pView->getNaviPoint(2));
        if(index & 0x08)
                DispLatitude(ui->naviPointPos_4, pView->getNaviPoint(3));
}

void OtherMenu::course_clicked(int val)
{
    if(MenuConfig.otherMenu.corseSelect != val){
    MenuConfig.otherMenu.corseSelect = val;
    CheckButton3(MenuConfig.otherMenu.corseSelect, ui->course_1,ui->course_2,ui->course_3);
    setChangedFlag();
  /*  if(lpOwnShipMonitor && val == 2)
                lpOwnShipMonitor->setCourse(MenuConfig.otherMenu.manCorse, 2);
                g_lpDialog->updateBoatInfoDisplay();*/
    }
}

void OtherMenu::speed_clicked(int val)
{
    if(MenuConfig.otherMenu.speedSelect != val){
    MenuConfig.otherMenu.speedSelect = val;
    CheckButton3(MenuConfig.otherMenu.speedSelect, ui->speed_1,ui->speed_2,ui->speed_3);
    setChangedFlag();
  /*  if(lpOwnShipMonitor && val == 2)
                lpOwnShipMonitor->setGroupSpeed(MenuConfig.otherMenu.manSpeed, 2);

                g_lpDialog->updateBoatInfoDisplay();*/
    }
}

void OtherMenu::transmit_clicked(int val)
{
     // 如果不允许开发射，则不处理
    //if(!PermitTransmit())	return;

    if(MenuConfig.otherMenu.transmite != val){
    //MenuConfig.otherMenu.transmite = val;
    CheckButton2(val, ui->transmit_1,ui->transmit_2);
    setChangedFlag();
    // 设置发射开关
    qDebug() << __FUNCTION__;
    lpInteract->BtnOpenClose();
    }
}

void OtherMenu::echoExpand_Changed(int val)
{
    //0禁止1模式12模式2
    if(MenuConfig.otherMenu.echoExpand != val) {
        MenuConfig.otherMenu.echoExpand = val;
        CheckButton3(val, ui->echoExpand_1, ui->echoExpand_2, ui->echoExpand_3);
        if(pView)
            pView->echoExpand(val);
        lpMainWindow->updateCtrlPanel(VIEW_UPDATE_AUTO_BIT);
        setChangedFlag();
    }
}

void OtherMenu::xinneng_clicked(int val)
{
    if(val != gXinnengMonitor)
    {
        gXinnengMonitor = val;
        CheckButton2(gXinnengMonitor, ui->xinneng_1, ui->xinneng_2);

        if(pView)
            pView->change_Xingneng(gXinnengMonitor);
    }
}

void OtherMenu::jam_clicked(int val)
{
    //0禁止1模式12模式2
   // qDebug() << __FUNCTION__ << val;
    if(MenuConfig.otherMenu.samefreqJam != val){
    MenuConfig.otherMenu.samefreqJam = val;
    CheckButton3(val, ui->jam_1,ui->jam_2, ui->jam_3);
    lpMainWindow->updateCtrlPanel(VIEW_UPDATE_JAM_BIT);
    setChangedFlag();
    if(pView);
        pView->setJam(val);

    }
}

void OtherMenu::vrm_clicked(int val)
{
    if(val < 4)
    {
        // switch state
        if(MenuConfig.otherMenu.vrmeblEnable & (1<<val))
            MenuConfig.otherMenu.vrmeblEnable &= ~(1 << val);
        else
            MenuConfig.otherMenu.vrmeblEnable |= (1 << val);  //相应的标志置位
        qDebug() << "MenuConfig.otherMenu.vrmeblEnable:" << MenuConfig.otherMenu.vrmeblEnable;

        QToolButton* btns[] = {ui->vr_enable_1,ui->vr_enable_2,ui->vr_enable_3,ui->vr_enable_4};
        btns[val]->setChecked(MenuConfig.otherMenu.vrmeblEnable & (1<<val));

        SystemInfo.VrmEblCtrl.vrmebl[0].show = (MenuConfig.otherMenu.vrmeblEnable & 0x03);  //0:vrm1   1:ebl1
        SystemInfo.VrmEblCtrl.vrmebl[1].show = ((MenuConfig.otherMenu.vrmeblEnable>>2) & 0x03);
        setChangedFlag();
        if(changedFlag() && pView)
            pView->update();
        if(lpMainWindow) lpMainWindow->updateVrmeblDisplay();

    }
}
//报警信息的显示与关闭
void OtherMenu::alarm_changed(int value)
{
    //0:显示1关闭报警声音
        alarm_btn = value;
        if(!alarm_btn) {
            QFile file(CURRENTPATH+ "/Alarm.dat");
            if(! file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qDebug()<< "can't open color.txt for read.";
                return;
            }
            QTextStream in(&file);
            QString text = in.readAll();

            file.close();

            MainWindow *win = qobject_cast<MainWindow*>(parent());
            if(win) {
                win->showAlarmDisplayMenu();
                win->setAlarmDisplayText(text);
            }


        }else {

            extern quint8 guardZone_alarm;
            guardZone_alarm = 0;
            //消除警戒区域报警声
            extern quint8 closeSpeaking;
            closeSpeaking = 1;

        }


}


// 设置点迹门限
void OtherMenu::plotLimit_valueChanged(int value)
{
    // 输入范围有效值为：[0,20] --> 点迹数[600, 2600]
    if(MenuConfig.otherMenu.plotLimit != value)
    {
        MenuConfig.otherMenu.plotLimit = value;
        //SendSignalParamCmd(false);
        setChangedFlag();
    }
}

// VRM1
void OtherMenu::on_vrm_1_textChanged(const QString& text)
{
    FloatFromText(MenuConfig.otherMenu.vrmebl[0], text, 0, 100);
    if(changedFlag()) {
        SystemInfo.VrmEblCtrl.vrmebl[0].vrm = MenuConfig.otherMenu.vrmebl[0];
        if(lpMainWindow) lpMainWindow->updateVrmeblDisplay();
    }
}

// EBL1
void OtherMenu::on_vrm_2_textChanged(const QString& text)
{
    FloatFromText(MenuConfig.otherMenu.vrmebl[1], text, 0, 100);
    if(changedFlag()) {
        SystemInfo.VrmEblCtrl.vrmebl[0].ebl = MenuConfig.otherMenu.vrmebl[1];
        if(lpMainWindow) lpMainWindow->updateVrmeblDisplay();
    }
}

// VRM2
void OtherMenu::on_vrm_3_textChanged(const QString& text)
{
    FloatFromText(MenuConfig.otherMenu.vrmebl[2], text, 0, 100);
    if(changedFlag()) {
        SystemInfo.VrmEblCtrl.vrmebl[1].vrm = MenuConfig.otherMenu.vrmebl[2];
        if(lpMainWindow) lpMainWindow->updateVrmeblDisplay();
    }
}

// EBL2
void OtherMenu::on_vrm_4_textChanged(const QString& text)
{
    FloatFromText(MenuConfig.otherMenu.vrmebl[3], text, 0, 100);
    if(changedFlag()) {
        SystemInfo.VrmEblCtrl.vrmebl[1].ebl = MenuConfig.otherMenu.vrmebl[3];
        if(lpMainWindow) lpMainWindow->updateVrmeblDisplay();
    }
}
//调谐操作,手动为0，自动为1
void OtherMenu::on_tuneMan_clicked(bool checked)
{
    MenuConfig.otherMenu.tuneMan = 0;  //为0时表示手动调谐打开
    MenuConfig.otherMenu.tuneAuto = 0;

  //  CheckButton1(1, ui->tuneMan);  CheckButton1(0, ui->tuneAuto);
    CheckButton2(0, ui->tuneMan, ui->tuneAuto);
    ui->tuneValue->setEnabled(true);
    pView->change_tune(MenuConfig.otherMenu.tuneMan, MenuConfig.otherMenu.tuneValue, true);
    setChangedFlag();
}
//自动调谐没有使用，只是选择之后手动不能使用
void OtherMenu::on_tuneAuto_clicked(bool checked)
{
    MenuConfig.otherMenu.tuneMan = 1;
    MenuConfig.otherMenu.tuneAuto = 1;
    //CheckButton1(1, ui->tuneAuto);  CheckButton1(0, ui->tuneMan);
    CheckButton2(1, ui->tuneMan, ui->tuneAuto);
    ui->tuneValue->setEnabled(false);
    pView->change_tune(MenuConfig.otherMenu.tuneAuto, MenuConfig.otherMenu.tuneValue);
    setChangedFlag();
}

void OtherMenu::tuneValue_valueChanged(int value)
{
    //0:减  1：加
    //qDebug() << __FUNCTION__ << value;
    if(MenuConfig.otherMenu.tuneMan == 0)  //手动才能使用
    {
        quint16 num = MenuConfig.otherMenu.tuneValue;
        if(value) {
            if(num < 512)
            num += 1;
        }else {
            if(num > 0)
            num -= 1;
        }
        MenuConfig.otherMenu.tuneValue = num;
        pView->change_tune(MenuConfig.otherMenu.tuneMan, MenuConfig.otherMenu.tuneValue);
        ui->tuneValue->setText(QString::number(MenuConfig.otherMenu.tuneValue,'f',0));
        setChangedFlag();
   }
}
void OtherMenu::setManTuneValue(quint16 val)
{
    ui->tuneValue->setText(QString::number(val, 'f', 0));
}

void OtherMenu::audibleWarning_clicked(int val)
{
     //qDebug() << __FUNCTION__ << val;
    if(MenuConfig.otherMenu.audibleWarningEnable != val){
        MenuConfig.otherMenu.audibleWarningEnable = val;
        if(!val) {
            if(lpAlarm)
                lpAlarm->stopAlarm();
        }
        CheckButton2(MenuConfig.otherMenu.audibleWarningEnable, ui->audibleWarning_1,ui->audibleWarning_2);
        setChangedFlag();
    }
}

void OtherMenu::guardZone_clicked(int val)
{
    //qDebug() << __FUNCTION__ << val;
     if(MenuConfig.otherMenu.guardZoneEnable != val){
         MenuConfig.otherMenu.guardZoneEnable = val;
         CheckButton2(MenuConfig.otherMenu.guardZoneEnable, ui->guardZoneEn_1,ui->guardZoneEn_2);
         setChangedFlag();
         //设置警戒区域都为空
         if(val) {  //打开警戒
             pView->setMouseShape(1);
             lpMouseOpetarion->setMouseProcessID(SELECT_AZI_SECTOR_PROCESS);
         }else{
             extern quint8 guardZone_alarm;
             guardZone_alarm = 0;
             lpAlarm->stopAlarm();
             pView->setGuardZoneText();
             pView->setMouseShape(0);
             lpMouseOpetarion->setMouseProcessID(NULL_PROCESS);
         }
         pView->setGuardZoneText();
         setGuardZoneText(0.0, 0.0, 0.0, 0.0);
         pView->setGuardZoneClear();
         pView->guardzone()->clearPoint();
     }
}
/*
//警戒区域自动手动设置,没用了
void OtherMenu::guardSelect_clicked(int val)
{
    //0手动 1自动，手动：根据右边的输入框画图，自动，鼠标选择两个坐标点
    if(MenuConfig.otherMenu.guardSelect != val) {
        MenuConfig.otherMenu.guardSelect = val;
        CheckButton2(MenuConfig.otherMenu.guardSelect, ui->guardZoneEn_3,ui->guardZoneEn_4);
        if(val) {
            //设置鼠标状态
            pView->setMouseShape(1);
            lpMouseOpetarion->setMouseProcessID(SELECT_AZI_SECTOR_PROCESS);
            setGuardZoneText(0.0, 0.0, 0.0, 0.0);
            pView->setGuardZoneClear();
            pView->guardzone()->clearPoint();

        }else{
            pView->setMouseShape(0);
            lpMouseOpetarion->setMouseProcessID(NULL_PROCESS);
            setGuardZoneText(0.0, 0.0, 0.0, 0.0);
            pView->setGuardZoneClear();
        }
        setChangedFlag();
    }
} */




void OtherMenu::on_cpaBtn_clicked(bool)
{
        //qDebug() << __FUNCTION__;
    MenuConfig.otherMenu.dcpaEnable = (MenuConfig.otherMenu.dcpaEnable ? 0 : 1);
    ui->cpaBtn->setChecked(MenuConfig.otherMenu.dcpaEnable);
    ui->cpaBtn->setText(MenuConfig.otherMenu.dcpaEnable? tr("启用"): tr("禁止"));
    lpAisManage->cpaAlarm();
   // lpTrack->cpaAlarm();
    setChangedFlag();
}

void OtherMenu::on_tcpaBtn_clicked(bool)
{
        //qDebug() << __FUNCTION__;
    MenuConfig.otherMenu.tcpaEnable = (MenuConfig.otherMenu.tcpaEnable ? 0 : 1);
    ui->tcpaBtn->setChecked(MenuConfig.otherMenu.tcpaEnable);
    ui->tcpaBtn->setText(MenuConfig.otherMenu.tcpaEnable? tr("启用") : tr("禁止"));
    lpAisManage->cpaAlarm();
   // lpTrack->cpaAlarm();
    setChangedFlag();
}

void OtherMenu::on_cpaEdit_textChanged(const QString& text)
{
        //qDebug() << __FUNCTION__ << text;
    FloatFromText(MenuConfig.otherMenu.dcpaValue, text, 0, 10);
    if(changedFlag()) {  //需要按键触发后才能使用
    lpAisManage->cpaAlarm();
    //lpTrack->cpaAlarm();
    }
}

void OtherMenu::on_tcpaEdit_textChanged(const QString& text)
{
        //qDebug() << __FUNCTION__ << text;
    FloatFromText(MenuConfig.otherMenu.tcpaValue, text, 0, 100);
    if(changedFlag()) {
    lpAisManage->cpaAlarm();
   // lpTrack->cpaAlarm();
    }
}

void OtherMenu::on_guardZone_0_textChanged(const QString& text)
{
   //qDebug() << __FUNCTION__ << text;
    FloatFromText(MenuConfig.otherMenu.guardZoneValue[0], text, 0, 50);
    if(changedFlag()) {
    }
}

void OtherMenu::on_guardZone_1_textChanged(const QString& text)
{
    //qDebug() << __FUNCTION__ << text;
    FloatFromText(MenuConfig.otherMenu.guardZoneValue[1], text, 0, 360);
    if(changedFlag()) {
    }
}

void OtherMenu::on_guardZone_2_textChanged(const QString& text)
{
    //qDebug() << __FUNCTION__ << text;
    FloatFromText(MenuConfig.otherMenu.guardZoneValue[2], text, 0, 50);
    if(changedFlag()) {
    }
}

void OtherMenu::on_guardZone_3_textChanged(const QString& text)
{
    //qDebug() << __FUNCTION__ << text;
    FloatFromText(MenuConfig.otherMenu.guardZoneValue[3], text, 0, 360);
    if(changedFlag()) {
    }
}

void OtherMenu::setGuardZoneText(float z0, float z1, float z2, float z3)
{
   ui->guardZone_0->setText(QString("%1").arg(z0));
   MenuConfig.otherMenu.guardZoneValue[0] = z0;
   ui->guardZone_1->setText(QString("%1").arg(z1));
   MenuConfig.otherMenu.guardZoneValue[1] = z1;
   ui->guardZone_2->setText(QString("%1").arg(z2));
   MenuConfig.otherMenu.guardZoneValue[2] = z2;
   ui->guardZone_3->setText(QString("%1").arg(z3));
   MenuConfig.otherMenu.guardZoneValue[3] = z3;
}

void OtherMenu::on_naviPoint_1_clicked(bool)
{
    lpMouseOpetarion->setMouseProcessID(SET_NAVI_POINT);
    ui->naviPoint_1->setChecked(lpMouseOpetarion->mouseProcessID() == SET_NAVI_POINT);
    navi_btn = 0;
    CheckButton2(navi_btn, ui->naviPoint_1, ui->naviPoint_2);
    pView->resetNaviPointIndex();
}

void OtherMenu::on_naviPoint_2_clicked(bool)
{

    lpMouseOpetarion->setMouseProcessID(DEL_NAVI_POINT);
    ui->naviPoint_2->setChecked(lpMouseOpetarion->mouseProcessID() == DEL_NAVI_POINT);
    navi_btn = 1;
    CheckButton2(navi_btn, ui->naviPoint_1, ui->naviPoint_2);
}

void OtherMenu::on_saveRecord_clicked(bool)
{
    if(lpMainWindow)
        lpMainWindow->switchRecord();
}
void OtherMenu::on_replayRecord_clicked(bool )
{
    record_btn = record_btn ? 0 : 1;
    if(MenuConfig.otherMenu.transmite && !isReplaying)  //发射状态先关闭
        lpInteract->BtnOpenClose();
    if(lpMainWindow)
        lpMainWindow->showReplayDlg(record_btn);
}

/*************************************************
* class  : SerialConfigMenu
* author : wangmingxiao
* date   : 2015-8-18
*************************************************/
SerialConfigMenu::SerialConfigMenu(QWidget *parent)
    :MenuDlg(parent),ui(new Ui::SerialConfigMenu)
{
    ui->setupUi(this);
    setWindowTitle(tr("串口配置菜单"));

    SignalMap4(clicked(bool), SerialConfig_clicked, ui->toolButton_1,ui->toolButton_2,ui->toolButton_3,ui->toolButton_4);
    connect(ui->closeButton, SIGNAL(clicked(bool)), this, SLOT(toHideMe()));
}

SerialConfigMenu::~SerialConfigMenu()
{
    delete ui;
}

void SerialConfigMenu::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void SerialConfigMenu::updateDisplay()
{
    CheckButton4(MenuConfig.SerialConfigMenu.SerialConfig, ui->toolButton_1,ui->toolButton_2,ui->toolButton_3,ui->toolButton_4);
    m_name = QString("COM%1").arg(MenuConfig.SerialConfigMenu.SerialConfig+1);
    updateSerialConfig(m_name);
}

void SerialConfigMenu::SerialConfig_clicked(int val)
{
    if(MenuConfig.SerialConfigMenu.SerialConfig!= val)
    {
    MenuConfig.SerialConfigMenu.SerialConfig = val;//状态有改变时更新为当前的状态
    //根据当前的状态  设置toolbutton的状态
    CheckButton4(MenuConfig.SerialConfigMenu.SerialConfig, ui->toolButton_1,ui->toolButton_2,ui->toolButton_3,ui->toolButton_4);
    m_name = QString("COM%1").arg(val+1);
    //updateSerialConfig(QString("COM%1").arg(val+1));//函数的形参 只是外部向函数传进来的参数
    updateSerialConfig(m_name);
    setChangedFlag();
    }
}

void SerialConfigMenu::updateSerialConfig(const QString& name)
{
  /* SERIALPARAM para;
   lpInteract->getNameSerialPara(name,para);
   ui->baudrate->setCurrentIndex(ui->baudrate->findText(QString::number(para.baudrate)));
   ui->databit->setCurrentIndex(ui->databit->findText(QString::number(para.databit)));

   if(para.stopbit == 3)
       m_stopbit = 1.5;
   else m_stopbit = para.stopbit;
   ui->stopbit->setCurrentIndex(ui->stopbit->findText(QString::number(m_stopbit)));

   ui->parity->setCurrentIndex(para.parity);*/

}

void SerialConfigMenu::on_saveButton_clicked()
{
 /* SERIALPARAM para;

  QString baudrate = ui->baudrate->currentText();
  para.baudrate = baudrate.toUShort();

  QString databit = ui->databit->currentText();
  para.databit = (quint8)databit.toUShort();

  QString stopbit = ui->stopbit->currentText();
  if(stopbit.toFloat() == 1.5)
      para.stopbit = 3;
  else
     para.stopbit = (quint8)stopbit.toUShort();

  para.parity = ui->parity->currentIndex();
  para.deviceName = m_name;

  lpInteract->setNameSerialPara(m_name, para);*/
}



/***************************************
* class : InputMenu
* author: wangmingxiao
* date  : 2015-9-6
***************************************/
InputMenu::InputMenu(QWidget *parent)
    :MenuDlg(parent),ui(new Ui::InputMenu)
{
    ui->setupUi(this);

    //setWindowFlags(Qt::Tool);
    setWindowTitle(tr("数字输入窗口"));
    setFixedWidth(256);

    m_floatFlag = false;

    QList<QToolButton*> m_buttons;
    m_buttons << ui->input_0 << ui->input_1 << ui->input_2 << ui->input_3 << ui->input_4;
    m_buttons << ui->input_5 << ui->input_6 << ui->input_7 << ui->input_8 << ui->input_9;
    m_buttons << ui->input_10 << ui->input_11 << ui->input_12 << ui->input_13 << ui->input_14;
   // AddGroup(true, NULL, m_buttons);
    SignalMap(clicked(bool), input_clicked, m_buttons);

  /*  LPNODE header = getNodeTree();
    if(header)
    {
        LPNODE first = header->child;
        while(first)
        {
            m_nodeList << first;
            first = first->next;
        }
    }*/
    setCrntIndex(0);
}

InputMenu::~InputMenu()
{
    delete ui;
}

void InputMenu::createOperation()
{
    addObject3(ui->input_0, ui->input_1, ui->input_2);

    addObject3(ui->input_3, ui->input_4, ui->input_5);
    addObject6(ui->input_6, ui->input_7, ui->input_8, ui->input_9, ui->input_10, ui->input_11);
    addObject3(ui->input_12, ui->input_13, ui->input_14);
}

void InputMenu::setPassword(quint8 flag)
{
    if(flag){
         ui->lineEdit->setEchoMode(QLineEdit::Password);
    }else {
        ui->lineEdit->setEchoMode(QLineEdit::Normal);
    }
}

void InputMenu::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    switch (event->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void InputMenu::showEvent(QShowEvent *event)
{
    MenuDlg::showEvent(event);
    if(lpMainWindow)
        lpMainWindow->displayMenu(this, true, false);
}

void InputMenu::hideEvent(QHideEvent *evt)
{
    MenuDlg::hideEvent(evt);
    if(lpMainWindow);
       // lpMainWindow->gobackMenu();
}

void InputMenu::updateDisplay()
{
    m_inputText = "";
    m_floatFlag = false;
    setCrntIndex(0);
    ui->lineEdit->setText(m_inputText);
}

int InputMenu::getDlgResult()
{
    //QMutexLocker lock(&m_mutex);
    //while(!m_mutex.tryLock())
    return result();
}

bool InputMenu::customDlgDirection(int direction)
{
  /*  int di = 0;
    switch(direction)
    {
    case 0: di = -1; break;
    case 1: di = -3; break;
    case 2: di = 1; break;
    case 3: di = 3; break;
    }

    if(di != 0)
    {
        const int max = m_nodeList.size();
        setCrntIndex((m_crntIndex+di+max)%max);
        return true;
    }*/

    return false;
}

void InputMenu::setCrntIndex(int index)
{
  /*  if(index >= 0 && index < m_nodeList.size())
    {
        m_crntIndex = index;
        setCrntNode(m_nodeList[index]);
    }*/
}

void InputMenu::input_clicked(int val)
{
    //qDebug() << __func__ << val;
    if(val >= 0 && val <= 9)
    {   // 输入数字0-9
        if(m_inputText == ""){
           if(val > 0)
               m_inputText = QString::number(val);
        }
        else if(m_inputText.size() > 0)
            m_inputText += QString::number(val);
            ui->lineEdit->setText(m_inputText);
    }
    else if(val == 10)
    {   // 小数点
        if(m_inputText.size() == 0)
            m_inputText = "0.";
        else
            m_inputText += ".";
        m_floatFlag = true;
        ui->lineEdit->setText(m_inputText);
    }
    else if(val == 11)
    {   // +/-号
        if(m_inputText.startsWith("-"))
            m_inputText = m_inputText.right(m_inputText.size()-1);
        else if(m_inputText.size() > 0)
            m_inputText = "-" + m_inputText;
        ui->lineEdit->setText(m_inputText);
    }
    else if(val == 12)
    {   // 清除
        m_inputText = "";
        m_floatFlag = false;
        ui->lineEdit->setText(m_inputText);
    }
    else if(val == 13)
    {   // 取消
        reject();
    }
    else if(val == 14)
    {   // 确定
        accept();
    }
}


/***************************************
* class : AlarmDisplay
* author: wangmingxiao
* date  : 2015-10-29
***************************************/
AlarmDisplay::AlarmDisplay(QWidget *parent):MenuDlg(parent),ui(new Ui::AlarmDisplay)
{
    ui->setupUi(this);

    //setWindowFlags(Qt::Tool);
    setWindowTitle(tr("报警显示窗口"));

    connect(ui->close, SIGNAL(clicked(bool)), this, SLOT(toHideMe()));

}

AlarmDisplay::~AlarmDisplay()
{
    delete ui;
}

void AlarmDisplay::createOperation()
{
    addObject(ui->close);
}

void AlarmDisplay::setDisplayText(const QString &text)
{
    ui->textdisplay->setText(text);
}


/************************************************************
* class ReplayDlg
*/
ReplayDlg::ReplayDlg(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::ReplayDlg),m_lpRecRep(NULL)
{
    ui->setupUi(this);
  //  setWindowFlags(Qt::Tool);
     setWindowFlags(Qt::FramelessWindowHint);   //Qt::Widget|
    setWindowTitle("记录重演控制窗口");

    const QDate dt0(2015,1,1);
    ui->dateTimeStart->setMinimumDate(dt0);
    ui->dateTimeStop->setMinimumDate(dt0);

    const QDateTime dt = QDateTime::currentDateTime();
    ui->dateTimeStop->setDateTime(dt);
    ui->dateTimeStart->setDateTime(dt.addDays(-1));
    //ui->dateTimeStop->setReadOnly(false);
    ui->totalTime->setDisabled(true);
    //ui->totalTime->setTime(QTime(1,0,0));

    m_pathFlag = 1;
    ui->localDisk->setChecked(m_pathFlag==1);
    ui->removeDisk->setChecked(m_pathFlag==2);

    cicleReplay = 0;
}

ReplayDlg::~ReplayDlg()
{
    delete ui;
}

void ReplayDlg::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

QString ReplayDlg::getRemoveDiskPath() const
{
    QDir dir(RECORDPATH);
   // dir.setFilter(QDir::NoDotAndDotDot);

    QFileInfoList list = dir.entryInfoList();
    QString path;
    for(int i=0; i<list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        QString name = fileInfo.fileName();
        if(name != "." && name != ".." ) {
            path = fileInfo.filePath();
           // qDebug() << QString("the remove disk path is: %1").arg(path);
        }
    }

    return path;
}

bool ReplayDlg::setRecordPath(quint8 pathv)
{
    //if(m_pathFlag != pathv)
    {
        QDir dir(RECORDPATH);

        if(2 == pathv && getRemoveDiskPath().isEmpty())
        {
            QMessageBox::critical(this, "移动磁盘不存在","请插入移动磁盘，再启动记录重放功能。");
            return false;
        }

        QString path;
        if(pathv == 1)
            path = CURRENTPATH + QString("/record/");
        else
            path = getRemoveDiskPath() +  QString("/record/");

        m_lpRecRep->setRecordPath(path);
        m_pathFlag = pathv;
    }
    return true;
}

void ReplayDlg::on_recordBtn1_clicked(bool checked)
{
    if(m_lpRecRep)
    {
        if(checked)
        {
            if(!m_lpRecRep->isRecording())
            {
                if(setRecordPath(m_pathFlag))
                m_lpRecRep->startRecord();
            }
        }
        else if(m_lpRecRep->isRecording())
        {
            m_lpRecRep->stopRecord();
        }
        ui->recordBtn1->setChecked(m_lpRecRep->isRecording());
    }
}


void ReplayDlg::on_localDisk_clicked(bool)
{
        if(m_lpRecRep && (m_lpRecRep->isRecording() || m_lpRecRep->isReplaying()))
        {
                ui->localDisk->setChecked(false);
                QMessageBox::critical(this, "磁盘选择错误","要更改磁盘位置，请先停止记录和重放。");
                return;
        }

        m_pathFlag = 1;
        ui->localDisk->setChecked(m_pathFlag==1);
        ui->removeDisk->setChecked(m_pathFlag==2);
}

void ReplayDlg::on_removeDisk_clicked(bool)
{
        if(m_lpRecRep && (m_lpRecRep->isRecording() || m_lpRecRep->isReplaying()))
        {
                ui->removeDisk->setChecked(false);
                QMessageBox::critical(this, "磁盘选择错误","要更改磁盘位置，请先停止记录和重放。");
                return;
        }

        m_pathFlag = 2;
        ui->localDisk->setChecked(m_pathFlag==1);
        ui->removeDisk->setChecked(m_pathFlag==2);
}


// 开始时间选择
void ReplayDlg::on_dateTimeStart_dateTimeChanged ( const QDateTime & datetime )
{
     //   qDebug() << "on_dateTimeStart_dateTimeChanged:" << datetime;
    // 如果开始时间改变，则需要修改结束时间
    if(m_dateTimeStart != datetime)
    {
        m_dateTimeStart = datetime;
        //setReplayStopTime (m_dateTimeStart.addSecs(GetTotalTime()));
        //qDebug() << m_dateTimeStart;
    }
}

// 结束时间选择
void ReplayDlg::on_dateTimeStop_dateTimeChanged ( const QDateTime & datetime )
{
        if(datetime != m_dateTimeStop)
        {
                m_dateTimeStop = datetime;
        }
}

// 总时间选择
void ReplayDlg::on_totalTime_timeChanged ( const QTime & tm0 )
{
     //   qDebug() << "on_totalTime_timeChanged:" << tm0;
    if (tm0 != m_totalTime)
    {
        m_totalTime = tm0;
        //setReplayStopTime (m_dateTimeStart.addSecs(GetTotalTime()));
        //qDebug() << m_totalTime;
    }
}


// 速度选择
void ReplayDlg::on_speedX1_clicked(bool)
{
        setReplaySpeed(1);
}
//void ReplayDlg::on_speedX2_clicked(bool)
//{
//	setReplaySpeed(2);
//}
void ReplayDlg::on_speedX3_clicked(bool)
{
        setReplaySpeed(2);
}
void ReplayDlg::on_speedX4_clicked(bool)
{
   //     setReplaySpeed(3);
}
void ReplayDlg::on_speedX5_clicked(bool)
{
   //     setReplaySpeed(10);
}

// 开始、暂停、停止操作
void ReplayDlg::on_toStart_clicked (bool)
{
    if(setRecordPath(m_pathFlag));
        //qDebug() << "on_toStart_clicked";

    if(!MenuConfig.otherMenu.transmite) {
        MenuConfig.otherMenu.transmite  = 1;  //置为发射状态，但是不开启天线
        MenuConfig.dispMenu.showHeadLine = 1;
        SystemInfo.ViewDisplay.guildline = 1;
    }
    toStartReplay (0);
}

void ReplayDlg::on_toPause_clicked (bool)
{
        if(m_lpRecRep)
        {
    m_lpRecRep->pause();

    if (m_lpRecRep->isPaused())
        ui->toPause->setText (tr("继续"));
    else
        ui->toPause->setText (tr("暂停"));
        }
}

void ReplayDlg::on_toStop_clicked (bool)
{
    m_lpRecRep->stopReplay();

    MenuConfig.otherMenu.transmite = 0;  //关闭显示
    pView->initEchoView();
    pView->update();
        //ResetAll();
}
void ReplayDlg::on_closeBtn_clicked(bool)
{
    //关闭记录
    if(m_lpRecRep->isRecording())
        m_lpRecRep->stopRecord();
    //关闭重演
    if(m_lpRecRep->isReplaying() || m_lpRecRep->isPaused())
        m_lpRecRep->stopReplay();
    //关闭窗口
    hide();

}

// 设置重演速度
void ReplayDlg::setReplaySpeed(int spd)
{
      //  qDebug() << "setReplaySpeed:" << spd;
    if (m_lpRecRep)
    m_lpRecRep->changeReplaySpeed(spd);
}

// 设置重演结束时间
void ReplayDlg::setReplayStopTime (const QDateTime& dt)
{
    if (dt != m_dateTimeStop)
    {
        m_dateTimeStop = dt;
        ui->dateTimeStop->setDateTime (m_dateTimeStop);
    }
}

// 获取总重演时间长度
int ReplayDlg::GetTotalTime () const
{
    static QTime tm(0,0,0);
    return tm.secsTo (m_totalTime);
}

void ReplayDlg::toStartReplay (quint8 mode)
{
        if(!m_lpRecRep)
                return ;

    if (isTimeValid())
    {
        const quint32 tm1 = m_dateTimeStart.toTime_t();
        const quint32 tm2 = m_dateTimeStop.toTime_t();
        if (!m_lpRecRep->startReplay("", tm1, tm2))
            return;
    }
    else
    {
        QMessageBox::information(this, tr("时间选择错误"), tr("时间选择错误，请选择回放时间"));
        return;
    }

        // 更新重演时间显示
    updateReplayTotalTime (m_lpRecRep->getReplayTotalTime());
        // 总清操作
        //ResetAll();
}

// 判断指定的时间是否有效
bool ReplayDlg::isTimeValid () const
{
        return m_dateTimeStop > m_dateTimeStart;
}

// 更新状态
void ReplayDlg::updateStatus()
{
    if (!m_lpRecRep)
        return;

    const bool replayingFlag = m_lpRecRep->isReplaying();
    const bool notReplayingFlag = (!replayingFlag);
    isReplaying = replayingFlag;

   // qDebug() << "updateStatus " << replayingFlag;

    ui->toStart->setEnabled(notReplayingFlag);
    ui->toPause->setEnabled(replayingFlag);
    ui->toStop->setEnabled(replayingFlag);

    ui->dateTimeStart->setEnabled (notReplayingFlag);
    ui->dateTimeStop->setEnabled (notReplayingFlag);
    ui->totalTime->setEnabled(false);//(notReplayingFlag);

    // 指定速度
    switch(m_lpRecRep->replaySpeed())
    {
    case 1:
        ui->speedX1->setChecked(true);
        break;
    case 2:
        ui->speedX3->setChecked(true);
        break;
    case 5:
        ui->speedX4->setChecked(true);
        break;
    case 10:
        ui->speedX5->setChecked(true);
        break;
    case 20:
      //  ui->speedX5->setChecked(true);
        break;
    default:
        break;
    }

        /*
        const bool canChangeSpeed = true;
        ui->speedX1->setEnabled(canChangeSpeed);
        //ui->speedX2->setEnabled(canChangeSpeed);
        ui->speedX3->setEnabled(canChangeSpeed);
        ui->speedX4->setEnabled(canChangeSpeed);
        ui->speedX5->setEnabled(canChangeSpeed);
        */

    ui->timeCtrl->setEnabled(false);

    if (m_lpRecRep->isPaused())
        ui->toPause->setText (tr("继续"));
    else
        ui->toPause->setText (tr("暂停"));

    updateReplayTime();
}

// 更新重演总时间显示
void ReplayDlg::updateReplayTotalTime (quint32 tt)
{
    m_replayTotalTime = tt;
    ui->timeCtrl->setRange (0, m_replayTotalTime);
    ui->timeCtrl->setValue (0);
}

// 更新时间显示
void ReplayDlg::updateReplayTime ()
{
    quint32 tm = m_lpRecRep ? m_lpRecRep->getReplayTime () : 0;

    if (m_replayTotalTime > 0)
    {
        int tt = (int)(100.0 * (float)tm / (float)m_replayTotalTime);
        QString timeString = tr("%1%").arg(tt, 2, 10, QLatin1Char('0'));
        ui->timeCtrl->setValue (tm);
        ui->timeShow->setText (timeString);
        ui->timeCtrl->setToolTip(timeString);
    }
    else
    {
        ui->timeShow->setText (tr("00%"));
        //timeCtrl->setToolTip("No replay file");
    }

    //播放完毕循环播放
    if(tm == m_replayTotalTime) {
         m_lpRecRep->stopReplay();
         cicleReplay = 1;
      }
    if(cicleReplay && !m_lpRecRep->isRecording()) {

       QEventLoop eventloop;
        QTimer::singleShot(200, &eventloop, SLOT(quit()));
        eventloop.exec();

        toStartReplay(0);
        cicleReplay = 0;
     }

}










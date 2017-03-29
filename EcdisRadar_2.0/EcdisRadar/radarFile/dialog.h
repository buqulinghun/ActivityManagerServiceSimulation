#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QMutex>
#include <QDateTime>
#include <QTextEdit>




namespace Ui {
    class OperateDlg;   //就是设计图上最上层的名字
}
namespace TgtBox {
        enum {
                TGTBOX_CLEAR = 0,
                TGTBOX_UPDATE,
                TGTBOX_INSERT,
                TGTBOX_SWITCH,
        };
}



class AisBox;
class AtaBox;




//链表型节点结构
typedef struct tagNode
{
    tagNode *next;  //下一节点
    tagNode *prev;  //前一节点
    QWidget *widget;   //包含的控件
    QRect labelrect;  //部件所在区域，全局坐标

    bool focusion;  //确认标志

    //构造函数
    tagNode(QWidget *w=0):next(0),prev(0),widget(w),labelrect(0,0,0,0),focusion(false)
    {}

    bool isFirst() const
    {
        return (prev == NULL);
    }

    bool isLast() const
    {
        return (next == NULL);
    }
}NODE,*LPNODE;

#define MSGID_AlarmInfo (QEvent::User+1)

class Dialog : public QDialog
{
    Q_OBJECT
public:
    Dialog(QWidget *parent = 0);
    ~Dialog();

    // 更新本船信息显示
    void updateBoatInfoDisplay();
    // 更新系统日期时间显示
    void updateDateTimeDisplay();
    // 更新按钮状态
    void updateButtonState();
    void updateUpmode();
    void updateMotion();
    void updateDisplay();
    void updateOffset();
    void updateHeadline();
    void updateRngring();
    void updateAtaBtnState();
    void updateDataSwitch();

    // 跟新亮度显示
    void updateBright(int val);
    // 更新链路状态显示
    void setLinkStatusDisp(quint8 index, quint8 status);

    void alarmProcess();
    void clearAlarmInfo();

    void setDispinfo(int);
    void showAlarmInfo(const QString& information);
    void customEvent(QEvent *);

    // flag 0:clear, 1:update, 2:insert,3:switch
    void updateAis(quint32 mmsi, quint8 flag = 0);
    void updateAta(quint32 index, quint8 flag = 0);
    // 更新目标Box显示
    void clearTargetBoxDisplay(quint8 flag = 0xff);
    // 获取AIS目标显示索引号
    int getAisDispIndex(quint32 mmsi);
    // 获取ATA目标显示索引号
    int getAtaDispIndex(quint32 index);


    //将部件加入鼠标操作
    void addOperateWidget(QWidget *widget, QRect rect);
    void initOperateWidget();
    //键盘确认键处理，如果处理返回true，没有处理返回false
    bool kbd_confirm(QPoint pt,quint8 flag=0);


public slots:
    void on_alarmStatus_clicked(bool);
    //1:ais  2:ata
    void on_toolButton_1_clicked(bool);
    void on_toolButton_2_clicked(bool);


protected:
    void changeEvent(QEvent *e);
    void showEvent(QShowEvent *);

    void createAisAndAtaBox();

private:
    Ui::OperateDlg *ui;
    //StatusBtn *m_alarmStatusBtn;
    //StatusBtn *m_linkStatusBtn[5];
    int m_brightValue;

    QMutex  m_mutex;
    AisBox *m_aisBoxes[5];
    quint8  m_crntAisIndex;
    quint32 m_aisIndex[5];

    AtaBox *m_ataBoxes[5];
    quint8  m_crntAtaIndex;
    quint32 m_ataIndex[5];

    //关于鼠标操作监视界面的事
    LPNODE m_operateNodeTree;

};

/*************************************************
* class  : AisBox
* author :
* date   :2015-7-15
*************************************************/
namespace Ui {
        class AisBox;
}

class AisBox : public QWidget
{
public:
    AisBox(QWidget *parent = 0);
    ~AisBox();

    void updateDisplay(quint32 mmsi);
    void clearDisplay();
    quint32 index() const
    {	return m_mmsi;	}

protected:
    void changeEvent(QEvent *e);

private:
        Ui::AisBox *ui;
        quint32 m_mmsi;
};
/*************************************************
* class  : AtaBox
* author :
* date   : 2015-7-15
*************************************************/
namespace Ui {
        class AtaBox;
}

class AtaBox : public QWidget
{
public:
    AtaBox(QWidget *parent = 0);
    ~AtaBox();

    void updateDisplay(quint32 mmsi);
    void clearDisplay();
    quint32 index() const
    {	return m_index;	}

protected:
    void changeEvent(QEvent *e);

private:
        Ui::AtaBox *ui;
        quint32 m_index;
};






/*************************************************
* class  : MenuDlg
* author :
* date   : 2015-8-18
*************************************************/
class MenuDlg : public QDialog
{
    Q_OBJECT
public:
    MenuDlg(QWidget *parent = 0);
    virtual ~MenuDlg();

    //键盘确认键处理，如果处理返回true，没有处理返回false
    bool kbd_confirm(QPoint pt,quint8 flag=0);

    //在链表中加入控件
    void addWidget(QWidget *widget,QRect rect=QRect(0,0,0,0));

    //获取链表型头
    LPNODE getNodeHead() const
    {
        return m_lpNodeHead;
    }

    void updateDlgDisplay()
    {  updateDisplay();  }

protected:
    //更新显示,由子类实现
    virtual void updateDisplay() {};



    void hideEvent(QHideEvent *event);
    void showEvent(QShowEvent *event);
    //void mouseMoveEvent(QMouseEvent *event);  //鼠标移动处理，处理按键

    void setChangedFlag()
    {   m_changed = true;   }
    bool changedFlag() const
    {   return m_changed;   }

    void freeLink(LPNODE head);


protected slots:
    void toHideMe();

private:
    bool   m_changed;

    LPNODE m_lpNodeHead;  //链表头

};


/*************************************************
* class  : MainMenu
* author : wangmingxiao
* date   : 2015-8-18
*************************************************/
namespace Ui {
    class MainMenu;
}
class MainMenu : public MenuDlg
{
    Q_OBJECT
public:
    MainMenu(QWidget *parent = 0);
    ~MainMenu();

    void createOperation();

protected:
    void changeEvent(QEvent *event);



protected slots:
    void on_toolButton_1_clicked(bool);
    void on_toolButton_2_clicked(bool);
    void on_toolButton_3_clicked(bool);

private:
    Ui::MainMenu *ui;

};
/*************************************************
* class  : DisplayMenu
* author : wangmingxiao
* date   : 2015-8-18
*************************************************/
namespace Ui {
    class DisplayMenu;
}
class DisplayMenu : public MenuDlg
{
    Q_OBJECT
public:
    DisplayMenu(QWidget *parent = 0);
    ~DisplayMenu();

    void createOperation();



protected:
    void changeEvent(QEvent *event);
    void updateDisplay();

protected slots:
    void upmode_clicked(int);
    void motionmode_clicked(int);
    void daymode_clicked(int);
    void offset_clicked(int);
    void headlinedisp_clicked(int);
    void rngringdisp_clicked(int);
    void trail_clicked(int);
    void veclen_clicked(int);
    void track_clicked(int);
    void symbSize_clicked(int);
    void color_clicked(int);
    void aisdisplay_clicked(int);
    void arpadisplay_clicked(int);
    void bright_valueChanged(int);

private:
    Ui::DisplayMenu *ui;
};
/*************************************************
* class  : InstallMenu
* author : wangmingxiao
* date   : 2015-8-18
*************************************************/
namespace Ui {
    class InstallMenu;
}
class InstallMenu : public MenuDlg
{
    Q_OBJECT
public:
    InstallMenu(QWidget *parent = 0);
    ~InstallMenu();

    void createOperation();

protected:
    void changeEvent(QEvent *event);
    void updateDisplay();

protected slots:
    void DataOutput_clicked(int val);
    void langSelect_clicked(int val);
    void on_serialConfig_clicked(bool);
    void on_systemCheck_clicked(bool);

    void on_dateTimeEdit_dateTimeChanged(const QDateTime &datetime);
    void on_tuneMan_clicked(bool);
    void tuneValue_valueChanged(int);
    void range_changed(int val);
    void band_changed(int val);
    void HL1710_clicked(int val);
    void on_clearAlarm_clicked(bool);


    void aziAdjust_Changed(int val);
    void rngAdjust_Changed(int val);
    void mbsAdjust_Changed(int val);

    void on_timeUsed_textChanged(const QString &text);
    void on_timeTrans_textChanged(const QString &text);

private:
    Ui::InstallMenu *ui;



};
/*************************************************
* class  : OtherMenu
* author : wangmingxiao
* date   : 2015-8-18
*************************************************/
namespace Ui {
    class OtherMenu;
}
class OtherMenu : public MenuDlg
{
    Q_OBJECT
public:
    OtherMenu(QWidget *parent = 0);
    ~OtherMenu();

    void updateNaviPointDisplay(quint8 idx);

    void createOperation();
    //刷新警戒区域显示
    void setGuardZoneText(const float z0, const float z1, const float z2, const float z3);

    //设置手动调谐值显示
    void setManTuneValue(quint16 val);

protected:
    void changeEvent(QEvent *event);
    void updateDisplay();

protected slots:
    void course_clicked(int val);
    void speed_clicked(int val);
    void transmit_clicked(int val);
    void xinneng_clicked(int val);
    void jam_clicked(int val);  //同频干扰
    void vrm_clicked(int val);
    void on_vrm_1_textChanged(const QString& text);
    void on_vrm_2_textChanged(const QString& text);
    void on_vrm_3_textChanged(const QString& text);
    void on_vrm_4_textChanged(const QString& text);
    void plotLimit_valueChanged(int);  //检测门限
    void echoExpand_Changed(int val);  //回波扩展

    void audibleWarning_clicked(int val);
    void guardZone_clicked(int val);
  //  void guardSelect_clicked(int val);
    void on_cpaBtn_clicked(bool);
    void on_tcpaBtn_clicked(bool);
    void on_cpaEdit_textChanged(const QString& text);
    void on_tcpaEdit_textChanged(const QString& text);
    void on_guardZone_0_textChanged(const QString& text);
    void on_guardZone_1_textChanged(const QString& text);
    void on_guardZone_2_textChanged(const QString& text);
    void on_guardZone_3_textChanged(const QString& text);

    void on_naviPoint_1_clicked(bool);
    void on_naviPoint_2_clicked(bool);

    void on_tuneMan_clicked(bool checked);
    void on_tuneAuto_clicked(bool checked);
    void tuneValue_valueChanged(int value);
    void alarm_changed(int value);

    void on_saveRecord_clicked(bool);
    void on_replayRecord_clicked(bool);

private:
    Ui::OtherMenu *ui;

    quint8 navi_btn;
    quint8 alarm_btn;
    quint8 record_btn;

};
/*************************************************
* class  : SerialConfigMenu
* author : wangmingxiao
* date   : 2015-8-18
*************************************************/
namespace Ui {
    class SerialConfigMenu;
}

class SerialConfigMenu : public MenuDlg
{
    Q_OBJECT
public:
    SerialConfigMenu(QWidget *parent = 0);
    ~SerialConfigMenu();

    void createOperation() {}

protected:
    void changeEvent(QEvent *e);
    void updateSerialConfig(const QString& name);
    void updateDisplay();

protected slots:
    void SerialConfig_clicked(int val);
    void on_saveButton_clicked();

private:
    Ui::SerialConfigMenu *ui;

    QString m_name;
    float m_stopbit;
};

/***************************************
* class : InputMenu
* author: wangmingxiao
* date  : 2015-9-6
***************************************/
namespace Ui {
    class InputMenu;
}

class InputMenu : public MenuDlg {
    Q_OBJECT
public:
    InputMenu(QWidget *parent = 0);
    ~InputMenu();

    void createOperation();

    QString getText() const
    {   return m_inputText.isEmpty() ? "0" : m_inputText; }
    int getInt() const
    {   return m_inputText.toInt();     }
    float getFloat() const
    {   return m_inputText.toFloat();  }
    bool isFloat() const
    {   return m_floatFlag; }

    int getDlgResult();

    //设置输入菜单的密码形式
    void setPassword(quint8 flag);


protected:
    void changeEvent(QEvent *event);
    void updateDisplay();
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    bool customDlgDirection(int direction);

    void setCrntIndex(int index);

protected slots:
    void input_clicked(int);

private:
    Ui::InputMenu *ui;
    int m_crntIndex;

    QString m_inputText;
    bool m_floatFlag;
};


namespace Ui {
    class AlarmDisplay;
}

class AlarmDisplay : public MenuDlg {
    Q_OBJECT
public:
    AlarmDisplay(QWidget *parent=0);
    ~AlarmDisplay();

    void createOperation();

    void setDisplayText(const QString &text);

protected:
   // void on_close_clicked(bool);

private:
    Ui::AlarmDisplay *ui;
};

namespace Ui{
    class ReplayDlg;
}
class Recordreplay;
class ReplayDlg : public QDialog {
    Q_OBJECT
public:
    ReplayDlg(QWidget *parent = NULL);
    ~ReplayDlg();

    void updateStatus();
    // 更新重演总时间显示
    void updateReplayTotalTime (quint32 totalTime);
    // 更新时间显示
    void updateReplayTime ();
    // 设置重演对象
    void setReplayObject(Recordreplay* obj)
    {	m_lpRecRep = obj;		}

protected:
    void changeEvent(QEvent *e);

protected:
    // 设置重演结束时间
    void setReplayStopTime (const QDateTime& dt);
    // 获取总重演时间长度
    int  GetTotalTime () const;
    // 设置重演速度
    void setReplaySpeed(int spd);
    // 判断指定的时间是否有效
    bool isTimeValid () const;
    // 启动重演
    void toStartReplay (quint8 mode);

    bool setRecordPath(quint8 pathv);

protected slots:
    void on_recordBtn1_clicked(bool checked);

    void on_localDisk_clicked(bool);
    void on_removeDisk_clicked(bool);

    // 时间选择
    void on_dateTimeStart_dateTimeChanged ( const QDateTime & datetime ) ;
    void on_dateTimeStop_dateTimeChanged ( const QDateTime & datetime ) ;
    void on_totalTime_timeChanged ( const QTime & time );

    // 速度选择
    void on_speedX1_clicked(bool);
    //void on_speedX2_clicked(bool);
    void on_speedX3_clicked(bool);
    void on_speedX4_clicked(bool);
    void on_speedX5_clicked(bool);

    // 开始、暂停、停止操作
    void on_toStart_clicked (bool);
    void on_toPause_clicked (bool);
    void on_toStop_clicked (bool);
    void on_closeBtn_clicked(bool);

    //获取移动磁盘目录
    QString getRemoveDiskPath(void) const;

private:
    Ui::ReplayDlg *ui;

    Recordreplay * m_lpRecRep;
    bool	m_fileValidFlag;
    quint8	m_pathFlag;

    QDateTime	m_dateTimeStart;
    QDateTime	m_dateTimeStop;
    QDateTime	m_fileTimeStart;
    QDateTime	m_fileTimeStop;
    QTime	m_totalTime;

    quint32	m_replayTotalTime;
    quint8 cicleReplay;

};





#endif // DIALOG_H


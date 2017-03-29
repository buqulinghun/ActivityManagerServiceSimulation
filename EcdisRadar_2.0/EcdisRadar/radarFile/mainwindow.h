#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QToolButton>
#include <QWidget>
#include <QObject>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QHideEvent>
#include <QShowEvent>
#include <QResizeEvent>
#include <QString>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QGroupBox>
#include <QToolButton>
#include <QByteArray>
#include <QEvent>
#include <QStack>
#include <QTranslator>
#include <QProgressBar>

#include "dialog.h"
#include "define.h"



class Dialog;

enum {
        VIEW_UPDATE_RANGE_BIT = 0,
        VIEW_UPDATE_TMRM_BIT,
        VIEW_UPDATE_UP_BIT,
        VIEW_UPDATE_DAY_BIT,
        VIEW_UPDATE_HL_BIT,
        VIEW_UPDATE_RNGRING_BIT,
        VIEW_UPDATE_AIS_BIT,
        VIEW_UPDATE_ATA_BIT,
        VIEW_UPDATE_COLOR_BIT,
        VIEW_UPDATE_SYMBOL_BIT,
        VIEW_UPDATE_TRAIL_BIT,
        VIEW_UPDATE_SECTER_BIT,
        VIEW_UPDATE_TRACK_BIT,
        VIEW_UDPATE_TR_BIT,
        VIEW_UPDATE_JAM_BIT,
        VIEW_UPDATE_GAIN_BIT,
        VIEW_UPDATE_RAIN_BIT,
        VIEW_UPDATE_CLUTTER_BIT,
        VIEW_UPDATE_EBL1_BIT,
        VIEW_UPDATE_VRM1_BIT,
        VIEW_UPDATE_EBL2_BIT,
        VIEW_UPDATE_VRM2_BIT,
        VIEW_UPDATE_MOUSEPOS_BIT,
        VIEW_UPDATE_AUTO_BIT,
        VIEW_UPDATE_OFST_BIT,
};



typedef struct tagLinkStatus
{
    quint8 changed : 1;
    quint8 status  : 7;
}LINKSTATUS;

enum{
    AIS_DEVICE = 0,
    GPS_DEVICE,
    COMP_DEVICE,
    LOG_DEVICE,
    ALARM_DEVICE,
};
#define MAX_OUTER_DEVICE 4
#define BROKEN 0
#define NORMAL 11


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    // 设置当前系统时间
    void setCrntTime(quint32 tm, bool udpate = true);
    //设置颜色模式
    void setColorMode(int flag);
    //////////按键操作
    //设置距标圈显示
    void setRngringMode();
    //设置船艏线显示
    void setHeadlineMode();
    //VRM/EBL显示
    void setVrmeblMode();
    //VRM切换
    void switchVrmeblMode();
    //调整VRM半径
    void change_Vrm(int val);
    //调整EBL半径
    void change_EBL(int val);
    //设置调谐
    void setTuneMode();
    //设置船艏向显示
    void setGuildMode();
    //偏心显示设置
    void setOffsetMode(bool flag=true);  //1:真正的偏心显示，0更改面板显示而已

    //设置亮度模式
    void setBrightMode(int index, int val);
    // 设置屏幕亮度
    void setScreenBright(int val);
    // 设置键盘亮度
    void setKbdBright(int val);
    // 设置固定标线亮度
    void setFixLineBright(int val);
    // 设置活动标线亮度
    void setVarLineBright(int val);
    //设置显示情景模式
    void setDisplayMode(int val);

    //设置亮度选择
    void setBrightSelect();
    //设置亮度大小
    void setBrightValue(quint8 flag);
    void toSetLanguage(quint8 flag);
    //清空报警显示
    void clearAlarmInfo();
    //显示重演窗口
    void showReplayDlg(bool show);
    //切换记录状态
    void switchRecord();




protected:
    virtual void timerEvent(QTimerEvent *event);
    void changeEvent(QEvent *event);
    void mouseMoveEvent(QMouseEvent *event);




private:

    int m_timerid1;
    quint8 m_colorMode;
    quint8 m_switchVrmebl;  //vrm/ebl切换
    quint8 m_vrmeblChanged;
    quint32 m_time100ms;  //时间记数,采用定时器更新VRM和EBL，因为采用信号槽机制消耗CPU太多
#ifdef  TIMEEVENTFLASH
    quint32 m_time50ms;
#endif

    //信号处理参数,用来判断刷新
    int m_gaint;
    int m_restraint;
    int m_cluttert;
    int m_tunet;

    quint8 m_brightIndex;  //颜色索引
    quint8 m_lightChanged;   //亮度改变标志
    quint8 m_screenLightChanged;  //屏幕亮度

    QTranslator translator;
    void retranslate(QWidget *mainwindow=0);  //定义翻译函数，实现实时更换语言



public:
    //刷新设备
    void refreshDeviceLink(int device);
    const LINKSTATUS* linkStatus() const
    {  return m_linkStatus;    }
    LINKSTATUS linkStatus(int device) const
    {  if(device < MAX_OUTER_DEVICE)    return m_linkStatus[device];   }



protected:
    void updateLinkInfo();


private:
    //设备链路状态类
    int linkKeepTime[MAX_OUTER_DEVICE];  //状态保持时间
    LINKSTATUS m_linkStatus[MAX_OUTER_DEVICE];  //链路状态


//////////////////创建控制面板，就是回波显示中的四个角的标签显示///////////////
public:
    //更新显示控制
    void updateDispCtrl();
    void updateTuneDisp();
    void createCtrlPanel();
    void updateCtrlPanel(quint32 flag);
    //更新鼠标显示
    void updateMousePosDisplay();

    //检测是否有按钮按下
    bool btn_confirm(QPoint pt);
    //屏幕亮度
    void updateScreen(quint8 val);


    //刷新警戒区域显示
    void refreshGuardZone();
    //设置系统时间
    void setSystemTime(int year,int mon,int day,int hour,int min,int sec);
    //设置手动调谐显示值
    void setManTuneValue(int val);


    //刷新功能面板显示
    void updateOtherDlg();


signals:
    void mousePressed(QMouseEvent *event);
    void dialogMousePressed(QMouseEvent *event);


public slots:
    void on_m_menu_btn_clicked(bool);
    void on_m_cancel_btn_clicked(bool);
    //左上角的按键操作
    void on_m_up_btn_clicked(bool);
    void on_m_tmrm_btn_clicked(bool);
    void on_m_trial_btn_clicked(bool);

    void updateVrmeblDisplay();


    void KbdDataSolve(QByteArray data);

private:


    //四周控制面板变量
    QPoint m_gboxPos[6];
    QGroupBox *m_gbox[6];

    QLineEdit *m_range;  //最大量程
    QLineEdit *m_range2;  //距离圈刻度所代表的量程
   // QLineEdit *m_gain;    //增益
  //  QLineEdit *m_restrain;   //雨雪
   // QLineEdit *m_clutter;    //杂波
    QProgressBar *m_tune;  //调谐显示
    QProgressBar *m_gain;
    QProgressBar *m_restrain;
    QProgressBar *m_clutter;

    QLabel *label_gain;
    QLabel *label_restrain;
    QLabel *label_clutter;
    QLabel *label_vrm1;
    QLabel *label_ebl1;
    QLabel *label_vrm2;
    QLabel *label_ebl2;
    QLabel *label_tune;

    QToolButton *m_tmrm_btn, *m_up_btn, *m_trial_btn, *m_menu_btn, *m_x_btn;

    QToolButton *m_vrm1_btn, *m_ebl1_btn, *m_vrm2_btn, *m_ebl2_btn;
    QCheckBox *m_showEbl1, *m_showEbl2;
    QLineEdit *m_ebl1, *m_ebl2;
    QCheckBox *m_showVrm1, *m_showVrm2;
    QLineEdit *m_vrm1, *m_vrm2;

    QLabel *m_mousePositionLabel;
    QLabel *m_systemInfoLabel;
    QLabel *m_dataTimeLabel;
    QLabel *m_dispCtrlLabel;

    QLabel *m_irLabel;
    QLabel *m_autoLabel;
    QLabel *m_ofstLabel, *m_aisLabel, *m_ataLabel;

 ///////////////////////////右边对话框操作类//////////////////////
public:
    void displayMenu(QDialog *menu, bool hidelast = true, bool pushlast = true); //显示新菜单窗口
    void gobackMenu();  //返回上层窗口
    void hideAllMenu();  //隐藏所有窗口
    bool isMenuShow() const
    {  return !m_menutree.isEmpty();  }  //判断是否有菜单窗口显示
    QDialog* showMainMenu();  //显示主菜单
    QDialog* showDisplayMenu();  //显示显示菜单窗口
    QDialog* showInstallMenu();  //显示安装菜单窗口
    QDialog* showOtherMenu();   //显示其他菜单窗口
    QDialog* showSerialConfigMenu(); //显示串口配置菜单窗口
    // 显示输入菜单窗口
    QDialog* showInputMenu();
    QDialog* showAlarmDisplayMenu();  //显示报警信息窗口
    void setAlarmDisplayText(const QString &text);  //设置显示的报警信息
    QDialog* crntTopMenu()
    {  return m_menutree.isEmpty() ? NULL : m_menutree.top();  }   //获取当前显示的窗口
    QDialog* showChangedDispMenu();  //显示更改颜色的显示菜单

    void setInputStack(QDialog *menu, quint8 flag);








protected:

    QStack<QDialog *> m_menutree;

    class MainMenu* m_lpMainMenu;
    class DisplayMenu* m_lpDispMenu;
    class InstallMenu* m_lpInstMenu;
    class OtherMenu* m_lpOtherMenu;
    class SerialConfigMenu* m_lpSerialConfigMenu;
    class InputMenu* m_lpInputMenu;
    class AlarmDisplay* m_lpAlarmDispMenu;

    quint8 m_eblvrmSwitch[2];
    quint8 m_aisarpaSwitch;


    ReplayDlg* m_lpRecordDlg;


};
/////////////////////////////////////////////
//DockWidget的定义

class DockWidget : public QDockWidget
{
    Q_OBJECT

public:
    DockWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0)
            :QDockWidget(parent, flags)
    {
        initialize();
    }
    DockWidget(const QString &title, QWidget *parent = 0, Qt::WindowFlags flags = 0)
            :QDockWidget(title, parent, flags)
    {
        initialize();
    }

public:
    inline void allow(Qt::DockWidgetArea area, bool allow);
    inline void place(Qt::DockWidgetArea area, bool place);

    inline bool IsVisible() const;

public slots:
    void changeClosable(bool on);
    void changeMovable(bool on);
    void changeFloatable(bool on);
    void changeFloating(bool on);
    void changeVerticalTitleBar(bool on);

    void allowLeft(bool a);
    void allowRight(bool a);
    void allowTop(bool a);
    void allowBottom(bool a);

    void placeLeft(bool p);
    void placeRight(bool p);
    void placeTop(bool p);
    void placeBottom(bool p);

    void splitInto(QAction *action);
    void tabInto(QAction *action);

protected:
    virtual void moveEvent (QMoveEvent*);
    virtual void leaveEvent(QEvent* event);

private:
    void initialize ();

};



#endif // MAINWINDOW_H

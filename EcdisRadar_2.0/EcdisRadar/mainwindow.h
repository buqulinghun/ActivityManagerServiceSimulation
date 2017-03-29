#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>


#include "myscene.h"
#include "myview.h"
#include "chart.h"
#include "waypoint.h"
#include "filepath.h"
#include "chart.h"
#include "longlat.h"
#include "mydialog.h"
#include "database.h"


namespace Ui {
    class MainWindow;
}
class FactorDiaog;
class AdjustDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void Init();

    void setCoordinateDisp(int x, int y, double lon, double lat);   //设置坐标显示
    void setScaleDisp(int scale);   //设置比例尺显示

    QPointF screenCrenterToScene() const;

protected:
    void changeEvent(QEvent *e);



    void refreshDisplayMenu();  //刷新显示菜单选项
    void setStatusBarWidget();



private:
    Ui::MainWindow *ui;
    QLabel *screenCoord;
    QLabel *longlatCoord;
    QLabel *scaleNumber;

    MyScene *myScene;  //海图元素绘制场景
    MyView *myView;
    MyDialog* myDialog;  //海图选择窗口

    AdjustDialog *adjustDlg;   //调整参数窗口


private slots:
    void on_safeContour_triggered();
    void on_chartSelect_triggered();
    void on_alarmRecoder_triggered();
    void on_sailRecoder_triggered();

    void display_clicked(int);
    void color_clicked(int);
    void point_clicked(int);
    void area_clicked(int);
    void language_clicked(int);

    void on_openAndClose_triggered();
    void on_rangeAdd_triggered();
    void on_rangeSec_triggered();
    void on_offsetOps_triggered();
    void on_gainOps_triggered();
    void on_restrainOps_triggered();
    void on_clutterOps_triggered();
    void on_tuneMan_triggered();
    void on_tuneAuto_triggered();
    void on_action_2_triggered();
    void on_scaleMatch_triggered();
    void on_closeRadar_triggered();
    void on_showRadar_triggered();
    void on_green_triggered();
    void on_yellow_triggered();
    void on_red_triggered();
    void on_highAlpha_triggered();
    void on_middleAlpha_triggered();
    void on_lowAlpha_triggered();
};



namespace Ui {
    class factorDialog;
}

class FactorDiaog : public QDialog {
    Q_OBJECT
public:
    FactorDiaog(QWidget *parent=0);
    ~FactorDiaog();


protected slots:
    void on_okButton_clicked();

private:
    Ui::factorDialog *ui;

};

namespace Ui {
    class adjustDialog;
}

class AdjustDialog : public QDialog{
    Q_OBJECT
public:
    AdjustDialog(QWidget *parent=0);
    ~AdjustDialog();

    enum dialogType{
        gainDlg = 1,
        restrainDlg,
        clutterDlg,
        tuneDlg
    };


    void setDialogType(dialogType type);
    AdjustDialog::dialogType getDialogType() const
    {   return  dlgType;     }





protected slots:
    void on_decButton_clicked();
    void on_addButton_clicked();

   inline  void setAdjustLabel(QString label);
    inline void setNumBarValue(int number);


private slots:
    void on_enterButton_clicked();

private:
    Ui::adjustDialog *ui;

    dialogType dlgType;   //本次显示的哪个调节窗口

};

#endif // MAINWINDOW_H

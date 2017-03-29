#ifndef MYDIALOG_H
#define MYDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QPushButton>
#include <QDateTimeEdit>
#include <QTableView>


class MyDialog : public QObject
{
Q_OBJECT
public:
    MyDialog(QObject *parent = 0);


    //显示航行记录窗口
    void showSailRecorderWindow(quint32 starttime, quint32 stoptime, quint32 interval);
    //显示报警记录窗口
    void showAlarmRecorderWindow(quint32 starttime, quint32 stoptime);



signals:

public slots:
    void setStartTime(const QDateTime &date);
    void setEndTime(const QDateTime &date);
    void queryRecord(void);
    void closeDialog(void);


private:
    QStandardItemModel *model;
    QTableView *view;
    QDialog *dialog;

    QPushButton* close;   //关闭按钮
    QPushButton* query;   //查询按钮
    QDateTimeEdit*  queryContextStart;   //查询编辑框
    QDateTimeEdit*  queryContextEnd;

    quint32 startTime;
    quint32 endTime;
    bool flag;
    bool flag_window;   //真为航行记录表，假为报警记录表
};

#endif // MYDIALOG_H

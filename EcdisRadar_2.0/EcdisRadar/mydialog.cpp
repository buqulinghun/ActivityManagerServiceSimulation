#include "mydialog.h"
#include "mainwindow.h"
#include "recorder.h"

#include <QHBoxLayout>
#include <QVBoxLayout>




extern MainWindow* lpMainWindow;
extern AlarmRecorder* alarmRecorder;
extern SailRecorder* sailRecorder;



MyDialog::MyDialog(QObject *parent) :
    QObject(parent),startTime(0), endTime(0), flag(false),flag_window(false)
{
    dialog = new QDialog(lpMainWindow);
    model = new QStandardItemModel;
    view = new QTableView;


    QVBoxLayout* mainLayout = new QVBoxLayout;
    QHBoxLayout* bottomLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(1);
    bottomLayout->setContentsMargins(0,0,0,0);
    bottomLayout->setSpacing(1);
    close = new QPushButton(tr("关闭"));
    query = new QPushButton(tr("查询"));
    queryContextStart = new QDateTimeEdit;
    queryContextEnd = new QDateTimeEdit;
    close->setFixedSize(70,30);
    query->setFixedSize(70,30);
    queryContextStart->setFixedSize(150,30);
    queryContextEnd->setFixedSize(150,30);
    queryContextStart->setCalendarPopup(true);
    queryContextEnd->setCalendarPopup(true);
    const QDate dates = QDate(2015, 1, 1);
    startTime = QDateTime(dates).toTime_t();   //设置时间开始
    queryContextStart->setMinimumDate(dates);
    queryContextEnd->setMinimumDate(dates);
    queryContextEnd->setDateTime(QDateTime::fromTime_t(time(0)));

    bottomLayout->addWidget(queryContextStart);
    bottomLayout->addWidget(queryContextEnd);
    bottomLayout->addWidget(query);
    bottomLayout->addWidget(close);
    mainLayout->addWidget(view);
    mainLayout->addLayout(bottomLayout);

    dialog->setLayout(mainLayout);
    dialog->resize(600, 400);
    dialog->close();   //先不显示

    QObject::connect(queryContextStart, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(setStartTime(QDateTime)));
    QObject::connect(queryContextEnd, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(setEndTime(QDateTime)));
    QObject::connect(query, SIGNAL(clicked()), this, SLOT(queryRecord()));
    QObject::connect(close, SIGNAL(clicked()), this, SLOT(closeDialog()));
}



//显示航行记录窗口
void MyDialog::showSailRecorderWindow(quint32 starttime, quint32 stoptime, quint32 interval)
{
    flag_window = true;
   // if(dialog->isVisible())
      //  dialog->close();
    //确保model中的数据已经清空
    if(model->rowCount() != 0)
        model->clear();

    //设置显示标题
    QStringList header;
    header.append(tr("时间"));
    header.append(tr("经度"));
    header.append(tr("纬度"));
    header.append(tr("速度"));
    header.append(tr("航向"));
    header.append(tr("船艏向"));
    model->setHorizontalHeaderLabels(header);

    //获取显示数据
    sailRecorder->getRecord(starttime, stoptime, interval, model);
    view->setModel(model);
    view->resizeColumnsToContents();
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    dialog->setWindowTitle(tr("船舶航行记录查询表"));
    dialog->show();
    dialog->move((lpMainWindow->width() - dialog->width())/2, (lpMainWindow->height() - dialog->height())/2);
}

//显示报警记录窗口
void MyDialog::showAlarmRecorderWindow(quint32 starttime, quint32 stoptime)
{
    flag_window = false;    
   // if(dialog->isVisible())
     //   dialog->close();
    //确保model中的数据已经清空
    if(model->rowCount() != 0)
        model->clear();

    //设置显示标题
    QStringList header;
    header.append(tr("时间"));
    header.append(tr("报警等级"));
    header.append(tr("报警内容"));
    model->setHorizontalHeaderLabels(header);

    //获取显示数据
    alarmRecorder->getRecord(starttime, stoptime, model);
    view->setModel(model);
    view->resizeColumnsToContents();
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    dialog->setWindowTitle(tr("船舶报警记录查询表"));
    dialog->show();
    dialog->move((lpMainWindow->width() - dialog->width())/2, (lpMainWindow->height() - dialog->height())/2);
}


void MyDialog::setStartTime(const QDateTime &date)
{
    startTime = date.toTime_t();
    flag = true;
}
void MyDialog::setEndTime(const QDateTime &date)
{
    endTime = date.toTime_t();
    flag = true;
}
void MyDialog::queryRecord()
{
    if(flag) {
        flag = false;

        if(flag_window)
            showSailRecorderWindow(startTime, endTime, 60);
        else
            showAlarmRecorderWindow(startTime, endTime);
    }
}
void MyDialog::closeDialog()
{
    dialog->close();
    model->clear();
}

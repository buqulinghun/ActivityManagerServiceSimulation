/********************************************************************
 *日期: 2016-03-28
 *作者: 王名孝
 *作用: 存储系统电子海图数据的接口，提供保存/读取/显示接口等操作,利用单独的一个dialog进行显示
 *修改:
 ********************************************************************/
#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QMutex>
#include <QStandardItemModel>
#include <QTableView>
#include <QDialog>
#include <QPushButton>

#include "sqlite3/sqlite3.h"
#include "s57/s57chart.h"
#include "chart.h"
#include "mercatorproj.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <iostream>
#include <QSqlError>
#include <vector>




class DataBase : public QObject
{
    Q_OBJECT
public:
    DataBase(QObject *parent = NULL);
    ~DataBase();

    //存储一个系统电子海图数据文件
    bool addSencRecord(Chart* newchart, quint32 addtime);
    //查询一个海图记录
    bool getSencRecord(const QString& name, QStandardItemModel* model);
    //显示海图选择窗口
    void showChartSelectWindow(void);


    QPointF screenToLatitude(double x, double y)     //屏幕坐标转换为经纬度
    {
        return mercatorProj->screenToLatitude(x, y, nowChart);
    }

protected:
    bool openDatabase();   //打开数据库
    void closeDatabase();   //关闭数据库
    void openChart(const QString &filePath);  //打开海图数据库文件

    //加载一幅海图
    bool LoadChartHelper(const QString &fileName);
    //保存海图到数据库中
    bool SaveChartDataBase(S57Chart* chartData);


public slots:
    void queryRecord(void);   //查询海图
    void closeDialog(void);   //关闭窗口
    void setTextFlag(void);   //查询内容是否输入
    void openChartClicked(void);    //打开海图
    void addChart(void);    //添加海图原始文件
    void doubleRowClicked(const QModelIndex &index);   //双击选取


private:
    /*数据库锁，因为并没有多个同时读取的情况，所以就用这个，考虑使用QReadWriteLock  */
    QMutex mutex;

    sqlite3* data_db;   //海图信息数据库
    QString dataBaseName;
    sqlite3* chart_db;   //海图打开保存数据库
    QSqlDatabase mysqlDB;//MySQL数据库

    /////////////显示部分界面////////////
    QStandardItemModel *model;
    QTableView *view;
    QDialog *dialog;

    QPushButton* close;   //关闭按钮
    QPushButton* query;   //查询按钮
    QPushButton* open;    //打开海图按钮
    QPushButton* add;    //添加海图按钮
    QLineEdit*  queryContext;   //查询编辑框
    bool flag;

    //墨卡托投影类
    MercatorProj *mercatorProj;
    QSqlDatabase db;
    QSqlQuery *sqlQuery;

    void initMysql();
    void createMysqlTable(QString name,QString instruction);

    int getSizeFromTable(QString& sql);

    bool isExist(Chart* pChart);
    void saveDspmMysql(Chart* pChart);
    void saveFeaturesMysql(Chart* pChart);
    void saveFeaturesMysql(SpaceObject* pso,Chart* pChart);
    void saveFeatureAttfs(SpaceObject* pso,Chart* pChart);
    void saveFeatureNatfs(SpaceObject* pso,Chart* pChart);
    void saveFeatureFfpts(SpaceObject* pso,Chart* pChart);
    void saveFeatureFspts(SpaceObject* pso,Chart* pChart);
    void saveFeatureAttvs(SpaceObject *pso,const std::vector<attv_t> &attvs);

    void updateSpaceFeature(SpaceObject* pso,Chart* pChart);




public:
    //////////保存海图数据/////////////
    QVector<Chart *> ChartData;  //需要显示的海图数据存储

    Chart *nowChart;   //当前显示的海图
signals:
    void readyToRender(std::string description);
    void readyToRenderOther(QString chartName, bool, bool, bool);


};

#endif // DATABASE_H

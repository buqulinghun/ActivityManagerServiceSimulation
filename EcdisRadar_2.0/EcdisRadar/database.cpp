/********************************************************************
 *日期: 2016-03-28
 *作者: 王名孝
 *作用: 存储系统电子海图数据的接口，提供保存/读取/显示接口等操作
 *修改:
 ********************************************************************/
#include "database.h"
#include "mainwindow.h"
#include "ecdis.h"

#include <QDebug>
#include <QMutexLocker>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QStringList>
#include <QSqlError>
#include <QTime>
using namespace std;


extern MainWindow* lpMainWindow;
extern MARINERSELECT MarinerSelect;

extern QString Longitude2String(double lon, bool enflag);
extern QString Latitude2String(double lat, bool enflag);




DataBase::DataBase(QObject* parent): QObject(parent), flag(false)
{
    //创建mysql表格
    initMysql();

    mercatorProj = new MercatorProj;

    data_db = NULL;
    chart_db = NULL;
    nowChart = NULL;
    dataBaseName = SOFTWAREPATH + QString("data/chartData.db");

    if(!openDatabase())
        qDebug() << "cann't open chart database success!";


    ////////设置显示界面//////////////////////
    dialog = new QDialog(lpMainWindow);
    model = new QStandardItemModel;
    view = new QTableView;
    view->setSelectionBehavior(QAbstractItemView::SelectRows);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    QHBoxLayout* bottomLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(1);
    bottomLayout->setContentsMargins(0,0,0,0);
    bottomLayout->setSpacing(1);
    close = new QPushButton(tr("关闭"));
    query = new QPushButton(tr("查询"));
    open = new QPushButton(tr("打开"));
    add = new QPushButton(tr("添加海图"));
    queryContext = new QLineEdit;
    close->setFixedSize(70, 30);
    query->setFixedSize(70, 30);
    open->setFixedSize(70, 30);
    queryContext->setFixedSize(150, 30);   //前面可以加个Label，现在木有时间美化
    add->setFixedSize(130, 30);

    bottomLayout->addWidget(queryContext);
    bottomLayout->addWidget(query);
    bottomLayout->addWidget(open);
    bottomLayout->addWidget(close);
    bottomLayout->addWidget(add);
    mainLayout->addWidget(view);
    mainLayout->addLayout(bottomLayout);

    dialog->setLayout(mainLayout);
    dialog->resize(600, 400);

    QObject::connect(queryContext, SIGNAL(textChanged(QString)), this, SLOT(setTextFlag()));
    QObject::connect(query, SIGNAL(clicked()), this, SLOT(queryRecord()));
    QObject::connect(close, SIGNAL(clicked()), this, SLOT(closeDialog()));
    QObject::connect(open, SIGNAL(clicked()), this, SLOT(openChartClicked()));
    QObject::connect(add, SIGNAL(clicked()), this, SLOT(addChart()));
    QObject::connect(view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doubleRowClicked(QModelIndex)));
}

DataBase::~DataBase()
{
    closeDatabase();

    QVector<Chart *>::const_iterator ch_it;
    QVector<Chart *>::const_iterator ch_end(ChartData.end());
    for (ch_it = ChartData.begin(); ch_it != ch_end; ++ch_it)
        delete *ch_it;

    delete sqlQuery;
}

//添加信息记录
bool DataBase::addSencRecord(Chart *newchart, quint32 addtime)
{
    //数据锁定
    QMutexLocker locker(&mutex);

    int result;
    const quint32 time0 = addtime;  //更新时间
    const QString tableName = QString("ChartTable");

    // 插入记录
    QString insert = QString("INSERT INTO '%1' VALUES ('%2', '%3', '%4', '%5', '%6', '%7', '%8', '%9', '%10', '%11')").arg(tableName)  \
                     .arg(QString::fromStdString(newchart->description)).arg(QString::fromStdString(newchart->dsid.dataset_name)).arg(QString::fromStdString(newchart->dsid.edition_num)) \
                   .arg(QString::fromStdString(newchart->dsid.update_num)).arg(time0).arg(newchart->dspm.comp_sod)  \
                   .arg(newchart->slat.minutes).arg(newchart->wlon.minutes).arg(newchart->nlat.minutes).arg(newchart->elon.minutes);
    result = sqlite3_exec(data_db, insert.toLocal8Bit().constData(), 0, 0, 0);


    return (SQLITE_OK == result);

}


#define SQLSELECT(tbl,tm1) QString("SELECT * FROM '%1' WHERE chartnum == '%2'").arg(tbl).arg(tm1);
#define SetTableItem(model, row, column, data)	\
{		\
    QStandardItem *item = new QStandardItem;	\
    item->setData(data, Qt::DisplayRole);      \
    model->setItem(row, column, item);		\
}
#define SetTableItem2(model, row, column, disp, data)	\
{							\
    QStandardItem *item = new QStandardItem;	       \
    item->setData(disp, Qt::DisplayRole);		\
    item->setData(data, Qt::UserRole);			\
    model->setItem(row, column, item);			\
}
//获取海图信息.flag为真精确匹配,假全部读取,忽略名字
bool DataBase::getSencRecord(const QString &name, QStandardItemModel *model)
{
    QMutexLocker locker(&mutex);

    sqlite3_stmt* stmt;
    int result, row = 0;

    //model->clear();
    const QString tableName = QString("ChartTable");
    if(!name.isEmpty()) {  //精确查找
        const QString select = SQLSELECT(tableName, name);
        result = sqlite3_prepare_v2(data_db, select.toLocal8Bit().constData(), select.size(), &stmt, NULL);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_prepare find chart\n");
            fprintf(stderr, "%s", sqlite3_errmsg(data_db));
            return false;
        }
        do
        {
            result = sqlite3_step(stmt);
            if(SQLITE_ROW == result)
            {
                const QString chartnum = QString::fromLocal8Bit((const char*)sqlite3_column_text(stmt, 0));
                const QString chartname = QString::fromLocal8Bit((const char*)sqlite3_column_text(stmt, 1));
                const QString editnum = QString::fromLocal8Bit((const char*)sqlite3_column_text(stmt, 2));
                const QString updatenum = QString::fromLocal8Bit((const char*)sqlite3_column_text(stmt, 3));
                const int updatetime = sqlite3_column_int(stmt, 4);
                const QString strDate = QDateTime::fromTime_t(updatetime).toString("yyyy-MM-dd hh:mm:ss");
                const int scale = sqlite3_column_int(stmt, 5);
                const double llat = sqlite3_column_double(stmt, 6);   //得到度数
                const double llon = sqlite3_column_double(stmt, 7);
                const double rlat = sqlite3_column_double(stmt, 8);
                const double rlon = sqlite3_column_double(stmt, 9);

                SetTableItem(model, row, 0, chartnum);
                SetTableItem(model, row, 1, chartname);
                SetTableItem(model, row, 2, editnum);
                SetTableItem(model, row, 3, updatenum);
                SetTableItem(model, row, 4, strDate);
                SetTableItem(model, row, 5, scale);
                SetTableItem(model, row, 6, Latitude2String(llat, true));
                SetTableItem(model, row, 7, Longitude2String(llon, true));
                SetTableItem(model, row, 8, Latitude2String(rlat, true));
                SetTableItem(model, row, 9, Longitude2String(rlon, true));
                row ++;
            }
            else
                break;
        }while(1);

        result = sqlite3_finalize(stmt);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_finalize\n");
        }

    }else {   //查找所有海图信息
        const QString select = QString("SELECT * FROM '%1'").arg(tableName);
        result = sqlite3_prepare_v2(data_db, select.toLocal8Bit().constData(), select.size(), &stmt, NULL);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_prepare find chart all\n");
            fprintf(stderr, "%s", sqlite3_errmsg(data_db));
            return false;
        }
        do
        {
            result = sqlite3_step(stmt);
            if(SQLITE_ROW == result)
            {
                const QString chartnum = QString::fromLocal8Bit((const char*)sqlite3_column_text(stmt, 0));
                const QString chartname = QString::fromLocal8Bit((const char*)sqlite3_column_text(stmt, 1));
                const QString editnum = QString::fromLocal8Bit((const char*)sqlite3_column_text(stmt, 2));
                const QString updatenum = QString::fromLocal8Bit((const char*)sqlite3_column_text(stmt, 3));
                const int updatetime = sqlite3_column_int(stmt, 4);
                const QString strDate = QDateTime::fromTime_t(updatetime).toString("yyyy-MM-dd hh:mm:ss");
                const int scale = sqlite3_column_int(stmt, 5);
                const double llat = sqlite3_column_double(stmt, 6);   //得到度数
                const double llon = sqlite3_column_double(stmt, 7);
                const double rlat = sqlite3_column_double(stmt, 8);
                const double rlon = sqlite3_column_double(stmt, 9);
                SetTableItem(model, row, 0, chartnum);
                SetTableItem(model, row, 1, chartname);
                SetTableItem(model, row, 2, editnum);
                SetTableItem(model, row, 3, updatenum);
                SetTableItem(model, row, 4, strDate);
                SetTableItem(model, row, 5, scale);
                SetTableItem(model, row, 6, Latitude2String(llat, true));
                SetTableItem(model, row, 7, Longitude2String(llon, true));
                SetTableItem(model, row, 8, Latitude2String(rlat, true));
                SetTableItem(model, row, 9, Longitude2String(rlon, true));
                row ++;
            }
            else
                break;
        }while(1);

        result = sqlite3_finalize(stmt);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_finalize\n");
        }
    }


    return true;
}
//显示海图选择对话框
void DataBase::showChartSelectWindow()
{
    //确保model中的数据已经清空
    if(model->rowCount() != 0)
        model->clear();

    //设置显示标题
    QStringList header;
    header.append(tr("图号"));
    header.append(tr("图名"));
    header.append(tr("版本号"));
    header.append(tr("升级号"));
    header.append(tr("更新日期"));
    header.append(tr("比例尺"));
    header.append(tr("左下角纬度"));
    header.append(tr("左下角经度"));
    header.append(tr("右上角纬度"));
    header.append(tr("右上角经度"));
    model->setHorizontalHeaderLabels(header);
    //获取显示数据
    getSencRecord("", model);
    view->setModel(model);
    view->resizeColumnsToContents();
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    dialog->setWindowTitle(tr("系统电子海图查询列表"));
   // dialog->setWindowFlags(Qt::Widget|Qt::FramelessWindowHint);
  //  dialog->move(QPoint(300,200));
    dialog->show();
}


bool DataBase::openDatabase()
{
    if(data_db)
        return true;

    int result;
    result = sqlite3_open_v2(dataBaseName.toLocal8Bit().constData(),
                             &data_db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, NULL);    //可以设置标志位
    if(result != SQLITE_OK) {
        fprintf(stderr, "cann't open database: %s \n", sqlite3_errmsg(data_db));
        sqlite3_close(data_db);
        data_db = NULL;
        return false;
    }

    return true;
}

void DataBase::closeDatabase()
{
    if(!data_db)
        return;

    sqlite3_close(data_db);
    data_db = NULL;
}


//查询操作
void DataBase::queryRecord()
{
    if(flag) {
        flag = false;
        const QString context = queryContext->text();
        if(context.isEmpty())
            return;

        //确保model中的数据已经清空
        if(model->rowCount() != 0)
            model->clear();

        //获取显示数据
        getSencRecord(context, model);
        //设置显示标题
        QStringList header;
        header.append(tr("图号"));
        header.append(tr("图名"));
        header.append(tr("版本号"));
        header.append(tr("升级号"));
        header.append(tr("更新日期"));
        header.append(tr("比例尺"));
        header.append(tr("左下角纬度"));
        header.append(tr("左下角经度"));
        header.append(tr("右上角纬度"));
        header.append(tr("右上角经度"));
        model->setHorizontalHeaderLabels(header);

        view->resizeColumnsToContents();
        view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }

}

void DataBase::closeDialog()
{
    dialog->close();
    model->clear();
}

void DataBase::setTextFlag()
{
    //设置查询内容变化标志位
    flag = true;
}
void DataBase::openChartClicked()
{
    QItemSelectionModel *selections = view->selectionModel();
    QModelIndexList selected = selections->selectedIndexes();
    if(selected.isEmpty())
        return;
    int row = selected.at(0).row();
    const QString openFileName = SOFTWAREPATH + QString("data/SENC/")
            + model->data(model->index(row, 0)).toString() + QString(".wmc");
    MarinerSelect.chartScaleNum = model->data(model->index(row, 5)).toInt();   //设置显示比例尺
    //检测是否已经打开过
    QVector<Chart *>::iterator cit;
    QVector<Chart *>::iterator cend = ChartData.end();
    for(cit = ChartData.begin(); cit != cend; ++cit) {
        if((*cit)->filepath.string() == openFileName.toStdString()) {
            nowChart = *cit;
            emit readyToRenderOther(QString::fromStdString((*cit)->description),true,true,true);  //发送绘图信号
            return;
        }
    }
    //没有再打开原始海图
    openChart(openFileName);
}

/**
 *@desc 初始化mysql
 *@pram
 *
*/
void DataBase::initMysql()
{
    db = QSqlDatabase::addDatabase("QMYSQL");

    db.setHostName("localhost");
    db.setDatabaseName("geodatabase");
    db.setUserName("root");
    db.setPassword("123");


    if(!db.open()){
        qDebug()<<"Unable to open database";
    }else{
        qDebug()<<"Database connection established";
    }

  //  if(!sqlQuery)
       sqlQuery = new QSqlQuery(db);


    /*********************************************** 建表语句 begin *****************************************************/
    QString DSPMCreate = QString("create table if not exists dspm (dspm_id integer primary key auto_increment, \
                                 locationName varchar(50),\
                                 coord_mult_factor integer,  \
                                 horz_gd_datum integer,  \
                                 vert_datum integer,  \
                                 sounding_datum integer,  \
                                 comp_sod integer,  \
                                 depth_unit integer,  \
                                 height_unit integer,  \
                                 accuracy_unit integer,  \
                                 coord_unit integer,  \
                                 sounding_mult_factor integer,  \
                                 feature_start integer,  \
                                 feature_end integer)");




            QString FeatureCreate = QString("create table if not exists feature(id integer primary key auto_increment,\
                                            name varchar(50),\
                                            object_label integer,\
                                            record_id integer,\
                                            grp integer,\
                                            prim integer,\
                                            feature_id integer,\
                                            feature_subid integer,\
                                            attfs_start integer,\
                                            attfs_end integer,\
                                            natfs_start integer,\
                                            natfs_end integer,\
                                            ffpts_start integer,\
                                            ffpts_end integer,\
                                            fspts_start integer,\
                                            fspts_end integer,\
                                            priority integer,\
                                            discategory integer,\
                                            overRadar BOOL,\
                                            line linestring,\
                                            pgn Polygon,\
                                            point Point,\
                                            attvs_start integer,\
                                            attvs_end integer,\
                                            soundPoints MultiPoint,\
                                            depth TEXT )");



            QString attfsCreate = QString("create table if not exists attfs(attfs_id integer primary key auto_increment,\
                                          record_id integer,\
                                          attl integer,\
                                          atvl TEXT)");



            QString natfsCreate = QString("create table if not exists natfs(natfs_id integer primary key auto_increment,\
                                          record_id integer,\
                                          attl integer,\
                                          atvl TEXT)");



            QString ffptsCreate = QString("create table if not exists ffpts(ffpts_id integer primary key auto_increment,\
                                          record_id integer,\
                                          agen integer,\
                                          find integer,\
                                          fids integer,\
                                          rind integer,\
                                          comt TEXT )");
            /*********************************************** 建表语句 end *****************************************************/

            QString fsptsCreate = QString("create table if not exists fspts(fspts_id integer primary key auto_increment,\
                                          record_id integer,\
                                          rcnm integer,\
                                          rcid integer,\
                                          ornt integer,\
                                          usag integer,\
                                          mask integer)");

            QString attvCreate = QString("create table if not exists attvs(attvs_id integer primary key auto_increment,\
                                         record_id integer,\
                                         attl integer,\
                                         atvl Text)");

           createMysqlTable("dspm",DSPMCreate);
           createMysqlTable("feature",FeatureCreate);
           createMysqlTable("attfs",attfsCreate);
           createMysqlTable("natfs",natfsCreate);
           createMysqlTable("ffpts",ffptsCreate);
           createMysqlTable("fspts",fsptsCreate);
           createMysqlTable("attvs",attvCreate);
}

/**
 *@desc 根据建表语句创建数据库表
 *@pram 建表语句
 *
*/
void DataBase::createMysqlTable(QString name,QString instruction)
{
  bool sucess = sqlQuery->exec(instruction);
  if(!sucess)
  {
      QSqlError error =sqlQuery->lastError();
      qDebug()<<"error,when create table at:"<<name<<" Error:"<<error.databaseText();
  }

}

void DataBase::doubleRowClicked(const QModelIndex &index)
{
    int row = index.row();
    const QString openFileName = SOFTWAREPATH + QString("data/SENC/") + model->data(model->index(row, 0)).toString() + QString(".wmc");
    MarinerSelect.chartScaleNum = model->data(model->index(row, 5)).toInt();   //设置显示比例尺
    //检测是否已经打开过
    QVector<Chart *>::iterator cit;
    QVector<Chart *>::iterator cend = ChartData.end();
    for(cit = ChartData.begin(); cit != cend; ++cit) {
        if((*cit)->filepath.string() == openFileName.toStdString()) {
            nowChart = *cit;
            emit readyToRenderOther(QString::fromStdString((*cit)->description),true,true,true);  //发送绘图信号
            return;
        }
    }
    //没有再打开原始海图
    openChart(openFileName);
}

//打开SENC海图
void DataBase::openChart(const QString &filePath)
{
    QTime tt;
    tt.start();

    if(chart_db) {
        sqlite3_close(chart_db);
        chart_db = NULL;
    }
    //读取SENC数据进行绘图操作,以该图中心作为屏幕中心
    int result = 0;
    result = sqlite3_open_v2(filePath.toLocal8Bit().constData(),
                             &chart_db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, NULL);    //可以设置标志位
    if(result != SQLITE_OK) {
        fprintf(stderr, "cann't open database %s: %s \n", filePath.toLocal8Bit().constData(), sqlite3_errmsg(chart_db));
        sqlite3_close(chart_db);
        chart_db = NULL;
        return;
    }

    sqlite3_stmt* stmt;
    Chart* openChart = new Chart;
    openChart->filepath = FilePath(filePath.toStdString());

    //读取基本信息//////////////////////////////////////////////////////////////////////////////////////////////////////////////
    const QString select_desc = QString("SELECT * FROM '%1'").arg("DESCRIPTION");
    sqlite3_prepare_v2(chart_db, select_desc.toLocal8Bit().constData(), select_desc.size(), &stmt, NULL);
    result = sqlite3_step(stmt);
    if(result == SQLITE_ROW) {
        openChart->description = (const char *)sqlite3_column_text(stmt, 0);
    }else {
        fprintf(stderr, "SQL error: sqlite3_step get description\n");
        return;
    }
    sqlite3_finalize(stmt);
    //////////////////////////////////////////////////////
    const QString select_dsid = QString("SELECT * FROM '%1'").arg("DSID");
    sqlite3_prepare_v2(chart_db, select_dsid.toLocal8Bit().constData(), select_dsid.size(), &stmt, NULL);
    result = sqlite3_step(stmt);
    if(result == SQLITE_ROW) {
        openChart->dsid.dataset_name = (const char*)sqlite3_column_text(stmt, 0);
        openChart->dsid.edition_num = (const char*)sqlite3_column_text(stmt, 1);
        openChart->dsid.issue_date = (const char*)sqlite3_column_text(stmt, 2);
        openChart->dsid.update_num = (const char*)sqlite3_column_text(stmt, 3);
        openChart->dsid.s57_edition = sqlite3_column_int(stmt, 4);
        openChart->dsid.producing_agency = sqlite3_column_int(stmt, 5);
        openChart->dsid.data_struct = sqlite3_column_int(stmt, 6);
        openChart->dsid.attf_lexl = sqlite3_column_int(stmt, 7);
        openChart->dsid.natf_lexl = sqlite3_column_int(stmt, 8);
        openChart->dsid.num_meta_rec = sqlite3_column_int(stmt, 9);
        openChart->dsid.num_cart_rec = sqlite3_column_int(stmt, 10);
        openChart->dsid.num_geo_rec = sqlite3_column_int(stmt, 11);
        openChart->dsid.num_coll_rec = sqlite3_column_int(stmt, 12);
        openChart->dsid.num_isonode_rec = sqlite3_column_int(stmt, 13);
        openChart->dsid.num_connnode_rec = sqlite3_column_int(stmt, 14);
        openChart->dsid.num_edge_rec = sqlite3_column_int(stmt, 15);
        openChart->dsid.num_face_rec = sqlite3_column_int(stmt, 16);
    }else {
        fprintf(stderr, "SQL error: sqlite3_step get dsid");
        return;
    }
    sqlite3_finalize(stmt);
    //////////////////////////////////////////////////////
    const QString select_dspm = QString("SELECT * FROM '%1'").arg("DSPM");
    sqlite3_prepare_v2(chart_db, select_dspm.toLocal8Bit().constData(), select_dspm.size(), &stmt, NULL);
    result = sqlite3_step(stmt);
    if(result == SQLITE_ROW) {
        memcpy(&openChart->dspm, (const char *)sqlite3_column_blob(stmt, 0), sizeof(DSPM));
    }else {
        fprintf(stderr, "SQL error: sqlite3_step get dspm");
        return;
    }
    sqlite3_finalize(stmt);
    /////////////////////////////////////////////////
    const QString select_range = QString("SELECT * FROM '%1'").arg("RANGE");
    sqlite3_prepare_v2(chart_db, select_range.toLocal8Bit().constData(), select_range.size(), &stmt, NULL);
    result = sqlite3_step(stmt);
    if(result == SQLITE_ROW) {
        openChart->slat = sqlite3_column_double(stmt, 0);
        openChart->wlon = sqlite3_column_double(stmt, 1);
        openChart->nlat = sqlite3_column_double(stmt, 2);
        openChart->elon = sqlite3_column_double(stmt, 3);
    }else {
        fprintf(stderr, "SQL error: sqlite3_step get range");
        return;
    }
    sqlite3_finalize(stmt);
    ///////////////////////////////////////////////////////////////////
    const QString select_sg2d = QString("SELECT * FROM '%1'").arg("SG2D");
    sqlite3_prepare_v2(chart_db, select_sg2d.toLocal8Bit().constData(), select_sg2d.size(), &stmt, NULL);
    result = sqlite3_step(stmt);
    if(result == SQLITE_ROW) {
        sg2d_t* sg2ds = NULL;
        int sg2d_size = sqlite3_column_bytes(stmt, 0) / sizeof(sg2d_t);
        sg2ds = (sg2d_t*)sqlite3_column_blob(stmt, 0);
        for(int i=0; i<sg2d_size; i++) {
            openChart->sg2ds.push_back(sg2ds[i]);
        }
    }else {
        fprintf(stderr, "SQL error: sqlite3_step get sg2d");
        return;
    }
    sqlite3_finalize(stmt);
    ///////////////依次读取数据库中的属性信息，对应一个物标，每次读取创建一个物标并填充其他信息
    char *errmsg = NULL;
    char **featureResult, **attfResult, **natfResult, **ffptResult, **fsptResult;
    int nRow, nColumn, attf_nrow, attf_ncol, natf_nrow, natf_ncol, ffpt_nrow, ffpt_ncol, fspt_nrow, fspt_ncol;
    int index, index_prim, index_attfs, index_natfs, index_ffpts, index_fspts;

    const QString select = QString("SELECT * FROM '%1'").arg("FEATURES");
    result = sqlite3_get_table(chart_db, select.toLocal8Bit().constData(), &featureResult, &nRow, &nColumn, &errmsg );
    if( SQLITE_OK == result ){     //查询成功
        const QString select_attfs = QString("SELECT * FROM '%1'").arg("ATTFS");
        result = sqlite3_get_table(chart_db, select_attfs.toLocal8Bit().constData(), &attfResult, &attf_nrow, &attf_ncol, &errmsg);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_get_table attfs all\n");
            fprintf(stderr, "%s", sqlite3_errmsg(chart_db));
            return;
        }
        const QString select_natfs = QString("SELECT * FROM '%1'").arg("NATFS");
        result = sqlite3_get_table(chart_db, select_natfs.toLocal8Bit().constData(), &natfResult, &natf_nrow, &natf_ncol, &errmsg);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_get_table natfs all\n");
            fprintf(stderr, "%s", sqlite3_errmsg(chart_db));
            return;
        }
        const QString select_ffpts = QString("SELECT * FROM '%1'").arg("FFPTS");
        result = sqlite3_get_table(chart_db, select_ffpts.toLocal8Bit().constData(), &ffptResult, &ffpt_nrow, &ffpt_ncol, &errmsg);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_get_table ffpts all\n");
            fprintf(stderr, "%s", sqlite3_errmsg(chart_db));
            return;
        }
        const QString select_fspts = QString("SELECT * FROM '%1'").arg("FSPTS");
        result = sqlite3_get_table(chart_db, select_fspts.toLocal8Bit().constData(), &fsptResult, &fspt_nrow, &fspt_ncol, &errmsg);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_get_table fspts all\n");
            fprintf(stderr, "%s", sqlite3_errmsg(chart_db));
            return;
        }

#define getSencRecord(point_object)  {  \
        index++;\
        point_object.record_id = atoi(featureResult[index++]);\
        point_object.object_label = atoi(featureResult[index++]);\
        point_object.group = atoi(featureResult[index++]);\
        point_object.prim = prim;   index++;\
        point_object.feature_id = atoi(featureResult[index++]);\
        point_object.feature_subid = atoi(featureResult[index++]);\
        point_object.name = (const char*)featureResult[index++];\
        const int attf_num = atoi(featureResult[index++]);\
        const int natf_num = atoi(featureResult[index++]);\
        const int ffpt_num = atoi(featureResult[index++]);\
        const int fspt_num = atoi(featureResult[index++]);\
        for(int j=0; j<attf_num; j++) {\
            attf_t attf;\
            index_attfs++;\
            int x = atoi(attfResult[index_attfs++]);\
            if(point_object.record_id != x) {\
                qDebug() << "error while load attfs";\
                return;\
            }\
            attf.attl = atoi(attfResult[index_attfs++]);  \
            attf.atvl = (const char*)attfResult[index_attfs++]; \
            point_object.attfs.push_back(attf);\
        }\
        for(int j=0; j<natf_num; j++) {\
            attf_t natf;\
            index_natfs++;\
            if(point_object.record_id != atoi(natfResult[index_natfs++])) {\
                qDebug() << "error while load natfs";\
                return;\
            }\
            natf.attl = atoi(natfResult[index_natfs++]);\
            natf.atvl = (const char*)natfResult[index_natfs++];\
            point_object.natfs.push_back(natf);\
        }\
        for(int j=0; j<ffpt_num; j++) {\
            ffpt_t ffpt;\
            index_ffpts++;\
            if(point_object.record_id != atoi(ffptResult[index_ffpts++])) {\
                qDebug() << "error while load ffpts";\
                return;\
            }\
            ffpt.agen = atoi(ffptResult[index_ffpts++]);\
            ffpt.find = atoi(ffptResult[index_ffpts++]);\
            ffpt.fids = atoi(ffptResult[index_ffpts++]);\
            ffpt.rind = atoi(ffptResult[index_ffpts++]);\
            ffpt.comt = (const char *)ffptResult[index_ffpts++];\
            point_object.ffpts.push_back(ffpt);\
        }\
        for(int j=0; j<fspt_num; j++) {\
            fspt_t fspt;\
            index_fspts++;\
            if(point_object.record_id != atoi(fsptResult[index_fspts++])) {\
                qDebug() << "error while load fspts";\
                return;\
            }\
            fspt.rcnm = atoi(fsptResult[index_fspts++]);\
            fspt.rcid = atoi(fsptResult[index_fspts++]);\
            fspt.ornt = atoi(fsptResult[index_fspts++]);\
            fspt.usag = atoi(fsptResult[index_fspts++]);\
            fspt.mask = atoi(fsptResult[index_fspts++]);\
            point_object.fspts.push_back(fspt);\
        }\
    }

        //featureResult 前面第一行数据是字段名称，从 nColumn 索引开始才是真正的数据
        index = nColumn;
        index_attfs = attf_ncol;
        index_natfs = natf_ncol;
        index_ffpts = ffpt_ncol;
        index_fspts = fspt_ncol;
        for(int i = 0; i < nRow ; i++ ){     //读取每行数据
            index_prim = index + 4;
            const int prim = atoi(featureResult[index_prim]);
            switch(prim) {
                case 1:
                {
                    int object_label = atoi(featureResult[index+2]);
                    if(object_label == 129){  //SOUND
                        SoundObject sound_object;
                        getSencRecord(sound_object);
                        openChart->sound_object_vector.push_back(sound_object);
                    }else {
                        PointObject point_object;
                        getSencRecord(point_object);
                        openChart->point_objects_map[point_object.object_label].push_back(point_object);
                    }
                }
                break;
                case 2:
                {
                    LineObject line_object;
                    getSencRecord(line_object);
                    openChart->line_objects_map[line_object.object_label].push_back(line_object);
                }
                break;
                case 3:
                {
                    AreaObject area_object;
                    getSencRecord(area_object);
                    openChart->area_objects_map[area_object.object_label].push_back(area_object);
                }
                break;
            }  //end switch

        }
    }   //到这里，不论数据库查询是否成功，都释放 char** 查询结果，使用 sqlite 提供的功能来释放
    sqlite3_free_table(featureResult);
    sqlite3_free_table(attfResult);
    sqlite3_free_table(natfResult);
    sqlite3_free_table(ffptResult);
    sqlite3_free_table(fsptResult);
    ////////////////////读取空间物标信息
    char **isolatedResult, **connectedResult, **edgesResult, **attvResult, **sg3dResult, **index2dResult;
    int isolated_nrow, isolated_ncol, connected_nrow, connected_ncol, edges_nrow, edges_ncol, attv_nrow, attv_ncol, sg3d_nrow, sg3d_ncol, index2d_nrow, index2d_ncol;
    int index_isolated, index_connected, index_edges, index_attvs, index_sg3ds, index_index2d;

    const QString select_isola = QString("SELECT * FROM '%1'").arg("ISOLATED");
    result = sqlite3_get_table(chart_db,
                        select_isola.toLocal8Bit().constData(), &isolatedResult, &isolated_nrow, &isolated_ncol, &errmsg);
    if( result != SQLITE_OK ){
        fprintf(stderr, "SQL error: sqlite3_get_table isolated all\n");
        fprintf(stderr, "%s", sqlite3_errmsg(chart_db));
        return;
    }
    const QString select_attvs = QString("SELECT * FROM '%1'").arg("ATTVS");
    result = sqlite3_get_table(chart_db, select_attvs.toLocal8Bit().constData(), &attvResult, &attv_nrow, &attv_ncol, &errmsg);
    if( result != SQLITE_OK ){
        fprintf(stderr, "SQL error: sqlite3_get_table attvs all\n");
        fprintf(stderr, "%s", sqlite3_errmsg(chart_db));
        return;
    }
    const QString select_conn = QString("SELECT * FROM '%1'").arg("CONNECTED");
    result = sqlite3_get_table(chart_db, select_conn.toLocal8Bit().constData(), &connectedResult, &connected_nrow, &connected_ncol, &errmsg);
    if( result != SQLITE_OK ){
        fprintf(stderr, "SQL error: sqlite3_get_table connected all\n");
        fprintf(stderr, "%s", sqlite3_errmsg(chart_db));
        return;
    }
    const QString select_edge = QString("SELECT * FROM '%1'").arg("EDGES");
    result = sqlite3_get_table(chart_db, select_edge.toLocal8Bit().constData(), &edgesResult, &edges_nrow, &edges_ncol, &errmsg);
    if( result != SQLITE_OK ){
        fprintf(stderr, "SQL error: sqlite3_get_table edges all\n");
        fprintf(stderr, "%s", sqlite3_errmsg(chart_db));
        return;
    }
    const QString select_sg3d = QString("SELECT * FROM '%1'").arg("SG3DS");
    result = sqlite3_get_table(chart_db, select_sg3d.toLocal8Bit().constData(), &sg3dResult, &sg3d_nrow, &sg3d_ncol, &errmsg);
    if( result != SQLITE_OK ){
        fprintf(stderr, "SQL error: sqlite3_get_table sg3ds all\n");
        fprintf(stderr, "%s", sqlite3_errmsg(chart_db));
        return;
    }
    const QString select_index2d = QString("SELECT * FROM '%1'").arg("SG2DINDEX");
    result = sqlite3_get_table(chart_db, select_index2d.toLocal8Bit().constData(), &index2dResult, &index2d_nrow, &index2d_ncol, &errmsg);
    if( result != SQLITE_OK ){
        fprintf(stderr, "SQL error: sqlite3_get_table index2ds all\n");
        fprintf(stderr, "%s", sqlite3_errmsg(chart_db));
        return;
    }

    index_isolated = isolated_ncol;
    index_connected = connected_ncol;
    index_edges = edges_ncol;
    index_attvs = attv_ncol;
    index_sg3ds = sg3d_ncol;
    index_index2d = index2d_ncol;
    for(int i = 0; i < isolated_nrow ; i++ ){     //读取孤立点数据
        index_isolated++;
        const int record_id = atoi(isolatedResult[index_isolated++]);
        IsolatedNodeVector isolateVector;
        isolateVector.sg2d_index = atoi(isolatedResult[index_isolated++]);
        const int size = atoi(isolatedResult[index_isolated++]);

        //将isolateVector.sg3ds尺寸字段储存在表Isolate中，在SG3DS中根据size取对应的字段
        for(int j=0; j<size; j++) {
            sg3d_t sg3d;
            index_sg3ds++;
            if(record_id != atoi(sg3dResult[index_sg3ds++])) {
                qDebug() << "error while load isolated sg3ds";
                return;
            }
            sg3d.long_lat[0] = atof(sg3dResult[index_sg3ds++]);
            sg3d.long_lat[1] = atof(sg3dResult[index_sg3ds++]);
            sg3d.depth = atof(sg3dResult[index_sg3ds++]);
            isolateVector.sg3ds.push_back(sg3d);
        }

        //将isolateVector.attvs的size取出
        const int attv_size = atoi(isolatedResult[index_isolated++]);
        for(int j=0; j<attv_size; j++) {
            attv_t attv;
            index_attvs++;
            if(record_id != atoi(attvResult[index_attvs++])) {
                qDebug() << "error while load isolated attvs";
                return;
            }
            attv.attl = atoi(attvResult[index_attvs++]);
            attv.atvl = (const char *)attvResult[index_attvs++];
            isolateVector.attvs.push_back(attv);
        }
        openChart->isolated_node_vectors_map[record_id] = isolateVector;
    }

    for(int i = 0; i < connected_nrow ; i++ ){     //读取连接点数据
        index_connected++;
        const int record_id = atoi(connectedResult[index_connected++]);
        ConnectedNodeVector connected;
        connected.sg2d_index = atoi(connectedResult[index_connected++]);
        const int attv_size = atoi(connectedResult[index_connected++]);

        //根据size取出connected.attvs属性。
        for(int j=0; j<attv_size; j++) {
            attv_t attv;
            index_attvs++;
            if(record_id != atoi(attvResult[index_attvs++])) {
                qDebug() << "error while load connected attvs";
                return;
            }
            attv.attl = atoi(attvResult[index_attvs++]);
            attv.atvl = (const char *)attvResult[index_attvs++];
            connected.attvs.push_back(attv);
        }

        openChart->connected_node_vectors_map[record_id] = connected;
    }

    for(int i = 0; i < edges_nrow ; i++ ){     //读取线数据
        index_edges++;
        const int record_id = atoi(edgesResult[index_edges++]);
        EdgeVector edge;
        edge.beg_node = atoi(edgesResult[index_edges++]);
        edge.end_node = atoi(edgesResult[index_edges++]);
        edge.ornt = atoi(edgesResult[index_edges++]);
        edge.usag = atoi(edgesResult[index_edges++]);
        const int sg2d_size = atoi(edgesResult[index_edges++]);
        for(int j=0; j<sg2d_size; j++) {      //读取其中的坐标索引
            index_index2d++;
            if(record_id != atoi(index2dResult[index_index2d++])) {
                qDebug() << "error while load isolated index2ds";
                return;
            }
            edge.sg2d_indices.push_back(atoi(index2dResult[index_index2d++]));
        }
        const int attv_size = atoi(edgesResult[index_edges++]);
        for(int j=0; j<attv_size; j++) {
            attv_t attv;
            index_attvs++;
            if(record_id != atoi(attvResult[index_attvs++])) {
                qDebug() << "error while load isolated attvs";
                return;
            }
            attv.attl = atoi(attvResult[index_attvs++]);
            attv.atvl = (const char *)attvResult[index_attvs++];
            edge.attvs.push_back(attv);
        }
        openChart->edge_vectors_map[record_id] = edge;
    }
    sqlite3_free_table(isolatedResult);
    sqlite3_free_table(connectedResult);
    sqlite3_free_table(edgesResult);
    sqlite3_free_table(attvResult);
    sqlite3_free_table(sg3dResult);
    sqlite3_free_table(index2dResult);

    //关闭数据库
    sqlite3_close(chart_db);
    chart_db = NULL;
    qDebug()<< "open database to read SENC, time is: "<<tt.elapsed();

    //海图数据的一些处理//////////////////////////////////////////////////////
    openChart->SetupEdgeVectors();      //组合线物标
    openChart->SetupAreaObjects();
    openChart->SetupLineObjects();
    openChart->SetupPointObjects();
    openChart->SetupSoundObjects();

    mercatorProj->ChartMercatorProj(openChart);  //进行墨卡托投影进行坐标转换
  //  qDebug()<<"wocao.....";

    //存储相应的属性到数据库
    if(!isExist(openChart))
    {
        saveDspmMysql(openChart);
        saveFeaturesMysql(openChart);
    }

    mercatorProj->ConvertToScreen(openChart);  //得到屏幕坐标
    openChart->calculateCenter();  //计算所需区域的中心坐标

    ChartData.append(openChart);  //保存显示的海图数据
    nowChart = openChart;    //设置当前的显示海图
    emit readyToRender(openChart->description);  //发送绘图信号
}


//存储DSPM
void DataBase::saveDspmMysql(Chart* pChart)
{   //先判断表存在不存在




    QString featureQuery = "select * from feature";
    sqlQuery->exec(featureQuery);
    int featureStart = sqlQuery->size();


    QString insertDSPM = "insert into dspm values(null,?,?,?,?,?,?,?,?,?,?,?,?,?)";
    sqlQuery->prepare(insertDSPM);
    sqlQuery->bindValue(0,QVariant(QString::fromStdString(pChart->description)));
    sqlQuery->bindValue(1,pChart->dspm.coord_mult_factor);
    sqlQuery->bindValue(2,pChart->dspm.horz_gd_datum);
    sqlQuery->bindValue(3,pChart->dspm.vert_datum);


    sqlQuery->bindValue(4,pChart->dspm.sounding_datum);
    sqlQuery->bindValue(5,pChart->dspm.comp_sod);
    sqlQuery->bindValue(6,pChart->dspm.depth_unit);
    sqlQuery->bindValue(7,pChart->dspm.height_unit);
    sqlQuery->bindValue(8,pChart->dspm.accuracy_unit);

    sqlQuery->bindValue(9,pChart->dspm.coord_unit);
    sqlQuery->bindValue(10,pChart->dspm.sounding_mult_factor);

    qDebug()<<"name:"<<QString::fromStdString(pChart->description);
    qDebug()<<pChart->dspm.coord_mult_factor;
    qDebug()<<pChart->dspm.horz_gd_datum;
    qDebug()<<pChart->dspm.vert_datum;
    qDebug()<<pChart->dspm.sounding_datum;
    qDebug()<<pChart->dspm.comp_sod;
    qDebug()<<pChart->dspm.depth_unit;
    qDebug()<<pChart->dspm.height_unit;
    qDebug()<<pChart->dspm.accuracy_unit;
    qDebug()<<pChart->dspm.coord_unit;
    qDebug()<<pChart->dspm.sounding_mult_factor;


//    QString insertDSPMss = QString("insert into dspm values(null,%1,%2,%3,%4,%5 ,%6,%7,%8,%9,%10 ,%11,%12,%13)")
//            .arg(pChart->dspm.coord_mult_factor,pChart->dspm.horz_gd_datum,pChart->dspm.vert_datum,
//                 pChart->dspm.sounding_datum,pChart->dspm.comp_sod,pChart->dspm.depth_unit,
//                 pChart->dspm.height_unit,pChart->dspm.accuracy_unit,pChart->dspm.coord_unit,pChart->dspm.sounding_mult_factor);

//    qDebug()<<"test:"<<insertDSPMss;
    int interval = pChart->line_objects_map.size() + pChart->point_objects_map.size()
                        + pChart->area_objects_map.size();

    int featureEnd = featureStart + interval -1 > 0 ? featureStart + interval -1 :0;

    //注意索引是否正确
    sqlQuery->bindValue(11,featureStart);
    sqlQuery->bindValue(12,featureEnd);

     qDebug()<<featureStart;
     qDebug()<<featureEnd;
     qDebug();
     qDebug();

    bool insertDspmRet = sqlQuery->exec();
    if(!insertDspmRet)
    {

//       QSqlError DspminsertError = sqlQuery.lastError();
      //  qDebug()<<"insertDspmError"<<sqlQuery.lastError().databaseText();
        QSqlError dspmError = sqlQuery->lastError();
        qDebug()<<"insertFeatureError:"<<dspmError.databaseText();

 //       qDebug()<<"insertDspmError:"<<insertDSPM;
    }


}
//存储Feature
void DataBase::saveFeaturesMysql(Chart* pChart)
{
     //存储点
         int i = 0;
         std::map<int, std::vector<PointObject> >::iterator pfm;
         std::map<int, std::vector<PointObject> >::iterator pend(pChart->point_objects_map.end());

         for(pfm = pChart->point_objects_map.begin();pfm != pend;++pfm)
         {

             std::vector<PointObject> :: iterator pit;
             std::vector<PointObject> :: iterator pEnd(pfm->second.end());
             for(pit = pfm->second.begin();pit != pEnd;++pit){

                 SpaceObject* psb = &(*pit);
                 saveFeaturesMysql(psb,pChart);
             }
         }


         std::map<int, std::vector<LineObject> >::iterator lfm;
         std::map<int, std::vector<LineObject> >::iterator lfm_end(pChart->line_objects_map.end());

         for(lfm = pChart->line_objects_map.begin();lfm != lfm_end;++lfm)
         {
             std::vector<LineObject> :: iterator lit;
             std::vector<LineObject> :: iterator lEnd(lfm->second.end());
             for(lit = lfm->second.begin();lit != lEnd;++lit){
               //  qDebug()<<"line:"<<i++;
                 SpaceObject* lsb = &(*lit);
                 saveFeaturesMysql(lsb,pChart);

             }
         }


         std::map<int, std::vector<AreaObject> >::iterator afm;
         std::map<int, std::vector<AreaObject> >::iterator aend(pChart->area_objects_map.end());

         for(afm = pChart->area_objects_map.begin();afm != aend;++afm)
         {

             std::vector<AreaObject> :: iterator ait;
             std::vector<AreaObject> :: iterator aEnd(afm->second.end());
             for(ait = afm->second.begin();ait != aEnd;++ait){
                 qDebug()<<"area:"<<i++;
                 SpaceObject* asb = &(*ait);
                 saveFeaturesMysql(asb,pChart);

             }
         }

//         std::vector<SoundObject>::iterator sfm;
//         std::vector<SoundObject>::iterator send(pChart->sound_object_vector.end());

//         for(sfm = pChart->sound_object_vector.begin();sfm != send;++sfm)
//         {
//               qDebug()<<"sound:"<<i++;
//               SpaceObject* ssb = &(*sfm);
//               saveFeaturesMysql(ssb,pChart);
//        }

}
int DataBase::getSizeFromTable(QString& sql)
{
    bool sizeQuery = sqlQuery->exec(sql);
    if(!sizeQuery)
    {
      QSqlError lastError = sqlQuery->lastError();
      qDebug()<<"sizeQueryError:"<<lastError.databaseText();
    }
    int size = sqlQuery->size();
    return size;
}

/**
 *@desc
 *@pram 判断该表是否存在
 *
*/
bool DataBase::isExist(Chart* pChart)
{

    QString tableNameQuery = QString("select locationName from dspm where locationName = '%1'").arg(QString::fromStdString(pChart->description));

    sqlQuery->exec(tableNameQuery);
    int size = sqlQuery->size();
    qDebug()<<"tableNameQuery:"<<tableNameQuery<<" size: "<<size;
    if(size > 0)
        return true;
    return false;
}

//存储每一个feature元素
void DataBase::saveFeaturesMysql(SpaceObject* pso,Chart* pChart)
{

    QString queryAttfs = "select * from attfs";
    QString queryNatfs = "select * from natfs";
    QString queryFfpts = "select * from ffpts";
    QString queryFspts = "select * from fspts";   


    int attfsSize = getSizeFromTable(queryAttfs);
    int natfsSize = getSizeFromTable(queryNatfs);
    int ffptsSize = getSizeFromTable(queryFfpts);
    int fsptsSize = getSizeFromTable(queryFspts);

  //  qDebug()<<"name:"<<QString::fromStdString(pso->name);
  //   qDebug()<<"name:"<<QString::fromStdString(pso->name)<<"reid"<<pso->record_id<<"ob_lable"<<pso->object_label<<"gp"<<pso->group<<"prim"<<pso->prim<<"feaid:"<<pso->feature_id<<"subid:"<<pso->feature_subid<<"natfSize:"<<natfsSize<<"priority:"<<pso->priority<<"discate:"<<pso->dispcategory<<"overRadar:"<<pso->overRadar;

    QString insertFeature = "insert into feature (object_label,record_id,grp,prim,\
                             feature_id,feature_subid,attfs_start,attfs_end,natfs_start,\
                             natfs_end,ffpts_start,ffpts_end,fspts_start,fspts_end,\
                             priority,discategory,overRadar)values(?,?,?,?,?,?,?,?,?,?, ?,?,?,?,?,?,?)";

   //qDebug()<<"insertFeature:"<<insertFeature;

    sqlQuery->prepare(insertFeature);
    sqlQuery->bindValue(0,QVariant(QString::fromStdString(pso->name)));
    sqlQuery->bindValue(0,pso->object_label);
    sqlQuery->bindValue(1,pso->record_id);
    sqlQuery->bindValue(2,pso->group);
    sqlQuery->bindValue(3,pso->prim); //feature 的类别
    sqlQuery->bindValue(4,pso->feature_id);
    sqlQuery->bindValue(5,pso->feature_subid);

    int attfsInterval = pso->attfs.size();
    int attfsEnd = (attfsSize + attfsInterval -1)>0?attfsSize + attfsInterval -1:0;
    sqlQuery->bindValue(6,attfsSize);
    sqlQuery->bindValue(7,attfsEnd);
//    qDebug()<<"Astart:"<<attfsSize<<"Aend:"<<attfsEnd<<"Attfssize:"<<attfsInterval;


    int natfsInterval = pso->natfs.size();
    int natfsEnd = (natfsSize + natfsInterval -1) > 0?natfsSize + natfsInterval -1:0;
    sqlQuery->bindValue(8,natfsSize);
    sqlQuery->bindValue(9,natfsEnd);
//    qDebug()<<"Nstart:"<<natfsSize<<"Nend:"<<natfsEnd<<"Natfssize:"<<natfsInterval;


    int ffptsInterval = pso->ffpts.size();
    int ffptsEnd = (ffptsSize +ffptsInterval -1)> 0?ffptsSize +ffptsInterval -1:0;
    sqlQuery->bindValue(10,ffptsSize);
    sqlQuery->bindValue(11,ffptsEnd);
//    qDebug()<<"ffstart:"<<ffptsSize<<"ffend:"<<ffptsEnd<<"ffptssize:"<<ffptsInterval;


    int fsptsInterval = pso->fspts.size();
    int fsptsEnd = (fsptsSize + fsptsInterval -1)>0?fsptsSize + fsptsInterval -1:0;
    sqlQuery->bindValue(12,fsptsSize);
    sqlQuery->bindValue(13,fsptsEnd);
//    qDebug()<<"fsptStart:"<<fsptsSize<<"fsptsEnd:"<<fsptsEnd<<" fsptsSize"<<fsptsInterval;


    sqlQuery->bindValue(14,pso->priority);
    sqlQuery->bindValue(15,pso->dispcategory);
    sqlQuery->bindValue(16,pso->overRadar);

    bool insertFeatureRet = sqlQuery->exec();
    if(!insertFeatureRet)
    {
      QSqlError featureError = sqlQuery->lastError();
      qDebug()<<"insertFeatureError:"<<featureError.databaseText();
    }

    saveFeatureAttfs(pso,pChart);
    saveFeatureNatfs(pso,pChart);
    saveFeatureFfpts(pso,pChart);
    saveFeatureFspts(pso,pChart);
//    saveFeatureAttvs(pso);

   updateSpaceFeature(pso,pChart);


}





/*
    std::vector<attf_t> attfs;   //属性字段
    std::vector<attf_t> natfs;    //国家属性字段
    std::vector<ffpt_t> ffpts;    //特征记录到特征物标指针
    std::vector<fspt_t> fspts;    //特征字段到空间字段指针

*/
void DataBase::saveFeatureAttfs(SpaceObject* pso,Chart* pChart)
{
    std::vector<attf_t>::iterator att;
    std::vector<attf_t>::iterator aend(pso->attfs.end());

    for(att = pso->attfs.begin();att != aend ;att++)
    {
        QString attfInsert = "insert into attfs values(null,?,?,?)";
        sqlQuery->prepare(attfInsert);
        sqlQuery->bindValue(0,pso->record_id);
        sqlQuery->bindValue(1,att->attl);
        sqlQuery->bindValue(2,QVariant(QString::fromStdString(att->atvl)));
        bool attfInsertRet = sqlQuery->exec();      

        if(!attfInsertRet)
        {
            QSqlError lastError = sqlQuery->lastError();
            qDebug()<<"attfInsertError:"<<lastError.databaseText();
        }
    }


}
void DataBase::saveFeatureNatfs(SpaceObject* pso,Chart* pChart)
{
    std::vector<attf_t>::iterator attit;
    std::vector<attf_t>::iterator aend(pso->natfs.end());
    int nasize = pso->natfs.size();
    qDebug()<<"nasize:"<<nasize;
    qDebug();

    for(attit = pso->natfs.begin();attit != aend ;attit++)
    {
        QString attfInsert = "insert into natfs values(null,?,?,?,?)";
        sqlQuery->prepare(attfInsert);
        sqlQuery->bindValue(0,pso->record_id);
        sqlQuery->bindValue(1,attit->attl);
        sqlQuery->bindValue(2,QVariant(QString::fromStdString(attit->atvl)));
        bool attfRet = sqlQuery->exec();
        if(!attfRet)
        {
         QSqlError sqlError = sqlQuery->lastError();
         qDebug()<<"attfInsertError:"<<sqlError.databaseText();
        }
    }
}
void DataBase::saveFeatureFfpts(SpaceObject* pso,Chart* pChart)
{
    std::vector<ffpt_t>::iterator ffit;
    std::vector<ffpt_t>::iterator ffend(pso->ffpts.end());

    int ffsize = pso->ffpts.size();

    for(ffit = pso->ffpts.begin();ffit != ffend;ffit++)
    {
        QString ffptInsert = "insert into ffpts values(null,?,?,?,?,?,?)";
        sqlQuery->prepare(ffptInsert);
        sqlQuery->bindValue(0,pso->record_id);
        sqlQuery->bindValue(1,ffit->agen);
        sqlQuery->bindValue(2,ffit->find);
        sqlQuery->bindValue(3,ffit->fids);
        sqlQuery->bindValue(4,ffit->rind);
        sqlQuery->bindValue(5,QVariant(QString::fromStdString(ffit->comt)));
        bool ffptInsertRet = sqlQuery->exec();
        if(!ffptInsertRet)
        {
            QSqlError error = sqlQuery->lastError();
            qDebug()<<"ffptInsert:"<<error.databaseText();
        }
    }
}
void DataBase::saveFeatureFspts(SpaceObject* pso,Chart* pChart)
{
   std::vector<fspt_t> ::iterator fsit;
   std::vector<fspt_t> ::iterator fsend(pso->fspts.end());
   for(fsit = pso->fspts.begin();fsit != fsend;fsit++)
   {
       QString fsptInsert = "insert into fspts values(null,?,?,?, ?,?,?)";
       sqlQuery->prepare(fsptInsert);
       sqlQuery->bindValue(0,pso->record_id);
       sqlQuery->bindValue(1,fsit->rcnm);
       sqlQuery->bindValue(2,fsit->rcid);
       sqlQuery->bindValue(3,fsit->ornt);
       sqlQuery->bindValue(4,fsit->usag);
       sqlQuery->bindValue(5,fsit->mask);
       bool fsptRet = sqlQuery->exec();
       if(!fsptRet)
       {
          QSqlError error = sqlQuery->lastError();
          qDebug()<<"fsptInsertError"<<error.databaseText();
       }
   }

}

void DataBase::saveFeatureAttvs(SpaceObject *pso,const std::vector<attv_t> &attvs)
{
    std::vector<attv_t> ::const_iterator ait;
    std::vector<attv_t> ::const_iterator aEnd(attvs.end());

    for(ait = attvs.begin();ait != aEnd;ait++)
    {
        QString attvitInsert = QString("insert into attvs values(null,?,?,?)");
        sqlQuery->bindValue(0,pso->record_id);
        sqlQuery->bindValue(1,ait->attl);
        sqlQuery->bindValue(2,QVariant(QString::fromStdString(ait->atvl)));

        qDebug()<<"attvsInsert:"<<attvitInsert;
        qDebug();

        bool attvitInsertRet = sqlQuery->exec(attvitInsert);
        if(!attvitInsertRet)
        {
            QSqlError error = sqlQuery->lastError();
            qDebug()<<"attvitInsertError,when record_id = "<<pso->record_id<<error.databaseText();
        }
    }
}


//添加ENC原始海图，解析并存储为数据库文件
void DataBase::addChart()
{
    QStringList files = QFileDialog::getOpenFileNames(lpMainWindow,
                                                      "Select one or more enc files to open",
                                                      QDir::currentPath(),
                                                      "ENC (*.000)");
    QStringList list = files;
    QStringList::Iterator it = list.begin();
    while(it != list.end()) {
        LoadChartHelper(*it);   //循环解析海图数据
        ++it;
    }
}

//获取海图文件的名字，去掉路径
QString getEncName(const QString &fileName)
{
    const QStringList filename = fileName.split("/");
    const QStringList name = filename.last().split(".");
    return name.first();
}

//解析原始海图数据
bool DataBase::LoadChartHelper(const QString &fileName)
{
    // first we check if this chart has already been loaded
    //从数据库中查询
    QMutexLocker locker(&mutex);

    int result;
    sqlite3_stmt* stmt = NULL;

    // 创建表格,海图共用一个表格，因为内容不多
    const QString tableName = QString("ChartTable");
    QString crttbl = QString("CREATE TABLE IF NOT EXISTS '%1' (chartnum TEXT, chartname TEXT, editionnum TEXT, updatenum TEXT,\
                             updatetime INTEGER, scale INTEGER, leftlat REAL, leftlon REAL, rightlat REAL, rightlon REAL)").arg(tableName);
    result = sqlite3_exec(data_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);
    //检测是否有该海图数据存在
    const QString select = QString("SELECT 1 FROM %1 WHERE chartnum == '%2' LIMIT 1").arg(tableName).arg(getEncName(fileName));
    result = sqlite3_prepare_v2(data_db, select.toLocal8Bit().constData(), select.size(), &stmt, NULL);
    if( result != SQLITE_OK ){
        return false;
    }

    result = sqlite3_step(stmt);
    if( result == SQLITE_ROW ){
        QMessageBox box;
        box.setWindowTitle(QObject::tr("警告"));
        box.setIcon(QMessageBox::Warning);
        box.setText(QObject::tr("海图文件已加载！"));
        box.setStandardButtons(QMessageBox::Yes);
        if(box.exec() == QMessageBox::Yes)
            return NULL;
    }
    locker.unlock();   //必须解锁，后面需要获得锁 */


    S57Chart* chart = new S57Chart;
    try {
        chart->Load(fileName.toStdString());
        chart->description = getEncName(fileName).toStdString();
        //获得海图的边界经纬度,得到的是度为单位
        chart->DiscoverDimensions();

     } catch (std::string error) {
        std::string errstr = "Loading chart failed: " + error;
        qDebug() << QString().fromStdString(errstr);
        delete chart;
        return false;
     }

    /*********************************************** test begin *****************************************************/
    //海图数据的一些处理//////////////////////////////////////////////////////
//    chart->SetupEdgeVectors();      //组合线物标
//    chart->SetupAreaObjects();
//    chart->SetupLineObjects();
//    chart->SetupPointObjects();
//    chart->SetupSoundObjects();

//    mercatorProj->ChartMercatorProj(chart);  //进行墨卡托投影进行坐标转换
//    qDebug()<<"wocao.....";

//    //存储相应的属性到数据库
//    saveDspmMysql(chart);
//    saveFeaturesMysql(chart);

    /*********************************************** test end *****************************************************/



  
    //将解析后的数据保存到SENC数据库中，如果保存成功则将该海图信息保存
//    if(SaveChartDataBase(chart)) {
//        //将海图信息加入数据库
//        const quint32 time0 = time(0);
//        addSencRecord(chart, time0);
//        showChartSelectWindow();

//        delete chart;
//        return true;
//    }else {
//        qDebug() << "save chart data error! filePaths is: " << fileName;
//        delete chart;
//        return false;
//    }
}

//保存一幅海图到数据库
//通过S57Chart类的解析,将每个物标的相关信息保存在object中，数据库只保存这些值
bool DataBase::SaveChartDataBase(S57Chart *chartData)
{
    QTime tt;
    tt.start();


    const QString sencFilePath = SOFTWAREPATH + QString("data/SENC/");
    const QString chartFilePath = sencFilePath + QString::fromStdString(chartData->description) + QString(".wmc");

    //创建数据库
    int result;
    result = sqlite3_open_v2(chartFilePath.toLocal8Bit().constData(), &chart_db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, NULL);    //可以设置标志位
    if(result != SQLITE_OK) {
        fprintf(stderr, "cann't create database: %s \n", sqlite3_errmsg(chart_db));
        sqlite3_close(chart_db);
        data_db = NULL;
        return false;
    }
    // 创建表格,每个表格代表一项内容,插入海图所有数据项
    {
        QString crttbl = QString("CREATE TABLE IF NOT EXISTS '%1' (description TEXT)").arg(QString("DESCRIPTION"));
        result = sqlite3_exec(chart_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);
        // 插入记录
        QString insert = QString("INSERT INTO '%1' VALUES ('%2')").arg(QString("Description")).arg(QString::fromStdString(chartData->description));
        result = sqlite3_exec(chart_db, insert.toLocal8Bit().constData(), 0, 0, 0);
    }
    //DSID,有字符串，不能作为BLOB存储
    {
        QString crttbl = QString("CREATE TABLE IF NOT EXISTS '%1' (dataset_name TEXT, edition_num TEXT, issue_date TEXT, update_num TEXT, s57_edition INTEGER, \
                                                                   producing_agency INTEGER, data_struct INTEGER, attf_lexl INTEGER, natf_lexl INTEGER, \
                                                                   num_meta_rec INTEGER, num_cart_rec INTEGER, num_geo_rec INTEGER, num_coll_rec INTEGER, \
                                                                   num_isonode_rec INTEGER, num_connnode_rec INTEGER, num_edge_rec INTEGER, num_face_rec INTEGER)").arg(QString("DSID"));
        result = sqlite3_exec(chart_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);
        // 插入记录
        sqlite3_stmt* stmt = NULL;
        QString insert = QString("INSERT INTO DSID VALUES ('%1','%2','%3','%4','%5','%6','%7','%8','%9','%10','%11','%12','%13','%14','%15','%16','%17')").arg(QString::fromStdString(chartData->dsid.dataset_name))
                         .arg(QString::fromStdString(chartData->dsid.edition_num)).arg(QString::fromStdString(chartData->dsid.issue_date)).arg(QString::fromStdString(chartData->dsid.update_num))
                         .arg((chartData->dsid.s57_edition)).arg((chartData->dsid.producing_agency)).arg((chartData->dsid.data_struct)).arg((chartData->dsid.attf_lexl))
                         .arg((chartData->dsid.natf_lexl)).arg((chartData->dsid.num_meta_rec)).arg((chartData->dsid.num_cart_rec)).arg((chartData->dsid.num_geo_rec))
                         .arg((chartData->dsid.num_coll_rec)).arg((chartData->dsid.num_isonode_rec)).arg((chartData->dsid.num_connnode_rec)).arg((chartData->dsid.num_edge_rec))
                         .arg((chartData->dsid.num_face_rec));
        result = sqlite3_prepare_v2(chart_db, insert.toLocal8Bit().constData(), -1, &stmt, NULL);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_prepare insert into dsid\n");
            return false;
        }
        result = sqlite3_step(stmt);
        if( result != SQLITE_DONE){
            fprintf(stderr, "SQL error: sqlite3_step dsid bolb \n");
            return false;
        }
        sqlite3_finalize(stmt);
    }
    //DSPM
    {
        QString crttbl = QString("CREATE TABLE IF NOT EXISTS '%1' (dspm BLOB)").arg(QString("DSPM"));
        result = sqlite3_exec(chart_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);
        // 插入记录
        sqlite3_stmt* stmt = NULL;
        result = sqlite3_prepare_v2(chart_db, "INSERT INTO DSPM VALUES (?)", -1, &stmt, NULL);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_prepare insert into dspm\n");
            return false;
        }
        int size = sizeof(chartData->dspm);
        sqlite3_bind_blob(stmt, 1, (const void*)&(chartData->dspm), size, NULL);
        result = sqlite3_step(stmt);
        if( result != SQLITE_DONE ){
            fprintf(stderr, "SQL error: sqlite3_step dsim bolb \n");
            return false;
        }
        sqlite3_finalize(stmt);
    }
    //矩形范围
    {
        QString crttbl = QString("CREATE TABLE IF NOT EXISTS '%1' (slat REAL, wlon REAL, nlat REAL, elon REAL)").arg(QString("RANGE"));
        result = sqlite3_exec(chart_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);
        // 插入记录
        QString insert = QString("INSERT INTO '%1' VALUES ('%2', '%3', '%4', '%5')").arg(QString("RANGE"))
                              .arg(chartData->slat.minutes).arg(chartData->wlon.minutes).arg(chartData->nlat.minutes).arg(chartData->elon.minutes);
        result = sqlite3_exec(chart_db, insert.toLocal8Bit().constData(), 0, 0, 0);
    }
    //存储经纬度点坐标
    {
        QString crttbl = QString("CREATE TABLE IF NOT EXISTS '%1' (sg2d BLOB)").arg(QString("SG2D"));
        result = sqlite3_exec(chart_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);
        // 插入记录
        sqlite3_stmt* stmt = NULL;
        result = sqlite3_prepare_v2(chart_db, "INSERT INTO SG2D VALUES (?)", -1, &stmt, NULL);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_prepare insert into sg2d\n");
            return false;
        }
        /////////////将vector容器数据保存到数据库中
        const int size = chartData->sg2ds.size() * sizeof(sg2d_t);
      //  void* buffer = NULL;   //创建内存指针
       // buffer = new char(size);   //初始化存储空间
      //  memcpy(buffer, (char*)&chartData->sg2ds[0], size);   //sg2ds时模板类型，要用sg2ds[0]
        result = sqlite3_bind_blob(stmt, 1, (const char*)&(chartData->sg2ds[0]), size, NULL);
        result = sqlite3_step(stmt);
        if( result != SQLITE_DONE ){
            fprintf(stderr, "SQL error: sqlite3_step sg2ds bolb \n");
            return false;
        }
        sqlite3_finalize(stmt);
       // delete buffer;
    }
    //保存属性数据信息,所有表的内容以record_id为主键
    //去掉record_id这个记录，因为存储是按顺序来的，所以直接保存属性个数，再依次读取就行了，减少存储量
    {
        //创建属性表,注意字段的命名，比如group就不行，浪费了好长时间找问题
        QString crttbl = QString("CREATE TABLE IF NOT EXISTS '%1' (ids INTEGER UNIQUE PRIMARY KEY, record_id INTEGER, object_label INTEGER, groups INTEGER, prim INTEGER, feature_id INTEGER, \
                                                                   feature_subid INTEGER, name TEXT, attf_num INTEGER, natf_num INTEGER, ffpt_num INTEGER, fspt_num INTEGER)").arg(QString("FEATURES"));
        result = sqlite3_exec(chart_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);
        fprintf(stderr, "%s", sqlite3_errmsg(chart_db));
        //创建attfs属性字段

        crttbl = QString("CREATE TABLE IF NOT EXISTS '%1' (ids INTEGER UNIQUE PRIMARY KEY, record_id INTEGER, attl INTEGER, atvl TEXT)").arg(QString("ATTFS"));
        result = sqlite3_exec(chart_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);
       // crttbl = QString("CREATE INDEX IF NOT EXISTS index_id on ATTFS(record_id)");
       // result = sqlite3_exec(chart_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);
        //创建natfs国家属性字段表
        crttbl = QString("CREATE TABLE IF NOT EXISTS '%1' (ids INTEGER UNIQUE PRIMARY KEY, record_id INTEGER, attl INTEGER, atvl TEXT)").arg(QString("NATFS"));
        result = sqlite3_exec(chart_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);
       // crttbl = QString("CREATE INDEX IF NOT EXISTS index_id on NATFS(record_id)");
       // result = sqlite3_exec(chart_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);
        //创建ffpts特征记录到特征记录指针表
        crttbl = QString("CREATE TABLE IF NOT EXISTS '%1' (ids INTEGER UNIQUE PRIMARY KEY, record_id INTEGER, agen INTEGER, find INTEGER, fids INTEGER, rind INTEGER, comt TEXT)").arg(QString("FFPTS"));
        result = sqlite3_exec(chart_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);
       // crttbl = QString("CREATE INDEX IF NOT EXISTS index_id on FFPTS(record_id)");
       // result = sqlite3_exec(chart_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);
        //创建fspts特征指针到空间段表
        crttbl = QString("CREATE TABLE IF NOT EXISTS '%1' (ids INTEGER UNIQUE PRIMARY KEY, record_id INTEGER, rcnm INTEGER, rcid INTEGER, ornt INTEGER, usag INTEGER, mask INTEGER)").arg(QString("FSPTS"));
        result = sqlite3_exec(chart_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);
       // crttbl = QString("CREATE INDEX IF NOT EXISTS index_id on FSPTS(record_id)");
       // result = sqlite3_exec(chart_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);

        //插入属性记录
        sqlite3_stmt* stmt = NULL;
        result = sqlite3_prepare_v2(chart_db, "INSERT INTO FEATURES VALUES (?,?,?,?,?,?,?,?,?,?,?,?)", -1, &stmt, NULL);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_prepare insert into sg2d\n");
            fprintf(stderr, "%s", sqlite3_errmsg(chart_db));
            return false;
        }
        sqlite3_stmt* stmt1 = NULL;
        result = sqlite3_prepare_v2(chart_db, "INSERT INTO ATTFS VALUES (?,?,?,?)", -1, &stmt1, NULL);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_prepare insert into attfs\n");
            fprintf(stderr, "%s", sqlite3_errmsg(chart_db));
            return false;
        }
        sqlite3_stmt* stmt2 = NULL;
        result = sqlite3_prepare_v2(chart_db, "INSERT INTO NATFS VALUES (?,?,?,?)", -1, &stmt2, NULL);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_prepare insert into natfs\n");
            return false;
        }
        sqlite3_stmt* stmt3 = NULL;
        result = sqlite3_prepare_v2(chart_db, "INSERT INTO FFPTS VALUES (?,?,?,?,?,?,?)", -1, &stmt3, NULL);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_prepare insert into ffpts\n");
            return false;
        }
        sqlite3_stmt* stmt4 = NULL;
        result = sqlite3_prepare_v2(chart_db, "INSERT INTO FSPTS VALUES (?,?,?,?,?,?,?)", -1, &stmt4, NULL);
        if( result != SQLITE_OK ) {
            fprintf(stderr, "SQL error: sqlite3_prepare insert into fspts\n");
            return false;
        }

     //   void saveFeatures()

#define saveFeatures(pit) {   \
        sqlite3_bind_int(stmt, 1, num_feature++);  \
        sqlite3_bind_int(stmt, 2, pit->record_id);  \
        sqlite3_bind_int(stmt, 3, pit->object_label);\
        sqlite3_bind_int(stmt, 4, pit->group);\
        sqlite3_bind_int(stmt, 5, pit->prim);\
        sqlite3_bind_int(stmt, 6, pit->feature_id);\
        sqlite3_bind_int(stmt, 7, pit->feature_subid);\
        sqlite3_bind_text(stmt, 8, pit->name.c_str(), -1, NULL);\
        sqlite3_bind_int(stmt, 9, pit->attfs.size()); \
        sqlite3_bind_int(stmt, 10, pit->natfs.size());\
        sqlite3_bind_int(stmt, 11, pit->ffpts.size());\
        sqlite3_bind_int(stmt, 12, pit->fspts.size());\
        result = sqlite3_step(stmt);\
        if( result != SQLITE_DONE ){\
            fprintf(stderr, "SQL error: sqlite3_step feature \n");\
            sqlite3_exec (chart_db , "rollback transaction" , 0, 0, 0);\
            return false;\
        }\
        sqlite3_reset(stmt);\
        const int s1 = pit->attfs.size();\
        for(int i=0; i<s1; i++) {\
            sqlite3_bind_int(stmt1, 1, num_attfs++);\
            sqlite3_bind_int(stmt1, 2, pit->record_id);\
            sqlite3_bind_int(stmt1, 3, pit->attfs[i].attl);\
            sqlite3_bind_text(stmt1, 4, pit->attfs[i].atvl.c_str(), -1, NULL);\
            result = sqlite3_step(stmt1);\
            if( result != SQLITE_DONE ){\
                fprintf(stderr, "SQL error: sqlite3_step attfs \n");\
                fprintf(stderr, "%s", sqlite3_errmsg(chart_db));\
                sqlite3_exec (chart_db , "rollback transaction" , 0, 0, 0);\
                return false;\
            }\
            sqlite3_reset(stmt1);\
        }\
        const int s2 = pit->natfs.size();\
        for(int i=0; i<s2; i++) {\
            sqlite3_bind_int(stmt2, 1, num_natfs++);\
            sqlite3_bind_int(stmt2, 2, pit->record_id);\
            sqlite3_bind_int(stmt2, 3, pit->natfs[i].attl);\
            sqlite3_bind_text(stmt2, 4, pit->natfs[i].atvl.c_str(), -1, NULL);\
            result = sqlite3_step(stmt2);\
            if( result != SQLITE_DONE ){\
                fprintf(stderr, "SQL error: sqlite3_step natfs \n");\
                sqlite3_exec (chart_db , "rollback transaction" , 0, 0, 0);\
                return false;\
            }\
            sqlite3_reset(stmt2);\
        }\
        const int s3 = pit->ffpts.size();\
        for(int i=0; i<s3; i++) {\
            sqlite3_bind_int(stmt3, 1, num_ffpts++);\
            sqlite3_bind_int(stmt3, 2, pit->record_id);\
            sqlite3_bind_int(stmt3, 3, pit->ffpts[i].agen);\
            sqlite3_bind_int(stmt3, 4, pit->ffpts[i].find);\
            sqlite3_bind_int(stmt3, 5, pit->ffpts[i].fids);\
            sqlite3_bind_int(stmt3, 6, pit->ffpts[i].rind);\
            sqlite3_bind_text(stmt3, 7, pit->ffpts[i].comt.c_str(), -1, NULL);\
            result = sqlite3_step(stmt3);\
            if( result != SQLITE_DONE ){\
                fprintf(stderr, "SQL error: sqlite3_step ffpts \n");\
                sqlite3_exec (chart_db , "rollback transaction" , 0, 0, 0);\
                return false;\
            }\
            sqlite3_reset(stmt3);\
        }\
        const int s4 = pit->fspts.size();\
        for(int i=0; i<s4; i++) {\
            sqlite3_bind_int(stmt4, 1, num_fspts++);\
            sqlite3_bind_int(stmt4, 2, pit->record_id);\
            sqlite3_bind_int(stmt4, 3, pit->fspts[i].rcnm);\
            sqlite3_bind_int(stmt4, 4, pit->fspts[i].rcid);\
            sqlite3_bind_int(stmt4, 5, pit->fspts[i].ornt);\
            sqlite3_bind_int(stmt4, 6, pit->fspts[i].usag);\
            sqlite3_bind_int(stmt4, 7, pit->fspts[i].mask);\
            result = sqlite3_step(stmt4);\
            if( result != SQLITE_DONE ){\
                fprintf(stderr, "SQL error: sqlite3_step fspts \n");\
                sqlite3_exec (chart_db , "rollback transaction" , 0, 0, 0);\
                return false;\
            }\
            sqlite3_reset(stmt4);\
        }\
    }

        //绑定数据
        int num_attfs=0, num_natfs=0, num_ffpts=0, num_fspts=0, num_feature=0;
        sqlite3_exec(chart_db , "begin transaction" ,0 , 0, 0);

        std::map<int, std::vector<Feature> >::iterator pfm;
        std::map<int, std::vector<Feature> >::iterator end(chartData->point_features_map.end());
        for (pfm = chartData->point_features_map.begin(); pfm != end; ++pfm) {
            std::vector<Feature>::iterator pit;
            std::vector<Feature>::iterator end = pfm->second.end();
            for (pit = pfm->second.begin(); pit != end; ++pit) {   //保存每一个属性值，后面建立链表用object_label字段
               saveFeatures(pit);  //其中包括水深点，标号129
            }  //end for
        } //end for

        std::map<int, std::vector<Feature> >::iterator lfm;
        std::map<int, std::vector<Feature> >::iterator lfm_end(chartData->line_features_map.end());
        for (lfm = chartData->line_features_map.begin(); lfm != lfm_end; ++lfm) {
            std::vector<Feature>::iterator lit;
            std::vector<Feature>::iterator  lend = lfm->second.end();
            for (lit = lfm->second.begin(); lit != lend; ++lit) {
                saveFeatures(lit);
            }
        }

        std::map<int, std::vector<Feature> >::iterator afm;
        std::map<int, std::vector<Feature> >::iterator afm_end = chartData->area_features_map.end();
        for (afm = chartData->area_features_map.begin(); afm != afm_end; ++afm) {
            std::vector<Feature>::iterator ait;
            std::vector<Feature>::iterator aend = afm->second.end();
            for (ait = afm->second.begin(); ait != aend; ++ait) {
                saveFeatures(ait);
            }
        }
        sqlite3_exec (chart_db, "commit transaction" ,0 , 0, 0);
        sqlite3_finalize(stmt);
        sqlite3_finalize(stmt1);
        sqlite3_finalize(stmt2);
        sqlite3_finalize(stmt3);
        sqlite3_finalize(stmt4);
    }
    //保存空间记录信息,主键被自动排序了
    {
        //创建独立点表
        QString crttbl = QString("CREATE TABLE IF NOT EXISTS '%1' (ids INTEGER PRIMARY KEY, record_id INTEGER, indexs INTEGER, size INTEGER, attv_num INTEGER)").arg(QString("ISOLATED"));
        result = sqlite3_exec(chart_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);

        //创建连接点表
        crttbl = QString("CREATE TABLE IF NOT EXISTS '%1' (ids INTEGER PRIMARY KEY, record_id INTEGER, indexs INTEGER, attv_num INTEGER)").arg(QString("CONNECTED"));
        result = sqlite3_exec(chart_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);
        //创建线段表
        crttbl = QString("CREATE TABLE IF NOT EXISTS '%1' (ids INTEGER PRIMARY KEY, record_id INTEGER, beg INTEGER, end INTEGER, ornt INTEGER, usag INTEGER, sg2d_size INTEGER, attv_num INTEGER)").arg(QString("EDGES"));
        result = sqlite3_exec(chart_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);
        //创建attvs属性字段表
        crttbl = QString("CREATE TABLE IF NOT EXISTS '%1' (ids INTEGER PRIMARY KEY, record_id INTEGER, attl INTEGER, atvl TEXT)").arg(QString("ATTVS"));
        result = sqlite3_exec(chart_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);
        //创建孤立点坐标
        crttbl = QString("CREATE TABLE IF NOT EXISTS '%1' (ids INTEGER PRIMARY KEY, record_id INTEGER, lon REAL, lat REAL, depth REAL)").arg(QString("SG3DS"));
        result = sqlite3_exec(chart_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);
        //创建线段坐标索引
        crttbl = QString("CREATE TABLE IF NOT EXISTS '%1' (ids INTEGER PRIMARY KEY, record_id INTEGER, sg2dindex INTEGER)").arg(QString("SG2DINDEX"));
        result = sqlite3_exec(chart_db, crttbl.toLocal8Bit().constData(), 0, 0, 0);
        fprintf(stderr, "%s", sqlite3_errmsg(chart_db));
        //插入空间记录
        sqlite3_stmt* stmt = NULL;
        result = sqlite3_prepare_v2(chart_db, "INSERT INTO ISOLATED VALUES (?,?,?,?,?)", -1, &stmt, NULL);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_prepare insert into sg2d\n");
            fprintf(stderr, "%s", sqlite3_errmsg(chart_db));
            return false;
        }
        sqlite3_stmt* stmt1 = NULL;
        result = sqlite3_prepare_v2(chart_db, "INSERT INTO CONNECTED VALUES (?,?,?,?)", -1, &stmt1, NULL);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_prepare insert into attfs\n");
            fprintf(stderr, "%s", sqlite3_errmsg(chart_db));
            return false;
        }
        sqlite3_stmt* stmt2 = NULL;
        result = sqlite3_prepare_v2(chart_db, "INSERT INTO EDGES VALUES (?,?,?,?,?,?,?,?)", -1, &stmt2, NULL);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_prepare insert into natfs\n");
            return false;
        }
        sqlite3_stmt* stmt3 = NULL;
        result = sqlite3_prepare_v2(chart_db, "INSERT INTO ATTVS VALUES (?,?,?,?)", -1, &stmt3, NULL);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_prepare insert into ffpts\n");
            return false;
        }
        sqlite3_stmt* stmt4 = NULL;
        result = sqlite3_prepare_v2(chart_db, "INSERT INTO SG3DS VALUES (?,?,?,?,?)", -1, &stmt4, NULL);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_prepare insert into sg3ds\n");
            return false;
        }
        sqlite3_stmt* stmt5 = NULL;
        result = sqlite3_prepare_v2(chart_db, "INSERT INTO SG2DINDEX VALUES (?,?,?)", -1, &stmt5, NULL);
        if( result != SQLITE_OK ){
            fprintf(stderr, "SQL error: sqlite3_prepare insert into index2ds\n");
            fprintf(stderr, "%s", sqlite3_errmsg(chart_db));
            return false;
        }

#define saveAttvs(pfm) {  \
        const int s1 = pfm->second.attvs.size(); \
        for(int i=0; i<s1; i++) {\
            sqlite3_bind_int(stmt3, 1, num_attvs++);\
            sqlite3_bind_int(stmt3, 2, pfm->first);\
            sqlite3_bind_int(stmt3, 3, pfm->second.attvs[i].attl);\
            sqlite3_bind_text(stmt3, 4, pfm->second.attvs[i].atvl.c_str(), -1, NULL);\
            result = sqlite3_step(stmt3);\
            if( result != SQLITE_DONE ){\
                fprintf(stderr, "SQL error: sqlite3_step attfs \n");\
                fprintf(stderr, "%s", sqlite3_errmsg(chart_db));\
                sqlite3_exec (chart_db , "rollback transaction" , 0, 0, 0);\
                return false;\
            }\
            sqlite3_reset(stmt3);\
        }     \
    }

        int num_attvs = 0, num_isolated=0, num_connected=0, num_edges=0, num_sg3ds=0, num_index2d=0;
        sqlite3_exec(chart_db , "begin transaction" ,0 , 0, 0);
        std::map<int, IsolatedNodeVector>::iterator pfm;
        std::map<int, IsolatedNodeVector>::iterator end(chartData->isolated_node_vectors_map.end());
        for (pfm = chartData->isolated_node_vectors_map.begin(); pfm != end; ++pfm) {
            sqlite3_bind_int(stmt, 1, num_isolated++);
            sqlite3_bind_int(stmt, 2, pfm->first);
            sqlite3_bind_int(stmt, 3, pfm->second.sg2d_index);
            int size = pfm->second.sg3ds.size();
            sqlite3_bind_int(stmt, 4, size);
            if(size > 0) {  //保存三维点到表中
                for(int j=0; j<size; j++) {
                    sqlite3_bind_int(stmt4, 1, num_sg3ds++);
                    sqlite3_bind_int(stmt4, 2, pfm->first);
                    sqlite3_bind_double(stmt4, 3, pfm->second.sg3ds[j].long_lat[0]);
                    sqlite3_bind_double(stmt4, 4, pfm->second.sg3ds[j].long_lat[1]);
                    sqlite3_bind_double(stmt4, 5, pfm->second.sg3ds[j].depth);
                    result = sqlite3_step(stmt4);
                    if( result != SQLITE_DONE ){
                        fprintf(stderr, "SQL error: sqlite3_step sg3ds \n");
                        fprintf(stderr, "%s", sqlite3_errmsg(chart_db));
                        sqlite3_exec (chart_db , "rollback transaction" , 0, 0, 0);
                        return false;
                    }
                    sqlite3_reset(stmt4);
                }
            }
            sqlite3_bind_int(stmt, 5, pfm->second.attvs.size());
            result = sqlite3_step(stmt);
            if( result != SQLITE_DONE ){
                fprintf(stderr, "SQL error: sqlite3_step isolated \n");
                sqlite3_exec (chart_db , "rollback transaction" , 0, 0, 0);
                return false;
            }
            sqlite3_reset(stmt);
            saveAttvs(pfm);
        } //end for
        std::map<int, ConnectedNodeVector>::iterator lfm;
        std::map<int, ConnectedNodeVector>::iterator lfm_end(chartData->connected_node_vectors_map.end());
        for (lfm = chartData->connected_node_vectors_map.begin(); lfm != lfm_end; ++lfm) {
            sqlite3_bind_int(stmt1, 1, num_connected++);
            sqlite3_bind_int(stmt1, 2, lfm->first);
            sqlite3_bind_int(stmt1, 3, lfm->second.sg2d_index);
            sqlite3_bind_int(stmt1, 4, lfm->second.attvs.size());
            result = sqlite3_step(stmt1);
            if( result != SQLITE_DONE ){
                fprintf(stderr, "SQL error: sqlite3_step connected \n");
                sqlite3_exec (chart_db , "rollback transaction" , 0, 0, 0);
                return false;
            }
            sqlite3_reset(stmt1);
            saveAttvs(lfm);
        } //end for
        std::map<int, EdgeVector>::iterator afm;
        std::map<int, EdgeVector>::iterator afm_end(chartData->edge_vectors_map.end());
        for (afm = chartData->edge_vectors_map.begin(); afm != afm_end; ++afm) {
            sqlite3_bind_int(stmt2, 1, num_edges++);
            sqlite3_bind_int(stmt2, 2, afm->first);
            sqlite3_bind_int(stmt2, 3, afm->second.beg_node);
            sqlite3_bind_int(stmt2, 4, afm->second.end_node);
            sqlite3_bind_int(stmt2, 5, afm->second.ornt);
            sqlite3_bind_int(stmt2, 6, afm->second.usag);
            sqlite3_bind_int(stmt2, 7, afm->second.sg2d_indices.size());
            sqlite3_bind_int(stmt2, 8, afm->second.attvs.size());
            result = sqlite3_step(stmt2);
            if( result != SQLITE_DONE ){
                fprintf(stderr, "SQL error: sqlite3_step edges \n");
                sqlite3_exec (chart_db , "rollback transaction" , 0, 0, 0);
                return false;
            }
            sqlite3_reset(stmt2);
            //保存中间坐标索引
            const int size_2ds = afm->second.sg2d_indices.size();
            for(int j=0; j<size_2ds; j++) {
                sqlite3_bind_int(stmt5, 1, num_index2d++);
                sqlite3_bind_int(stmt5, 2, afm->first);
                sqlite3_bind_int(stmt5, 3, afm->second.sg2d_indices[j]);
                result = sqlite3_step(stmt5);
                if( result != SQLITE_DONE ){
                    fprintf(stderr, "SQL error: sqlite3_step index2ds \n");
                    fprintf(stderr, "%s", sqlite3_errmsg(chart_db));
                    sqlite3_exec (chart_db , "rollback transaction" , 0, 0, 0);
                    return false;
                }
                sqlite3_reset(stmt5);
            }
            saveAttvs(afm);
        } //end for

        sqlite3_exec (chart_db, "commit transaction" ,0 , 0, 0);
        sqlite3_finalize(stmt);
        sqlite3_finalize(stmt1);
        sqlite3_finalize(stmt2);
        sqlite3_finalize(stmt3);
        sqlite3_finalize(stmt4);
        sqlite3_finalize(stmt5);
    }

    //关闭数据库
    sqlite3_close(chart_db);
    chart_db = NULL;

    qDebug() << "time of save chartdata is: " << tt.elapsed();

    return true;

}

void DataBase::updateSpaceFeature(SpaceObject *pso, Chart *pChart)
{
    //处理类型转换
//    PointObject * ppo = NULL;
//    LineObject * plb = NULL;
 //   AreaObject* pab = NULL;
//    SoundObject* psb = NULL;
    int spacePrim = pso->prim;
    switch(pso->prim)
    {
    case 1:
    {
        if(pso->object_label == 129)
        {
            if(SoundObject* psb = dynamic_cast<SoundObject*>(pso))
            {
                QString queryAttvs = "select * from attvs";
                int attvsStart = getSizeFromTable(queryAttvs);
                int interval = psb->attvs.size();
                int attvsEnd = attvsStart + interval -1;
                //构建三维:深度之关联二维坐标。深度值格式 数字，数字.对应
                QString s2d = QString("lineStringfromtext('lineString(");
                QString depths = QString("");
                std::vector<sg3d_t>::iterator sgit;
                std::vector<sg3d_t>::iterator sgEnd = psb->sg3ds.end();
                for(sgit = psb->sg3ds.begin();sgit != sgEnd;sgit++)
                {

                    QString xstr = QString::number(sgit->long_lat[0],10,4);
                    QString ystr = QString::number(sgit->long_lat[1],10,4);
                    s2d.append(QString("%1 %2").arg(xstr,ystr));
                    depths.append(QString::number(sgit->depth));

                    if(sgit != sgEnd - 1)
                    {
                        s2d.append(",");
                        depths.append(",");
                    }

                }

                s2d.append(")')");
                qDebug()<<"s2d:"<<s2d;
                qDebug();
                qDebug()<<"depths:"<<depths;
                qDebug();

                //打印 测试下

                QString updateSound = QString("update feature set attvs_start = %1,attvs_end = %2,soundPoints = %3,depth = %4  where record_id =%5").arg(QString::number(attvsStart),QString::number(attvsEnd),s2d,depths,QString::number(pso->record_id));

                bool updateSoundRet = sqlQuery->exec(updateSound);
                if(!updateSoundRet)
                {
                    QSqlError lastError = sqlQuery->lastError();
                    qDebug()<<"updateSoundError:"<<lastError.databaseText();
                }
                qDebug()<<"sound attvs size:"<<psb->attvs.size();
                qDebug();

                saveFeatureAttvs(pso,psb->attvs);
            }
        }else{
            if(PointObject* ppo = dynamic_cast<PointObject*>(pso))
            {
                QString queryAttvs = "select * from attvs";
                int attvsStart = getSizeFromTable(queryAttvs);
                int interval = ppo->attvs.size();
                int attvsEnd = attvsStart + interval -1 > 0 ? attvsStart + interval -1 : 0;

                double px = pChart->sg2ds[ppo->index].long_lat[0].minutes;
                double py = pChart->sg2ds[ppo->index].long_lat[1].minutes;

                QString pointStr = QString("pointFromText('point(%1 %2)')").arg(QString::number(px,10,4),QString::number(py,10,4));

                QString updatePoint = QString("update feature set attvs_start = %1,attvs_end = %2,point = %3 where record_id = %4")
                        .arg(QString::number(attvsStart),QString::number(attvsEnd),pointStr,QString::number(ppo->record_id));




                bool updatePointRet = sqlQuery->exec(updatePoint);
                if(!updatePointRet)
                {
                    QSqlError lastError = sqlQuery->lastError();
                    qDebug()<<"updatePointRet:"<<lastError.text();
                }

                qDebug()<<"point attvs size:"<<ppo->attvs.size();
                qDebug();

                saveFeatureAttvs(pso,ppo->attvs);
            }
        }
    }
        break;

//      case 2:
//      {
//        if(LineObject* plb = dynamic_cast<LineObject*>(pso))
//        {

//            QString lineString = "geomfromtext('linestring(";
//            std::vector<int>::iterator lit;
//            std::vector<int>::iterator lend(plb->indices.end());
//            for(lit = plb->indices.begin();lit != lend;lit++)
//            {
//                double lineX = pChart->sg2ds[*lit].long_lat[0].minutes;
//                double lineY = pChart->sg2ds[*lit].long_lat[1].minutes;
//                lineString.append(QString("%1 %2").arg(QString::number(lineX,10,3),QString::number(lineY,10,3)));
//                if(lit != lend - 1)
//                {
//                    lineString.append(",");
//                }
//            }

//            lineString.append(QString(")')"));

//            QString updateLine = QString("update feature set line = %1 where record_id = %2").arg(lineString,QString::number(plb->record_id));

//          //  int strLen = lineString.length();

//            qDebug()<<lineString;
//            qDebug()<<updateLine;
//            bool updateLineRet = sqlQuery->exec(updateLine);
//            if(!updateLineRet)
//            {
//              QSqlError lastError = sqlQuery->lastError();
//              qDebug()<<"updateLineRet:"<<lastError.databaseText();
//            }
//        }
//      }

//        break;

//    case 3:
//    {
//     if(AreaObject* pab = dynamic_cast<AreaObject*>(pso))
//      {  //insert into test6(id,pgns) values(7,geometryFromText('multipolygon(((3 0,4 0,4 1,3 1,3 0)),((0 0,1 0,1 1,0 1,0 0)))'));

//          QString pgnString = QString("geomfromtext('multipolygon(");
//          std::vector<std::vector<int> >:: iterator ait;
//          std::vector<std::vector<int> >:: iterator aend = pab->contours.end();

//          pgnString.append("((");
//          for(ait = pab->contours.begin();ait != aend;ait++)
//          {
//            std::vector<int> ::iterator vit;
//            std::vector<int> ::iterator vend = ait->end();


//            for(vit = ait->begin();vit != vend;vit++)
//            {
//                double vx = pChart->sg2ds[*vit].long_lat[0].minutes;
//                double vy = pChart->sg2ds[*vit].long_lat[1].minutes;
//                pgnString.append(QString("%1 %2").arg(QString::number(vx,10,3),QString::number(vy,10,3)));
//       //         qDebug()<<"pngString"<<pgnString;
//                if(vit != vend -1)
//                {
//                    pgnString.append(",");
//                }
//            }

//            pgnString.append("))");
//            if(ait != aend -1)
//            {
//                pgnString.append(",");
//            }
//          }

//          pgnString.append(")')");

//          QString updatePng = QString("update feature set pgn = %1 where record_id = %2").arg(pgnString,QString::number(pab->record_id));
//          qDebug()<<updatePng;
//          qDebug();

//          bool updatePngRet = sqlQuery->exec(updatePng);
//          if(!updatePngRet)
//          {
//              QSqlError lastError = sqlQuery->lastError();
//              qDebug()<<"updatePngRet:"<<lastError.databaseText();
//          }

//      }
//    }
//      break;
    }//end switch

    //存储其他属性值
//    saveFeatureNatfs(pso,pChart);
//    saveFeatureFfpts(pso,pChart);
//    saveFeatureFspts(pso,pChart);
}



















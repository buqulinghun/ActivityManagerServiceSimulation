#include <QtGui/QApplication>
#include<QSqlDatabase>
#include "mainwindow.h"
#include<QDebug>
#include<QSqlQuery>
#include<QSqlError>
#include<iostream>
using namespace std;

int main(int argc, char *argv[])
{
    /*
           select id,AsText(pgns) from test6 where MBRIntersects(geometryfromText('polygon(0 0,2 0,2 2,0 2,0 0)'),pgns);

           insert into test6(id,pgns) values(7,geometryFromText('multipolygon(((3 0,4 0,4 1,3 1,3 0)),((0 0,1 0,1 1,0 1,0 0)))'));

           insert into test6(id,pgns) values(7,geometryFromText('multipolygon(((3 0,4 0,4 1,3 1,3 0)),((0 0,1 0,1 1,0 1,0 0)))'));

*/

    QCoreApplication a(argc, argv);
     QSqlDatabase db=QSqlDatabase::addDatabase("QMYSQL");
     db.setHostName("localhost");
     db.setDatabaseName("geodatabase");
     db.setUserName("root");
     db.setPassword("123");

   //  QString str = "select id,name,AsText(pgn) from test";
   //   QString str = "select * from attfs";
    // qDebug()<<str;


     if(!db.open()){
         qDebug()<<"Unable to open database";
     }else{
         qDebug()<<"Database connection established";
     }


     QSqlQuery query;
//     bool sucess = query.exec(str);
//     if(!sucess){
//         qDebug()<<"query fail";
//     }

//     qDebug()<<"size:"<<query.size();


//     while(query.next()){
//         qDebug()<<query.value(0)<<" "<<query.value(1)<<" "<<query.value(2);
//     }

//     QString instStr = "insert into test(id,name,pgn) values(null,'with2',GEOMFROMTEXT('POLYGON((2 2,4 2,4 5,2 5,2 2))'))";
//     qDebug()<<instStr;
//     query.prepare(instStr);
//     bool inst = query.exec();
//     if(!inst){
//         qDebug()<<"insert fail";
//     }

     QString createSql = "create table if not exists test8(id integer primary key,\
                                             name varchar(11),\
                                             points MultiPoint,\
                                             line linestring,\
                                             isOK BOOL,\
                                             pgns MultiPolygon);";
     qDebug()<<"create:"<<createSql;
     bool creSuc = query.exec(createSql);
     if(!creSuc){
        cerr<<"create table fail"<<endl;
     }

     // insert into test6(id,pgns) values(7,geometryFromText('multipolygon(((3 0,4 0,4 1,3 1,3 0)),((0 0,1 0,1 1,0 1,0 0)))'));
//  QString insertStr ="insert into test8(id,name) values(:id,:name)";
   QString insertStr ="insert into test8(id,name,isOK) values(?,?,?)";
   query.prepare(insertStr);
    query.bindValue(0,6);
    // query.bindValue(1,QVariant(QString("yubin")));
    // QString geo = QString("geometryFromText('multipolygon(((3 0,4 0,4 1,3 1,3 0)),((0 0,1 0,1 1,0 1,0 0)))')");
   //QString line = QString("LineStringFromtext('LineString(0 0,10 10,20 20)')");
   //  qDebug()<<line;
   query.bindValue(1,QVariant(QString("wahaha")));
   query.bindValue(2,true);

   bool insertSucess = query.exec();
   if(!insertSucess)
       {
           QSqlError lastError = query.lastError();
           qDebug()<<"insert value fail:"<<lastError.databaseText();

       }

//   QString addQuery = QString("update test8 set line = %1").arg("LineStringFromtext('LineString(0 0,10 10,20 20)')");
//   qDebug()<<addQuery;
//   query.exec(addQuery);

//     QString lineInsert = "insert into test8(id,pgns) values(8,geometryFromText('multipolygon(((3 0,4 0,4 1,3 1,3 0)),((0 0,1 0,1 1,0 1,0 0)))'))";
//     bool insertSucess = query.exec(lineInsert);

//     if(!insertSucess)
//     {
//         QSqlError lastError = query.lastError();
//         qDebug()<<"insert value fail:"<<lastError.databaseText();

//     }

   QString findQuery = "select * from test8 where name = 'wahaha'";
   query.exec(findQuery);
   qDebug()<<"size:"<<query.size();


     return a.exec();
}

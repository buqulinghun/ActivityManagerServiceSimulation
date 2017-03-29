/********************************************************************
 *日期: 2016-01-22
 *作者: 王名孝
 *作用: 加载原始提供的物标描述文件生成.png的图像文件保存，以后只需加载即可
 *修改:开始使用QGraphicsSvgItem矢量图像来绘制，但是在使用纹理时不可使用，所以还是回到开始使用时的QPixmap来绘制
 ********************************************************************/

#include "symbols.h"
#include "configuration.h"
#include "ecdis.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QSvgGenerator>
#include <QPen>
#include <QPainter>


extern Configuration *configuration;;
extern MARINERSELECT MarinerSelect;

Symbols::Symbols(QObject *parent) :
    QObject(parent)
{

}
Symbols::~Symbols()
{

}

void Symbols::Init()
{
#ifdef GENERATOR
    LoadS57Symbols();
    LoadS57ComplexLine();
    LoadS57Patterns();
#endif

    LoadAllSvg();

}

void Symbols::LoadAllSvg()
{
    QString path = QString("symbol/bright/");

    //加载点物标并存储
    QString filepath = path + QString("symbols/");
    QFile file(filepath + "control.dat");
    if(! file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"can't open files:"<< file.fileName();
        return;
    }
    QTextStream in(&file);
    while(! in.atEnd()){
        QString line = in.readLine();   //获取物标名称
        const QStringList context = line.split(",");
        if(context.size() == 3){
            const QPoint point = QPoint(context.at(1).toInt(), context.at(2).toInt());
            const QString m_path = filepath + context.at(0) + QString(".png");
            QFile m_file(m_path);
            if(m_file.exists()){
                MySvgItem* myItem = new MySvgItem(m_file.fileName(), point);
                bright_symbols.insert(context.at(0), myItem);
            }else{
                qDebug() << "symbols file is not exist:"<< m_file.fileName();
            }

        }else{
            qDebug() << "error while load /symbols/control.dat!";
        }
    }
    file.close();

    //加载线物标并存储
    filepath = path + QString("lines/");
    QFile file1(filepath + "control.dat");
    if(! file1.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"can't open files:"<< file1.fileName();
        return;
    }
    QTextStream in1(&file1);
    while(! in1.atEnd()){
        QString line = in1.readLine();   //获取物标名称
        const QStringList context = line.split(",");
        if(context.size() == 6){
            const QPoint point = QPoint(context.at(1).toInt(), context.at(2).toInt());
            const QString m_path = filepath + context.at(0) + QString(".png");
            QFile m_file(m_path);
            if(m_file.exists()){
                MyLineSvgItem* myItem = new MyLineSvgItem(m_file.fileName(), point);

                bright_complexlines.insert(context.at(0), myItem);
                myItem->setMapColor(context.at(3).toInt(), context.at(4).toInt(), context.at(5).toInt());
            }else{
                qDebug() << "lines file is not exist:"<< m_file.fileName();
            }

        }else{
            qDebug() << "error while load /lines/control.dat!";
        }
    }
    file1.close();

    //加载填充物标
    filepath = path + QString("patterns/");
    QFile file2(filepath + "control.dat");
    if(! file2.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"can't open files:"<< file2.fileName();
        return;
    }
    QTextStream in2(&file2);
    while(! in2.atEnd()){
        QString line = in2.readLine();   //获取物标名称
        const QStringList context = line.split(",");
        if(context.size() == 7){
            const QPoint point = QPoint(context.at(1).toInt(), context.at(2).toInt());
            const QString m_path = filepath + context.at(0) + QString(".png");
            QFile m_file(m_path);
            if(m_file.exists()){
                MyPatternSvgItem* myItem = new MyPatternSvgItem(m_file.fileName(), point);
                bool stgLin = context.at(3).toInt();
                bool conScl = context.at(4).toInt();
                quint16 miniDist = context.at(5).toInt();
                quint16 maxDist = context.at(6).toInt();
                myItem->setPatternPara(miniDist, maxDist, stgLin, conScl);   //设置参数
                bright_patterns.insert(context.at(0), myItem);
            }else{
                qDebug() << "patterns file is not exist:"<< m_file.fileName();
            }

        }else{
            qDebug() << "error while load /patterns/control.dat!";
        }
    }
    file2.close();


}

#ifdef GENERATOR
//加载点物标图形
void Symbols::LoadS57Symbols()
{
    QFile file_save("symbol/bright/symbols/control.dat");
    if(! file_save.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        qDebug()<<"can't open symbol/bright/symbols/control.dat files for write!";
        return;
    }
    QTextStream out(&file_save);

    QFile file("S57Lib/S57Symbols");
    if(! file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"can't open S57Lib/S57Symbols files!";
        return;
     }
    QTextStream in(&file);
    while(! in.atEnd()){
        QString line = in.readLine();
        if(line.contains("SYMD")){
            //找到一个物标
            QPoint pivot_point,bounding_box,upper_left;
            QMap<QString,QString> colour_table;
            QString symbol_name = line.mid(9,8);
            pivot_point.setX(line.mid(18,5).toInt() * FACTOR);
            pivot_point.setY(line.mid(23,5).toInt() * FACTOR);
            bounding_box.setX(line.mid(28,5).toInt() * FACTOR);
            bounding_box.setY(line.mid(33,5).toInt() * FACTOR);
            upper_left.setX((line.mid(38,5).toInt() - 20) * FACTOR);
            upper_left.setY((line.mid(43,5).toInt() - 15) * FACTOR);
            //SXPO描述信息
            line.clear();
            line = in.readLine();
            QString description = line.mid(9, line.size() - 10);

            line.clear();
            line = in.readLine();
            if(line.contains("SCRF")){  //最多三种画笔
                if(line.size() == 15)
                    colour_table.insert(line.mid(9,1),line.mid(10,5));
                else if(line.size() == 21){
                    colour_table.insert(line.mid(9,1),line.mid(10,5));
                    colour_table.insert(line.mid(15,1),line.mid(16,5));
                }else if(line.size() == 27){
                    colour_table.insert(line.mid(9,1),line.mid(10,5));
                    colour_table.insert(line.mid(15,1),line.mid(16,5));
                    colour_table.insert(line.mid(21,1),line.mid(22,5));
                }else{
                    qDebug()<<"error while Load SCRF filed of symbols file!";
                }
            }else{
                qDebug()<<"error! no SCRF filed!";
            }
/*
            //初始化svg生成器
            QString path = QString("svg/bright/symbols/") + symbol_name + QString(".svg");
            QSvgGenerator generator;
            generator.setFileName(path);
            generator.setSize(QSize((bounding_box.x() + 50*FACTOR), (bounding_box.y() + 50*FACTOR)));
            generator.setViewBox(QRect(0, 0, (bounding_box.x() + 50*FACTOR), (bounding_box.y() + 50*FACTOR)));
            generator.setTitle(symbol_name);     //名称设置为偏移量
            generator.setDescription(description);
*/
            QPixmap pixmap(QSize((bounding_box.x() + 3), (bounding_box.y() + 3)));
            pixmap.fill(Qt::transparent);
            //一毫米约等于3.78像素,单位0.01毫米
            QPen pen;
            QColor color;
            QPainter painter;
            QPointF start(0.0,0.0);
            painter.begin(&pixmap);
        //  painter.setRenderHint(QPainter::Antialiasing,true);   //抗锯齿

            //SVCT绘图操作
            line.clear();
            line = in.readLine();
            while(line.contains("SVCT")){
               line = line.mid(9);
               QStringList operation = line.split(";");
               QPainterPath path;
               QBrush brush(Qt::SolidPattern);
               QPolygon polygon;
               int polygon_flag(0);
               int x1(0),y1(0);
               path.moveTo(start);
               for(int i=0; i < operation.size(); i++){
                  QString s = operation.at(i);
                  if(s.contains("SP")){
                      QString c = colour_table.value(s.right(1));   //获取对应颜色名称
                      QMap<QString, Rgb>::const_iterator col = configuration->color_map.find(MarinerSelect.dayState).value().find(c);   //寻找迭代器
                      color.setRgb(col.value().r, col.value().g, col.value().b, 255);
                      pen.setColor(color);
                  }else if(s.contains("SW")){
                      pen.setWidth(s.right(1).toInt());  //单位0.3mm,在1440*900的显示器为0.285mm一个像素,但是不好看
                      painter.setPen(pen);
                  }else if(s.contains("ST")){
                      int d = s.right(1).toInt();
                      switch(d){
                      case 0: color.setAlphaF(1.0); break;
                      case 1: color.setAlphaF(0.75); break;
                      case 2: color.setAlphaF(0.5); break;
                      case 3: color.setAlphaF(0.25); break;
                      }
                      brush.setColor(color);
                  }else if(s.contains("PU")){
                      x1 = s.split(",").at(0).mid(2).toInt() * FACTOR;
                      y1 = s.split(",").at(1).toInt() * FACTOR;
                      path.moveTo((x1-upper_left.x()), (y1-upper_left.y()));
                  }else if(s.contains("PM0")){
                      polygon<< QPoint((x1-upper_left.x()), (y1-upper_left.y()));
                      polygon_flag = 1;
                  } else if(s.contains("PD")){
                      int size = s.split(",").size();
                      if(size != 1)   //不知道啥意思.在DANGER区域
                      {
                          for(int s_l = 0; s_l<size; s_l+=2){
                              int x(0),y(0);
                              if(s_l == 0)
                                  x = s.split(",").at(s_l).mid(2).toInt() * FACTOR;
                              else
                                  x = s.split(",").at(s_l).toInt() * FACTOR;
                               y = s.split(",").at(s_l+1).toInt() * FACTOR;
                               if(polygon_flag)
                                   polygon << QPoint((x-upper_left.x()), (y-upper_left.y()));
                                else
                                   path.lineTo((x-upper_left.x()), (y-upper_left.y()));
                         }
                       }else
                           painter.drawPoint(QPoint((x1-upper_left.x()), (y1-upper_left.y())));
                   }else if(s.contains("PM2")){
                       polygon_flag = 0;
                       painter.save();
                       QPainterPath polygon_path;
                       polygon_path.addPolygon(polygon);
                       painter.setBrush(brush);     //此处已经完成了FP字段的填充颜色功能，因为都是在PM2后面，所以不需要另外加了
                       painter.setPen(pen);
                       painter.drawPath(polygon_path);
                       painter.restore();
                       polygon.clear();

                   }else if(s.contains("CI")){
                      int radius = s.mid(2).toInt() * FACTOR;
                      if(polygon_flag){
                          painter.save();
                          polygon_flag = 0;
                          QPainterPath polygon_path;
                          polygon_path.addEllipse(QPoint((x1-upper_left.x()), (y1-upper_left.y())), radius,radius);
                          painter.setBrush(brush);
                          painter.setPen(pen);
                          painter.drawPath(polygon_path);
                          painter.restore();
                      }else{
                          path.addEllipse(QPoint((x1-upper_left.x()), (y1-upper_left.y())),radius, radius);
                      }
                  }
               }
               line.clear();
               line = in.readLine();
               painter.drawPath(path);
               start = path.currentPosition();
            }
            painter.end();
            QString filepath = QString("symbol/bright/symbols/") + symbol_name + QString(".png");
            pixmap.save(filepath, "PNG");
            //将文件名及偏移量存入文件中好读取
            const QPoint Offset = QPoint((pivot_point.x()-upper_left.x()), (pivot_point.y()-upper_left.y()));
            const QString offset = QString("%1,%2").arg(Offset.x()).arg(Offset.y());
            out << symbol_name << "," << offset << "\n";
        } //end if

    }
    file.close();
    file_save.close();
}
//加载复杂线型物标
void Symbols::LoadS57ComplexLine()
{
    QFile file_save("symbol/bright/lines/control.dat");
    if(! file_save.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        qDebug()<<"can't open symbol/bright/lines/control.dat files for write!";
        return;
    }
    QTextStream out(&file_save);

    QFile file("S57Lib/S57ComplexLineStyles");
    if(! file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"can't open S57Lib/S57ComplexLineStyles files!";
        return;
     }
    QTextStream in(&file);
    while(! in.atEnd()){
        QString line = in.readLine();
        if(line.contains("LIND")){
            //找到一个物标
            QPoint pivot_point,bounding_box,upper_left;
            QMap<QString,QString> colour_table;
            QString line_name = line.mid(9,8);
            pivot_point.setX(line.mid(17,5).toInt() * FACTORLINE);
            pivot_point.setY(line.mid(22,5).toInt() * FACTORLINE);
            bounding_box.setX(line.mid(27,5).toInt() * FACTORLINE);
            bounding_box.setY(line.mid(32,5).toInt() * FACTORLINE);
            upper_left.setX((line.mid(37,5).toInt() - 20) * FACTORLINE);
            upper_left.setY((line.mid(42,5).toInt() - 15) * FACTORLINE);
            //SXPO描述信息
            line.clear();
            line = in.readLine();
            QString description = line.mid(9, line.size() - 10);

            line.clear();
            line = in.readLine();
            if(line.contains("LCRF")){  //最多三种画笔
                if(line.size() == 15)
                    colour_table.insert(line.mid(9,1),line.mid(10,5));
                else if(line.size() == 21){
                    colour_table.insert(line.mid(9,1),line.mid(10,5));
                    colour_table.insert(line.mid(15,1),line.mid(16,5));
                }else if(line.size() == 27){
                    colour_table.insert(line.mid(9,1),line.mid(10,5));
                    colour_table.insert(line.mid(15,1),line.mid(16,5));
                    colour_table.insert(line.mid(21,1),line.mid(22,5));
                }else{
                    qDebug()<<"error while Load LCRF filed of symbols file!";
                }
            }else{
                qDebug()<<"error! no LCRF filed!";
            }
/*
            //初始化svg生成器
            QString path = QString("svg/bright/lines/") + line_name + QString(".svg");
            QSvgGenerator generator;
            generator.setFileName(path);
            generator.setSize(QSize((bounding_box.x() + 50*FACTOR), (bounding_box.y() + 50*FACTOR)));
            generator.setViewBox(QRect(0, 0, (bounding_box.x() + 50*FACTOR), (bounding_box.y() + 50*FACTOR)));
            generator.setTitle(line_name);
            generator.setDescription(description);
*/

            QPixmap pixmap(QSize((bounding_box.x() + 3), (bounding_box.y() + 3)));
            pixmap.fill(Qt::transparent);

            //一毫米约等于3.78像素,单位0.01毫米
            QPen pen;
            QColor color;
            QPainter painter;
            QPointF start(0.0,0.0);
            painter.begin(&pixmap);
        //  painter.setRenderHint(QPainter::Antialiasing,true);   //抗锯齿

            //LVCT绘图操作
            line.clear();
            line = in.readLine();
            while(line.contains("LVCT")){
                line = line.mid(9);
                QStringList operation = line.split(";");
                QPainterPath path;
                QBrush brush(Qt::SolidPattern);
                QPolygon polygon;
                int polygon_flag(0);
                int x1(0),y1(0);
                path.moveTo(start);
                for(int i=0; i < operation.size(); i++){
                   QString s = operation.at(i);
                   if(s.contains("SP")){
                       QString c = colour_table.value(s.right(1));   //获取对应颜色名称
                       QMap<QString, Rgb>::const_iterator col = configuration->color_map.find(MarinerSelect.dayState).value().find(c);   //寻找迭代器
                       color.setRgb(col.value().r, col.value().g, col.value().b);
                       pen.setColor(color);
                   }else if(s.contains("SW")){
                       pen.setWidth(s.right(1).toInt());  //单位0.3mm,在1440*900的显示器为0.285mm一个像素,但是不好看
                       painter.setPen(pen);
                   }else if(s.contains("ST")){
                       int d = s.right(1).toInt();
                       switch(d){
                       case 0: color.setAlphaF(1.0); break;
                       case 1: color.setAlphaF(0.75); break;
                       case 2: color.setAlphaF(0.5); break;
                       case 3: color.setAlphaF(0.25); break;
                       }
                       brush.setColor(color);
                   }else if(s.contains("PU")){
                       x1 = s.split(",").at(0).mid(2).toInt() * FACTORLINE;
                       y1 = s.split(",").at(1).toInt() * FACTORLINE;
                       path.moveTo((x1-upper_left.x()), (y1-upper_left.y()));
                   }else if(s.contains("PM0")){
                       polygon<< QPoint((x1-upper_left.x()), (y1-upper_left.y()));
                       polygon_flag = 1;
                   } else if(s.contains("PD")){
                       int size = s.split(",").size();
                       if(size != 1)   //不知道啥意思.在DANGER区域
                       {
                           for(int s_l = 0; s_l<size; s_l+=2){
                               int x(0),y(0);
                               if(s_l == 0)
                                   x = s.split(",").at(s_l).mid(2).toInt() * FACTORLINE;
                               else
                                   x = s.split(",").at(s_l).toInt() * FACTORLINE;
                                y = s.split(",").at(s_l+1).toInt() * FACTORLINE;
                                if(polygon_flag)
                                    polygon << QPoint((x-upper_left.x()), (y-upper_left.y()));
                                 else
                                    path.lineTo((x-upper_left.x()), (y-upper_left.y()));
                          }
                        }else
                            painter.drawPoint(QPoint((x1-upper_left.x()), (y1-upper_left.y())));
                    }else if(s.contains("PM2")){
                        polygon_flag = 0;
                        painter.save();
                        QPainterPath polygon_path;
                        polygon_path.addPolygon(polygon);
                        painter.setBrush(brush);
                        painter.setPen(pen);
                        painter.drawPath(polygon_path);
                        painter.restore();
                        polygon.clear();

                    }else if(s.contains("CI")){
                       int radius = s.mid(2).toInt() * FACTORLINE;
                       if(polygon_flag){
                           painter.save();
                           polygon_flag = 0;
                           QPainterPath polygon_path;
                           polygon_path.addEllipse(QPoint((x1-upper_left.x()), (y1-upper_left.y())), radius, radius);
                           painter.setBrush(brush);
                           painter.setPen(pen);
                           painter.drawPath(polygon_path);
                           painter.restore();
                       }else{
                           path.addEllipse(QPoint((x1-upper_left.x()), (y1-upper_left.y())),radius, radius);
                       }
                   }
                }
                line.clear();
                line = in.readLine();
                painter.drawPath(path);
                start = path.currentPosition();
             }
            painter.end();
            QString filepath = QString("symbol/bright/lines/") + line_name + QString(".png");
            pixmap.save(filepath, "PNG");
            //将文件名存入文件中好读取
            const QPoint Offset = QPoint((pivot_point.x()-upper_left.x()), (pivot_point.y()-upper_left.y()));
            const QString offset = QString("%1,%2").arg(Offset.x()).arg(Offset.y());
            out << line_name << "," << offset << "," << color.red() << ","<< color.green()
                    << "," << color.blue() << "\n";
        }   //end if
    }
    file.close();
    file_save.close();
}
//加载模板图形
void Symbols::LoadS57Patterns()
{
    QFile file_save("symbol/bright/patterns/control.dat");
    if(! file_save.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        qDebug()<<"can't open symbol/bright/patterns/control.dat files for write!";
        return;
    }
    QTextStream out(&file_save);

    QFile file("S57Lib/S57Patterns");
    if(! file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"can't open S57Lib/S57ComplexLineStyles files!";
        return;
    }
    QTextStream in(&file);
    while(! in.atEnd()){
        QString line = in.readLine();
        if(line.contains("PATD")){
            //找到一个物标
            QPoint pivot_point,bounding_box,upper_left;
            QMap<QString,QString> colour_table;
            QString pattern_name = line.mid(9,8);
            //中间还有填充模式，间距模式，间距最小最大值
            quint8 stgLin = (line.mid(18, 3) == "STG") ? 1 : 0;
            quint8 conScl = (line.mid(21, 3) == "CON") ? 1 : 0;
            quint16 miniDist = line.mid(24, 5).toInt() * FACTOR;
            quint16 maxDist = line.mid(29, 5).toInt() * FACTOR;
            pivot_point.setX(line.mid(34,5).toInt() * FACTOR);
            pivot_point.setY(line.mid(39,5).toInt() * FACTOR);
            bounding_box.setX(line.mid(44,5).toInt() * FACTOR);
            bounding_box.setY(line.mid(49,5).toInt() * FACTOR);
            upper_left.setX((line.mid(54,5).toInt() - 20) * FACTOR);
            upper_left.setY((line.mid(59,5).toInt() - 15) * FACTOR);
            //SXPO描述信息
            line.clear();
            line = in.readLine();
            QString description = line.mid(9, line.size() - 10);

            line.clear();
            line = in.readLine();
            if(line.contains("PCRF")){  //最多三种画笔
                if(line.size() == 15)
                    colour_table.insert(line.mid(9,1),line.mid(10,5));
                else if(line.size() == 21){
                    colour_table.insert(line.mid(9,1),line.mid(10,5));
                    colour_table.insert(line.mid(15,1),line.mid(16,5));
                }else if(line.size() == 27){
                    colour_table.insert(line.mid(9,1),line.mid(10,5));
                    colour_table.insert(line.mid(15,1),line.mid(16,5));
                    colour_table.insert(line.mid(21,1),line.mid(22,5));
                }else{
                    qDebug()<<"error while Load PCRF filed of symbols file!";
                }
            }else{
                qDebug()<<"error! no PCRF filed!";
            }
/*
            //初始化svg生成器
            QString path = QString("svg/bright/patterns/") + pattern_name + QString(".svg");
            QSvgGenerator generator;
            generator.setFileName(path);
            generator.setSize(QSize((bounding_box.x() + 50*FACTOR), (bounding_box.y() + 50*FACTOR)));
            generator.setViewBox(QRect(0, 0, (bounding_box.x() + 50*FACTOR), (bounding_box.y() + 50*FACTOR)));
            generator.setTitle(pattern_name);
            generator.setDescription(description);
*/
            QPixmap pixmap(QSize((bounding_box.x() + 3), (bounding_box.y() + 3)));
            pixmap.fill(Qt::transparent);

            //一毫米约等于3.78像素,单位0.01毫米
            QPen pen;
            QColor color;
            QPainter painter;
            QPointF start(0.0,0.0);
            painter.begin(&pixmap);
        //  painter.setRenderHint(QPainter::Antialiasing,true);   //抗锯齿

            //PVCT绘图操作
            line.clear();
            line = in.readLine();
            while(line.contains("PVCT")){
                line = line.mid(9);
                QStringList operation = line.split(";");
                QPainterPath path;
                QBrush brush(Qt::SolidPattern);
                QPolygon polygon;
                int polygon_flag(0);
                int x1(0),y1(0);
                path.moveTo(start);
                for(int i=0; i < operation.size(); i++){
                   QString s = operation.at(i);
                   if(s.contains("SP")){
                       QString c = colour_table.value(s.right(1));   //获取对应颜色名称
                       QMap<QString, Rgb>::const_iterator col = configuration->color_map.find(MarinerSelect.dayState).value().find(c);   //寻找迭代器
                       color.setRgb(col.value().r, col.value().g, col.value().b);
                       pen.setColor(color);
                   }else if(s.contains("SW")){
                       pen.setWidth(s.right(1).toInt());  //单位0.3mm,在1440*900的显示器为0.285mm一个像素,但是不好看
                       painter.setPen(pen);
                   }else if(s.contains("ST")){
                       int d = s.right(1).toInt();
                       switch(d){
                       case 0: color.setAlphaF(1.0); break;
                       case 1: color.setAlphaF(0.75); break;
                       case 2: color.setAlphaF(0.5); break;
                       case 3: color.setAlphaF(0.25); break;
                       }
                       brush.setColor(color);
                   }else if(s.contains("PU")){
                       x1 = s.split(",").at(0).mid(2).toInt() * FACTOR;
                       y1 = s.split(",").at(1).toInt() * FACTOR;
                       path.moveTo((x1-upper_left.x()), (y1-upper_left.y()));
                   }else if(s.contains("PM0")){
                       polygon<< QPoint((x1-upper_left.x()), (y1-upper_left.y()));
                       polygon_flag = 1;
                   } else if(s.contains("PD")){
                       int size = s.split(",").size();
                       if(size != 1)   //不知道啥意思.在DANGER区域
                       {
                           for(int s_l = 0; s_l<size; s_l+=2){
                               int x(0),y(0);
                               if(s_l == 0)
                                   x = s.split(",").at(s_l).mid(2).toInt() * FACTOR;
                               else
                                   x = s.split(",").at(s_l).toInt() * FACTOR;
                                y = s.split(",").at(s_l+1).toInt() * FACTOR;
                                if(polygon_flag)
                                    polygon << QPoint((x-upper_left.x()), (y-upper_left.y()));
                                 else
                                    path.lineTo((x-upper_left.x()), (y-upper_left.y()));
                          }
                        }else
                            painter.drawPoint(QPoint((x1-upper_left.x()), (y1-upper_left.y())));
                    }else if(s.contains("PM2")){
                        polygon_flag = 0;
                        painter.save();
                        QPainterPath polygon_path;
                        polygon_path.addPolygon(polygon);
                        painter.setBrush(brush);
                        painter.setPen(pen);
                        painter.drawPath(polygon_path);
                        painter.restore();
                        polygon.clear();

                    }else if(s.contains("CI")){
                       int radius = s.mid(2).toInt();
                       if(polygon_flag){
                           painter.save();
                           polygon_flag = 0;
                           QPainterPath polygon_path;
                           polygon_path.addEllipse(QPoint((x1-upper_left.x()), (y1-upper_left.y())), radius, radius);
                           painter.setBrush(brush);
                           painter.setPen(pen);
                           painter.drawPath(polygon_path);
                           painter.restore();
                       }else{
                           path.addEllipse(QPoint((x1-upper_left.x()), (y1-upper_left.y())),radius, radius);
                       }
                   }
                }
                line.clear();
                line = in.readLine();
                painter.drawPath(path);
                start = path.currentPosition();
             }
            painter.end();
            QString filepath = QString("symbol/bright/patterns/") + pattern_name + QString(".png");
            pixmap.save(filepath, "PNG");
            //将文件名存入文件中好读取
            const QPoint Offset = QPoint((pivot_point.x()-upper_left.x()), (pivot_point.y()-upper_left.y()));
            const QString offset = QString("%1,%2").arg(Offset.x()).arg(Offset.y());
            out << pattern_name << "," << offset << "," << stgLin << "," << conScl << "," << miniDist << "," << maxDist << "\n";
        }  //end if

    }
    file.close();
    file_save.close();
}

#endif

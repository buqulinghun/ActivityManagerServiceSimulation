/********************************************************************
 *日期: 2016-01-22
 *作者: 王名孝
 *作用: 加载原始提供的物标描述文件生成.png的图像文件保存，以后只需加载即可
 *修改:开始使用QGraphicsSvgItem矢量图像来绘制，但是在使用纹理时不可使用，所以还是回到开始使用时的QPixmap来绘制
 ********************************************************************/

#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <QObject>
#include <QString>
#include <QHash>

#include "mysvgitem.h"


#define GENERATOR
#define FACTOR 0.02   //单位为0.01mm，假设一个像素为0.312mm，需要除以30  0.032
#define FACTORLINE 0.022

class Symbols : public QObject
{
Q_OBJECT
public:
    explicit Symbols(QObject *parent = 0);
    ~Symbols();
    void Init();   //初始化

    //图像存储表
    QHash<QString, MySvgItem*> bright_symbols;
    QHash<QString, MyLineSvgItem*> bright_complexlines;
    QHash<QString, MyPatternSvgItem*> bright_patterns;

private:

    //加载svg文件存储用来显示
    void LoadAllSvg();


#ifdef GENERATOR
    //读取符号库文件并生成svg文件
    void LoadS57Symbols();
    //读取复杂线型
    void LoadS57ComplexLine();
    //读取填充图形
    void LoadS57Patterns();
#endif

};

#endif // SYMBOLS_H

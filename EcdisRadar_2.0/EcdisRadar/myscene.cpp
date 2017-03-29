#include "myscene.h"
#include "configuration.h"
#include "mycomplexlineitem.h"
#include "ecdis.h"
#include "mainwindow.h"
#include "radarFile/radaritem.h"
#include "radarFile/interact.h"

#include <QGraphicsPathItem>
#include <QGraphicsItem>
#include <QTime>
#include <QDebug>
#include <QPainter>
#include <QGraphicsSceneDragDropEvent>


#define RADARALLITEM

extern SYSTEMINFO RadarSysinfo;
extern Configuration *configuration;
extern MARINERSELECT MarinerSelect;
extern MainWindow* lpMainWindow;
extern Symbols *symbols;   //加载物标图形
extern RadarItem *lpRadarItem;
extern Interact * lpInteract;


MyScene::MyScene(QObject *parent) :
    QGraphicsScene(parent), conditionalItems(new ConditionalItem)
{
    radarAllItems = NULL;
    chartAllItemsUnderRadar = NULL;
    chartAllItemsOverRadar = NULL;

    nowChart = NULL;
    symbols = new Symbols;
    //加载点物标图像
    symbols->Init();
    nowTime = QDateTime::currentDateTime();


}


void MyScene::initData(QVector<Chart *> *data)
{
    openChartData = data;
}

//根据选择显示相应海图
void MyScene::renderChart(std::string chart_description)
{
    QVector<Chart *>::iterator cit;
    QVector<Chart *>::iterator cend = openChartData->end();
    for(cit = openChartData->begin(); cit != cend; ++cit) {
        //查找对应海图数据并显示
        if((*cit)->description == chart_description) {
           // this->clear();  //删除之前的图形项
            if(chartAllItemsUnderRadar && chartAllItemsOverRadar){
                this->removeItem(chartAllItemsUnderRadar);
                this->removeItem(chartAllItemsOverRadar);
            }

            conditionalItems->setNowChart(*cit);  //设置条件物标的当前处理海图
            GenerateInstruction(*cit, true, true, true);   //产生符号化语言,只能产生一次

            centerLong = ((*cit)->wlon.minutes + (*cit)->elon.minutes) / 2;
            centerLat = ((*cit)->nlat.minutes + (*cit)->slat.minutes) / 2;
            centerLongScreen = ((*cit)->Melon - (*cit)->Mwlon) / 2;
            centerLatScreen = ((*cit)->Mnlat - (*cit)->Mslat) / 2;

            Render(*cit);
            nowChartName = QString::fromStdString(chart_description);
        }
    }
}

//绘制单幅海图,查询到的到lookup表并解析
void MyScene::Render(Chart *chart)
{

#ifdef RADARALLITEM
     chartAllItemsUnderRadar = new QGraphicsItemGroup;
     chartAllItemsOverRadar = new QGraphicsItemGroup;
#endif
   // QTime tt;
  //  tt.start();
    lpMainWindow->setScaleDisp(MarinerSelect.chartScaleNum);
    nowChart = chart;
    QMap<QString, Rgb> &colorTable =  configuration->color_map.find(MarinerSelect.dayState).value();
    //draw areas
    {
        std::map<int, std::vector<AreaObject> >::iterator aom_it;
        std::map<int, std::vector<AreaObject> >::iterator aom_end = chart->area_objects_map.end();
        for(aom_it = chart->area_objects_map.begin(); aom_it != aom_end; ++aom_it) {
            std::vector<AreaObject>::iterator ao_it;
            std::vector<AreaObject>::iterator ao_end = aom_it->second.end();
            for(ao_it = aom_it->second.begin(); ao_it != ao_end; ++ao_it) {
                if(ao_it->dispcategory <= MarinerSelect.dispCategory)
                    parseLookupTable(ao_it, chart->sg2ds, colorTable);
            }  //end for Areaobject
        }
    }

    //draw lines
    {
        std::map<int, std::vector<LineObject> >::iterator lom_it;
        std::map<int, std::vector<LineObject> >::iterator lom_end = chart->line_objects_map.end();
        for(lom_it = chart->line_objects_map.begin(); lom_it != lom_end; ++lom_it) {
            std::vector<LineObject>::iterator lo_it;
            std::vector<LineObject>::iterator lo_end = lom_it->second.end();
            for(lo_it = lom_it->second.begin(); lo_it != lo_end; ++lo_it) {
                if(lo_it->dispcategory <= MarinerSelect.dispCategory)
                    parseLookupTable(lo_it, chart->sg2ds, colorTable);
            }  // end for LineObject
        }
    }
    //draw points
    {
        std::map<int, std::vector<PointObject> >::iterator pom_it;
        std::map<int, std::vector<PointObject> >::iterator pom_end = chart->point_objects_map.end();
        for(pom_it = chart->point_objects_map.begin(); pom_it != pom_end; ++pom_it) {
            std::vector<PointObject>::iterator po_it;
            std::vector<PointObject>::iterator po_end = pom_it->second.end();
            for (po_it = pom_it->second.begin(); po_it != po_end; ++po_it) {
                if(po_it->dispcategory <= MarinerSelect.dispCategory)
                    parseLookupTable(po_it, chart->sg2ds, colorTable);
            }  // end for PointObject
        }
    }
    //draw sounds
    if(MarinerSelect.dispCategory >= OTHER){
        //水深点不需要取查找lookup表，因为只有一个固定值，所有都这样显示CS(SOUNDG02) OTHER 33010 OVERRADAR 6
        std::vector<SoundObject>::iterator so_it;
        std::vector<SoundObject>::iterator so_end = chart->sound_object_vector.end();
        for(so_it = chart->sound_object_vector.begin(); so_it != so_end; ++so_it) {
            int quapos = 0;
            int scamin = 0;
            std::vector<attf_t>::iterator attf_it;
            std::vector<attf_t>::iterator attf_end = so_it->attfs.end();
            for(attf_it = so_it->attfs.begin(); attf_it != attf_end; ++ attf_it){
                if((attf_it->attl == 133)) {    //SCAMIN
                    scamin = QString(attf_it->atvl.c_str()).toInt();
                }
            }
            if(scamin && (scamin < MarinerSelect.chartScaleNum))
                continue;
            std::vector<attv_t>::iterator attv_it;
            std::vector<attv_t>::iterator attv_end = so_it->attvs.end();
            for(attv_it = so_it->attvs.begin(); attv_it != attv_end; ++ attv_it){
                if((attv_it->attl == 402)) {
                    quapos = QString(attv_it->atvl.c_str()).toInt();
                }
            }

            std::vector<sg3d_t>::iterator it;
            std::vector<sg3d_t>::iterator end = so_it->sg3ds.end();
            for(it = so_it->sg3ds.begin(); it != end; ++it) {
                const QPointF pos = QPointF(it->long_lat[0], it->long_lat[1]);
                float depthValue = it->depth;
                QStringList symbol = conditionalItems->setup_SNDFRM03(depthValue, quapos, (so_it->attfs));
                const int size  = symbol.size();
                for(int i=0; i<size; i++) {
                    MySvgItem *myitem = symbols->bright_symbols.value(symbol.at(i));
                    const QPixmap map = myitem->svgItem;
                    QGraphicsPixmapItem *point = new QGraphicsPixmapItem(map);
                    point->setOffset(-myitem->offset.x(), -myitem->offset.y());
                    point->setPos(pos);
                    point->setZValue(6);
#ifdef RADARALLITEM
                    chartAllItemsOverRadar->addToGroup(point);
 #else
                    this->addItem(point);
 #endif
                }

            }
        }
    }

#ifdef RADARALLITEM
    this->addItem(chartAllItemsUnderRadar);
    chartAllItemsUnderRadar->setZValue(-1);
    this->addItem(chartAllItemsOverRadar);
    chartAllItemsOverRadar->setZValue(1);
#endif

    moveRadarToCenter();

    this->update();
  //  qDebug() << "time to render chart is: "<< tt.elapsed();

}
//比较是否处于周期段时间
bool MyScene::comparePertime(const QString &persta, const QString &perend)
{
    // CCYYMMDD/--MMDD/--MM
    if(persta.startsWith("--")) {  //周期性的
        int month = nowTime.date().month();
        int day = nowTime.date().day();
        int stamonth = persta.mid(2,2).toInt();
        int endmonth = perend.mid(2,2).toInt();
        if(persta.size() > 4) {  //带有日子
            bool mon_ok = stamonth < endmonth ? ((month > stamonth) && (month < endmonth)) : ((month > stamonth) || (month < endmonth));
            if(!mon_ok) {
                int staday = persta.mid(4,2).toInt();
                int endday = perend.mid(4,2).toInt();
                if(!((month == stamonth && day > staday) || (month == endmonth && day < endday)))
                    return false;
            }
        }else{
            bool mon_ok = stamonth < endmonth ? ((month >= stamonth) && (month <= endmonth)) : ((month >= stamonth) || (month <= endmonth));
            if(!mon_ok)
                return false;
        }

    }else{  //某个时间段
        QDate end = QDate(perend.mid(0,4).toInt(), perend.mid(4,2).toInt(), perend.mid(6,2).toInt());
        QDate start = QDate(persta.mid(0,4).toInt(), persta.mid(4,2).toInt(), persta.mid(6,2).toInt());
        if(!((nowTime < QDateTime(end)) && (nowTime > QDateTime(start))))
            return false;
    }
    return true;
}


void MyScene::GenerateInstruction(Chart *chart, bool area, bool line, bool point)
{
    // areas
    if(area){
        //查询表设置
        int areaType = MarinerSelect.plainArea ? PLAINAREA : SYMBOLAREA;
        QVector<LookupTable> &lookUp_Area = configuration->Tables.find(areaType).value();
        QVector<LookupTable>::iterator start_find;
        QVector<LookupTable>::iterator end_find = lookUp_Area.end();

        std::map<int, std::vector<AreaObject> >::iterator aom_it;
        std::map<int, std::vector<AreaObject> >::iterator aom_end = chart->area_objects_map.end();
        for(aom_it = chart->area_objects_map.begin(); aom_it != aom_end; ++aom_it) {
            //一类物标,标号为aom_it->first，查找这类物标的相应表
            QVector<LookupTable> tableAreaFind;
            for(start_find = lookUp_Area.begin(); start_find != end_find; ++start_find){
                if(aom_it->first == start_find->objName)  //匹配物标名称
                    tableAreaFind.append(*start_find);
            }
            const int find_size = tableAreaFind.size();   //跟据其匹配行的多少来选择
            //开始每个物标的查找
            std::vector<AreaObject>::iterator ao_it;
            std::vector<AreaObject>::iterator ao_end = aom_it->second.end();
            for(ao_it = aom_it->second.begin(); ao_it != ao_end; ++ao_it) {
                //查询Look-up表得到符号化内容
                LookupTable findItem;
                switch(find_size) {
                    case 0:    //没有找到，使用look-up表中的第一行进行显示
                    {
                        findItem = lookUp_Area.at(0);
                        qDebug()<< "not find the area table : object and recordID:" << aom_it->first << ao_it->record_id;
                    }
                    break;
                    case 1:   //只有一行，则不管属性如何，都使用它表示
                    {
                        findItem = tableAreaFind.at(0);
                    }
                    break;
                    default:  //大于一条表找到,从查找表中取属性及值在物标属性中查找，如果物标属性中包含所有查找表属性则选取，如果没有则取第一行查找表，其属性区域为空
                    {
                        bool flagFindSuccess = false;
                        for(int i=1; i < find_size; i++) {   //从第二行开始查找，没有找到就选第一行
                            const LookupTable table = tableAreaFind.at(i);
                            const int sizeAttr = table.attrValue.size();
                            quint8 findFlag = 0;
                            for(int j=0; j < sizeAttr; j++) {
                                const Attr att = table.attrValue.at(j);
                                //对lookup表中的每个属性在物标的属性列表中查找
                                std::vector<attf_t>::iterator attf_it;
                                std::vector<attf_t>::iterator attf_end = ao_it->attfs.end();
                                for(attf_it = ao_it->attfs.begin(); attf_it != attf_end; ++ attf_it){
                                    if((attf_it->attl == att.attl)) {
                                        if(att.attl == 117){    //ORIENT后面是没有跟数值的，只是在后面需要显示数值，但不用匹配
                                            findFlag++;
                                        }else{
                                            const QString at = QString(attf_it->atvl.c_str());   //物标属性值
                                            if(at.contains(",") && at.startsWith(att.atvl))
                                                findFlag++;
                                            else if(at == att.atvl)
                                                findFlag++;
                                            else if((att.atvl == "?") && (at == "unknow"))   //DRVAL1? DRVAL2?  这两个有问号，只有属性值为unknow时才能匹配上
                                                findFlag++;
                                        }
                                        break;     //进行下一个属性匹配
                                    }
                                }
                            }
                            //第一行找到的表就是所需的
                            if(findFlag == sizeAttr) {
                                findItem = table;
                                flagFindSuccess = true;
                                break;
                            }
                        }
                        //没有找到合适的表，选择第一行
                        if(!flagFindSuccess)
                            findItem = tableAreaFind.at(0);
                    }
                    break;
                }
                //得到该物标的符号化语句
                ao_it->priority = findItem.priority;
                ao_it->dispcategory = findItem.dispcategory;
                ao_it->overRadar = findItem.overRadar;
                if(! ao_it->symInstruction.isEmpty())
                    ao_it->symInstruction.clear();   //清空之前的符号化语句
                const QStringList instruction = findItem.symInstruction.split(";");   //符号化指令
                const int size = instruction.size();
                for(int i=0; i<size; i++) {
                    const QString context = instruction.at(i);
                    const QString style = context.left(2);
                    if(style == "CS") {   //得到条件化指令
                         QStringList symInsu = conditionalItems->dealConditionalAreaItem(context.mid(3,8), ao_it);
                        if(!symInsu.isEmpty())
                            ao_it->symInstruction.append(symInsu);
                    }else
                        ao_it->symInstruction.append(context);
                }
            }  //end for Areaobject
        }
    } //end area

    //lines
    if(line){
        QVector<LookupTable> &lookUp_Lines = configuration->Tables.find(LINESYMBOL).value();
        QVector<LookupTable>::iterator start_find;
        QVector<LookupTable>::iterator end_find = lookUp_Lines.end();

        std::map<int, std::vector<LineObject> >::iterator lom_it;
        std::map<int, std::vector<LineObject> >::iterator lom_end = chart->line_objects_map.end();
        for(lom_it = chart->line_objects_map.begin(); lom_it != lom_end; ++lom_it) {
            //开始查询表选取,检测同一类线物标，保存查找表
            QVector<LookupTable> tableLineFind;
            for(start_find = lookUp_Lines.begin(); start_find != end_find; ++start_find) {
                if(lom_it->first == start_find->objName)
                    tableLineFind.append(*start_find);
            }
            const int find_size = tableLineFind.size();   //得到查找到的表的行数

            std::vector<LineObject>::iterator lo_it;
            std::vector<LineObject>::iterator lo_end = lom_it->second.end();
            for(lo_it = lom_it->second.begin(); lo_it != lo_end; ++lo_it) {
                //检查显示时间是不是在规定时间内,面物标没有该属性
                std::vector<attf_t>::iterator attf_it;
                std::vector<attf_t>::iterator attf_end = lo_it->attfs.end();
                QString datasta, dataend, persta, perend;
                for(attf_it = lo_it->attfs.begin(); attf_it != attf_end; ++attf_it){
                    switch(attf_it->attl) {
                        case 86:     datasta = QString::fromStdString(attf_it->atvl);  break;    //DATSTA属性
                        case 85:     dataend = QString::fromStdString(attf_it->atvl);  break;    //DATEND
                        case 119:    persta = QString::fromStdString(attf_it->atvl);   break;   //PERSTA
                        case 118:    perend = QString::fromStdString(attf_it->atvl);   break;    //PEREND
                    }
                }
                if(!perend.isEmpty()) {
                    if(!comparePertime(persta, perend))
                        continue;
                }
                if(!dataend.isEmpty()) {
                    QDate end = QDate(dataend.mid(0,4).toInt(), dataend.mid(4,2).toInt(), dataend.mid(6,2).toInt());
                    QDate start = QDate(datasta.mid(0,4).toInt(), datasta.mid(4,2).toInt(), datasta.mid(6,2).toInt());
                    if(!((nowTime < QDateTime(end)) && (nowTime > QDateTime(start))))
                        continue;
                }

                //查询Look-up表得到符号化内容
                LookupTable findItem;
                switch(find_size) {
                    case 0:    //没有找到，使用look-up表中的第一行进行显示
                    {
                        findItem = lookUp_Lines.at(0);
                        qDebug()<< "not find the area table : object and recordID:" << lom_it->first << lo_it->record_id;
                    }
                    break;
                    case 1:   //只有一行，则不管属性如何，都使用它表示
                    {
                        findItem = tableLineFind.at(0);
                    }
                    break;
                    default:  //大于一条表找到,从查找表中取属性及值在物标属性中查找，如果物标属性中包含所有查找表属性则选取，如果没有则取第一行查找表，其属性区域为空
                    {
                        bool flagFindSuccess = false;
                        for(int i=1; i < find_size; i++) {   //从第二行开始查找，没有找到就选第一行
                            const LookupTable table = tableLineFind.at(i);
                            const int sizeAttr = table.attrValue.size();
                            quint8 findFlag = 0;
                            for(int j=0; j < sizeAttr; j++) {
                                const Attr att = table.attrValue.at(j);
                                //对lookup表中的每个属性在物标的属性列表中查找
                                std::vector<attf_t>::iterator attf_it;
                                std::vector<attf_t>::iterator attf_end = lo_it->attfs.end();
                                for(attf_it = lo_it->attfs.begin(); attf_it != attf_end; ++ attf_it){
                                    if((attf_it->attl == att.attl)) {
                                        if(att.attl == 117){    //ORIENT后面是没有跟数值的，只是在后面需要显示数值，但不用匹配
                                            findFlag++;
                                        }else{
                                            const QString at = QString(attf_it->atvl.c_str());   //物标属性值
                                            if(at.contains(",") && at.startsWith(att.atvl))
                                                findFlag++;
                                            else if(at == att.atvl)
                                                findFlag++;
                                            else if((att.atvl == "?") && (at == "unknow"))   //DRVAL1? DRVAL2?  这两个有问号，只有属性值为unknow时才能匹配上
                                                findFlag++;
                                        }
                                        break;     //进行下一个属性匹配
                                    }
                                }
                            }
                            //第一行找到的表就是所需的
                            if(findFlag == sizeAttr) {
                                findItem = table;
                                flagFindSuccess = true;
                                break;
                            }
                        }
                        //没有找到合适的表，选择第一行
                        if(!flagFindSuccess)
                            findItem = tableLineFind.at(0);
                    }
                    break;
                }
                //得到该物标的符号化语句
                lo_it->priority = findItem.priority;
                lo_it->dispcategory = findItem.dispcategory;
                lo_it->overRadar = findItem.overRadar;
                if(! lo_it->symInstruction.isEmpty())
                    lo_it->symInstruction.clear();   //清空之前的符号化语句
                const QStringList instruction = findItem.symInstruction.split(";");   //符号化指令
                const int size = instruction.size();
                for(int i=0; i<size; i++) {
                    const QString context = instruction.at(i);
                    const QString style = context.left(2);
                    if(style == "CS") {   //得到条件化指令
                        QStringList symInsu = conditionalItems->dealConditionalLineItem(context.mid(3,8), lo_it);
                        if(!symInsu.isEmpty())
                            lo_it->symInstruction.append(symInsu);
                    }else
                        lo_it->symInstruction.append(context);
                }

            }  // end for LineObject

        }

    }  //end draw lines
    //points
    if(point){
        //获取对应的Lookup表
        int pointType = MarinerSelect.simplePoint ? SIMPLEPOINT : PAPERPOINT;
        QVector<LookupTable> &lookUp_Point = configuration->Tables.find(pointType).value();
        QVector<LookupTable>::iterator start_find;
        QVector<LookupTable>::iterator end_find = lookUp_Point.end();

        std::map<int, std::vector<PointObject> >::iterator pom_it;
        std::map<int, std::vector<PointObject> >::iterator pom_end = chart->point_objects_map.end();
        for(pom_it = chart->point_objects_map.begin(); pom_it != pom_end; ++pom_it) {
            //开始查询表选取,检测同一类线物标，保存查找表
            QVector<LookupTable> tablePointFind;
            for(start_find = lookUp_Point.begin(); start_find != end_find; ++start_find) {
                if(pom_it->first == start_find->objName)
                    tablePointFind.append(*start_find);
            }
            const int find_size = tablePointFind.size();   //得到查找到的表的行数

            std::vector<PointObject>::iterator po_it;
            std::vector<PointObject>::iterator po_end = pom_it->second.end();
            for (po_it = pom_it->second.begin(); po_it != po_end; ++po_it) {
                //检查显示时间是不是在规定时间内,面物标没有该属性
                std::vector<attf_t>::iterator attf_it;
                std::vector<attf_t>::iterator attf_end = po_it->attfs.end();
                QString datasta, dataend, persta, perend;
                for(attf_it = po_it->attfs.begin(); attf_it != attf_end; ++attf_it){
                    switch(attf_it->attl) {
                        case 86:     datasta = QString::fromStdString(attf_it->atvl);  break;    //DATSTA属性
                        case 85:     dataend = QString::fromStdString(attf_it->atvl);  break;    //DATEND
                        case 119:    persta = QString::fromStdString(attf_it->atvl);   break;   //PERSTA
                        case 118:    perend = QString::fromStdString(attf_it->atvl);   break;    //PEREND
                    }
                }
                if(!perend.isEmpty()) {
                    if(!comparePertime(persta, perend))
                        continue;
                }
                if(!dataend.isEmpty()) {
                    QDate end = QDate(dataend.mid(0,4).toInt(), dataend.mid(4,2).toInt(), dataend.mid(6,2).toInt());
                    QDate start = QDate(datasta.mid(0,4).toInt(), datasta.mid(4,2).toInt(), datasta.mid(6,2).toInt());
                    if(!((nowTime < QDateTime(end)) && (nowTime > QDateTime(start))))
                        continue;
                }

                //查询Look-up表得到符号化内容
                LookupTable findItem;
                switch(find_size) {
                    case 0:    //没有找到，使用look-up表中的第一行进行显示
                    {
                        findItem = lookUp_Point.at(0);
                        qDebug()<< "not find the area table : object and recordID:" << pom_it->first << po_it->record_id;
                    }
                    break;
                    case 1:   //只有一行，则不管属性如何，都使用它表示
                    {
                        findItem = tablePointFind.at(0);
                    }
                    break;
                    default:  //大于一条表找到,从查找表中取属性及值在物标属性中查找，如果物标属性中包含所有查找表属性则选取，如果没有则取第一行查找表，其属性区域为空
                    {
                        bool flagFindSuccess = false;
                        for(int i=1; i < find_size; i++) {   //从第二行开始查找，没有找到就选第一行
                            const LookupTable table = tablePointFind.at(i);
                            const int sizeAttr = table.attrValue.size();
                            quint8 findFlag = 0;
                            for(int j=0; j < sizeAttr; j++) {
                                const Attr att = table.attrValue.at(j);
                                //对lookup表中的每个属性在物标的属性列表中查找
                                std::vector<attf_t>::iterator attf_it;
                                std::vector<attf_t>::iterator attf_end = po_it->attfs.end();
                                for(attf_it = po_it->attfs.begin(); attf_it != attf_end; ++ attf_it){
                                    if((attf_it->attl == att.attl)) {
                                        if(att.attl == 117){    //ORIENT后面是没有跟数值的，只是在后面需要显示数值，但不用匹配
                                            findFlag++;
                                        }else{
                                            const QString at = QString(attf_it->atvl.c_str());   //物标属性值
                                            if(at.contains(",") && at.startsWith(att.atvl))
                                                findFlag++;
                                            else if(at == att.atvl)
                                                findFlag++;
                                            else if((att.atvl == "?") && (at == "unknow"))   //DRVAL1? DRVAL2?  这两个有问号，只有属性值为unknow时才能匹配上
                                                findFlag++;
                                        }
                                        break;     //进行下一个属性匹配
                                    }
                                }
                            }
                            //第一行找到的表就是所需的
                            if(findFlag == sizeAttr) {
                                findItem = table;
                                flagFindSuccess = true;
                                break;
                            }
                        }
                        //没有找到合适的表，选择第一行
                        if(!flagFindSuccess)
                            findItem = tablePointFind.at(0);
                    }
                    break;
                }
                //得到该物标的符号化语句
                po_it->priority = findItem.priority;
                po_it->dispcategory = findItem.dispcategory;
                po_it->overRadar = findItem.overRadar;
                if(! po_it->symInstruction.isEmpty())
                    po_it->symInstruction.clear();   //清空之前的符号化语句
                const QStringList instruction = findItem.symInstruction.split(";");   //符号化指令
                const int size = instruction.size();
                for(int i=0; i<size; i++) {
                    const QString context = instruction.at(i);
                    const QString style = context.left(2);
                    if(style == "CS") {   //得到条件化指令
                        QStringList symInsu = conditionalItems->dealConditionalPointItem(context.mid(3,8), po_it);
                        if(!symInsu.isEmpty())
                            po_it->symInstruction.append(symInsu);
                    }else
                        po_it->symInstruction.append(context);
                }
            }  // end for PointObject
        }
    } // end point

}


//解析符号化语句得到显示对象
void MyScene::parseLookupTable(const std::vector<PointObject>::iterator &object, const std::vector<sg2d_t> &sg2ds, const QMap<QString, Rgb> &colorTable)
{
    //一个点物标开始
    if(object->index == -1) {
      //  qDebug()<< "error index of points";    //不知道为什么它的索引出界了
        return;
    }
    const QPointF pos = QPointF(sg2ds[object->index].long_lat[0].minutes, sg2ds[object->index].long_lat[1].minutes);
    const QStringList instruction = object->symInstruction;   //符号化指令
    const int size = instruction.size();
    for(int i=0; i < size; i++) {
        const QString context = instruction.at(i);
        const QString style = context.left(2);
        if(style == "SY"){
            //////////////////////绘制简单符号点//////////假设条件物标得到的都时SY的物标////////////////////
            MySvgItem *myitem = symbols->bright_symbols.value(context.mid(3,8));
            const QPixmap map = myitem->svgItem;
            QGraphicsPixmapItem *point = new QGraphicsPixmapItem(map);
            point->setOffset(-myitem->offset.x(), -myitem->offset.y());
            point->setPos(pos);
            if(context.mid(12, 6) == "ORIENT"){  //SY旋转标志
                std::vector<attf_t>::iterator attf_it;
                std::vector<attf_t>::iterator attf_end = object->attfs.end();
                for(attf_it = object->attfs.begin(); attf_it != attf_end; ++ attf_it){
                    if(attf_it->attl == 117) {   //旋转方向
                        const float orientValue = atof(attf_it->atvl.c_str());
                        point->rotate(orientValue);
                        break;
                    }
                }
            }
            if(context.split(",").size() == 3) {
                const float orientValue = context.split(",").at(1).toFloat();
                point->rotate(orientValue);
            }
            point->setZValue(object->priority);
#ifdef RADARALLITEM
            if(object->overRadar)
                chartAllItemsOverRadar->addToGroup(point);
            else
                chartAllItemsUnderRadar->addToGroup(point);
#else
            this->addItem(point);
#endif
        }else if(style == "TX"){
            //字符数字显示,当字符中所需显示的属性在物标中没有的话，该显示命令应该舍弃，其他的继续

        }else if(style == "LS") {
            float len = context.split(",").at(3).toFloat();
            float orient = context.split(",").at(4).toFloat();
            QGraphicsLineItem *Line = new QGraphicsLineItem;
            Line->setLine(0, 0, 0, len);
            QPen pen;
            const Rgb pColor = colorTable.find(context.mid(10,5)).value();
            const QString type = context.mid(3,4);
            if(type == "SOLD")
                pen.setStyle(Qt::SolidLine);
            else if(type == "DASH")
                pen.setStyle(Qt::DashLine);
            else if(type == "DOTT")
                pen.setStyle(Qt::DotLine);
            pen.setWidthF(context.mid(8,1).toInt() * 0.6);
            pen.setColor(QColor(pColor.r, pColor.g, pColor.b));
            Line->setPen(pen);
            Line->rotate(orient);  //旋转角度不知道对不对
            Line->setPos(pos);
            Line->setZValue(object->priority);
#ifdef RADARALLITEM
            if(object->overRadar)
                chartAllItemsOverRadar->addToGroup(Line);
            else
                chartAllItemsUnderRadar->addToGroup(Line);
#else
            this->addItem(Line);
#endif
        }else if(style == "AR") {  //画圆弧
            float radius = context.split(",").at(3).toFloat();
            float sectr1 = context.split(",").at(4).toFloat();
            float sectr2 = context.split(",").at(5).toFloat();
            QPainterPath arc;
            arc.moveTo(radius, 0);
            arc.arcTo(0, 0, radius+radius, radius+radius, 90 ,-(sectr2 - sectr1));
            QGraphicsPathItem *Arc = new QGraphicsPathItem(arc);
            QPen pen;
            const Rgb pColor = colorTable.find(context.mid(10,5)).value();
            const QString type = context.mid(3,4);
            if(type == "SOLD")
                pen.setStyle(Qt::SolidLine);
            else if(type == "DASH")
                pen.setStyle(Qt::DashLine);
            else if(type == "DOTT")
                pen.setStyle(Qt::DotLine);
            pen.setWidthF(context.mid(8,1).toInt()>>1);
            pen.setColor(QColor(pColor.r, pColor.g, pColor.b));
            Arc->setPen(pen);
            //绕中心旋转
            Arc->setTransform(QTransform().translate(radius, radius).rotate(sectr1 - 180).translate(-radius, -radius));
            Arc->setPos(pos-QPointF(radius,radius));
            Arc->setZValue(object->priority);
#ifdef RADARALLITEM
            if(object->overRadar)
                chartAllItemsOverRadar->addToGroup(Arc);
            else
                chartAllItemsUnderRadar->addToGroup(Arc);
#else
            this->addItem(Arc);
#endif
        }
    }

}
void MyScene::parseLookupTable(const std::vector<LineObject>::iterator &object, const std::vector<sg2d_t> &sg2ds, const QMap<QString, Rgb> &colorTable)
{
    //一个线物标开始
    std::vector<int>::iterator sg2d_index_it;
    std::vector<int>::iterator sg2d_index_end;
    const QStringList instruction = object->symInstruction;   //符号化指令
    const int size = instruction.size();
    for(int i=0; i < size; i++) {
        const QString context = instruction.at(i);
        const QString style = context.left(2);
        if(style == "LS") {
            /////////////////绘制简单线型///////////////////////////
            QPainterPath path;
            if(context.size() > 16) {
                int ref_record = context.split(",").at(3).toInt();   //LS(SOLD,2,DEPSC,index_edge,)
                std::map<int, EdgeVector>::iterator evm_it;
                evm_it = nowChart->edge_vectors_map.find(ref_record);
                std::vector<int>::iterator start;
                std::vector<int>::iterator end= evm_it->second.sg2d_indices.end();
                start = evm_it->second.sg2d_indices.begin();
                path.moveTo(QPointF(sg2ds[*start].long_lat[0].minutes, sg2ds[*start].long_lat[1].minutes));
                for (start = evm_it->second.sg2d_indices.begin()+1; start != end; ++start) {
                    path.lineTo(QPointF(sg2ds[*start].long_lat[0].minutes, sg2ds[*start].long_lat[1].minutes));
                }
            }else{  //LS(SOLD,2,DEPSC)
                sg2d_index_end = object->indices.end();
                sg2d_index_it = object->indices.begin();
                path.moveTo(QPointF(sg2ds[*sg2d_index_it].long_lat[0].minutes, sg2ds[*sg2d_index_it].long_lat[1].minutes));
                for (sg2d_index_it = object->indices.begin()+1; sg2d_index_it != sg2d_index_end; ++sg2d_index_it) {
                    path.lineTo(QPointF(sg2ds[*sg2d_index_it].long_lat[0].minutes, sg2ds[*sg2d_index_it].long_lat[1].minutes));
                }
            }

            QGraphicsPathItem *Line = new QGraphicsPathItem(path);
            QPen pen;
            const Rgb pColor = colorTable.find(context.mid(10,5)).value();
            const QString type = context.mid(3,4);
            if(type == "SOLD")
                pen.setStyle(Qt::SolidLine);
            else if(type == "DASH")
                pen.setStyle(Qt::DashLine);
            else if(type == "DOTT")
                pen.setStyle(Qt::DotLine);
            pen.setWidthF(context.mid(8,1).toInt() * 0.6);  // * 0.0378
            pen.setColor(QColor(pColor.r, pColor.g, pColor.b));
            Line->setPen(pen);
            Line->setZValue(object->priority);
#ifdef RADARALLITEM
            if(object->overRadar)
                chartAllItemsOverRadar->addToGroup(Line);
            else
                chartAllItemsUnderRadar->addToGroup(Line);
#else
            this->addItem(Line);
#endif
        }else if(style == "LC") {
            //////////////////绘制复杂线型///////////////////////////
            const QString lineName = context.mid(3,8);
            MyLineSvgItem *myitem = symbols->bright_complexlines.value(lineName);
            if(context.size() > 12) {
                int ref_record = context.split(",").at(1).toInt();   //LC(LOWACC21,(index_edge),)
                std::map<int, EdgeVector>::iterator evm_it;
                evm_it = nowChart->edge_vectors_map.find(ref_record);
                sg2d_index_end= evm_it->second.sg2d_indices.end() - 1;
                sg2d_index_it = evm_it->second.sg2d_indices.begin();
            }else{
                sg2d_index_end = object->indices.end() - 1;       //两个坐标点连成一条线
                sg2d_index_it = object->indices.begin();
            }

            for(; sg2d_index_it != sg2d_index_end; ++sg2d_index_it) {
                const QPointF start = QPointF(sg2ds[*sg2d_index_it].long_lat[0].minutes, sg2ds[*sg2d_index_it].long_lat[1].minutes);
                const QPointF end = QPointF(sg2ds[*(sg2d_index_it+1)].long_lat[0].minutes, sg2ds[*(sg2d_index_it+1)].long_lat[1].minutes);
                const int mapWidth = myitem->svgItem.width();
                const QLineF line = QLineF(start ,end);
                const int lineLength = line.length();
                if(lineLength == 0)
                    continue;
                const int num = lineLength / mapWidth;           //绘制的图形个数
                const int remain = lineLength - mapWidth * num;
                if(remain > (mapWidth>>1)) {              //需要绘制复杂图形
                    const double angle = (360-line.angle());   //0度表示三点方向,绘制时需要加上90度
                    QPixmap map = QPixmap(lineLength, myitem->svgItem.height());   //先绘制在图像上，再添加
                    map.fill(Qt::transparent);
                    QPainter p;
                    p.begin(&map);
                    for(int i=0; i< num+1; i++) {
                        p.drawPixmap(QPoint(i * mapWidth, 0), myitem->svgItem);
                    }
                    p.end();
                    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(map);
                    item->rotate(angle);
                    item->setOffset(0, -myitem->offset.y());
                    item->setPos(start);
                    item->setZValue(object->priority);
#ifdef RADARALLITEM
                    if(object->overRadar)
                        chartAllItemsOverRadar->addToGroup(item);
                    else
                        chartAllItemsUnderRadar->addToGroup(item);
#else
            this->addItem(item);
#endif
                }else {   //剩下的线比较短
                    const double angle = (360-line.angle());   //0度表示三点方向,绘制时需要加上90度
                    QPixmap map = QPixmap(lineLength, myitem->svgItem.height());   //先绘制在图像上，再添加
                    map.fill(Qt::transparent);
                    QPainter p;
                    p.begin(&map);
                    for(int i=0; i<num; i++) {
                       p.drawPixmap(QPoint(i * mapWidth, 0), myitem->svgItem);
                    }
                    const Rgb color = myitem->mapColor;
                    QPen pen = QPen(QColor(color.r, color.g, color.b));
                    pen.setWidthF(1.0 * 0.6);
                    p.setPen(pen);
                    p.drawLine(QPoint(num * mapWidth, myitem->offset.y()), QPoint((remain + num*mapWidth), myitem->offset.y()));  //剩下的一部分划线就行
                    p.end();
                    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(map);
                    item->rotate(angle);
                    item->setOffset(0, -myitem->offset.y());
                    item->setPos(start);
                    item->setZValue(object->priority);
#ifdef RADARALLITEM
                    if(object->overRadar)
                        chartAllItemsOverRadar->addToGroup(item);
                    else
                        chartAllItemsUnderRadar->addToGroup(item);
#else
            this->addItem(item);
#endif
                }
            }  //end for

        }else if(style == "SY"){
            //////////////////////在线段中间绘制简单符号点//////////////////////////////
          /*  MySvgItem *myitem = symbols->bright_symbols.value(context.mid(3,8));
            const QPixmap map = myitem->svgItem;
            QGraphicsPixmapItem *point = new QGraphicsPixmapItem(map);
            point->setOffset(-myitem->offset.x(), -myitem->offset.y());
            point->setPos(object->centerPoint);   //需要计算线段中间的坐标点
            if(context.mid(12, 6) == "ORIENT"){  //SY旋转标志
                std::vector<attf_t>::iterator attf_it;
                std::vector<attf_t>::iterator attf_end = object->attfs.end();
                for(attf_it = object->attfs.begin(); attf_it != attf_end; ++ attf_it){
                    if(attf_it->attl == 117) {   //旋转方向
                        const float orientValue = atof(attf_it->atvl.c_str());
                        point->rotate(orientValue);
                        break;
                    }
                }
            }
            point->setZValue(object->priority);
            this->addItem(point);  */
        }else if(style == "TX"){
            //字符数字显示,当字符中所需显示的属性在物标中没有的话，该显示命令应该舍弃，其他的继续
        }
    }  //end for

}
void MyScene::parseLookupTable(const std::vector<AreaObject>::iterator &object, const std::vector<sg2d_t> &sg2ds, const QMap<QString, Rgb> &colorTable)
{
    //一个面物标开始
    QPainterPath path;
    std::vector<std::vector<int > >::iterator cit;
    std::vector<std::vector<int > >::iterator cend = object->contours.end();
    for (cit = object->contours.begin(); cit != cend; ++cit) {
        std::vector<int>::iterator vit;
        std::vector<int>::iterator vend = cit->end();
        //绘制区域路径
        vit = cit->begin();
        path.moveTo(QPointF(sg2ds[*vit].long_lat[0].minutes, sg2ds[*vit].long_lat[1].minutes));
        for (vit = cit->begin()+1; vit != vend; ++vit) {
            path.lineTo(QPointF(sg2ds[*vit].long_lat[0].minutes, sg2ds[*vit].long_lat[1].minutes));
        }
    }

    //边解析语句边输出图形符号
    const QStringList instruction = object->symInstruction;   //符号化指令
    const int size = instruction.size();
    for(int i=0; i < size; i++) {
        const QString context = instruction.at(i);
        const QString style = context.left(2);
        if(style == "AC") {
            ///////////////////////纯色填充区域////////////////
            QGraphicsPathItem *Area = new QGraphicsPathItem(path);
            QBrush fill_brush(Qt::SolidPattern);
            const Rgb fill = colorTable.find(context.mid(3, 5)).value();
            switch(context.mid(9, 1).toInt()){
                case 0:   fill_brush.setColor(QColor(fill.r, fill.g, fill.b));     break;
                case 1:   fill_brush.setColor(QColor(fill.r, fill.g, fill.b, 192));   break;  //75%
                case 2:   fill_brush.setColor(QColor(fill.r, fill.g, fill.b, 128));   break;   //50%
                case 3:   fill_brush.setColor(QColor(fill.r, fill.g, fill.b, 64));   break;  //25%
            }
            Area->setBrush(fill_brush);
            Area->setPen(QPen(Qt::NoPen));
            Area->setZValue(object->priority);
#ifdef RADARALLITEM
            if(object->overRadar)
                chartAllItemsOverRadar->addToGroup(Area);
            else
                chartAllItemsUnderRadar->addToGroup(Area);
#else
            this->addItem(Area);
#endif
        }else if(style == "AP") {
            //////////////////模板填充区域///////////////////
            MyPatternSvgItem *myitem = symbols->bright_patterns.value(context.mid(3, 8));  //得到模板图形
            const QPixmap map = myitem->svgItem;
            QGraphicsPathItem *Area = new QGraphicsPathItem(path);
            QBrush fill_brush;
            QPixmap fillMap;
            //此处由于画笔模板的间距不能改变，所以只能重新定义图片，将模板插入其中，从而达到间距变化的效果
            //当根据比例尺变化的时候，直接大于原始比例尺则用大距离，小于原始比例尺为小距离
            if(myitem->stgLin) {   //为stg排列
                int distance = 0;
                if(myitem->conScl) {
                    distance = myitem->miniDist;
                }else{
                    distance = (MarinerSelect.chartScaleNum < 10000) ? myitem->miniDist : myitem->maxDist;
                }
                const int sizeMapWidth = distance + map.width();
                const int sizeMapHeith = distance + map.height();
                fillMap = QPixmap(sizeMapWidth + sizeMapWidth, sizeMapHeith + sizeMapHeith);
                fillMap.fill(Qt::transparent);
                QPainter p;
                p.begin(&fillMap);
                p.drawPixmap(0, 0, map);
                p.drawPixmap(sizeMapWidth, sizeMapHeith, map);
                p.end();
            }else{  //为线型排列
                int distance = 0;
                if(myitem->conScl) {  //间距恒定
                    distance = myitem->miniDist;
                }else{  //间距根据比例尺大小变化
                    distance = (MarinerSelect.chartScaleNum < 10000) ? myitem->miniDist : myitem->maxDist;
                }
                fillMap = QPixmap(distance + map.width(), distance + map.height());
                fillMap.fill(Qt::transparent);
                QPainter p;
                p.begin(&fillMap);
                p.drawPixmap(0, 0, map);
                p.end();
            }
            fill_brush.setTexture(fillMap);
            Area->setBrush(fill_brush);
            Area->setPen(QPen(Qt::NoPen));
            Area->setZValue(object->priority);
#ifdef RADARALLITEM
            if(object->overRadar)
                chartAllItemsOverRadar->addToGroup(Area);
            else
                chartAllItemsUnderRadar->addToGroup(Area);
#else
            this->addItem(Area);
#endif
        }else if(style == "LS") {
            /////////////////绘制简单线型///////////////////////////
            QGraphicsPathItem *Area;
            if(context.size() > 16) {
                QPainterPath Apath;
                int ref_record = context.split(",").at(3).toInt();   //LS(SOLD,2,DEPSC,index_edge,)
                std::map<int, EdgeVector>::iterator evm_it;
                evm_it = nowChart->edge_vectors_map.find(ref_record);
                std::vector<int>::iterator start;
                std::vector<int>::iterator end= evm_it->second.sg2d_indices.end();
                start = evm_it->second.sg2d_indices.begin();
                Apath.moveTo(QPointF(sg2ds[*start].long_lat[0].minutes, sg2ds[*start].long_lat[1].minutes));
                for (start = evm_it->second.sg2d_indices.begin()+1; start != end; ++start) {
                    Apath.lineTo(QPointF(sg2ds[*start].long_lat[0].minutes, sg2ds[*start].long_lat[1].minutes));
                }
                Area = new QGraphicsPathItem(Apath);
            }else
                Area = new QGraphicsPathItem(path);

            QPen pen;
            const Rgb pColor = colorTable.find(context.mid(10,5)).value();
            const QString type = context.mid(3,4);
            if(type == "SOLD")
                pen.setStyle(Qt::SolidLine);
            else if(type == "DASH")
                pen.setStyle(Qt::DashLine);
            else if(type == "DOTT")
                pen.setStyle(Qt::DotLine);
            pen.setWidthF(context.mid(8,1).toInt() * 0.6);  // * 0.0378
            pen.setColor(QColor(pColor.r, pColor.g, pColor.b));
            Area->setPen(pen);
            Area->setZValue(object->priority);
#ifdef RADARALLITEM
            if(object->overRadar)
                chartAllItemsOverRadar->addToGroup(Area);
            else
                chartAllItemsUnderRadar->addToGroup(Area);
#else
            this->addItem(Area);
#endif
        }else if(style == "LC") {
            //////////////////绘制复杂线型///////////////////////////
            const QString lineName = context.mid(3,8);
            MyLineSvgItem *myitem = symbols->bright_complexlines.value(lineName);
            if(context.size() > 12) {
                int ref_record = context.split(",").at(1).toInt();   //LC(LOWACC41,index_edge,)
                std::map<int, EdgeVector>::iterator evm_it;
                evm_it = nowChart->edge_vectors_map.find(ref_record);
                std::vector<int>::iterator sg2d_index_it = evm_it->second.sg2d_indices.begin();
                std::vector<int>::iterator sg2d_index_end = evm_it->second.sg2d_indices.end() - 1;
                for(; sg2d_index_it != sg2d_index_end; ++sg2d_index_it) {
                    const QPointF start = QPointF(sg2ds[*sg2d_index_it].long_lat[0].minutes, sg2ds[*sg2d_index_it].long_lat[1].minutes);
                    const QPointF end = QPointF(sg2ds[*(sg2d_index_it+1)].long_lat[0].minutes, sg2ds[*(sg2d_index_it+1)].long_lat[1].minutes);
                    const int mapWidth = myitem->svgItem.width();
                    const QLineF line = QLineF(start ,end);
                    const int lineLength = line.length();
                    if(lineLength == 0)
                        continue;
                    const int num = lineLength / mapWidth;           //绘制的图形个数
                    const int remain = lineLength - mapWidth * num;
                    if(remain > (mapWidth>>1)) {              //需要绘制复杂图形
                        const double angle = (360-line.angle());   //0度表示三点方向,绘制时需要加上90度
                        QPixmap map = QPixmap(lineLength, myitem->svgItem.height());   //先绘制在图像上，再添加
                        map.fill(Qt::transparent);
                        QPainter p;
                        p.begin(&map);
                        for(int i=0; i< num+1; i++) {
                            p.drawPixmap(QPoint(i * mapWidth, 0), myitem->svgItem);
                        }
                        p.end();
                        QGraphicsPixmapItem *item = new QGraphicsPixmapItem(map);
                        item->rotate(angle);
                        item->setOffset(0, -myitem->offset.y());
                        item->setPos(start);
                        item->setZValue(object->priority);
#ifdef RADARALLITEM
                        if(object->overRadar)
                            chartAllItemsOverRadar->addToGroup(item);
                        else
                             chartAllItemsUnderRadar->addToGroup(item);
#else
            this->addItem(item);
#endif
                    }else {   //剩下的线比较短
                        const double angle = (360-line.angle());   //0度表示三点方向,绘制时需要加上90度
                        QPixmap map = QPixmap(lineLength, myitem->svgItem.height());   //先绘制在图像上，再添加
                        map.fill(Qt::transparent);
                        QPainter p;
                        p.begin(&map);
                        for(int i=0; i<num; i++) {
                           p.drawPixmap(QPoint(i * mapWidth, 0), myitem->svgItem);
                        }
                        const Rgb color = myitem->mapColor;
                        QPen pen = QPen(QColor(color.r, color.g, color.b));
                        pen.setWidthF(1.0 * 0.6);
                        p.setPen(pen);
                        p.drawLine(QPoint(num * mapWidth, myitem->offset.y()), QPoint((remain + num*mapWidth), myitem->offset.y()));  //剩下的一部分划线就行
                        p.end();
                        QGraphicsPixmapItem *item = new QGraphicsPixmapItem(map);
                        item->rotate(angle);
                        item->setOffset(0, -myitem->offset.y());
                        item->setPos(start);
                        item->setZValue(object->priority);
#ifdef RADARALLITEM
                        if(object->overRadar)
                            chartAllItemsOverRadar->addToGroup(item);
                        else
                            chartAllItemsUnderRadar->addToGroup(item);
#else
            this->addItem(item);
#endif
                    }
                }  //end for

            }else{

            std::vector<std::vector<int > >::iterator cits;
            std::vector<std::vector<int > >::iterator cends = object->contours.end();
            for (cits = object->contours.begin(); cits != cends; ++cits) {
                std::vector<int>::iterator vits;
                std::vector<int>::iterator vends = cits->end() - 1;
                //绘制区域边缘路径
                for (vits = cits->begin(); vits != vends; ++vits) {
                    const QPointF start = QPointF(sg2ds[*vits].long_lat[0].minutes, sg2ds[*vits].long_lat[1].minutes);
                    const QPointF end = QPointF(sg2ds[*(vits+1)].long_lat[0].minutes, sg2ds[*(vits+1)].long_lat[1].minutes);

                    const int mapWidth = myitem->svgItem.width();
                    const QLineF line = QLineF(start ,end);
                    const int lineLength = line.length();
                    if(lineLength == 0)
                        continue;
                    const int num = lineLength / mapWidth;           //绘制的图形个数
                    const int remain = lineLength - mapWidth * num;
                    if(remain > (mapWidth>>1)) {              //需要绘制复杂图形
                        const double angle = (360-line.angle());   //0度表示三点方向,绘制时需要加上90度
                        QPixmap map = QPixmap(lineLength, myitem->svgItem.height());   //先绘制在图像上，再添加
                        map.fill(Qt::transparent);
                        QPainter p;
                        p.begin(&map);
                        for(int i=0; i< num+1; i++) {
                            p.drawPixmap(QPoint(i * mapWidth, 0), myitem->svgItem);
                        }
                        p.end();
                        QGraphicsPixmapItem *item = new QGraphicsPixmapItem(map);
                        item->rotate(angle);
                        item->setOffset(0, -myitem->offset.y());
                        item->setPos(start);
                        item->setZValue(object->priority);
#ifdef RADARALLITEM
                        if(object->overRadar)
                            chartAllItemsOverRadar->addToGroup(item);
                        else
                            chartAllItemsUnderRadar->addToGroup(item);
#else
            this->addItem(item);
#endif
                    }else {   //剩下的线比较短
                        const double angle = (360-line.angle());   //0度表示三点方向,绘制时需要加上90度
                        QPixmap map = QPixmap(lineLength, myitem->svgItem.height());   //先绘制在图像上，再添加
                        map.fill(Qt::transparent);
                        QPainter p;
                        p.begin(&map);
                        for(int i=0; i<num; i++) {
                           p.drawPixmap(QPoint(i * mapWidth, 0), myitem->svgItem);
                        }
                        const Rgb color = myitem->mapColor;
                        QPen pen = QPen(QColor(color.r, color.g, color.b));
                        pen.setWidthF(1.0 * 0.6);
                        p.setPen(pen);
                        p.drawLine(QPoint(num * mapWidth, myitem->offset.y()), QPoint((remain + num*mapWidth), myitem->offset.y()));  //剩下的一部分划线就行
                        p.end();
                        QGraphicsPixmapItem *item = new QGraphicsPixmapItem(map);
                        item->rotate(angle);
                        item->setOffset(0, -myitem->offset.y());
                        item->setPos(start);
                        item->setZValue(object->priority);
#ifdef RADARALLITEM
                        if(object->overRadar)
                            chartAllItemsOverRadar->addToGroup(item);
                        else
                           chartAllItemsUnderRadar->addToGroup(item);
#else
            this->addItem(item);
#endif
                    }
                }  //end for
            }
          }
        }else if(style == "LZ") {  //LZ(SOLD,2,DEPSC,OVERRADAR,Priority,index_edge,),没有比例尺限制
            int ref_record = context.split(",").at(5).toInt();
            int priority = context.split(",").at(4).toInt();
            std::map<int, EdgeVector>::iterator evm_it;
            evm_it = nowChart->edge_vectors_map.find(ref_record);
            std::vector<int>::iterator start;
            std::vector<int>::iterator end= evm_it->second.sg2d_indices.end();
            QPainterPath linepath;
            start = evm_it->second.sg2d_indices.begin();
            linepath.moveTo(QPointF(sg2ds[*start].long_lat[0].minutes, sg2ds[*start].long_lat[1].minutes));
            for (start = evm_it->second.sg2d_indices.begin()+1; start != end; ++start) {
                linepath.lineTo(QPointF(sg2ds[*start].long_lat[0].minutes, sg2ds[*start].long_lat[1].minutes));
            }

            QGraphicsPathItem *Area = new QGraphicsPathItem(linepath);
            QPen pen;
            const Rgb pColor = colorTable.find(context.mid(10,5)).value();
            const QString type = context.mid(3,4);
            if(type == "SOLD")
                pen.setStyle(Qt::SolidLine);
            else if(type == "DASH")
                pen.setStyle(Qt::DashLine);
            else if(type == "DOTT")
                pen.setStyle(Qt::DotLine);
            pen.setWidthF(context.mid(8,1).toInt() * 0.6);
            pen.setColor(QColor(pColor.r, pColor.g, pColor.b));
            Area->setPen(pen);
            Area->setZValue(priority);
#ifdef RADARALLITEM
            if(object->overRadar)
                chartAllItemsOverRadar->addToGroup(Area);
            else
                chartAllItemsUnderRadar->addToGroup(Area);
#else
            this->addItem(Area);
#endif
        }else if(style == "SY"){
            //////////////////////绘制简单符号点//////////////////////////////
            MySvgItem *myitem = symbols->bright_symbols.value(context.mid(3,8));
            const QPixmap map = myitem->svgItem;
            QGraphicsPixmapItem *point = new QGraphicsPixmapItem(map);
            point->setOffset(-myitem->offset.x(), -myitem->offset.y());
            point->setPos(object->centerPoint);
            if(context.mid(12, 6) == "ORIENT"){  //SY旋转标志
                std::vector<attf_t>::iterator attf_it;
                std::vector<attf_t>::iterator attf_end = object->attfs.end();
                for(attf_it = object->attfs.begin(); attf_it != attf_end; ++ attf_it){
                    if(attf_it->attl == 117) {   //旋转方向
                        const float orientValue = atof(attf_it->atvl.c_str());
                        point->rotate(orientValue);
                        break;
                    }
                }
            }
            point->setZValue(object->priority);
#ifdef RADARALLITEM
            if(object->overRadar)
                chartAllItemsOverRadar->addToGroup(point);
            else
                chartAllItemsUnderRadar->addToGroup(point);
#else
            this->addItem(point);
#endif
        }else if(style == "TX"){
            //字符数字显示,当字符中所需显示的属性在物标中没有的话，该显示命令应该舍弃，其他的继续

        }
    }
}

void MyScene::refreshScreen(QString nowChart, bool area, bool line, bool point)
{

    QVector<Chart *>::iterator cit;
    QVector<Chart *>::iterator cend = openChartData->end();
    for(cit = openChartData->begin(); cit != cend; ++cit) {
        //查找对应海图数据并显示
        if((*cit)->description == nowChart.toStdString()) {
           // this->clear();  //删除之前的图形项
            if(chartAllItemsUnderRadar && chartAllItemsOverRadar){
                this->removeItem(chartAllItemsUnderRadar);
                this->removeItem(chartAllItemsOverRadar);
            }

            GenerateInstruction(*cit, area, line, point);
            Render(*cit);
            nowChartName = nowChart;
        }
    }

}

void MyScene::slot_scale(float scale)
{
    if(!nowChart)   return;
    nowChart->absolute_zoom_factor *= scale;
    MarinerSelect.chartScaleNum /= scale;

    //将海图里的坐标变化再绘制，保存放大倍数
    std::vector<sg2d_t>::iterator sg2dit;
    std::vector<sg2d_t>::iterator sg2dend = nowChart->sg2ds.end();
    for (sg2dit = nowChart->sg2ds.begin(); sg2dit != sg2dend; ++sg2dit) {
            sg2dit->long_lat[0] = sg2dit->long_lat[0].minutes * scale;
            sg2dit->long_lat[1] = sg2dit->long_lat[1].minutes * scale;
        }

    std::vector<SoundObject>::iterator so_it;
    std::vector<SoundObject>::iterator so_end = nowChart->sound_object_vector.end();
    for(so_it = nowChart->sound_object_vector.begin(); so_it != so_end; ++so_it) {
        std::vector<sg3d_t>::iterator it;
        std::vector<sg3d_t>::iterator end = so_it->sg3ds.end();
        for(it = so_it->sg3ds.begin(); it != end; ++it) {
            it->long_lat[0] = it->long_lat[0] * scale;
            it->long_lat[1] = it->long_lat[1] * scale;
        }
    }

    //重新计算海图中心
    centerLongScreen *= scale;
    centerLatScreen *= scale;

    //计算区域中心
    nowChart->calculateCenter();

    refreshScreen(QString::fromStdString(nowChart->description), false, false, false);

}

void MyScene::createRadarImage()
{

        if(MarinerSelect.radarShow) {
            if(!lpRadarItem)
                lpRadarItem = new RadarItem;

            radarAllItems = new QGraphicsItemGroup;
            radarAllItems->addToGroup(lpRadarItem);
            this->addItem(radarAllItems);
             radarAllItems->setZValue(0);
             moveRadarToCenter();
        }else{
            if(RadarSysinfo.transmite)    //关闭雷达发射
                lpInteract->BtnOpenClose();
            if(radarAllItems)
                this->removeItem(radarAllItems);
        }
}

void MyScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if( RadarSysinfo.offset &&  lpRadarItem)
        lpRadarItem->mousePressEvent(event);

    QGraphicsScene::mousePressEvent(event);
}

//@todo  设置雷达图像的位置，在这里可以对雷达图像的位置进行矫正
void MyScene::moveRadarToCenter()
{
     if(MarinerSelect.radarShow && radarAllItems) {
         //const QPointF center = lpMainWindow->screenCrenterToScene();
         radarAllItems->setPos(QPointF(centerLongScreen-400, centerLatScreen-400));

     }
}

void MyScene::calculateMatchScale()
{
    if(MarinerSelect.radarShow && MarinerSelect.scaleMatch) {
        quint8 x = RadarSysinfo.RangeScale.rngIndex();
        const quint8 RngCount = 19;
        //每个量程的实际距离,转化为米  *1.852*1000
        const float rngDistance[RngCount] = {231.5, 463, 926, 1389, 1852, 2778, 3704, 5556, 7408, 11112, 14816, 22224, 29632, 44448, 66672, 88896, 118528, 133344, 177792};
        if(x < RngCount) {
            const float nowDistance = rngDistance[x];
            //计算对应的比例尺，即400像素，约为10.583厘米，0.10583米
            const float radarScale =(int)(nowDistance / 0.10583);
            const float chartScale = MarinerSelect.chartScaleNum;   //当前海图比例尺
            //进行调整
            const float scale = chartScale / radarScale;
            slot_scale(scale);
        }
    }


}





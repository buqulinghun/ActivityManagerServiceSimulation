/********************************************************************
 *日期: 2016-01-12
 *作者: 王名孝
 *作用: 加载各种配置文件，包括lookup表/属性表/颜色表/符号表/复杂线型表/模板表及一些定义
 *修改:
 ********************************************************************/

#include <iostream>
#include <fstream>
#include <QFile>
#include <QTime>
#include <QTextStream>
#include <QDebug>
#include <QStringList>


#include "iso8211lib/iso8211.h"
#include "configuration.h"

/**
 * Constructor
 */
Configuration::Configuration()
    : catalog_loaded(false)
{
    SetupObjectNameNumMap();    //根据物标名称设置对应的编号

}

/**
 * Destructor
 */
Configuration::~Configuration()
{
}


/**
 * Load configuration using the current set dir
 */
void Configuration::Load()
{
    LoadAttributeCode();      //加载属性列表
    LoadLookUpTable();       //加载lookup表
    LoadColorTable();       //加载颜色表

    //GenerateNecessaryConfiguration();
    //LoadObjectNameFile();
    //LoadCatalogFile();
}

/**
  加载Look-up表内容并存储
  */
void Configuration::LoadLookUpTable()
{
    QTime tt;
    tt.start();

    QVector<LookupTable> paperPoints;
    QVector<LookupTable> simplePoints;
    QVector<LookupTable> Lines;
    QVector<LookupTable> plainAreas;
    QVector<LookupTable> symbolAreas;


    QFile file("S57Lib/S57LookupTable");
    if(! file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "open file S57Lib/S57LookupTable error!";
        return;
    }
    QTextStream in(&file);

    while(! in.atEnd()){
        QString line = in.readLine();
        if(line.startsWith("LUPT")){    //一条Lookup表开始
            LookupTable table;
            QString name = line.mid(19, 6);
            QString type = line.mid(25, 1);
            QString priority = line.mid(26, 5);
            QString radar = line.mid(31, 1);
            QString set = line.mid(32);
            table.objName = object_name_num_map[name.toStdString()];  //名称
            table.priority = priority.toInt();  //显示优先级
            if(radar == "O")   //雷达覆盖
                table.overRadar = true;
            else
                table.overRadar = false;

            //第二行，属性组合
            line.clear();
            line = in.readLine();
            if(line.startsWith("ATTC")){
                if(line.size() > 13) {   //包含有属性，前面九个多余字符
                    const QStringList att = line.mid(9).split("");
                    const int size = att.size() - 1;  //会多余一个分割
                    for(int i=0; i<size; i++){
                        Attr attr;
                        attr.attl = attribute_name_num_map[att.at(i).left(6)].code;
                        attr.atvl = att.at(i).mid(6);
                        table.attrValue.append(attr);
                    }
                }
            }else{
                qDebug() << "ATTC not behind the LUPT,error!";
            }
            //第三行，符号化指令
            line.clear();
            line = in.readLine();
            if(line.startsWith("INST")){
                const QStringList ins = line.mid(9).split("");   //第一个，会有第二个是空的
                table.symInstruction = ins.at(0);
            }else{
                qDebug() << "INST not behind the ATTC,error!";
            }
            //第四行，显示类别
            line.clear();
            line = in.readLine();
            if(line.startsWith("DISC")){
                QString cate = line.mid(9);
                if(cate.contains("DISPLAYBASE"))
                    table.dispcategory = DISPLAY_BASE;
                else if(cate.contains("STANDARD"))
                    table.dispcategory = STANDARD;
                else if(cate.contains("OTHER"))
                    table.dispcategory = OTHER;
                else if(cate.contains("MARINERS STANDARD"))
                    table.dispcategory = MARINERS_STANDARD;
                else if(cate.contains("MARINERS OTHER"))
                    table.dispcategory = MARINERS_OTHER;
               /* else
                    qDebug() << "DISC field of type is error!";   //不用显示  */

            }else{
                qDebug() << "DISC not behind the INST,error!";
            }
            //第五行，显示组
            line.clear();
            line = in.readLine();
            if(line.startsWith("LUCM")){
                table.viewGroup = line.mid(9, 5).toInt();
            }else{
                qDebug() << "LUCM not behind the DISC,error!";
            }

            //分类保存查找表数据           
            if((type == "P") && (set.contains("PAPER_CHART")))   //类型中的类别
                paperPoints.append(table);
            else if((type == "P") && (set.contains("SIMPLIFIED")))
                simplePoints.append(table);
            else if((type == "A") && (set.contains("PLAIN_BOUNDARIES")))
                plainAreas.append(table);
            else if((type == "A") && (set.contains("SYMBOLIZED_BOUNDARIES")))
                symbolAreas.append(table);
            else if((type == "L") && (set.contains("LINES")))
                Lines.append(table);

        }

      }
    file.close();
    //保存解析的查找表数据
    Tables.insert(PAPERPOINT, paperPoints);
    Tables.insert(SIMPLEPOINT, simplePoints);
    Tables.insert(LINESYMBOL, Lines);
    Tables.insert(PLAINAREA, plainAreas);
    Tables.insert(SYMBOLAREA, symbolAreas);

    qDebug()<< "time of parse look-up table is:"<< tt.elapsed();


}
/**
  加载属性表内容并存储
  */
void Configuration::LoadAttributeCode()
{
    QTime tt;
    tt.start();

    QFile file("S57Lib/S57Attributes");
    if(! file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "open file S57Lib/S57Attributes error!";
        return;
    }
    QTextStream in(&file);
    while(! in.atEnd()){
        //第二行为属性字符编码
        QString line = in.readLine();
        if(line.contains("Acronym:")){
            Attribute attr;
            QString name;
            QStringList list1 = line.split(" ");
             name = list1.at(1);
             //第三行为数字编码
             line.clear();
             line = in.readLine();
             if(line.contains("Code:")){
                 QStringList list2 = line.split(" ");
                 attr.code = list2.at(1).toInt();

             }
             //第四行忽略
             in.readLine();
             //第五行，为>号则没有枚举，否则有
             line.clear();
             line = in.readLine();
             if(line.contains(">")){   //该属性结束
                 attribute_name_num_map[name] = attr;
             }else if(line.contains(":")){
                 QStringList att = line.split(": ");
                 QString attrtype = att.at(1);
                 attr.attrType.append(attrtype);
                 //循环检测是否还有其他枚举字符串
                 bool attrOver = true;
                 while(attrOver) {
                     line.clear();
                     line = in.readLine();
                     if(line.contains("enum end")){    //该属性结束
                         attrOver = false;
                         attribute_name_num_map[name] = attr;
                     }else if(line.contains(":")){
                         QString type = line.split(": ").at(1);
                         attr.attrType.append(type);
                     }
                 }
             }

        }
    }
    file.close();

    qDebug()<< "time of parse attribue table is:"<< tt.elapsed();
}
/**
  加载颜色表内容并存储
  */
void Configuration::LoadColorTable()
{
    QFile file("S57Lib/S57Colours");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "open file S57Lib/S57Colours error!";
        return;
    }
    QTextStream in(&file);
    int type = 0;   //从头开始
    QMap<QString, Rgb> bright_color_map;
    QMap<QString, Rgb> whiteback_color_map;
    QMap<QString, Rgb> blackback_color_map;
    QMap<QString, Rgb> dusk_color_map;
    QMap<QString, Rgb> night_color_map;

    while(! in.atEnd()) {
        QString line = in.readLine();
        QStringList c_line = line.split(";");    //拆分之后都会多一个
        QString name;
        Rgb rgb;
        if(c_line.size() == 9){
            name = c_line.at(0);
            rgb.r = c_line.at(5).toInt();
            rgb.g = c_line.at(6).toInt();
            rgb.b = c_line.at(7).toInt();

            switch(type){
                case DAY_BRIGHT:
                bright_color_map[name] = rgb;
                break;
                case DAY_WHITEBACK:
                whiteback_color_map[name] = rgb;
                break;
                case DAY_BLACKBACK:
                blackback_color_map[name] = rgb;
                break;
                case DUSK:
                dusk_color_map[name] = rgb;
                break;
                case NIGHT:
                night_color_map[name] = rgb;
                break;
                default:
                qDebug() <<"error while loading color table, wrong type!";
                break;
            }
        }else{
            if(line.contains("DAY_BRIGHT"))
                type = DAY_BRIGHT;
            else if(line.contains("DAY_WHITEBACK"))
                type = DAY_WHITEBACK;
            else if(line.contains("DAY_BLACKBACK"))
                type = DAY_BLACKBACK;
            else if(line.contains("DUSK"))
                type = DUSK;
            else if(line.contains("NIGHT"))
                type = NIGHT;
        }

    }

    file.close();

    color_map[DAY_BRIGHT] = bright_color_map;
    color_map[DAY_WHITEBACK] = whiteback_color_map;
    color_map[DAY_BLACKBACK] = blackback_color_map;
    color_map[DUSK] = dusk_color_map;
    color_map[NIGHT] = night_color_map;

}
/**
 * Load the object name file
 */
void Configuration::LoadObjectNameFile()
{
    std::ifstream file;
    FilePath tmp_fpath = dir / "objectnames";
    file.open(tmp_fpath.string().c_str());

    if (!file) {
        std::string errstr = "Configuration::LoadObjectNameFile(): "
                    "Couldn't open file " + (dir / "objectnames").string();
        throw errstr;
    }

    std::string field;
    while(!file.eof()) {
        std::string int_name;
        std::string string_name;
        std::getline(file, int_name, ' ');
        std::getline(file, string_name);

        remove_whitespace(int_name);
        remove_whitespace(string_name);

        if (int_name != "" && string_name != "") {
            if (object_name_num_map[int_name])
                object_num_fullname_map[object_name_num_map[int_name]] =
                                                                string_name;
        }
    }

    file.close();
}



/**
 * Load the catalog file 
 */
void Configuration::LoadCatalogFile()
{
    std::ifstream file;
    FilePath tmp_fpath = dir / "enc";
    file.open(tmp_fpath.string().c_str());

    if (!file) {
        std::string errstr = "Configuration::LoadCatalogFile():"
                   "Couldn't open file " + (dir / "enc").string();
        throw errstr;
    }

    std::string field;
    std::getline(file, field);

    remove_whitespace(field);

    file.close();

    if (field == "NONE")
        catalog_loaded = false;
    else {
        catalog = field;
        if (!is_catalog(field.c_str())) {
            std::string errstr = "Configuration::LoadCatalogFile(): " + field;
            throw errstr;
        }
        catalog_loaded = true;
    }
}



/**
 * Generate the enc catalog configuration
 * @param catalog_file catalog file
 */
void Configuration::GenerateCatalogConfiguration(FilePath &catalog_file)
{
    if (!(dir.exists())) {
        std::string errstr = "Configuration::GenerateCatalogConfiguration: "
                             "No config dir at " + dir.string();
        catalog_loaded = false;
        throw errstr;
    }

    if (!is_catalog(catalog_file.string().c_str())) {
        std::string errstr = "Configuration::GenerateCatalogConfiguration: "
                  "No config dir at " + dir.string();
        catalog_loaded = false;
        throw errstr;
    }

    FilePath filename = dir / "enc";
    std::ofstream enc(filename.string().c_str(), std::ios::trunc);
    if (!enc.is_open()) {
        std::string errstr = "Configuration::GenerateCatalogConfiguration:: "
                  "Error generating enc file";
        catalog_loaded = false;
        throw errstr;
    } else {
        enc << catalog_file.string() << std::endl;
    }
    enc.close();

    catalog_loaded = true;
    catalog = catalog_file;
}

/**
 * Generate the rest of the configuration
 */
void Configuration::GenerateNecessaryConfiguration()
{
    FilePath filename;

    if (!dir.exists())
        dir.create_directory();

    // generate blank enc config
    filename = dir / "enc";
    std::ofstream enc(filename.string().c_str(), std::ios::trunc);
    if (!enc.is_open()) {
        throw std::string("Configuration::GenerateConfiguration:: "
                          "Error generating enc file"); 
    } else enc << "NONE" << std::endl;

    enc.close();


    // create the default objectnames file
    filename = dir / "objectnames";
    std::ofstream objectnames(filename.string().c_str(), std::ios::trunc);
    if (!objectnames.is_open()) {
        throw std::string("Configuration::GenerateConfiguration:: "
                         "Error generating objectnames file");
    } else {
        objectnames << "ACHARE Anchorage area" << std::endl;
        objectnames << "ACHBRT Anchor berth" << std::endl;
        objectnames << "ADMARE Administration Area (Named)" << std::endl;
        objectnames << "AIRARE Airport/airfield" << std::endl;
        objectnames << "BCNCAR Beacon, cardinal" << std::endl;
        objectnames << "BCNISD Beacon, isolated danger" << std::endl;
        objectnames << "BCNLAT Beacon, lateral" << std::endl;
        objectnames << "BCNSAW Beacon, safe water" << std::endl;
        objectnames << "BCNSPP Beacon, special purpose/general" << std::endl;
        objectnames << "BERTHS Berth" << std::endl;
        objectnames << "BOYCAR Buoy, cardinal" << std::endl;
        objectnames << "BOYINB Buoy, installation" << std::endl;
        objectnames << "BOYISD Buoy, isolated danger" << std::endl;
        objectnames << "BOYLAT Buoy, lateral" << std::endl;
        objectnames << "BOYSAW Buoy, safe water" << std::endl;
        objectnames << "BOYSPP Buoy, special purpose/general" << std::endl;
        objectnames << "BRIDGE Bridge" << std::endl;
        objectnames << "BUAARE Built-up area" << std::endl;
        objectnames << "BUISGL Building, single" << std::endl;
        objectnames << "CANALS Canal" << std::endl;
        objectnames << "CAUSWY Causeway" << std::endl;
        objectnames << "CBLARE Cable area" << std::endl;
        objectnames << "CBLOHD Cable, overhead" << std::endl;
        objectnames << "CBLSUB Cable, submarine" << std::endl;
        objectnames << "CGUSTA Coastguard station" << std::endl;
        objectnames << "CHKPNT Checkpoint" << std::endl;
        objectnames << "COALNE Coastline" << std::endl;
        objectnames << "CONVYR Conveyor" << std::endl;
        objectnames << "CONZNE Contiguous zone" << std::endl;
        objectnames << "COSARE Continental shelf area" << std::endl;
        objectnames << "CRANES Crane" << std::endl;
        objectnames << "CTNARE Caution area" << std::endl;
        objectnames << "CTRPNT Control point" << std::endl;
        objectnames << "CTSARE Cargo transhipment area" << std::endl;
        objectnames << "CURENT Current - non-gravitational" << std::endl;
        objectnames << "CUSZNE Custom zone" << std::endl;
        objectnames << "DAMCON Dam" << std::endl;
        objectnames << "DAYMAR Daymark" << std::endl;
        objectnames << "DEPARE Depth area" << std::endl;
        objectnames << "DEPCNT Depth contour" << std::endl;
        objectnames << "DISMAR Distance mark" << std::endl;
        objectnames << "DOCARE Dock area" << std::endl;
        objectnames << "DRGARE Dredged area" << std::endl;
        objectnames << "DRYDOC Dry dock" << std::endl;
        objectnames << "DMPGRD Dumping ground" << std::endl;
        objectnames << "DYKCON Dyke" << std::endl;
        objectnames << "DWRTCL Deep water route centerline" << std::endl;
        objectnames << "DWRTPT Deep water route part" << std::endl;
        objectnames << "EXEZNE Exclusive economic zone" << std::endl;
        objectnames << "FAIRWY Fairway" << std::endl;
        objectnames << "FERYRT Ferry route" << std::endl;
        objectnames << "FLODOC Floating dock" << std::endl;
        objectnames << "FNCLNE Fence/wall" << std::endl;
        objectnames << "FOGSIG Fog signal" << std::endl;
        objectnames << "FORSTC Fortified structure" << std::endl;
        objectnames << "FRPARE Free port area" << std::endl;
        objectnames << "FSHFAC Fishing facility" << std::endl;
        objectnames << "FSHGRD Fishing ground" << std::endl;
        objectnames << "FSHZNE Fishery zone" << std::endl;
        objectnames << "GATCON Gate" << std::endl;
        objectnames << "GRIDRN Gridiron" << std::endl;
        objectnames << "HRBARE Harbour area (administrative)" << std::endl;
        objectnames << "HRBFAC Harbour facility" << std::endl;
        objectnames << "HULKES Hulk" << std::endl;
        objectnames << "ICEARE Ice area" << std::endl;
        objectnames << "ICNARE Incineration area" << std::endl;
        objectnames << "ISTZNE Inshore traffic zone" << std::endl;
        objectnames << "LAKARE Lake" << std::endl;
        objectnames << "LNDARE Land area" << std::endl;
        objectnames << "LNDELV Land elevation" << std::endl;
        objectnames << "LNDMRK Landmark" << std::endl;
        objectnames << "LNDRGN Land region" << std::endl;
        objectnames << "LIGHTS Light" << std::endl;
        objectnames << "LITFLT Light float" << std::endl;
        objectnames << "LITVES Light vessel" << std::endl;
        objectnames << "LOCMAG Local magnetic anomaly" << std::endl;
        objectnames << "LOGPON Log pond" << std::endl;
        objectnames << "LOKBSN Lock basin" << std::endl;
        objectnames << "MAGVAR Magnetic variation" << std::endl;
        objectnames << "MARCUL Marine farm/culture" << std::endl;
        objectnames << "MIPARE Military practice area" << std::endl;
        objectnames << "MORFAC Mooring/Warping facility" << std::endl;
        objectnames << "NAVLNE Navigation line" << std::endl;
        objectnames << "OBSTRN Obstruction" << std::endl;
        objectnames << "OFSPLF Offshore platform" << std::endl;
        objectnames << "OSPARE Offshore production area" << std::endl;
        objectnames << "OILBAR Oil barrier" << std::endl;
        objectnames << "PILBOP Pilot boarding place" << std::endl;
        objectnames << "PILPNT Pile" << std::endl;
        objectnames << "PIPARE Pipeline area" << std::endl;
        objectnames << "PIPOHD Pipeline, overhead" << std::endl;
        objectnames << "PIPSOL Pipeline, submarine/on land" << std::endl;
        objectnames << "PONTON Pontoon" << std::endl;
        objectnames << "PRCARE Precautionary area" << std::endl;
        objectnames << "PRDARE Production/storage area" << std::endl;
        objectnames << "PYLONS Pylon/bridge support" << std::endl;
        objectnames << "RADLNE Radar line" << std::endl;
        objectnames << "RADRNG Radar range" << std::endl;
        objectnames << "RADRFL Radar reflector" << std::endl;
        objectnames << "RADSTA Radar station" << std::endl;
        objectnames << "RAILWY Railway" << std::endl;
        objectnames << "RAPIDS Rapids" << std::endl;
        objectnames << "RCRTCL Recommended route centerline" << std::endl;
        objectnames << "RCTLPT Recommended traffic lane part" << std::endl;
        objectnames << "RDOCAL Radio calling-in point" << std::endl;
        objectnames << "RDOSTA Radio station" << std::endl;
        objectnames << "RECTRC Recommended track" << std::endl;
        objectnames << "RESARE Restricted area" << std::endl;
        objectnames << "RETRFL Retro-reflector" << std::endl;
        objectnames << "RIVERS River" << std::endl;
        objectnames << "ROADWY Road" << std::endl;
        objectnames << "RSCSTA Rescue station" << std::endl;
        objectnames << "RTPBCN Radar transponder beacon" << std::endl;
        objectnames << "RUNWAY Runway" << std::endl;
        objectnames << "SBDARE Seabed area" << std::endl;
        objectnames << "SEAARE Sea area/named water area" << std::endl;
        objectnames << "SILTNK Silo/tank" << std::endl;
        objectnames << "SISTAT Signal station, traffic" << std::endl;
        objectnames << "SISTAW Signal station, warning" << std::endl;
        objectnames << "SLCONS Shoreline construction" << std::endl;
        objectnames << "SLOTOP Slope topline" << std::endl;
        objectnames << "SLOGRD Sloping ground" << std::endl;
        objectnames << "SMCFAC Small craft facility" << std::endl;
        objectnames << "SOUNDG Sounding" << std::endl;
        objectnames << "SNDWAV Sand waves" << std::endl;
        objectnames << "SPLARE Sea-plane landing area" << std::endl;
        objectnames << "SPRING Spring" << std::endl;
        objectnames << "STSLNE Straight territorial sea baseline" << std::endl;
        objectnames << "SUBTLN Submarine transit lane" << std::endl;
        objectnames << "SWPARE Swept Area" << std::endl;
        objectnames << "TESARE Territorial sea area" << std::endl;
        objectnames << "TIDEWY Tideway" << std::endl;
        objectnames << "TOPMAR Topmark" << std::endl;
        objectnames << "TSELNE Traffic separation line" << std::endl;
        objectnames << "TSEZNE Traffic separation zone" << std::endl;
        objectnames << "TSSBND Traffic separation scheme boundary"
                    << std::endl;
        objectnames << "TSSCRS Traffic separation scheme crossing"
                    << std::endl;
        objectnames << "TSSLPT Traffic separation scheme lane part"
                    << std::endl;
        objectnames << "TSSRON Traffic separation scheme roundabout"
                    << std::endl;
        objectnames << "TUNNEL Tunnel" << std::endl;
        objectnames << "TWRTPT Two-way route part" << std::endl;
        objectnames << "UNSARE Unsurveyed area" << std::endl;
        objectnames << "UWTROC Underwater/awash rock" << std::endl;
        objectnames << "VEGATN Vegetation" << std::endl;
        objectnames << "WATFAL Waterfall" << std::endl;
        objectnames << "WATTUR Water turbulence" << std::endl;
        objectnames << "WEDKLP Weed/Kelp" << std::endl;
        objectnames << "WRECKS Wreck" << std::endl;
        objectnames << "C_AGGR Aggregation" << std::endl;
        objectnames << "C_ASSO Association" << std::endl;
        objectnames << "M_ACCY Accuracy of data" << std::endl;
        objectnames << "M_COVR Coverage" << std::endl;
        objectnames << "M_CSCL Compilation scale of data" << std::endl;
        objectnames << "M_HOPA Horizontal datum shift parameters" << std::endl;
        objectnames << "M_NPUB Nautical publication information" << std::endl;
        objectnames << "M_NSYS Navigational system of marks" << std::endl;
        objectnames << "M_QUAL Quality of data" << std::endl;
        objectnames << "M_SDAT Sounding datum" << std::endl;
        objectnames << "M_SREL Survey reliability" << std::endl;
        objectnames << "M_VDAT Vertical datum of data" << std::endl;
        objectnames << "T_HMON Tide - harmonic prediction" << std::endl;
        objectnames << "T_NHMN Tide - non-harmonic prediction" << std::endl;
        objectnames << "T_TIMS Tide - time series" << std::endl;
        objectnames << "TS_FEB Tidal stream - flood/ebb" << std::endl;
        objectnames << "TS_PAD Tidal stream panel data" << std::endl;
        objectnames << "TS_PNH Tidal stream - non-harmonic prediction"
                    << std::endl;
        objectnames << "TS_PRH Tidal stream - harmonic prediction" << std::endl;
        objectnames << "TS_TIS Tidal stream - time series" << std::endl;
    }
    objectnames.close();

}

void Configuration::remove_whitespace(std::string &str)
{
    while (isspace(str[str.length()-1]))
        str.erase(str.length()-1);

    while (isspace(*(str.begin())))
        str.erase(str.begin());
}

bool Configuration::is_catalog(const char *filename)
{
    DDFModule ddf;
    if (ddf.Open(filename) == 0) { // DDFModule returns 0 on fail
        ddf.Close();
        return false;
    }
    DDFRecord *record;

    while ((record = ddf.ReadRecord()) != NULL) {
        int field_count = record->GetFieldCount();
        if (field_count != 2) {
            ddf.Close();
            return false;
        }
        DDFField *field = record->GetField(1);
        if (!field) {
            ddf.Close();
            return false;
        }

        const char *field_name;
        DDFFieldDefn *field_defn;

        field_defn = field->GetFieldDefn();
        field_name = field_defn->GetName();

        if (strncmp(field_name, "CATD", 4) != 0) {
            ddf.Close();
            return false;
        }

        if (field_defn->GetSubfieldCount() != 12) {
            ddf.Close();
            return false;
        }
    }

    ddf.Close();
    return true;
}

/**
 * Setup the object:name number map.
 */
void Configuration::SetupObjectNameNumMap()
{
    object_name_num_map.insert(std::make_pair("ADMARE", ADMARE));
    object_name_num_map.insert(std::make_pair("AIRARE", AIRARE));
    object_name_num_map.insert(std::make_pair("ACHBRT", ACHBRT));
    object_name_num_map.insert(std::make_pair("ACHARE", ACHARE));
    object_name_num_map.insert(std::make_pair("BCNCAR", BCNCAR));
    object_name_num_map.insert(std::make_pair("BCNISD", BCNISD));
    object_name_num_map.insert(std::make_pair("BCNLAT", BCNLAT));
    object_name_num_map.insert(std::make_pair("BCNSAW", BCNSAW));
    object_name_num_map.insert(std::make_pair("BCNSPP", BCNSPP));
    object_name_num_map.insert(std::make_pair("BERTHS", BERTHS));
    object_name_num_map.insert(std::make_pair("BRIDGE", BRIDGE));
    object_name_num_map.insert(std::make_pair("BUISGL", BUISGL));
    object_name_num_map.insert(std::make_pair("BUAARE", BUAARE));
    object_name_num_map.insert(std::make_pair("BOYCAR", BOYCAR));
    object_name_num_map.insert(std::make_pair("BOYINB", BOYINB));
    object_name_num_map.insert(std::make_pair("BOYISD", BOYISD));
    object_name_num_map.insert(std::make_pair("BOYLAT", BOYLAT));
    object_name_num_map.insert(std::make_pair("BOYSAW", BOYSAW));
    object_name_num_map.insert(std::make_pair("BOYSPP", BOYSPP));
    object_name_num_map.insert(std::make_pair("CBLARE", CBLARE));
    object_name_num_map.insert(std::make_pair("CBLOHD", CBLOHD));
    object_name_num_map.insert(std::make_pair("CBLSUB", CBLSUB));
    object_name_num_map.insert(std::make_pair("CANALS", CANALS));
    object_name_num_map.insert(std::make_pair("CANBNK", CANBNK));
    object_name_num_map.insert(std::make_pair("CTSARE", CTSARE));
    object_name_num_map.insert(std::make_pair("CAUSWY", CAUSWY));
    object_name_num_map.insert(std::make_pair("CTNARE", CTNARE));
    object_name_num_map.insert(std::make_pair("CHKPNT", CHKPNT));
    object_name_num_map.insert(std::make_pair("CGUSTA", CGUSTA));
    object_name_num_map.insert(std::make_pair("COALNE", COALNE));
    object_name_num_map.insert(std::make_pair("CONZNE", CONZNE));
    object_name_num_map.insert(std::make_pair("COSARE", COSARE));
    object_name_num_map.insert(std::make_pair("CTRPNT", CTRPNT));
    object_name_num_map.insert(std::make_pair("CONVYR", CONVYR));
    object_name_num_map.insert(std::make_pair("CRANES", CRANES));
    object_name_num_map.insert(std::make_pair("CURENT", CURENT));
    object_name_num_map.insert(std::make_pair("CUSZNE", CUSZNE));
    object_name_num_map.insert(std::make_pair("DAMCON", DAMCON));
    object_name_num_map.insert(std::make_pair("DAYMAR", DAYMAR));
    object_name_num_map.insert(std::make_pair("DWRTCL", DWRTCL));
    object_name_num_map.insert(std::make_pair("DWRTPT", DWRTPT));
    object_name_num_map.insert(std::make_pair("DEPARE", DEPARE));
    object_name_num_map.insert(std::make_pair("DEPCNT", DEPCNT));
    object_name_num_map.insert(std::make_pair("DISMAR", DISMAR));
    object_name_num_map.insert(std::make_pair("DOCARE", DOCARE));
    object_name_num_map.insert(std::make_pair("DRGARE", DRGARE));
    object_name_num_map.insert(std::make_pair("DRYDOC", DRYDOC));
    object_name_num_map.insert(std::make_pair("DMPGRD", DMPGRD));
    object_name_num_map.insert(std::make_pair("DYKCON", DYKCON));
    object_name_num_map.insert(std::make_pair("EXEZNE", EXEZNE));
    object_name_num_map.insert(std::make_pair("FAIRWY", FAIRWY));
    object_name_num_map.insert(std::make_pair("FNCLNE", FNCLNE));
    object_name_num_map.insert(std::make_pair("FERYRT", FERYRT));
    object_name_num_map.insert(std::make_pair("FSHZNE", FSHZNE));
    object_name_num_map.insert(std::make_pair("FSHFAC", FSHFAC));
    object_name_num_map.insert(std::make_pair("FSHGRD", FSHGRD));
    object_name_num_map.insert(std::make_pair("FLODOC", FLODOC));
    object_name_num_map.insert(std::make_pair("FOGSIG", FOGSIG));
    object_name_num_map.insert(std::make_pair("FORSTC", FORSTC));
    object_name_num_map.insert(std::make_pair("FRPARE", FRPARE));
    object_name_num_map.insert(std::make_pair("GATCON", GATCON));
    object_name_num_map.insert(std::make_pair("GRIDRN", GRIDRN));
    object_name_num_map.insert(std::make_pair("HRBARE", HRBARE));
    object_name_num_map.insert(std::make_pair("HRBFAC", HRBFAC));
    object_name_num_map.insert(std::make_pair("HULKES", HULKES));
    object_name_num_map.insert(std::make_pair("ICEARE", ICEARE));
    object_name_num_map.insert(std::make_pair("ICNARE", ICNARE));
    object_name_num_map.insert(std::make_pair("ISTZNE", ISTZNE));
    object_name_num_map.insert(std::make_pair("LAKARE", LAKARE));
    object_name_num_map.insert(std::make_pair("LAKSHR", LAKSHR));
    object_name_num_map.insert(std::make_pair("LNDARE", LNDARE));
    object_name_num_map.insert(std::make_pair("LNDELV", LNDELV));
    object_name_num_map.insert(std::make_pair("LNDRGN", LNDRGN));
    object_name_num_map.insert(std::make_pair("LNDMRK", LNDMRK));
    object_name_num_map.insert(std::make_pair("LIGHTS", LIGHTS));
    object_name_num_map.insert(std::make_pair("LITFLT", LITFLT));
    object_name_num_map.insert(std::make_pair("LITVES", LITVES));
    object_name_num_map.insert(std::make_pair("LOCMAG", LOCMAG));
    object_name_num_map.insert(std::make_pair("LOKBSN", LOKBSN));
    object_name_num_map.insert(std::make_pair("LOGPON", LOGPON));
    object_name_num_map.insert(std::make_pair("MAGVAR", MAGVAR));
    object_name_num_map.insert(std::make_pair("MARCUL", MARCUL));
    object_name_num_map.insert(std::make_pair("MIPARE", MIPARE));
    object_name_num_map.insert(std::make_pair("MORFAC", MORFAC));
    object_name_num_map.insert(std::make_pair("NAVLNE", NAVLNE));
    object_name_num_map.insert(std::make_pair("OBSTRN", OBSTRN));
    object_name_num_map.insert(std::make_pair("OFSPLF", OFSPLF));
    object_name_num_map.insert(std::make_pair("OSPARE", OSPARE));
    object_name_num_map.insert(std::make_pair("OILBAR", OILBAR));
    object_name_num_map.insert(std::make_pair("PILPNT", PILPNT));
    object_name_num_map.insert(std::make_pair("PILBOP", PILBOP));
    object_name_num_map.insert(std::make_pair("PIPARE", PIPARE));
    object_name_num_map.insert(std::make_pair("PIPOHD", PIPOHD));
    object_name_num_map.insert(std::make_pair("PIPSOL", PIPSOL));
    object_name_num_map.insert(std::make_pair("PONTON", PONTON));
    object_name_num_map.insert(std::make_pair("PRCARE", PRCARE));
    object_name_num_map.insert(std::make_pair("PRDARE", PRDARE));
    object_name_num_map.insert(std::make_pair("PYLONS", PYLONS));
    object_name_num_map.insert(std::make_pair("RADLNE", RADLNE));
    object_name_num_map.insert(std::make_pair("RADRNG", RADRNG));
    object_name_num_map.insert(std::make_pair("RADRFL", RADRFL));
    object_name_num_map.insert(std::make_pair("RADSTA", RADSTA));
    object_name_num_map.insert(std::make_pair("RTPBCN", RTPBCN));
    object_name_num_map.insert(std::make_pair("RDOCAL", RDOCAL));
    object_name_num_map.insert(std::make_pair("RDOSTA", RDOSTA));
    object_name_num_map.insert(std::make_pair("RAILWY", RAILWY));
    object_name_num_map.insert(std::make_pair("RAPIDS", RAPIDS));
    object_name_num_map.insert(std::make_pair("RCRTCL", RCRTCL));
    object_name_num_map.insert(std::make_pair("RECTRC", RECTRC));
    object_name_num_map.insert(std::make_pair("RCTLPT", RCTLPT));
    object_name_num_map.insert(std::make_pair("RSCSTA", RSCSTA));
    object_name_num_map.insert(std::make_pair("RESARE", RESARE));
    object_name_num_map.insert(std::make_pair("RETRFL", RETRFL));
    object_name_num_map.insert(std::make_pair("RIVERS", RIVERS));
    object_name_num_map.insert(std::make_pair("RIVBNK", RIVBNK));
    object_name_num_map.insert(std::make_pair("ROADWY", ROADWY));
    object_name_num_map.insert(std::make_pair("RUNWAY", RUNWAY));
    object_name_num_map.insert(std::make_pair("SNDWAV", SNDWAV));
    object_name_num_map.insert(std::make_pair("SEAARE", SEAARE));
    object_name_num_map.insert(std::make_pair("SPLARE", SPLARE));
    object_name_num_map.insert(std::make_pair("SBDARE", SBDARE));
    object_name_num_map.insert(std::make_pair("SLCONS", SLCONS));
    object_name_num_map.insert(std::make_pair("SISTAT", SISTAT));
    object_name_num_map.insert(std::make_pair("SISTAW", SISTAW));
    object_name_num_map.insert(std::make_pair("SILTNK", SILTNK));
    object_name_num_map.insert(std::make_pair("SLOTOP", SLOTOP));
    object_name_num_map.insert(std::make_pair("SLOGRD", SLOGRD));
    object_name_num_map.insert(std::make_pair("SMCFAC", SMCFAC));
    object_name_num_map.insert(std::make_pair("SOUNDG", SOUNDG));
    object_name_num_map.insert(std::make_pair("SPRING", SPRING));
    object_name_num_map.insert(std::make_pair("SQUARE", SQUARE));
    object_name_num_map.insert(std::make_pair("STSLNE", STSLNE));
    object_name_num_map.insert(std::make_pair("SUBTLN", SUBTLN));
    object_name_num_map.insert(std::make_pair("SWPARE", SWPARE));
    object_name_num_map.insert(std::make_pair("TESARE", TESARE));
    object_name_num_map.insert(std::make_pair("TS_PRH", TS_PRH));
    object_name_num_map.insert(std::make_pair("TS_PNH", TS_PNH));
    object_name_num_map.insert(std::make_pair("TS_PAD", TS_PAD));
    object_name_num_map.insert(std::make_pair("TS_TIS", TS_TIS));
    object_name_num_map.insert(std::make_pair("T_HMON", T_HMON));
    object_name_num_map.insert(std::make_pair("T_NHMN", T_NHMN));
    object_name_num_map.insert(std::make_pair("T_TIMS", T_TIMS));
    object_name_num_map.insert(std::make_pair("TIDEWY", TIDEWY));
    object_name_num_map.insert(std::make_pair("TOPMAR", TOPMAR));
    object_name_num_map.insert(std::make_pair("TSELNE", TSELNE));
    object_name_num_map.insert(std::make_pair("TSSBND", TSSBND));
    object_name_num_map.insert(std::make_pair("TSSCRS", TSSCRS));
    object_name_num_map.insert(std::make_pair("TSSLPT", TSSLPT));
    object_name_num_map.insert(std::make_pair("TSSRON", TSSRON));
    object_name_num_map.insert(std::make_pair("TSEZNE", TSEZNE));
    object_name_num_map.insert(std::make_pair("TUNNEL", TUNNEL));
    object_name_num_map.insert(std::make_pair("TWRTPT", TWRTPT));
    object_name_num_map.insert(std::make_pair("UWTROC", UWTROC));
    object_name_num_map.insert(std::make_pair("UNSARE", UNSARE));
    object_name_num_map.insert(std::make_pair("VEGATN", VEGATN));
    object_name_num_map.insert(std::make_pair("WATTUR", WATTUR));
    object_name_num_map.insert(std::make_pair("WATFAL", WATFAL));
    object_name_num_map.insert(std::make_pair("WEDKLP", WEDKLP));
    object_name_num_map.insert(std::make_pair("WRECKS", WRECKS));
    object_name_num_map.insert(std::make_pair("TS_FEB", TS_FEB));
    object_name_num_map.insert(std::make_pair("M_ACCY", M_ACCY));
    object_name_num_map.insert(std::make_pair("M_CSCL", M_CSCL));
    object_name_num_map.insert(std::make_pair("M_COVR", M_COVR));
    object_name_num_map.insert(std::make_pair("M_HDAT", M_HDAT));
    object_name_num_map.insert(std::make_pair("M_HOPA", M_HOPA));
    object_name_num_map.insert(std::make_pair("M_NPUB", M_NPUB));
    object_name_num_map.insert(std::make_pair("M_NSYS", M_NSYS));
    object_name_num_map.insert(std::make_pair("M_PROD", M_PROD));
    object_name_num_map.insert(std::make_pair("M_QUAL", M_QUAL));
    object_name_num_map.insert(std::make_pair("M_SDAT", M_SDAT));
    object_name_num_map.insert(std::make_pair("M_SREL", M_SREL));
    object_name_num_map.insert(std::make_pair("M_UNIT", M_UNIT));
    object_name_num_map.insert(std::make_pair("M_VDAT", M_VDAT));
    object_name_num_map.insert(std::make_pair("C_AGGR", C_AGGR));
    object_name_num_map.insert(std::make_pair("C_ASSO", C_ASSO));
    object_name_num_map.insert(std::make_pair("C_STAC", C_STAC));
    object_name_num_map.insert(std::make_pair("_AREAS", _AREAS));
    object_name_num_map.insert(std::make_pair("_LINES", _LINES));
    object_name_num_map.insert(std::make_pair("_CSYMB", _CSYMB));
    object_name_num_map.insert(std::make_pair("_COMPS", _COMPS));
    object_name_num_map.insert(std::make_pair("_TEXTS", _TEXTS));
}

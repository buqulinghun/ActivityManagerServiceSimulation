/********************************************************************
 *日期: 2016-01-12
 *作者: 王名孝
 *作用: 加载各种配置文件，包括lookup表/属性表/颜色表/符号表/复杂线型表/模板表及一些定义
 *修改:
 *提示：
      1.字符显示TE/TX：显示字符串，代表字符串的属性编码，水平偏移（123分别为字符串中心在中间/右边/左边）,
                  垂直偏移量（123分别为字符串中心在底部/中间案/顶部），符号间隔（123分别代表字符串均匀分布在物标中间/标准空格/标准空格加单行超过8个字符换行）,
                  字符格式（“15110”表示字体/重量/画笔宽度/符号高度（10/3.51mm）），X轴偏移量，Y轴偏移量（单位为符号高度，3.51mm），
                  颜色编码，显示组（由航海人员选择是否显示）
 ********************************************************************/

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <map>
#include <string>
#include <QVector>
#include <QMap>
#include <QDebug>

#include "filepath.h"
#include "s57/s57.h"

//显示类别枚举
enum DispCategory{
    DISPLAY_BASE = 1,
    STANDARD, OTHER, MARINERS_STANDARD, MARINERS_OTHER
};
//颜色表类型枚举
enum colorCategory{
    DAY_BRIGHT = 1,
    DAY_WHITEBACK, DAY_BLACKBACK, DUSK, NIGHT
};

//Look-up表分类
enum lookupCategory{
    PAPERPOINT = 1,    //(buoys and beacons are similar to the paper chart)
    SIMPLEPOINT,    // (buoys and beacons are more prominent)
    LINESYMBOL,
    PLAINAREA,   //(for general use)
    SYMBOLAREA   // (for large scale display)
};
//RGB结构体
typedef struct tagRgb{
    int r;
    int g;
    int b;
}Rgb;
//查找表中属性信息
typedef struct tagAttr
{
    int attl;    //在S57文件中枚举
    QString atvl;    //属性值,colour3,4,3类似这种不好用int表示
}Attr;
//查找表结构体
typedef struct tagLookupTable
{
    int objName;    //在S57文件中枚举
    QVector<Attr> attrValue;
    QString symInstruction;    //符号化指令
    int priority;   //显示优先级
    bool overRadar;     //雷达覆盖
    int dispcategory;    //显示类别
    int viewGroup;      //显示组
}LookupTable;
//属性表结构体，包含其枚举字符信息
typedef struct tagAttribute{
    int code;     //属性编码
    QVector<QString> attrType;   //枚举类型，主要用在字符显示提取内容,按顺序存储
}Attribute;





/**
 * Class containing configuration data which is used by the renderer
 * and by charts
 */
class Configuration
{
    public:
        Configuration();
        ~Configuration();


        void Load();
        void GenerateCatalogConfiguration(FilePath &catalog_file);
        void GenerateNecessaryConfiguration();

        // custom signal handlers
        void on_changed(feature_type_e type, int index, bool drawn);
        void on_line_color_set(int index, double r, double g, double b);

        // public data
        FilePath catalog;
        bool catalog_loaded;
        // name/num maps
        std::map<std::string, int> object_name_num_map;
        std::map<int, std::string> object_num_fullname_map;



        //属性名称和编码对应表
        QMap<QString, Attribute> attribute_name_num_map;
        //颜色名称和对应的RGB值表,分为五种
        QMap<int, QMap<QString, Rgb> > color_map;
        //lookup表中数据，分为5类，分别存放表示
        QMap<int, QVector<LookupTable> > Tables;

    private:
        // configuration loading helpers
        void LoadObjectNameFile();
        void LoadCatalogFile();
        void SetupObjectNameNumMap();
           
        void remove_whitespace(std::string &str);
        bool is_catalog(const char *filename);


        void LoadLookUpTable();   //加载look-up表
        void LoadAttributeCode();   //加载属性编码
        void LoadColorTable();    //加载颜色配置表



        FilePath dir;       //配置文件目录

};

#endif

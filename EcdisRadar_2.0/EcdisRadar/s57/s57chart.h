 /********************************************************************
 *日期: 2016-01-12
 *作者: 王名孝
 *作用: 加载海图原始文件，生成各个物标的详细信息，通过查找lookup表生成所需的绘图信息并保存，生成SENC文件
 *修改:
 ********************************************************************/

#ifndef S57CHART_H_
#define S57CHART_H_

#include "chart.h"

#include "iso8211lib/iso8211.h"

#include "feature.h"
#include "configuration.h"


/**
 * Class for an S57 Chart
 */
class S57Chart : public Chart
{
    public:
        S57Chart();
        ~S57Chart();
        void Load(const FilePath &fpath);


    private:
        // Load data into chart object classes

        void ReadDSIDRecord(DDFRecord *record);
        void ReadDSPMRecord(DDFRecord *record);

        void ReadFeatureRecord(DDFRecord *record);
        void LoadPointFeature(DDFRecord *record);
        void LoadLineFeature(DDFRecord *record);
        void LoadAreaFeature(DDFRecord *record);

        void ReadVectorRecord(DDFRecord *record);
        void LoadIsolatedNodeVectorRecord(DDFRecord *record);
        void LoadConnectedNodeVectorRecord(DDFRecord *record);
        void LoadEdgeVectorRecord(DDFRecord *record);



public:
        // vectors maps,索引为物标类型，后者里面包含物标的唯一编码
        std::map<int, std::vector<Feature> > point_features_map;
        std::map<int, std::vector<Feature> > line_features_map;
        std::map<int, std::vector<Feature> > area_features_map;

        // meta data loading

        bool dsid_loaded;

        bool dspm_loaded;

        int num_sg2ds;
};

#endif

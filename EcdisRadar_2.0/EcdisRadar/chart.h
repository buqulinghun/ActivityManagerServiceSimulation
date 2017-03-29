/********************************************************************
 *日期: 2016-01-12
 *作者: 王名孝
 *作用: 海图信息基本类，包含绘图基本信息
 *修改:
 ********************************************************************/

#ifndef CHART_H_
#define CHART_H_

#include <map>
#include <vector>


#include "filepath.h"
#include "longlat.h"
#include "pointobject.h"
#include "lineobject.h"
#include "areaobject.h"
#include "soundobject.h"
#include "s57/dsid.h"
#include "s57/dspm.h"
#include "s57/edgevector.h"
#include "s57/isolatednodevector.h"
#include "s57/connectednodevector.h"




/**
 * Base class for a chart
 */
class Chart
{
    public:
        Chart();
        virtual ~Chart();
        virtual void Load(const FilePath &fpath) {  };  // = 0   纯虚函数
        void DiscoverDimensions();  //获取海图尺寸
        void calculateCenter();  //计算区域中心

        void SetupPointObjects();   //完成各个物标的内容
        void SetupLineObjects();
        void SetupAreaObjects();
        void SetupSoundObjects();

        void NormalizeCoordinates();  //得到实际经纬度
        void SetupEdgeVectors();   //得到线段的引用关系




 // public data
        FilePath filepath;
        std::string description;

        DSID dsid;
        DSPM dspm;

        LongLat slat, wlon, nlat, elon;
        double Mslat, Mwlon, Mnlat, Melon;  //转换为墨卡托坐标后的边缘
        float absolute_zoom_factor;
        float zoom_exponent;
        std::vector<sg2d_t> sg2ds;

        //物标结构存储，按物标名称索引
        std::map<int, std::vector<LineObject> > line_objects_map;
        std::map<int, std::vector<PointObject> > point_objects_map;
        std::map<int, std::vector<AreaObject> > area_objects_map;
        std::vector<SoundObject> sound_object_vector;

        std::map<int, IsolatedNodeVector> isolated_node_vectors_map;   //标识为record_id号，唯一
        std::map<int, ConnectedNodeVector> connected_node_vectors_map;  //连接点应该都不用存了，它用在edge的组成中，读取的时候已经使用过了
        std::map<int, EdgeVector> edge_vectors_map;


        bool non_fatal_error;
};

#endif

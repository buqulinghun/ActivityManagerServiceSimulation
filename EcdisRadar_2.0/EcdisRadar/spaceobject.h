#ifndef SPACEOBJECT_H
#define SPACEOBJECT_H
#include <vector>
#include "s57/s57.h"
#include<QStringList>

class SpaceObject
{
public:
    SpaceObject();
    virtual ~SpaceObject(){};


    //属性信息
    int object_label;  //物标类型
    int record_id;  //物标唯一编码
    int group;
    int prim;

    int feature_id;
    int feature_subid;

    std::string name;  //物标名称

    std::vector<attf_t> attfs;   //属性字段
    std::vector<attf_t> natfs;    //国家属性字段
    std::vector<ffpt_t> ffpts;    //特征记录到特征物标指针
    std::vector<fspt_t> fspts;    //特征字段到空间字段指针




    //符号化相关
    int priority;   //显示优先级
    int dispcategory;    //显示类别
    bool overRadar;     //雷达覆盖
    QStringList symInstruction;    //符号化指令

};

#endif // SPACEOBJECT_H

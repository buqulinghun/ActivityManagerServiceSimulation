#ifndef SOUNDOBJECT_H
#define SOUNDOBJECT_H
#include "spaceobject.h"

#include <QVector>

#include "s57/s57.h"



class SoundObject:public SpaceObject
{
public:
    SoundObject();
    ~SoundObject();

    //空间属性
    std::vector<attv_t> attvs;
    std::vector<sg3d_t> sg3ds;

    //符号化相关   CS(SOUNDG02) OTHER 33010 OVERRADAR 6
   // QStringList symInstruction;    //符号化指令,只保存了符号名字

};

#endif // SOUNDOBJECT_H

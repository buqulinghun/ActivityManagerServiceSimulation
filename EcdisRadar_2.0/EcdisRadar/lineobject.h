#ifndef LINEOBJECT_H_
#define LINEOBJECT_H_

#include <vector>
#include <QString>
#include <QList>
#include <QStringList>
#include "spaceobject.h"

#include "s57/s57.h"

class LineObject :public SpaceObject
{
    public:
        LineObject();
        ~LineObject();

     //空间属性, 其属性值后面有需要再加
     std::vector<int> indices;

};

#endif

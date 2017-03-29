/* GHelm - Nautical Navigation Software
 * Copyright (C) 2004 Jon Michaelchuck
 *
 * This application is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.
 */

#ifndef EDGEVECTOR_H_
#define EDGEVECTOR_H_

#include <map>
#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <QVector>

#include "s57.h"
#include "iso8211lib/iso8211.h"
#include "connectednodevector.h"

/**
 * An edge is a string of SG2Ds with a pointer to a connected node at each
 * end.
 */
class EdgeVector 
{
    public:
        EdgeVector(); 
        ~EdgeVector();
        void CompleteEdge(std::map<int, ConnectedNodeVector>
                                    &connected_node_vectors_map);
        int beg_node;
        int end_node;
        int ornt;
        int usag;
        std::vector<int> sg2d_indices;
        std::vector<attv_t> attvs;     //这个属性值后面条件物标实现时需要

      //  QVector<int> quoteIndex;  //引用该线段的物标名称保存,如果有DEPCNT,则保存VALDCO属性
      //  float valdco;
};

#endif

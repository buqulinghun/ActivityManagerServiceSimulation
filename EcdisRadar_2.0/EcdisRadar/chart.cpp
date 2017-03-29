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

#include <iostream>
#include <math.h>

#include "chart.h"
#include<QDebug>

/**
 * Constructor
 */
Chart::Chart() :
    slat(0), wlon(0), nlat(0), elon(0), Mslat(0), Mwlon(0), Mnlat(0), Melon(0), absolute_zoom_factor(1),
    zoom_exponent(.45), non_fatal_error(false)
{
}

/**
 * Destructor
 */
Chart::~Chart()
{ 
}

/**
 * DiscoverDimensions - set the dimensions of the chart via iteration.
 */
void Chart::DiscoverDimensions()
{
    //坐标的经纬度已经转化为度
    nlat.minutes = elon.minutes = -10799;
    slat.minutes = wlon.minutes = 10799;
    std::vector<sg2d_t>::iterator sg2dit;
    std::vector<sg2d_t>::iterator sg2dend = sg2ds.end();
    for (sg2dit = sg2ds.begin(); sg2dit != sg2dend; ++sg2dit) {
        if (elon < sg2dit->long_lat[0])
            elon = sg2dit->long_lat[0];
        if (wlon > sg2dit->long_lat[0])
            wlon = sg2dit->long_lat[0];
        if (nlat < sg2dit->long_lat[1])
            nlat = sg2dit->long_lat[1];
        if (slat > sg2dit->long_lat[1])
            slat = sg2dit->long_lat[1];
    }
    if ((slat > nlat) || (wlon > elon)) {
        std::cerr << "Chart::DiscoverDimensions(): Bad dimensions" << std::endl;
        nlat.minutes = elon.minutes = 10799;
        slat.minutes = wlon.minutes = -10799;
    }
  //  qDebug()<<"eLon:";

  qDebug()<<"eLon:"<<elon.minutes<<" wlon: "<<wlon.minutes<<" nlat: "<<nlat.minutes<<" slat: "<<slat.minutes;

}


void Chart::calculateCenter()
{
    std::map<int, std::vector<AreaObject> >::iterator aom_it;
    std::map<int, std::vector<AreaObject> >::iterator aom_end;

    aom_end = area_objects_map.end();
    for (aom_it = area_objects_map.begin();aom_it != aom_end; ++aom_it) {
        //一类物标,名称为aom_it->first;
        /*if(aom_it->first == DEPARE || aom_it->first == RESARE || aom_it->first == ACHARE || aom_it->first == ACHBRT
           || aom_it->first == BERTHS || aom_it->first == BRIDGE || aom_it->first == CBLARE || aom_it->first == CHKPNT) */{
            std::vector<AreaObject>::iterator ao_it;
            std::vector<AreaObject>::iterator ao_end = aom_it->second.end();
            for (ao_it = aom_it->second.begin(); ao_it != ao_end; ++ao_it) {
            //此处应该计算面的中心，当有需要的时候

                    double Area = 0;
                    double X = 0;
                    double Y = 0;

                    std::vector<std::vector<int > >::iterator cit;
                    std::vector<std::vector<int > >::iterator cend = ao_it->contours.end();
                    for (cit = ao_it->contours.begin(); cit != cend; ++cit) {
                        std::vector<int>::iterator vit;
                        std::vector<int>::iterator vend = cit->end();
                        for (vit = cit->begin(); vit != vend - 1; ++vit) {
                            const double x1 = sg2ds[*vit].long_lat[0].minutes;
                            const double y1 = sg2ds[*vit].long_lat[1].minutes;
                            const double x2 = sg2ds[*(vit+1)].long_lat[0].minutes;
                            const double y2 = sg2ds[*(vit+1)].long_lat[1].minutes;

                            const double temp = (x1*y2 - x2*y1);

                            Area += temp;
                            X += ((x1 + x2) * temp);
                            Y += ((y1 + y2) * temp);
                        }

                    }
                    X /= (3 * Area);
                    Y /= (3 * Area);

                    ao_it->centerPoint = QPointF(X, Y);

             }

        }
    }
}

void Chart::SetupPointObjects()
{
    std::map<int, std::vector<PointObject> >::iterator pfm;
    std::map<int, std::vector<PointObject> >::iterator end(point_objects_map.end());
    for(pfm = point_objects_map.begin(); pfm != end; ++pfm){
        std::vector<PointObject>::iterator pit;
        std::vector<PointObject>::iterator pend = pfm->second.end();
        for(pit = pfm->second.begin(); pit != pend; ++pit) {

            //保存引用的空间物标信息
            std::vector<int> ref_record_ids;
            std::vector<fspt_t>::iterator fit;
            std::vector<fspt_t>::iterator fend = pit->fspts.end();
            for(fit = pit->fspts.begin(); fit != fend; ++fit) {
                ref_record_ids.push_back(fit->rcid);
            }


            //查找空间位置坐标
            if (ref_record_ids.size() != 1) {
                std::cerr << "Non-fatal-error: S57Chart::SetupPointObjects: sizeof ref record id != 1" << std::endl;
                non_fatal_error = true;
            }else {
                int id = ref_record_ids[0];

                std::map<int, IsolatedNodeVector>::iterator inv_it;
                inv_it = isolated_node_vectors_map.find(id);
                if ((inv_it == isolated_node_vectors_map.end())) {
                   // std::cerr << "Non-fatal-error: S57Chart::SetupPointObjects: point recordID out of range" << std::endl;
                    non_fatal_error = true;
                }else if((inv_it->second.sg2d_index >= sg2ds.size())){
                    std::cerr << "Non-fatal-error: S57Chart::SetupPointObjects: sg2d_index out of range" << std::endl;
                    non_fatal_error = true;
                }else {
                    pit->index = inv_it->second.sg2d_index;
                    pit->attvs = inv_it->second.attvs;  //获取空间属性
                }
            }

        }  //end for pit
    }

}
void Chart::SetupSoundObjects()
{
    std::vector<SoundObject>::iterator sit;
    std::vector<SoundObject>::iterator end = sound_object_vector.end();
    for(sit = sound_object_vector.begin(); sit != end; ++sit) {
        //保存引用的空间物标信息
        std::vector<int> ref_record_ids;
        std::vector<fspt_t>::iterator fit;
        std::vector<fspt_t>::iterator fend = sit->fspts.end();
        for(fit = sit->fspts.begin(); fit != fend; ++fit) {
            ref_record_ids.push_back(fit->rcid);
        }

        //查找空间位置坐标
        if (ref_record_ids.size() != 1) {
            std::cerr << "Non-fatal-error: S57Chart::SetupSoundObjects: sizeof ref record id != 1" << std::endl;
            non_fatal_error = true;
        } else {
            int id = ref_record_ids[0];

            std::map<int, IsolatedNodeVector>::iterator inv_it;
            inv_it = isolated_node_vectors_map.find(id);
            if ((inv_it == isolated_node_vectors_map.end())) {
                std::cerr << "Non-fatal-error: S57Chart::SetupSoundObjects: point recordID out of range" << std::endl;
                non_fatal_error = true;
            }else {

                sit->sg3ds = inv_it->second.sg3ds;
                sit->attvs = inv_it->second.attvs;  //获取空间属性
                //此处将水深值除以对应的比例系数得到真实的水深
                const int s = sit->sg3ds.size();
                for(int i=0; i<s; i++) {
                    sit->sg3ds[i].depth /= dspm.sounding_mult_factor;
                    sit->sg3ds[i].depth += 0.01;   //由于double型的精度问题，其结果不准确，比如2.3为2.299999999，且小数只需一位，所以增加值得到正确值,在后面为负数的时候要补上来
                }

            }
        }

    }  //end for sit
}

void Chart::SetupLineObjects()
{
    std::map<int, std::vector<LineObject> >::iterator lfm;
    std::map<int, std::vector<LineObject> >::iterator end = line_objects_map.end();
    for(lfm = line_objects_map.begin(); lfm != end; ++lfm) {
        std::vector<LineObject>::iterator lit;
        std::vector<LineObject>::iterator lend = lfm->second.end();
        for(lit = lfm->second.begin(); lit != lend; ++lit) {
            //保存引用的空间物标信息
            std::vector<int> ref_record_ids;
            std::vector<fspt_t>::iterator fit;
            std::vector<fspt_t>::iterator fend = lit->fspts.end();
            for(fit = lit->fspts.begin(); fit != fend; ++fit) {
                ref_record_ids.push_back(fit->rcid);
            }

            //建立空间坐标信息/////////////////////////
            std::vector<int>::const_iterator ref_it;
            std::vector<int>::const_iterator ref_end = ref_record_ids.end();
            std::map<int, EdgeVector>::iterator evm_it;
            int z(0);   //用于迭代线的方向容器
            for (ref_it = ref_record_ids.begin(); ref_it != ref_end; ++ref_it) {
                //将线段的所有坐标索引存入容器中
                evm_it = edge_vectors_map.find(*ref_it);
                if (evm_it == edge_vectors_map.end()) {
                    non_fatal_error = true;
                    std::cerr << "S57Chart::SetupLineObjects: "<< "ref_record_id out of bounds" << std::endl;
                } else {
                    std::vector<int>::const_iterator it;
                    std::vector<int>::const_iterator end = evm_it->second.sg2d_indices.end();

                    if(lit->fspts[z].ornt == 1){ //正向
                        for (it = evm_it->second.sg2d_indices.begin();it != end; ++it) {
                            if (*it > sg2ds.size()) {
                                non_fatal_error = true;
                                std::cerr << "S57Chart::SetupLines: " << "sg2d index out of range" << std::endl;
                            } else
                                lit->indices.push_back(*it);
                        }
                     }else{  //反向
                         for (it = evm_it->second.sg2d_indices.end()-1;
                                                           it != evm_it->second.sg2d_indices.begin()-1; --it) {
                             if (*it > sg2ds.size()) {
                                 non_fatal_error = true;
                                 std::cerr << "S57Chart::SetupLines: " << "sg2d index out of range" << std::endl;
                             } else
                                 lit->indices.push_back(*it);
                         }
                     }
                }
                z++;
            }
        }  // end for lit
    }
}

void Chart::SetupAreaObjects()
{
    std::map<int, EdgeVector>::iterator evm_it; // used for bounds checking
    std::map<int, std::vector<AreaObject> >::iterator afm;
    std::map<int, std::vector<AreaObject> >::iterator end = area_objects_map.end();
    for(afm = area_objects_map.begin(); afm != end; ++afm) {
        std::vector<AreaObject>::iterator ait;
        std::vector<AreaObject>::iterator aend = afm->second.end();
        for(ait = afm->second.begin(); ait != aend; ++ait) {
            //保存引用的空间物标信息
            std::vector<int> ref_record_ids;
            std::vector<fspt_t>::iterator fit;
            std::vector<fspt_t>::iterator fend = ait->fspts.end();
            for(fit = ait->fspts.begin(); fit != fend; ++fit) {
                ref_record_ids.push_back(fit->rcid);
            }

            //填充空间信息
            int ref_count = ref_record_ids.size();
            std::vector<int> contour_edges;
            std::map<int,int> Orient;//存储相应的线的方向
            // iterate through each edge, we record the first edge's first
            // vertex, we then go through each last vertex of each edge
            // to check if it equals the first edge's first vertex. if it
            // does we have a contour.
            int first_vertex;
            int last_vertex;
            if (ait->fspts[0].ornt == 1){
                first_vertex = edge_vectors_map[ref_record_ids[0]].sg2d_indices.front();
            } else {
                first_vertex = edge_vectors_map[ref_record_ids[0]].sg2d_indices.back();
            }

            for (int i = 0; i < ref_count; i++) {
                if (ait->fspts[i].ornt == 1) {
                    last_vertex = edge_vectors_map[ref_record_ids[i]].sg2d_indices.back();
                } else {
                    last_vertex = edge_vectors_map[ref_record_ids[i]].sg2d_indices.front();
                }

                contour_edges.push_back(ref_record_ids[i]);  //存储引用的线的索引号
                Orient[ref_record_ids[i]] = ait->fspts[i].ornt; //存储引用线方向
                // FIXME : write comparison operator for longlat

                //这里原先是采用如果引用的线条中有线段的首尾两点相同就把它存起来当作一个区域，就是一个完整的面。但是画出来会多余一些线条，
                //这是因为在加载坐标时没有考虑线的方向，存索引时如果方向为反需要从后面取依次存入
                if ((sg2ds[first_vertex].long_lat[0].minutes == sg2ds[last_vertex].long_lat[0].minutes) &&
                    (sg2ds[first_vertex].long_lat[1].minutes  == sg2ds[last_vertex].long_lat[1].minutes)) {
                    // we've found a complete contour! push it back and get a new beginning vertex.
                    std::vector<int> new_contour;

                    std::vector<int>::iterator ced_it;
                    std::vector<int>::iterator ced_end = contour_edges.end();
                    std::map<int,int>::iterator xyz;  //用来查找方向

                    for (ced_it = contour_edges.begin(); ced_it != ced_end; ++ced_it) {
                        //在面容器中查找引用的索引号对应的边
                        evm_it = edge_vectors_map.find(*ced_it);
                        xyz = Orient.find(*ced_it);

                        if (evm_it == edge_vectors_map.end()) {
                            std::cout << "ruhroh: " << *ced_it << std::endl;   // FIXME non fatal error.
                        } else {

                            std::vector<int>::iterator vit;
                            std::vector<int>::iterator vend = evm_it->second.sg2d_indices.end();
                            //将查找到的线中点索引存进容器中
                            if(xyz->second == 1){   //正向
                                for (vit = evm_it->second.sg2d_indices.begin(); vit != vend; ++vit) {
                                    new_contour.push_back(*vit);
                                }
                            }
                            else{  //反向
                                for (vit = evm_it->second.sg2d_indices.end()-1; vit != evm_it->second.sg2d_indices.begin()-1; --vit) {
                                    new_contour.push_back(*vit);
                                }
                            }

                        }
                    }

                    ait->contours.push_back(new_contour);
                    contour_edges.clear();
                    if (i < (ref_count - 1)) {
                        if (ait->fspts[i+1].ornt == 1) {
                            first_vertex = edge_vectors_map[ref_record_ids[i+1]].sg2d_indices.front();
                        } else {
                            first_vertex = edge_vectors_map[ref_record_ids[i+1]].sg2d_indices.back();
                        }
                    }
                }
            }

        }  //end for ait
    }
}





//*****************************************************************************
//
// Setup of edges, areas, display lists
//
//*****************************************************************************

/**
 * Multiply all coordinates by the normalization factor.
 */
void Chart::NormalizeCoordinates()
{
    //转化为度
    int coord_mult_factor = dspm.GetCoordMultFactor();

    std::map<int, IsolatedNodeVector>::iterator inv;
    std::map<int, IsolatedNodeVector>::iterator
                                iso_end(isolated_node_vectors_map.end());
    for (inv = isolated_node_vectors_map.begin(); inv != iso_end; ++inv)
        inv->second.Normalize(coord_mult_factor);

    std::vector<sg2d_t>::iterator sg2dit;
    std::vector<sg2d_t>::iterator sg2dend = sg2ds.end();
    for (sg2dit = sg2ds.begin(); sg2dit != sg2dend; ++sg2dit) {
        sg2dit->long_lat[0] = sg2dit->long_lat[0] / coord_mult_factor ;
        sg2dit->long_lat[1] = sg2dit->long_lat[1] / coord_mult_factor ;
    }

}

/**
 * Complete edge vectors
 */
void Chart::SetupEdgeVectors()
{
    std::map<int, EdgeVector>::iterator pos;
    std::map<int, EdgeVector>::iterator end = edge_vectors_map.end();
    for (pos = edge_vectors_map.begin(); pos != end; ++pos) {
        pos->second.CompleteEdge(connected_node_vectors_map);
    }
}


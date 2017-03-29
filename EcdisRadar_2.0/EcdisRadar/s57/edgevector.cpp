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

#include "edgevector.h"
#include <math.h>

/**
 * Constructor
 */
EdgeVector::EdgeVector() :
    beg_node(-1), end_node(-1)/*, valdco(-1.0)*/
{
}

/**
 * Destructor
 */
EdgeVector::~EdgeVector()
{
}

/**
 * Complete an edge given a connected node vectors map
 * @param connected_node_vectors_map Map to use
 */
void EdgeVector::CompleteEdge(std::map<int, ConnectedNodeVector>
                                    &connected_node_vectors_map)
{
    std::map<int, ConnectedNodeVector>::iterator cnv_it;

    cnv_it = connected_node_vectors_map.find(beg_node);

    if (cnv_it == connected_node_vectors_map.end())
        throw std::string("EdgeVector::CompleteEdge: First node out of range");

    int sg2d_index = cnv_it->second.sg2d_index;

    if (sg2d_index < 0)
        throw std::string("EdgeVector::CompleteEdge: Bad sg2d index");
    
    sg2d_indices.insert(sg2d_indices.begin(), sg2d_index);

    cnv_it = connected_node_vectors_map.find(end_node);

    if (cnv_it == connected_node_vectors_map.end())
        throw std::string("EdgeVector::CompleteEdge: End node out of range");

    sg2d_index = cnv_it->second.sg2d_index;

    if (sg2d_index < 0)
        throw std::string("EdgeVector::CompleteEdge: Bad sg2d index");

    sg2d_indices.push_back(sg2d_index);
}

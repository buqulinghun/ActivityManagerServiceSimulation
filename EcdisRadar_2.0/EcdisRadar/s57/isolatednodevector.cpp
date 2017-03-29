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

#include "isolatednodevector.h"

/**
 * Constructor
 */
IsolatedNodeVector::IsolatedNodeVector() :
    sg2d_index(-1)
{
}

/**
 * Destructor
 */
IsolatedNodeVector::~IsolatedNodeVector()
{
}

/**
 * Divide our coordinates by a factor
 * @param coord_mult_factor factor
 */
void IsolatedNodeVector::Normalize(float coord_mult_factor)
{
    std::vector<sg3d_t>::iterator it;
    std::vector<sg3d_t>::iterator end = sg3ds.end();
    for (it = sg3ds.begin(); it != end; ++it) {
        it->long_lat[0] /= coord_mult_factor;
        it->long_lat[1] /= coord_mult_factor;
    }
}

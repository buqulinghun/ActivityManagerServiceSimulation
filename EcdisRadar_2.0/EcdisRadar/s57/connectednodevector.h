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

#ifndef CONNECTEDNODEVECTOR_H_
#define CONNECTEDNODEVECTOR_H_

#include <vector>
#include <string>
#include <iostream>

#include "s57.h"
#include "iso8211lib/iso8211.h"

/**
 * A connected node is one which a beggining and/or end of an Edge node.
 */
class ConnectedNodeVector
{
    public:
        ConnectedNodeVector(); 
        ~ConnectedNodeVector();
        int sg2d_index;
        std::vector<attv_t> attvs;
};

#endif

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

#ifndef DSID_H_
#define DSID_H_

#include <string>
#include <iostream>
#include "iso8211lib/iso8211.h"

/**
 * Class for loading/holding DSID data
 */
class DSID
{
    public:
        DSID();
        ~DSID();
        void Load(DDFRecord * const record);
        int GetNumConnNodeRec() { return num_connnode_rec; }
        int GetNumEdgeRec() { return num_edge_rec; }
  //  private:
        // from dsid
        std::string dataset_name;
        std::string edition_num;
        std::string issue_date;
        std::string update_num;
        int s57_edition;
        int producing_agency;

        // from dssi
        int data_struct;
        int attf_lexl;
        int natf_lexl;
        int num_meta_rec;
        int num_cart_rec;
        int num_geo_rec;
        int num_coll_rec;
        int num_isonode_rec;
        int num_connnode_rec;
        int num_edge_rec;
        int num_face_rec;
};

#endif

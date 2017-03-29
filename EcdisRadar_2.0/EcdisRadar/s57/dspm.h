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

#ifndef DSPM_H_
#define DSPM_H_

#include <string>
#include <iostream>
#include "iso8211lib/iso8211.h"

/**
 * Class for loading/holding DSPM data
 */
class DSPM
{
    public:
        DSPM();
        ~DSPM();
        void Load(DDFRecord * const record);
        int GetCoordMultFactor() { return coord_mult_factor; }
        int GetSoundingMultFactor() { return sounding_mult_factor; }

        int GetScaleMultFactor()  {  return comp_sod;  }
   // private:
        int coord_mult_factor;
        int horz_gd_datum;
        int vert_datum;
        int sounding_datum;
        int comp_sod; // compilation scale of data绘图比例尺
        int depth_unit;
        int height_unit;
        int accuracy_unit;
        int coord_unit;
        int sounding_mult_factor;
};

#endif

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

#include "dspm.h"
#include "iso8211lib/iso8211.h"

/**
 * Constructor
 */
DSPM::DSPM() :
    coord_mult_factor(0), horz_gd_datum(0), vert_datum(0), sounding_datum(0),
    comp_sod(0), depth_unit(0), height_unit(0), accuracy_unit(0),
    coord_unit(0), sounding_mult_factor(0)
{
}

/**
 * Destructor
 */
DSPM::~DSPM()
{
}

/** Load dspm data from a record
 * @param record Record to read
 */
void DSPM::Load(DDFRecord * const record)
{
    int bytes_remaining;
    int bytes_consumed;
    const char *field_data;
    const char *field_name;
    DDFField *field;
    DDFFieldDefn *field_defn;
    DDFSubfieldDefn *subfield_defn;

    // Read the DSPM field and associated subfields
    field = record->GetField(1);
    if (!field)
        throw std::string("DSPM::Load: bad field");
    field_defn = field->GetFieldDefn();
    if (!field_defn)
        throw std::string("DSPM::Load: bad field defn");
    field_name = field_defn->GetName();
    field_data = field->GetData();
    if (!field_data)
        throw std::string("DSPM::Load: bad field data");
    bytes_remaining = field->GetDataSize();

    int tmp;

    subfield_defn = field_defn->GetSubfield(0);
    if (!subfield_defn)
        throw std::string("DSPM::Load: bad subfield defn");
    tmp = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    if (tmp != 20)
        throw std::string("DSPM::Load: bad rcnm");

    // skip rcid
    subfield_defn = field_defn->GetSubfield(1);
    if (!subfield_defn)
        throw std::string("DSPM::Load: bad subfield defn");
    subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;
   
    subfield_defn = field_defn->GetSubfield(2);
    if (!subfield_defn)
        throw std::string("DSPM::Load: bad subfield defn");
    horz_gd_datum = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;
    
    subfield_defn = field_defn->GetSubfield(3);
    if (!subfield_defn)
        throw std::string("DSPM::Load: bad subfield defn");
    vert_datum = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;
    
    subfield_defn = field_defn->GetSubfield(4);
    if (!subfield_defn)
        throw std::string("DSPM::Load: bad subfield defn");
    sounding_datum = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;
  
    subfield_defn = field_defn->GetSubfield(5);
    if (!subfield_defn)
        throw std::string("DSPM::Load: bad subfield defn");
    comp_sod = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    subfield_defn = field_defn->GetSubfield(6);
    if (!subfield_defn)
        throw std::string("DSPM::Load: bad subfield defn");
    depth_unit = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    subfield_defn = field_defn->GetSubfield(7);
    if (!subfield_defn)
        throw std::string("DSPM::Load: bad subfield defn");
    height_unit = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    subfield_defn = field_defn->GetSubfield(8);
    if (!subfield_defn)
        throw std::string("DSPM::Load: bad subfield defn");
    accuracy_unit = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    subfield_defn = field_defn->GetSubfield(9);
    if (!subfield_defn)
        throw std::string("DSPM::Load: bad subfield defn");
    coord_unit = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    subfield_defn = field_defn->GetSubfield(10);
    if (!subfield_defn)
        throw std::string("DSPM::Load: bad subfield defn");
    coord_mult_factor = subfield_defn->ExtractIntData(field_data,
                                 bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    subfield_defn = field_defn->GetSubfield(11);
    if (!subfield_defn)
        throw std::string("DSPM::Load: bad subfield defn");
    sounding_mult_factor = subfield_defn->ExtractIntData(field_data,
                                 bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    // TODO There is more stuff in the DSPM record!
}

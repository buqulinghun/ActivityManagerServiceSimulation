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

#include "dsid.h"
#include "iso8211lib/iso8211.h"

/**
 * Constructor
 */
DSID::DSID() :
    s57_edition(0), producing_agency(0), data_struct(0), attf_lexl(0),
    natf_lexl(0), num_meta_rec(0), num_cart_rec(0), num_geo_rec(0),
    num_coll_rec(0), num_isonode_rec(0), num_connnode_rec(0),
    num_edge_rec(0), num_face_rec(0)
{
}

/**
 * Destructor
 */
DSID::~DSID()
{
}

/** Load dsid data from a record
 * @param record Record to read
 */
void DSID::Load(DDFRecord * const record)
{
    int bytes_remaining;
    int bytes_consumed;
    const char *field_data;
    const char *field_name;
    DDFField *field;
    DDFFieldDefn *field_defn;
    DDFSubfieldDefn *subfield_defn;

    int field_count = record->GetFieldCount();

    if (field_count != 3)
        throw std::string("DSID::Load(): bad field_count");

    // Read the DSID field and associated subfields
    field = record->GetField(1);
    if (!field)
        throw std::string("DSID::Load(): bad field");
    field_defn = field->GetFieldDefn();
    if (!field_defn)
        throw std::string("DSID::Load(): bad field defn");
    field_name = field_defn->GetName();
    field_data = field->GetData();
    if (!field_data)
        throw std::string("DSID::Load(): bad field data");
    bytes_remaining = field->GetDataSize();

    // skip record name
    subfield_defn = field_defn->GetSubfield(0);
    if (!subfield_defn)
        throw std::string("DSID::Load(): bad subfield defn");
    subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    for (int i = 1; i < 4; i++) { // skip rcid, expp, intu
        subfield_defn = field_defn->GetSubfield(i);
        if (!subfield_defn)
            throw std::string("DSID::Load(): bad subfield defn");
        subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                            &bytes_consumed);
        bytes_remaining -= bytes_consumed;
        field_data += bytes_consumed;
    }

    subfield_defn = field_defn->GetSubfield(4);
    if (!subfield_defn)
        throw std::string("DSID::Load(): bad subfield defn");
    dataset_name = subfield_defn->ExtractStringData(field_data,
                             bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    subfield_defn = field_defn->GetSubfield(5);
    if (!subfield_defn)
        throw std::string("DSID::Load(): bad subfield defn");
    edition_num = subfield_defn->ExtractStringData(field_data,
                             bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    subfield_defn = field_defn->GetSubfield(6);
    if (!subfield_defn)
        throw std::string("DSID::Load(): bad subfield defn");
    update_num = subfield_defn->ExtractStringData(field_data,
                             bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    subfield_defn = field_defn->GetSubfield(7);
    if (!subfield_defn)
        throw std::string("DSID::Load(): bad subfield defn");
    subfield_defn->ExtractStringData(field_data,
                             bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    // skip update num, update app date
   /* for (int i = 6; i < 8; i++) {
        subfield_defn = field_defn->GetSubfield(i);
        if (!subfield_defn)
            throw std::string("DSID::Load(): bad subfield defn");
        subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                            &bytes_consumed);
        bytes_remaining -= bytes_consumed;
        field_data += bytes_consumed;
    } */

    subfield_defn = field_defn->GetSubfield(8);
    if (!subfield_defn)
        throw std::string("DSID::Load(): bad subfield defn");
    issue_date = subfield_defn->ExtractStringData(field_data,
                             bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    subfield_defn = field_defn->GetSubfield(9);
    if (!subfield_defn)
        throw std::string("DSID::Load(): bad subfield defn");
    s57_edition = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    // skip product spec 
    subfield_defn = field_defn->GetSubfield(10);
    if (!subfield_defn)
        throw std::string("DSID::Load(): bad subfield defn");
    s57_edition = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    // skip product spec descr, edition
    for (int i = 11; i < 13; i++) {
        subfield_defn = field_defn->GetSubfield(i);
        if (!subfield_defn)
            throw std::string("DSID::Load(): bad subfield defn");
        subfield_defn->ExtractStringData(field_data, bytes_remaining,
                                         &bytes_consumed);
        bytes_remaining -= bytes_consumed;
        field_data += bytes_consumed;
    }

    // skip app profile id
    subfield_defn = field_defn->GetSubfield(13);
    if (!subfield_defn)
        throw std::string("DSID::Load(): bad subfield defn");
    subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    subfield_defn = field_defn->GetSubfield(14);
    if (!subfield_defn)
        throw std::string("DSID::Load(): bad subfield defn");
    producing_agency = subfield_defn->ExtractIntData(field_data,
                                bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    // skip dsid comment
/* 
    // Now read the dssi record
    field = record->GetField(2);
    field_defn = field->GetFieldDefn();
    field_name = field_defn->GetName();
    field_data = field->GetData();
    bytes_remaining = field->GetDataSize();

    if (field_name != "DSSI") {
        std::cerr << "DSID::LoadDSIDRecord(): Invalid DSSI field name"
                  << std::endl;
        return -1;
    }

    data_struct = read_int_data(field_defn, field_data, &bytes_remaining,
                                     &bytes_consumed, 0);
    field_data += bytes_consumed;

    attf_lexl = read_int_data(field_defn, field_data, &bytes_remaining,
                                     &bytes_consumed, 1);
    field_data += bytes_consumed;

    natf_lexl = read_int_data(field_defn, field_data, &bytes_remaining,
                                     &bytes_consumed, 2);
    field_data += bytes_consumed;
    
    num_meta_rec = read_int_data(field_defn, field_data, &bytes_remaining,
                                     &bytes_consumed, 3);
    field_data += bytes_consumed;

    num_cart_rec = read_int_data(field_defn, field_data, &bytes_remaining,
                                     &bytes_consumed, 4);
    field_data += bytes_consumed;

    num_geo_rec = read_int_data(field_defn, field_data, &bytes_remaining,
                                     &bytes_consumed, 5);
    field_data += bytes_consumed;

    num_coll_rec = read_int_data(field_defn, field_data, &bytes_remaining,
                                     &bytes_consumed, 6);
    field_data += bytes_consumed;

    num_isonode_rec = read_int_data(field_defn, field_data, &bytes_remaining,
                                     &bytes_consumed, 7);
    field_data += bytes_consumed;

    num_connnode_rec = read_int_data(field_defn, field_data, &bytes_remaining,
                                     &bytes_consumed, 8);
    field_data += bytes_consumed;

    num_edge_rec = read_int_data(field_defn, field_data, &bytes_remaining,
                                     &bytes_consumed, 9);
    field_data += bytes_consumed;

    num_face_rec = read_int_data(field_defn, field_data, &bytes_remaining,
                                     &bytes_consumed, 10);
    field_data += bytes_consumed;

*/
}

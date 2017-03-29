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

#include "feature.h"

/**
 * Constructor
 */
Feature::Feature() :
    object_label(0), record_id(0), group(0), feature_id(0), feature_subid(0)
{
}

/**
 * Destructor
 */
Feature::~Feature()
{
}

/**
 * Load feature data from a record
 * @param record Record to read
 * @return object label 
 */
int Feature::Load(DDFRecord* const record)
{
    int bytes_remaining;
    int bytes_consumed;
    const char *field_data;
    const char *field_name;
    DDFField *field;
    DDFFieldDefn *field_defn;
    DDFSubfieldDefn *subfield_defn;

    int field_count = record->GetFieldCount();
    if (field_count < 2)
        throw std::string("Feature::Load(): Field count < 2");

    // Read the FRID field and associated subfields
    field = record->GetField(1);
    if (!field)
        throw std::string("Feature::Load(): Bad field");
    field_defn = field->GetFieldDefn();
    if (!field_defn)
        throw std::string("Feature::Load(): Bad field defn");
    field_name = field_defn->GetName();
    field_data = field->GetData();
    if (!field_data)
        throw std::string("Feature::Load(): Bad field data");
    bytes_remaining = field->GetDataSize();

    // skip rcnm
    subfield_defn = field_defn->GetSubfield(0);
    if (!subfield_defn)
        throw std::string("Feature::Load(): Bad subfield defn");
    subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    subfield_defn = field_defn->GetSubfield(1);
    if (!subfield_defn)
        throw std::string("Feature::Load(): Bad subfield defn");
    record_id = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    subfield_defn = field_defn->GetSubfield(2);
    if (!subfield_defn)
        throw std::string("Feature::Load(): Bad subfield defn");
    prim = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed); // BSA
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    subfield_defn = field_defn->GetSubfield(3);
    if (!subfield_defn)
        throw std::string("Feature::Load(): Bad subfield defn");
    group = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    subfield_defn = field_defn->GetSubfield(4);
    if (!subfield_defn)
        throw std::string("Feature::Load(): Bad subfield defn");
    object_label = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    // skip record ver, update ins
    for (int i = 5; i < 7; i++) {
        subfield_defn = field_defn->GetSubfield(i);
        if (!subfield_defn)
            throw std::string("Feature::Load(): Bad subfield defn");
        subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                          &bytes_consumed);
        bytes_remaining -= bytes_consumed;
        field_data += bytes_consumed;
    }

    // read the FOID 
    field = record->GetField(2);
    if (!field)
        throw std::string("Feature::Load(): Bad field");
    field_defn = field->GetFieldDefn();
    if (!field_defn)
        throw std::string("Feature::Load(): Bad field defn");
    field_name = field_defn->GetName();
    field_data = field->GetData();
    if (!field_data)
        throw std::string("Feature::Load(): Bad field data");
    bytes_remaining = field->GetDataSize();

    // skip agen
    subfield_defn = field_defn->GetSubfield(0);
    if (!subfield_defn)
        throw std::string("Feature::Load(): Bad subfield defn");
    subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    subfield_defn = field_defn->GetSubfield(1);
    if (!subfield_defn)
        throw std::string("Feature::Load(): Bad subfield defn");
    feature_id = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    subfield_defn = field_defn->GetSubfield(2);
    if (!subfield_defn)
        throw std::string("Feature::Load(): Bad subfield defn");
    feature_subid = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    // Now read the next fields. They could be ATTF<R>,
    // NATF<R>, FFPT<R>, FSPT<R>
    for (int i = 3; i < field_count; i++) {
        field = record->GetField(i);
        if (!field)
            throw std::string("Feature::Load(): Bad field");
        field_defn = field->GetFieldDefn();
        if (!field_defn)
            throw std::string("Feature::Load(): Bad field defn");
        field_data = field->GetData();
        if (!field_data)
            throw std::string("Feature::Load(): Bad field data");
        bytes_remaining = field->GetDataSize();

        field_name = field_defn->GetName();
        if (strncmp(field_name, "ATTF", 4) == 0) {
            int repeat_count = field->GetRepeatCount();
            for (int j = 0; j < repeat_count; j++) {
                attf_t attf;

                subfield_defn = field_defn->GetSubfield(0);
                if (!subfield_defn)
                    throw std::string("Feature::Load(): Bad subfield defn");
                attf.attl = subfield_defn->ExtractIntData(field_data,
                                        bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                subfield_defn = field_defn->GetSubfield(1);
                if (!subfield_defn)
                    throw std::string("Feature::Load(): Bad subfield defn");
                attf.atvl = subfield_defn->ExtractStringData(field_data,
                                        bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                if (attf.attl == 116)
                    name = attf.atvl;

                attfs.push_back(attf);
            }
        }

        else if (strncmp(field_name, "NATF", 4) == 0) {
            int repeat_count = field->GetRepeatCount();
            for (int j = 0; j < repeat_count; j++) {
                attf_t natf;

                subfield_defn = field_defn->GetSubfield(0);
                if (!subfield_defn)
                    throw std::string("Feature::Load(): Bad subfield defn");
                natf.attl = subfield_defn->ExtractIntData(field_data,
                                        bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                subfield_defn = field_defn->GetSubfield(1);
                if (!subfield_defn)
                    throw std::string("Feature::Load(): Bad subfield defn");
                natf.atvl = subfield_defn->ExtractStringData(field_data,
                                        bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                natfs.push_back(natf);
            }
        }

        else if (strncmp(field_name, "FFPT", 4) == 0) {
            int repeat_count = field->GetRepeatCount();
            for (int j = 0; j < repeat_count; j++) {
                ffpt_t ffpt;

                subfield_defn = field_defn->GetSubfield(0);
                if (!subfield_defn)
                    throw std::string("Feature::Load(): Bad subfield defn");
                
                unsigned char *bstr = (unsigned char *)
                       subfield_defn->ExtractStringData(field_data,
                                bytes_remaining, &bytes_consumed);
                int agen_find_fids[3];
                decode_lnam_string(bstr, bytes_consumed, agen_find_fids);
                ffpt.agen = agen_find_fids[0];
                ffpt.find = agen_find_fids[1];
                ffpt.fids = agen_find_fids[2];
                field_data += bytes_consumed;
                bytes_remaining -= bytes_consumed;

                subfield_defn = field_defn->GetSubfield(1);
                if (!subfield_defn)
                    throw std::string("Feature::Load(): Bad subfield defn");
                ffpt.rind = subfield_defn->ExtractIntData(field_data,
                                        bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                subfield_defn = field_defn->GetSubfield(2);
                if (!subfield_defn)
                    throw std::string("Feature::Load(): Bad subfield defn");
                ffpt.comt = subfield_defn->ExtractStringData(field_data,
                                        bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                ffpts.push_back(ffpt);
            }
        }

        else if (strncmp(field_name, "FSPT", 4) == 0) {
            int repeat_count = field->GetRepeatCount();
            for (int j = 0; j < repeat_count; j++) {
                fspt_t fspt;

                subfield_defn = field_defn->GetSubfield(0);
                if (!subfield_defn)
                    throw std::string("Feature::Load(): Bad subfield defn");

                unsigned char *bstr = (unsigned char *)
                       subfield_defn->ExtractStringData(field_data,
                                bytes_remaining, &bytes_consumed);
                int rcnm_rcid[2];
                decode_name_string(bstr, bytes_consumed, rcnm_rcid);
                fspt.rcnm = rcnm_rcid[0];
                fspt.rcid = rcnm_rcid[1];
                field_data += bytes_consumed;
                bytes_remaining -= bytes_consumed;

                ref_record_ids.push_back(fspt.rcid);

                subfield_defn = field_defn->GetSubfield(1);
                if (!subfield_defn)
                    throw std::string("Feature::Load(): Bad subfield defn");
                fspt.ornt = subfield_defn->ExtractIntData(field_data,
                                        bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                subfield_defn = field_defn->GetSubfield(2);
                if (!subfield_defn)
                    throw std::string("Feature::Load(): Bad subfield defn");
                fspt.usag = subfield_defn->ExtractIntData(field_data,
                                        bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;
                
                subfield_defn = field_defn->GetSubfield(3);
                if (!subfield_defn)
                    throw std::string("Feature::Load(): Bad subfield defn");
                fspt.mask = subfield_defn->ExtractIntData(field_data,
                                        bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                fspts.push_back(fspt);
            }
        }
        else if ((strncmp(field_name, "FSPC", 4) == 0) ||
                 (strncmp(field_name, "FFPC", 4) == 0)) {
        } else
            throw std::string("Feature::Load(): Unknown field");
    }
    
    return object_label;
}

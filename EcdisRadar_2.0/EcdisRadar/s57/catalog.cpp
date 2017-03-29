#include "catalog.h"
#include "iso8211lib/iso8211.h"

Catalog::Catalog()
{
}

Catalog::~Catalog()
{
}

void Catalog::Load(FilePath &path)
{
    DDFModule ddf;  // ddf returns 0 on fail
    if (ddf.Open(path.string().c_str()) == 0) {
        ddf.Close();
        throw std::string("Catalog::Load: Failed loading catalog file");
    }

    DDFRecord *record;
    while ((record = ddf.ReadRecord()) != NULL) {
        //signal_progress.emit();
        int field_count = record->GetFieldCount();
        if (field_count != 2) {
            ddf.Close();
            throw std::string("Catalog::Load(): Bad field count");
        }

        DDFField *field = record->GetField(1);
        if (!field) {
            ddf.Close();
            throw std::string( "Catalog::Load(): Error getting field");
        }

        int bytes_remaining;
        int bytes_consumed;
        const char *field_data;
        const char *field_name;
        DDFFieldDefn *field_defn;
        DDFSubfieldDefn *subfield_defn;

        field_defn = field->GetFieldDefn();
        field_name = field_defn->GetName();
        field_data = field->GetData();
        bytes_remaining = field->GetDataSize();

        if (strncmp(field_name, "CATD", 4) != 0) {
            ddf.Close();
            throw std::string("Catalog::Load(): Bad field name");
        }

        if (field_defn->GetSubfieldCount() != 12) {
            ddf.Close();
            throw std::string("Catalog::Load(): Bad subfield count");
        }

        // skip rcnm
        subfield_defn = field_defn->GetSubfield(0);
        subfield_defn->ExtractStringData(field_data, bytes_remaining,
                                            &bytes_consumed);
        bytes_remaining -= bytes_consumed;
        field_data += bytes_consumed;

        subfield_defn = field_defn->GetSubfield(1);
        subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                            &bytes_consumed);
        bytes_remaining -= bytes_consumed;
        field_data += bytes_consumed;

        // filename
        std::string filename; 

        subfield_defn = field_defn->GetSubfield(2);
        filename = subfield_defn->ExtractStringData(field_data, bytes_remaining,
                                            &bytes_consumed);
        bytes_remaining -= bytes_consumed;
        field_data += bytes_consumed;

        FilePath filepath(filename);
        filepath.win32_to_unix();

        std::string extension = filepath.extension();
        if ((extension !=  "TXT") && (filepath.string().size() > 0)) {
            FilePath fullpath = path;
            fullpath = fullpath.branch_path() / filepath;
            if (is_chart(fullpath.string().c_str())) {
                // description
                std::string description;
                subfield_defn = field_defn->GetSubfield(3);
                description = subfield_defn->ExtractStringData(field_data,
                        bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                // skip stuff to get to long/lat range
                for (int i = 4; i < 6; i++) {
                    subfield_defn = field_defn->GetSubfield(i);
                    subfield_defn->ExtractStringData(field_data,
                            bytes_remaining, &bytes_consumed);
                    bytes_remaining -= bytes_consumed;
                    field_data += bytes_consumed;
                }

                LongLat slat, wlon, nlat, elon;

                subfield_defn = field_defn->GetSubfield(6);
                slat = (subfield_defn->ExtractFloatData(field_data,
                        bytes_remaining, &bytes_consumed) * 60);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                subfield_defn = field_defn->GetSubfield(7);
                wlon = (subfield_defn->ExtractFloatData(field_data,
                        bytes_remaining, &bytes_consumed) * 60);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                subfield_defn = field_defn->GetSubfield(8);
                nlat = (subfield_defn->ExtractFloatData(field_data,
                        bytes_remaining, &bytes_consumed) * 60);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                subfield_defn = field_defn->GetSubfield(9);
                elon = (subfield_defn->ExtractFloatData(field_data,
                        bytes_remaining, &bytes_consumed) * 60);
                bytes_remaining -= bytes_consumed;

                catalog_chart_t chart;
                chart.filename = filepath.string();
                chart.description = description;
                chart.slat = slat; chart.wlon = wlon;
                chart.nlat = nlat; chart.elon = elon;

                charts.push_back(chart);
            }
        }
    }
    ddf.Close();
}

bool Catalog::is_chart(const char *filename)
{
    DDFModule ddf;
    if (ddf.Open(filename) == 0) { // DDFModule returns 0 on fail
        ddf.Close();
        return false;
    }
    DDFRecord *record;

    record = ddf.ReadRecord();
    if (!record) {
        ddf.Close();
        return false;
    }

    int field_count = record->GetFieldCount();
    if (field_count != 3) {
        ddf.Close();
        return false;
    }
    DDFField *field = record->GetField(1);
    if (!field) {
        ddf.Close();
        return false;
    }

    const char *field_name;
    DDFFieldDefn *field_defn;

    field_defn = field->GetFieldDefn();
    field_name = field_defn->GetName();

    if (!field_name) {
        ddf.Close();
        return false;
    }

    if (!field_defn) {
        ddf.Close();
        return false;
    }

    if (strncmp(field_name, "DSID", 4) != 0) {
        ddf.Close();
        return false;
    }

    if (field_defn->GetSubfieldCount() != 16) {
        ddf.Close();
        return false;
    } 

    ddf.Close();
    return true;
}

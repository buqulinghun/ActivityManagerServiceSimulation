/********************************************************************
 *日期: 2016-01-12
 *作者: 王名孝
 *作用: 加载海图原始文件，生成各个物标的详细信息，通过查找lookup表生成所需的绘图信息并保存，生成SENC文件
 *修改:
 ********************************************************************/

#include <iostream>
#include <math.h>
#include <stdio.h>
#include <QTime>
#include <QDebug>
#include <QStringList>
#include <QPolygonF>

#include "s57chart.h"
#include "s57/s57.h"



extern Configuration *configuration;




/**
 * Constructor
 */
S57Chart::S57Chart() :
    dsid_loaded(false), dspm_loaded(false), num_sg2ds(0)
{
}

/**
 * Destructor
 */
S57Chart::~S57Chart()
{ 
}

/**
 * Load chart data
 * @param fpath The filepath
 */
void S57Chart::Load(const FilePath &fpath)
{
    QTime tt;
    tt.start();

    const char *filename = fpath.string().c_str();
    DDFModule ddf;
    if (ddf.Open(filename) == 0) { // DDFModule returns 0 on fail
        ddf.Close();
        throw std::string("S57Chart::S57Load(): Failed loading file");
    }

    try {
        DDFRecord *record;
        while ((record = ddf.ReadRecord()) != NULL) {
            //signal_progress.emit();
            DDFField *field = record->GetField(1);
            if (!field) {
                ddf.Close();
                throw std::string("S57Chart::Load(): Error getting field");
            }
            DDFFieldDefn *field_defn = field->GetFieldDefn();
            const char *field_name = field_defn->GetName();
            
            if (strncmp(field_name, "DSID", 4) == 0) {
                ReadDSIDRecord(record);
            } else if (strncmp(field_name, "DSPM", 4) == 0) {
                ReadDSPMRecord(record);
            } else if (strncmp(field_name, "CATD", 4) == 0) {
            } else if (strncmp(field_name, "DDDF", 4) == 0) {
            } else if (strncmp(field_name, "DDSI", 4) == 0) {
            } else if (strncmp(field_name, "FRID", 4) == 0) {
                ReadFeatureRecord(record);
            } else if (strncmp(field_name, "VRID", 4) == 0) {
                ReadVectorRecord(record);
            } else {
                non_fatal_error = true;
                std::cerr << "S57Chart::Load: Unknown leading record name"
                          << std::endl;
            }
        }
    } catch (std::string err) {
        ddf.Close();
        throw err;
    }

    ddf.Close();

    // we've read the file.
    if (!dsid_loaded)
        throw std::string("S57Chart::Load(): DSID not loaded");

    if (!dspm_loaded)
        throw std::string("S57Chart::Load(): DSPM not loaded");

 //   SetupEdgeVectors();      //组合线物标

    NormalizeCoordinates();   //得到经纬度

  //  SetupPointObjects();      //建立各个物标，包含绘图信息
  //  SetupLineObjects();       //将线段中的引用对象保存起来，后面确定安全线时需要进行筛选
  //  SetupAreaObjects();      //也需要将引用对象保存下来

    //此处将需要的空间物标的被引用信息保存到相应的物标中
 //   saveQuoteRecord();

    filepath = fpath;

    if (non_fatal_error) {
        /*signal_non_fatal_error.emit("S57Chart::Load: "
                                    "there was an error loading the chart, "
                                    "it will be displayed but there may be "
                                    "errors or omissions.");*/
    }

    qDebug() << "time of parse Nautical chart is" << tt.elapsed();
}


#ifdef USE
void S57Chart::parseLookupTable(const LookupTable &findItem, LineObject &line_object, const std::vector<Feature>::iterator &lit)
{
 /*   line_object.dispCategory = findItem.dispcategory;   //显示类别
    line_object.recordId = lit->record_id;
    line_object.dispPriority = findItem.priority;
    line_object.overRadar = findItem.overRadar;
    line_object.viewGroup = findItem.viewGroup;
    const QStringList instruction = findItem.symInstruction.split(";");   //符号化指令
    for(int i=0; i<instruction.size(); i++) {
        const QString context = instruction.at(i);
        const QString style = context.left(2);
        if(style == "LS") {
            line_object.paintType |= 0x01;
            const QString type = context.mid(3,4);
            if(type == "SOLD")
                line_object.penType = 1;
            else if(type == "DASH")
                line_object.penType = 2;
            else if(type == "DOTT")
                line_object.penType = 3;
            else
                qDebug() << "error pen type while loading Line-LS!";
            line_object.penWidth = context.mid(8,1).toInt();
            line_object.penColor = context.mid(10,5);
        }else if(style == "LC") {
            line_object.paintType |= 0x02;
            line_object.complexName = context.mid(3,8);
        }else if(style == "SY"){
            line_object.paintType |= 0x04;  //点物标
            line_object.symbolName = context.mid(3,8);
            if((context.size() > 12)) {
                if(context.mid(12, 6) == "ORIENT"){
                    line_object.paintType |= 0x80;                    //SY旋转标志

                    std::vector<attf_t>::iterator attf_it;
                    std::vector<attf_t>::iterator attf_end = lit->attfs.end();
                    for(attf_it = lit->attfs.begin(); attf_it != attf_end; ++ attf_it){
                        if(attf_it->attl == 117) {   //旋转方向
                            line_object.orientValue = atof(attf_it->atvl.c_str());
                            break;
                        }
                    }
                }else{
                    qDebug() << "symbol lines error while load SY and the last flag is: "<< context.mid(12, 6);
                }
            }
        }else if(style == "CS"){
            line_object.paintType |= 0x08;   //条件物标
            line_object.conditionalName = context.mid(3,8);
            line_object.attfs = lit->attfs;
        }else if(style == "TX"){   //字符数字显示
            const QString string = context.mid(3, context.size() - 4);
            const QStringList str = string.split(",");
            const QString name = str.at(0);                                 //显示物标属性
            int attr = configuration->attribute_name_num_map[name].code;    //属性编码，获取对应属性的值,有些小写的属性没有，不知道是怎么回事

            TextString text;
            std::vector<attf_t>::iterator attf_it;
            std::vector<attf_t>::iterator attf_end = lit->attfs.end();
            for(attf_it = lit->attfs.begin(); attf_it != attf_end; ++ attf_it){
                if(attf_it->attl == attr) {
                    text.textString = QString(attf_it->atvl.c_str());
                    break;
                }
             }

            text.textHJust = str.at(1).toInt();
            text.textVJust = str.at(2).toInt();
            text.textSpace = str.at(3).toInt();
            text.textWeight = str.at(4).mid(2, 1).toInt();
            text.textSize = str.at(4).mid(4, 2).toInt();
            text.textXOffset = str.at(5).toInt() + 8;
            text.textYOffset = str.at(6).toInt() + 8;
            text.textDispGroup = str.at(8).toInt();
            line_object.textString.append(text);
            line_object.paintText = true;

        }else if(style == "TE") {      //数字显示
            const QString string = context.mid(3, context.size() - 4);
            const QStringList str = string.split(",");
            const QString format = str.at(0).mid(1, str.at(0).size() - 2);
            const QString name = str.at(1).mid(1, str.at(1).size() - 2);

            int attr = configuration->attribute_name_num_map[name].code;
            std::string value;
            std::vector<attf_t>::iterator attf_it;
            std::vector<attf_t>::iterator attf_end = lit->attfs.end();
            for(attf_it = lit->attfs.begin(); attf_it != attf_end; ++ attf_it){
                if(attf_it->attl == attr) {    //获取对应属性的值
                    value = attf_it->atvl;
                    break;
                }
            }

            TextString text;
            const char* format1 = format.toStdString().c_str();   //这个到底是输出字符串还是对应的标号？？？？？？
            text.textString.sprintf(format1, value.c_str());
            text.textHJust = str.at(2).toInt();
            text.textVJust = str.at(3).toInt();
            text.textSpace = str.at(4).toInt();
            text.textWeight = str.at(5).mid(2, 1).toInt();
            text.textSize = str.at(5).mid(4, 2).toInt();
            text.textXOffset = str.at(6).toInt() + 8;
            text.textYOffset = str.at(7).toInt() + 8;
            text.textDispGroup = str.at(9).toInt();
            line_object.textString.append(text);
            line_object.paintText = true;
        }
    }
    */
}

#endif


/**
 * Read dsid data from a record
 * @param record The record to read
 */
void S57Chart::ReadDSIDRecord(DDFRecord *record)
{
    if (dsid_loaded) {
        throw std::string("S57Chart::ReadDSIDRecord(): DSID already loaded");
        return;
    }

    dsid.Load(record);

    dsid_loaded = true;
}

/**
 * Read dspm data from a record
 * @param record The record to read
 */
void S57Chart::ReadDSPMRecord(DDFRecord *record)
{
    if (dspm_loaded) {
        throw std::string("S57Chart::ReadDSPMRecord(): DSPM already loaded");
        return;
    }

    dspm.Load(record); 

    dspm_loaded = true;
}

//*****************************************************************************
//
// Feature record loading
//
//*****************************************************************************

/**
 * Read feature data from a record.
 * @param record The record to read
 */
void S57Chart::ReadFeatureRecord(DDFRecord *record)
{
    DDFField *field = record->GetField(1);
    if (!field)
        throw std::string("S57Chart::ReadFeatureRecord: Bad field");
            
    DDFFieldDefn *field_defn = field->GetFieldDefn();
    if (!field_defn)
        throw std::string("S57Chart::ReadFeatureRecord: Bad field defn");

    const char *field_data = field->GetData();
    if (!field_data)
        throw std::string("S57Chart::ReadFeatureRecord: Bad field data");

    int bytes_remaining = field->GetDataSize();
    int bytes_consumed = 0;

    DDFSubfieldDefn *subfield_defn;
    subfield_defn = field_defn->GetSubfield(2);
    if (!subfield_defn)
        throw std::string("S57Chart::ReadFeatureRecord: Bad subfield defn");

    // skip rcnm, rcid
    for (int i = 0; i < 2; i++) {
        subfield_defn = field_defn->GetSubfield(i);
        if (!subfield_defn)
            throw std::string("S57Chart::ReadFeatureRecord: Bad subfield defn");
        subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                            &bytes_consumed);
        bytes_remaining -= bytes_consumed;
        field_data += bytes_consumed;
    }

    subfield_defn = field_defn->GetSubfield(2);
    if (!subfield_defn)
        throw std::string("S57Chart::ReadFeatureRecord: Bad subfield defn");
    int prim = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                            &bytes_consumed);

    switch (prim)
    {
        case 1:
            LoadPointFeature(record);
            break;
        case 2:
            LoadLineFeature(record);
            break;
        case 3:
            LoadAreaFeature(record);
            break;
        case 255:
            break;
        default:
            throw std::string("S57Chart::ReadFeatureRecord(): Unknown prim");
    }
}

/**
 * Read point feature data from a record
 * @param record The record to read
 */
void S57Chart::LoadPointFeature(DDFRecord *record)
{
    Feature point_feature;
    int result = point_feature.Load(record);
    
    point_features_map[result].push_back(point_feature);
}

/**
 * Read line feature data from a record
 * @param record The record to read
 */
void S57Chart::LoadLineFeature(DDFRecord *record)
{
    Feature line_feature;
    int result = line_feature.Load(record);

    line_features_map[result].push_back(line_feature);
}

/**
 * Read area feature data from a record
 * @param record The record to read
 */
void S57Chart::LoadAreaFeature(DDFRecord *record)
{
    Feature area_feature;
    int result = area_feature.Load(record);

    area_features_map[result].push_back(area_feature);
}

//*****************************************************************************
//
// Vector record loading
//
//*****************************************************************************

/**
 * Read vector data from  a record
 * @param record The record to read
 */
void S57Chart::ReadVectorRecord(DDFRecord *record)
{
    DDFField *field = record->GetField(1);
    if (!field)
        throw std::string("S57Chart::ReadVectorRecord: Bad field");
    DDFFieldDefn *field_defn = field->GetFieldDefn();
    if (!field_defn)
        throw std::string("S57Chart::ReadVectorRecord: Bad field defn");
    const char *field_data = field->GetData();
    if (!field_data)
        throw std::string("S57Chart::ReadVectorRecord: Bad field data");
    int bytes_remaining = field->GetDataSize();
    int bytes_consumed = 0;

    DDFSubfieldDefn *subfield_defn;

    subfield_defn = field_defn->GetSubfield(0);
    if (!subfield_defn)
        throw std::string("S57Chart::ReadVectorRecord: Bad subfield defn");
    int rcnm = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                            &bytes_consumed);

    switch (rcnm)
    {
        case 110:
            LoadIsolatedNodeVectorRecord(record);//line_features_map
            break;
        case 120:
            LoadConnectedNodeVectorRecord(record);//connected_node_vectors_map
            break;
        case 130:
            LoadEdgeVectorRecord(record);//edge_vectors_map
            break;
        case 140:
            break;
        default:
            throw std::string("S57Chart::ReadVectorRecord(): Unknown rcnm");
    }
}

/**
 * Read isolated node vector data from a record
 * @param record The record to read
 */
void S57Chart::LoadIsolatedNodeVectorRecord(DDFRecord *record)
{
    IsolatedNodeVector isolated_node_vector;

    sg2d_t *sg2d = NULL;
    int bytes_remaining;
    int bytes_consumed;
    const char *field_data;
    const char *field_name;
    DDFField *field;
    DDFFieldDefn *field_defn;
    DDFSubfieldDefn *subfield_defn;

    int field_count = record->GetFieldCount();
    Feature line_feature;
    int result = line_feature.Load(record);

    line_features_map[result].push_back(line_feature);
}

/**
    // Read the VRID field
    field = record->GetField(1);
    if (!field)
        throw std::string("S57Chart::LoadIsolatedNodeVectorRecord: bad field");
    field_defn = field->GetFieldDefn();
    if (!field_defn) {
        throw std::string("S57Chart::LoadIsolatedNodeVectorRecord: "
                          "bad field defn");
    }
    field_name = field_defn->GetName();
    field_data = field->GetData();
    if (!field_data) {
        throw std::string("S57Chart::LoadIsolatedNodeVectorRecord: "
                          "bad field data");
    }
    bytes_remaining = field->GetDataSize();

    // skip record name
    subfield_defn = field_defn->GetSubfield(0);
    if (!subfield_defn) {
        throw std::string("S57Chart::LoadIsolatedNodeVectorRecord: "
                          "bad subfield defn");
    }
    subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    subfield_defn = field_defn->GetSubfield(1);
    if (!subfield_defn) {
        throw std::string("S57Chart::LoadIsolatedNodeVectorRecord: "
                          "bad subfield defn");
    }
    int record_id = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    // skip record version, update instruction

    // Now read the next fields
    for (int i = 2; i < field_count; i++) {
        field = record->GetField(i);
        if (!field) {
            throw std::string("S57Chart::LoadIsolatedNodeVectorRecord: "
                              "bad field");
        }

        field_defn = field->GetFieldDefn();
        if (!field_defn) {
            throw std::string("S57Chart::LoadIsolatedNodeVectorRecord: "
                              "bad field defn");
        }

        field_data = field->GetData();
        if (!field_data) {
            throw std::string("S57Chart::LoadIsolatedNodeVectorRecord: "
                              "bad field data");
        }

        bytes_remaining = field->GetDataSize();

        field_name = field_defn->GetName();

        if (strncmp(field_name, "ATTV", 4) == 0) {
            int repeat_count = field->GetRepeatCount();

            for (int j = 0; j < repeat_count; j++) {
                attv_t attv;

                subfield_defn = field_defn->GetSubfield(0);
                if (!subfield_defn) {
                    throw std::string("S57Chart::LoadIsolatedNodeVectorRecord: "
                                      "bad subfield defn");
                }
                
                attv.attl = subfield_defn->ExtractIntData(field_data,
                                        bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                subfield_defn = field_defn->GetSubfield(1);
                if (!subfield_defn) {
                    throw std::string("S57Chart::LoadIsolatedNodeVectorRecord: "
                                      "bad subfield defn");
                }
                
                attv.atvl = subfield_defn->ExtractStringData(field_data,
                                        bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                isolated_node_vector.attvs.push_back(attv); 
            }
        }

        else if (strncmp(field_name, "SG2D", 4) == 0) {
            int repeat_count = field->GetRepeatCount();
            if (repeat_count > 1) {
                throw std::string("S57Chart::LoadIsolatedNodeVectorRecord(): "
                                  "More than one sg2d");
            }

            subfield_defn = field_defn->GetSubfield(0);
            if (!subfield_defn) {
                throw std::string("S57Chart::LoadIsolatedNodeVectorRecord: "
                                  "bad subfield defn");
            }
            
            int ycoo = subfield_defn->ExtractIntData(field_data,
                                bytes_remaining, &bytes_consumed);
            bytes_remaining -= bytes_consumed;
            field_data += bytes_consumed;
            
            subfield_defn = field_defn->GetSubfield(1);
            if (!subfield_defn) {
                throw std::string("S57Chart::LoadIsolatedNodeVectorRecord: "
                                  "bad subfield defn");
            }
           
            int xcoo = subfield_defn->ExtractIntData(field_data,
                                bytes_remaining, &bytes_consumed);
            bytes_remaining -= bytes_consumed;
            field_data += bytes_consumed;

            if (sg2d) {
                throw std::string("S57Chart::LoadIsolatedNodeVectorRecord(): "
                                  "More than one sg2d loaded");
            }
            // note that we don't worry about deleting this; when we return
            // the chart is deleted and since this is in the sg2ds vector
            // it is also.
            sg2d_t sg2d;
            sg2d.long_lat[1] = (double)ycoo;
            sg2d.long_lat[0] = (double)xcoo;
            sg2ds.push_back(sg2d);
            isolated_node_vector.sg2d_index = num_sg2ds;
            num_sg2ds++;
        }

        else if (strncmp(field_name, "SG3D", 4) == 0) {
            int repeat_count = field->GetRepeatCount();

            for (int j = 0; j < repeat_count; j++) {
                sg3d_t sg3d;

                subfield_defn = field_defn->GetSubfield(0);
                if (!subfield_defn) {
                    throw std::string("S57Chart::LoadIsolatedNodeVectorRecord: "
                                      "bad subfield defn");
                }
                
                int ycoo = subfield_defn->ExtractIntData(field_data,
                                    bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;
                
                subfield_defn = field_defn->GetSubfield(1);
                if (!subfield_defn) {
                    throw std::string("S57Chart::LoadIsolatedNodeVectorRecord: "
                                      "bad subfield defn");
                }
               
                int xcoo = subfield_defn->ExtractIntData(field_data,
                                    bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                sg3d.long_lat[1] =  ((double)ycoo);
                sg3d.long_lat[0] =  ((double)xcoo);

                subfield_defn = field_defn->GetSubfield(2);
                if (!subfield_defn) {
                    throw std::string("S57Chart::LoadIsolatedNodeVectorRecord: "
                                      "bad subfield defn");
                }
               
                sg3d.depth = subfield_defn->ExtractIntData(field_data,
                                    bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                isolated_node_vector.sg3ds.push_back(sg3d);
            }
        }
        else {
            throw std::string("S57Chart::LoadIsolatedNodeVectorRecord(): "
                              "unknown field name");
        }
    }

    isolated_node_vectors_map[record_id] = isolated_node_vector;
}

/**
 * Read connected node vector data from a record
 * @param record The record to read
 */
void S57Chart::LoadConnectedNodeVectorRecord(DDFRecord *record)
{
    ConnectedNodeVector connected_node_vector;
    sg2d_t *sg2d = NULL;

    int bytes_remaining;
    int bytes_consumed;
    const char *field_data;
    const char *field_name;
    DDFField *field;
    DDFFieldDefn *field_defn;
    DDFSubfieldDefn *subfield_defn;

    int field_count = record->GetFieldCount();

    // Read the VRID field
    field = record->GetField(1);
    if (!field)
        throw std::string("S57Chart::LoadConnectedNodeVector: bad field");

    field_defn = field->GetFieldDefn();
    if (!field_defn)
        throw std::string("S57Chart::LoadConnectedNodeVector: bad field defn");

    field_name = field_defn->GetName();
    field_data = field->GetData();
    if (!field_data)
        throw std::string("S57Chart::LoadConnectedNodeVector: bad field data");

    bytes_remaining = field->GetDataSize();

    // skip record name
    subfield_defn = field_defn->GetSubfield(0);
    if (!subfield_defn) {
        throw std::string("S57Chart::LoadConnectedNodeVector: "
                          "bad subfield defn");
    }

    subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    subfield_defn = field_defn->GetSubfield(1);
    if (!subfield_defn) {
        throw std::string("S57Chart::LoadConnectedNodeVector: "
                          "bad subfield defn");
    }

    int record_id = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    // skip record version, update instruction

    // Now read the next fields
    for (int i = 2; i < field_count; i++) {
        field = record->GetField(i);
        if (!field)
            throw std::string("S57Chart::LoadConnectedNodeVector: bad field");
        
        field_defn = field->GetFieldDefn();
        if (!field_defn) {
            throw std::string("S57Chart::LoadConnectedNodeVector: "
                              "bad field defn");
        }
        
        field_data = field->GetData();
        if (!field_data) {
            throw std::string("S57Chart::LoadConnectedNodeVector: "
                              "bad field data");
        }
        
        bytes_remaining = field->GetDataSize();

        field_name = field_defn->GetName();

        if (strncmp(field_name, "ATTV", 4) == 0) {
            int repeat_count = field->GetRepeatCount();

            for (int j = 0; j < repeat_count; j++) {
                attv_t attv;

                subfield_defn = field_defn->GetSubfield(0);
                if (!subfield_defn) {
                    throw std::string("S57Chart::LoadConnectedNodeVector: "
                                      "bad subfield defn");
                }
                
                attv.attl = subfield_defn->ExtractIntData(field_data,
                                        bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                subfield_defn = field_defn->GetSubfield(1);
                if (!subfield_defn) {
                    throw std::string("S57Chart::LoadConnectedNodeVector: "
                                      "bad subfield defn");
                }
                
                attv.atvl = subfield_defn->ExtractStringData(field_data,
                                        bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                connected_node_vector.attvs.push_back(attv); 
            }
        }

        else if (strncmp(field_name, "SG2D", 4) == 0) {
            int repeat_count = field->GetRepeatCount();
            if (repeat_count > 1) {
                throw std::string("S57Chart::LoadConnectedNodeVectorRecord(): "
                                  "sg2d repeat count > 1");
            }

            subfield_defn = field_defn->GetSubfield(0);
            if (!subfield_defn) {
                throw std::string("S57Chart::LoadConnectedNodeVectorRecord(): "
                                  "bad subfield defn");
            }
            
            int ycoo = subfield_defn->ExtractIntData(field_data,
                                    bytes_remaining, &bytes_consumed);
            bytes_remaining -= bytes_consumed;
            field_data += bytes_consumed;

            subfield_defn = field_defn->GetSubfield(1);
            if (!subfield_defn) {
                throw std::string("S57Chart::LoadConnectedNodeVectorRecord(): "
                                  "bad subfield defn");
            }
            
            int xcoo = subfield_defn->ExtractIntData(field_data,
                                    bytes_remaining, &bytes_consumed);
            bytes_remaining -= bytes_consumed;
            field_data += bytes_consumed;

            if (sg2d) {
                throw std::string ("S57Chart::LoadConnectedNodeVectorRecord(): "
                                   "More than one sg2d loaded");
            }

            // note that we don't worry about deleting this; when we return
            // the chart is deleted and since this is in the sg2ds vector
            // it is also.
            sg2d_t sg2d;
            sg2d.long_lat[1] = (double)ycoo;
            sg2d.long_lat[0] = (double)xcoo;
            sg2ds.push_back(sg2d);
            connected_node_vector.sg2d_index = num_sg2ds;
            num_sg2ds++;
        } 
        
        else {
            throw std::string("S57Chart::LoadConnectedNodeVectorRecord(): "
                              "Unknown field");
        }
    }

    connected_node_vectors_map[record_id] = connected_node_vector;
}

/**
 * Read edge node vector data from a record
 * @param record The record to read
 */
void S57Chart::LoadEdgeVectorRecord(DDFRecord *record)
{
    EdgeVector edge_vector;

    int bytes_remaining;
    int bytes_consumed;
    const char *field_data;
    const char *field_name;
    DDFField *field;
    DDFFieldDefn *field_defn;
    DDFSubfieldDefn *subfield_defn;

    int field_count = record->GetFieldCount();

    // Read the VRID field
    field = record->GetField(1);
    if (!field)
        throw std::string("S57Chart::LoadEdgeVectorRecord: bad field");

    field_defn = field->GetFieldDefn();
    if (!field_defn)
        throw std::string("S57Chart::LoadEdgeVectorRecord: bad field defn");

    field_name = field_defn->GetName();
    field_data = field->GetData();
    if (!field_data)
        throw std::string("S57Chart::LoadEdgeVectorRecord: bad field data");

    bytes_remaining = field->GetDataSize();

    // skip record name
    subfield_defn = field_defn->GetSubfield(0);
    if (!subfield_defn)
        throw std::string("S57Chart::LoadEdgeVectorRecord: bad subfield defn");

    subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    subfield_defn = field_defn->GetSubfield(1);
    if (!subfield_defn)
        throw std::string("S57Chart::LoadEdgeVectorRecord: bad subfield defn");

    int record_id = subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                        &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    // skip record version, update instruction

    for (int i = 2; i < field_count; i++) {
        field = record->GetField(i);
        if (!field)
            throw std::string("S57Chart::LoadEdgeVectorRecord: bad field");

        field_defn = field->GetFieldDefn();
        if (!field_defn)
            throw std::string("S57Chart::LoadEdgeVectorRecord: bad field defn");

        bytes_remaining = field->GetDataSize();

        field_name = field_defn->GetName();

        if (strncmp(field_name, "VRPT", 4) == 0) {
            field_data = field->GetData();
            if (!field_data) {
                throw std::string("S57Chart::LoadEdgeVectorRecord: "
                                  "bad field data");
            }

            int repeat_count = field->GetRepeatCount();

            for (int j = 0; j < repeat_count; j++) {
                subfield_defn = field_defn->GetSubfield(0);
                if (!subfield_defn) {
                    throw std::string("S57Chart::LoadEdgeVectorRecord: "
                                      "bad subfield defn");
                }

                unsigned char *bstr;
                bstr = (unsigned char *)
                        subfield_defn->ExtractStringData(field_data,
                                bytes_remaining, &bytes_consumed);

                int rcnm_rcid[2];
                decode_name_string(bstr, bytes_consumed, rcnm_rcid);

                int rcid = rcnm_rcid[1];

                field_data += bytes_consumed;
                bytes_remaining -= bytes_consumed;

                subfield_defn = field_defn->GetSubfield(i);
                if (!subfield_defn) {
                    throw std::string("S57Chart::LoadEdgeVectorRecord: "
                                      "bad subfield defn");
                }
                edge_vector.ornt = subfield_defn->ExtractIntData(field_data,
                            bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                subfield_defn = field_defn->GetSubfield(i);
                if (!subfield_defn) {
                    throw std::string("S57Chart::LoadEdgeVectorRecord: "
                                      "bad subfield defn");
                }
                edge_vector.usag = subfield_defn->ExtractIntData(field_data,
                                bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                subfield_defn = field_defn->GetSubfield(3);
                if (!subfield_defn) {
                    throw std::string("S57Chart::LoadEdgeVectorRecord: "
                                      "bad subfield defn");
                }

                int topi = subfield_defn->ExtractIntData(field_data,
                                     bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;
                if (topi == 1)
                {
                    edge_vector.beg_node = rcid;
                    qDebug()<<"beg:"<<rcid;
                }
                else if (topi == 2)
                {
                    edge_vector.end_node = rcid;
                    qDebug()<<"end:"<<rcid;
                }
                else {
                    throw std::string("S57Chart::LoadEdgeVectorRecord: "
                                      "Bad TOPI number");
                }

                // skip mask
                subfield_defn = field_defn->GetSubfield(4);
                if (!subfield_defn) {
                    throw std::string("S57Chart::LoadEdgeVectorRecord: "
                                      "bad subfield defn");
                }
                subfield_defn->ExtractIntData(field_data, bytes_remaining,
                                                    &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;
            }
        } 
         
        else if (strncmp(field_name, "ATTV", 4) == 0) {
            int repeat_count = field->GetRepeatCount();

            for (int j = 0; j < repeat_count; j++) {
                attv_t attv;

                subfield_defn = field_defn->GetSubfield(0);
                if (!subfield_defn) {
                    throw std::string("S57Chart::LoadEdgeVectorRecord: "
                                      "bad subfield defn");
                }
                attv.attl = subfield_defn->ExtractIntData(field_data,
                                        bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                subfield_defn = field_defn->GetSubfield(1);
                if (!subfield_defn) {
                    throw std::string("S57Chart::LoadEdgeVectorRecord: "
                                      "bad subfield defn");
                }
                attv.atvl = subfield_defn->ExtractStringData(field_data,
                                        bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                if(attv.attl == 402)
                    edge_vector.attvs.push_back(attv);
            }    //所有的属性都为1,对于后面没有用，不需要，空间物标只需要402这个属性，对于点物标有
        }

        else if (strncmp(field_name, "SG2D", 4) == 0) {
            int repeat_count = field->GetRepeatCount();

            field_data = field->GetData();
            if (!field_data) {
                throw std::string("S57Chart::LoadEdgeVectorRecord: "
                                  "bad field data");
            }

            for (int j = 0; j < repeat_count; j++) {
                subfield_defn = field_defn->GetSubfield(0);
                if (!subfield_defn) {
                    throw std::string("S57Chart::LoadEdgeVectorRecord: "
                                      "bad subfield defn");
                }

                int ycoo = subfield_defn->ExtractIntData(field_data,
                                        bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                subfield_defn = field_defn->GetSubfield(1);
                if (!subfield_defn) {
                    throw std::string("S57Chart::LoadEdgeVectorRecord: "
                                      "bad subfield defn");
                }

                int xcoo = subfield_defn->ExtractIntData(field_data,
                                        bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                sg2d_t sg2d;
                sg2d.long_lat[1] = (double)ycoo;
                sg2d.long_lat[0] = (double)xcoo;
                sg2ds.push_back(sg2d);
                edge_vector.sg2d_indices.push_back(num_sg2ds);
                num_sg2ds++;
            }
        } else 
            throw std::string("S57Chart::LoadEdgeVectorRecord: Unknown field");
    }

    edge_vectors_map[record_id] = edge_vector;
}



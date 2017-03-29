/********************************************************************
 *日期: 2016-01-12
 *作者: 王名孝
 *作用: 基本结构体/枚举定义
 *修改:
 ********************************************************************/
#ifndef S57_H_
#define S57_H_

#define MINIMUM(a,b)      ((a<b) ? a : b)

#include <string>
#include <QString>
#include "longlat.h"

enum feature_type_e {
    POINT = 1, LINE, AREA
};

//
// feature draw properties
//

/**
 * Point object drawing properties
 */
struct point_drawprop_t
{
    float rgb[3];
    float size;
};

/**
 * Line object drawing properties
 */
struct line_drawprop_t
{
    float rgb[3];
    float size;
};

//
// feature record stuff
//

/**
 * attf
 */
struct attf_t
{
    int attl;
    std::string atvl;
};

/**
 * ffpt
 */
struct ffpt_t
{
    int agen;
    int find;
    int fids;
    int rind;
    std::string comt;
};

/**
 * fspt
 */
struct fspt_t
{
    int rcnm;
    int rcid;
    int ornt;
    int usag;
    int mask;
};

//
// vector record stuff
//

/**
 * attv
 */
struct attv_t
{
    int attl;
    std::string atvl;
};

/**
 * vrpc
 */
struct vrpc_t
{
    int vpui;
    int vpix;
    int nvpt;
};

/**
 * vrpt
 */
struct vrpt_t
{
    int rcnm;
    int rcid;
    int ornt;
    int usag;
    int topi;
    int mask;
};

/**
 * sg3d
 */
struct sg3d_t
{
    double long_lat[2];
    double depth;
};

enum object_classes_e {
    // geo object classes
    ADMARE = 1,
    AIRARE, ACHBRT, ACHARE, BCNCAR, BCNISD, BCNLAT, BCNSAW, BCNSPP,
    BERTHS, BRIDGE, BUISGL, BUAARE, BOYCAR, BOYINB, BOYISD, BOYLAT,
    BOYSAW, BOYSPP, CBLARE, CBLOHD, CBLSUB, CANALS, CANBNK, CTSARE,
    CAUSWY, CTNARE, CHKPNT, CGUSTA, COALNE, CONZNE, COSARE, CTRPNT,
    CONVYR, CRANES, CURENT, CUSZNE, DAMCON, DAYMAR, DWRTCL, DWRTPT,
    DEPARE, DEPCNT, DISMAR, DOCARE, DRGARE, DRYDOC, DMPGRD, DYKCON,
    EXEZNE, FAIRWY, FNCLNE, FERYRT, FSHZNE, FSHFAC, FSHGRD, FLODOC,
    FOGSIG, FORSTC, FRPARE, GATCON, GRIDRN, HRBARE, HRBFAC, HULKES,
    ICEARE, ICNARE, ISTZNE, LAKARE, LAKSHR, LNDARE, LNDELV, LNDRGN,
    LNDMRK, LIGHTS, LITFLT, LITVES, LOCMAG, LOKBSN, LOGPON, MAGVAR,
    MARCUL, MIPARE, MORFAC, NAVLNE, OBSTRN, OFSPLF, OSPARE, OILBAR,
    PILPNT, PILBOP, PIPARE, PIPOHD, PIPSOL, PONTON, PRCARE, PRDARE,
    PYLONS, RADLNE, RADRNG, RADRFL, RADSTA, RTPBCN, RDOCAL, RDOSTA,
    RAILWY, RAPIDS, RCRTCL, RECTRC, RCTLPT, RSCSTA, RESARE, RETRFL,
    RIVERS, RIVBNK, ROADWY, RUNWAY, SNDWAV, SEAARE, SPLARE, SBDARE,
    SLCONS, SISTAT, SISTAW, SILTNK, SLOTOP, SLOGRD, SMCFAC, SOUNDG,
    SPRING, SQUARE, STSLNE, SUBTLN, SWPARE, TESARE,
    TS_PRH, TS_PNH, TS_PAD, TS_TIS, T_HMON, T_NHMN, T_TIMS, TIDEWY,
    TOPMAR, TSELNE, TSSBND, TSSCRS, TSSLPT, TSSRON, TSEZNE, TUNNEL,
    TWRTPT, UWTROC, UNSARE, VEGATN, WATTUR, WATFAL, WEDKLP, WRECKS,
    TS_FEB,
    // meta object classes
    M_ACCY = 300, M_CSCL, M_COVR, M_HDAT, M_HOPA, M_NPUB, M_NSYS,
    M_PROD, M_QUAL, M_SDAT, M_SREL, M_UNIT, M_VDAT,
    // collection object classes
    C_AGGR = 400, C_ASSO, C_STAC,
    // cartographic object classes
    _AREAS = 500, _LINES, _CSYMB, _COMPS, _TEXTS
};



std::string decode_binary_string(unsigned char *bstr, int bcount);
void decode_lnam_string(unsigned char *bstr, int bcount, int *agen_find_fids);
void decode_name_string(unsigned char *bstr, int bcount, int *rcnm_rcid);

//字符显示相关信息结构体
typedef struct tagTextString{
    //画笔宽度都为1，字体格式也为1，颜色都为黑色CHBLK
    QString textString;      //显示字符内容
    quint32 textHJust:2;     //水平调整
    quint32 textVJust:2;     //垂直调整
    quint32 textSpace:2;     //字符间隔
    quint32 textWeight:4;     //字体大小  456
    quint32 textSize:6;       //尺寸
    quint32 textXOffset:4;     //x轴偏移量，加8变成无理数,默认为8
    quint32 textYOffset:4;     //y轴偏移量,加8变成无理数
    quint32 textDispGroup:8;  //字符显示组
}TextString;


#endif

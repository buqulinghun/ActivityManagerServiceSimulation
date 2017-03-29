
#include "TargetManage_global.h"
#include <math.h>

#define MAXINDEX 8000
#define INDEX1   2000
#define INDEX2   4000
#define INDEX3   6000
#define INDEX4   MAXINDEX
#define MAXSININDEX    INDEX1+1

float SIN_VALUE[MAXSININDEX];

const float rate1 = MAXINDEX / M_2PI;
const float rate2 = M_2PI / MAXINDEX;

#define SCALETO2PI(x)  ScaleTo2PI(x)
//#define SCALETO2PI(x)  (x)
//#define RADIANTOINDEX(x) radianToIndex(x)
#define RADIANTOINDEX(x) ((quint32)(((float)(x)) * rate1))
#define INDEXTORADIAN(x) ((float) (((float)(x)) * rate2))
#define NEXTINDEX(i)    ((i)<MAXINDEX-1 ? (i)+1 : 0)

void initializeMathLib ()
{
    for(quint32 i=0; i<MAXSININDEX; i++)
    {
        SIN_VALUE[i] = sin(INDEXTORADIAN(i%INDEX1));
    }
    SIN_VALUE[INDEX1] = 1.0;
}

float ScaleTo2PI(float x)
{
    while(x<0)
        x += M_2PI;
    while(x >= M_2PI)
        x -= M_2PI;
    return x;
}

#define SINVALUE1(i) ((i)<=INDEX1 ? SIN_VALUE[(i)] : SIN_VALUE[INDEX2-(i)])
#define SINVALUE2(i) ((i)<=INDEX3 ? -SIN_VALUE[(i)-INDEX2] : -SIN_VALUE[INDEX4-(i)])
#define GETSINVALUE(i) ((i)<=INDEX2 ? SINVALUE1(i) : SINVALUE2(i))

#define COSVALUE1(i) ((i)<=INDEX2 ? -SIN_VALUE[(i)-INDEX1] : -SIN_VALUE[INDEX3-(i)])
#define COSVALUE2(i) ((i)<=INDEX1 ? SIN_VALUE[INDEX1-(i)] : SIN_VALUE[(i)-INDEX3])
#define GETCOSVALUE(i) (((i)>INDEX1 && (i)<INDEX3) ? COSVALUE1(i) : COSVALUE2(i))

float Sin(float x)
{
    x = SCALETO2PI(x);
    const quint32 index1 = RADIANTOINDEX(x);
    const quint32 index2 = NEXTINDEX(index1);
//    if(index1 >= MAXINDEX || index2 >= MAXINDEX)
//        DEBUG << "index error";
    const float x1 = INDEXTORADIAN(index1);//, x2 = x1 + rate2;
    const float y1 = GETSINVALUE(index1), y2 = GETSINVALUE(index2);
    return (y1 + (x-x1)*(y2-y1)/rate2);
/*
    const quint32 index = RADIANTOINDEX(SCALETO2PI(x));
    if(index <= INDEX2)
    {
        return (index<INDEX1 ? SIN_VALUE[index] : SIN_VALUE[INDEX2-index]);
    }
    else
    {
        return -1.0*(index<INDEX3 ? SIN_VALUE[index-INDEX2] : SIN_VALUE[INDEX4-index]);
    }*/
}

float Cos(float x)
{
    x = SCALETO2PI(x);
    const quint32 index1 = RADIANTOINDEX(x);
    const quint32 index2 = NEXTINDEX(index1);
//    if(index1 >= MAXINDEX || index2 >= MAXINDEX)
//        DEBUG << "index error";
    const float x1 = INDEXTORADIAN(index1);//, x2 = x1 + rate2;
    const float y1 = GETCOSVALUE(index1), y2 = GETCOSVALUE(index2);
    return (y1 + (x-x1)*(y2-y1)/rate2);
    /*
    quint32 index = RADIANTOINDEX(SCALETO2PI(x));

    if(index >= INDEX1 && index <= INDEX3)
    {
        return -1.0*(index<=INDEX2 ? SIN_VALUE[index-INDEX1] : SIN_VALUE[INDEX3-index]);
    }
    else
    {
        return (index<INDEX1 ? SIN_VALUE[INDEX1-index] : SIN_VALUE[index-INDEX3]);
    }*/
}

float Tan(float x)
{
    /*
    quint32 index = RADIANTOINDEX(SCALETO2PI(x));
    float sinx = 0.0, cosx = 0.0;
    if(index <= INDEX1)
    {   // 0-PI/2
        sinx = SIN_VALUE[index];
        cosx = SIN_VALUE[INDEX1-index];
    }
    else if(index <= INDEX2)
    {   // PI/2-PI
        sinx = -SIN_VALUE[INDEX2-index];
        cosx = SIN_VALUE[index-INDEX1];
    }
    else if(index <= INDEX3)
    {   // PI-3PI/2
        sinx = SIN_VALUE[index-INDEX2];
        cosx = SIN_VALUE[INDEX3-index];
    }
    else
    {   // PI3/2-2PI
        sinx = -SIN_VALUE[INDEX4-index];
        cosx = SIN_VALUE[index-INDEX3];
    }*/

    x = SCALETO2PI(x);
    const quint32 index1 = RADIANTOINDEX(x);
    const quint32 index2 = NEXTINDEX(index1);
//    if(index1 >= MAXINDEX || index2 >= MAXINDEX)
//        DEBUG << "index error";
    const float x1 = INDEXTORADIAN(index1);//, x2 = x1 + rate2;
    const float sin1 = GETSINVALUE(index1), sin2 = GETSINVALUE(index2);
    const float cos1 = GETCOSVALUE(index1), cos2 = GETCOSVALUE(index2);

    const float dx = (x-x1)/rate2;
    const float sinx = sin1+dx*(sin2-sin1);
    const float cosx = cos1+dx*(cos2-cos1);
    return (EQUALZERO(cosx) ? (1.0e30) : sinx/cosx);
}

float Atan (float x)
{
    return 0.0;
}

float Atan2(float x, float y)
{
    return 0.0;
}



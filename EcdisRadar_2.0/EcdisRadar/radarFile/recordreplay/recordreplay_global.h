#ifndef RECORDREPLAY_GLOBAL_H
#define RECORDREPLAY_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef _MSVC_

#  define RECORDREPLAYSHARED_EXPORT

#else

#if defined(RECORDREPLAY_LIBRARY)
#  define RECORDREPLAYSHARED_EXPORT Q_DECL_EXPORT
#else
#  define RECORDREPLAYSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif

#pragma pack(1)   //zijie duiqi
// 记录文件头格式
typedef struct _rec_file_header
{
    qint8	_Flag[6];	// 标志位
    qint16	_Count;		// 记录次数
    qint32	_Length;	// 记录数据总长度 (包括数据头和数据信息的长度)
    qint32	_LocDataHeader;	// 最新的一次记录的起始位置
}RECFILEHEADER;
// 记录数据头格式
typedef struct _rec_data_header
{
    qint32	_StartTime;	// 记录起始时间
    qint32	_StopTime;	// 记录停止时间
    qint32	_TotalTime;	// 总时长
    qint32	_TrackNumberFlag[32];
    qint32	_Location;	// 记录头在文件中的起始位置
    qint32	_Count;		// 记录命令数量
    qint32	_Length;	// 记录数据长度 (包括数据头和数据信息的长度)
}RECDATAHEADER;
// 记录数据格式
typedef struct _rec_data_info
{
    quint16	_Flag;			// 数据标志
    quint16 _SrcId;         // 数据来源(2010-07-12添加)
    qint32	_RecTime;		// 该命令的记录时间
    quint16	_Length;		// 该命令的长度
//  qint8	_Content[0];	// 该命令的内容
}RECDATAINFO;
#pragma pack()

// 重演数据处理函数原型
typedef void (*DATAPROCESSFUNC)(const QByteArray& data, quint16 srcid);

#endif // RECORDREPLAY_GLOBAL_H

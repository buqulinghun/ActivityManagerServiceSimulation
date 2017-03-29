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
// ��¼�ļ�ͷ��ʽ
typedef struct _rec_file_header
{
    qint8	_Flag[6];	// ��־λ
    qint16	_Count;		// ��¼����
    qint32	_Length;	// ��¼�����ܳ��� (��������ͷ��������Ϣ�ĳ���)
    qint32	_LocDataHeader;	// ���µ�һ�μ�¼����ʼλ��
}RECFILEHEADER;
// ��¼����ͷ��ʽ
typedef struct _rec_data_header
{
    qint32	_StartTime;	// ��¼��ʼʱ��
    qint32	_StopTime;	// ��¼ֹͣʱ��
    qint32	_TotalTime;	// ��ʱ��
    qint32	_TrackNumberFlag[32];
    qint32	_Location;	// ��¼ͷ���ļ��е���ʼλ��
    qint32	_Count;		// ��¼��������
    qint32	_Length;	// ��¼���ݳ��� (��������ͷ��������Ϣ�ĳ���)
}RECDATAHEADER;
// ��¼���ݸ�ʽ
typedef struct _rec_data_info
{
    quint16	_Flag;			// ���ݱ�־
    quint16 _SrcId;         // ������Դ(2010-07-12���)
    qint32	_RecTime;		// ������ļ�¼ʱ��
    quint16	_Length;		// ������ĳ���
//  qint8	_Content[0];	// �����������
}RECDATAINFO;
#pragma pack()

// �������ݴ�����ԭ��
typedef void (*DATAPROCESSFUNC)(const QByteArray& data, quint16 srcid);

#endif // RECORDREPLAY_GLOBAL_H

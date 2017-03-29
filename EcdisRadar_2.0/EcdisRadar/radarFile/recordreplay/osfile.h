#ifndef OSFILE_H
#define OSFILE_H

#include "recordreplay_global.h"

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QString>
#include <QtCore/QHash>

typedef void* LPVOID;

/*
  �ļ���:osFile
  �ṩ�ļ��ڴ�ӳ�䷽ʽ�����ļ��� �������ڼ�¼���ݶ��ļ��ķ���
  �����ߣ�moflying
  ���ڣ�2010-04-05
*/
class RECORDREPLAYSHARED_EXPORT osFile
{
public:
    osFile(const QString& fileName=NULL, qint32 flag=0);
    ~osFile();

public:
    enum {
            os_seek_bgn = 0,
            os_seek_set = 1,
            os_seek_end = 2
    };

    // ��ȡ�ļ���
    QString GetFileName () const;

    bool Open (const QString& fileName, qint32 flag=0);
    // �ر��ļ�
    bool Close ();
    // ��ȡ�ļ�
    quint32 Read (LPVOID pBuffer, quint32 dwSize);
    // д�ļ�
    quint32 Write (LPVOID pBuffer, quint32 dwSize);
    // ��ȡ�ļ�����
    quint32 GetFileLength () const;
    // �����ļ��ڴ�ָ��λ��
    quint32 SetPosition (qint32 dwOffset, quint32 dwFlag = osFile::os_seek_bgn);
    // ��ȡ�ļ��ڴ�ָ��λ��
    quint32 GetPosition () const;
    // �ж��ļ��Ƿ���Ч
    bool IsValidate () const;

private:
    class osFileImpl* m_implement;
    static QHash<QString, class osFileImpl*> m_implementTable;
};

#endif // OSFILE_H

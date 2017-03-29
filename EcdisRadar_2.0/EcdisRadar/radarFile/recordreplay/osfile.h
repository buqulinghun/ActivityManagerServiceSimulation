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
  文件类:osFile
  提供文件内存映射方式访问文件， 该类用于记录重演对文件的访问
  创作者：moflying
  日期：2010-04-05
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

    // 获取文件名
    QString GetFileName () const;

    bool Open (const QString& fileName, qint32 flag=0);
    // 关闭文件
    bool Close ();
    // 读取文件
    quint32 Read (LPVOID pBuffer, quint32 dwSize);
    // 写文件
    quint32 Write (LPVOID pBuffer, quint32 dwSize);
    // 获取文件长度
    quint32 GetFileLength () const;
    // 设置文件内存指针位置
    quint32 SetPosition (qint32 dwOffset, quint32 dwFlag = osFile::os_seek_bgn);
    // 获取文件内存指针位置
    quint32 GetPosition () const;
    // 判断文件是否有效
    bool IsValidate () const;

private:
    class osFileImpl* m_implement;
    static QHash<QString, class osFileImpl*> m_implementTable;
};

#endif // OSFILE_H

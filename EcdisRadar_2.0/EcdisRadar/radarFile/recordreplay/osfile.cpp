#include "osfile.h"
#include <QtCore/QFile>
#include <QtCore/QtDebug>

#ifdef Q_OS_WIN32
#include <windows.h>
#endif

#ifdef Q_OS_LINUX
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif

class CReferenceObject : public QObject
{
public:
    CReferenceObject(QObject* parent=NULL)
        :QObject(parent),m_referenceCount(1)
    {
    }

    /*由类工厂 ObjectFactory 负责管理引用计数和申请、删除对象*/
    int increaseReference()
    {   return ++m_referenceCount;  }
    int decreaseReference()
    {   return --m_referenceCount;  }

protected:
    int m_referenceCount;
};

/*
  文件类:osFileImpl
  提供文件内存映射方式访问文件， 该类用于记录重演对文件的访问
  创作者：moflying
  日期：2010-04-05
*/
class osFileImpl : public CReferenceObject
{
public:
    osFileImpl (const char* fileName=NULL, qint32 flag=0);
    virtual ~osFileImpl ();

    // 访问接口定义
public:
    // 获取文件名
    QString GetFileName () const
    {	return m_fileName;	}

    bool Open (const char* fileName, qint32 flag=0);
    // 关闭文件
    bool Close ();
    // 读取文件
    quint32 Read (LPVOID pBuffer, quint32 dwSize);
    // 写文件
    quint32 Write (LPVOID pBuffer, quint32 dwSize);
    // 获取文件长度
    quint32 GetFileLength () const
    {   return m_dwFileSize;    }
    // 设置文件内存指针位置
    quint32 SetPosition (qint32 dwOffset, quint32 dwFlag = osFile::os_seek_bgn);
    // 获取文件内存指针位置
    quint32 GetPosition () const
    {   return m_dwPosition;    }
    // 判断文件是否有效
    bool IsValidate () const
    {
#ifdef Q_OS_WIN32
        return (m_hFile != NULL);
#endif
#ifdef Q_OS_LINUX
        return (m_fd > 0);
#endif
        return false;
    }

    // 内部对象
private:
#ifdef Q_OS_WIN32
    HANDLE	m_hFile;		// 文件句柄
    HANDLE	m_hFileMap;		// 文件映射句柄
    //quint32	m_dwGranularity;// 系统分配粒度
#endif
#ifdef Q_OS_LINUX
    int     m_fd;
#endif
    quint8*	m_pMemHeader;	// 内存映射首地址
    quint32	m_dwPosition;
    quint32	m_dwFileSize;	// 文件大小

    QMutex	m_access_lock;
    QString	m_fileName;
};

/**********************************\
 *
 * class : osFileImpl
 * author：moflying
 * date：2010-04-05
 *
\**********************************/
osFileImpl::osFileImpl (const char* fileName, qint32 flag)
:CReferenceObject(0)
{
#ifdef Q_OS_WIN32
    m_hFile = NULL;			// 文件句柄
    m_hFileMap = NULL;		// 文件映射句柄

    //SYSTEM_INFO sinf;
    //GetSystemInfo (&sinf);
    //m_dwGranularity = sinf.dwAllocationGranularity;
#endif
#ifdef Q_OS_LINUX
    m_fd = 0;
#endif

    m_pMemHeader = NULL;	// 内存映射首地址
    m_dwPosition = 0;		// 内存地址偏移量
    m_dwFileSize = 0;

    if (fileName)
        Open (fileName, flag);
}

osFileImpl::~osFileImpl ()
{
    Close ();
}

// 访问接口定义

// 打开文件
bool osFileImpl::Open (const char* fileName, qint32 flag)
{
    // 首选关闭以前打开的文件映射
    Close ();

    if (!fileName)
        return false;

    m_fileName = QString(fileName);

    try
    {
    QMutexLocker locker(&m_access_lock);

#ifdef Q_OS_WIN32
    quint32 dwDesiredAccess = (GENERIC_READ | GENERIC_WRITE);
    quint32 dwShareMode = (FILE_SHARE_READ | FILE_SHARE_WRITE);
    // 打开文件对象
    m_hFile = ::CreateFileA (fileName, dwDesiredAccess, dwShareMode, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_hFile == INVALID_HANDLE_VALUE)
        throw ("Create File Failed\n");

    // 获取文件大小
    m_dwFileSize = ::GetFileSize (m_hFile, NULL);

    // 创建文件映射对象(文件最大为4个Gb)
/*	m_hFileMap = ::CreateFileMapping (m_hFile, NULL, PAGE_READWRITE, 0, 0xFFFFFFFF, NULL);
    if (!m_hFileMap)
        throw ("Create File Map Failed\n");*/

    // 将文件数据映射到内存空间
/*	m_pMemHeader = (quint8*)::MapViewOfFile (m_hFileMap, FILE_MAP_WRITE, 0, 0, 0);
    if (!m_pMemHeader)
        throw ("Map view of file failed\n");*/    
#endif

#ifdef Q_OS_LINUX
    m_fd = open(fileName, O_RDWR | O_CREAT);
    if(m_fd == -1)
        throw "Create File Failed\n";

    struct stat buf;
    fstat(m_fd, &buf);
    m_dwFileSize = buf.st_size;
#endif

    return true;
    }

    catch (...)
    {
        //quint32 dwError = GetLastError ();
        //printf ("Error Code:%d\n", dwError);
        //ShowSystemMsg(TR("打开文件失败:")+QString(fileName));
        qDebug() << "open file failed:" << QString::fromLocal8Bit(fileName);

        Close ();

        return false;
    }

    return false;
}

// 关闭文件
bool osFileImpl::Close ()
{
    QMutexLocker locker(&m_access_lock);

#ifdef Q_OS_WIN32
    if (m_pMemHeader)
    {
        UnmapViewOfFile (m_pMemHeader);
        m_pMemHeader = NULL;
    }
    if (m_hFileMap)
    {
        CloseHandle (m_hFileMap);
        m_hFileMap = NULL;
    }
    if (m_hFile)
    {
        CloseHandle (m_hFile);
        m_hFile = NULL;
    }
#endif
#ifdef Q_OS_LINUX
    if(m_pMemHeader)
    {
        msync(m_pMemHeader, m_dwFileSize, MS_SYNC);
        munmap(m_pMemHeader, m_dwFileSize);
    }

    if(m_fd > 0)
    {
        close(m_fd);
        m_fd = 0;
    }
#endif

    return true;
}

// 读取文件
quint32 osFileImpl::Read (LPVOID pBuffer, quint32 dwSize)
{
    QMutexLocker locker(&m_access_lock);

    const quint32 dwFileLength = GetFileLength ();
#ifdef Q_OS_WIN32
    //quint32 dwFileOffset = (quint32)(m_dwPosition / m_dwGranularity) * m_dwGranularity;
    if (m_dwPosition >= dwFileLength)
        return 0;

    // Create file
    m_hFileMap = ::CreateFileMapping (m_hFile, NULL, PAGE_READWRITE, 0, dwFileLength, NULL);
    if (!m_hFileMap)
        return 0;
    // map file to memory
    m_pMemHeader = (quint8*)MapViewOfFile (m_hFileMap, FILE_MAP_READ, 0, 0, dwFileLength);
    CloseHandle (m_hFileMap);
    m_hFileMap = 0;
    if (!m_pMemHeader)
        return 0;
    // copy data
    const quint32 bytesInBlock = qMin((int)dwSize, (int)(dwFileLength-m_dwPosition));
    memcpy (pBuffer, &m_pMemHeader[m_dwPosition], bytesInBlock);
    // update pos
    m_dwPosition += bytesInBlock;
    // unmap
    UnmapViewOfFile (m_pMemHeader);
    m_pMemHeader = NULL;

    return bytesInBlock;
#endif
#ifdef Q_OS_LINUX
#if 0
    m_pMemHeader = (quint8*)mmap (NULL, dwFileLength, PROT_READ, MAP_SHARED, m_fd, 0);
    if(!m_pMemHeader)
        return 0;

    // copy data
    const quint32 bytesInBlock = qMin((int)dwSize, (int)(dwFileLength-m_dwPosition));
    memcpy (pBuffer, &m_pMemHeader[m_dwPosition], bytesInBlock);
    // update pos
    m_dwPosition += bytesInBlock;
    // unmap
    munmap(m_pMemHeader, dwFileLength);
    m_pMemHeader = NULL;
#else
    const quint32 bytesInBlock = qMin((int)dwSize, (int)(dwFileLength-m_dwPosition));
    if(lseek(m_fd, m_dwPosition, SEEK_SET) != m_dwPosition)
        qDebug() << "lseek failed.";
    if(read(m_fd, pBuffer, bytesInBlock) != bytesInBlock)
        qDebug() << "read failed.";
    m_dwPosition += bytesInBlock;
#endif

    return bytesInBlock;
#endif

    return 0;
}

// 写文件
quint32 osFileImpl::Write (LPVOID pBuffer, quint32 dwSize)
{
    QMutexLocker locker(&m_access_lock);

    const quint32 dwFileLength = GetFileLength ();
#ifdef Q_OS_WIN32
    if (m_dwPosition > dwFileLength)
        m_dwPosition = dwFileLength;
    quint32 dwFileLength1 = m_dwPosition + dwSize;
    // 长度错误
    if (dwFileLength1 == 0)
        return 0;

    // create file
    m_hFileMap = ::CreateFileMapping (m_hFile, NULL, PAGE_READWRITE, 0, dwFileLength1, NULL);
    if (!m_hFileMap)
        return 0;
    // map file to memory
    m_pMemHeader = (quint8*)MapViewOfFile (m_hFileMap, FILE_MAP_WRITE, 0, 0, dwFileLength1);
    CloseHandle (m_hFileMap);
    m_hFileMap = 0;
    if (!m_pMemHeader)
        return 0;
    // copy data
    memcpy (&m_pMemHeader[m_dwPosition], pBuffer, dwSize);
    // update
    m_dwPosition += dwSize;
    m_dwFileSize = qMax ((quint32)m_dwPosition, (quint32)m_dwFileSize);
    FlushViewOfFile (m_pMemHeader, m_dwFileSize);
    // unmap
    UnmapViewOfFile(m_pMemHeader);
    m_pMemHeader = NULL;

    return dwSize;
#endif

#ifdef Q_OS_LINUX
    if (m_dwPosition > dwFileLength)
        m_dwPosition = dwFileLength;
    quint32 dwFileLength1 = m_dwPosition + dwSize;
    // 长度错误
    if (dwFileLength1 == 0)
        return 0;

#if 0
    dwFileLength1 = (dwFileLength1/1024+1)*1024;

    qDebug() << "write to file 11111" << dwFileLength1;
    // map file to memory
    //m_pMemHeader = (quint8*)mmap (NULL, dwFileLength1, PROT_WRITE, MAP_SHARED, m_fd, 0);
    m_pMemHeader = (quint8*)mmap (NULL, dwFileLength1, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, m_fd, 0);
    if (m_pMemHeader == MAP_FAILED)
        return 0;
    qDebug() << "write to file ,m_pMemHeader:" << QString::number(int(m_pMemHeader), 16);
    // copy data
    memcpy (&m_pMemHeader[m_dwPosition], pBuffer, dwSize);
    qDebug() << "write to file 33333";
    // update
    m_dwPosition += dwSize;
    m_dwFileSize = qMax ((quint32)m_dwPosition, (quint32)m_dwFileSize);
    msync(m_pMemHeader, m_dwFileSize, MS_SYNC);
    qDebug() << "write to file 44444";
    // unmap
    munmap(m_pMemHeader, m_dwFileSize);
    m_pMemHeader = NULL;

    qDebug() << "write to file" << dwSize;
#else
    if(lseek(m_fd, m_dwPosition, SEEK_SET) != m_dwPosition)
        qDebug() << "lseek failed.";
    if(write(m_fd, pBuffer, dwSize) != dwSize)
        qDebug() << "read failed.";
    m_dwPosition += dwSize;
    m_dwFileSize = qMax ((quint32)m_dwPosition, (quint32)m_dwFileSize);
#endif

    return dwSize;
#endif

    return 0;
}

// 设置文件内存指针位置
quint32 osFileImpl::SetPosition (qint32 dwOffset, quint32 dwFlag)
{
    QMutexLocker locker(&m_access_lock);

    switch (dwFlag)
    {
    case osFile::os_seek_bgn:
    {
        m_dwPosition = dwOffset;
        break;
    }
    case osFile::os_seek_set:
    {
        m_dwPosition += dwOffset;
        break;
    }
    case osFile::os_seek_end:
    {
        const quint32 dwFileLength = GetFileLength();
        if (dwFileLength > dwOffset)
                m_dwPosition = dwFileLength - dwOffset;
        else
                m_dwPosition = 0;
        break;
    }

    default:
        break;
    }

    return m_dwPosition;
}


///////////////////////////////////////////////////
QHash<QString, class osFileImpl*> osFile::m_implementTable;

osFile::osFile(const QString& filename, qint32 flag) :
    m_implement(NULL)
{
    Open(filename, flag);
}

osFile::~osFile()
{
    if(m_implement && m_implement->decreaseReference() == 0)
	{
		m_implementTable.remove(m_implement->GetFileName());
        delete m_implement;
	}
}

// 获取文件名
QString osFile::GetFileName () const
{
    return m_implement?m_implement->GetFileName():QString();
}

bool osFile::Open (const QString& filename, qint32 flag)
{
    //if(QFile::exists(filename))
    {
        if(m_implement = m_implementTable.value(filename, NULL))
        {
            m_implement->increaseReference();
            if(!m_implement->IsValidate())
                m_implement->Open(filename.toLocal8Bit());
        }
        else
        {
            m_implement = new osFileImpl(filename.toLocal8Bit(), flag);
            m_implementTable.insert(filename,m_implement);
        }
    }

    return (m_implement && m_implement->IsValidate());
}

// 关闭文件
bool osFile::Close ()
{
    return m_implement?m_implement->Close():false;
}

// 读取文件
quint32 osFile::Read (LPVOID pBuffer, quint32 dwSize)
{
    return m_implement?m_implement->Read(pBuffer, dwSize):0;
}

// 写文件
quint32 osFile::Write (LPVOID pBuffer, quint32 dwSize)
{
    return m_implement?m_implement->Write(pBuffer, dwSize):0;
}

// 获取文件长度
quint32 osFile::GetFileLength () const
{
    return m_implement?m_implement->GetFileLength():0;
}
// 设置文件内存指针位置
quint32 osFile::SetPosition (qint32 dwOffset, quint32 dwFlag)
{
    return m_implement?m_implement->SetPosition(dwOffset, dwFlag):0;
}

// 获取文件内存指针位置
quint32 osFile::GetPosition () const
{
    return m_implement?m_implement->GetPosition():0;
}
// 判断文件是否有效
bool osFile::IsValidate () const
{
    return m_implement?m_implement->IsValidate():false;
}

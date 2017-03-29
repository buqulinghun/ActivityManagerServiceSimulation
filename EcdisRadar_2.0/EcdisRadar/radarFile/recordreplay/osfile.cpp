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

    /*���๤�� ObjectFactory ����������ü��������롢ɾ������*/
    int increaseReference()
    {   return ++m_referenceCount;  }
    int decreaseReference()
    {   return --m_referenceCount;  }

protected:
    int m_referenceCount;
};

/*
  �ļ���:osFileImpl
  �ṩ�ļ��ڴ�ӳ�䷽ʽ�����ļ��� �������ڼ�¼���ݶ��ļ��ķ���
  �����ߣ�moflying
  ���ڣ�2010-04-05
*/
class osFileImpl : public CReferenceObject
{
public:
    osFileImpl (const char* fileName=NULL, qint32 flag=0);
    virtual ~osFileImpl ();

    // ���ʽӿڶ���
public:
    // ��ȡ�ļ���
    QString GetFileName () const
    {	return m_fileName;	}

    bool Open (const char* fileName, qint32 flag=0);
    // �ر��ļ�
    bool Close ();
    // ��ȡ�ļ�
    quint32 Read (LPVOID pBuffer, quint32 dwSize);
    // д�ļ�
    quint32 Write (LPVOID pBuffer, quint32 dwSize);
    // ��ȡ�ļ�����
    quint32 GetFileLength () const
    {   return m_dwFileSize;    }
    // �����ļ��ڴ�ָ��λ��
    quint32 SetPosition (qint32 dwOffset, quint32 dwFlag = osFile::os_seek_bgn);
    // ��ȡ�ļ��ڴ�ָ��λ��
    quint32 GetPosition () const
    {   return m_dwPosition;    }
    // �ж��ļ��Ƿ���Ч
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

    // �ڲ�����
private:
#ifdef Q_OS_WIN32
    HANDLE	m_hFile;		// �ļ����
    HANDLE	m_hFileMap;		// �ļ�ӳ����
    //quint32	m_dwGranularity;// ϵͳ��������
#endif
#ifdef Q_OS_LINUX
    int     m_fd;
#endif
    quint8*	m_pMemHeader;	// �ڴ�ӳ���׵�ַ
    quint32	m_dwPosition;
    quint32	m_dwFileSize;	// �ļ���С

    QMutex	m_access_lock;
    QString	m_fileName;
};

/**********************************\
 *
 * class : osFileImpl
 * author��moflying
 * date��2010-04-05
 *
\**********************************/
osFileImpl::osFileImpl (const char* fileName, qint32 flag)
:CReferenceObject(0)
{
#ifdef Q_OS_WIN32
    m_hFile = NULL;			// �ļ����
    m_hFileMap = NULL;		// �ļ�ӳ����

    //SYSTEM_INFO sinf;
    //GetSystemInfo (&sinf);
    //m_dwGranularity = sinf.dwAllocationGranularity;
#endif
#ifdef Q_OS_LINUX
    m_fd = 0;
#endif

    m_pMemHeader = NULL;	// �ڴ�ӳ���׵�ַ
    m_dwPosition = 0;		// �ڴ��ַƫ����
    m_dwFileSize = 0;

    if (fileName)
        Open (fileName, flag);
}

osFileImpl::~osFileImpl ()
{
    Close ();
}

// ���ʽӿڶ���

// ���ļ�
bool osFileImpl::Open (const char* fileName, qint32 flag)
{
    // ��ѡ�ر���ǰ�򿪵��ļ�ӳ��
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
    // ���ļ�����
    m_hFile = ::CreateFileA (fileName, dwDesiredAccess, dwShareMode, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_hFile == INVALID_HANDLE_VALUE)
        throw ("Create File Failed\n");

    // ��ȡ�ļ���С
    m_dwFileSize = ::GetFileSize (m_hFile, NULL);

    // �����ļ�ӳ�����(�ļ����Ϊ4��Gb)
/*	m_hFileMap = ::CreateFileMapping (m_hFile, NULL, PAGE_READWRITE, 0, 0xFFFFFFFF, NULL);
    if (!m_hFileMap)
        throw ("Create File Map Failed\n");*/

    // ���ļ�����ӳ�䵽�ڴ�ռ�
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
        //ShowSystemMsg(TR("���ļ�ʧ��:")+QString(fileName));
        qDebug() << "open file failed:" << QString::fromLocal8Bit(fileName);

        Close ();

        return false;
    }

    return false;
}

// �ر��ļ�
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

// ��ȡ�ļ�
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

// д�ļ�
quint32 osFileImpl::Write (LPVOID pBuffer, quint32 dwSize)
{
    QMutexLocker locker(&m_access_lock);

    const quint32 dwFileLength = GetFileLength ();
#ifdef Q_OS_WIN32
    if (m_dwPosition > dwFileLength)
        m_dwPosition = dwFileLength;
    quint32 dwFileLength1 = m_dwPosition + dwSize;
    // ���ȴ���
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
    // ���ȴ���
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

// �����ļ��ڴ�ָ��λ��
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

// ��ȡ�ļ���
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

// �ر��ļ�
bool osFile::Close ()
{
    return m_implement?m_implement->Close():false;
}

// ��ȡ�ļ�
quint32 osFile::Read (LPVOID pBuffer, quint32 dwSize)
{
    return m_implement?m_implement->Read(pBuffer, dwSize):0;
}

// д�ļ�
quint32 osFile::Write (LPVOID pBuffer, quint32 dwSize)
{
    return m_implement?m_implement->Write(pBuffer, dwSize):0;
}

// ��ȡ�ļ�����
quint32 osFile::GetFileLength () const
{
    return m_implement?m_implement->GetFileLength():0;
}
// �����ļ��ڴ�ָ��λ��
quint32 osFile::SetPosition (qint32 dwOffset, quint32 dwFlag)
{
    return m_implement?m_implement->SetPosition(dwOffset, dwFlag):0;
}

// ��ȡ�ļ��ڴ�ָ��λ��
quint32 osFile::GetPosition () const
{
    return m_implement?m_implement->GetPosition():0;
}
// �ж��ļ��Ƿ���Ч
bool osFile::IsValidate () const
{
    return m_implement?m_implement->IsValidate():false;
}

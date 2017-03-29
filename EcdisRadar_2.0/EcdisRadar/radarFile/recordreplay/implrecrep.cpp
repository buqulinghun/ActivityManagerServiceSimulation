
#include "implrecrep.h"
#include "osfile.h"

#include "../CustomEvent.h"
#include "../define.h"

#include <QtCore/QtDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <time.h>

bool toExit = false;
bool autoStopReplayFlag = false;
void showSystemMsg(const QString& ){}
void showRecrepMsg(const QString& ){}
void showDebugMsg(const QString& ) {}

/*******************************************************\
*
* class : ImplRecRep
*
* ��¼���ݴ������ʵ��
*
\*******************************************************/
const int sizeofFileHeader = sizeof(RECFILEHEADER);	//16;
const int sizeofDataHeader = sizeof(RECDATAHEADER);	//152;
const int sizeofDataInfo   = sizeof(RECDATAINFO);	// 10

const int DataBufferSize = 10100;
const int MaxOneFrameSize = 10000;

const int SecondTo100ms = 10, MsecondTo100ms = 100;

ImplRecRep::ImplRecRep (QObject *parent)
    : QObject(parent)
{
    connect (this, SIGNAL(stopSignal()), this, SLOT(stopSlot()));

    // Ĭ���ڵ�ǰ·��
    setRecordPath(CURRENTPATH);

    m_recDataBuffer = new quint8 [DataBufferSize];
    m_recWriteIndex = 0;

    m_recordingFlag = false;
    m_replayingFlag = false;

    m_computerTime_100ms = 0;
    m_nRecordComputerTime_100ms = 0;
    m_nReplayComputerTime_100ms = 0;

    m_dataProcFunc = NULL;
    m_updateEventId = 0;

    m_paused = false;
    m_recFile = NULL;
    m_repFile = NULL;

    m_replayThread = NULL;
    m_nRepSpeed = 1;
    m_waitMode = 0;

//	startRecord ();
}

ImplRecRep::~ImplRecRep ()
{
    stopRecord ();
    stopReplay ();

    if (m_recDataBuffer)
    delete[] m_recDataBuffer;
}

// ��ȡ��ǰϵͳʱ��
//quint32 ImplRecRep::systemTime () const
//{
    //return SystemInfo.SystemDateTime.toTime_t();
//    return time(NULL);
//}

// �ú���ֻ�ڼ�¼ʱ���ã�����ʹ�ü����ʵ��ʱ��
quint32 ImplRecRep::computerTime () const
{
    return time(0);
}

// ��ȡ��ǰϵͳʱ��100ms
qint64 ImplRecRep::computerTime100ms () const
{
    QDateTime dt = QDateTime::currentDateTime();
    return dt.toTime_t() * SecondTo100ms + dt.time().msec() / MsecondTo100ms;
//	return time(0) * SecondTo100ms + QTime::currentTime().msec() / MsecondTo100ms;
}

// ��ȡʱ���ַ�����ʾ
QString ImplRecRep::getTimeString (const QDateTime& dt) const
{
    const QDate d = dt.date();
    const QTime t = dt.time();
    const QChar fillchar('0');
    return tr("%1%2%3-%4%5%6").arg(d.year(), 4, 10, fillchar).arg(d.month(), 2, 10, fillchar).arg(d.day(), 2, 10, fillchar)\
            .arg(t.hour(), 2, 10, fillchar).arg(t.minute(), 2, 10, fillchar).arg(t.second(), 2, 10, fillchar);
}

void ImplRecRep::stopSlot ()
{
    stopReplay ();
}

// ·���ַ��������/�ж�
QString PathWithFlash(const QString& path)
{
    const QString endchar = "/";

    if(path.endsWith(endchar))
        return path;
    else
        return path + endchar;
}

////////////////////////////////////////////////////////////////////
// ���ü�¼������Ŀ¼
void ImplRecRep::setRecordPath(const QString& path)
{
    m_replayFilePath = PathWithFlash(path);

    QDir dir(m_replayFilePath);
    if (!dir.exists())
            dir.mkpath (m_replayFilePath);
}

// ��ʼ��¼
bool ImplRecRep::startRecord ()
{
    const QDateTime dt = QDateTime::currentDateTime();
    QString filename = m_replayFilePath + getTimeString(dt) + tr(".rec");
	const char * tempFileName = filename.toLocal8Bit().data();
    if (startRecord(filename))
    {
        showRecrepMsg(tr("��¼..."));
        // ����״̬��ʾ
        updateStatusDisplay();
        return true;
    }
    else
    {
        return false;
    }
}

// ��ʼ��¼
bool ImplRecRep::startRecord (const QString& fileName)
{	
    // �����ǰ���ڼ�¼״̬����ֹͣ
    if (isRecording())
        stopRecord ();

    // ������¼״̬
    QMutexLocker locker(&m_recordingMutex);

    // �����¼���ݻ�����
    if (!m_recDataBuffer)
        m_recDataBuffer = new quint8 [DataBufferSize];
    m_recWriteIndex = 0;

    // ���ļ����
    m_recFile = openFile (fileName);
    if ((!m_recFile) || (!m_recFile->IsValidate()))
    {
        showSystemMsg(tr("���ļ�ʧ��:���ܼ�¼"));
        closeRecordFile ();
        return false;
    }

    // ��ȡ��ǰʱ��
    time_t tm0 = computerTime();//time(NULL);

    // ��ȡ�ļ�����
    qint32 length = m_recFile->GetFileLength();
    qint32 nextPos = length;
    bool AddRecTimes = FALSE;

    // ����ļ�����С���ļ�ͷ�ĳ��ȣ������´����ļ�
    if (length < sizeofFileHeader)
    {	// ��ʼ���ļ�ͷ����
        memset (&m_recFileHeader, 0, sizeofFileHeader);
        strcpy ((char*)m_recFileHeader._Flag, "hktk");
        m_recFileHeader._Count = 1;
        m_recFileHeader._Length = sizeofFileHeader;
        // ������ʼλ��Ϊ�ļ�ͷ֮��ĵ�1���ֽ�
        nextPos = sizeofFileHeader;
        m_recFileHeader._LocDataHeader = nextPos;
    }
    else
    {
        // ��ȡ�ļ�ͷ����
        const int nRead = fileRead (m_recFile, &m_recFileHeader, sizeofFileHeader, 0);
        if(nRead != sizeofFileHeader)
        {
            closeRecordFile ();
            return false;
        }

        // �ж����һ������ļ�¼ͷ�Ƿ�Ϊ��
        if (length >= m_recFileHeader._LocDataHeader + sizeofDataHeader)
        {
            const int sizeofRead = fileRead (m_recFile, &m_recDataHeader, sizeofDataHeader, m_recFileHeader._LocDataHeader);
            if (sizeofRead == sizeofDataHeader && m_recDataHeader._Count > 0)
            {	// �ϴμ�¼�ǿգ�����µļ�¼ͷ
                //nextPos = m_recDataHeader._Location;
                AddRecTimes = TRUE;
            }
        }

        // ���ʹ���µļ�¼�����¼������1������ʹ���ϴμ�¼ͷ��λ��
        if (AddRecTimes)
            m_recFileHeader._Count ++;
        else
            nextPos = m_recDataHeader._Location;
    }

    // �ļ����ݳ�����Ҫ���ϼ�¼ͷ�ĳ���
    m_recFileHeader._Length = nextPos + sizeofDataHeader;
    // ��¼���¼�¼����ʼλ��
    m_recFileHeader._LocDataHeader = nextPos;

    // �����ļ�ͷ���ݿ鵽�ļ���
    writeFileHeader ();

    // ��ʼ������ͷ
    memset (&m_recDataHeader, 0, sizeofDataHeader);
    m_recDataHeader._StartTime = tm0;
    m_recDataHeader._StopTime = tm0;
    m_recDataHeader._Count = 0;
    m_recDataHeader._Length = sizeofDataHeader;
    m_recDataHeader._Location = nextPos;

    // �����¼ͷ���ļ���
    writeDataHeader(nextPos);

    // ���ݱ�־
    m_recDataInfo._Flag = 0xFFFF;

    m_nRecordComputerTime_100ms = computerTime100ms ();
    m_nRecordingTime_100ms = 0;
    m_recordingFlag = true;

    return TRUE;
}

// ֹͣ��¼
void ImplRecRep::stopRecord ()
{
    // ������¼״̬
    QMutexLocker locker(&m_recordingMutex);

    // ���������ݱ��浽�ļ�
    writeRecordDataToFile ();

    m_recordingFlag = false;

    // �رմ򿪵��ļ����
    closeRecordFile ();

    showRecrepMsg(tr("ֹͣ��¼"));
    // ����״̬��ʾ
    updateStatusDisplay();
}

// ��¼����
bool ImplRecRep::recording (const char* pData, qint32 nLength, quint16 srcid)
{
    // ���ݳ��ȴ���
    if (nLength > MaxOneFrameSize)
            return false;

    // �����ǰ���ڼ�¼״̬���򷵻ش���
    if (!isRecording())
        return FALSE;

    // һ�죬ʹ��һ���µ��ļ�
    if (m_nRecordingTime_100ms >= 288000l)	// 8640000l = 24Сʱ = 24 * 3600 * 10 100ms
        startRecord ();

    // ������¼״̬
    QMutexLocker locker(&m_recordingMutex);

    // �����ܳ��� = ����ͷ�ĳ���(6)+���ݵĳ���(nLength)
    const int sizeofData = sizeofDataInfo + nLength;

    // �����ļ�ͷ�ĳ���(+�����ܳ���)
    m_recFileHeader._Length += sizeofData;
//	writeFileHeader();

    // �������ű�־
/*  qint16 track_no, track_3a;
    if (lpChannel->GetTrackNumber((quint8*)pData, nLength, track_no, track_3a) &&
            track_no > 0 && track_no < 1000)
    {
        track_no -= 1;
        int index = track_no / 32;
        int bit = track_no % 32;
        m_recDataHeader._TrackNumberFlag[index] |= (1 << bit);
    }*/

	qint32 stopTime = computerTime ();
	if(stopTime < m_recDataHeader._StopTime)
	{	// ���ʱ���쳣�������¼���ʱ��
		stopTime = m_recDataHeader._StopTime + (m_nRecordingTime_100ms-m_recDataHeader._TotalTime) / 10;
	}

    // ���¼�¼ͷ
    m_recDataHeader._Count ++;
    m_recDataHeader._StopTime = stopTime;
    m_recDataHeader._TotalTime = m_nRecordingTime_100ms;
    m_recDataHeader._Length += sizeofData;
//	writeDataHeader(m_recDataHeader._Location);

    // �����¼����
    //m_recDataInfo._RecTime = tm0;
    m_recDataInfo._SrcId = srcid;   // ������Դ
    m_recDataInfo._RecTime = m_nRecordingTime_100ms;	// ����100�������ʱ��
    m_recDataInfo._Length = nLength;
    writeData (pData, nLength);

    return TRUE;
}

////////////////////////////////////////////////////////////////////
// replaying process

// ��ʼ����(��ָ���ļ���ʼ����)
bool ImplRecRep::startReplay (const QString& fileName, qint32 start_tm, qint32 stop_tm)
{
    if (isReplaying())
    {
        showSystemMsg (tr("��������"));
        return false;
    }

    // ����ʱ��
    m_nReplayStartTime = start_tm;
    m_nReplayStopTime = stop_tm >= 0 ? stop_tm : computerTime() - 1;

    // 2015��1��1��00:00:01
    qint32 lt = QDateTime::fromString("M1d1y201500:00:01","'M'M'd'd'y'yyyyhh:mm:ss").toTime_t();
    // ʱ�����
    if (m_nReplayStartTime > 0 &&
        (m_nReplayStartTime <= lt ||
        (m_nReplayStopTime > 0 && m_nReplayStartTime >= m_nReplayStopTime)) )
        return false;

    // ��ʱ����Ҷ�Ӧ�ļ�¼�ļ�
    int count = toGetReplayFiles (fileName);
	if (count <= 0){
		qDebug() << __FUNCTION__ << "can't find replay file.";
        return false;
	}

    autoStopReplayFlag = false;
    // ���ÿ��Ʋ��������������߳�
    toStartReplay ();

    return true;
}

// ���ÿ��Ʋ��������������߳�
bool ImplRecRep::toStartReplay ()
{
    // ����Ѿ�������,�����
    if (isReplaying())
        return false;//stopReplay ();

    // ���ÿ��Ʋ���
    m_nReplayTotalTime_100ms =  (m_nReplayStopTime - m_nReplayStartTime) * SecondTo100ms;
    m_nReplayComputerTime_100ms = computerTime100ms ();
    m_nReplayingTime_100ms = 0;
    m_paused = false;
    m_replayFinished = false;
    m_replayingFlag = true;

    // ���������߳�
    if(!m_replayThread)
        m_replayThread = new QRecordReplayThread(this);
    m_replayThread->start();

	qDebug() << __FUNCTION__ << "========== ok";
    showRecrepMsg(tr("����..."));
    // ����״̬��ʾ
    updateStatusDisplay();

    return true;
}

void ImplRecRep::stopReplay ()
{
    if (isReplaying())
    {
        showRecrepMsg(tr("ֹͣ����"));

        autoStopReplayFlag = true;

        if(m_paused)
            pause();

        // �ر���������
        if (m_replayThread)
        {
            if ((!m_replayFinished) && m_replayThread->isRunning()){
                if(!m_replayThread->wait(2000)){
                    qDebug() << "can't stop replay thread, terminate force.";
                    m_replayThread->stop ();
                }
            }
#ifdef Q_OS_LINUX
            //delete m_replayThread;
            //m_replayThread = NULL;
#endif
        }

        showSystemMsg(tr("��������"));
    }

    // �������ݿ��Ʊ�־
    m_replayFinished = true;
    m_replayingFlag = false;

    //  reset replay time
    m_nReplayingTime_100ms = 0;
	m_nReplayTotalTime_100ms = 0;
    m_nReplayStartTime = 0;
    m_nReplayStopTime = 0;

    // �ر������ļ�
    closeReplayFile ();

    if (!toExit)
    {
        // ����״̬��ʾ
        updateStatusDisplay ();
    }
}

// ������ͣ����
void ImplRecRep::pause ()
{
    if (isReplaying())
    {
        // ���µ�ǰʱ��
        m_nReplayComputerTime_100ms = computerTime100ms ();

        if (m_paused)
        {
            m_paused = false;
            m_waitPauseMutex.release();
            showRecrepMsg(tr("��������"));
        }
        else
        {
            m_waitPauseMutex.acquire(m_waitPauseMutex.available());
            m_paused = true;
            showRecrepMsg(tr("��ͣ����"));
        }

        // ����״̬��ʾ
        updateStatusDisplay ();
    }
}

bool ImplRecRep::toReplayFile (const QString& fileName)
{
    try
    {
    //	qDebug() << __FUNCTION__ << fileName;

        // �򿪼�¼�����ļ�
        m_repFile = openFile (fileName);
        if ((!m_repFile) || (!m_repFile->IsValidate()))
            throw (tr("���ļ�ʧ��:��������"));

        // ��ȡ�ļ�����
        //const int fileLength = m_repFile->GetFileLength ();

        // ��ȡ�ļ�ͷ
        const int nRead = fileRead (m_repFile, &m_repFileHeader, sizeofFileHeader, 0);
        m_lRepDataPos = sizeofFileHeader;

        // ��ȡ��Ӧ�ļ�¼ͷ
        qint32 nPos = sizeofFileHeader;
        const qint32 count = m_repFileHeader._Count;
        int sizeofRead, errorCode = 0;

        quint8 dataBuffer[DataBufferSize], tempBuffer[DataBufferSize];
        // ��ʼ����
        for (qint32 i=0; i<count; i++)
        {
            // Ƭ����ʼλ��
            m_lRepDataPos = nPos;
            sizeofRead = fileRead (m_repFile, &m_repDataHeader, sizeofDataHeader, m_lRepDataPos);
            nPos += m_repDataHeader._Length;
            if (sizeofRead != sizeofDataHeader)
                break;

			if(m_repDataHeader._TotalTime == 0)
			m_repDataHeader._TotalTime = 36000;

            //δ����ʼʱ��, ��ȡ��һ��Ƭ��
            const int StopTime = m_repDataHeader._StartTime + m_repDataHeader._TotalTime / SecondTo100ms;
            //const int StopTime = m_repDataHeader._StopTime;
            //if (m_repDataHeader._StopTime < m_nReplayStartTime)
            if (StopTime < m_nReplayStartTime)
                continue;

            // ��������ʱ��,�����
            if (m_repDataHeader._StartTime > m_nReplayStopTime)
                break;

            // Ƭ��������ʼλ��
            m_lRepDataPos += sizeofDataHeader;

            // Ƭ����ʼʱ�� (1s-->100ms)
            const qint32 sectorBaseTime_100ms = (m_repDataHeader._StartTime - m_nReplayStartTime) * SecondTo100ms;
            // Ƭ�ν���ʱ��
            const qint32 sectorStopTime_100ms = (StopTime - m_nReplayStartTime) * SecondTo100ms;
            // ��¼����
            //const qint32 recordNumber = m_repDataHeader._Count;

            // ��ʱ������Ϊ��ǰƬ�ϵ���ʼʱ��,����Ƭǰ���õȴ�
            if (m_nReplayingTime_100ms < sectorBaseTime_100ms)
                m_nReplayingTime_100ms = sectorBaseTime_100ms;

            // ���ݴ���
            int DataIndex_R = 0, tempIndex_W = 0, needData = 0;
            RECDATAINFO *lpRepDataInfo = NULL;
            quint8 dataFlag = 0, stopFlag = 0;
            quint32 DataCount = 0;

            sizeofRead = fileRead (m_repFile, dataBuffer, DataBufferSize, m_lRepDataPos);
            m_lRepDataPos += sizeofRead;

            while (sizeofRead)
            {
                DataIndex_R = 0;

                // ��Ҫ����ǰһ����
                if (tempIndex_W > 0 && needData > 0 && DataIndex_R + needData < sizeofRead)
                {
                    memcpy (&tempBuffer[tempIndex_W], dataBuffer, needData);
                    tempIndex_W += needData;
                    DataIndex_R += needData;
                }

                needData = 0;

                do
                {
                    if(dataFlag == 0)	// to read data info
                    {
                        lpRepDataInfo = NULL;
                        if (tempIndex_W >= sizeofDataInfo)
                        {	// ʹ����������
                            lpRepDataInfo = (RECDATAINFO*) (tempBuffer);
                            tempIndex_W = 0;
                        }
                        else if (DataIndex_R + sizeofDataInfo < sizeofRead)
                        {	// ʹ�ñ�������
                            lpRepDataInfo = (RECDATAINFO*) (&dataBuffer[DataIndex_R]);
                            DataIndex_R += sizeofDataInfo;
                        }
                        else
                        {	// ����һ������
                            needData = sizeofDataInfo - (sizeofRead - DataIndex_R);
                            break;
                        }

                        // data info ok
                        if (lpRepDataInfo && lpRepDataInfo->_Flag == 0xFFFF && lpRepDataInfo->_Length < MaxOneFrameSize)
                        {
                            m_repDataInfo = *lpRepDataInfo;
                            dataFlag = 1;
                        }
                        else
                        {
                            DataIndex_R ++;
                        }
                    }
                    else if (dataFlag == 1)	// to read data
                    {
                        const int sizeofData = m_repDataInfo._Length;
                        quint8 *lpData = NULL;
                        if (tempIndex_W >= sizeofData)
                        {	// ʹ����������
                            lpData = (quint8*) (tempBuffer);
                            tempIndex_W = 0;
                        }
                        else if (DataIndex_R + sizeofData < sizeofRead)
                        {	// ʹ�ñ�������
                            lpData = (quint8*) (&dataBuffer[DataIndex_R]);
                            DataIndex_R += sizeofData;
                        }
                        else
                        {	// ����һ������
                            needData = sizeofData - (sizeofRead - DataIndex_R);
                            break;
                        }

                        // data ok
                        dataFlag = 0;
                        DataCount ++;

                        // ��ǰ����ļ�¼ʱ��
                        const qint32 recTime_100ms = m_repDataInfo._RecTime;// / 10;	// 10ms->100ms
                        // ��ǰ���������ʱ��
                        const qint32 repTime_100ms = sectorBaseTime_100ms + recTime_100ms;

                        // �������ʱ��������ʱ��֮ǰ������
                        if (repTime_100ms < 0)
                            continue;

                        // ʱ���ж�
                        if (needWaiting ())
                        {
                            qint32 dt = repTime_100ms - m_nReplayingTime_100ms;
                            while(dt > 0)
                            {
                                m_waitTimeMutex.tryAcquire(1, (dt * MsecondTo100ms)/replaySpeed());	// 100ms-->ms
                                dt = repTime_100ms - m_nReplayingTime_100ms;
                                if(autoStopReplayFlag){
                                    //qDebug() << "autoStopReplayFlag 1111";
                                    stopFlag = true;
                                    break;
                                }
                            }
                        }

                        if(autoStopReplayFlag){
                            //qDebug() << "autoStopReplayFlag 2222";
                            stopFlag = true;
                            break;
                        }

                        // �ж��Ƿ�����ͣ״̬�������ͣ����ȴ�
                        if (isPaused())
                            m_waitPauseMutex.acquire();

                        // �������ݴ���
                        if(m_dataProcFunc)
                            m_dataProcFunc(QByteArray((const char*)lpData, sizeofData), lpRepDataInfo->_SrcId);

                        // ʱ�䵽,�����
                        const int tm = qMax ((int)repTime_100ms, (int)m_nReplayingTime_100ms);
                        if (tm >= sectorStopTime_100ms || tm >= m_nReplayTotalTime_100ms)
                        {
                            stopFlag = true;
                            break;
                        }

                        if(autoStopReplayFlag){
                            //qDebug() << "autoStopReplayFlag 3333";
                            stopFlag = true;
                            break;
                        }

                        QSemaphore sema(0);
                        sema.tryAcquire(1, 2.5/replaySpeed());

                        // ���̶߳�����Ϣ
                        if ((!needWaiting ()) && (DataCount % 100 == 0))
                        {
                            m_nReplayingTime_100ms = repTime_100ms;
                            #ifdef Q_OS_WIN32
                            QThread::currentThread()->wait (20);
                            #endif
                            #ifdef Q_OS_UNIX
                            //QThread::msleep(20);
                            QSemaphore sema(0);
                            sema.tryAcquire(1, 20);  //wait 20ms
                            #endif
                        }
                    }	// {end else if(data flag == 1)}
                    else
                    {
                        dataFlag = 0;
                    }
                }while(DataIndex_R < sizeofRead);

                // to stop
                if (stopFlag)
                    break;

                // to save additional data
                if (DataIndex_R < sizeofRead)
                {
                    tempIndex_W = sizeofRead-DataIndex_R;
                    memcpy (&tempBuffer[0], &dataBuffer[DataIndex_R], tempIndex_W);
                }

                // to read next
                sizeofRead = fileRead (m_repFile, dataBuffer, DataBufferSize, m_lRepDataPos);
                m_lRepDataPos += sizeofRead;
            }	// { end of while(sizeofRead) }
        }

        closeReplayFile ();

        return true;
    }

    catch (...)
    {
        closeReplayFile ();
        return false;
    }
}


void ImplRecRep::replayProcess ()
{
    foreach (QString filename, m_replayFiles.values())
    {
        showSystemMsg (tr("��ʼ�����ļ�:")+filename);
        toReplayFile (filename.toLocal8Bit());
        showDebugMsg (tr("���������ļ�:")+filename);
    }

    m_nReplayingTime_100ms = m_nReplayTotalTime_100ms;
    // ���ݽ���
    m_replayFinished = true;

    // ��������״̬��ʾ
    updateStatusDisplay();
}

// ������ʱ���ȡ�����ļ�,������ȡ�����ļ�����m_replayFiles�б���,�Թ���������ʹ��
int ImplRecRep::toGetReplayFiles (const QString& filename)
{
    quint32 startTime = 0;
    m_replayFiles.clear ();

    if (QFile::exists(filename))
    {
        if (isInReplayTime(filename, startTime))
            m_replayFiles.insert(startTime, filename);
    }
    else
    {
        bool oneFileFlag = ((m_nReplayStartTime == 0) || (m_nReplayStopTime == 0));

        QDir repDir(m_replayFilePath);
        QStringList filters;
        filters << "*.rec";
        QFileInfoList filist = repDir.entryInfoList(filters, QDir::Files|QDir::NoDotAndDotDot, QDir::Name);
        QFileInfoList::const_iterator it1 = filist.constBegin(), it2 = filist.constEnd();
        for (; it1 != it2; ++it1)
        {
            QString filepath = (*it1).filePath();
            if (isInReplayTime (filepath, startTime))
            {
                m_replayFiles.insert(startTime, filepath);
                if (oneFileFlag)
                    break;
            }
        }
    }

    return m_replayFiles.size();
}

// �ж��ļ��Ƿ���ָ������ʱ����,����ļ���ָ������ʱ����,����������,��ͨ���������ظ��ļ�����ʼʱ��
bool ImplRecRep::isInReplayTime (const QString& fileName, quint32 &fileStartTime)
{
    try
    {
        bool result = false;
        // �򿪼�¼�����ļ�
        m_repFile = openFile (fileName);
        if ((!m_repFile) || (!m_repFile->IsValidate()))
            throw (tr("���ļ�ʧ��:��������"));

        // ��ȡ�ļ�ͷ
        const int nRead = fileRead (m_repFile, &m_repFileHeader, sizeofFileHeader, 0);

        // ��ȡ��Ӧ�ļ�¼ͷ
        qint32 nPos = sizeofFileHeader;
        const qint32 count = m_repFileHeader._Count;

        for (qint32 i=0; i<count; i++)
        {
            const int nRead = fileRead (m_repFile, &m_repDataHeader, sizeofDataHeader, nPos);
            nPos += m_repDataHeader._Length;

			if(m_repDataHeader._TotalTime == 0)
			m_repDataHeader._TotalTime = 36000;

            const int StartTime = m_repDataHeader._StartTime;
            //const int StopTime = m_repDataHeader._StopTime;
            const int StopTime = m_repDataHeader._StartTime + m_repDataHeader._TotalTime / SecondTo100ms;

            // �жϿ�ʼʱ��
            if (m_nReplayStartTime == 0)
                m_nReplayStartTime = StartTime;
            // �жϽ���ʱ��
            if (m_nReplayStopTime == 0)
                m_nReplayStopTime = StopTime;

            // �жϿ�ʼʱ��
            if (StartTime < m_nReplayStopTime &&
                StopTime > m_nReplayStartTime)
            {
                result = true;
                break;
            }
        }

        closeReplayFile ();
        fileStartTime = m_repDataHeader._StartTime;
        return result;
    }

    catch (...)
    {
		printf("ImplRecRep::isInReplayTime(%s) error.", fileName.toLocal8Bit().data());
        closeReplayFile ();
        return false;
    }
}

// ���¼�¼����״̬��ʾ
void ImplRecRep::updateStatusDisplay()
{
    if(m_updateEventId)
    {
        CustomEvent* evt = new CustomEvent((QEvent::Type)m_updateEventId, isReplaying()?1:0);
		QCoreApplication::postEvent (parent(), evt);
    }
}

// �ı������ٶ�
void ImplRecRep::changeReplaySpeed (quint8 speed)
{
    if (m_nRepSpeed != speed)
    {
        m_nRepSpeed = speed;
        m_waitTimeMutex.release();
    }
}


// �򿪼�¼�����ļ�
osFile* ImplRecRep::openFile (const QString& filename)
{
    if (m_recFile)
    {
        QFileInfo fi_a(filename);
        QFileInfo fi_b(m_recFile->GetFileName());
        if (fi_a.absoluteFilePath() == fi_b.absoluteFilePath())
        {
            return (m_recFile);
        }
    }

    if (m_repFile)
    {
        QFileInfo fi_a(filename);
        QFileInfo fi_b(m_repFile->GetFileName());
        if (fi_a.absoluteFilePath() == fi_b.absoluteFilePath())
        {
            return (m_repFile);
        }
    }

    return (new osFile(filename));
}

void ImplRecRep::closeRecordFile ()
{
    if (m_recFile)
    {
        QMutexLocker locker(&m_fileMutex);
        osFile* temp = m_recFile;
        m_recFile = NULL;
        if (temp != m_repFile)
                delete temp;
    }
}

void ImplRecRep::closeReplayFile ()
{
    if (m_repFile)
    {
        QMutexLocker locker(&m_fileMutex);
        osFile* temp = m_repFile;
        m_repFile = NULL;
        if (temp != m_recFile)
                delete temp;
    }
}

void ImplRecRep::closeNormalFile (osFile* lpFile)
{
    if (lpFile)
    {
        QMutexLocker locker(&m_fileMutex);
        osFile* temp = lpFile;
        lpFile = NULL;
        if (temp != m_repFile && temp != m_recFile)
                delete temp;
    }
}

// ��ȡ��¼�ļ�������ͷ
QList<RECDATAHEADER> ImplRecRep::getDataHeaders (const QString&  filename)
{
    return getDataHeaders (openFile(filename));
}

// ��ȡ��¼�ļ�������ͷ
QList<RECDATAHEADER> ImplRecRep::getDataHeaders (osFile* myosFile)
{
    QList<RECDATAHEADER> mylist;

    if (myosFile && myosFile->IsValidate())
    {
        //quint32 fileLength = myosFile->GetFileLength ();
        quint32 sizeofRead = 0;

        RECFILEHEADER fileHeader;
        RECDATAHEADER dataHeader;

        // ��ȡ�ļ�ͷ
        const int nRead = fileRead (myosFile, &fileHeader, sizeofFileHeader, 0);
        // �ж��ļ�ͷ��־
        if (strcmp ((char*)fileHeader._Flag, "mohf") == 0)
        {
            const int count = fileHeader._Count;
            quint32 pos = sizeofFileHeader;
            // ��ȡ����ͷ
            for (int i=0; i<count; ++i)
            {
                    sizeofRead = fileRead (myosFile, &dataHeader, sizeofDataHeader, pos);
                    if (sizeofRead < sizeofDataHeader)// || pos + dataHeader._Length > fileLength)
                        break;
                    mylist << dataHeader;
                    pos += dataHeader._Length;
            }
        }
    }

    // ɾ����ʱ�ļ�
    closeNormalFile (myosFile);

    return mylist;
}

// �����ļ�ͷ���ݿ鵽�ļ��У����Ƿ����ļ�����ǰ��
void ImplRecRep::writeFileHeader ()
{
    QMutexLocker locker(&m_fileMutex);
    m_recFile->SetPosition(0, osFile::os_seek_bgn);
    m_recFile->Write(&m_recFileHeader, sizeofFileHeader);
}

// ��������ͷ���ļ���
void ImplRecRep::writeDataHeader (qint32 pos)
{
    QMutexLocker locker(&m_fileMutex);
    m_recFile->SetPosition(pos, osFile::os_seek_bgn);
    m_recFile->Write(&m_recDataHeader, sizeofDataHeader);
}

// �������ݵ��ļ���
void ImplRecRep::writeData (const char* pData, qint32 length)
{
    const int sizeofData = sizeofDataInfo + length + 1;

    // ������ݻ�������,�򽫻���������д���ļ�
    if (m_recWriteIndex + sizeofData >= DataBufferSize)
    {
        writeRecordDataToFile ();
    }

    // ������д�뻺��
    memcpy (&m_recDataBuffer[m_recWriteIndex], &m_recDataInfo, sizeofDataInfo);
    m_recWriteIndex += sizeofDataInfo;
    memcpy (&m_recDataBuffer[m_recWriteIndex], pData, length);
    m_recWriteIndex += length;

	// write data to file directly
	//writeRecordDataToFile();
}

void ImplRecRep::writeRecordDataToFile ()
{
    if (m_recWriteIndex)
    {
        // �ļ�ͷ
        writeFileHeader();
        // ����ͷ
        writeDataHeader(m_recDataHeader._Location);
        // ����
        QMutexLocker locker(&m_fileMutex);
        m_recFile->SetPosition(0, osFile::os_seek_end);
        m_recFile->Write(m_recDataBuffer, m_recWriteIndex);
        m_recWriteIndex = 0;
    }
}

int ImplRecRep::fileRead (osFile* lpFile, void* pData, int sizeofData, int offset, quint32 dwFlag)
{
    QMutexLocker locker(&m_fileMutex);
    int sizeofRead = 0;
    if (lpFile)
    {
        lpFile->SetPosition (offset, dwFlag);
        sizeofRead = lpFile->Read(pData, sizeofData);
    }
    return sizeofRead;
}

int ImplRecRep::fileWrite (osFile* lpFile, void* pData, int sizeofData, int offset, quint32 dwFlag)
{
    QMutexLocker locker(&m_fileMutex);
    int sizeofRead = 0;
    if (lpFile)
    {
        lpFile->SetPosition (offset, dwFlag);
        sizeofRead = lpFile->Write(pData, sizeofData);
    }
    return sizeofRead;
}


void QRecordReplayThread::run()
{
    ImplRecRep * recrep = (ImplRecRep*)(parent());
    if(recrep)
        recrep->replayProcess();
}

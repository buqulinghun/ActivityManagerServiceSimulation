
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
* 记录重演处理类的实现
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

    // 默认在当前路径
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

// 获取当前系统时间
//quint32 ImplRecRep::systemTime () const
//{
    //return SystemInfo.SystemDateTime.toTime_t();
//    return time(NULL);
//}

// 该函数只在记录时有用，可用使用计算机实际时间
quint32 ImplRecRep::computerTime () const
{
    return time(0);
}

// 获取当前系统时间100ms
qint64 ImplRecRep::computerTime100ms () const
{
    QDateTime dt = QDateTime::currentDateTime();
    return dt.toTime_t() * SecondTo100ms + dt.time().msec() / MsecondTo100ms;
//	return time(0) * SecondTo100ms + QTime::currentTime().msec() / MsecondTo100ms;
}

// 获取时间字符串表示
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

// 路径字符串后面的/判断
QString PathWithFlash(const QString& path)
{
    const QString endchar = "/";

    if(path.endsWith(endchar))
        return path;
    else
        return path + endchar;
}

////////////////////////////////////////////////////////////////////
// 设置记录重演主目录
void ImplRecRep::setRecordPath(const QString& path)
{
    m_replayFilePath = PathWithFlash(path);

    QDir dir(m_replayFilePath);
    if (!dir.exists())
            dir.mkpath (m_replayFilePath);
}

// 开始记录
bool ImplRecRep::startRecord ()
{
    const QDateTime dt = QDateTime::currentDateTime();
    QString filename = m_replayFilePath + getTimeString(dt) + tr(".rec");
	const char * tempFileName = filename.toLocal8Bit().data();
    if (startRecord(filename))
    {
        showRecrepMsg(tr("记录..."));
        // 更新状态显示
        updateStatusDisplay();
        return true;
    }
    else
    {
        return false;
    }
}

// 开始记录
bool ImplRecRep::startRecord (const QString& fileName)
{	
    // 如果当前处于记录状态，则停止
    if (isRecording())
        stopRecord ();

    // 锁定记录状态
    QMutexLocker locker(&m_recordingMutex);

    // 申请记录数据缓存区
    if (!m_recDataBuffer)
        m_recDataBuffer = new quint8 [DataBufferSize];
    m_recWriteIndex = 0;

    // 打开文件句柄
    m_recFile = openFile (fileName);
    if ((!m_recFile) || (!m_recFile->IsValidate()))
    {
        showSystemMsg(tr("打开文件失败:不能记录"));
        closeRecordFile ();
        return false;
    }

    // 获取当前时间
    time_t tm0 = computerTime();//time(NULL);

    // 读取文件长度
    qint32 length = m_recFile->GetFileLength();
    qint32 nextPos = length;
    bool AddRecTimes = FALSE;

    // 如果文件长度小于文件头的长度，则重新创建文件
    if (length < sizeofFileHeader)
    {	// 初始化文件头数据
        memset (&m_recFileHeader, 0, sizeofFileHeader);
        strcpy ((char*)m_recFileHeader._Flag, "hktk");
        m_recFileHeader._Count = 1;
        m_recFileHeader._Length = sizeofFileHeader;
        // 设置起始位置为文件头之后的第1个字节
        nextPos = sizeofFileHeader;
        m_recFileHeader._LocDataHeader = nextPos;
    }
    else
    {
        // 读取文件头数据
        const int nRead = fileRead (m_recFile, &m_recFileHeader, sizeofFileHeader, 0);
        if(nRead != sizeofFileHeader)
        {
            closeRecordFile ();
            return false;
        }

        // 判断最后一次输入的记录头是否为空
        if (length >= m_recFileHeader._LocDataHeader + sizeofDataHeader)
        {
            const int sizeofRead = fileRead (m_recFile, &m_recDataHeader, sizeofDataHeader, m_recFileHeader._LocDataHeader);
            if (sizeofRead == sizeofDataHeader && m_recDataHeader._Count > 0)
            {	// 上次记录非空，添加新的记录头
                //nextPos = m_recDataHeader._Location;
                AddRecTimes = TRUE;
            }
        }

        // 如果使用新的记录，则记录次数加1，否则使用上次记录头的位置
        if (AddRecTimes)
            m_recFileHeader._Count ++;
        else
            nextPos = m_recDataHeader._Location;
    }

    // 文件数据长度需要加上记录头的长度
    m_recFileHeader._Length = nextPos + sizeofDataHeader;
    // 记录最新记录的起始位置
    m_recFileHeader._LocDataHeader = nextPos;

    // 保存文件头数据块到文件中
    writeFileHeader ();

    // 初始化数据头
    memset (&m_recDataHeader, 0, sizeofDataHeader);
    m_recDataHeader._StartTime = tm0;
    m_recDataHeader._StopTime = tm0;
    m_recDataHeader._Count = 0;
    m_recDataHeader._Length = sizeofDataHeader;
    m_recDataHeader._Location = nextPos;

    // 保存记录头到文件中
    writeDataHeader(nextPos);

    // 数据标志
    m_recDataInfo._Flag = 0xFFFF;

    m_nRecordComputerTime_100ms = computerTime100ms ();
    m_nRecordingTime_100ms = 0;
    m_recordingFlag = true;

    return TRUE;
}

// 停止记录
void ImplRecRep::stopRecord ()
{
    // 锁定记录状态
    QMutexLocker locker(&m_recordingMutex);

    // 将缓冲数据保存到文件
    writeRecordDataToFile ();

    m_recordingFlag = false;

    // 关闭打开的文件句柄
    closeRecordFile ();

    showRecrepMsg(tr("停止记录"));
    // 更新状态显示
    updateStatusDisplay();
}

// 记录数据
bool ImplRecRep::recording (const char* pData, qint32 nLength, quint16 srcid)
{
    // 数据长度错误
    if (nLength > MaxOneFrameSize)
            return false;

    // 如果当前不在记录状态，则返回错误
    if (!isRecording())
        return FALSE;

    // 一天，使用一个新的文件
    if (m_nRecordingTime_100ms >= 288000l)	// 8640000l = 24小时 = 24 * 3600 * 10 100ms
        startRecord ();

    // 锁定记录状态
    QMutexLocker locker(&m_recordingMutex);

    // 数据总长度 = 数据头的长度(6)+数据的长度(nLength)
    const int sizeofData = sizeofDataInfo + nLength;

    // 更新文件头的长度(+数据总长度)
    m_recFileHeader._Length += sizeofData;
//	writeFileHeader();

    // 保存批号标志
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
	{	// 如果时间异常，则重新计算时间
		stopTime = m_recDataHeader._StopTime + (m_nRecordingTime_100ms-m_recDataHeader._TotalTime) / 10;
	}

    // 更新记录头
    m_recDataHeader._Count ++;
    m_recDataHeader._StopTime = stopTime;
    m_recDataHeader._TotalTime = m_nRecordingTime_100ms;
    m_recDataHeader._Length += sizeofData;
//	writeDataHeader(m_recDataHeader._Location);

    // 保存记录命令
    //m_recDataInfo._RecTime = tm0;
    m_recDataInfo._SrcId = srcid;   // 数据来源
    m_recDataInfo._RecTime = m_nRecordingTime_100ms;	// 保存100毫秒相对时间
    m_recDataInfo._Length = nLength;
    writeData (pData, nLength);

    return TRUE;
}

////////////////////////////////////////////////////////////////////
// replaying process

// 开始重演(从指定文件开始重演)
bool ImplRecRep::startReplay (const QString& fileName, qint32 start_tm, qint32 stop_tm)
{
    if (isReplaying())
    {
        showSystemMsg (tr("正在重演"));
        return false;
    }

    // 保存时间
    m_nReplayStartTime = start_tm;
    m_nReplayStopTime = stop_tm >= 0 ? stop_tm : computerTime() - 1;

    // 2015年1月1日00:00:01
    qint32 lt = QDateTime::fromString("M1d1y201500:00:01","'M'M'd'd'y'yyyyhh:mm:ss").toTime_t();
    // 时间错误
    if (m_nReplayStartTime > 0 &&
        (m_nReplayStartTime <= lt ||
        (m_nReplayStopTime > 0 && m_nReplayStartTime >= m_nReplayStopTime)) )
        return false;

    // 由时间查找对应的记录文件
    int count = toGetReplayFiles (fileName);
	if (count <= 0){
		qDebug() << __FUNCTION__ << "can't find replay file.";
        return false;
	}

    autoStopReplayFlag = false;
    // 设置控制参数并启动重演线程
    toStartReplay ();

    return true;
}

// 设置控制参数并启动重演线程
bool ImplRecRep::toStartReplay ()
{
    // 如果已经在重演,则结束
    if (isReplaying())
        return false;//stopReplay ();

    // 设置控制参数
    m_nReplayTotalTime_100ms =  (m_nReplayStopTime - m_nReplayStartTime) * SecondTo100ms;
    m_nReplayComputerTime_100ms = computerTime100ms ();
    m_nReplayingTime_100ms = 0;
    m_paused = false;
    m_replayFinished = false;
    m_replayingFlag = true;

    // 启动重演线程
    if(!m_replayThread)
        m_replayThread = new QRecordReplayThread(this);
    m_replayThread->start();

	qDebug() << __FUNCTION__ << "========== ok";
    showRecrepMsg(tr("重演..."));
    // 更新状态显示
    updateStatusDisplay();

    return true;
}

void ImplRecRep::stopReplay ()
{
    if (isReplaying())
    {
        showRecrepMsg(tr("停止重演"));

        autoStopReplayFlag = true;

        if(m_paused)
            pause();

        // 关闭重演任务
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

        showSystemMsg(tr("结束重演"));
    }

    // 设置重演控制标志
    m_replayFinished = true;
    m_replayingFlag = false;

    //  reset replay time
    m_nReplayingTime_100ms = 0;
	m_nReplayTotalTime_100ms = 0;
    m_nReplayStartTime = 0;
    m_nReplayStopTime = 0;

    // 关闭重演文件
    closeReplayFile ();

    if (!toExit)
    {
        // 更新状态显示
        updateStatusDisplay ();
    }
}

// 重演暂停处理
void ImplRecRep::pause ()
{
    if (isReplaying())
    {
        // 更新当前时间
        m_nReplayComputerTime_100ms = computerTime100ms ();

        if (m_paused)
        {
            m_paused = false;
            m_waitPauseMutex.release();
            showRecrepMsg(tr("继续重演"));
        }
        else
        {
            m_waitPauseMutex.acquire(m_waitPauseMutex.available());
            m_paused = true;
            showRecrepMsg(tr("暂停重演"));
        }

        // 更新状态显示
        updateStatusDisplay ();
    }
}

bool ImplRecRep::toReplayFile (const QString& fileName)
{
    try
    {
    //	qDebug() << __FUNCTION__ << fileName;

        // 打开记录数据文件
        m_repFile = openFile (fileName);
        if ((!m_repFile) || (!m_repFile->IsValidate()))
            throw (tr("打开文件失败:不能重演"));

        // 获取文件长度
        //const int fileLength = m_repFile->GetFileLength ();

        // 读取文件头
        const int nRead = fileRead (m_repFile, &m_repFileHeader, sizeofFileHeader, 0);
        m_lRepDataPos = sizeofFileHeader;

        // 读取对应的记录头
        qint32 nPos = sizeofFileHeader;
        const qint32 count = m_repFileHeader._Count;
        int sizeofRead, errorCode = 0;

        quint8 dataBuffer[DataBufferSize], tempBuffer[DataBufferSize];
        // 开始重演
        for (qint32 i=0; i<count; i++)
        {
            // 片断起始位置
            m_lRepDataPos = nPos;
            sizeofRead = fileRead (m_repFile, &m_repDataHeader, sizeofDataHeader, m_lRepDataPos);
            nPos += m_repDataHeader._Length;
            if (sizeofRead != sizeofDataHeader)
                break;

			if(m_repDataHeader._TotalTime == 0)
			m_repDataHeader._TotalTime = 36000;

            //未到开始时间, 则取下一个片段
            const int StopTime = m_repDataHeader._StartTime + m_repDataHeader._TotalTime / SecondTo100ms;
            //const int StopTime = m_repDataHeader._StopTime;
            //if (m_repDataHeader._StopTime < m_nReplayStartTime)
            if (StopTime < m_nReplayStartTime)
                continue;

            // 超过结束时间,则结束
            if (m_repDataHeader._StartTime > m_nReplayStopTime)
                break;

            // 片断数据起始位置
            m_lRepDataPos += sizeofDataHeader;

            // 片段启始时间 (1s-->100ms)
            const qint32 sectorBaseTime_100ms = (m_repDataHeader._StartTime - m_nReplayStartTime) * SecondTo100ms;
            // 片段结束时间
            const qint32 sectorStopTime_100ms = (StopTime - m_nReplayStartTime) * SecondTo100ms;
            // 记录总数
            //const qint32 recordNumber = m_repDataHeader._Count;

            // 将时间设置为当前片断的起始时间,即在片前不用等待
            if (m_nReplayingTime_100ms < sectorBaseTime_100ms)
                m_nReplayingTime_100ms = sectorBaseTime_100ms;

            // 重演处理
            int DataIndex_R = 0, tempIndex_W = 0, needData = 0;
            RECDATAINFO *lpRepDataInfo = NULL;
            quint8 dataFlag = 0, stopFlag = 0;
            quint32 DataCount = 0;

            sizeofRead = fileRead (m_repFile, dataBuffer, DataBufferSize, m_lRepDataPos);
            m_lRepDataPos += sizeofRead;

            while (sizeofRead)
            {
                DataIndex_R = 0;

                // 需要链接前一数据
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
                        {	// 使用链接数据
                            lpRepDataInfo = (RECDATAINFO*) (tempBuffer);
                            tempIndex_W = 0;
                        }
                        else if (DataIndex_R + sizeofDataInfo < sizeofRead)
                        {	// 使用本组数据
                            lpRepDataInfo = (RECDATAINFO*) (&dataBuffer[DataIndex_R]);
                            DataIndex_R += sizeofDataInfo;
                        }
                        else
                        {	// 读下一组数据
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
                        {	// 使用链接数据
                            lpData = (quint8*) (tempBuffer);
                            tempIndex_W = 0;
                        }
                        else if (DataIndex_R + sizeofData < sizeofRead)
                        {	// 使用本组数据
                            lpData = (quint8*) (&dataBuffer[DataIndex_R]);
                            DataIndex_R += sizeofData;
                        }
                        else
                        {	// 读下一组数据
                            needData = sizeofData - (sizeofRead - DataIndex_R);
                            break;
                        }

                        // data ok
                        dataFlag = 0;
                        DataCount ++;

                        // 当前命令的记录时间
                        const qint32 recTime_100ms = m_repDataInfo._RecTime;// / 10;	// 10ms->100ms
                        // 当前命令的重演时间
                        const qint32 repTime_100ms = sectorBaseTime_100ms + recTime_100ms;

                        // 如果数据时间在重演时间之前，则丢弃
                        if (repTime_100ms < 0)
                            continue;

                        // 时间判断
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

                        // 判断是否处于暂停状态，如果暂停，则等待
                        if (isPaused())
                            m_waitPauseMutex.acquire();

                        // 重演数据处理
                        if(m_dataProcFunc)
                            m_dataProcFunc(QByteArray((const char*)lpData, sizeofData), lpRepDataInfo->_SrcId);

                        // 时间到,则结束
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

                        // 让线程额外休息
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
        showSystemMsg (tr("开始重演文件:")+filename);
        toReplayFile (filename.toLocal8Bit());
        showDebugMsg (tr("结束重演文件:")+filename);
    }

    m_nReplayingTime_100ms = m_nReplayTotalTime_100ms;
    // 重演结束
    m_replayFinished = true;

    // 更新重演状态显示
    updateStatusDisplay();
}

// 由重演时间获取重演文件,并将获取到的文件插入m_replayFiles列表中,以供重演任务使用
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

// 判断文件是否在指定重演时间内,如果文件在指定重演时间内,则函数返回真,并通过参数返回该文件的起始时间
bool ImplRecRep::isInReplayTime (const QString& fileName, quint32 &fileStartTime)
{
    try
    {
        bool result = false;
        // 打开记录数据文件
        m_repFile = openFile (fileName);
        if ((!m_repFile) || (!m_repFile->IsValidate()))
            throw (tr("打开文件失败:不能重演"));

        // 读取文件头
        const int nRead = fileRead (m_repFile, &m_repFileHeader, sizeofFileHeader, 0);

        // 读取对应的记录头
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

            // 判断开始时间
            if (m_nReplayStartTime == 0)
                m_nReplayStartTime = StartTime;
            // 判断结束时间
            if (m_nReplayStopTime == 0)
                m_nReplayStopTime = StopTime;

            // 判断开始时间
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

// 更新记录重演状态显示
void ImplRecRep::updateStatusDisplay()
{
    if(m_updateEventId)
    {
        CustomEvent* evt = new CustomEvent((QEvent::Type)m_updateEventId, isReplaying()?1:0);
		QCoreApplication::postEvent (parent(), evt);
    }
}

// 改变重演速度
void ImplRecRep::changeReplaySpeed (quint8 speed)
{
    if (m_nRepSpeed != speed)
    {
        m_nRepSpeed = speed;
        m_waitTimeMutex.release();
    }
}


// 打开记录数据文件
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

// 获取记录文件的数据头
QList<RECDATAHEADER> ImplRecRep::getDataHeaders (const QString&  filename)
{
    return getDataHeaders (openFile(filename));
}

// 获取记录文件的数据头
QList<RECDATAHEADER> ImplRecRep::getDataHeaders (osFile* myosFile)
{
    QList<RECDATAHEADER> mylist;

    if (myosFile && myosFile->IsValidate())
    {
        //quint32 fileLength = myosFile->GetFileLength ();
        quint32 sizeofRead = 0;

        RECFILEHEADER fileHeader;
        RECDATAHEADER dataHeader;

        // 读取文件头
        const int nRead = fileRead (myosFile, &fileHeader, sizeofFileHeader, 0);
        // 判断文件头标志
        if (strcmp ((char*)fileHeader._Flag, "mohf") == 0)
        {
            const int count = fileHeader._Count;
            quint32 pos = sizeofFileHeader;
            // 读取数据头
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

    // 删除临时文件
    closeNormalFile (myosFile);

    return mylist;
}

// 保存文件头数据块到文件中，总是放在文件的最前面
void ImplRecRep::writeFileHeader ()
{
    QMutexLocker locker(&m_fileMutex);
    m_recFile->SetPosition(0, osFile::os_seek_bgn);
    m_recFile->Write(&m_recFileHeader, sizeofFileHeader);
}

// 保存数据头到文件中
void ImplRecRep::writeDataHeader (qint32 pos)
{
    QMutexLocker locker(&m_fileMutex);
    m_recFile->SetPosition(pos, osFile::os_seek_bgn);
    m_recFile->Write(&m_recDataHeader, sizeofDataHeader);
}

// 保存数据到文件中
void ImplRecRep::writeData (const char* pData, qint32 length)
{
    const int sizeofData = sizeofDataInfo + length + 1;

    // 如果数据缓冲区满,则将缓冲区数据写入文件
    if (m_recWriteIndex + sizeofData >= DataBufferSize)
    {
        writeRecordDataToFile ();
    }

    // 将数据写入缓冲
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
        // 文件头
        writeFileHeader();
        // 数据头
        writeDataHeader(m_recDataHeader._Location);
        // 数据
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

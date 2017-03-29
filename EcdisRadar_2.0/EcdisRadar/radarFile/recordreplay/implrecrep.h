#ifndef IMPLRECREP_H
#define IMPLRECREP_H

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QSemaphore>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QDateTime>
#include <QtCore/QThread>

#include "osfile.h"
#include "recordreplay_global.h"

class ImplRecRep : public QObject
{
Q_OBJECT
public:
    explicit ImplRecRep(QObject *parent = 0);
    ~ImplRecRep ();

signals:
    void stopSignal ();

public slots:
    void stopSlot ();

    // 外部调用接口
public:
    // 设置记录重演主目录
    void setRecordPath(const QString& path);
    // 设置重演处理函数
    void setReplayProcessFunc(DATAPROCESSFUNC func)
    {   m_dataProcFunc = func;      }
    // 设置更新事件ID
    void setUpdateEventId(int id)
    {   m_updateEventId = id;   }
    // 设置等待模式
    void setWaitMode(bool flag)
    {   m_waitMode = flag;  }


    // 开始记录
    bool startRecord ();
    bool startRecord (const QString& fileName);
    // 记录数据
    bool recording (const char* pData, qint32 nLength, quint16 srcid);
    // 停止记录
    void stopRecord ();

    // 开始重演(如果指定文件有效,则重演该文件.否则按指定时间,自动寻找相应的文件进行重演)
    bool startReplay (const QString& fileName, qint32 start_tm=0, qint32 stop_tm=0);
    // 停止重演
    void stopReplay ();
    // 重演暂停处理
    void pause ();
    // 改变重演速度
    void changeReplaySpeed (quint8 speed);
    // 重演处理
    void replayProcess ();

    // 返回重演速度
    quint8 replaySpeed() const
    {	return m_nRepSpeed > 0 ? m_nRepSpeed : 1;		}
    // 判断记录标志
    bool isRecording () const
    {	return m_recordingFlag;	}
    // 判断重演标志
    bool isReplaying () const
    {	return m_replayingFlag;	}
    // 返回当前暂停标志
    bool isPaused() const
    {	return m_paused;	}

    // 获取记录文件的数据头
    QList<RECDATAHEADER> getDataHeaders (const QString& filename);

    // 更新记录相对时间(输入参数为100ms相对时间)
    void updateComputerTime(quint32 delTime)
    {
        const qint64 lastComputerTime = m_computerTime_100ms;
        m_computerTime_100ms = computerTime100ms ();

        // 由当前时间和上一次的时间,判断计算机时间是否被人为的修改过
        const qint64 dt = m_computerTime_100ms - lastComputerTime;
        const bool computerTimeChangedFlag = (lastComputerTime > 0 && (dt < -1 || dt > 100));

        if (isRecording())
        {
            if (computerTimeChangedFlag)
            {	// 时间改变,重新开始记录
                startRecord ();
            }
            else
            {	// 正常,更新记录时间
            m_nRecordingTime_100ms += m_computerTime_100ms - m_nRecordComputerTime_100ms;
            // 更新记录当前时间
            m_nRecordComputerTime_100ms = m_computerTime_100ms;
            }
        }

        if (isReplaying() && needWaiting() && (!isPaused()) && (!m_replayFinished))
        {
            // 计算重演过去的时间
            if (computerTimeChangedFlag)
                // 如果时间改变,则由计算机时间无效,需要由输入的时间差计算重演过去的时间
                m_nReplayingTime_100ms += delTime * m_nRepSpeed;
            else
                // 由计算机时间计算重演过去的时间
                m_nReplayingTime_100ms += (m_computerTime_100ms - m_nReplayComputerTime_100ms) * m_nRepSpeed;
            // 更新重演当前时间
            m_nReplayComputerTime_100ms = m_computerTime_100ms;
        }
    }

    // 设置重演时间
    void setReplayTime (quint32 tm)
    {
// 重演时不能人工修改重演时间
#if 0
        if (isNormalReplay ())
        {
            if (m_nReplayingTime_100ms < tm)
            {	// 重演时间往后调
                m_nReplayingTime_100ms = tm;
                m_waitTimeMutex.release ();
            }
            else
            {	// 重演时间往前调
            }
        }
#endif
    }

	quint32 crntReplayTime() const
	{
		return m_nReplayStartTime + m_nReplayingTime_100ms / 10;
	}

    // 获取重演时间
    quint32 getReplayTime () const
    {	return m_nReplayingTime_100ms;		}

    qint32 getReplayTotalTime () const
    {	return 	m_nReplayTotalTime_100ms;	}


    // 内部处理接口
protected:
    bool toStartReplay ();
    bool toReplayFile (const QString& fileName);

    int toGetReplayFiles (const QString& fileName = QString());
    // 由重演时间获取重演文件
    //int ToGetReplayFiles ();
    // 判断文件是否在指定重演时间内
    bool isInReplayTime (const QString& fileName, quint32 &startTime);

private:
    // 保存文件头到文件中
    void writeFileHeader ();
    // 保存数据头到文件中
    void writeDataHeader (qint32 pos);
    // 保存数据到文件中
    void writeData (const char* pData, qint32 length);
    // 将记录缓冲区中的数据写入文件
    void writeRecordDataToFile ();

    inline int fileRead  (osFile* lpFile, void* pData, int sizeofData, int offset, quint32 dwFlag=osFile::os_seek_bgn);
    inline int fileWrite (osFile* lpFile, void* pData, int sizeofData, int offset, quint32 dwFlag=osFile::os_seek_bgn);

    // 获取时间字符串表示
    QString getTimeString (const QDateTime& dt) const;

    QList<RECDATAHEADER> getDataHeaders (osFile*);

    osFile* openFile (const QString& filename);
    void closeRecordFile ();
    void closeReplayFile ();
    void closeNormalFile (osFile* lpFile);

    bool needWaiting () const
    {
        return m_waitMode;
    }

    // 更新记录重演状态显示
    void updateStatusDisplay();

    // 获取当前系统时间(100ms)
    qint64 computerTime100ms () const;
    // 获取当前计算机时间(秒)
    inline quint32 computerTime () const;
    // 获取当前系统时间 s
    //inline quint32 systemTime () const;

    // 私有数据
private:
    // 记录重演文件IO对象
    osFile*	m_recFile;
    osFile*	m_repFile;
    //osFile*	m_outFile;

    bool	m_recordingFlag;    // 记录标志
    bool	m_replayingFlag;    // 重演标志

    qint64	m_nRecordComputerTime_100ms;	// 当前记录的计算机时间(10ms)
    qint64	m_nReplayComputerTime_100ms;	// 当前重演的计算机时间(10ms)
    qint64	m_computerTime_100ms;			// 当前计算机时间

    qint32	m_nReplayStartTime;	// 重演起始时间(UTC 单位:秒)
    qint32	m_nReplayStopTime;	// 重演结束时间
    qint32	m_nReplayTotalTime_100ms;	// 重演总时间
    qint32	m_nReplayingTime_100ms;

    // 记录相对时间：从开始记录开始记时 (really 10ms)
    qint32	m_nRecordingTime_100ms;

    quint8  m_waitMode;     // 重演模式 0:正常重演,1:数据输出
    quint8	m_nRepSpeed;		// 重演速度 1:正常，2:2倍速，4:4倍速
    qint32	m_lRepDataPos;		// 重演数据文件中的指针位置，指标当前读取数据的位置

    // 重演文件列表
    QMap<quint32, QString>	m_replayFiles;

    // 记录文件头数据
    RECFILEHEADER	m_recFileHeader;
    // 记录数据头
    RECDATAHEADER	m_recDataHeader;
    // 当前记录命令数据
    RECDATAINFO		m_recDataInfo;

    // 重演文件头数据
    RECFILEHEADER	m_repFileHeader;
    // 重演数据头
    RECDATAHEADER	m_repDataHeader;
    // 当前重演命令数据
    RECDATAINFO		m_repDataInfo;

    // 记录数据缓存区
    quint8*			m_recDataBuffer;
    quint16			m_recWriteIndex;

    // 用于重演等待时间的信号量
    QSemaphore		m_waitTimeMutex;
    // 用于重演暂停控制的信号量
    QSemaphore		m_waitPauseMutex;
    // 用于记录状态的信号量
    QMutex			m_recordingMutex;
    QMutex			m_replayingMutex;
    // 文件互斥访问
    QMutex			m_fileMutex;
    // 重演暂停标志
    bool		m_paused;
    // 重演状态
    quint8		m_replayMode;	// 重演状态：0：正常重演，1：显示静态航迹，2：保存航迹数据
    bool		m_replayFinished;

    QString		m_replayFilePath;

    class QRecordReplayThread* m_replayThread;

    DATAPROCESSFUNC m_dataProcFunc;
    int     m_updateEventId;
};

// 记录重演处理线程
class QRecordReplayThread : public QThread
{
protected:
    void run ();

public:
    QRecordReplayThread(QObject* parent=NULL)
        : QThread(parent)
    {}

    void stop ()
    {
        setTerminationEnabled (true);
        terminate ();
    }

};

#endif // IMPLRECREP_H

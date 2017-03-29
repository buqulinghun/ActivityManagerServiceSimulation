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

    // �ⲿ���ýӿ�
public:
    // ���ü�¼������Ŀ¼
    void setRecordPath(const QString& path);
    // �������ݴ�����
    void setReplayProcessFunc(DATAPROCESSFUNC func)
    {   m_dataProcFunc = func;      }
    // ���ø����¼�ID
    void setUpdateEventId(int id)
    {   m_updateEventId = id;   }
    // ���õȴ�ģʽ
    void setWaitMode(bool flag)
    {   m_waitMode = flag;  }


    // ��ʼ��¼
    bool startRecord ();
    bool startRecord (const QString& fileName);
    // ��¼����
    bool recording (const char* pData, qint32 nLength, quint16 srcid);
    // ֹͣ��¼
    void stopRecord ();

    // ��ʼ����(���ָ���ļ���Ч,�����ݸ��ļ�.����ָ��ʱ��,�Զ�Ѱ����Ӧ���ļ���������)
    bool startReplay (const QString& fileName, qint32 start_tm=0, qint32 stop_tm=0);
    // ֹͣ����
    void stopReplay ();
    // ������ͣ����
    void pause ();
    // �ı������ٶ�
    void changeReplaySpeed (quint8 speed);
    // ���ݴ���
    void replayProcess ();

    // ���������ٶ�
    quint8 replaySpeed() const
    {	return m_nRepSpeed > 0 ? m_nRepSpeed : 1;		}
    // �жϼ�¼��־
    bool isRecording () const
    {	return m_recordingFlag;	}
    // �ж����ݱ�־
    bool isReplaying () const
    {	return m_replayingFlag;	}
    // ���ص�ǰ��ͣ��־
    bool isPaused() const
    {	return m_paused;	}

    // ��ȡ��¼�ļ�������ͷ
    QList<RECDATAHEADER> getDataHeaders (const QString& filename);

    // ���¼�¼���ʱ��(�������Ϊ100ms���ʱ��)
    void updateComputerTime(quint32 delTime)
    {
        const qint64 lastComputerTime = m_computerTime_100ms;
        m_computerTime_100ms = computerTime100ms ();

        // �ɵ�ǰʱ�����һ�ε�ʱ��,�жϼ����ʱ���Ƿ���Ϊ���޸Ĺ�
        const qint64 dt = m_computerTime_100ms - lastComputerTime;
        const bool computerTimeChangedFlag = (lastComputerTime > 0 && (dt < -1 || dt > 100));

        if (isRecording())
        {
            if (computerTimeChangedFlag)
            {	// ʱ��ı�,���¿�ʼ��¼
                startRecord ();
            }
            else
            {	// ����,���¼�¼ʱ��
            m_nRecordingTime_100ms += m_computerTime_100ms - m_nRecordComputerTime_100ms;
            // ���¼�¼��ǰʱ��
            m_nRecordComputerTime_100ms = m_computerTime_100ms;
            }
        }

        if (isReplaying() && needWaiting() && (!isPaused()) && (!m_replayFinished))
        {
            // �������ݹ�ȥ��ʱ��
            if (computerTimeChangedFlag)
                // ���ʱ��ı�,���ɼ����ʱ����Ч,��Ҫ�������ʱ���������ݹ�ȥ��ʱ��
                m_nReplayingTime_100ms += delTime * m_nRepSpeed;
            else
                // �ɼ����ʱ��������ݹ�ȥ��ʱ��
                m_nReplayingTime_100ms += (m_computerTime_100ms - m_nReplayComputerTime_100ms) * m_nRepSpeed;
            // �������ݵ�ǰʱ��
            m_nReplayComputerTime_100ms = m_computerTime_100ms;
        }
    }

    // ��������ʱ��
    void setReplayTime (quint32 tm)
    {
// ����ʱ�����˹��޸�����ʱ��
#if 0
        if (isNormalReplay ())
        {
            if (m_nReplayingTime_100ms < tm)
            {	// ����ʱ�������
                m_nReplayingTime_100ms = tm;
                m_waitTimeMutex.release ();
            }
            else
            {	// ����ʱ����ǰ��
            }
        }
#endif
    }

	quint32 crntReplayTime() const
	{
		return m_nReplayStartTime + m_nReplayingTime_100ms / 10;
	}

    // ��ȡ����ʱ��
    quint32 getReplayTime () const
    {	return m_nReplayingTime_100ms;		}

    qint32 getReplayTotalTime () const
    {	return 	m_nReplayTotalTime_100ms;	}


    // �ڲ�����ӿ�
protected:
    bool toStartReplay ();
    bool toReplayFile (const QString& fileName);

    int toGetReplayFiles (const QString& fileName = QString());
    // ������ʱ���ȡ�����ļ�
    //int ToGetReplayFiles ();
    // �ж��ļ��Ƿ���ָ������ʱ����
    bool isInReplayTime (const QString& fileName, quint32 &startTime);

private:
    // �����ļ�ͷ���ļ���
    void writeFileHeader ();
    // ��������ͷ���ļ���
    void writeDataHeader (qint32 pos);
    // �������ݵ��ļ���
    void writeData (const char* pData, qint32 length);
    // ����¼�������е�����д���ļ�
    void writeRecordDataToFile ();

    inline int fileRead  (osFile* lpFile, void* pData, int sizeofData, int offset, quint32 dwFlag=osFile::os_seek_bgn);
    inline int fileWrite (osFile* lpFile, void* pData, int sizeofData, int offset, quint32 dwFlag=osFile::os_seek_bgn);

    // ��ȡʱ���ַ�����ʾ
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

    // ���¼�¼����״̬��ʾ
    void updateStatusDisplay();

    // ��ȡ��ǰϵͳʱ��(100ms)
    qint64 computerTime100ms () const;
    // ��ȡ��ǰ�����ʱ��(��)
    inline quint32 computerTime () const;
    // ��ȡ��ǰϵͳʱ�� s
    //inline quint32 systemTime () const;

    // ˽������
private:
    // ��¼�����ļ�IO����
    osFile*	m_recFile;
    osFile*	m_repFile;
    //osFile*	m_outFile;

    bool	m_recordingFlag;    // ��¼��־
    bool	m_replayingFlag;    // ���ݱ�־

    qint64	m_nRecordComputerTime_100ms;	// ��ǰ��¼�ļ����ʱ��(10ms)
    qint64	m_nReplayComputerTime_100ms;	// ��ǰ���ݵļ����ʱ��(10ms)
    qint64	m_computerTime_100ms;			// ��ǰ�����ʱ��

    qint32	m_nReplayStartTime;	// ������ʼʱ��(UTC ��λ:��)
    qint32	m_nReplayStopTime;	// ���ݽ���ʱ��
    qint32	m_nReplayTotalTime_100ms;	// ������ʱ��
    qint32	m_nReplayingTime_100ms;

    // ��¼���ʱ�䣺�ӿ�ʼ��¼��ʼ��ʱ (really 10ms)
    qint32	m_nRecordingTime_100ms;

    quint8  m_waitMode;     // ����ģʽ 0:��������,1:�������
    quint8	m_nRepSpeed;		// �����ٶ� 1:������2:2���٣�4:4����
    qint32	m_lRepDataPos;		// ���������ļ��е�ָ��λ�ã�ָ�굱ǰ��ȡ���ݵ�λ��

    // �����ļ��б�
    QMap<quint32, QString>	m_replayFiles;

    // ��¼�ļ�ͷ����
    RECFILEHEADER	m_recFileHeader;
    // ��¼����ͷ
    RECDATAHEADER	m_recDataHeader;
    // ��ǰ��¼��������
    RECDATAINFO		m_recDataInfo;

    // �����ļ�ͷ����
    RECFILEHEADER	m_repFileHeader;
    // ��������ͷ
    RECDATAHEADER	m_repDataHeader;
    // ��ǰ������������
    RECDATAINFO		m_repDataInfo;

    // ��¼���ݻ�����
    quint8*			m_recDataBuffer;
    quint16			m_recWriteIndex;

    // �������ݵȴ�ʱ����ź���
    QSemaphore		m_waitTimeMutex;
    // ����������ͣ���Ƶ��ź���
    QSemaphore		m_waitPauseMutex;
    // ���ڼ�¼״̬���ź���
    QMutex			m_recordingMutex;
    QMutex			m_replayingMutex;
    // �ļ��������
    QMutex			m_fileMutex;
    // ������ͣ��־
    bool		m_paused;
    // ����״̬
    quint8		m_replayMode;	// ����״̬��0���������ݣ�1����ʾ��̬������2�����溽������
    bool		m_replayFinished;

    QString		m_replayFilePath;

    class QRecordReplayThread* m_replayThread;

    DATAPROCESSFUNC m_dataProcFunc;
    int     m_updateEventId;
};

// ��¼���ݴ����߳�
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

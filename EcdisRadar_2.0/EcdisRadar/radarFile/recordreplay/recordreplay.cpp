#include "recordreplay.h"
#include "implrecrep.h"


Recordreplay::Recordreplay(QObject *parent)
:m_implement(new ImplRecRep(parent))
{
}

Recordreplay::~Recordreplay()
{
    delete m_implement;
}

// ���ü�¼������Ŀ¼
void Recordreplay::setRecordPath(const QString& path)
{
    m_implement->setRecordPath(path);
}

// �������ݴ�����
void Recordreplay::setReplayProcessFunc(DATAPROCESSFUNC func)
{
    m_implement->setReplayProcessFunc(func);
}

// ���õȴ�ģʽ
void Recordreplay::setWaitMode(bool flag)
{
    m_implement->setWaitMode(flag);
}

// ���ø����¼�ID
void Recordreplay::setUpdateEventId(int id)
{
    m_implement->setUpdateEventId(id);
}

// ��ʼ��¼
bool Recordreplay::startRecord ()
{
    return m_implement->startRecord();
}

// ��¼����
bool Recordreplay::recording (const char* pData, qint32 nLength, quint16 devid)
{
    return m_implement->recording(pData, nLength, devid);
}

// ֹͣ��¼
void Recordreplay::stopRecord ()
{
    m_implement->stopRecord();
}

// ��ʼ����(���ָ���ļ���Ч,�����ݸ��ļ�.����ָ��ʱ��,�Զ�Ѱ����Ӧ���ļ���������)
bool Recordreplay::startReplay (const QString& fileName, qint32 start_tm, qint32 stop_tm)
{
    return m_implement->startReplay(fileName, start_tm, stop_tm);
}

// ֹͣ����
void Recordreplay::stopReplay ()
{
    m_implement->stopReplay();
}

// ������ͣ����
void Recordreplay::pause ()
{
    m_implement->pause();
}

// �ı������ٶ�
void Recordreplay::changeReplaySpeed (quint8 speed)
{
    m_implement->changeReplaySpeed(speed);
}

// ���ص�ǰ��ͣ��־
bool Recordreplay::isPaused() const
{
    return m_implement->isPaused();
}
// ���������ٶ�
quint8 Recordreplay::replaySpeed() const
{
    return m_implement->replaySpeed();
}
// �жϼ�¼��־
bool Recordreplay::isRecording () const
{
    return m_implement->isRecording();
}
// �ж����ݱ�־
bool Recordreplay::isReplaying () const
{
    return m_implement->isReplaying();
}

// ��ȡ��¼�ļ�������ͷ
QList<RECDATAHEADER> Recordreplay::getDataHeaders (const QString& filename)
{
    return m_implement->getDataHeaders(filename);
}

// ���¼�¼���ʱ��(�������Ϊ100ms���ʱ��)
void Recordreplay::updateComputerTime(quint32 delTime)
{
    m_implement->updateComputerTime(delTime);
}

// ��������ʱ��
void Recordreplay::setReplayTime (quint32 tm)
{
    m_implement->setReplayTime(tm);
}

// ��ȡ���ݵ�ǰʱ��
quint32 Recordreplay::crntReplayTime() const
{
	return m_implement->crntReplayTime();
}

// ��ȡ���ݵ�ǰʱ��
quint32 Recordreplay::getReplayTime () const
{
    return m_implement->getReplayTime();
}
// ��ȡ������ʱ��
qint32 Recordreplay::getReplayTotalTime () const
{
    return m_implement->getReplayTotalTime();
}

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

// 设置记录重演主目录
void Recordreplay::setRecordPath(const QString& path)
{
    m_implement->setRecordPath(path);
}

// 设置重演处理函数
void Recordreplay::setReplayProcessFunc(DATAPROCESSFUNC func)
{
    m_implement->setReplayProcessFunc(func);
}

// 设置等待模式
void Recordreplay::setWaitMode(bool flag)
{
    m_implement->setWaitMode(flag);
}

// 设置更新事件ID
void Recordreplay::setUpdateEventId(int id)
{
    m_implement->setUpdateEventId(id);
}

// 开始记录
bool Recordreplay::startRecord ()
{
    return m_implement->startRecord();
}

// 记录数据
bool Recordreplay::recording (const char* pData, qint32 nLength, quint16 devid)
{
    return m_implement->recording(pData, nLength, devid);
}

// 停止记录
void Recordreplay::stopRecord ()
{
    m_implement->stopRecord();
}

// 开始重演(如果指定文件有效,则重演该文件.否则按指定时间,自动寻找相应的文件进行重演)
bool Recordreplay::startReplay (const QString& fileName, qint32 start_tm, qint32 stop_tm)
{
    return m_implement->startReplay(fileName, start_tm, stop_tm);
}

// 停止重演
void Recordreplay::stopReplay ()
{
    m_implement->stopReplay();
}

// 重演暂停处理
void Recordreplay::pause ()
{
    m_implement->pause();
}

// 改变重演速度
void Recordreplay::changeReplaySpeed (quint8 speed)
{
    m_implement->changeReplaySpeed(speed);
}

// 返回当前暂停标志
bool Recordreplay::isPaused() const
{
    return m_implement->isPaused();
}
// 返回重演速度
quint8 Recordreplay::replaySpeed() const
{
    return m_implement->replaySpeed();
}
// 判断记录标志
bool Recordreplay::isRecording () const
{
    return m_implement->isRecording();
}
// 判断重演标志
bool Recordreplay::isReplaying () const
{
    return m_implement->isReplaying();
}

// 获取记录文件的数据头
QList<RECDATAHEADER> Recordreplay::getDataHeaders (const QString& filename)
{
    return m_implement->getDataHeaders(filename);
}

// 更新记录相对时间(输入参数为100ms相对时间)
void Recordreplay::updateComputerTime(quint32 delTime)
{
    m_implement->updateComputerTime(delTime);
}

// 设置重演时间
void Recordreplay::setReplayTime (quint32 tm)
{
    m_implement->setReplayTime(tm);
}

// 获取重演当前时间
quint32 Recordreplay::crntReplayTime() const
{
	return m_implement->crntReplayTime();
}

// 获取重演当前时间
quint32 Recordreplay::getReplayTime () const
{
    return m_implement->getReplayTime();
}
// 获取重演总时间
qint32 Recordreplay::getReplayTotalTime () const
{
    return m_implement->getReplayTotalTime();
}

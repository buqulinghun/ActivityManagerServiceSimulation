#ifndef RECORDREPLAY_H
#define RECORDREPLAY_H

#include "recordreplay_global.h"

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>

class ImplRecRep;

class RECORDREPLAYSHARED_EXPORT Recordreplay {
public:
    Recordreplay(QObject *parent=NULL);
    virtual ~Recordreplay();

    // 外部调用接口
public:
    // 设置记录重演主目录
    void setRecordPath(const QString& path);
    // 设置重演处理函数
    void setReplayProcessFunc(DATAPROCESSFUNC func);
    // 设置更新事件ID
    void setUpdateEventId(int id);
    // 设置等待模式
    void setWaitMode(bool flag);

    // 开始记录
    bool startRecord ();
    // 记录数据
    bool recording (const char* pData, qint32 nLength, quint16 devid);
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

    // 更新记录相对时间(输入参数为100ms相对时间)
    void updateComputerTime(quint32 delTime);

    // 返回重演速度
    quint8 replaySpeed() const;
    // 判断记录标志
    bool isRecording () const;
    // 判断重演标志
    bool isReplaying () const;
    // 返回当前暂停标志
    bool isPaused() const;

    // 获取记录文件的数据头
    QList<RECDATAHEADER> getDataHeaders (const QString& filename);

    // 设置重演时间
    void setReplayTime (quint32 tm);
	// 获取重演当前时间
	quint32 crntReplayTime() const;
    // 获取重演当前时间
    quint32 getReplayTime () const;
    // 获取重演总时间
    qint32 getReplayTotalTime () const;

private:
    ImplRecRep* m_implement;
};

#endif // RECORDREPLAY_H

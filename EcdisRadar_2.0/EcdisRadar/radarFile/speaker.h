#ifndef SPEAKER_H
#define SPEAKER_H

#include <QtCore/QObject>
#include <QtCore/QBitArray>
#include <QtCore/QMutex>
#include <QtCore/QThread>




class Speaker : public QThread
{

public:
    Speaker();
    virtual ~Speaker();

    // 启动发声,mode指定发声模式，按位判断是1发声还是0不发声，
    // 每位的时间间隔由m_timeInterval控制；
    // repeat指定重复次数，为0表示循环发声
    void startSounding(const QBitArray& mode, quint8 repeat, int timeInterval);
    // 停止发声
    void stopSounding();

protected:
    void run();

    //控制扬声器发声的函数
    void Beep(int, int);

    //正真控制发声器
    void realBeep(quint8 flag);

private:
    QBitArray   m_soundMode;
    quint8      m_soundRepeat;
    int         m_timeInterval;
    quint8	m_stopFlag;
    QMutex      m_mutex;
};

#endif // SPEAKER_H

#include "speaker.h"
#include "interact.h"

#include <QDataStream>



extern Interact *lpInteract;
quint8 flag_isSpeaking=0;





Speaker::Speaker()
{
     m_stopFlag = 0;
}

Speaker::~Speaker()
{
}

// 启动发声,mode指定发声模式，按位判断是1发声还是0不发声，
// 每位的时间间隔由m_timeInterval控制；难道是每次发声的间隔。。。。
// repeat指定重复次数，为0表示循环发声
void Speaker::startSounding(const QBitArray& mode, quint8 repeat, int timeInterval)
{
    m_mutex.lock();

    // 保存参数
    m_stopFlag = 0;
    m_soundMode = mode;
    m_soundRepeat = repeat;
    m_timeInterval = timeInterval;  //发声频率
    // 启动线程
    start();

    m_mutex.unlock();
}

// 停止发声
void Speaker::stopSounding()
{
    m_stopFlag = 1;
    flag_isSpeaking = 0;

    // 终止线程
    if(!wait(100))
        terminate();
}

void Speaker::run()
{
   /* for(quint8 i=0; i<m_soundRepeat; i++)
    {
        for(quint8 j=0; j<m_soundMode.count(); j++)
        {
            if(m_stopFlag)	return;

            if(m_soundMode[j]==true)
            {
               // Beep(2500, m_timeInterval);  //发声并保持时间
                realBeep(1);
                msleep(10);
                realBeep(1);

                msleep(m_timeInterval);
            }

        }
    } */

    forever{
        flag_isSpeaking = 1;  //正在发声标志
        for(quint8 j=0; j<m_soundMode.count(); j++)
        {
            if(m_stopFlag)	return;

            if(m_soundMode[j]==true)
            {
               // Beep(2500, m_timeInterval);  //发声并保持时间
                realBeep(1);
                msleep(10);
                realBeep(1);

                msleep(m_timeInterval);
            }

        }
    }

}

//间隔发声
void Speaker::Beep(int, int interval)
{
    // 1.发出声音
    realBeep(1);
    // 2.等待时间
    msleep(interval);
    // 3.关闭声音
    realBeep(0);
}

//控制硬件发声
void Speaker::realBeep(quint8 flag)
{
    //1：发声，0：关闭声音
    QByteArray sound;
    QDataStream in(&sound, QIODevice::WriteOnly);
    if(flag) {
        in << (quint8)0x80;
        lpInteract->SeirialWrite(sound);  //键盘串口设备号：5
    }

}






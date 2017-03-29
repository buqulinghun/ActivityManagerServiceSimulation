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

    // �ⲿ���ýӿ�
public:
    // ���ü�¼������Ŀ¼
    void setRecordPath(const QString& path);
    // �������ݴ�����
    void setReplayProcessFunc(DATAPROCESSFUNC func);
    // ���ø����¼�ID
    void setUpdateEventId(int id);
    // ���õȴ�ģʽ
    void setWaitMode(bool flag);

    // ��ʼ��¼
    bool startRecord ();
    // ��¼����
    bool recording (const char* pData, qint32 nLength, quint16 devid);
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

    // ���¼�¼���ʱ��(�������Ϊ100ms���ʱ��)
    void updateComputerTime(quint32 delTime);

    // ���������ٶ�
    quint8 replaySpeed() const;
    // �жϼ�¼��־
    bool isRecording () const;
    // �ж����ݱ�־
    bool isReplaying () const;
    // ���ص�ǰ��ͣ��־
    bool isPaused() const;

    // ��ȡ��¼�ļ�������ͷ
    QList<RECDATAHEADER> getDataHeaders (const QString& filename);

    // ��������ʱ��
    void setReplayTime (quint32 tm);
	// ��ȡ���ݵ�ǰʱ��
	quint32 crntReplayTime() const;
    // ��ȡ���ݵ�ǰʱ��
    quint32 getReplayTime () const;
    // ��ȡ������ʱ��
    qint32 getReplayTotalTime () const;

private:
    ImplRecRep* m_implement;
};

#endif // RECORDREPLAY_H

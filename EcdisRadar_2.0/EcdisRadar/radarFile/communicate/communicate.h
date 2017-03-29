#ifndef IMPLCOMMUNICATE_H
#define IMPLCOMMUNICATE_H

#include <QtCore/qglobal.h>
#include <QtNetwork/QHostAddress>
#include <QtCore/QString>
#include <QtCore/QByteArray>

#include "communicate_global.h"

typedef struct tagSerialConfig
{
	quint32 baudrate;
	quint8	databit;
	quint8	stopbit;
	quint8	parity;
}SERIALCONFIG;

class CCommunicate;
class COMMUNICATESHARED_EXPORT Communicate
{
public:
    Communicate();
    virtual ~Communicate();

    void setLocalHost (const QHostAddress& local_addr);
    /*创建UDP通信对象接口*/
    quint32 open(const QHostAddress& addr, quint16 recvport, quint16 sendport=0, quint32 defaultId = 0);

    /*创建串口通信对象接口*/
    quint32 open(const QString & devicename, const SERIALCONFIG& cfg, quint32 defaultId = 0);
    quint32 reopen(const QString & devicename, const SERIALCONFIG& cfg, quint32 defaultId = 0);

    bool close(quint32 id);
    /*设置组播地址*/
    quint8 setMultiBroadcast(quint32 id, QList<QHostAddress> multiAddress);
    quint8 setMultiBroadcast(QList<QHostAddress> multiAddress);

    /*发送数据接口*/
    int sendData(const QByteArray& data, quint32 id);
    /*接收数据接口，返回设备ID值*/
    quint32 recvData(QByteArray& data);

private:
    CCommunicate* m_pImpliment;
};

#endif // IMPLCOMMUNICATE_H

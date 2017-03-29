#include "communicate.h"
#include "implcommobject.h"


Communicate::Communicate()
:m_pImpliment(new CCommunicate)
{
}

Communicate::~Communicate()
{
    if(m_pImpliment)
    {
    delete m_pImpliment;
    }
}

void Communicate::setLocalHost (const QHostAddress& local_addr)
{
    m_pImpliment->setLocalHost(local_addr);
}

/*创建UDP通信对象
*/
quint32 Communicate::open(const QHostAddress& addr, quint16 recvport, quint16 sendport, quint32 defaultId)
{
    return m_pImpliment->open(addr, recvport, sendport, defaultId);
}
/*创建串口通信对象*/
quint32 Communicate::open(const QString & devicename, const SERIALCONFIG& cfg, quint32 defaultId)
{
    return m_pImpliment->open(devicename, cfg, defaultId);
}
quint32 Communicate::reopen(const QString & devicename, const SERIALCONFIG& cfg, quint32 defaultId)
{
    return m_pImpliment->reopen(devicename, cfg, defaultId);
}

bool Communicate::close(quint32 id)
{
    return m_pImpliment->close(id);
}

/*设置组播地址*/
quint8 Communicate::setMultiBroadcast(quint32 id, QList<QHostAddress> multiAddress)
{
    return m_pImpliment->setMultiBroadcast(id, multiAddress);
}

quint8 Communicate::setMultiBroadcast(QList<QHostAddress> multiAddress)
{
    return m_pImpliment->setMultiBroadcast(multiAddress);
}



/*发送数据接口：通过指定的设备发送数据*/
int Communicate::sendData(const QByteArray& data, quint32 id)
{
    return m_pImpliment->sendData(data, id);
}

/*接收数据接口：如果某设备有数据，则获取该数据，并返回该设备号，
  以同步阻塞方式实现，如果没有数据，则该函数阻塞
*/
quint32 Communicate::recvData(QByteArray& data)
{
    return m_pImpliment->recvData(data);
}

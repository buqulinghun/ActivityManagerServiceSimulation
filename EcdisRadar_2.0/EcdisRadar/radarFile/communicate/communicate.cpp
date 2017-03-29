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

/*����UDPͨ�Ŷ���
*/
quint32 Communicate::open(const QHostAddress& addr, quint16 recvport, quint16 sendport, quint32 defaultId)
{
    return m_pImpliment->open(addr, recvport, sendport, defaultId);
}
/*��������ͨ�Ŷ���*/
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

/*�����鲥��ַ*/
quint8 Communicate::setMultiBroadcast(quint32 id, QList<QHostAddress> multiAddress)
{
    return m_pImpliment->setMultiBroadcast(id, multiAddress);
}

quint8 Communicate::setMultiBroadcast(QList<QHostAddress> multiAddress)
{
    return m_pImpliment->setMultiBroadcast(multiAddress);
}



/*�������ݽӿڣ�ͨ��ָ�����豸��������*/
int Communicate::sendData(const QByteArray& data, quint32 id)
{
    return m_pImpliment->sendData(data, id);
}

/*�������ݽӿڣ����ĳ�豸�����ݣ����ȡ�����ݣ������ظ��豸�ţ�
  ��ͬ��������ʽʵ�֣����û�����ݣ���ú�������
*/
quint32 Communicate::recvData(QByteArray& data)
{
    return m_pImpliment->recvData(data);
}

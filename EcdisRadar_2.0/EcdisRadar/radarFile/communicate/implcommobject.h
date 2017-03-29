#ifndef IMPLCOMMOBJECT_H
#define IMPLCOMMOBJECT_H

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QSemaphore>
#include <QtCore/QMutexLocker>
#include <QtCore/QThread>
#include <QtNetwork/QHostAddress>

#include "communicate.h"

#include "../qextserialport-1.2win-alpha/qextserialport.h"


struct NETWORKOPTIONS;

#define RAWSOCKET 0

#ifdef Q_OS_LINUX
#define closesocket close
typedef struct sockaddr*    LPSOCKADDR;
typedef struct sockaddr_in  SOCKADDR_IN;
typedef struct hostent*    PHOSTENT;
#endif

/* 4 bytes IP address */
typedef struct ip_address
{
        quint8 byte1;
        quint8 byte2;
        quint8 byte3;
        quint8 byte4;
}ip_address;

/* IPv4 header */
typedef struct ip_header
{
        quint32	ver_ihl : 8;	// Version (4 bits) + Internet header length (4 bits)
        quint32	tos : 8;	// Type of service
        quint32 tlen : 16;	// Total length

        quint32 identification :16; // Identification
        quint32 flags_fo:16;	// Flags (3 bits) + Fragment offset (13 bits)

        quint32	ttl:8;		// Time to live
        quint32	proto:8;	// Protocol
        quint32 crc:16;		// Header checksum
        quint32	saddr;		// Source address
        quint32	daddr;		// Destination address
//	quint32	op_pad;		// Option + Padding
}ip_header;

/* UDP header*/
typedef struct udp_header
{
        quint32 sport : 16;			// Source port
        quint32 dport : 16;			// Destination port
        quint32 len : 16;			// UDP�����ֶ�ָ����UDP�ײ���UDP���ݵ��ֽڳ���
        quint32 crc : 16;			// Checksum
}udp_header;

typedef struct _PSHeader
{
        unsigned long srcaddr;
        unsigned long destaddr;

        unsigned char zero;
        unsigned char protocol;
        unsigned short len;
}PSHeader;

typedef struct tagData2Transmit {
    QByteArray data;
    QHostAddress host_addr;
    quint16 host_port;
}Data2Transmit;


///////////////////////////////////////////////////////////////
/*
  ���ü�����:CReferenceObject
*/
class CReferenceObject : public QObject
{
public:
    CReferenceObject(QObject* parent=NULL)
        :QObject(parent),m_referenceCount(1)
    {
    }

    /*���๤�� ObjectFactory ����������ü��������롢ɾ������*/
    int increaseReference()
    {   return ++m_referenceCount;  }
    int decreaseReference()
    {   return --m_referenceCount;  }

protected:
    int m_referenceCount;
};

///////////////////////////////////////////////////////////////
/*
  UDP���ݽ�����:CUdpTransmiter
  ���ڼ���ָ��UDP�˿ڣ������ոö˿�����
*/
class CUdpTransmiter : public CReferenceObject
{
public:
    CUdpTransmiter(QObject* parent=NULL);
    virtual ~CUdpTransmiter();

    void setLocalHost (const QHostAddress& local_addr, quint16 local_port=0);
    int sendTo (const QByteArray& data, const QHostAddress& host_addr, quint16 host_port);

    // ���������鲥����
    void setMultiBroadcast(bool flag);

protected:
    void run();

private:
    int         m_socket;

#if RAWSOCKET
    char*	m_buffer;
    char*	m_data;
    ip_header*	m_ip_header;
    udp_header*	m_udp_header;
#endif
};

class CUdpReceiverThread : public QThread
{
public:
    CUdpReceiverThread(QObject* parent=NULL)
        :QThread(parent)
    {
    }

protected:
    void run();
};

///////////////////////////////////////////////////////////////
/*
  UDP���ݽ�����:CUdpReceiver
  ���ڼ���ָ��UDP�˿ڣ������ոö˿�����
*/
class CUdpReceiver : public CReferenceObject
{
public:
    CUdpReceiver(QObject* parent=NULL);
    virtual ~CUdpReceiver();

    // �󶨶˿ں�
    void bindPort(quint16 port);
    quint16 port() const
    {   return m_port;   }

    // �����鲥��ַ
    quint8 setMultiBroadcast(QList<QHostAddress> multiAddress);

    void run();


    //get ip and mac
    void getIpAndMac(void);
    bool print_hw_addr(int fd, const char* if_name);



private:
    quint16 m_port;
    int     m_socket;
    char*   m_buffer;


    CUdpReceiverThread*    m_thread;
};


///////////////////////////////////////////////////////////////
/*
  �����豸:CUdpTransmiter
  ���ڼ���ָ��UDP�˿ڣ������ոö˿�����
*/
class CBaseDevice : public QObject
{
    Q_OBJECT
public:
    enum {
        UDP_DEVICE = 1,
        SERIAL_DEVICE,
    };

public:
    CBaseDevice(quint32 id=0, QObject* parent=NULL)
        :QObject(parent),m_deviceid(id),m_deviceType(0)
    {}
    virtual ~CBaseDevice()
    {}

    void setDeviceId(quint32 id)
    {   m_deviceid = id;    }
    quint32 deviceid()const
    {   return m_deviceid;  }

    bool isValid () const
    {   return m_valid; }

    bool isUdpDevice() const
    {   return m_deviceType == UDP_DEVICE;   }
    bool isSerialDevice() const
    {	return m_deviceType == SERIAL_DEVICE;	}

protected:
    quint32 m_deviceid;
    bool    m_valid;
    quint8  m_deviceType;   // �豸���ͣ� 1:udp 2:serial
};

///////////////////////////////////////////////////////////////
/*
  UDP�豸����:CUdpDevice
*/
class CUdpDevice : public CBaseDevice
{
    Q_OBJECT
public:
    CUdpDevice(quint32 id=0, QObject* parent=NULL);
    virtual ~CUdpDevice();

    bool open(const QHostAddress& addr, quint16 recvport, quint16 sendport=0);

//    int recvData(QByteArray& data);
//    int sendData(const QByteArray& data);

    // �����鲥��ַ
    quint8 setMultiBroadcast(QList<QHostAddress> multiAddress);

    QHostAddress hostAddress()
    {   return m_hostAddress;   }
    quint32 recvPort() const
    {   return m_recvPort;  }
    quint32 sendPort() const
    {   return m_sendPort;  }

private:
    QHostAddress    m_hostAddress;
    quint16         m_recvPort;
    quint16         m_sendPort;

    CUdpReceiver*   m_udpReciever;
};

///////////////////////////////////////////////////////////

class QextSerialPort;  //
class CSerialReceiverThread;
class CSerialDevice : public CBaseDevice
{
    Q_OBJECT
public:
    CSerialDevice(quint32 id=0, QObject *parent=NULL);
    virtual ~CSerialDevice();

    quint8 setSerialConfig(const SERIALCONFIG &cfg);

    bool open(const QString &name);
    bool close();

    int sendData(const QByteArray &data);

public slots:
    void recvData();

protected:
    CSerialReceiverThread *m_serialRecvThread;
    QextSerialPort *m_serialPort;

    QString m_serialName;
    quint32 m_baudrate;
    quint8 m_databit;
    quint8 m_stopbit;
    quint8 m_parity;

    char *m_recvBuffer;
};

class CSerialReceiverThread : public QThread
{
public:
    CSerialReceiverThread(QObject *parent=NULL)
        :QThread(parent),m_stopFlag(0)
    {}

    void stop()
    {
        m_stopFlag = 1;
        wait();
    }

protected:
    void run();

private:
    quint8 m_stopFlag;
};



typedef struct tagDeviceData
{
    quint32 deviceid;
    QByteArray  data;

    tagDeviceData()
        :deviceid(0)
    {}

    tagDeviceData(quint32 _id, const QByteArray& _data)
    :deviceid(_id),data(_data)
    {}

}DEVICEDATA;

class CCommunicate : public QObject
{
public:
    CCommunicate(QObject* parent=NULL);
    virtual ~CCommunicate();

    void setLocalHost (const QHostAddress& local_addr)
    {
        m_udpTransmiter->setLocalHost(local_addr);
    }

    // �豸��
    quint32 open(const QHostAddress& addr, quint16 recvport, quint16 sendport=0, quint32 defaultId = 0);

    /*��������ͨ�Ŷ���ӿ�*/
    quint32 open(const QString & devicename, const SERIALCONFIG& cfg, quint32 defaultId = 0);
    quint32 reopen(const QString & devicename, const SERIALCONFIG& cfg, quint32 defaultId = 0);

    // �豸�ر�
    bool close(quint32 id);
    /*�������ݵ�ָ���豸*/
    int sendData(const QByteArray& data, quint32 id);
    /*��������*/
    quint32 recvData(QByteArray& data);

    /*�����鲥��ַ*/
    quint8 setMultiBroadcast(quint32 id, QList<QHostAddress> multiAddress);
    quint8 setMultiBroadcast(QList<QHostAddress> multiAddress);


    /*�յ�UDP����*/
    void udpDataReceived(const QByteArray& data, const QHostAddress& addr, quint16 port);

    /*���ô��ڲ���*/
    quint8 setSerialConfig(quint32 id, const SERIALCONFIG& cfg);
    /*���յ���������*/
    void serialDataReceived(const QByteArray& data, quint32 devid);


    /*�ж��豸�Ƿ���Ч*/
    int isDeviceValid(quint32 id);


protected:
    // ͨ��IP��ַ�Ͷ˿ںŲ��Ҷ�Ӧ���豸ID
    inline quint32 udpDeviceId(const QHostAddress& addr, quint16 recvport);

    /*������õ��豸��*/
    inline quint32 applyDeviceId() const;
    /*�ж��豸���Ƿ��Ѿ�ʹ�ã�����0û��ʹ�ã�����1���Ѿ�ʹ��*/
    inline int isDeviceIdUsed(quint32) const;

    // �ر������豸
    void closeAllDevices();

private:
    /*�豸����� <�豸ID, �豸����>*/
    QHash<quint32, CBaseDevice*>  m_deviceObjectTable;
    QMutex  m_deviceTableMutex;

    QSemaphore m_dataRecved;
    QMutex  m_dataRecvedMutex;
    QList<DEVICEDATA>   m_dataRecvedList;

    CUdpTransmiter* m_udpTransmiter;
};

class UdpFactory
{
public:
    static CUdpReceiver* createUdpReceiverObject(quint16 port, QObject* parent=NULL);
    static void deleteUdpReceiverObject(CUdpReceiver* udp);

private:
    // key:�˿ں�, value:udpͨ�Ŷ���
    static QHash<quint16, CUdpReceiver*> m_udpBinded;
    static QMutex m_udpMutex;
};


#endif // IMPLCOMMOBJECT_H

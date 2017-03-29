
#include "communicate_global.h"
#include "implcommobject.h"


#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>  // for inet_addr
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <fcntl.h>


#include <string>
#include <iostream>
#include <cstring>
#include <QStringList>


using namespace std;

const int ipHeaderSize = sizeof(ip_header);
const int udpHeaderSize = sizeof(udp_header);


quint32 host_Ip;
QString host_Mac;


#define MAXDEVICEID 1000

//CheckSum:计算校验和的子函数
quint16 checksum(quint16 *buffer, int size)
{
    unsigned long cksum=0;

    while (size > 1)
    {
    cksum += *buffer++;
    size -= sizeof(quint16);
    }

    if (size)
    {
    cksum += *(quint8*)buffer;
    }

    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);

    return (quint16)(~cksum);
}

quint16 ipHeaderCheckSum (const ip_header* ih)
{
    return checksum((quint16*)(ih), sizeof(ip_header));
}



bool GetLinuxHostIPAddress(char* IPAddress,int HostID)
{
    int s;
    struct ifconf conf;
    struct ifreq *ifr;
    char buff[BUFSIZ];
    int i, num;
    int find = 0;
    
    s = socket(PF_INET, SOCK_DGRAM, 0);
    conf.ifc_len = BUFSIZ;
    conf.ifc_buf = buff;
    
    ioctl(s, SIOCGIFCONF, &conf);
    num = conf.ifc_len / sizeof(struct ifreq);
    ifr = conf.ifc_req;
    
    for(i=0;i < num;i++)
    {
        struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);
        
        ioctl(s, SIOCGIFFLAGS, ifr);
        if(((ifr->ifr_flags & IFF_LOOPBACK) == 0) && (ifr->ifr_flags & IFF_UP))
        {
			find = 1;
			strcpy(IPAddress, inet_ntoa(sin->sin_addr));
			printf("%s : (%s)\n",
                ifr->ifr_name,
                inet_ntoa(sin->sin_addr));
        }
        ifr++;
    }

	return find;
}

#define GetHostIPAddress GetLinuxHostIPAddress


/**********************************
 * class : CUdpTransmiter
**********************************/
#define MAXUDPPACKETSIZE 4096
CUdpTransmiter::CUdpTransmiter(QObject* parent)
:CReferenceObject(parent)
{
#if RAWSOCKET
    m_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);

    int on = 1;
    int ret = setsockopt(m_socket, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on));
    //if (ret == SOCKET_ERROR)
    {
    }

    m_buffer = new char[MAXUDPPACKETSIZE];
    memset(m_buffer, 0, MAXUDPPACKETSIZE);

    m_ip_header = (ip_header*)(m_buffer);
    m_udp_header = (udp_header*)(m_buffer+ipHeaderSize);
    m_data = (char*)(m_buffer + ipHeaderSize + udpHeaderSize);

    m_ip_header->ver_ihl = 0x45;		// Version (4 bits) + Internet header length (4 bits) (0x45)
    m_ip_header->tos = 0;			// Type of service
    m_ip_header->tlen = 0;			// Total length
    m_ip_header->identification = 1; // Identification
    m_ip_header->flags_fo = 0;		// Flags (3 bits) + Fragment offset (13 bits)
    m_ip_header->ttl = 255;			// Time to live
    m_ip_header->proto = IPPROTO_UDP;			// Protocol
    m_ip_header->crc = 0;			// Header checksum

    m_udp_header->crc = 0 ;

#else

    m_socket = socket(AF_INET,SOCK_DGRAM,0);
    if(m_socket == -1)
        qDebug() << "create udp transmiter socket error.";

//    struct sockaddr_in local;
//    local.sin_family=AF_INET;
//    local.sin_port=htons(9000); ///监听端口
//    local.sin_addr.s_addr=INADDR_ANY; ///本机
//    bind(m_socket,(struct sockaddr*)&local,sizeof(local));

#endif

    // 允许组播发送
    setMultiBroadcast(true);

}

CUdpTransmiter::~CUdpTransmiter()
{
    closesocket (m_socket);

#if RAWSOCKET
    delete[] m_buffer;
#endif
}

void CUdpTransmiter::setLocalHost (const QHostAddress& local_addr, quint16 local_port)
{
#if RAWSOCKET
    m_ip_header->saddr = inet_addr(local_addr.toString().toLocal8Bit());
    m_udp_header->sport = htons (local_port);
#endif
}

/*直接发送，不需要额外线程来处理*/
int CUdpTransmiter::sendTo (const QByteArray& data, const QHostAddress& host_addr, quint16 host_port)
{
#if RAWSOCKET
    const int dataSize = data.size();
    const int udpSize = udpHeaderSize + dataSize;
    const int ipSize = ipHeaderSize + udpSize;

    m_ip_header->daddr = inet_addr(host_addr.toString().toLocal8Bit());
    m_ip_header->tlen = ipSize;
    m_ip_header->crc = checksum((unsigned short *) m_ip_header, ipHeaderSize);

    m_udp_header->sport = htons(host_port);
    m_udp_header->dport = htons(host_port);
    m_udp_header->len = htons(udpSize);
    //m_udp_header->crc = 0 ;

    memcpy (m_data, data.data(), dataSize);

    SOCKADDR_IN sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons (host_port);
    sock_addr.sin_addr.s_addr = m_ip_header->daddr;

    return sendto(m_socket, (char*)m_buffer, ipSize, 0, (LPSOCKADDR)&sock_addr, sizeof(sock_addr));
#else

    //qDebug() << "send data:" << host_addr << host_port;

    SOCKADDR_IN	sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons (host_port);
    sock_addr.sin_addr.s_addr = inet_addr(host_addr.toString().toLocal8Bit());
    //return sendto(m_socket, data.data(), data.size(), 0, (LPSOCKADDR)&sock_addr, sizeof(sock_addr));
	int res = 0;
	if((res = sendto(m_socket, data.data(), data.size(), 0, (LPSOCKADDR)&sock_addr, sizeof(sock_addr))) < 0)
	{
		qDebug() << "send data error, try again, " << host_addr.toString() << host_port;
		if(res = sendto(m_socket, data.data(), data.size(), 0, (LPSOCKADDR)&sock_addr, sizeof(sock_addr)) < 0)
			qDebug() << "========== send data error ==========";
	}
	return res;
#endif
}

// 设置允许组播发送
void CUdpTransmiter::setMultiBroadcast(bool flag)
{
    //获取可用IP地址
    char IPaddress[30];
    GetHostIPAddress(IPaddress,1);

    /*允许组播发送*/
    struct in_addr inaddr;
    inaddr.s_addr = inet_addr(IPaddress);
    setsockopt(m_socket,IPPROTO_IP,IP_MULTICAST_IF,(char*)&inaddr,sizeof(inaddr));

    /* 回环发送 optval 1:允许，0:禁止 */
#ifdef Q_OS_WIN32
    char optval = 0;
#endif
#ifdef Q_OS_LINUX
    char optval = 1;
#endif
    setsockopt(m_socket,IPPROTO_IP,IP_MULTICAST_LOOP,(char*)&optval,sizeof(optval));

}

/*如果需要可以将实际数据发送搬到线程里面来处理*/
void CUdpTransmiter::run()
{
}


/**********************************
 * class : CUdpReceiver
**********************************/
#define MAXRECVSIZE 10000
CUdpReceiver::CUdpReceiver(QObject* parent)
:CReferenceObject(parent),m_thread(new CUdpReceiverThread(this))
{
    int reuse;
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);

   // fcntl(m_socket, F_SETFL, O_NONBLOCK);
   //  fcntl(m_socket, F_SETFL, fcntl(m_socket, F_GETFL) & ~O_NONBLOCK);
    if(m_socket == -1)
        qDebug() << "create udp receiver socket error.";
    setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

    m_buffer = new char[MAXRECVSIZE];


    //get ip mac
    getIpAndMac();
}


CUdpReceiver::~CUdpReceiver()
{
    m_thread->terminate();
    delete m_thread;

    closesocket (m_socket);
    delete[] m_buffer;
}

void CUdpReceiver::getIpAndMac()
{
    //lookback not select
    ifreq ifs[16] = {0};
    ifconf conf = {sizeof(ifs)};
    conf.ifc_req = ifs;
    if(-1 == ioctl(m_socket, SIOCGIFCONF, &conf)) {
        perror("Failed IOCTL SIOCGIFCONF.");
        return;
    }
    if(conf.ifc_len >= sizeof(ifs)) {
        perror("Buffer too small for IOCTL SIOCGIFCONF.");
        return;
    }

    int num = conf.ifc_len / sizeof(ifreq);
    cout << num << " interface entry retrieved." << endl;
    for(int i = 0; i < num; ++i) {

        if(print_hw_addr(m_socket, ifs[i].ifr_name)) {
            cout << "[ " << ifs[i].ifr_name << " ]" << endl;
            sockaddr_in* sai = (sockaddr_in*)&ifs[i].ifr_addr;
            cout << "Addr: " << inet_ntoa(sai->sin_addr) << endl;
            cout << endl;

            QString IPString(inet_ntoa(sai->sin_addr));
            QStringList IPList;
            IPList = IPString.split(".");
            const quint8 ip1 = (quint8)(IPList[0].toInt());
            const quint8 ip2 = (quint8)(IPList[1].toInt());
            const quint8 ip3 = (quint8)(IPList[2].toInt());
            const quint8 ip4 = (quint8)(IPList[3].toInt());
            host_Ip = (quint32)(ip1 << 24) + (quint32)(ip2 << 16) + (quint32)(ip3 << 8) + (quint32)(ip4);
        }
    }
}

bool CUdpReceiver::print_hw_addr(int fd, const char* if_name) {
    ifreq req = {0};
    strcpy(req.ifr_name, if_name);
    if(-1 == ioctl(fd, SIOCGIFFLAGS, &req)) {
        perror("Failed IOCTL SIOCGIFFLAGS.");
        return false;
    }
    if(req.ifr_flags & IFF_LOOPBACK) {
        //cout << "Is LOOPBACK." << endl;
        return false;
    }

    if(-1 == ioctl(fd, SIOCGIFHWADDR, &req)) {
        perror("Failed IOCTL SIOCGIFHWADDR.");
        return false;
    }
    unsigned char* puc = (unsigned char*)req.ifr_hwaddr.sa_data;
    printf("HW addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
        puc[0], puc[1], puc[2], puc[3], puc[4], puc[5]);


    host_Mac = QString("%1:%2:%3:%4:%5:%6").arg(puc[0]).arg(puc[1]).arg(puc[2]).arg(puc[3]).arg(puc[4]).arg(puc[5]);


}

void CUdpReceiver::bindPort(quint16 port)
{
    m_port = port;

    struct sockaddr_in local;
    local.sin_family=AF_INET;
    local.sin_port=htons(m_port); ///监听端口
    local.sin_addr.s_addr=INADDR_ANY; ///本机
    int r = bind(m_socket,(struct sockaddr*)&local,sizeof(local));
    if(r < 0)
        qDebug() << "bind to port: error" << port;

    if(!m_thread->isRunning())
        m_thread->start();
}

// 设置组播地址
quint8 CUdpReceiver::setMultiBroadcast(QList<QHostAddress> multiAddress)
{
    //获取可用IP地址
    char IPaddress[30];
    GetHostIPAddress(IPaddress,1);

    /*加入组*/
    struct ip_mreq mip;

    /*将m_ReceiveSocketID加入PPI的组播地址中*/
    memset((char*)&mip,0,sizeof(mip));
    /*要使用组播，不能固定IP绑定在一起，但可以通过这里来区别*/
    mip.imr_interface.s_addr = inet_addr(IPaddress);
    /*将接收SOCKET加入组播地址*/
    QList<QHostAddress>::const_iterator it1 = multiAddress.constBegin(), it2 = multiAddress.constEnd();
    for(; it1 != it2; ++ it1)
    {
    mip.imr_multiaddr.s_addr = inet_addr((*it1).toString().toLocal8Bit());
    setsockopt(m_socket,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char*)&mip,sizeof(mip));
    }

    return 0;
}

// 网络监视线程调用，等待网络数据
void CUdpReceiver::run ()
{
    SOCKADDR_IN from;
    int fromlen = sizeof(from);
    int recvlength = 0;

    CCommunicate* pcom = (CCommunicate*)(parent());
    if(!pcom)
	{
		qDebug() << "CCommunicate object is NULL";
		return;
	}

    qDebug()<<"inter receive thread is running";
    forever
    {
        recvlength = recvfrom(m_socket,(char*)m_buffer,MAXRECVSIZE,0,
                (struct sockaddr*)&from,(socklen_t*)&fromlen);

        if (recvlength > 0)
        {
            //to save data
            {
                QHostAddress hostaddr((struct sockaddr*)&from);
                QByteArray datagram;
                datagram.resize(recvlength);
                memcpy (datagram.data(), m_buffer, recvlength);
                pcom->udpDataReceived(datagram, hostaddr, port());
                //qDebug() << "recv data:" << hostaddr << port();
            }
        }
    }
}

void CUdpReceiverThread::run()
{
    setTerminationEnabled(true);
    CUdpReceiver *udp = (CUdpReceiver*)(parent());
    if(udp)
        udp->run();
}

/**********************************
 * class : CUdpDevice
**********************************/
CUdpDevice::CUdpDevice(quint32 id, QObject* parent)
    :CBaseDevice(id, parent),m_udpReciever(NULL)
{
    m_deviceType = UDP_DEVICE;
}

CUdpDevice::~CUdpDevice()
{
    if(m_udpReciever)
        UdpFactory::deleteUdpReceiverObject(m_udpReciever);
}

bool CUdpDevice::open(const QHostAddress& addr, quint16 recvport, quint16 sendport)
{
    m_hostAddress = addr;
    m_recvPort = recvport;
    m_sendPort = sendport;//(sendport ? sendport : recvport);

    if(recvport)
    m_udpReciever = UdpFactory::createUdpReceiverObject(recvport, parent());

    return true;
}

// 设置组播地址
quint8 CUdpDevice::setMultiBroadcast(QList<QHostAddress> multiAddress)
{
    if(m_udpReciever)
        return m_udpReciever->setMultiBroadcast(multiAddress);
    else
        return -1;
}


//int CUdpDevice::recvData(QByteArray& data)
//{
//    return 0;
//}

//int CUdpDevice::sendData(const QByteArray& data)
//{
//    return 0;
//}

/**********************************
 * class : CCommunicate
**********************************/
CCommunicate::CCommunicate(QObject* parent)
:QObject(parent)
{
#ifdef Q_OS_WIN32
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2,2), &wsa_data);
#endif

    m_udpTransmiter = new CUdpTransmiter(this);
}

CCommunicate::~CCommunicate()
{
    delete m_udpTransmiter;
	closeAllDevices();
}

// 设备打开
quint32 CCommunicate::open(const QHostAddress& addr, quint16 recvport, quint16 sendport, quint32 defaultId)
{
    CUdpDevice* device = NULL;
    quint32 devid = defaultId;
    try
    {
        if(isDeviceIdUsed(devid))
            devid = applyDeviceId();

        if(devid)
        {
            device = new CUdpDevice(devid, this);
            if(!device->open(addr, recvport, sendport))
                throw 0;

            m_deviceTableMutex.lock();
            m_deviceObjectTable.insert(devid, device);
            m_deviceTableMutex.unlock();

            return devid;
        }
    }

    catch(...)
    {
        if(device)
            delete device;
        return 0;
    }

    return 0;
}

/*创建串口通信对象接口*/
quint32 CCommunicate::open(const QString & devicename, const SERIALCONFIG& cfg, quint32 defaultId)
{

    CSerialDevice* device = NULL;
    quint32 devid = defaultId;
    try
    {
        if(isDeviceIdUsed(devid))
            devid = applyDeviceId();

        if(devid)
        {
            device = new CSerialDevice(devid, this);
            device->setSerialConfig(cfg);
            if(!device->open(devicename))
                throw 0;

            m_deviceTableMutex.lock();
            m_deviceObjectTable.insert(devid, device);
            m_deviceTableMutex.unlock();

            qDebug()<< "serial device id is opened:"<< devid;

            return devid;
        }
    }

    catch(...)
    {
        if(device)
            delete device;
        return 0;
    }


    return 0;
}

quint32 CCommunicate::reopen(const QString & devicename, const SERIALCONFIG& cfg, quint32 id)
{

    try
    {
        CSerialDevice* device = (CSerialDevice*)(m_deviceObjectTable.value(id, NULL));
        if(!device)
             return open(devicename, cfg, id);

        m_deviceTableMutex.lock();
                device->setSerialConfig(cfg);
        m_deviceTableMutex.unlock();
        return id;
    }

    catch(...)
    {
        return 0;
    }


    return 0;
}



// 设备关闭
bool CCommunicate::close(quint32 id)
{
    QMutexLocker lock(&m_deviceTableMutex);
    CBaseDevice* object = m_deviceObjectTable.value(id, NULL);
    if(object)
    {
        m_deviceObjectTable.remove(id);
        delete object;
    }

    return true;
}

// 关闭所有设备
void CCommunicate::closeAllDevices()
{
    QMutexLocker lock(&m_deviceTableMutex);
	foreach(CBaseDevice* object, m_deviceObjectTable)
	{
		delete object;
	}
	m_deviceObjectTable.clear();
}

/*设置组播地址*/
quint8 CCommunicate::setMultiBroadcast(quint32 id, QList<QHostAddress> multiAddress)
{    
    QMutexLocker lock(&m_deviceTableMutex);
    CBaseDevice* object = m_deviceObjectTable.value(id, NULL);
    if(object && object->isUdpDevice())
    {
        CUdpDevice* udp = qobject_cast<CUdpDevice*>(object);
        return udp->setMultiBroadcast(multiAddress);
    }

    return -1;
}

quint8 CCommunicate::setMultiBroadcast(QList<QHostAddress> multiAddress)
{
    QHash<quint32, CBaseDevice*>::const_iterator it1, it2;
    it1 = m_deviceObjectTable.constBegin();
    it2 = m_deviceObjectTable.constEnd();
    for(; it1 != it2; ++it1)
    {
        CUdpDevice* udp = qobject_cast<CUdpDevice*>(it1.value());
        if(udp)
            udp->setMultiBroadcast(multiAddress);
    }

    return 0;
}
/*设置串口参数*/
quint8 CCommunicate::setSerialConfig(quint32 id, const SERIALCONFIG& cfg)
{

    QMutexLocker lock(&m_deviceTableMutex);
    CBaseDevice* object = m_deviceObjectTable.value(id, NULL);
    if(object && object->isSerialDevice())
    {
        CSerialDevice* comm = qobject_cast<CSerialDevice*>(object);
        return comm->setSerialConfig(cfg);
    }

    return -1;
}

/*判断设备是否有效*/
int CCommunicate::isDeviceValid(quint32 id)
{
    //QMutexLocker locker(&m_objectTableLock);
    CBaseDevice* object = m_deviceObjectTable.value(id, NULL);
    return (object ? object->isValid() : false);
}

/*申请可用的设备号*/
quint32 CCommunicate::applyDeviceId() const
{
    for(int id=1; id<MAXDEVICEID; id++)
    {
        if(!isDeviceIdUsed(id))
            return id;
    }
    return 0;
}

/*判断设备号是否已经使用，返回0:没有使用，1:已经使用*/
int CCommunicate::isDeviceIdUsed(quint32 id) const
{
    return (m_deviceObjectTable.contains(id) ? 1 : 0);
}
/////send data function
int CCommunicate::sendData(const QByteArray& data, quint32 id)
{
    QMutexLocker locker(&m_deviceTableMutex);

    CBaseDevice* dev = m_deviceObjectTable.value(id,NULL);
    if(dev->isUdpDevice())
    {	// send data from udp socket
            CUdpDevice* udp = qobject_cast<CUdpDevice*>(dev);
            if(udp)
                m_udpTransmiter->sendTo(data, udp->hostAddress(), udp->sendPort());
            else
                qDebug() << "CCommunicate::sendData error. No dev id:" << id;
    }
    else if(dev->isSerialDevice())
    {
        // send data from serial com
        CSerialDevice* com = qobject_cast<CSerialDevice*>(dev);
        if(com)
                com->sendData(data);
    }

    return id;
}

quint32 CCommunicate::recvData(QByteArray& data)
{
    tagDeviceData  devdata;
    //qDebug() << "recvData in CCompunicate" << __func__;
    m_dataRecved.acquire();//用于实现数据的多线程公用同步
    //qDebug() << "recvData in CCompunicate acquire" << __func__;
    m_dataRecvedMutex.lock();
    devdata = m_dataRecvedList.takeFirst();
    m_dataRecvedMutex.unlock();

    data = devdata.data;
    return devdata.deviceid;
}

void CCommunicate::udpDataReceived(const QByteArray& data, const QHostAddress& addr, quint16 port)
{
    const quint32 id = udpDeviceId(addr, port);
   // qDebug()<<"id is"<<id;
    if(id)
    {
        QMutexLocker locker(&m_dataRecvedMutex); //这样调用recvData中的m_dataRecvedMutex，即写入数据的时候不能读取
        //qDebug() << "CCommunicate::udpDataReceived" << __func__ << id;
        m_dataRecvedList.append(tagDeviceData(id,data));
        m_dataRecved.release();
    }
}
/*接收到串口数据*/
void CCommunicate::serialDataReceived(const QByteArray& data, quint32 devid)
{
    if(m_deviceObjectTable.contains(devid))
    {
        QMutexLocker locker(&m_dataRecvedMutex); ////这样调用recvData中的m_dataRecvedMutex，即写入数据的时候不能读取
        m_dataRecvedList.append(tagDeviceData(devid, data));
        m_dataRecved.release();
    }
}

// 通过IP地址和端口号查找对应的设备ID
quint32 CCommunicate::udpDeviceId(const QHostAddress& addr, quint16 recvport)
{
    QMutexLocker locker(&m_deviceTableMutex);

    CUdpDevice* device;
    QHash<quint32, CBaseDevice*>::const_iterator it1=m_deviceObjectTable.begin();
    QHash<quint32, CBaseDevice*>::const_iterator it2=m_deviceObjectTable.end(), it;
    for(it=it1; it!=it2; ++it)
    {
        device = qobject_cast<CUdpDevice*>(*it);
        if(device && device->hostAddress() == addr && \
           device->recvPort() == recvport)
            return device->deviceid();
    }

    return 0;
}

/**********************************
 * class : UdpFactory
**********************************/
QHash<quint16, CUdpReceiver*> UdpFactory::m_udpBinded;
QMutex UdpFactory::m_udpMutex;

CUdpReceiver* UdpFactory::createUdpReceiverObject(quint16 port, QObject* parent)
{
    QMutexLocker locker(&m_udpMutex);
    CUdpReceiver* object = m_udpBinded.value(port, NULL);
    if(!object)
    {
        object = new CUdpReceiver(parent);
        object->bindPort(port);
        m_udpBinded.insert(port, object);
    }
    else
    {
        // 重复使用，增加引用计数
        object->increaseReference();
    }

    return object;
}

void UdpFactory::deleteUdpReceiverObject(CUdpReceiver* udp)
{
    if(udp->decreaseReference() == 0)
    {
        m_udpMutex.lock();
        m_udpBinded.remove(udp->port());
        m_udpMutex.unlock();

        delete udp;
    }
}

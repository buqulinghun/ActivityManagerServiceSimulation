#include "implcommobject.h"

const int SerialMaxBufferSize = 1000;

CSerialDevice::CSerialDevice(quint32 id, QObject *parent)
    :CBaseDevice(id, parent),m_serialRecvThread(NULL),m_serialPort(NULL)
{
    m_deviceType = SERIAL_DEVICE;
    m_recvBuffer = new char[SerialMaxBufferSize];
}
CSerialDevice::~CSerialDevice()
{
    close();
    delete[] m_recvBuffer;
}

/////////////
BaudRateType getBaudRateType(quint32 baudrate)
{
        static quint32 pivots[] = {
                50,                //POSIX ONLY
                75,                //POSIX ONLY
                110,
                134,               //POSIX ONLY
                150,               //POSIX ONLY
                200,               //POSIX ONLY
                300,
                600,
                1200,
                1800,              //POSIX ONLY
                2400,
                4800,
                9600,
                14400,             //WINDOWS ONLY
                19200,
                38400,
                56000,             //WINDOWS ONLY
                57600,
                76800,             //POSIX ONLY
                115200,
                128000,            //WINDOWS ONLY
                256000             //WINDOWS ONLY
        };
        for(int i = 0; i < sizeof(pivots)/sizeof(quint32); ++i)
             if(pivots[i] == baudrate)
                 return (BaudRateType)(i);
        return BAUD50;
}

DataBitsType getDataBitsType(quint8 databit)
{
        static  quint8 pivots[] = {
                5,6,7,8
        };
        for(int i =0; i < sizeof(pivots)/sizeof(quint8); ++i)
                if(pivots[i] == databit)
                        return (DataBitsType)i;
        return DATA_5;
}

StopBitsType getStopBitsType(quint8 stopbit)
{
        static  quint8 pivots[] = {
                1,3, 2
        };
        for(int i =0; i < sizeof(pivots)/sizeof(quint8); ++i)
                if(pivots[i] == stopbit)
                        return (StopBitsType)i;
        return STOP_1;
}

ParityType getParityType(quint8 parity)
{
        return (ParityType)parity;
}

//for linux: "/dev/ttyS0"  "/dev/ttyS1"
bool CSerialDevice::open(const QString & name)
{
    if(!m_serialPort)
    {
            m_serialName = name;
            m_serialPort = new QextSerialPort(m_serialName, QextSerialPort::Polling);
            if(!m_serialPort->open(QIODevice::ReadWrite))
                    qDebug() << "open device " << name << " error.";
            m_serialPort->setBaudRate(getBaudRateType(m_baudrate));
            m_serialPort->setDataBits(getDataBitsType(m_databit));
            m_serialPort->setStopBits(getStopBitsType(m_stopbit));
            m_serialPort->setParity(getParityType(m_parity));
            m_serialPort->setFlowControl(FLOW_OFF);
            m_serialPort->setTimeout(0);  //等待时间没有特定要求，每次接受三个字节

            m_serialRecvThread = new CSerialReceiverThread(this);
            m_serialRecvThread->start();
            //if(!connect(m_serialPort, SIGNAL(readyRead()), this, SLOT(recvData())))
            //	qDebug() << "connect signal QextSerialPort::readyRead to slot recvData error.";
            // listener->connect(port, SIGNAL(readyRead()), listener, SLOT(receive()));
    }

    return 1;

}
/*设置串口参数*/
quint8 CSerialDevice::setSerialConfig(const SERIALCONFIG& cfg)
{
    m_baudrate = cfg.baudrate;
    m_databit = cfg.databit;
    m_stopbit = cfg.stopbit;
    m_parity = cfg.parity;

    if(m_serialPort){
        m_serialPort->setBaudRate(getBaudRateType(m_baudrate));
        m_serialPort->setDataBits(getDataBitsType(m_databit));
        m_serialPort->setStopBits(getStopBitsType(m_stopbit));
        m_serialPort->setParity(getParityType(m_parity));
    }

    return 1;
}

bool CSerialDevice::close()
{

    if(m_serialRecvThread)
    {
            m_serialRecvThread->stop();
            delete m_serialRecvThread;
    }

    if(m_serialPort)
    {
            m_serialPort->close();
            delete m_serialPort;
    }

    return true;
}
// 通过串口发送数据
int CSerialDevice::sendData(const QByteArray& data)
{
    if(m_serialPort)
        return m_serialPort->write(data);
    return -1;
}

// 接收数据
void CSerialDevice::recvData()
{

    int recvlength;
    if(m_serialPort)// && m_serialPort->waitForReadyRead(500))
    {

        //qDebug() << "enter recvData:222";
         int length = m_serialPort->bytesAvailable();
         while(length > 0)
         {
             recvlength = m_serialPort->read(m_recvBuffer, qMin(SerialMaxBufferSize, length));
             if(recvlength > 0)
             {
                 length -= recvlength;

                 //to save data
                 CCommunicate* pcom = (CCommunicate*)(parent());
                 if(pcom)
                 {
                         QByteArray datagram;
                         datagram.resize(recvlength);
                         memcpy (datagram.data(), m_recvBuffer, recvlength);
                         pcom->serialDataReceived(datagram, deviceid());
                         //qDebug() << "recv data:" << recvlength << ByteArray2String(datagram);
                 }
             }
             else if (recvlength == 0)
             {
                 qDebug() << "recv no data.";
                 break;
             }
             else
             {
                 qDebug() << "recv error.";
                 break;
             }
         }

     }

}

void CSerialReceiverThread::run()
{
    CSerialDevice* pSerial = qobject_cast<CSerialDevice*>(parent());
    if(!pSerial)
            return;

    setTerminationEnabled(true);

    qDebug() << "CSerialReceiverThread::run()";

    for(;;)
    {
            pSerial->recvData();
            msleep(3);
            if(m_stopFlag)
                 break;
    }

}





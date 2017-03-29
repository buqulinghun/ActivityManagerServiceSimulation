/****************************************************************************
file name: interact.cpp
author: wang ming xiao
date: 2015/07/05
comments: 数据接受线程，后面要增加串口通信方面的内容，数据处理在openglview.cpp文件中
***************************************************************************/
#include "interact.h"
#include "communicate/communicate.h"
#include "dataprocess.h"
#include "../ecdis.h"

#include <QFile>


extern Interact * lpInteract;
extern DataProcess *lpDataProcess;
extern SYSTEMINFO RadarSysinfo;

quint8 antennaWarning=0;
quint8 firstStartFlag = 0;  //启动标志，第一圈的数据丢掉，等0角度的信号才开始画


Interact::Interact()
        :QThread(), m_pCommunicate(new Communicate)
{
    m_openFlag = 0;  //默认为0
    m_gain = 80;
    m_clutter = 0;
    m_restrain = 0;
    m_tune = 0;
    m_gain_real = 0;
}

Interact::~Interact()
{
    terminate(); //终止线程
    if(m_pCommunicate)
        delete m_pCommunicate;

}
//打开UDP通信和串口通信设备
bool Interact::initialize()
{
    //打开通信设备，发送数据的目的地址，接收端口号，发送端口号，设备ID号
    parseConfigFile(CURRENTPATH + "/communicate.cfg");
    createAllDevice();

    //启动线程处理
    start();

    return true;
}
//打开串口设备
void Interact::createSerialDevice()
{
   const quint32 id = Device_KDB;

   SERIALCONFIG cfg;
   cfg.baudrate = 115200;
   cfg.parity = 0;
   cfg.databit = 8;
   cfg.stopbit = 0;

    // 打开设备
    m_pCommunicate->open("/dev/ttyUSB0", cfg, id);
    {
        qDebug()<<"open /dev/ttyUSB0 success!";
    }

}


//解析配置文件
void Interact::parseConfigFile(const QString &configFileName)
{
    QFile file(configFileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug()<<"Interact: configFileName can't open file:"<<configFileName;
        return;
    }

    DEVICECONFIG* config;

    //以#开头为注释
    QChar notechar('#');
    QTextStream in(&file);
    QString line;
    while(1) {
        line = in.readLine();
        //去掉头尾多余空格
        line = line.trimmed();
        //到达文件尾
        if(line.isNull())
            break;

        //如果为空活或者为注释则不处理
        if(line.isEmpty() || line.startsWith(notechar))
            continue;

        QStringList splits = line.split(",", QString::SkipEmptyParts);
        const int size = splits.size();
        if(size == 0 || splits[0].isEmpty())
            continue;

        if(size == 5) {
           // 网络配置格式  name id ip recvport  sendport
            const QString name = splits[0].toUpper();
            const quint32 deviceid = splits[1].toInt();
            if(deviceid > 0) {
                config = new DEVICECONFIG;
                config->name = name;
                config->deviceid = deviceid;
                config->type = 0;  //0 udp 1 serial
                config->udpParam.hostAddress.setAddress(splits[2]);
                config->udpParam.recvport = splits[3].toInt();
                config->udpParam.sendport = splits[4].toInt();
                m_deviceConfigs.insert(deviceid, config);
            }

        }else if(size == 7) {
            //串口配置格式  name id serialname baudrate parity databit stopbit
            const QString name = splits[0].toUpper();
            const quint32 deviceid = splits[1].toInt();
            if(deviceid >0) {
                config = new DEVICECONFIG;
                config->name = name;
                config->deviceid = deviceid;
                config->type = 1;
                config->serialParam.deviceName = splits[2];
                config->serialParam.baudrate = splits[3].toInt();
                config->serialParam.parity = (splits[4] == "N" ? 0 : 1);
                config->serialParam.databit = splits[5].toInt();
                config->serialParam.stopbit = splits[6].toInt();
                m_deviceConfigs.insert(deviceid, config);
            }
        }

    }

}

void Interact::createAllDevice()
{
    quint32 id;
    DEVICECONFIG* config;
    QHash<quint32, DEVICECONFIG*>::const_iterator it,it1,it2;
    it1 = m_deviceConfigs.constBegin();
    it2 = m_deviceConfigs.constEnd();
    for(it=it1; it!=it2; ++it) {
        id = it.key();
        config = it.value();
        if(config->type == 0) {  //udp
            //打开通信设备，发送数据的目的地址，接收端口号，发送端口号，设备ID号
            m_pCommunicate->open(config->udpParam.hostAddress, config->udpParam.recvport, config->udpParam.sendport, id);
            qDebug()<<"udp device open success!";

        }else {

            SERIALCONFIG cfg;
            cfg.baudrate = config->serialParam.baudrate;
            cfg.parity = config->serialParam.parity;
            cfg.databit = config->serialParam.databit;
            cfg.stopbit = config->serialParam.stopbit;

             // 打开设备
             m_pCommunicate->open(config->serialParam.deviceName, cfg, id);
             {
                 qDebug()<<"open serial device success"<<config->serialParam.deviceName;
             }

        }
    }
}


//开机/关机命令发送
void Interact::BtnOpenClose()
{

    //发送开机命令
    QByteArray senddata;
    int x;
    QDataStream in(&senddata, QIODevice::WriteOnly);
    if(! m_openFlag) {
        in<<(quint8)170<<(quint8)0<<(quint8)0<<(quint8)0<<(quint8)1;  //AA00000001  ST
        if((x=FpgaWrite(senddata)) == Device_FPGA) {
            qDebug()<<"start success!";
            m_openFlag = 1;  //开机标志置位
            RadarSysinfo.transmite = true;

            //发送IP和MAC
            sendIpAndMac();

            //发送默认的配置
            //HL1710配置
            change_HL1710(RadarSysinfo.antennaSelect);
            //量程配置
            senddata.clear();
            const quint8 r = RadarSysinfo.RangeScale.rngIndex();
             in<<(quint8)170<<(quint8)0<<(quint8)9<<(quint16)(RadarSysinfo.bestTuneValue[r])  \
             <<(quint8)170<<(quint8)0<<(quint8)0x19<<(quint16)(RadarSysinfo.bestTuneAdd[r])  \
             <<(quint8)170<<(quint8)0<<(quint8)1<<(quint8)0<<(quint8)r;
            if(this->FpgaWrite(senddata) != Device_FPGA)  qDebug()<< "config FPGA range error!";
            //增益配置
            change_gain(2);
            //雨雪配置
            change_restrain(2);
            //海浪配置
            change_clutter(2);
            //同频干扰配置
           // pView->setJam(MenuConfig.otherMenu.samefreqJam+5);
            //MBS配置
           // setMBSAdjust(MenuConfig.installMenu.mbsAdjust);
            //距离调节配置
            setRngAdjust(RadarSysinfo.rngAdjust);
            //关闭偏心
            senddata.clear();
            in<<(quint8)170<<(quint8)0<<(quint8)8<<(quint8)0<<(quint8)0;
            if(this->FpgaWrite(senddata) != Device_FPGA)  qDebug()<< "config FPGA offset error!";

        }else{
            qDebug()<<"start error";
        }

    }else {
        in<<(quint8)170<<(quint8)0<<(quint8)0<<(quint8)0<<(quint8)0;  //AA00000000  BY
        if((x=FpgaWrite(senddata)) == Device_FPGA) {
            qDebug()<<"close success!";

            m_openFlag = 0;
            RadarSysinfo.transmite = 0;
            RadarSysinfo.offset  = 0;

            //清空显示数据

        }else{
            qDebug()<<"close error";
        }
    }

}

void Interact::sendIpAndMac()
{
    extern quint32 host_Ip;
    extern QString host_Mac;

 //   const quint32 Ip = host_Ip;
    const quint32 Ip = 0xac1834fe;
    const QString Mac = host_Mac;

    QByteArray senddata;
    QDataStream in(&senddata, QIODevice::WriteOnly);
    quint16 heigh=0,lower=0;

     if(lpInteract) {
         //发送IP 地址
         heigh = (quint16)((Ip >> 16) & (0xffff));
         lower = (quint16)(Ip & 0xffff);
         in<<(quint8)170<<(quint8)0<<(quint8)0x10<<(quint16)(heigh);  //AA0010 高16位
         if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
         // qDebug()<< "FPGA write Ip heigh 16 success!";
         senddata.clear();
         in<<(quint8)170<<(quint8)0<<(quint8)0x11<<(quint16)(lower);  // AA0011 低16位
         if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
         // qDebug()<< "FPGA write Ip lower 16 success!";
         senddata.clear();

         //发送MAC地址
         if(!Mac.isEmpty()) {
             QStringList IPList;
             IPList = Mac.split(":");
             const quint8 mac1 = (quint8)(IPList[0].toInt());
             const quint8 mac2 = (quint8)(IPList[1].toInt());
             const quint8 mac3 = (quint8)(IPList[2].toInt());
             const quint8 mac4 = (quint8)(IPList[3].toInt());
             const quint8 mac5 = (quint8)(IPList[4].toInt());
             const quint8 mac6 = (quint8)(IPList[5].toInt());
             in<<(quint8)170<<(quint8)0<<(quint8)0x12<<(quint8)mac1<<(quint8)mac2;  //AA0012 高16位
             if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
             // qDebug()<< "FPGA write Mac heigh 16 success!";
             senddata.clear();
             in<<(quint8)170<<(quint8)0<<(quint8)0x13<<(quint8)mac3<<(quint8)mac4;  // AA0013 中16位
             if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
             // qDebug()<< "FPGA write Mac middle 16 success!";
             senddata.clear();
             in<<(quint8)170<<(quint8)0<<(quint8)0x14<<(quint8)mac5<<(quint8)mac6;  // AA0014 低16位
             if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
             // qDebug()<< "FPGA write Mac lower 16 success!";
         }

     }

}


int Interact::FpgaWrite(QByteArray &data)
{
    return m_pCommunicate->sendData(data, Device_FPGA);
}
int Interact::SeirialWrite(QByteArray &data)
{
    return m_pCommunicate->sendData(data, Device_KDB);
}





//数据处理线程
void Interact::run()
{
    quint32 id;
    QByteArray data;


    msleep(200);  //休眠200毫秒


    setTerminationEnabled(true);
    qDebug() << "Data receiver process thread is running!";

    forever {

        id = m_pCommunicate->recvData(data);

        switch(id){
        case Device_FPGA:
        {
            //收到UDP数据包，解包处理
            lpDataProcess->dataProcess(data);

        }
        break;
    /*    case Device_KDB:
        //键盘信号处理
        {
            //serial data,每次接受的数据为三个字节,基本上不会超过，只会少于
            if((data.size() >= 3))
                emit sigbalKbdData(data);

        }
        break;
        case Device_AIS:
        //GPS和AIS和罗经数据处理
        {
            const LINKSTATUS* lpStatus = lpMainWindow->linkStatus();
            if(NORMAL == lpStatus[GPS_DEVICE].status)  //GPS数据有效才能使用AIS
                lpParseDevice->unpacking_AIS(data, id);
        }
        break;
        case Device_GPS:
        {
            lpParseDevice->unpacking_GPS(data, id);
        }
        break;
        case Device_COMP:
        {
            lpParseDevice->unpacking_COMP(data, id);
        }
        break;
        case Device_LOG:
        {
            lpParseDevice->unpacking_LOG(data, id);
        }
        break;  */

        default:
        break;
    }
    }  //end while

}


void Interact::change_HL1710(int flag)
{
    QByteArray senddata;
    QDataStream in(&senddata, QIODevice::WriteOnly);

    if(lpInteract) {
        in<<(quint8)170<<(quint8)0<<(quint8)6<<(quint8)0<<(quint8)flag;  //AA0006 0000 --- AA0006 0001
        if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
           // qDebug()<< "FPGA write HL1710 success!"<< flag;
    }

}
void Interact::change_gain(int flag)
{

    QByteArray senddata;
    int x;
    QDataStream in(&senddata, QIODevice::WriteOnly);
    //flag  1:up   0:down 2:nochange
    switch(flag) {
        case 1:
        if(m_gain < 100){
            m_gain++;
            x = 1;
         }
        break;
        case 0:
        if(m_gain > 0) {
            m_gain--;
            x = 1;
        }
        break;
        case 2:
        x=1;
        break;

    }

    if(x && lpInteract) {
        if(m_gain < 28)
            m_gain_real = m_gain + m_gain;
        else
            m_gain_real = m_gain + 27;

        in<<(quint8)170<<(quint8)0<<(quint8)4<<(quint8)0<<(quint8)m_gain_real;  //AA0004 0001 --- AA0004 007F   0-127
        if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
           // qDebug()<< "FPGA write gain success!"<< m_gain;

    }
}

void Interact::change_restrain(int flag)
{
    quint8 m_restrain_real;
    QByteArray senddata;
    int x;
    QDataStream in(&senddata, QIODevice::WriteOnly);
    //flag  1:up   0:down
    switch(flag) {
        case 1:
        if(m_restrain < 100){
            m_restrain++;
            x = 1;
         }
        break;
        case 0:
        if(m_restrain > 0) {
            m_restrain--;
            x = 1;
        }
        break;
        case 2:
        x=1;
        break;

    }

    if(x && lpInteract) {
        if(m_restrain < 70)
            m_restrain_real = m_restrain;
        else
            m_restrain_real = m_restrain + m_restrain - 70;

        in<<(quint8)170<<(quint8)0<<(quint8)2<<(quint8)0<<(quint8)m_restrain_real;  //AA0002 0001 --- AA0002 007F   0-127
        if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
          //  qDebug()<< "FPGA write restrain success!"<< m_restrain;
    }
}
void Interact::change_clutter(int flag)
{
    quint8 m_clutter_real;
    QByteArray senddata;
    int x;
    QDataStream in(&senddata, QIODevice::WriteOnly);
    //flag  1:up   0:down
    switch(flag) {
        case 1:
        if(m_clutter < 100){
            m_clutter++;
            x = 1;
         }
        break;
        case 0:
        if(m_clutter > 0) {
            m_clutter--;
            x = 1;
        }
        break;
        case 2:
        x=1;
        break;

    }
    if(x && lpInteract) {
        if(m_clutter < 70)
            m_clutter_real = (quint8)(m_clutter);
        else
            m_clutter_real = (quint8)(m_clutter + m_clutter - 70);
        in<<(quint8)170<<(quint8)0<<(quint8)3<<(quint8)0<<(quint8)m_clutter_real;  //AA0003 0001 --- AA0003 003F   0-63
        if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
          //  qDebug()<< "FPGA write restrain success!"<< m_clutter;
    }
}
void Interact::setRngAdjust(int rng)
{
    QByteArray senddata;
    QDataStream in(&senddata, QIODevice::WriteOnly);

    RadarSysinfo.rngAdjust = rng;

     if(lpInteract) {
         in<<(quint8)170<<(quint8)0<<(quint8)13<<(quint8)0<<(quint8)rng;  //AA000d 0000 --- AA000d 00FF
         if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
         // qDebug()<< "FPGA write rng success!"<< flag;
     }

}

//调谐
void Interact::change_tune(quint8 flag, quint32 val)
{
    QByteArray senddata;
    QDataStream in(&senddata, QIODevice::WriteOnly);
    //0:手动  1：自动
    if(!flag) {


            in<<(quint8)170<<(quint8)0<<(quint8)5<<(quint8)0<<(quint8)1;  //AA0005 0000 --- AA0005 0001  MT-AT
            if(lpInteract && (lpInteract->FpgaWrite(senddata) == Device_FPGA))
           // qDebug()<< "FPGA write  man tune success!";
            senddata.clear();


            //发送手动调谐值
            in<<(quint8)170<<(quint8)0<<(quint8)7<<(quint16)val;  //AA0007 0000 --- AA0007 03FF  0-1023
            if(lpInteract && (lpInteract->FpgaWrite(senddata) == Device_FPGA));
            //   qDebug()<< "FPGA write  man tune data success!"<<val;


    }else {
        if(val == 65500) {  //表示开机后的自动调谐
            in<<(quint8)170<<(quint8)0<<(quint8)5<<(quint8)0<<(quint8)3;
            if(lpInteract && (lpInteract->FpgaWrite(senddata) == Device_FPGA));
             //   qDebug()<< "FPGA write  auto tune success!";

        }else {  //正常运行时的调谐，命令为0
            in<<(quint8)170<<(quint8)0<<(quint8)5<<(quint8)0<<(quint8)0;
            if(lpInteract && (lpInteract->FpgaWrite(senddata) == Device_FPGA));
              //  qDebug()<< "FPGA write normal auto tune success!";
        }
    }

}

//量程改变
void Interact::rangeChanged(bool flag)
{
    quint8 x = RadarSysinfo.RangeScale.rngIndex();
    const quint32 ran = 0xffffffff;
    //1：up  0: down
    if(flag) {
        if(x < 18)
            x++;
        else
            return;

        while(!(ran & ((quint32)1 << x))) {  //该位有量程显示
           if(x < 18)
               x++;
           else
               return;
        }
    }else {
        if(x > 0)
            x--;
        else
            return;

        while(!(ran & ((quint32)1 << x))) {  //该位有量程显示
           if(x > 0)
               x--;
           else
               return;
        }
    }


    if(RadarSysinfo.transmite)
    {
        //向FPGA发送量程变化命令
        QByteArray senddata;
        QDataStream in(&senddata, QIODevice::WriteOnly);
        if(lpInteract) {

            if(RadarSysinfo.bestTuneValue[x] != 0) {
                in<<(quint8)170<<(quint8)0<<(quint8)9<<(quint16)(RadarSysinfo.bestTuneValue[x])  \
                 <<(quint8)170<<(quint8)0<<(quint8)0x19<<(quint16)(RadarSysinfo.bestTuneAdd[x])   \
                <<(quint8)170<<(quint8)0<<(quint8)1<<(quint8)0<<(quint8)x;  //AA0009 0000----发送最佳调谐值
              //  lpMainWindow->setManTuneValue(MenuConfig.installMenu.bestTuneValue[x]);
            } else {
                in<<(quint8)170<<(quint8)0<<(quint8)9<<(quint16)(450)<<(quint8)170<<(quint8)0<<(quint8)1<<(quint8)0<<(quint8)x;
              //  lpMainWindow->setManTuneValue(256);
             }
            if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
            //  qDebug()<< "FPGA write the best tune success!"<< x;

        }

    }

}
//偏心设置
void Interact::setOffset(quint8 ofst)
{
        //偏心操作
        if(ofst == 1) {
            //向FPGA发送命令
            QByteArray senddata;
            QDataStream in(&senddata, QIODevice::WriteOnly);
            if(lpInteract) {
                in<<(quint8)170<<(quint8)0<<(quint8)8<<(quint8)0<<(quint8)1;  //AA0008 0001 --- AA0008 0000   AM  NM
                if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
                   // qDebug()<< "FPGA write offset open success!";
            }

        }else {
            //向FPGA发送命令
            QByteArray senddata;
            QDataStream in(&senddata, QIODevice::WriteOnly);
            if(lpInteract) {
                in<<(quint8)170<<(quint8)0<<(quint8)8<<(quint8)0<<(quint8)0;
                if(lpInteract->FpgaWrite(senddata) == Device_FPGA);
                   // qDebug()<< "FPGA write offset close success!";
            }
        }

 }






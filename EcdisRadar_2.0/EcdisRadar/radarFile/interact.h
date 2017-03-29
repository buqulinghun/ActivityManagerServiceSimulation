#ifndef INTERACT_H
#define INTERACT_H

#include <QThread>
#include <QObject>
#include <QHostAddress>


typedef struct tagUdpParam  //UDP参数标签
{
    QHostAddress    hostAddress;
    quint16         recvport;
    quint16         sendport;
}UDPPARAM;

typedef struct tagSerialParam //串口参数标签
{
        QString  deviceName; //设备名称,/dev/ttyUSB0
        quint32	 baudrate;   //比特率  传输速度
        quint8	 databit;    //数据位  标准7位，扩展的8位
        quint8	 stopbit;    //停止位  字节的最后一位
        quint8   parity;     //校验位
}SERIALPARAM;

typedef struct DEVICECONFIG    //设备配置
{
    quint32     deviceid;   // 设备ID
    QString     name;       // 设备名称
    quint8	type;	   // 类型 0:udp 1:serial

    UDPPARAM	udpParam;
    SERIALPARAM	serialParam;

}DEVICECONFIG;


class Communicate;


class Interact : public QThread
{
    Q_OBJECT
public:
    Interact();
    virtual ~Interact();

public:
    //初始化处理
    bool initialize();
    //create new serial device
    void createSerialDevice();


    /***************一些按键的操作程序**************************/
    //开机，关机命令
    void BtnOpenClose();
    //往FPGA发送数据
    int FpgaWrite(QByteArray &data);  //返回设备ID号
    //往串口发信息
    int SeirialWrite(QByteArray &data);




    /*******************************************************/
    //一些配置接口
    void change_HL1710(int flag);
    void change_gain(int flag);
    void change_restrain(int flag);
    void change_clutter(int flag);
    void setRngAdjust(int rng);
    void change_tune(quint8 flag, quint32 val);
    void rangeChanged(bool flag);
    void setOffset(quint8 ofst);

signals:
    void signalsFlushView(void);

    //按键操作
    void sigbalKbdData(QByteArray data);

protected:
    //数据处理线程
    void run();
    //发送IP和MAC地址
    void sendIpAndMac();

private:
    //解析配置文件
    inline void parseConfigFile(const QString& configFileName);
    //打开所有设备
    void createAllDevice();


    QHash<quint32, DEVICECONFIG*> m_deviceConfigs;
    Communicate *m_pCommunicate;  //通信接口对象
    quint8 m_openFlag;  //开机标志


    //信号处理参数
    int m_gain;
    int m_restrain;
    int m_clutter;
    int m_tune;  //调谐值
    quint8 m_gain_real;

};

#endif // INTERACT_H

#ifndef PARSEDEVICE_H
#define PARSEDEVICE_H

#include "parseais.h"
#include "parsegps.h"
#include <QHash>
#include <QMutex>



//默认的为AIS，其他为GPS COMP LOG

typedef struct tagReservePackage
{
    quint8 totalPackageSize[10];  //AIS数据包中一包数据最多分割为9包数据，连续识别标志0-9，牺牲空间方便编程
    QByteArray packageList[10];
}RESERVEPACKAGE;

enum {
    WAIT_START = 0,
    WAIT_CONTENT,
    WAIT_CHECKCODE
};


class ParseDevice
{
public:
    ParseDevice();

    //解包处理
    void unpacking_AIS(const QByteArray& datagram, quint32 deviceid);
    void unpacking_GPS(const QByteArray &datagram, quint32 deviceid);
    void unpacking_COMP(const QByteArray &datagram, quint32 deviceid);
    void unpacking_LOG(const QByteArray &datagram, quint32 deviceid);
    //校验
    quint8 checkCode(char* m_data_buffer, quint16 m_data_index_w);

    //数据处理接口
    void dataProcess(const char* data, quint16 length, quint32 deviceid=0);
protected:

    //完整包处理
    inline void packetProcess(const char* data, quint16 length, quint32 deviceid=0);
    //处理字符串结束符
    inline void vCharProcess(char* data, quint16 length);




private:
    Ais* AisObj;
    CGPS* GpsObj;


    //id, value
    QHash<int, RESERVEPACKAGE> m_reservePackage;  //存储合并的数据，可能会有几个ID的数据一起处理，所以用容器


    QMutex m_mutex;

    quint8 m_check_code;
    quint32 m_max_datasize;


    char* m_data_buffer_AIS; //存储解包后的数据
    char* m_data_buffer_GPS;
    char* m_data_buffer_COMP;
    char* m_data_buffer_LOG;

    quint8  m_data_format_AIS;
    quint8  m_data_format_GPS;
    quint8  m_data_format_COMP;
    quint8  m_data_format_LOG;
    quint32 m_data_index_w_AIS;
    quint32 m_data_index_w_GPS;
    quint32 m_data_index_w_COMP;
    quint32 m_data_index_w_LOG;

};

#endif // PARSEDEVICE_H

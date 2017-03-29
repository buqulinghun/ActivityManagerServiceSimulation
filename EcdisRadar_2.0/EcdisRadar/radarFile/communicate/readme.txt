发送：使用sendto函数发送原始报文
接收：使用recvfrom函数，对应每一个端口开一个线程，来调用recvfrom接收数据

udp接收对象（udpReceiver）：管理一个socket对象，一个线程，用于接收指定端口的数据。
    若收到数据，则按数据的来源IP地址，保存到相应的设备对象。
udp发送对象(udpTransmiter):具体是维护一个用于数据发送的socket对象和数据发送线程，
以实现对数据的发送；

串口对象：

设备对象(CBaseDevice)：主机与之通信的实体设备的虚拟表示，有唯一ID号，提供收发数据的接口，
    具体实现为以下几种，从而封装了不同通讯设备对象的实现。
UDP设备对象(CUdpDevice)：对应一个IP地址，本机与之通信的2个端口号（接收数据和发送数据）
串口设备对象(CSerialDevice)：

通信管理对象(CCommunication)：
    负责管理数据发送，系统使用一个udpTransmiter对象，来发送所有的数据
    负责管理数据接收，维护udpReceiver对象与端口号关系表
    负责设备对象管理，维护一张设备对象与IP地址关系表，

    ID open(param):用于创建设备对象，返回相应设备ID；
    void close(ID):释放指定的设备。
    void closeAll();

    sendData(ID, data):发送数据到指定的设备
    ID recvData(Data):接收数据，若有数据，则返回设备ID，否则返回-1。

    udpRecvData(data, addr, port);接收UDP数据处理，按IP和端口号查找对应的设备，
        如果找到，则保存数据
    serialRecvData(data, serialport);接收串口数据处理，按串口号查找对应的设备，
        如果找到，则保存数据


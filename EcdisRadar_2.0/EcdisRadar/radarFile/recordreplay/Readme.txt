记录重演库说明:
1、记录，提供StartRecord, Recording,StopRecord三个接口；
2、重演，提供两种方式，一是传统需要按时间重演，二是不用等待的重演，数据将连续不间断的输出，不需要进行时间判断；
        提供读取重演数据的接口(GetReplayData(QByteArray&)),该函数以阻塞的方式实现,需要时间到才将数据输出.

#ifndef COMPUTERINFO_H
#define COMPUTERINFO_H


#include <stdio.h>
#include <stdint.h>
#include <string.h>





class ComputerInfo
{
public:
    ComputerInfo();

    //获得CPU厂商信息
    void GetCpuProducerInfo(char *prod);
    void GetCpuSerialNumber(char *id);


private:



};

#endif // COMPUTERINFO_H

#include "computerinfo.h"
#include <QDebug>

ComputerInfo::ComputerInfo()
{


}

void ComputerInfo::GetCpuProducerInfo(char *prod)
{
    unsigned int eax = 0;
    unsigned int ebx, ecx, edx;

    asm volatile   //volatile表示对该语句不进行优化，参考内嵌汇编语言语法
    (
     "cpuid"
     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
     : "0"(0)
    );

    snprintf(prod, 5, "%s", (char*)&ebx);
    snprintf(prod+4, 5, "%s", (char*)&edx);
    snprintf(prod+8, 5, "%s", (char*)&ecx);

  //  qDebug((char*)prod);
}

void ComputerInfo::GetCpuSerialNumber(char *id)
{
    /********网上说当eax为1时执行命令返回的是序列号中的高两个16位，eax为3时返回ecx和edx按低位到高位的顺序存储的4个16位*******/
    /********但是程序执行eax为3时返回的都为空********/

    unsigned long s1,s2,s3,s4;

    unsigned int eax = 0;
    unsigned int ebx, ecx, edx;

    //两个%%表示在出现%0或%1的占位符时，又有寄存器%eax时，前面就要加一个%区分
    //第二句对edx寄存器赋0.优势是编码短速度快
    asm volatile
    (
     "movl $0x01, %%eax; \n\t"
     "xorl %%edx, %%edx; \n\t"
     "cpuid; \n\t"
     "movl %%edx, %0; \n\t"
     "movl %%eax, %1; \n\t"
     :"=m"(s1),"=m"(s2)
    );
    sprintf((char*)id, "%08X%08X", s1, s2);
  // qDebug((char*)id);


    asm volatile
    (
    "movl $0x03,%%eax ;\n\t"
    "xorl %%ecx,%%ecx ;\n\t"
    "xorl %%edx,%%edx ;\n\t"
    "cpuid ;\n\t"
    "movl %%edx,%0 ;\n\t"
    "movl %%ecx,%1 ;\n\t"
    :"=m"(s3),"=m"(s4)
    );
   // sprintf((char *)(id+16), "%08X-%08X\n", s3, s4);
   // qDebug((char*)id);
}

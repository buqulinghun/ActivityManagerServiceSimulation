#include "s57.h"

#include <stdio.h>

std::string decode_binary_string(unsigned char *bstr, int bcount)
{
    std::string str;
    char buf[2];
    for (int i = 0; i < MINIMUM(bcount, 24); i++) {
        sprintf(buf, "%02X", bstr[i]);
        str += buf;
    }
    return str;
}

void decode_lnam_string(unsigned char *bstr, int bcount, int *agen_find_fids)
{
    char buf[4];
    for (int i = 0; i < MINIMUM(bcount, 24); i++) {
        sprintf(buf, "%02X", bstr[i]);
    }

    agen_find_fids[0] = bstr[0] + (bstr[1] * 256);
    agen_find_fids[1] = bstr[2] + (bstr[3] * 256) +
                        (bstr[4] * 65536) + (bstr[5] * 16777216);
    agen_find_fids[2] = bstr[6] + (bstr[7] * 256);
    
}

void decode_name_string(unsigned char *bstr, int bcount, int *rcnm_rcid)
{
    char buf[4];
    for (int i = 0; i < MINIMUM(bcount, 24); i++) {
        sprintf(buf, "%02X", bstr[i]);
    }

    rcnm_rcid[0] = bstr[0];
    rcnm_rcid[1] = bstr[1] + (bstr[2] * 256) +
        (bstr[3]*65536) + (bstr[4] * 16777216);
}

int hex_to_dec(std::string str)
{
    const char *hexStg = str.c_str();
    int n = 0;         // position in string
    int m = 0;         // position in digit[] to shift
    int count;         // loop index
    int intValue = 0;  // integer value of hex string
    int digit[5];      // hold values to convert
    while (n < 4) {
     if (hexStg[n]=='\0')
        break;
     if (hexStg[n] > 0x29 && hexStg[n] < 0x40 ) //if 0 to 9
        digit[n] = hexStg[n] & 0x0f;            //convert to int
     else if (hexStg[n] >='a' && hexStg[n] <= 'f') //if a to f
        digit[n] = (hexStg[n] & 0x0f) + 9;      //convert to int
     else if (hexStg[n] >='A' && hexStg[n] <= 'F') //if A to F
        digit[n] = (hexStg[n] & 0x0f) + 9;      //convert to int
     else break;
    n++;
    }
    count = n;
    m = n - 1;
    n = 0;
    while(n < count) {
     // digit[n] is value of hex digit at position n
     // (m << 2) is the number of positions to shift
     // OR the bits into return value
     intValue = intValue | (digit[n] << (m << 2));
     m--;   // adjust the position to set
     n++;   // next digit to process
    }
    return (intValue);
}

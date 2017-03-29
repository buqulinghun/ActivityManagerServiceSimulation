#ifndef LONGLAT_H_
#define LONGLAT_H_

#include <iostream>
#include <string>

class LongLat
{ 
    public:
        LongLat();
        LongLat(double min);
        LongLat(double deg, double min);
        ~LongLat();
        std::string string() const;
        std::pair<std::string, std::string> ToStringPair() const;

        double operator+() { return minutes; }
        double operator-() { return -minutes; }
        LongLat operator+(const LongLat &rhs);
        LongLat operator-(const LongLat &rhs);
        LongLat &operator+=(const LongLat &rhs);
        LongLat &operator-=(const LongLat &rhs);
        bool operator>(const LongLat &rhs);
        bool operator<(const LongLat &rhs);
        bool operator>=(const LongLat &rhs);
        bool operator<=(const LongLat &rhs);

        LongLat operator+(double min);
        LongLat operator-(double min);
        LongLat operator*(double min);
        LongLat operator/(double min);
        LongLat &operator+=(double min);
        LongLat &operator-=(double min);
        LongLat &operator/=(double min);
        bool operator>(double min);
        bool operator<(double min);

        double minutes;
};

struct sg2d_t
{
    LongLat long_lat[2];
};

#endif

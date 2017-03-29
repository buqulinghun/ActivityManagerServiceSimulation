#include <sstream>
#include <iostream>
#include <iomanip>
#include <math.h>

#include "longlat.h"

LongLat::LongLat() :
    minutes(0)
{
}

LongLat::LongLat(double min)
{
    minutes = min;
}

LongLat::LongLat(double deg, double min)
{
    minutes = min + 60 * deg;
}

LongLat::~LongLat()
{
}

std::string LongLat::string() const
{
    int degrees = 0;
    double tmp_min = minutes;
    double minute_flop;

    if ((tmp_min >= 60) || (tmp_min <= 60)) {
        degrees += int(tmp_min/60);
        minute_flop = tmp_min - int(tmp_min);
        tmp_min = (int)tmp_min % 60;
        tmp_min += minute_flop;
    }

    // do something about degree overflow

    std::ostringstream strstr;

    if ((tmp_min < 0) || (degrees < 0)) {
        if (tmp_min == 0) {
            strstr << "- " << std::setfill('0') << std::setw(3) << -degrees
                   << " " << std::setiosflags(std::ios_base::left |
                                              std::ios_base::showpoint)
                   << std::setw(9) << tmp_min;
        } else {
            strstr << "- " << std::setfill('0') << std::setw(3) << -degrees
                   << " " << std::setiosflags(std::ios_base::left |
                                              std::ios_base::showpoint)
                   << std::setw(9) << -tmp_min;
        }
    } else {
        strstr << "+ " << std::setfill('0') << std::setw(3) << degrees
               << " "
               << std::setiosflags(std::ios_base::left |
                                   std::ios_base::showpoint)
               << std::setw(9) << tmp_min;
    }
    return strstr.str();
}

std::pair<std::string, std::string> LongLat::ToStringPair() const
{
    int degrees = 0;
    double tmp_min = minutes;
    double minute_flop;

    if ((tmp_min >= 60) || (tmp_min <= 60)) {
        degrees += int(tmp_min/60);
        minute_flop = tmp_min - int(tmp_min);
        tmp_min = (int)tmp_min % 60;
        tmp_min += minute_flop;
    }

    // do something about degrees overflow

    std::ostringstream degstrstr;
    std::ostringstream minstrstr;

    degstrstr << degrees;
    minstrstr << tmp_min;

    return std::make_pair(degstrstr.str(), minstrstr.str());
}

// operators...

LongLat LongLat::operator+(const LongLat &rhs)
{
    LongLat tmp;
    tmp.minutes = this->minutes + rhs.minutes;
    return tmp;
}

LongLat LongLat::operator-(const LongLat &rhs)
{
    LongLat tmp;
    tmp.minutes = this->minutes - rhs.minutes;
    return tmp;
}


LongLat &LongLat::operator+=(const LongLat &rhs)
{
    this->minutes += rhs.minutes;
    return *this;
}

LongLat &LongLat::operator-=(const LongLat &rhs)
{
    this->minutes -= rhs.minutes;
    return *this;
}

bool LongLat::operator>(const LongLat &rhs)
{
    return (this->minutes > rhs.minutes);
}

bool LongLat::operator<(const LongLat &rhs)
{
    return (this->minutes < rhs.minutes);
}

bool LongLat::operator>=(const LongLat &rhs)
{
    return (this->minutes >= rhs.minutes);
}

bool LongLat::operator<=(const LongLat &rhs)
{
    return (this->minutes <= rhs.minutes);
}

bool LongLat::operator>(double min)
{
    return (minutes > min);
}

bool LongLat::operator<(double min)
{
    return (minutes < min);
}

LongLat LongLat::operator+(double min)
{
    LongLat tmp;
    tmp.minutes = this->minutes + min;
    return tmp;
}

LongLat LongLat::operator-(double min)
{
    LongLat tmp;
    tmp.minutes = this->minutes - min;
    return tmp;
}

LongLat LongLat::operator*(double min)
{
    LongLat tmp;
    tmp.minutes = this->minutes * min;
    return tmp;
}

LongLat LongLat::operator/(double min)
{
    LongLat tmp;
    tmp.minutes = this->minutes / min;
    return tmp;
}

LongLat &LongLat::operator/=(double min)
{
    this->minutes /= min;
    return *this;
}

LongLat &LongLat::operator+=(double min)
{
    this->minutes += min;
    return *this;
}

LongLat &LongLat::operator-=(double min)
{
    this->minutes -= min;
    return *this;
}

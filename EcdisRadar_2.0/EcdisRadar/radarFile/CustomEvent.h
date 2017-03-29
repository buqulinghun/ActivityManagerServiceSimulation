#ifndef CUSTOMEVENT_H
#define CUSTOMEVENT_H


#include <QtCore/QVariant>
#include <QtCore/QEvent>

class CustomEvent : public QEvent
{
public:
    CustomEvent (Type myType, const QVariant& v)
        : QEvent(myType), variant(v)
    {}

    QVariant	variant;
};



#endif // CUSTOMEVENT_H

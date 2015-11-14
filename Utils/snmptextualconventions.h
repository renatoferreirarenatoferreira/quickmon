#ifndef SNMPTEXTUALCONVENTIONS_H
#define SNMPTEXTUALCONVENTIONS_H

#include <QString>
#include <QVariant>

class SNMPTextualConventions
{
public:
    SNMPTextualConventions();
    ~SNMPTextualConventions();
    static QString parseString(QString valueType, QVariant variantValue);

private:
    static QString readDateAndTime(QString hexString);
    static QString readF5IPAddress(QString hexString);
};

#endif // SNMPTEXTUALCONVENTIONS_H

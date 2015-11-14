#include "snmptextualconventions.h"

SNMPTextualConventions::SNMPTextualConventions()
{

}

SNMPTextualConventions::~SNMPTextualConventions()
{

}

QString SNMPTextualConventions::parseString(QString valueType, QVariant variantValue)
{
    if (valueType == "Hex")
        return variantValue.toMap().value("hexValue").toString();
    else if (valueType == "DateAndTime")
        return readDateAndTime(variantValue.toMap().value("hexValue").toString());
    else if (valueType == "F5IPAddress")
        return readF5IPAddress(variantValue.toMap().value("hexValue").toString());
    else
        return variantValue.toMap().value("stringValue").toString();
}

QString SNMPTextualConventions::readDateAndTime(QString hexString)
{
    QString returnString;
    QString substring;
    bool ok;

    //read year
    substring = hexString.mid(0,2) + hexString.mid(3,2);
    returnString.append(QString::number(substring.toUInt(&ok, 16)));
    returnString.append("-");

    //read month
    substring = hexString.mid(6, 2);
    returnString.append(QString::number(substring.toUInt(&ok, 16)));
    returnString.append("-");

    //read day
    substring = hexString.mid(9, 2);
    returnString.append(QString::number(substring.toUInt(&ok, 16)));
    returnString.append(",");

    //read hour
    substring = hexString.mid(12, 2);
    returnString.append(QString::number(substring.toUInt(&ok, 16)));
    returnString.append(":");

    //read minutes
    substring = hexString.mid(15, 2);
    returnString.append(QString::number(substring.toUInt(&ok, 16)));
    returnString.append(":");

    //read seconds
    substring = hexString.mid(18, 2);
    returnString.append(QString::number(substring.toUInt(&ok, 16)));
    returnString.append(".");

    //read deci-seconds
    substring = hexString.mid(21, 2);
    returnString.append(QString::number(substring.toUInt(&ok, 16)));

    return returnString;
}

QString SNMPTextualConventions::readF5IPAddress(QString hexString)
{
    QString returnString;

    if (hexString.size() == 11)
    {
        //ipv4
        QString substring;
        bool ok;

        substring = hexString.mid(0, 2);
        returnString.append(QString::number(substring.toUInt(&ok, 16)));
        returnString.append(".");

        substring = hexString.mid(3, 2);
        returnString.append(QString::number(substring.toUInt(&ok, 16)));
        returnString.append(".");

        substring = hexString.mid(6, 2);
        returnString.append(QString::number(substring.toUInt(&ok, 16)));
        returnString.append(".");

        substring = hexString.mid(9, 2);
        returnString.append(QString::number(substring.toUInt(&ok, 16)));
    } else {
        //ipv6
        for (int i=0; i < hexString.size(); i+=6)
        {
            returnString.append(hexString.mid(i, 2));
            returnString.append(hexString.mid(i+3, 2));
            if (i < hexString.size()-6)
                returnString.append(":");
        }
    }

    return returnString;
}

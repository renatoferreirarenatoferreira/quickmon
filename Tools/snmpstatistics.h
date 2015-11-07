#ifndef SNMPSTATISTICS_H
#define SNMPSTATISTICS_H

#include <QLinkedList>
#include <QDateTime>
#include <QVariant>

#include "../Net/snmpclient.h"

#define SAMPLE_INTERVAL_SECONDS 100

#define TYPE_COUNTER 0
#define TYPE_COUNTERPERSECOND 1
#define TYPE_UINT 2
#define TYPE_INT 3

#define CALCULATION_NONE 0
#define CALCULATION_ADDITION 1
#define CALCULATION_SUBTRACTION 2
#define CALCULATION_MULTIPLICATION 3
#define CALCULATION_DIVISION 4

struct SNMPNumberInformation
{
    quint64 time;
    quint64 clientTime;
    bool response;
    bool validValue;
    double calculatedValue;
    //used for uint, counter and counter64
    quint64 unsignedNumber;
    //used for int32
    int signedNumber;
};

class SNMPStatistics
{
public:
    SNMPStatistics(QString Type, QString Calculate);
    SNMPStatistics(int sampleIntervalSeconds, QString Type, QString Calculate);
    ~SNMPStatistics();
    void reset();
    void updateStatistics(bool response, SNMPVariable value);
    void addResponse(SNMPVariable value);
    void addTimeout();
    double lastValue();
    double averageValue();
    double maxValue();
    double minValue();
    bool isValid();

private:
    void init(int sampleIntervalSeconds, QString Type, QString Calculate);
    int type;
    int calculate;
    double calculateBy;
    int sampleIntervalSeconds;
    QLinkedList<SNMPNumberInformation> lastValues;
    double _lastValue;
    double _averageValue;
    double _maxValue;
    double _minValue;
    double _isValue;
    SNMPVariable emptySNMPVariable;
    double notANumber = std::numeric_limits<double>::quiet_NaN();
};

#endif // SNMPSTATISTICS_H

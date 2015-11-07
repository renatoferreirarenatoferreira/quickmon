#include "snmpstatistics.h"

SNMPStatistics::SNMPStatistics(QString Type, QString Calculate)
{
    this->init(SAMPLE_INTERVAL_SECONDS, Type, Calculate);
}

SNMPStatistics::SNMPStatistics(int sampleIntervalSeconds, QString Type, QString Calculate)
{
    this->init(sampleIntervalSeconds, Type, Calculate);
}

SNMPStatistics::~SNMPStatistics()
{

}

void SNMPStatistics::init(int sampleIntervalSeconds, QString Type, QString Calculate)
{
    this->sampleIntervalSeconds = sampleIntervalSeconds;

    //read type
    if ( Type == "Counter")
        this->type = TYPE_COUNTER;
    else if ( Type == "CounterPerSecond")
        this->type = TYPE_COUNTERPERSECOND;
    else if ( Type == "Uint")
        this->type = TYPE_UINT;
    else if ( Type == "Int")
        this->type = TYPE_INT;
    else
        this->type = TYPE_INT;

    //read calculation
    if ( Calculate.size() > 2 )
    {
        if ( Calculate.startsWith("+") )
            this->calculate = CALCULATION_ADDITION;
        else if ( Calculate.startsWith("-") )
            this->calculate = CALCULATION_SUBTRACTION;
        else if ( Calculate.startsWith("*") )
            this->calculate = CALCULATION_MULTIPLICATION;
        else if ( Calculate.startsWith("/") )
            this->calculate = CALCULATION_DIVISION;
        else
            this->calculate = CALCULATION_NONE;

        if ( this->calculate != CALCULATION_NONE )
        {
            bool ok;
            this->calculateBy = Calculate.mid(1).toDouble(&ok);
            if (!ok)
                this->calculate = CALCULATION_NONE;
        }
    } else
        this->calculate = CALCULATION_NONE;

    this->reset();
}

void SNMPStatistics::reset()
{
    this->lastValues.clear();
    this->_averageValue = 0;
    this->_maxValue = 0;
    this->_minValue = 0;
}

void SNMPStatistics::updateStatistics(bool response, SNMPVariable value)
{
    quint64 currTime = QDateTime::currentMSecsSinceEpoch();
    quint64 startTime = currTime-(this->sampleIntervalSeconds*1000);

    //remove data oldest than the maximum sample interval
    SNMPNumberInformation oldest;
    while(this->lastValues.count() > 0)
    {
        oldest = this->lastValues.last();
        if (oldest.time < startTime)
            this->lastValues.removeLast();
        else
            break;
    }

    //add new data
    SNMPNumberInformation newest;
    newest.time = currTime;
    newest.response = response;
    newest.validValue = false;
    //calculate counters
    if (response)
    {
        //used for uint, counter and counter64
        if (type == TYPE_INT)
        {
            newest.signedNumber = value.variantValue.toInt();
            newest.calculatedValue = newest.signedNumber;
        } else //used for int32
        {
            newest.unsignedNumber = value.variantValue.toULongLong();
            newest.calculatedValue = newest.unsignedNumber;
        }

        if (type == TYPE_COUNTER || type == TYPE_COUNTERPERSECOND) //counters (unsigned)
        {
            //copy packet time for more precise calculations
            newest.clientTime = value.timestamp;

            //calculate only if this response is valid
            if (this->lastValues.count() > 0)
            {
                SNMPNumberInformation last = this->lastValues.first();

                //calculate only if last response is valid and the counter is incrementing
                if (last.response)
                {
                    newest.calculatedValue = newest.unsignedNumber - last.unsignedNumber;

                    //calculate delta per seconds
                    if (type == TYPE_COUNTERPERSECOND && newest.calculatedValue > 0)
                        newest.calculatedValue = (double) newest.calculatedValue / ((double)(newest.clientTime - last.clientTime) / 1000);

                    newest.validValue = true;
                }
            }
        } else
            newest.validValue = true;

        //proceed on final calculation when configured
        switch (this->calculate)
        {
            case CALCULATION_ADDITION:
                newest.calculatedValue += this->calculateBy;
                break;

            case CALCULATION_SUBTRACTION:
                newest.calculatedValue -= this->calculateBy;
                break;

            case CALCULATION_MULTIPLICATION:
                newest.calculatedValue *= this->calculateBy;
                break;

            case CALCULATION_DIVISION:
                newest.calculatedValue /= this->calculateBy;
                break;
        }
    }
    //add to list
    this->_isValue = newest.validValue;
    this->lastValues.prepend(newest);

    //calculate totals
    if (response)
    {
        this->_lastValue = newest.calculatedValue;

        //calculate min/max (the hard way)
        double tempAvg = 0;
        double tempMax = DBL_MIN;
        double tempMin = DBL_MAX;
        int validValues = 0;
        QLinkedListIterator<SNMPNumberInformation> iterator(this->lastValues);
        while (iterator.hasNext())
        {
            SNMPNumberInformation nextNumber = iterator.next();

            if (nextNumber.validValue)
            {
                validValues++;
                tempAvg += nextNumber.calculatedValue;

                if (nextNumber.calculatedValue > tempMax)
                    tempMax = nextNumber.calculatedValue;

                if (nextNumber.calculatedValue < tempMin)
                    tempMin = nextNumber.calculatedValue;
            }
        }

        if (validValues > 0)
        {
            this->_averageValue = (double) tempAvg/validValues;
            this->_maxValue = tempMax;
            this->_minValue = tempMin;
        }
    }
}

void SNMPStatistics::addResponse(SNMPVariable value)
{
    this->updateStatistics(true, value);
}

void SNMPStatistics::addTimeout()
{
    this->updateStatistics(false, this->emptySNMPVariable);
}

double SNMPStatistics::lastValue()
{
    return this->_lastValue;
}

double SNMPStatistics::averageValue()
{
    return this->_averageValue;
}

double SNMPStatistics::maxValue()
{
    return this->_maxValue;
}

double SNMPStatistics::minValue()
{
    return this->_minValue;
}

bool SNMPStatistics::isValid()
{
    return this->_isValue;
}

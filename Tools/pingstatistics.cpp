#include "pingstatistics.h"

PingStatistics::PingStatistics(int sampleIntervalSeconds)
{
    this->sampleIntervalSeconds = sampleIntervalSeconds;
    this->reset();
}

PingStatistics::PingStatistics()
{
    this->sampleIntervalSeconds = SAMPLE_INTERVAL_SECONDS;
    this->reset();
}

PingStatistics::~PingStatistics()
{

}

void PingStatistics::reset()
{
    lastValues.clear();
    this->totalResponses = 0;
    this->totalTimeouts = 0;
    this->totalCalculatedJitters = 0;
    this->totalLatencyMillisseconds = 0;
    this->totalJitterMillisseconds = 0;
    this->_lastLatency = 0;
    this->_averageLatency = 0;
    this->_averageJitter = 0;
    this->_percentLoss = 0;
    this->_percentJitter = 0;
}

void PingStatistics::updateStatistics(int response, int timeout, double latency)
{
    quint64 currTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    quint64 startTime = currTime-(this->sampleIntervalSeconds*1000);

    //remove data oldest than the maximum sample interval
    LatencyInformation oldest;
    while(lastValues.count() > 0)
    {
        oldest = lastValues.last();
        if (oldest.time < startTime)
        {
            this->totalResponses -= oldest.responseCount;
            this->totalTimeouts -= oldest.timeoutCount;
            this->totalCalculatedJitters -= oldest.jitterCalculatedCount;
            this->totalLatencyMillisseconds -= oldest.latencyMillisseconds;
            this->totalJitterMillisseconds -= oldest.jitterMillisseconds;
            lastValues.removeLast();
        } else
            break;
    }

    //add new data
    LatencyInformation newest;
    newest.time = currTime;
    newest.latencyMillisseconds = latency;
    //calculate jitter
    newest.jitterMillisseconds = 0;
    newest.jitterCalculatedCount = 0;
    if (response == 1 && lastValues.count() > 0)
    {
        LatencyInformation last = lastValues.first();
        if (last.responseCount == 1)
        {
            newest.jitterMillisseconds = fabs(last.latencyMillisseconds - latency);
            newest.jitterCalculatedCount = 1;
        }
    }
    newest.responseCount = response;
    newest.timeoutCount = timeout;
    lastValues.prepend(newest);

    //calculate totals
    this->totalResponses += response;
    this->totalTimeouts += timeout;
    this->totalCalculatedJitters += newest.jitterCalculatedCount;
    this->totalLatencyMillisseconds += latency;
    this->totalJitterMillisseconds += newest.jitterMillisseconds;
    this->_lastLatency = latency;
    this->_averageLatency = this->totalLatencyMillisseconds / this->totalResponses;
    this->_percentLoss = (float)(this->totalTimeouts*100)/(this->totalResponses+this->totalTimeouts);
    if (this->totalCalculatedJitters > 0)
    {
        this->_averageJitter = this->totalJitterMillisseconds / this->totalCalculatedJitters;
        this->_percentJitter = (float)(this->_averageJitter*100)/this->_averageLatency;
    }

}

void PingStatistics::addResponse(double latency)
{
    this->updateStatistics(1, 0, latency);
}

void PingStatistics::addTimeout()
{
    this->updateStatistics(0, 1, 0);
}

double PingStatistics::lastLatency()
{
    return this->_lastLatency;
}

double PingStatistics::averageLatency()
{
    return this->_averageLatency;
}

double PingStatistics::averageJitter()
{
    return this->_averageJitter;
}

float PingStatistics::percentLoss()
{
    return this->_percentLoss;
}

float PingStatistics::percentJitter()
{
    return this->_percentJitter;
}

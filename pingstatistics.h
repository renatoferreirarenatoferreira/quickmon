#ifndef PINGSTATISTICS_H
#define PINGSTATISTICS_H

#include <QLinkedList>
#include <QDateTime>

#define SAMPLE_INTERVAL_SECONDS 100

struct LatencyInformation
{
    quint64 time;
    double latencyMillisseconds;
    double jitterMillisseconds;
    short responseCount;
    short timeoutCount;
    short jitterCalculatedCount;
};

class PingStatistics
{
public:
    PingStatistics();
    PingStatistics(int sampleIntervalSeconds);
    ~PingStatistics();
    void reset();
    void addResponse(double latency);
    void addTimeout();
    double lastLatency();
    double averageLatency();
    double averageJitter();
    float percentLoss();
    float percentJitter();

private:
    int sampleIntervalSeconds;
    QLinkedList<LatencyInformation> lastValues;
    short totalResponses;
    short totalTimeouts;
    short totalCalculatedJitters;
    double totalLatencyMillisseconds;
    double totalJitterMillisseconds;
    double _lastLatency;
    double _averageLatency;
    double _averageJitter;
    float _percentLoss;
    float _percentJitter;
    void updateStatistics(int response, int timeout, double latency);
};

#endif // PINGSTATISTICS_H

#ifndef PINGER_H
#define PINGER_H

#include <QMutex>
#include <QThread>
#include <QHostAddress>
#include <QThreadPool>
#include <QMutex>

#include <Ws2tcpip.h>
#include <Iphlpapi.h>
#include <icmpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#define PINGER_PING_SIZE 32
#define PINGER_PING_TIMEOUT 3000
#define PINGER_PING_TTL 255

#define PINGER_STATUS_SUCCESS 0
#define PINGER_STATUS_ERROR 1
#define PINGER_STATUS_TIMEOUT 2
#define PINGER_STATUS_EXPIREDINTRANSIT 3

struct PingContext {
    HANDLE* hICMPv4File;
    HANDLE* hICMPv6File;
    struct sockaddr_in6* sourceAddressIPv6;

    QHostAddress requestAddress;
    PVOID listener;
    IPAddr addressIPv4;
    sockaddr_in6 addressIPv6;
    LARGE_INTEGER startTime;
    LARGE_INTEGER endTime;
    char* sendData;
    char* replyBuffer;
    IP_OPTION_INFORMATION options;
    int timeout;
    QMutex* mutex;

    QAbstractSocket::NetworkLayerProtocol protocol;
    int replyStatus;
    QHostAddress replyAddress;
    long millisLatencyLowResolution;
    double millisLatencyHighResolution;
};

class IPingReplyListener
{
    public:
        virtual void receivePingReply(PingContext* context) = 0;
};

class Pinger : QObject
{
public:
    static Pinger* Instance();
    bool supportIPv4();
    bool supportIPv6();
    PingContext* ping(QHostAddress address, IPingReplyListener* listener);
    PingContext* ping(QHostAddress address, IPingReplyListener* listener, int ttl);
    PingContext* ping(QHostAddress address, IPingReplyListener* listener, int ttl, int timeout);
    void receiveReply(PingContext* contextStruct);
    static void stopListening(PingContext* contextStruct);

private:
    static Pinger* m_Instance;
    Pinger();
    ~Pinger();
    HANDLE hICMPv4File;
    HANDLE hICMPv6File;
    QHash<QString, int> pendingPings;
    struct sockaddr_in6 sourceAddressIPv6;
    LARGE_INTEGER performanceFrequency;
    QThreadPool* workerPool;
};

class PingerWorker : public QRunnable
{
public:
    PingerWorker(PingContext* contextStruct);
    void run();

private:
    PingContext* contextStruct;
};

#endif // PINGER_H

#ifndef PINGER_H
#define PINGER_H

#include <QMutex>
#include <QThread>
#include <QHostAddress>

#ifdef QT_DEBUG
#include <QDebug>
#endif

#include <Ws2tcpip.h>
#include <Iphlpapi.h>
#include <icmpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#define PINGER_PING_SIZE 32
#define PINGER_PING_TIMEOUT 1000
#define PINGER_PING_TTL 255

#define PINGER_STATUS_SUCCESS 0
#define PINGER_STATUS_ERROR 1
#define PINGER_STATUS_EXPIREDINTRANSIT 2

struct PingContext {
    PVOID listener;
    IPAddr addressIPv4;
    sockaddr_in6 addressIPv6;
    LARGE_INTEGER startTime;
    LARGE_INTEGER endTime;
    char* sendData;
    char* replyBuffer;

    QAbstractSocket::NetworkLayerProtocol protocol;
    int sequence;
    int replyStatus;
    QHostAddress replyAddress;
    unsigned long millisLatencyLowResolution;
    double millisLatencyHighResolution;
};

class IPingReplyListener
{
    public:
        virtual void receivePingReply(PingContext* context) = 0;
};

class Pinger
{
public:
    static Pinger* Instance();
    bool supportIPv4();
    bool supportIPv6();
    bool ping(QHostAddress address, IPingReplyListener* listener);
    bool ping(QHostAddress address, IPingReplyListener* listener, int sequence, int ttl, int timeout);
    void receiveReply(PingContext* contextStruct);

    bool PingTest(char* hostaddress);

private:
    static Pinger* m_Instance;
    Pinger();
    ~Pinger();
    HANDLE hICMPv4File;
    HANDLE hICMPv6File;
    QHash<QString, int> pendingPings;
    struct sockaddr_in6 sourceAddressIPv6;
    LARGE_INTEGER performanceFrequency;
};

void NTAPI receiveProxy(PVOID param);

#endif // PINGER_H

#include "pinger.h"

Pinger* Pinger::m_Instance = 0;

Pinger::Pinger()
{
    this->hICMPv4File = IcmpCreateFile();
    this->hICMPv6File = Icmp6CreateFile();

    QueryPerformanceFrequency(&this->performanceFrequency);

    this->sourceAddressIPv6.sin6_addr = in6addr_any;
    this->sourceAddressIPv6.sin6_family = AF_INET6;
    this->sourceAddressIPv6.sin6_flowinfo = 0;
    this->sourceAddressIPv6.sin6_port = 0;
}

Pinger::~Pinger()
{
    if (this->supportIPv4())
        IcmpCloseHandle(this->hICMPv4File);

    if (this->supportIPv6())
        IcmpCloseHandle(this->hICMPv6File);
}

Pinger* Pinger::Instance()
{
    static QMutex mutex;
    if (!m_Instance)
    {
        mutex.lock();

        if (!m_Instance)
        {
            m_Instance = new Pinger;
        }

        mutex.unlock();
    }

    return m_Instance;
}

bool Pinger::supportIPv4()
{
    return !(this->hICMPv4File == INVALID_HANDLE_VALUE);
}

bool Pinger::supportIPv6()
{
    return !(this->hICMPv6File == INVALID_HANDLE_VALUE);
}

bool Pinger::ping(QHostAddress address, IPingReplyListener* listener)
{
    return this->ping(address, listener, 0, PINGER_PING_TTL, PINGER_PING_TIMEOUT);
}

bool Pinger::ping(QHostAddress address, IPingReplyListener* listener, int sequence, int ttl, int timeout)
{
    //start data structure
    PingContext* contextStruct = new PingContext();
    contextStruct->listener = (PVOID)listener;
    contextStruct->sequence = sequence;
    contextStruct->protocol = address.protocol();

    //set buffers
    contextStruct->sendData = (char*) malloc(PINGER_PING_SIZE);
    if (contextStruct->protocol == QAbstractSocket::IPv4Protocol)
        contextStruct->replyBuffer = (char*) malloc(sizeof(ICMP_ECHO_REPLY)+PINGER_PING_SIZE);
    else if (contextStruct->protocol == QAbstractSocket::IPv6Protocol)
        contextStruct->replyBuffer = (char*) malloc(sizeof(ICMPV6_ECHO_REPLY)+PINGER_PING_SIZE);
    else
        return false;

    //fill sendData with letters
    int c=0;
    for(int i=97; i<=122; i++)
    {
        if (c >= PINGER_PING_SIZE)
            break;

        contextStruct->sendData[c] = i;

        c++;
        if (i == 122)
            i=97;
    }

    //set destination address
    QString addressString = address.toString();
    wchar_t* wCharAddress = (wchar_t*) malloc (sizeof(wchar_t)*(addressString.length()+1));
    wCharAddress[addressString.length()] = NULL;
    addressString.toWCharArray(wCharAddress);
    if (contextStruct->protocol == QAbstractSocket::IPv4Protocol)
    {
        memset(&contextStruct->addressIPv4, 0, sizeof(contextStruct->addressIPv4));
        InetPton(AF_INET, wCharAddress, &contextStruct->addressIPv4);
    } else if (contextStruct->protocol == QAbstractSocket::IPv6Protocol)
    {
        memset(&contextStruct->addressIPv6, 0, sizeof(contextStruct->addressIPv6));
        contextStruct->addressIPv6.sin6_family = AF_INET6;
        InetPton(AF_INET6, wCharAddress, &contextStruct->addressIPv6.sin6_addr);
    }
    free(wCharAddress);

    //set ttl
    IP_OPTION_INFORMATION options;
    memset((void *) &options, 0, sizeof(IP_OPTION_INFORMATION));
    options.Ttl = ttl;

    DWORD dwRetVal;
    if (contextStruct->protocol == QAbstractSocket::IPv4Protocol)
    {
        QueryPerformanceCounter(&contextStruct->startTime);
        dwRetVal = IcmpSendEcho2(this->hICMPv4File,
                                 NULL,
                                 (FARPROC)receiveProxy,
                                 contextStruct,
                                 (IPAddr)contextStruct->addressIPv4,
                                 contextStruct->sendData,
                                 PINGER_PING_SIZE,
                                 &options,
                                 contextStruct->replyBuffer,
                                 sizeof(ICMP_ECHO_REPLY)+PINGER_PING_SIZE,
                                 timeout);
    }
    else if (contextStruct->protocol == QAbstractSocket::IPv6Protocol)
    {
        QueryPerformanceCounter(&contextStruct->startTime);
        dwRetVal = Icmp6SendEcho2(this->hICMPv6File,
                                  NULL,
                                  (FARPROC)receiveProxy,
                                  contextStruct,
                                  &this->sourceAddressIPv6,
                                  &contextStruct->addressIPv6,
                                  contextStruct->sendData,
                                  PINGER_PING_SIZE,
                                  &options,
                                  contextStruct->replyBuffer,
                                  sizeof(ICMPV6_ECHO_REPLY)+PINGER_PING_SIZE,
                                  timeout);
    }

    return true;
}

void Pinger::receiveReply(PingContext* contextStruct)
{
     if (contextStruct->protocol == QAbstractSocket::IPv4Protocol)
    {
        //get reply data
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY) contextStruct->replyBuffer;
        //test status
        if (pEchoReply->Status == IP_SUCCESS)
        {
            //copy status
            contextStruct->replyStatus = PINGER_STATUS_SUCCESS;
            //copy address
            char stringAddress[INET_ADDRSTRLEN];
            InetNtopA(AF_INET, &pEchoReply->Address, stringAddress, sizeof(stringAddress));
            contextStruct->replyAddress.setAddress(stringAddress);
            //set times
            contextStruct->millisLatencyLowResolution = pEchoReply->RoundTripTime;
            contextStruct->millisLatencyHighResolution = (double) (contextStruct->endTime.QuadPart - contextStruct->startTime.QuadPart) * 1000.0 / (double) this->performanceFrequency.QuadPart;
        }
        else if (pEchoReply->Status == IP_TTL_EXPIRED_TRANSIT)
            contextStruct->replyStatus = PINGER_STATUS_EXPIREDINTRANSIT;
        else
            contextStruct->replyStatus = PINGER_STATUS_ERROR;
    } else if (contextStruct->protocol == QAbstractSocket::IPv6Protocol)
    {
        //get reply data
        PICMPV6_ECHO_REPLY pEchoReply = (PICMPV6_ECHO_REPLY) contextStruct->replyBuffer;
        //test status
        if (pEchoReply->Status == IP_SUCCESS)
        {
            //copy status
            contextStruct->replyStatus = PINGER_STATUS_SUCCESS;
            //copy address
            char stringAddress[INET6_ADDRSTRLEN];
            InetNtopA(AF_INET6, &pEchoReply->Address.sin6_addr, stringAddress, sizeof(stringAddress));
            contextStruct->replyAddress.setAddress(stringAddress);
            //set times
            contextStruct->millisLatencyLowResolution = pEchoReply->RoundTripTime;
            contextStruct->millisLatencyHighResolution = (double) (contextStruct->endTime.QuadPart - contextStruct->startTime.QuadPart) * 1000.0 / (double) this->performanceFrequency.QuadPart;
        }
        else if (pEchoReply->Status == IP_TTL_EXPIRED_TRANSIT)
            contextStruct->replyStatus = PINGER_STATUS_EXPIREDINTRANSIT;
        else
            contextStruct->replyStatus = PINGER_STATUS_ERROR;
    }

     //callback listener
    ((IPingReplyListener*)contextStruct->listener)->receivePingReply(contextStruct);

    //free memory
    free(contextStruct->replyBuffer);
    free(contextStruct->sendData);
    delete contextStruct;
}

void NTAPI receiveProxy(PVOID contextStruct)
{
    QueryPerformanceCounter(&((PingContext*)contextStruct)->endTime);
    Pinger::Instance()->receiveReply((PingContext*)contextStruct);
}

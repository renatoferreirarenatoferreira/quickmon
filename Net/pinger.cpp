#include "pinger.h"

Pinger* Pinger::m_Instance = 0;

Pinger::Pinger()
{
    QueryPerformanceFrequency(&this->performanceFrequency);

    this->sourceAddressIPv6.sin6_addr = in6addr_any;
    this->sourceAddressIPv6.sin6_family = AF_INET6;
    this->sourceAddressIPv6.sin6_flowinfo = 0;
    this->sourceAddressIPv6.sin6_port = 0;

    this->workerPool = new QThreadPool(this);
    this->workerPool->setMaxThreadCount(10);
}

Pinger::~Pinger()
{

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

PingContext* Pinger::ping(QHostAddress address, IPingReplyListener* listener)
{
    return this->ping(address, listener, PINGER_PING_TTL, PINGER_PING_TIMEOUT);
}

PingContext* Pinger::ping(QHostAddress address, IPingReplyListener* listener, int ttl)
{
    return this->ping(address, listener, ttl, PINGER_PING_TIMEOUT);
}

PingContext* Pinger::ping(QHostAddress address, IPingReplyListener* listener, int ttl, int timeout)
{
    //start data structure
    PingContext* contextStruct = new PingContext();
    contextStruct->requestAddress = address;
    contextStruct->listener = (PVOID)listener;
    contextStruct->timeout = timeout;
    contextStruct->protocol = address.protocol();
    contextStruct->mutex = new QMutex(QMutex::Recursive);
    //copy shared handles and references
    contextStruct->sourceAddressIPv6 = &this->sourceAddressIPv6;

    //set buffers
    contextStruct->sendData = (char*) malloc(PINGER_PING_SIZE);
    if (contextStruct->protocol == QAbstractSocket::IPv4Protocol)
        contextStruct->replyBuffer = (char*) malloc(sizeof(ICMP_ECHO_REPLY)+PINGER_PING_SIZE);
    else if (contextStruct->protocol == QAbstractSocket::IPv6Protocol)
        contextStruct->replyBuffer = (char*) malloc(sizeof(ICMPV6_ECHO_REPLY)+PINGER_PING_SIZE);
    else
    {
        delete contextStruct;
        return NULL;
    }

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
    memset((void *) &contextStruct->options, 0, sizeof(IP_OPTION_INFORMATION));
    contextStruct->options.Ttl = ttl;

    //start worker
    this->workerPool->start(new PingerWorker(contextStruct));

    return contextStruct;
}

void Pinger::receiveReply(PingContext* contextStruct)
{
    if (contextStruct->protocol == QAbstractSocket::IPv4Protocol)
    {
        //get reply data
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY) contextStruct->replyBuffer;
        //test status
        if (pEchoReply->Status == IP_SUCCESS || pEchoReply->Status == IP_TTL_EXPIRED_TRANSIT)
        {
            //copy status
            if (pEchoReply->Status == IP_SUCCESS)
                contextStruct->replyStatus = PINGER_STATUS_SUCCESS;
            else
                contextStruct->replyStatus = PINGER_STATUS_EXPIREDINTRANSIT;
            //copy address
            char stringAddress[INET_ADDRSTRLEN];
            InetNtopA(AF_INET, &pEchoReply->Address, stringAddress, sizeof(stringAddress));
            contextStruct->replyAddress.setAddress(stringAddress);
            //set times
            contextStruct->millisLatencyLowResolution = pEchoReply->RoundTripTime;
            contextStruct->millisLatencyHighResolution = (double) (contextStruct->endTime.QuadPart - contextStruct->startTime.QuadPart) * 1000.0 / (double) this->performanceFrequency.QuadPart;
        }
        else if (pEchoReply->Status == IP_REQ_TIMED_OUT)
            contextStruct->replyStatus = PINGER_STATUS_TIMEOUT;
        else
            contextStruct->replyStatus = PINGER_STATUS_ERROR;
    } else if (contextStruct->protocol == QAbstractSocket::IPv6Protocol)
    {
        //get reply data
        PICMPV6_ECHO_REPLY pEchoReply = (PICMPV6_ECHO_REPLY) contextStruct->replyBuffer;
        //test status
        if (pEchoReply->Status == IP_SUCCESS || pEchoReply->Status == IP_TTL_EXPIRED_TRANSIT)
        {
            //copy status
            if (pEchoReply->Status == IP_SUCCESS)
                contextStruct->replyStatus = PINGER_STATUS_SUCCESS;
            else
                contextStruct->replyStatus = PINGER_STATUS_EXPIREDINTRANSIT;
            //copy address
            char stringAddress[INET6_ADDRSTRLEN];
            InetNtopA(AF_INET6, &pEchoReply->Address.sin6_addr, stringAddress, sizeof(stringAddress));
            contextStruct->replyAddress.setAddress(stringAddress);
            //set times
            contextStruct->millisLatencyLowResolution = pEchoReply->RoundTripTime;
            contextStruct->millisLatencyHighResolution = (double) (contextStruct->endTime.QuadPart - contextStruct->startTime.QuadPart) * 1000.0 / (double) this->performanceFrequency.QuadPart;
        }
        else if (pEchoReply->Status == IP_REQ_TIMED_OUT)
            contextStruct->replyStatus = PINGER_STATUS_TIMEOUT;
        else
            contextStruct->replyStatus = PINGER_STATUS_ERROR;
    }

    //callback listener
    contextStruct->mutex->lock();
    if (contextStruct->listener != NULL)
        ((IPingReplyListener*)contextStruct->listener)->receivePingReply(contextStruct);
    contextStruct->listener = NULL;
    contextStruct->mutex->unlock();

    //free memory
    free(contextStruct->replyBuffer);
    free(contextStruct->sendData);
    delete contextStruct->mutex;
    delete contextStruct;
}

void Pinger::stopListening(PingContext* contextStruct)
{
    contextStruct->mutex->lock();
    if (contextStruct->listener != NULL)
        contextStruct->listener = NULL;
    contextStruct->mutex->unlock();
}

PingerWorker::PingerWorker(PingContext* contextStruct)
{
    this->contextStruct = contextStruct;
    this->hICMPv4File = NULL;
    this->hICMPv6File = NULL;
}

PingerWorker::~PingerWorker()
{
    if (this->hICMPv4File != NULL)
        IcmpCloseHandle(this->hICMPv4File);

    if (this->hICMPv6File != NULL)
        IcmpCloseHandle(this->hICMPv6File);
}

void PingerWorker::run()
{
    if (contextStruct->protocol == QAbstractSocket::IPv4Protocol)
    {
        if (this->hICMPv4File == NULL)
            this->hICMPv4File = IcmpCreateFile();

        QueryPerformanceCounter(&this->contextStruct->startTime);
        IcmpSendEcho2(this->hICMPv4File,
                      NULL,
                      NULL,
                      this->contextStruct,
                      (IPAddr)contextStruct->addressIPv4,
                      this->contextStruct->sendData,
                      PINGER_PING_SIZE,
                      &this->contextStruct->options,
                      this->contextStruct->replyBuffer,
                      sizeof(ICMP_ECHO_REPLY)+PINGER_PING_SIZE,
                      this->contextStruct->timeout);
        QueryPerformanceCounter(&this->contextStruct->endTime);
    }
    else if (contextStruct->protocol == QAbstractSocket::IPv6Protocol)
    {
        if (this->hICMPv6File == NULL)
            this->hICMPv6File = Icmp6CreateFile();

        QueryPerformanceCounter(&this->contextStruct->startTime);
        Icmp6SendEcho2(this->hICMPv6File,
                       NULL,
                       NULL,
                       this->contextStruct,
                       this->contextStruct->sourceAddressIPv6,
                       &this->contextStruct->addressIPv6,
                       this->contextStruct->sendData,
                       PINGER_PING_SIZE,
                       &this->contextStruct->options,
                       this->contextStruct->replyBuffer,
                       sizeof(ICMPV6_ECHO_REPLY)+PINGER_PING_SIZE,
                       this->contextStruct->timeout);
        QueryPerformanceCounter(&this->contextStruct->endTime);
    }

    Pinger::Instance()->receiveReply((PingContext*)contextStruct);
}

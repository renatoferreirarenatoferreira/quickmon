#include "tracerouteitemmanager.h"

TracerouteItemManager::TracerouteItemManager(QBoxLayout* container)
{
    this->container = container;

    //get a pinger instance
    this->pingerInstance = Pinger::Instance();
    //create the timer object
    this->asyncWorker = new QTimer(this);
    connect(this->asyncWorker, SIGNAL(timeout()), this, SLOT(asyncTask()));
    //initialize mutex
    this->mutex = new QMutex(QMutex::Recursive);
}

TracerouteItemManager::~TracerouteItemManager()
{
    this->clear();

    delete this->asyncWorker;
    delete this->mutex;
}

void TracerouteItemManager::addItem(int replyStatus, int ttl, QString address, double rtt)
{
    this->mutex->lock();

    //create reference data
    TracerouteItemReference* newItem = new TracerouteItemReference();
    newItem->address = address;
    newItem->validAddress = newItem->hostAddress.setAddress(address);

    //create widget
    newItem->widget = new TracerouteItem();
    newItem->widget->setTTL(ttl);
    if (replyStatus == PINGER_STATUS_SUCCESS ||
        replyStatus == PINGER_STATUS_EXPIREDINTRANSIT)
    {
        newItem->widget->setAddress(address);
        newItem->widget->setLastRTT(rtt);
    } else
        newItem->widget->setAddress(TRACEROUTE_TIMEOUT_ADDRESS);

    //add to container
    this->container->addWidget(newItem->widget);

    //starting pinging and reverse resolution if this item has a valid address
    if (newItem->validAddress)
    {
        //ping host
        newItem->waitingForReply = true;
        newItem->pigingContext = this->pingerInstance->ping(newItem->hostAddress, this);
        //prepare statistics calculator
        newItem->statistics = new PingStatistics(60);
        //reverse resolution
        QHostInfo::lookupHost(address, this, SLOT(lookupHostReply(QHostInfo)));

        //start async worker
        if (!this->asyncWorker->isActive())
            this->asyncWorker->start(2000);
    } else
        newItem->waitingForReply = false;

    //add to local list
    this->itemHash.insert(address, newItem);
    this->itemList.append(newItem);

    this->mutex->unlock();
}

void TracerouteItemManager::clear()
{
    this->mutex->lock();

    this->asyncWorker->stop();

    //remove items
    TracerouteItemReference* nextItem;
    for (int i = 0; i < this->itemList.size(); ++i) {
        nextItem = this->itemList.at(i);
        //
        this->container->removeWidget(nextItem->widget);
        delete nextItem->widget;
        //
        if (nextItem->validAddress)
        {
            if (nextItem->waitingForReply)
                Pinger::stopListening(nextItem->pigingContext);
            delete nextItem->statistics;
        }
        delete nextItem;
    }
    this->itemHash.clear();
    this->itemList.clear();

    this->mutex->unlock();
}

void TracerouteItemManager::lookupHostReply(QHostInfo hostInfo)
{
    //avoid processing responses from last session
    if (!this->itemHash.contains(hostInfo.addresses().first().toString()))
        return;

    TracerouteItemReference* item = this->itemHash.value(hostInfo.addresses().first().toString());
    if (item->address != hostInfo.hostName())
        item->widget->setAddress(hostInfo.hostName() + " (" + item->hostAddress.toString() + ")");
}

void TracerouteItemManager::receivePingReply(PingContext* context)
{
    //avoid processing responses from last session
    if (!this->itemHash.contains(context->requestAddress.toString()))
        return;

    TracerouteItemReference* item = this->itemHash.value(context->requestAddress.toString());

    if (context->replyStatus == PINGER_STATUS_SUCCESS ||
        context->replyStatus == PINGER_STATUS_EXPIREDINTRANSIT)
        item->statistics->addResponse(context->millisLatencyHighResolution);
    else
        item->statistics->addTimeout();

#ifdef QT_DEBUG
    if (context->replyStatus == PINGER_STATUS_SUCCESS ||
        context->replyStatus == PINGER_STATUS_EXPIREDINTRANSIT)
        qDebug() << "Traceroute RTT:" << item->address << context->millisLatencyHighResolution << "Avg:"<<item->statistics->averageLatency();
    else
        qDebug() << "Traceroute RTT:" << item->address << "TIMEOUT!" << "Avg:"<<item->statistics->averageLatency();
#endif

    if (context->replyStatus == PINGER_STATUS_SUCCESS ||
        context->replyStatus == PINGER_STATUS_EXPIREDINTRANSIT)
        item->widget->setLastRTT(item->statistics->lastLatency());

    if (item->statistics->averageLatency() > 0)
        item->widget->setAvgRTT(item->statistics->averageLatency());

    if (item->statistics->averageJitter() > 0)
        item->widget->setJitter(item->statistics->averageJitter(), item->statistics->percentJitter());

    if (item->statistics->totalResponses() > 0)
        item->widget->setPctLoss(item->statistics->percentLoss());

    item->waitingForReply = false;
}

void TracerouteItemManager::asyncTask()
{
    this->mutex->lock();

    TracerouteItemReference* nextItem;
    for (int i = 0; i < this->itemList.size(); ++i) {
        nextItem = this->itemList.at(i);
        if (nextItem->validAddress && !nextItem->waitingForReply)
        {
            nextItem->waitingForReply = true;
            nextItem->pigingContext = this->pingerInstance->ping(nextItem->hostAddress, this);

            //create an interval to avoid affecting the response time with the increase of the throughtput
            Sleep(25);
        }
    }

    this->mutex->unlock();
}

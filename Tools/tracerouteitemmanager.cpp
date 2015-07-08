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
        newItem->widget->setRTT(rtt);
    } else {
        newItem->widget->setAddress(TRACEROUTE_TIMEOUT_ADDRESS);
        newItem->widget->setRTT(TRACEROUTE_TIMEOUT_RTT);
    }
    //add to container
    this->container->addWidget(newItem->widget);

    //starting pinging and reverse resolution if this item has a valid address
    if (newItem->validAddress)
    {
        //ping host
        newItem->waitingForReply = true;
        newItem->pigingContext = this->pingerInstance->ping(newItem->hostAddress, this);
        //prepare statistics calculator
        newItem->statistics = new PingStatistics(10);
        //reverse resolution
        QHostInfo::lookupHost(address, this, SLOT(lookupHostReply(QHostInfo)));

        //start async worker
        if (!this->asyncWorker->isActive())
            this->asyncWorker->start(1000);
    } else
        newItem->waitingForReply = false;

    //add to local list
    this->itemList.insert(address, newItem);

    this->mutex->unlock();
}

void TracerouteItemManager::clear()
{
    this->mutex->lock();

    this->asyncWorker->stop();

    //remove items
    QHashIterator<QString, TracerouteItemReference*> iterator(this->itemList);
    TracerouteItemReference* nextItem;
    while (iterator.hasNext()) {
        iterator.next();
        nextItem = iterator.value();
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
    this->itemList.clear();

    this->mutex->unlock();
}

void TracerouteItemManager::lookupHostReply(QHostInfo hostInfo)
{
    TracerouteItemReference* item = this->itemList.value(hostInfo.addresses().first().toString());
    if (item->address != hostInfo.hostName())
        item->widget->setAddress(hostInfo.hostName() + " (" + item->hostAddress.toString() + ")");
}

void TracerouteItemManager::receivePingReply(PingContext* context)
{
    TracerouteItemReference* item = this->itemList.value(context->requestAddress.toString());

    if (context->replyStatus == PINGER_STATUS_SUCCESS)
        item->statistics->addResponse(context->millisLatencyHighResolution);
    else
        item->statistics->addTimeout();

    if (context->replyStatus == PINGER_STATUS_SUCCESS)
        qDebug() << "receivePingReply" << item->address << context->millisLatencyHighResolution << "Avg:"<<item->statistics->averageLatency();
    else
        qDebug() << "receivePingReply" << item->address << "TIMEOUT!" << "Avg:"<<item->statistics->averageLatency();

    item->widget->setRTT(item->statistics->averageLatency());

    item->waitingForReply = false;
}

void TracerouteItemManager::asyncTask()
{
    this->mutex->lock();

    QHashIterator<QString, TracerouteItemReference*> iterator(this->itemList);
    TracerouteItemReference* nextItem;
    while (iterator.hasNext()) {
        iterator.next();
        nextItem = iterator.value();
        if (nextItem->validAddress && !nextItem->waitingForReply)
        {
            nextItem->waitingForReply = true;
            nextItem->pigingContext = this->pingerInstance->ping(nextItem->hostAddress, this);
        }
    }

    this->mutex->unlock();
}

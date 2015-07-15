#ifndef TRACEROUTEITEMMANAGER_H
#define TRACEROUTEITEMMANAGER_H

#include <QObject>
#include <QHash>
#include <QMutex>
#include <QHostInfo>
#include <QTimer>
#include <QList>

#include "tracerouteitem.h"
#include "pingstatistics.h"
#include "../Net/pinger.h"

struct TracerouteItemReference
{
    QString address;
    TracerouteItem* widget;
    QHostAddress hostAddress;
    PingStatistics* statistics;
    PingContext* pigingContext;
    bool validAddress;
    bool waitingForReply;
};

class TracerouteItemManager : public QObject, public IPingReplyListener
{
    Q_OBJECT

public:
    TracerouteItemManager(QBoxLayout* container);
    ~TracerouteItemManager();
    void clear();
    void receivePingReply(PingContext* context);

public slots:
    void addItem(int replyStatus, int ttl, QString address, double rtt);
    void asyncTask();

private slots:
    void lookupHostReply(QHostInfo);

private:
    QBoxLayout* container;
    QList<TracerouteItemReference*> itemList;
    QHash<QString, TracerouteItemReference*> itemHash;
    QMutex* mutex;
    Pinger* pingerInstance;
    QTimer* asyncWorker;
};

#endif // TRACEROUTEITEMMANAGER_H

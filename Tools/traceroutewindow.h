#ifndef TRACEROUTEWINDOW_H
#define TRACEROUTEWINDOW_H

#include <QWidget>
#include <QHostInfo>
#include <QList>

#include "../hoststreeview.h"
#include "../Net/pinger.h"
#include "../pingstatistics.h"
#include "tracerouteitemmanager.h"

namespace Ui {
class TracerouteWindow;
}

class TracerouteWindow : public QWidget, public IPingReplyListener
{
    Q_OBJECT

public:
    explicit TracerouteWindow(QWidget *parent = 0);
    ~TracerouteWindow();
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);
    void receivePingReply(PingContext* context);

private slots:
    void run(int hostID);
    void lookupHostReply(QHostInfo);
    void asyncTask();
    void shrink();

private:
    Ui::TracerouteWindow *ui;
    int TTL;
    Pinger* pingerInstance;
    QTimer* asyncWorker;
    PingContext* pigingContext;
    QHostAddress destinationAddress;
    void resetValues();
    TracerouteItemManager* itemManager;
    bool waitingForReply;
    int lookupID;
    int running;
};

#endif // TRACEROUTEWINDOW_H

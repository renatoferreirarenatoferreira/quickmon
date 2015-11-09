#ifndef SNMPGRAPHWINDOW_H
#define SNMPGRAPHWINDOW_H

#include <QWidget>
#include <QMap>
#include <QSqlQuery>
#include <QHostAddress>
#include <QHostInfo>

#include "snmpgraphlegenditem.h"
#include "../hoststreeview.h"
#include "../Net/snmpclient.h"
#include "../localdata.h"
#include "snmpstatistics.h"

#define SCALE_PERCENT 0
#define SCALE_BYTE 1
#define SCALE_OTHER 2

namespace Ui {
class SNMPGraphWindow;
}

struct SNMPGraphLegendItemReference
{
    QString OID;
    SNMPGraphLegendItem* legendItem;
    int graphIndex;
    SNMPStatistics* statistics;
};

class SNMPGraphWindow : public QWidget, public ISNMPReplyListener
{
    Q_OBJECT

public:
    explicit SNMPGraphWindow(QWidget *parent = 0);
    ~SNMPGraphWindow();
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);
    void configure(QString templateName, QMap<QString, QVariant> templateItems, QString SNMPIndex = "");
    void configureAndRun(QString templateName, QMap<QString, QVariant> templateItems, QString SNMPIndex, int hostID);
    void receiveSNMPReply(SNMPData* data);

private slots:
    void run(int hostID);
    void lookupHostReply(QHostInfo);
    void asyncTask();
    void updateUI(int responseStatus, QString errorMessage, QList<SNMPVariable> returnVariables);

private:
    Ui::SNMPGraphWindow *ui;
    SNMPClient* clientInstance;
    QTimer* asyncWorker;
    QString templateName;
    QHash<QString, SNMPGraphLegendItemReference*> itemHash;
    QStringList OIDs;
    QHostAddress destinationAddress;
    int lookupID;
    int destinationSNMPVersion;
    QString destinationSNMPCommunity;
    QString destinationSNMPv3SecLevel;
    QString destinationSNMPv3AuthProtocol;
    QString destinationSNMPv3AuthPassPhrase;
    QString destinationSNMPv3PrivProtocol;
    QString destinationSNMPv3PrivPassPhrase;
    void resetValues();
    SNMPData* updateValues();
    bool waitingForReply;
    bool retryImmediatelly;
    int lastHostID;
    SNMPData* SNMPdata = NULL;
    QString unit;
    int scale;
    int interval;
};

#endif // SNMPGRAPHWINDOW_H

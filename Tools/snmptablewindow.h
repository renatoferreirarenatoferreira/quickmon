#ifndef SNMPTABLEWINDOW_H
#define SNMPTABLEWINDOW_H

#include <QWidget>
#include <QMap>
#include <QSqlQuery>
#include <QHostAddress>
#include <QHostInfo>
#include <QTableWidgetItem>
#include <QMenu>
#include <QAction>

#include "../hoststreeview.h"
#include "../Net/snmpclient.h"
#include "../localdata.h"
#include "snmpgraphwindow.h"
#include "../Utils/snmptextualconventions.h"

#define CONTEXTMENU_TYPE_GRAPH 0

namespace Ui {
class SNMPTableWindow;
}

struct SNMPTableItemReference
{
    QString OID;
    QString baseOID;
    QTableWidgetItem* tableItem;
    bool valueMapped;
    QString valueType;
};

struct SNMPTableContextMenuReference
{
    QAction* menuAction;
    QString itemName;
    QMap<QString, QVariant> templateItem;
    int itemType;
};

class SNMPTableWindow : public QWidget, public ISNMPReplyListener
{
    Q_OBJECT

public:
    explicit SNMPTableWindow(QWidget *parent = 0);
    ~SNMPTableWindow();
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);
    void configure(QString templateName, QMap<QString, QVariant> templateItems);
    void receiveSNMPReply(SNMPData* data);

private slots:
    void run(int hostID);
    void lookupHostReply(QHostInfo);
    void warn(QString title, QString message);
    void showContextMenu(const QPoint& pos);

private:
    Ui::SNMPTableWindow *ui;
    SNMPClient* clientInstance;
    QString templateName;
    QHash<QString, SNMPTableItemReference*> itemHash;
    QStringList OIDs;
    QHostAddress destinationAddress;
    int lookupID;
    int destinationSNMPVersion;
    QString destinationSNMPCommunityUser;
    QString destinationSNMPv3SecLevel;
    QString destinationSNMPv3AuthProtocol;
    QString destinationSNMPv3AuthPassPhrase;
    QString destinationSNMPv3PrivProtocol;
    QString destinationSNMPv3PrivPassPhrase;
    void resetValues();
    SNMPData* updateValues();
    SNMPData* updateValues(QStringList OIDs);
    bool waitingForReply;
    bool requestSent;
    int lastHostID;
    SNMPData* SNMPdata = NULL;
    QMenu contextMenu;
    QHash<QAction*, SNMPTableContextMenuReference*> menuHash;
    QIcon graphIcon;
};

#endif // SNMPTABLEWINDOW_H

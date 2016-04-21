#ifndef SNMPLISTWINDOW_H
#define SNMPLISTWINDOW_H

#include <QWidget>
#include <QMap>
#include <QSqlQuery>
#include <QHostAddress>
#include <QHostInfo>
#include <QHash>
#include <QMapIterator>

#include "snmplistitem.h"
#include "../hoststreeview.h"
#include "../Net/snmpclient.h"
#include "../localdata.h"
#include "../Utils/snmptextualconventions.h"

namespace Ui {
class SNMPListWindow;
}

struct SNMPListItemReference
{
    QString OID;
    SNMPListItem* listItem;
    bool valueMapped;
    QString valueType;
};

class SNMPListWindow : public QWidget, public ISNMPReplyListener
{
    Q_OBJECT

public:
    explicit SNMPListWindow(QWidget *parent = 0);
    ~SNMPListWindow();
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);
    void configure(QString templateName, QMap<QString, QVariant> templateItems);
    void receiveSNMPReply(SNMPData* data);

private slots:
    void run(int hostID);
    void lookupHostReply(QHostInfo);
    void shrink();
    void warn(QString title, QString message);

private:
    Ui::SNMPListWindow *ui;
    SNMPClient* clientInstance;
    QString templateName;
    QHash<QString, SNMPListItemReference*> itemHash;
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
    bool waitingForReply;
    bool requestSent;
    int lastHostID;
    SNMPData* SNMPdata = NULL;
};

#endif // SNMPLISTWINDOW_H

#include "snmplistwindow.h"
#include "ui_snmplistwindow.h"

SNMPListWindow::SNMPListWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SNMPListWindow)
{
    ui->setupUi(this);
    //window flags
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    //drop behavior
    setAcceptDrops(true);

    //get a client instance
    this->clientInstance = SNMPClient::Instance();

    //reset values
    this->resetValues();
    this->lastHostID = -1;
}

SNMPListWindow::~SNMPListWindow()
{
    //disable callback
    if (this->waitingForReply)
        SNMPClient::stopListening(this->SNMPdata);

    //delete items
    QHashIterator<QString, SNMPListItemReference*> iterator(this->itemHash);
    while (iterator.hasNext()) {
        iterator.next();
        SNMPListItemReference* nextItem = iterator.value();
        delete nextItem->listItem;
        delete nextItem;
    }
    this->itemHash.clear();
    this->OIDs.clear();

    delete ui;
}

void SNMPListWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasText())
        if (event->mimeData()->text().startsWith(MIMEDATA_PREFIX))
            event->acceptProposedAction();
}

void SNMPListWindow::dropEvent(QDropEvent* event)
{
    QMetaObject::invokeMethod(this, "run", Qt::QueuedConnection,
                              Q_ARG(int, event->mimeData()->text().mid(MIMEDATA_PREFIX_LENGTH).toInt()));

    event->acceptProposedAction();
}

void SNMPListWindow::configure(QString templateName, QMap<QString, QVariant> templateItems)
{
    this->setWindowTitle("SNMP List: " + templateName);
    ui->labelTemplate->setText(templateName);

    this->templateName = templateName;

    QMap<int, SNMPListItem*> orderedList;

    QMap<QString, QVariant> itemList = templateItems.value("Items").toMap();
    QMapIterator<QString, QVariant> itemListIterator(itemList);
    while (itemListIterator.hasNext())
    {
        itemListIterator.next();

        QString itemName = itemListIterator.key();
        QMap<QString, QVariant> itemValues = itemListIterator.value().toMap();
        QString OID = itemValues.value("OID").toString();
        int itemOrder = itemValues.value("Order").toInt();

        SNMPListItemReference* newItem = new SNMPListItemReference();
        newItem->listItem = new SNMPListItem();
        newItem->listItem->configure(itemOrder, itemName);
        newItem->valueType = itemValues.value("Type").toString();

        newItem->valueMapped = itemValues.contains("ValueMappings");
        if (newItem->valueMapped)
            this->clientInstance->addMappings(OID, itemValues.value("ValueMappings").toMap());

        orderedList.insert(itemOrder, newItem->listItem);
        this->itemHash.insert(OID, newItem);
        this->OIDs.append(OID);
    }

    //add items using the configured order
    QMapIterator<int, SNMPListItem*> iterator(orderedList);
    while (iterator.hasNext()) {
        iterator.next();
        ui->layoutListItems->addWidget(iterator.value());
    }
}

void SNMPListWindow::run(int hostID)
{
    //avoid re-running for the same host
    if (this->lastHostID == hostID)
        return;
    else
        this->lastHostID = hostID;

    //reset tasks
    this->resetValues();

    QSqlQuery query = LocalData::Instance()->createQuery("SELECT * FROM hosts WHERE ID=:ID");
    query.bindValue(":ID", hostID);
    LocalData::Instance()->executeQuery(query);

    if (query.next())
    {
        ui->labelName->setText(query.value("name").toString());
        ui->labelAddress->setText(query.value("address").toString());

        //copy snmp data
        this->destinationSNMPVersion = query.value("snmpversion").toInt();
        this->destinationSNMPCommunity = query.value("snmpcommunity").toString();
        this->destinationSNMPv3SecLevel = query.value("snmpv3seclevel").toString();
        this->destinationSNMPv3AuthProtocol = query.value("snmpv3authprotocol").toString();
        this->destinationSNMPv3AuthPassPhrase = query.value("snmpv3authpassphrase").toString();
        this->destinationSNMPv3PrivProtocol = query.value("snmpv3privprotocol").toString();
        this->destinationSNMPv3PrivPassPhrase = query.value("snmpv3privpassphrase").toString();

        //start immediately if adress is valid IPv4/6
        this->destinationAddress.setAddress(query.value("address").toString());
        if (this->destinationAddress.protocol() == QAbstractSocket::IPv4Protocol ||
            this->destinationAddress.protocol() == QAbstractSocket::IPv6Protocol)
        {
            //update title
            this->setWindowTitle(this->templateName + ": " + query.value("address").toString());
            //collect data
            this->SNMPdata = this->updateValues();
            this->waitingForReply = true;
            this->requestSent = true;
        } else {
            this->setWindowTitle(this->templateName + ": Looking up for hostname " + query.value("address").toString());
        }
        //always look up for host name
        this->lookupID = QHostInfo::lookupHost(query.value("address").toString(), this, SLOT(lookupHostReply(QHostInfo)));
    }
}

void SNMPListWindow::lookupHostReply(QHostInfo hostInfo)
{
    //avoid lookup overlapping
    if (this->lookupID != hostInfo.lookupId())
        return;

    if (!hostInfo.addresses().isEmpty())
    {
        //update address label
        this->destinationAddress = hostInfo.addresses().first();
        ui->labelAddress->setText(this->destinationAddress.toString()+" ("+hostInfo.hostName()+")");
        //update title
        this->setWindowTitle(this->templateName + ": " + this->destinationAddress.toString()+" ("+hostInfo.hostName()+")");

        if (!this->requestSent)
        {
            this->SNMPdata = this->updateValues();
            this->waitingForReply = true;
        }
    } else {
        QMessageBox::warning(this, "Hostname lookup", "Address cannot be resolved or is invalid!");
        this->setWindowTitle(this->templateName + ": Address cannot be resolved or is invalid!");
    }
}

SNMPData* SNMPListWindow::updateValues()
{
    if (this->destinationSNMPVersion == 1 || this->destinationSNMPVersion == 2)
        return this->clientInstance->SNMPGet(this->destinationSNMPVersion,
                                             this->destinationAddress,
                                             this->OIDs,
                                             this->destinationSNMPCommunity,
                                             this);
    else if (this->destinationSNMPVersion == 3)
        return this->clientInstance->SNMPv3Get(this->destinationAddress,
                                               this->OIDs,
                                               this->destinationSNMPv3SecLevel,
                                               this->destinationSNMPv3AuthProtocol,
                                               this->destinationSNMPv3AuthPassPhrase,
                                               this->destinationSNMPv3PrivProtocol,
                                               this->destinationSNMPv3PrivPassPhrase,
                                               this);
    else
        return NULL;
}

void SNMPListWindow::resetValues()
{
    //disable callback
    if (this->waitingForReply && this->SNMPdata != NULL)
        SNMPClient::stopListening(this->SNMPdata);

    //reset values
    this->waitingForReply = false;
    this->requestSent = false;
    this->destinationAddress.setAddress("0.0.0.0");

    //clear items
    QHashIterator<QString, SNMPListItemReference*> iterator(this->itemHash);
    while (iterator.hasNext()) {
        iterator.next();
        SNMPListItemReference* nextItem = iterator.value();
        nextItem->listItem->setValue("");
    }
    //reset size
    QTimer::singleShot(50, this, SLOT(shrink()));
}

void SNMPListWindow::shrink()
{
    resize(0, 0);
}

void SNMPListWindow::receiveSNMPReply(SNMPData* data)
{
    //ignore delayed responses
    if (data->requestAddress != this->destinationAddress)
        return;

    if (data->responseStatus == SNMP_RESPONSE_SUCCESS)
    {
        //update ui
        QListIterator<SNMPVariable> iterator(data->returnVariables);
        while (iterator.hasNext()) {
            SNMPVariable nextVariable = iterator.next();

            SNMPListItemReference* nextItem;
            if (this->itemHash.contains(nextVariable.OID))
            {
                nextItem = this->itemHash.value(nextVariable.OID);
                if (nextItem->valueMapped)
                    nextItem->listItem->setValue(this->clientInstance->mapValue(nextVariable.OID, nextVariable.variantValue.toString()));
                else if (nextVariable.type == SNMPVARIABLE_TYPE_OCTETSTRING)
                    if (nextItem->valueType == "Hex")
                        nextItem->listItem->setValue(nextVariable.variantValue.toMap().value("hexValue").toString());
                    else if (nextItem->valueType == "DateAndTime")
                        nextItem->listItem->setValue(SNMPClient::readDateAndTime(nextVariable.variantValue.toMap().value("hexValue").toString()));
                    else
                        nextItem->listItem->setValue(nextVariable.variantValue.toMap().value("stringValue").toString());
                else
                    nextItem->listItem->setValue(nextVariable.variantValue.toString());
            }
        }
    } else if (data->responseStatus == SNMP_RESPONSE_TIMEOUT)
        QMetaObject::invokeMethod(this, "warn", Qt::QueuedConnection, Q_ARG(QString, "SNMP error"),
                                                                      Q_ARG(QString, "SNMP request timed out!"));
    else
        QMetaObject::invokeMethod(this, "warn", Qt::QueuedConnection, Q_ARG(QString, "SNMP error"),
                                                                      Q_ARG(QString, "Unknown error in SNMP request!"));

    //disable waiting flag
    this->waitingForReply = false;

#ifdef QT_DEBUG
    QListIterator<SNMPVariable> iterator(data->returnVariables);
    while (iterator.hasNext()) {
        SNMPVariable nextVariable = iterator.next();
        qDebug() << "OID:" << nextVariable.OID << "Value:" << nextVariable.variantValue;
    }
#endif
}

void SNMPListWindow::warn(QString title, QString message)
{
    QMessageBox::warning(this, title, message);
}

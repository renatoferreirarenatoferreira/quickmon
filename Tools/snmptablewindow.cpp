#include "snmptablewindow.h"
#include "ui_snmptablewindow.h"

SNMPTableWindow::SNMPTableWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SNMPTableWindow)
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

SNMPTableWindow::~SNMPTableWindow()
{
    //disable callback
    if (this->waitingForReply)
        SNMPClient::stopListening(this->SNMPdata);

    //clean up table
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);
    //delete items
    QHashIterator<QString, SNMPTableItemReference*> iterator(this->itemHash);
    while (iterator.hasNext()) {
        iterator.next();
        SNMPTableItemReference* nextItem = iterator.value();
        //QTableWidget takes ownership of the item.
        //delete nextItem->tableItem;
        delete nextItem;
    }
    this->itemHash.clear();
    this->OIDs.clear();

    delete ui;
}

void SNMPTableWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasText())
        if (event->mimeData()->text().startsWith(MIMEDATA_PREFIX))
            event->acceptProposedAction();
}

void SNMPTableWindow::dropEvent(QDropEvent* event)
{
    QMetaObject::invokeMethod(this, "run", Qt::QueuedConnection,
                              Q_ARG(int, event->mimeData()->text().mid(MIMEDATA_PREFIX_LENGTH).toInt()));

    event->acceptProposedAction();
}

void SNMPTableWindow::configure(QString templateName, QMap<QString, QVariant> templateItems)
{
    this->setWindowTitle("SNMP Table: " + templateName);
    ui->labelTemplate->setText(templateName);

    this->templateName = templateName;

    QMap<int, SNMPTableItemReference*> orderedList;

    QMap<QString, QVariant> itemList = templateItems.value("Items").toMap();
    QMapIterator<QString, QVariant> itemListIterator(itemList);
    while (itemListIterator.hasNext())
    {
        itemListIterator.next();

        QString itemName = itemListIterator.key();
        QMap<QString, QVariant> itemValues = itemListIterator.value().toMap();
        QString OID = itemValues.value("OID").toString();
        int itemOrder = itemValues.value("Order").toInt();

        SNMPTableItemReference* newItem = new SNMPTableItemReference();
        newItem->OID = OID;
        newItem->tableItem = new QTableWidgetItem(itemName);
        newItem->valueType = itemValues.value("Type").toString();

        newItem->valueMapped = itemValues.contains("ValueMappings");
        if (newItem->valueMapped)
            this->clientInstance->addMappings(OID, itemValues.value("ValueMappings").toMap());

        orderedList.insert(itemOrder, newItem);
        this->itemHash.insert(newItem->OID, newItem);
    }

    //configure table
    ui->tableWidget->setColumnCount(orderedList.size());

    //add items using the configured order
    QMapIterator<int, SNMPTableItemReference*> iterator(orderedList);
    int column = 0;
    int columnSize = 625/orderedList.size();
    while (iterator.hasNext()) {
        iterator.next();
        SNMPTableItemReference* nextItem = iterator.value();

        ui->tableWidget->setHorizontalHeaderItem(column, nextItem->tableItem);
        ui->tableWidget->setColumnWidth(column, columnSize);

        this->OIDs.append(nextItem->OID);
        column++;
    }
}

void SNMPTableWindow::run(int hostID)
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

void SNMPTableWindow::lookupHostReply(QHostInfo hostInfo)
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

SNMPData* SNMPTableWindow::updateValues()
{
    if (this->destinationSNMPVersion == 1 || this->destinationSNMPVersion == 2)
        return this->clientInstance->SNMPWalk(this->destinationSNMPVersion,
                                              this->destinationAddress,
                                              this->OIDs.first(),
                                              this->destinationSNMPCommunity,
                                              this);
    else if (this->destinationSNMPVersion == 3)
        return this->clientInstance->SNMPv3Walk(this->destinationAddress,
                                                this->OIDs.first(),
                                                this->destinationSNMPv3SecLevel,
                                                this->destinationSNMPv3AuthProtocol,
                                                this->destinationSNMPv3AuthPassPhrase,
                                                this->destinationSNMPv3PrivProtocol,
                                                this->destinationSNMPv3PrivPassPhrase,
                                                this);
    else
        return NULL;
}

SNMPData* SNMPTableWindow::updateValues(QStringList OIDs)
{
    if (this->destinationSNMPVersion == 1 || this->destinationSNMPVersion == 2)
        return this->clientInstance->SNMPGet(this->destinationSNMPVersion,
                                             this->destinationAddress,
                                             OIDs,
                                             this->destinationSNMPCommunity,
                                             this);
    else if (this->destinationSNMPVersion == 3)
        return this->clientInstance->SNMPv3Get(this->destinationAddress,
                                               OIDs,
                                               this->destinationSNMPv3SecLevel,
                                               this->destinationSNMPv3AuthProtocol,
                                               this->destinationSNMPv3AuthPassPhrase,
                                               this->destinationSNMPv3PrivProtocol,
                                               this->destinationSNMPv3PrivPassPhrase,
                                               this);
    else
        return NULL;
}

void SNMPTableWindow::resetValues()
{
    //disable callback
    if (this->waitingForReply && this->SNMPdata != NULL)
        SNMPClient::stopListening(this->SNMPdata);

    //reset values
    this->waitingForReply = false;
    this->requestSent = false;
    this->destinationAddress.setAddress("0.0.0.0");

    //clean up table
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    //clear items
    QHashIterator<QString, SNMPTableItemReference*> iterator(this->itemHash);
    while (iterator.hasNext()) {
        iterator.next();
        SNMPTableItemReference* nextItem = iterator.value();
        //keep headers
        if (nextItem->baseOID.size() > 0)
        {
            //QTableWidget takes ownership of the item.
            //delete nextItem->tableItem;
            delete nextItem;
            this->itemHash.remove(iterator.key());
        }
    }
}

void SNMPTableWindow::receiveSNMPReply(SNMPData* data)
{
    //ignore delayed responses
    if (data->requestAddress != this->destinationAddress)
        return;

    if (data->responseStatus == SNMP_RESPONSE_SUCCESS)
    {
        if (data->queryType == SNMPCLIENT_QUERYTYPE_WALK) //phase 1
        {
            //configure table
            ui->tableWidget->setRowCount(data->returnVariables.size());

            QStringList remainingOIDs;
            int row = 0;

            QListIterator<SNMPVariable> returnIterator(data->returnVariables);
            while (returnIterator.hasNext())
            {
                SNMPVariable nextVariable = returnIterator.next();
                QString nextSufix = nextVariable.OID.right(nextVariable.OID.size() - this->OIDs.first().size());

                //add first item from snmpwalk
                SNMPTableItemReference* newItem = new SNMPTableItemReference();
                newItem->OID = nextVariable.OID;
                newItem->tableItem = new QTableWidgetItem();
                //set item configuration
                if (this->itemHash.contains(this->OIDs.first()))
                {
                    SNMPTableItemReference* searchItem = this->itemHash.value(this->OIDs.first());
                    newItem->valueMapped = searchItem->valueMapped;
                    newItem->valueType = searchItem->valueType;
                }
                //set item data
                if (newItem->valueMapped)
                    newItem->tableItem->setText(this->clientInstance->mapValue(newItem->baseOID, nextVariable.variantValue.toString()));
                else if (nextVariable.type == SNMPVARIABLE_TYPE_OCTETSTRING)
                    if (newItem->valueType == "Hex")
                        newItem->tableItem->setText(nextVariable.variantValue.toMap().value("hexValue").toString());
                    else
                        newItem->tableItem->setText(nextVariable.variantValue.toMap().value("stringValue").toString());
                else if (nextVariable.type == SNMPVARIABLE_TYPE_INTEGER)
                    newItem->tableItem->setData(Qt::DisplayRole, nextVariable.variantValue.toInt());
                else if (nextVariable.type == SNMPVARIABLE_TYPE_UNSIGNEDINT)
                    newItem->tableItem->setData(Qt::DisplayRole, nextVariable.variantValue.toUInt());
                else if (nextVariable.type == SNMPVARIABLE_TYPE_COUNTER64)
                    newItem->tableItem->setData(Qt::DisplayRole, nextVariable.variantValue.toULongLong());
                else
                    newItem->tableItem->setText(nextVariable.variantValue.toString());
                //add data to table
                ui->tableWidget->setItem(row, 0, newItem->tableItem);
                //add to hashtable
                this->itemHash.insert(nextVariable.OID, newItem);

                for (int i = 1; i < this->OIDs.size(); ++i)
                {
                    QString OID = this->OIDs.at(i);

                    //add remaining items
                    SNMPTableItemReference* newItem = new SNMPTableItemReference();
                    newItem->OID = OID + nextSufix;
                    newItem->baseOID = OID;
                    newItem->tableItem = new QTableWidgetItem();
                    //set item configuration
                    if (this->itemHash.contains(OID))
                    {
                        SNMPTableItemReference* searchItem = this->itemHash.value(OID);
                        newItem->valueMapped = searchItem->valueMapped;
                        newItem->valueType = searchItem->valueType;
                    }
                    //add data to table
                    ui->tableWidget->setItem(row, i, newItem->tableItem);
                    //add to hashtable
                    this->itemHash.insert(newItem->OID, newItem);

                    //populate query list
                    remainingOIDs.append(newItem->OID);
                }
                row++;
            }

            //get remaining table data
            this->SNMPdata = this->updateValues(remainingOIDs);

        } if (data->queryType == SNMPCLIENT_QUERYTYPE_GET) //phase 2
        {
            //populate table
            QListIterator<SNMPVariable> returnIterator(data->returnVariables);
            while (returnIterator.hasNext())
            {
                SNMPVariable nextVariable = returnIterator.next();

                //set table data
                if (this->itemHash.contains(nextVariable.OID))
                {
                    SNMPTableItemReference* searchItem = this->itemHash.value(nextVariable.OID);

                    //set item data
                    if (searchItem->valueMapped)
                        searchItem->tableItem->setText(this->clientInstance->mapValue(searchItem->baseOID, nextVariable.variantValue.toString()));
                    else if (nextVariable.type == SNMPVARIABLE_TYPE_OCTETSTRING)
                        if (searchItem->valueType == "Hex")
                            searchItem->tableItem->setText(nextVariable.variantValue.toMap().value("hexValue").toString());
                        else
                            searchItem->tableItem->setText(nextVariable.variantValue.toMap().value("stringValue").toString());
                    else if (nextVariable.type == SNMPVARIABLE_TYPE_INTEGER)
                        searchItem->tableItem->setData(Qt::DisplayRole, nextVariable.variantValue.toInt());
                    else if (nextVariable.type == SNMPVARIABLE_TYPE_UNSIGNEDINT)
                        searchItem->tableItem->setData(Qt::DisplayRole, nextVariable.variantValue.toUInt());
                    else if (nextVariable.type == SNMPVARIABLE_TYPE_COUNTER64)
                        searchItem->tableItem->setData(Qt::DisplayRole, nextVariable.variantValue.toULongLong());
                    else
                        searchItem->tableItem->setText(nextVariable.variantValue.toString());
                }
            }
        }
    } else if (data->responseStatus == SNMP_RESPONSE_TIMEOUT)
        QMetaObject::invokeMethod(this, "warn", Qt::QueuedConnection, Q_ARG(QString, "SNMP error"),
                                                                      Q_ARG(QString, "SNMP request timed out!"));
    else
        QMetaObject::invokeMethod(this, "warn", Qt::QueuedConnection, Q_ARG(QString, "SNMP error"),
                                                                      Q_ARG(QString, "Unknown error in SNMP request!"));

    //disable waiting flag (ONLY AFTER PHASE 2 or in case of errors)
    if (data->queryType == SNMPCLIENT_QUERYTYPE_GET || data->responseStatus != SNMP_RESPONSE_SUCCESS)
        this->waitingForReply = false;

#ifdef QT_DEBUG
    QListIterator<SNMPVariable> iterator(data->returnVariables);
    while (iterator.hasNext()) {
        SNMPVariable nextVariable = iterator.next();
        qDebug() << "OID:" << nextVariable.OID << "Value:" << nextVariable.variantValue;
    }
#endif
}

void SNMPTableWindow::warn(QString title, QString message)
{
    QMessageBox::warning(this, title, message);
}

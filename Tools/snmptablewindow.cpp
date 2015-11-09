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

    //configure context menu
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));
    //menu icons
    this->graphIcon.addFile(QStringLiteral(":/Icons/SNMPGraph"), QSize());

    //configure selection behavior
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

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

    //remove context menu items
    QHashIterator<QAction*, SNMPTableContextMenuReference*> contextMenuIterator(this->menuHash);
    while (contextMenuIterator.hasNext())
    {
        contextMenuIterator.next();
        SNMPTableContextMenuReference* nextItem = contextMenuIterator.value();
        delete nextItem->menuAction;
        delete nextItem;
    }
    this->menuHash.clear();

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

    // configure table data //////////////////////////////////////////////////
    QMap<int, SNMPTableItemReference*> orderedDataList;

    QMap<QString, QVariant> itemDataList = templateItems.value("Items").toMap();
    QMapIterator<QString, QVariant> itemDataListIterator(itemDataList);
    while (itemDataListIterator.hasNext())
    {
        itemDataListIterator.next();

        QString itemName = itemDataListIterator.key();
        QMap<QString, QVariant> itemValues = itemDataListIterator.value().toMap();
        QString OID = itemValues.value("OID").toString();
        int itemOrder = itemValues.value("Order").toInt();

        SNMPTableItemReference* newItem = new SNMPTableItemReference();
        newItem->OID = OID;
        newItem->tableItem = new QTableWidgetItem(itemName);
        newItem->valueType = itemValues.value("Type").toString();

        newItem->valueMapped = itemValues.contains("ValueMappings");
        if (newItem->valueMapped)
            this->clientInstance->addMappings(OID, itemValues.value("ValueMappings").toMap());

        orderedDataList.insert(itemOrder, newItem);
        this->itemHash.insert(newItem->OID, newItem);
    }

    //configure table
    ui->tableWidget->setColumnCount(orderedDataList.size()+1);
    //add hidden index/sufix column
    ui->tableWidget->setColumnHidden(0, true);
    ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("index"));

    //add items using the configured order
    QMapIterator<int, SNMPTableItemReference*> orderedDataListIterator(orderedDataList);
    int column = 1;
    int columnSize = 625/orderedDataList.size();
    while (orderedDataListIterator.hasNext())
    {
        orderedDataListIterator.next();
        SNMPTableItemReference* nextItem = orderedDataListIterator.value();

        ui->tableWidget->setHorizontalHeaderItem(column, nextItem->tableItem);
        ui->tableWidget->setColumnWidth(column, columnSize);

        this->OIDs.append(nextItem->OID);
        column++;
    }
    //////////////////////////////////////////////////////////////////////////


    // configure context menu //////////////////////////////////////////////////
    QMap<int, SNMPTableContextMenuReference*> orderedMenuList;

    QMap<QString, QVariant> itemMenuList = templateItems.value("ContextMenu").toMap();
    QMapIterator<QString, QVariant> itemMenuListIterator(itemMenuList);
    while (itemMenuListIterator.hasNext())
    {
        itemMenuListIterator.next();

        QString itemName = itemMenuListIterator.key();
        QMap<QString, QVariant> itemValues = itemMenuListIterator.value().toMap();
        int itemOrder = itemValues.value("Order").toInt();
        QString type = itemValues.value("Type").toString();

        SNMPTableContextMenuReference* newAction = new SNMPTableContextMenuReference();
        newAction->menuAction = new QAction(itemName, this);
        newAction->itemName = itemName;
        newAction->templateItem = itemValues;
        if (type == "Graph")
        {
            newAction->menuAction->setIcon(this->graphIcon);
            newAction->itemType = CONTEXTMENU_TYPE_GRAPH;
        }

        orderedMenuList.insert(itemOrder, newAction);
        this->menuHash.insert(newAction->menuAction, newAction);
    }

    QMapIterator<int, SNMPTableContextMenuReference*> orderedMenuListIterator(orderedMenuList);
    while (orderedMenuListIterator.hasNext())
    {
        orderedMenuListIterator.next();
        SNMPTableContextMenuReference* nextItem = orderedMenuListIterator.value();

        this->contextMenu.addAction(nextItem->menuAction);
    }
    ////////////////////////////////////////////////////////////////////////////
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
    while (iterator.hasNext())
    {
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

                //add hidden column for index/sufix
                ui->tableWidget->setItem(row, 0, new QTableWidgetItem(nextSufix));

                //add first visible column from snmpwalk
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
                ui->tableWidget->setItem(row, 1, newItem->tableItem);
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
                    ui->tableWidget->setItem(row, i+1, newItem->tableItem);
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

void SNMPTableWindow::showContextMenu(const QPoint& pos)
{
    //get selected row
    int row = ui->tableWidget->currentRow();
    if (row >= 0)
    {
        //get item information
        QString index = ui->tableWidget->item(row, 0)->text();
        QString description = ui->tableWidget->item(row, 1)->text();

        //format description texts
        QHashIterator<QAction*, SNMPTableContextMenuReference*> iterator(this->menuHash);
        while (iterator.hasNext())
        {
            iterator.next();
            SNMPTableContextMenuReference* nextItem = iterator.value();
            if (description.length() <= 50)
                nextItem->menuAction->setText(nextItem->itemName + " for \"" + description + "\"");
            else
                nextItem->menuAction->setText(nextItem->itemName + " for \"" + description.mid(0, 50) + "...\"");
        }

        //open menu
        QPoint globalPos = ui->tableWidget->mapToGlobal(pos);
        QAction* selectedItem = this->contextMenu.exec(globalPos);

        //execute selected item
        if (selectedItem)
        {
            if (this->menuHash.contains(selectedItem))
            {
                SNMPTableContextMenuReference* searchItem = this->menuHash.value(selectedItem);

                if (searchItem->itemType == CONTEXTMENU_TYPE_GRAPH)
                {
                    SNMPGraphWindow* newSNMPGraphWindow = new SNMPGraphWindow();
                    newSNMPGraphWindow->configureAndRun(searchItem->itemName + " for \"" + description + "\"",
                                                        searchItem->templateItem,
                                                        index,
                                                        this->lastHostID);
                    newSNMPGraphWindow->show();
                }
            }
        }
    }
}

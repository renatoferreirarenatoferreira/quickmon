#include "snmpgraphwindow.h"
#include "ui_snmpgraphwindow.h"

SNMPGraphWindow::SNMPGraphWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SNMPGraphWindow)
{
    ui->setupUi(this);
    //window flags
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    //drop behavior
    setAcceptDrops(true);

    //get a client instance
    this->clientInstance = SNMPClient::Instance();
    //create the timer object
    this->asyncWorker = new QTimer(this);
    connect(this->asyncWorker, SIGNAL(timeout()), this, SLOT(asyncTask()));

    //reset values
    this->resetValues();
    this->lastHostID = -1;

    //format plot
    ui->plotSNMPGraph->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->plotSNMPGraph->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->plotSNMPGraph->xAxis->setAutoTickStep(false);
}

SNMPGraphWindow::~SNMPGraphWindow()
{
    //stop tasks
    this->asyncWorker->stop();
    delete this->asyncWorker;

    //disable callback
    if (this->waitingForReply)
        SNMPClient::stopListening(this->SNMPdata);

    //clear items
    QHashIterator<QString, SNMPGraphLegendItemReference*> iterator(this->itemHash);
    while (iterator.hasNext())
    {
        iterator.next();
        SNMPGraphLegendItemReference* nextItem = iterator.value();
        ui->layoutLegendItems->removeWidget(nextItem->legendItem);
        delete nextItem->legendItem;
        delete nextItem->statistics;
        delete nextItem;
    }
    this->itemHash.clear();

    delete ui;
}

void SNMPGraphWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasText())
        if (event->mimeData()->text().startsWith(MIMEDATA_PREFIX))
            event->acceptProposedAction();
}

void SNMPGraphWindow::dropEvent(QDropEvent* event)
{
    QMetaObject::invokeMethod(this, "run", Qt::QueuedConnection,
                              Q_ARG(int, event->mimeData()->text().mid(MIMEDATA_PREFIX_LENGTH).toInt()));

    event->acceptProposedAction();
}

void SNMPGraphWindow::configure(QString templateName, QMap<QString, QVariant> templateItems, QString SNMPIndex)
{
    this->setWindowTitle("SNMP Graph: " + templateName);
    ui->labelTemplate->setText(templateName);

    this->templateName = templateName;
    this->unit = templateItems.value("Unit").toString();
    if (this->unit == "%")
        this->scale = GRAPH_SCALE_PERCENT;
    else if (this->unit == "B")
        this->scale = GRAPH_SCALE_BYTE;
    else if (this->unit == "bps")
        this->scale = GRAPH_SCALE_BPS;
    else
        this->scale = GRAPH_SCALE_OTHER;
    this->interval = templateItems.value("Interval").toInt();
    if (this->interval < 1000)
        this->interval = 1000;

    QMap<int, SNMPGraphLegendItemReference*> orderedList;

    QMap<QString, QVariant> itemList = templateItems.value("Items").toMap();
    QMapIterator<QString, QVariant> itemListIterator(itemList);
    while (itemListIterator.hasNext())
    {
        itemListIterator.next();

        QString itemName = itemListIterator.key();
        QMap<QString, QVariant> itemValues = itemListIterator.value().toMap();
        QString oid = itemValues.value("OID").toString();
        int itemOrder = itemValues.value("Order").toInt();
        QString type = itemValues.value("Type").toString();
        QString calculate = itemValues.value("Calculate").toString();

        //add index if defined
        if (SNMPIndex.length() > 0)
            oid.append(SNMPIndex);

        SNMPGraphLegendItemReference* newItem = new SNMPGraphLegendItemReference();
        newItem->legendItem = new SNMPGraphLegendItem();
        newItem->legendItem->configure(itemOrder, itemName, this->unit);
        newItem->statistics = new SNMPStatistics(this->interval/10, type, calculate);
        newItem->OID = oid;

        orderedList.insert(itemOrder, newItem);
        this->itemHash.insert(oid, newItem);
        this->OIDs.append(oid);
    }

    //add items using the configured order
    QMapIterator<int, SNMPGraphLegendItemReference*> iterator(orderedList);
    int graphIndex = 0;
    while (iterator.hasNext()) {
        iterator.next();
        ui->layoutLegendItems->addWidget(iterator.value()->legendItem);

        //format plot
        ui->plotSNMPGraph->addGraph();
        ui->plotSNMPGraph->graph(graphIndex)->setPen(QPen(iterator.value()->legendItem->getColor()));
        ui->plotSNMPGraph->graph(graphIndex)->setAntialiasedFill(false);

        //
        iterator.value()->graphIndex = graphIndex;
        graphIndex++;
    }

    //format plot
    ui->plotSNMPGraph->xAxis->setTickStep(this->interval/40);
    ui->plotSNMPGraph->yAxis->setAutoTickLabels(false);
    if (this->scale == GRAPH_SCALE_PERCENT)
    {
        ui->plotSNMPGraph->yAxis->setRangeUpper(100);
        ui->plotSNMPGraph->yAxis->setRangeLower(0);
        ui->plotSNMPGraph->yAxis->setAutoTicks(false);

        QVector<double> values;
        QVector<QString> labels;
        values << 10 << 20 << 30 << 40 << 50 << 60 << 70 << 80 << 90 << 100;
        labels << "10%" << "20%" << "30%" << "40%" << "50%" << "60%" << "70%" << "80%" << "90%" << "100%";
        ui->plotSNMPGraph->yAxis->setTickVector(values);
        ui->plotSNMPGraph->yAxis->setTickVectorLabels(labels);
    }
}

void SNMPGraphWindow::configureAndRun(QString templateName, QMap<QString, QVariant> templateItems, QString SNMPIndex, int hostID)
{
    //configure template
    this->configure(templateName, templateItems, SNMPIndex);
    //run for given host
    QMetaObject::invokeMethod(this, "run", Qt::QueuedConnection, Q_ARG(int, hostID));
}

void SNMPGraphWindow::run(int hostID)
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
            //start timer
            this->asyncWorker->start(this->interval);
        } else {
            this->setWindowTitle(this->templateName + ": Looking up for hostname " + query.value("address").toString());
        }
        //always look up for host name
        this->lookupID = QHostInfo::lookupHost(query.value("address").toString(), this, SLOT(lookupHostReply(QHostInfo)));
    }
}

void SNMPGraphWindow::lookupHostReply(QHostInfo hostInfo)
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

        //start if not started already
        if (!this->asyncWorker->isActive())
        {
            //collect data
            this->SNMPdata = this->updateValues();
            this->waitingForReply = true;
            //start timer
            this->asyncWorker->start(this->interval);
        }
    } else {
        QMessageBox::warning(this, "Hostname lookup", "Address cannot be resolved or is invalid!");
        this->setWindowTitle(this->templateName + ": Address cannot be resolved or is invalid!");
    }
}

SNMPData* SNMPGraphWindow::updateValues()
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

void SNMPGraphWindow::asyncTask()
{
    if (!this->waitingForReply && !this->retryImmediatelly)
    {
        this->waitingForReply = true;
        this->SNMPdata = this->updateValues();
    } else {
        this->retryImmediatelly = true;
    }
}

void SNMPGraphWindow::resetValues()
{
    //stop asynworker
    this->asyncWorker->stop();

    //disable callback
    if (this->waitingForReply && this->SNMPdata != NULL)
        SNMPClient::stopListening(this->SNMPdata);

    //reset values
    this->waitingForReply = false;
    this->retryImmediatelly = false;
    this->destinationAddress.setAddress("0.0.0.0");

    //clear items
    QHashIterator<QString, SNMPGraphLegendItemReference*> iterator(this->itemHash);
    while (iterator.hasNext())
    {
        iterator.next();
        SNMPGraphLegendItemReference* nextItem = iterator.value();
        //graphs
        ui->plotSNMPGraph->graph(nextItem->graphIndex)->clearData();
        //statistics
        nextItem->statistics->reset();
        //legent data
        nextItem->legendItem->reset();
    }
    ui->plotSNMPGraph->replot();
}

void SNMPGraphWindow::updateUI(int responseStatus, QString errorMessage, QList<SNMPVariable> returnVariables)
{
    //get key for X axis
    double plotKey = QDateTime::currentMSecsSinceEpoch()/1000.0;

    if (responseStatus == SNMP_RESPONSE_SUCCESS)
    {
        double upper = DBL_MIN;

        QListIterator<SNMPVariable> iterator(returnVariables);
        while (iterator.hasNext())
        {
            SNMPVariable nextVariable = iterator.next();

            if (this->itemHash.contains(nextVariable.OID))
            {
                SNMPGraphLegendItemReference* searchItem = this->itemHash.value(nextVariable.OID);

                //calculate next value/
                searchItem->statistics->addResponse(nextVariable);

                if (searchItem->statistics->isValid())
                {
                    //update legend table
                    searchItem->legendItem->updateLegend(searchItem->statistics->lastValue(),
                                                         searchItem->statistics->averageValue(),
                                                         searchItem->statistics->maxValue(),
                                                         searchItem->statistics->minValue());
                    //update graph
                    ui->plotSNMPGraph->graph(searchItem->graphIndex)->addData(plotKey, searchItem->statistics->lastValue());
                    //get max value for y axis scale
                    if (this->scale != GRAPH_SCALE_PERCENT && upper < searchItem->statistics->maxValue())
                        upper = searchItem->statistics->maxValue();
                } else {
                    //add blank value to graph
                    ui->plotSNMPGraph->graph(searchItem->graphIndex)->addData(plotKey, std::numeric_limits<double>::quiet_NaN());
                }

                //clear old data
                ui->plotSNMPGraph->graph(searchItem->graphIndex)->removeDataBefore(plotKey-((this->interval/10)*1.1));
            }
        }

        //calculate scale
        if (this->scale == GRAPH_SCALE_PERCENT)
        {
            //fixed scale for percent
            ui->plotSNMPGraph->yAxis->setRangeUpper(100);
            ui->plotSNMPGraph->yAxis->setRangeLower(0);
        } else if (upper != DBL_MIN) {
            //dynamic scale
            ui->plotSNMPGraph->yAxis->setRangeUpper((double)upper*1.1);
            ui->plotSNMPGraph->yAxis->setRangeLower(0);

            //calculate simplifiers
            quint64 scaleDivisor = 1;
            QString scaleUnit;
            if (this->scale == GRAPH_SCALE_BYTE)
            {
                if (upper >= 1073741824000)
                {
                    scaleDivisor = 1073741824000;
                    scaleUnit = 'T';
                }
                else if (upper >= 1073741824)
                {
                    scaleDivisor = 1073741824;
                    scaleUnit = 'G';
                }
                else if (upper >= 1048576)
                {
                    scaleDivisor = 1048576;
                    scaleUnit = 'M';
                }
                else if (upper >= 1024)
                {
                    scaleDivisor = 1024;
                    scaleUnit = 'K';
                }
            } else {
                if (upper >= 1000000000000)
                {
                    scaleDivisor = 1000000000000;
                    scaleUnit = 'T';
                }
                else if (upper >= 1000000000)
                {
                    scaleDivisor = 1000000000;
                    scaleUnit = (this->scale == GRAPH_SCALE_BPS?'G':'B'); //use Giga or Billions
                }
                else if (upper >= 1000000)
                {
                    scaleDivisor = 1000000;
                    scaleUnit = 'M';
                }
                else if (upper >= 1000)
                {
                    scaleDivisor = 1000;
                    scaleUnit = 'K';
                }
            }

            //rewrite ticks
            QVector<double> ticks = ui->plotSNMPGraph->yAxis->tickVector();
            QVector<QString> labels = ui->plotSNMPGraph->yAxis->tickVectorLabels();
            for (int i = 0; i < ticks.size(); ++i)
            {
                if ( ticks[i] == 0 )
                    labels[i] = "";
                else
                    labels[i] = QString::number((double)(ticks[i]/scaleDivisor), 'f', 0).append(scaleUnit).append(this->unit);
            }
            ui->plotSNMPGraph->yAxis->setTickVectorLabels(labels);
        }
    } else {
        //set invalid values for all items
        QHashIterator<QString, SNMPGraphLegendItemReference*> iterator(this->itemHash);
        while (iterator.hasNext())
        {
            iterator.next();
            SNMPGraphLegendItemReference* nextItem = iterator.value();
            //add timeout
            nextItem->statistics->addTimeout();
            //add blank value to graph
            ui->plotSNMPGraph->graph(nextItem->graphIndex)->addData(plotKey, std::numeric_limits<double>::quiet_NaN());
        }
    }
    ui->plotSNMPGraph->xAxis->setRange(plotKey+0.25, this->interval/10, Qt::AlignRight);
    ui->plotSNMPGraph->replot();

    //stop on critical errors
    if (responseStatus == SNMP_RESPONSE_ERROR)
    {
        //stop collecting data
        this->resetValues();
        //permit retry
        this->lastHostID = -1;
        //show error
        QString formattedError = "Critical error collecting SNMP Mib data! (";
        formattedError.append(errorMessage).append(")");
        QMessageBox::warning(this, "SNMP error", formattedError);
    }
}

void SNMPGraphWindow::receiveSNMPReply(SNMPData* data)
{
    //ignore delayed responses
    if (data->requestAddress != this->destinationAddress)
        return;

    //update ui
    //ToDo: clone data if needed
    QMetaObject::invokeMethod(this, "updateUI", Qt::QueuedConnection,
                              Q_ARG(int, data->responseStatus),
                              Q_ARG(QString, data->errorMessage),
                              Q_ARG(QList<SNMPVariable>, data->returnVariables));

    //retry immetdiatelly if this reply took too long to arrive
    if (this->retryImmediatelly)
    {
        this->SNMPdata = this->updateValues();
        this->retryImmediatelly = false;
    } else {
        this->waitingForReply = false;
    }

#ifdef QT_DEBUG
    qDebug() << "Status:" << data->errorMessage;
    QListIterator<SNMPVariable> iterator(data->returnVariables);
    while (iterator.hasNext())
    {
        SNMPVariable nextVariable = iterator.next();
        qDebug() << "  OID:" << nextVariable.OID << "Value:" << nextVariable.variantValue;
    }
#endif
}

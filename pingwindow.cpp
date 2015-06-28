#include "pingwindow.h"
#include "ui_pingwindow.h"

PingWindow::PingWindow(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::PingWindow)
{
    ui->setupUi(this);
    //window flags
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    //drop behavior
    setAcceptDrops(true);

    //get a pinger instance
    pingerInstance = Pinger::Instance();
    //create the timer object
    this->asyncWorker = new QTimer(this);
    connect(this->asyncWorker, SIGNAL(timeout()), this, SLOT(asyncTask()));

    //format plot
    ui->plotLatency->addGraph();
    ui->plotLatency->graph(0)->setPen(QPen(Qt::darkGreen));
    ui->plotLatency->graph(0)->setBrush(QBrush(QColor(230, 230, 230)));
    ui->plotLatency->graph(0)->setAntialiasedFill(false);
    //
    ui->plotLatency->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->plotLatency->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->plotLatency->xAxis->setAutoTickStep(false);
    ui->plotLatency->xAxis->setTickStep(25);

    //reset values
    this->resetValues();
}

PingWindow::~PingWindow()
{
    //stop tasks
    this->asyncWorker->stop();
    delete this->asyncWorker;

    //disable callback
    if (this->pigingContext != NULL)
        this->pigingContext->listener = NULL;

    delete ui;
}

void PingWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasText())
        if (event->mimeData()->text().startsWith(MIMEDATA_PREFIX))
            event->acceptProposedAction();
}

void PingWindow::dropEvent(QDropEvent* event)
{
    QMetaObject::invokeMethod(this, "run", Qt::QueuedConnection,
                              Q_ARG(int, event->mimeData()->text().mid(MIMEDATA_PREFIX_LENGTH).toInt()));

    event->acceptProposedAction();
}

void PingWindow::run(int hostID)
{
    this->asyncWorker->stop();
    //reset values
    this->resetValues();

    QSqlQuery query = LocalData::Instance()->createQuery("SELECT * FROM hosts WHERE ID=:ID");
    query.bindValue(":ID", hostID);
    LocalData::Instance()->executeQuery(query);

    if (query.next())
    {
        ui->labelName->setText(query.value("name").toString());
        ui->labelAddress->setText(query.value("address").toString());

        //start immediately if adress is valid IPv4/6
        this->pingingAddress.setAddress(query.value("address").toString());
        if (this->pingingAddress.protocol() == QAbstractSocket::IPv4Protocol ||
            this->pingingAddress.protocol() == QAbstractSocket::IPv6Protocol)
        {
            latencyCalculator.reset();
            this->waitingForReply = true;
            this->pigingContext = Pinger::Instance()->ping(this->pingingAddress, this);
            this->asyncWorker->start(1000);
            this->setWindowTitle("Pinging " + query.value("address").toString());
        } else {
            this->setWindowTitle("Ping: Looking up for hostname " + query.value("address").toString());
        }
        //always look up for host name
        QHostInfo::lookupHost(query.value("address").toString(), this, SLOT(lookupHostReply(QHostInfo)));
    }
}

void PingWindow::lookupHostReply(QHostInfo hostInfo)
{
    if (!hostInfo.addresses().isEmpty())
    {
        //update address label
        this->pingingAddress = hostInfo.addresses().first();
        ui->labelAddress->setText(this->pingingAddress.toString()+" ("+hostInfo.hostName()+")");
        this->setWindowTitle("Pinging " + this->pingingAddress.toString()+" ("+hostInfo.hostName()+")");

        //start pinging if not started already
        if (!this->asyncWorker->isActive())
        {
            latencyCalculator.reset();
            this->waitingForReply = true;
            this->pigingContext = Pinger::Instance()->ping(this->pingingAddress, this);
            this->asyncWorker->start(1000);
        }
    } else {
        QMessageBox::warning(this, "Hostname lookup", "Address cannot be resolved or is invalid!");
        this->setWindowTitle("Ping: Address cannot be resolved or is invalid!");
    }
}

void PingWindow::receivePingReply(PingContext* context)
{
    //ignore delayed responses
    if (context->requestAddress != this->pingingAddress)
        return;

    //update ui
    QMetaObject::invokeMethod(this, "updateUI", Qt::QueuedConnection,
                              Q_ARG(int, context->replyStatus),
                              Q_ARG(double, context->millisLatencyHighResolution));

    //reset context reference
    this->pigingContext = NULL;

    //retry immetdiatelly if this reply took too long to arrive
    if (this->retryImmediatelly)
    {
        this->pigingContext = Pinger::Instance()->ping(this->pingingAddress, this);
        this->retryImmediatelly = false;
    } else {
        this->waitingForReply = false;
    }

#ifdef QT_DEBUG
    if (context->replyStatus == PINGER_STATUS_SUCCESS)
        qDebug() << "Reply from " << context->replyAddress.toString() << " time(HR/LR)=" << context->millisLatencyHighResolution << "/" << context->millisLatencyLowResolution;
    else
        qDebug() << "Timeout!";
    qDebug() << "     LastLatency=" << latencyCalculator.lastLatency() << " AvgLatency=" << latencyCalculator.averageLatency() << " AvgJitter=" << latencyCalculator.averageJitter() << " PctJitter=" << latencyCalculator.percentJitter() << " PctLoss=" << latencyCalculator.percentLoss() << "%";
#endif
}

void PingWindow::asyncTask()
{
    if (!this->waitingForReply && !this->retryImmediatelly)
    {
        this->waitingForReply = true;
        this->pigingContext = Pinger::Instance()->ping(this->pingingAddress, this);
    } else {
        this->retryImmediatelly = true;
    }
}

void PingWindow::resetValues()
{
    this->pigingContext = NULL;
    ui->labelLastRTT->setText("0.0 ms");
    ui->labelAvgRTT->setText("0.0 ms");
    ui->labelAvgJitter->setText("0.0 ms");
    ui->labelPctLoss->setText("0.0 %");
    ui->labelPctLoss->setStyleSheet(STYLE_FIELD_NORMAL);
    ui->labelPctLossLabel->setStyleSheet(STYLE_LABEL_NORMAL);
    ui->plotLatency->graph(0)->clearData();
}

void PingWindow::updateUI(int replyStatus, double latency)
{
    if (replyStatus == PINGER_STATUS_SUCCESS)
    {
        //calculate statistics
        latencyCalculator.addResponse(latency);

        //update data
        ui->labelLastRTT->setText(QString::number(latencyCalculator.lastLatency(),'f',2)+" ms");
        ui->labelLastRTT->setStyleSheet("font: bold 18px;\ncolor: rgb(0, 0, 0);");
        ui->labelLastRTTLabel->setStyleSheet(STYLE_LABEL_NORMAL);
        ui->labelAvgRTT->setText(QString::number(latencyCalculator.averageLatency(),'f',2)+" ms");
        ui->labelAvgJitter->setText(QString::number(latencyCalculator.averageJitter(),'f',2)+" ms");
        //format jitter
        if (latencyCalculator.percentJitter() >= JITTER_PERCENT_RED)
        {
            ui->labelAvgJitter->setStyleSheet(STYLE_FIELD_RED);
            ui->labelAvgJitterLabel->setStyleSheet(STYLE_LABEL_RED);
        }
        else if (latencyCalculator.percentJitter() >= JITTER_PERCENT_YELLOW)
        {
            ui->labelAvgJitter->setStyleSheet(STYLE_FIELD_YELLOW);
            ui->labelAvgJitterLabel->setStyleSheet(STYLE_LABEL_YELLOW);
        }
        else
        {
            ui->labelAvgJitter->setStyleSheet(STYLE_FIELD_NORMAL);
            ui->labelAvgJitterLabel->setStyleSheet(STYLE_LABEL_NORMAL);
        }

        //update graph
        this->updateGraph(latencyCalculator.lastLatency());
    }
    else
    {
        //calculate statistics
        latencyCalculator.addTimeout();

        //update data
        if (replyStatus == PINGER_STATUS_TIMEOUT)
            ui->labelLastRTT->setText("Timeout!");
        else
            ui->labelLastRTT->setText("Error!");
        ui->labelLastRTT->setStyleSheet("font: bold 18px;\ncolor: rgb(153, 0, 0);\nbackground-color: rgb(255, 166, 166);");
        ui->labelLastRTTLabel->setStyleSheet(STYLE_LABEL_RED);

        //update graph
        this->updateGraph(std::numeric_limits<double>::quiet_NaN());
    }

    //set packet loss
    ui->labelPctLoss->setText(QString::number(latencyCalculator.percentLoss(),'f',2)+" %");
    //format packet loss
    if (latencyCalculator.percentLoss() >= LOSS_PERCENT_RED)
    {
        ui->labelPctLoss->setStyleSheet(STYLE_FIELD_RED);
        ui->labelPctLossLabel->setStyleSheet(STYLE_LABEL_RED);
    }
    else if (latencyCalculator.percentLoss() >= LOSS_PERCENT_YELLOW)
    {
        ui->labelPctLoss->setStyleSheet(STYLE_FIELD_YELLOW);
        ui->labelPctLossLabel->setStyleSheet(STYLE_LABEL_YELLOW);
    }
    else
    {
        ui->labelPctLoss->setStyleSheet(STYLE_FIELD_NORMAL);
        ui->labelPctLossLabel->setStyleSheet(STYLE_LABEL_NORMAL);
    }
}

void PingWindow::updateGraph(double lastLatency)
{
    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    ui->plotLatency->graph(0)->addData(key, lastLatency);
    ui->plotLatency->graph(0)->removeDataBefore(key-110);
    ui->plotLatency->graph(0)->rescaleValueAxis();
    ui->plotLatency->yAxis->setRangeUpper((double)ui->plotLatency->yAxis->range().upper*1.1);
    ui->plotLatency->yAxis->setRangeLower(0);
    ui->plotLatency->xAxis->setRange(key+0.25, 100, Qt::AlignRight);
    ui->plotLatency->replot();
}

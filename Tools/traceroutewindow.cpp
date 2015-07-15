#include "traceroutewindow.h"
#include "ui_traceroutewindow.h"

TracerouteWindow::TracerouteWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TracerouteWindow)
{
    ui->setupUi(this);
    //window flags
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    //drop behavior
    setAcceptDrops(true);

    //get a pinger instance
    this->pingerInstance = Pinger::Instance();
    //create the timer object
    this->asyncWorker = new QTimer(this);
    connect(this->asyncWorker, SIGNAL(timeout()), this, SLOT(asyncTask()));
    //configure manager
    this->itemManager = new TracerouteItemManager(ui->layoutTracerouteItems);

    //reset values
    this->resetValues();
    this->lastHostID = -1;
}

TracerouteWindow::~TracerouteWindow()
{
    //stop tasks
    this->asyncWorker->stop();
    delete this->asyncWorker;

    //delete managet
    delete this->itemManager;

    //disable callback
    if (this->waitingForReply)
        Pinger::stopListening(this->pigingContext);

    delete ui;
}

void TracerouteWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasText())
        if (event->mimeData()->text().startsWith(MIMEDATA_PREFIX))
            event->acceptProposedAction();
}

void TracerouteWindow::dropEvent(QDropEvent* event)
{
    QMetaObject::invokeMethod(this, "run", Qt::QueuedConnection,
                              Q_ARG(int, event->mimeData()->text().mid(MIMEDATA_PREFIX_LENGTH).toInt()));

    event->acceptProposedAction();
}

void TracerouteWindow::run(int hostID)
{
    //avoid re-running for the same host
    if (this->lastHostID == hostID)
        return;
    else
        this->lastHostID = hostID;

    this->asyncWorker->stop();
    //reset tasks
    this->resetValues();

    QSqlQuery query = LocalData::Instance()->createQuery("SELECT * FROM hosts WHERE ID=:ID");
    query.bindValue(":ID", hostID);
    LocalData::Instance()->executeQuery(query);

    if (query.next())
    {
        ui->labelName->setText(query.value("name").toString());
        ui->labelAddress->setText(query.value("address").toString());

        //start immediately if adress is valid IPv4/6
        this->destinationAddress.setAddress(query.value("address").toString());
        if (this->destinationAddress.protocol() == QAbstractSocket::IPv4Protocol ||
            this->destinationAddress.protocol() == QAbstractSocket::IPv6Protocol)
        {
            //update title
            this->setWindowTitle("Traceroute " + query.value("address").toString());
            //start tracing
            this->waitingForReply = true;
            this->pigingContext = this->pingerInstance->ping(this->destinationAddress, this, this->TTL);
            this->asyncWorker->start(1000);
        } else {
            this->setWindowTitle("Traceroute: Looking up for hostname " + query.value("address").toString());
        }
        //always look up for host name
        this->lookupID = QHostInfo::lookupHost(query.value("address").toString(), this, SLOT(lookupHostReply(QHostInfo)));
    }
}

void TracerouteWindow::lookupHostReply(QHostInfo hostInfo)
{
    //avoid lookup overlapping
    if (this->lookupID != hostInfo.lookupId())
        return;

    if (!hostInfo.addresses().isEmpty())
    {
        //update address label
        this->destinationAddress = hostInfo.addresses().first();
        ui->labelAddress->setText(this->destinationAddress.toString()+" ("+hostInfo.hostName()+")");
        this->setWindowTitle("Traceroute " + this->destinationAddress.toString()+" ("+hostInfo.hostName()+")");

        //start pinging if not started already
        if (!this->asyncWorker->isActive() && this->running)
        {
            //start tracing
            this->waitingForReply = true;
            this->pigingContext = this->pingerInstance->ping(this->destinationAddress, this, this->TTL);
            this->asyncWorker->start(1000);
        }
    } else {
        QMessageBox::warning(this, "Hostname lookup", "Address cannot be resolved or is invalid!");
        this->setWindowTitle("Traceroute: Address cannot be resolved or is invalid!");
    }
}

void TracerouteWindow::resetValues()
{
    this->TTL = 1;
    this->running = true;
    this->waitingForReply = false;
    this->destinationAddress.setAddress("0.0.0.0");

    //clear items
    this->itemManager->clear();
    //reset size
    QTimer::singleShot(0, this, SLOT(shrink()));
}

void TracerouteWindow::shrink()
{
    resize(0, 0);
}

void TracerouteWindow::receivePingReply(PingContext* context)
{
    //ignore delayed responses
    if (context->requestAddress != this->destinationAddress)
        return;

    //update ui
    QMetaObject::invokeMethod(this->itemManager, "addItem", Qt::QueuedConnection,
                              Q_ARG(int, context->replyStatus),
                              Q_ARG(int, this->TTL),
                              Q_ARG(QString, context->replyAddress.toString()),
                              Q_ARG(double, context->millisLatencyHighResolution));

    //disable waiting flag
    this->waitingForReply = false;

#ifdef QT_DEBUG
    if (context->replyStatus == PINGER_STATUS_EXPIREDINTRANSIT || context->replyStatus == PINGER_STATUS_SUCCESS)
        qDebug() << "Hop:" << this->TTL << "Request:" << context->requestAddress.toString() << "Reply:"<< context->replyAddress.toString() << "RTT:" << context->millisLatencyHighResolution;
    else
        qDebug() << "Hop:" << this->TTL << "Request:" << context->requestAddress.toString() << "Reply:"<< TRACEROUTE_TIMEOUT_ADDRESS << "RTT: Timeout!";
#endif

    if (context->replyStatus == PINGER_STATUS_SUCCESS || this->TTL >= 255)
    {
        this->running = false;
        QMetaObject::invokeMethod(this->asyncWorker, "stop", Qt::QueuedConnection);
    }
}

void TracerouteWindow::asyncTask()
{
    if (this->running)
    {
        if (!this->waitingForReply)
        {
            this->waitingForReply = true;
            this->TTL++;
            this->pigingContext = this->pingerInstance->ping(this->destinationAddress, this, this->TTL);
        }
    } else
        this->asyncWorker->stop();
}

#include "pingwindow.h"
#include "ui_pingwindow.h"

PingWindow::PingWindow(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::PingWindow)
{
    ui->setupUi(this);
    //window flags
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
    //drop behavior
    setAcceptDrops(true);

    //get a pinger instance
    pingerInstance = Pinger::Instance();
    //create the timer object
    this->asyncWorker = new QTimer(this);
    connect(this->asyncWorker, SIGNAL(timeout()), this, SLOT(asyncTask()));
}

PingWindow::~PingWindow()
{
    delete this->asyncWorker;

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

    QSqlQuery query = LocalData::Instance()->createQuery("SELECT * FROM hosts WHERE ID=:ID");
    query.bindValue(":ID", hostID);
    LocalData::Instance()->executeQuery(query);

    if (query.next())
    {
        ui->label_Name->setText(query.value("name").toString());
        ui->label_Address->setText(query.value("address").toString());
        QHostInfo::lookupHost(query.value("address").toString(), this, SLOT(lookupHostReply(QHostInfo)));
    }
}

void PingWindow::lookupHostReply(QHostInfo hostInfo)
{
    if (!hostInfo.addresses().isEmpty())
    {
        this->pingingAddress = hostInfo.addresses().first();

        ui->label_Address->setText(this->pingingAddress.toString()+" ("+hostInfo.hostName()+")");
        Pinger::Instance()->ping(this->pingingAddress, this);
        this->asyncWorker->start(1000);
    } else {
        QMessageBox::warning(this, "Hostname lookup", "Address cannot be resolved or is invalid!");
    }
}

void PingWindow::receivePingReply(PingContext* context)
{
    if (context->replyStatus == PINGER_STATUS_SUCCESS)
        qDebug() << "Reply from " << context->replyAddress.toString() << " time(HR/LR)=" << context->millisLatencyHighResolution << "/" << context->millisLatencyLowResolution;
}

void PingWindow::asyncTask()
{
    Pinger::Instance()->ping(this->pingingAddress, this);
}

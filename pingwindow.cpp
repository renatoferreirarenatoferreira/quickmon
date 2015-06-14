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
}

PingWindow::~PingWindow()
{
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
    QSqlQuery query = LocalData::Instance()->createQuery("SELECT * FROM hosts WHERE ID=:ID");
    query.bindValue(":ID", hostID);
    LocalData::Instance()->executeQuery(query);

    if (query.next())
    {
        ui->label_Name->setText(query.value("name").toString());
        ui->label_Address->setText(query.value("address").toString());

        hostInfo = QHostInfo::fromName(query.value("address").toString());
        if (!hostInfo.addresses().isEmpty())
        {
            ui->label_Address->setText(hostInfo.addresses().first().toString()+" ("+hostInfo.hostName()+")");
        } else {
            QMessageBox::warning(this, "Hostname lookup", "Address cannot be resolved or is invalid!");
        }
    }
}

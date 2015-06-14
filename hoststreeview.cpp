#include "hoststreeview.h"

HostsTreeView::HostsTreeView(QWidget* parent = 0) : QTreeView(parent)
{
    //initialize data
    QSqlQuery hostsQuery = LocalData::Instance()->createQuery("SELECT id,name,address,CASE WHEN snmpversion=0 THEN 'None' WHEN snmpversion=1 THEN 'v1' WHEN snmpversion=2 THEN 'v2c' WHEN snmpversion=3 THEN 'v3' END as snmpversiontranslated FROM hosts ORDER BY name, address, snmpversion");
    LocalData::Instance()->executeQuery(hostsQuery);
    //configure model
    hostsModel.setQuery(hostsQuery);
    hostsModel.setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    hostsModel.setHeaderData(1, Qt::Horizontal, QObject::tr("Name"));
    hostsModel.setHeaderData(2, Qt::Horizontal, QObject::tr("Address"));
    hostsModel.setHeaderData(3, Qt::Horizontal, QObject::tr("SNMP"));
    //proxy model
    proxyHostsModel = new QSortFilterProxyModel();
    proxyHostsModel->setSourceModel(&hostsModel);
    //configure treeview
    this->setModel(proxyHostsModel);
    this->setColumnHidden(0, true);
    this->setColumnWidth(1, 130);
    this->setColumnWidth(2, 80);
    this->setColumnWidth(3, 20);
    this->sortByColumn(1, Qt::SortOrder::AscendingOrder);
    //select first
    this->setCurrentIndex(proxyHostsModel->index(0, 0, QModelIndex()));

    //set hosteditor event
    connect(&hostEditor_Window, SIGNAL(accepted()), this, SLOT(hostEditor_Accepted()));
}

HostsTreeView::~HostsTreeView()
{

}

void HostsTreeView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();

    QTreeView::mousePressEvent(event);
}

void HostsTreeView::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength()
         < QApplication::startDragDistance())
        return;

    int hostID = hostsModel.record(proxyHostsModel->mapToSource(this->currentIndex()).row()).value("id").toInt();
    QString mimeText;
    QTextStream(&mimeText) << MIMEDATA_PREFIX << hostID;

    QDrag *drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData;
    mimeData->setText(mimeText);
    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction | Qt::MoveAction);
}

void HostsTreeView::updateHosts()
{
    QSqlQuery hostsQuery = LocalData::Instance()->createQuery("SELECT id,name,address,CASE WHEN snmpversion=0 THEN 'None' WHEN snmpversion=1 THEN 'v1' WHEN snmpversion=2 THEN 'v2c' WHEN snmpversion=3 THEN 'v3' END as snmpversiontranslated FROM hosts ORDER BY name, address, snmpversion");
    LocalData::Instance()->executeQuery(hostsQuery);
    hostsModel.setQuery(hostsQuery);
    this->setCurrentIndex(proxyHostsModel->index(0, 0, QModelIndex()));
}

void HostsTreeView::addHost()
{
    hostEditor_Window.addHost();
}

void HostsTreeView::editHost()
{
    int hostID = hostsModel.record(proxyHostsModel->mapToSource(this->currentIndex()).row()).value("id").toInt();
    hostEditor_Window.editHost(hostID);
}

void HostsTreeView::removeHost()
{
    //get data from database
    int hostID = hostsModel.record(proxyHostsModel->mapToSource(this->currentIndex()).row()).value("id").toInt();
    QSqlQuery query = LocalData::Instance()->createQuery("SELECT name, address FROM hosts WHERE ID=:ID");
    query.bindValue(":ID", hostID);
    LocalData::Instance()->executeQuery(query);

    if (query.next())
    {
        QString question = "Delete host \"" + query.value("name").toString() + "\" with address \"" + query.value("address").toString() + "\"?";
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm deletion", question, QMessageBox::Yes|QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            query = LocalData::Instance()->createQuery("DELETE FROM hosts WHERE id=:ID");
            query.bindValue(":ID", hostID);
            LocalData::Instance()->executeQuery(query);

            updateHosts();
        }
    }
}

void HostsTreeView::hostEditor_Accepted()
{
    this->updateHosts();
}

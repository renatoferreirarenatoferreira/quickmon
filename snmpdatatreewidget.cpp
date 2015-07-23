#include "snmpdatatreewidget.h"

SNMPDataTreeWidget::SNMPDataTreeWidget(QWidget* parent = 0) : QTreeWidget(parent)
{
    //async read templates
    QTimer::singleShot(100, this, SLOT(loadTemplates()));
}

SNMPDataTreeWidget::~SNMPDataTreeWidget()
{
    while (!this->internalItems.isEmpty())
        delete this->internalItems.takeFirst();

    while (!this->topLevelItems.isEmpty())
        delete this->topLevelItems.takeFirst();

    this->internalItems.clear();
    this->topLevelItems.clear();
    this->internalItemsHash.clear();
}

void SNMPDataTreeWidget::itemStateChanged(QTreeWidgetItem* item)
{
    if (item->isExpanded())
        item->setIcon(0, this->openIcon);
    else
        item->setIcon(0, this->closeIcon);
}

void SNMPDataTreeWidget::loadTemplates()
{
    //configure widget
    this->setColumnCount(1);
    //create icons
    this->tableIcon.addFile(QStringLiteral(":/Icons/SNMPTable"), QSize(), QIcon::Normal, QIcon::Off);
    this->listIcon.addFile(QStringLiteral(":/Icons/SNMPList"), QSize(), QIcon::Normal, QIcon::Off);
    this->graphIcon.addFile(QStringLiteral(":/Icons/SNMPGraph"), QSize(), QIcon::Normal, QIcon::Off);
    this->openIcon.addFile(QStringLiteral(":/Icons/FolderOpen"), QSize(), QIcon::Normal, QIcon::Off);
    this->closeIcon.addFile(QStringLiteral(":/Icons/FolderClose"), QSize(), QIcon::Normal, QIcon::Off);
    //connect events
    connect(this, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(itemStateChanged(QTreeWidgetItem*)));
    connect(this, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(itemStateChanged(QTreeWidgetItem*)));
    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(itemDoubleClicked(QTreeWidgetItem*, int)));

    //open embbeded file
    QFile templateListFile(":/SNMPTemplates/List");
    if (!templateListFile.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
    //parse json
    QJsonDocument templateListJsonDocument = QJsonDocument::fromJson(templateListFile.readAll());
    //process items
    QMapIterator<QString, QVariant> templateListIterator(templateListJsonDocument.object().toVariantMap());
    while (templateListIterator.hasNext()) {
        templateListIterator.next();

        //add main top level item
        QTreeWidgetItem* topLevelItem = new QTreeWidgetItem();
        topLevelItem->setText(0, templateListIterator.key());
        topLevelItem->setIcon(0, this->closeIcon);
        this->topLevelItems.append(topLevelItem);

        //add aditional files
        QFile templateFile(templateListIterator.value().toString());
        if (!templateFile.open(QIODevice::ReadOnly | QIODevice::Text))
                continue;
        QJsonDocument templateJsonDocument = QJsonDocument::fromJson(templateFile.readAll());
        QMapIterator<QString, QVariant> templateIterator(templateJsonDocument.object().toVariantMap());
        while (templateIterator.hasNext())
        {
            templateIterator.next();

            //get values
            QString templateName = templateIterator.key();
            QMap<QString, QVariant> templateItems = templateIterator.value().toMap();
            QString templateType = templateItems.value("Type").toString();

            //add main top level item
            QTreeWidgetItem* internalItem = new QTreeWidgetItem();
            internalItem->setText(0, templateName);
            topLevelItem->addChild(internalItem);
            this->internalItems.append(internalItem);
            this->internalItemsHash.insert(internalItem, templateItems);

            //select icon
            if (templateType == "List")
                internalItem->setIcon(0, this->listIcon);
            else if (templateType == "Table")
                internalItem->setIcon(0, this->tableIcon);
            else if (templateType == "Graph")
                internalItem->setIcon(0, this->graphIcon);
        }
    }

    this->addTopLevelItems(this->topLevelItems);
}

void SNMPDataTreeWidget::itemDoubleClicked(QTreeWidgetItem* item, int)
{
    if (!this->internalItemsHash.contains(item))
        return;

    QMap<QString, QVariant> templateItems = this->internalItemsHash.value(item);
    QString templateType = templateItems.value("Type").toString();

    if (templateType == "Graph")
    {
        SNMPGraphWindow* newSNMPGraphWindow = new SNMPGraphWindow();
        newSNMPGraphWindow->configure(item->text(0), templateItems);
        newSNMPGraphWindow->show();
    } else if (templateType == "List")
    {
        SNMPListWindow* newSNMPListWindow = new SNMPListWindow();
        newSNMPListWindow->configure(item->text(0), templateItems);
        newSNMPListWindow->show();
    } else if (templateType == "Table")
    {
        SNMPTableWindow* newSNMPTableWindow = new SNMPTableWindow();
        newSNMPTableWindow->configure(item->text(0), templateItems);
        newSNMPTableWindow->show();
    }
}

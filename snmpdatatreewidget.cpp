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

    //ordered list/map
    QMap<int, QStringList> externalOrderedList;

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

        //populate the ordered list for external items
        QMap<QString, QVariant> templateItems = templateListIterator.value().toMap();
        QStringList templateData;
        templateData.append(templateListIterator.key());

        if (templateItems.value("File").toString().startsWith(":"))
            //read file from resources
            templateData.append(templateItems.value("File").toString());
        else
            //read file from application path
#ifdef Q_OS_WIN32
            templateData.append(QCoreApplication::applicationDirPath() + "/" + templateItems.value("File").toString());
#else
            templateData.append("~/." + templateItems.value("File").toString());
#endif

        externalOrderedList.insert(templateItems.value("Order").toInt(), templateData);
    }

    //add external items using the configured order
    QMapIterator<int, QStringList> externalIterator(externalOrderedList);
    while (externalIterator.hasNext()) {
        externalIterator.next();

        //add main top level item
        QTreeWidgetItem* topLevelItem = new QTreeWidgetItem();
        topLevelItem->setText(0, externalIterator.value().first());
        topLevelItem->setIcon(0, this->closeIcon);
        this->topLevelItems.append(topLevelItem);

        //ordered list/map
        QMap<int, QTreeWidgetItem*> internalOrderedList;

        //add aditional files
        QFile templateFile(externalIterator.value().at(1));
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
            int internalItemOrder = templateItems.value("Order").toInt();

            //add main top level item
            QTreeWidgetItem* internalItem = new QTreeWidgetItem();
            internalItem->setText(0, templateName);
            this->internalItems.append(internalItem);
            this->internalItemsHash.insert(internalItem, templateItems);
            //add items in order
            internalOrderedList.insert(internalItemOrder, internalItem);

            //select icon
            if (templateType == "List")
                internalItem->setIcon(0, this->listIcon);
            else if (templateType == "Table")
                internalItem->setIcon(0, this->tableIcon);
            else if (templateType == "Graph")
                internalItem->setIcon(0, this->graphIcon);
        }

        //add items using the configured order
        QMapIterator<int, QTreeWidgetItem*> iterator(internalOrderedList);
        while (iterator.hasNext()) {
            iterator.next();
            topLevelItem->addChild(iterator.value());
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
        newSNMPGraphWindow->configure(item->parent()->text(0) + " / " + item->text(0), templateItems);
        newSNMPGraphWindow->show();
    } else if (templateType == "List")
    {
        SNMPListWindow* newSNMPListWindow = new SNMPListWindow();
        newSNMPListWindow->configure(item->parent()->text(0) + " / " + item->text(0), templateItems);
        newSNMPListWindow->show();
    } else if (templateType == "Table")
    {
        SNMPTableWindow* newSNMPTableWindow = new SNMPTableWindow();
        newSNMPTableWindow->configure(item->parent()->text(0) + " / " + item->text(0), templateItems);
        newSNMPTableWindow->show();
    }
}

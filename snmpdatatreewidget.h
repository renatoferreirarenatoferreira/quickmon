#ifndef SNMPDATATREEWIDGET_H
#define SNMPDATATREEWIDGET_H

#include <QTreeWidget>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

#include "Tools/snmplistwindow.h"
#include "Tools/snmptablewindow.h"
#include "Tools/snmpgraphwindow.h"

class SNMPDataTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    SNMPDataTreeWidget(QWidget* parent);
    ~SNMPDataTreeWidget();

private slots:
    void loadTemplates();
    void itemStateChanged(QTreeWidgetItem* item);
    void itemDoubleClicked(QTreeWidgetItem* item, int);

private:
    QIcon openIcon;
    QIcon closeIcon;
    QIcon tableIcon;
    QIcon listIcon;
    QIcon graphIcon;
    QList<QTreeWidgetItem*> topLevelItems;
    QList<QTreeWidgetItem*> internalItems;
    QHash<QTreeWidgetItem*, QVariantMap> internalItemsHash;
};

#endif // SNMPDATATREEWIDGET_H

#ifndef HOSTSTREEVIEW_H
#define HOSTSTREEVIEW_H

#include <QSqlQueryModel>
#include <QMessageBox>
#include <QTreeView>
#include <QMouseEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>

#include "localdata.h"
#include "dragablesqlquerymodel.h"
#include "hosteditor.h"

#define MIMEDATA_PREFIX "quickmon/host:"
#define MIMEDATA_PREFIX_LENGTH (int)strlen(MIMEDATA_PREFIX)

class HostsTreeView : public QTreeView
{
    Q_OBJECT

public:
    HostsTreeView(QWidget* parent);
    ~HostsTreeView();
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent *event);
    void addHost();
    void updateHosts();
    void editHost();
    void removeHost();

private slots:
    void hostEditor_Accepted();

private:
    QPoint dragStartPosition;
    DragableSqlQueryModel hostsModel;
    QSortFilterProxyModel *proxyHostsModel;
    HostEditor hostEditor_Window;
};

#endif // HOSTSTREEVIEW_H

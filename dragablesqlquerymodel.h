#ifndef DRAGABLESQLQUERYMODEL_H
#define DRAGABLESQLQUERYMODEL_H

#include <QAbstractListModel>
#include <QSqlQueryModel>

class DragableSqlQueryModel : public QSqlQueryModel
{
public:
    DragableSqlQueryModel();
    ~DragableSqlQueryModel();
    Qt::ItemFlags flags(const QModelIndex & index) const;
};

#endif // DRAGABLESQLQUERYMODEL_H

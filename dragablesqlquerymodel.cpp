#include "dragablesqlquerymodel.h"

DragableSqlQueryModel::DragableSqlQueryModel()
{

}

DragableSqlQueryModel::~DragableSqlQueryModel()
{

}

Qt::ItemFlags DragableSqlQueryModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QSqlQueryModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

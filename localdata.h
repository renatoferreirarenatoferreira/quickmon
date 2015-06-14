#ifndef LOCALDATA_H
#define LOCALDATA_H

#include <QtSql/QtSql>

class LocalData
{
public:
    static LocalData* Instance();
    QSqlQuery createQuery();
    QSqlQuery createQuery(const QString& query);
    bool executeQuery(const QString& query);
    bool executeQuery(QSqlQuery& queryObj);
    bool executeQuery(const QString& query, QSqlQuery & queryObj);
    QString getLastError();

private:
    static LocalData* m_Instance;
    QSqlDatabase db;
    bool initialized;
    bool initialize();
    bool isInitialized();
    LocalData();
    ~LocalData();
};

#endif // LOCALDATA_H

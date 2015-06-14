#include "localdata.h"
#include <QMutex>

#ifdef QT_DEBUG
#include <QDebug>
#endif

LocalData* LocalData::m_Instance = 0;

LocalData::LocalData()
{

}

LocalData::~LocalData()
{

}

LocalData* LocalData::Instance()
{
    static QMutex mutex;
    if (!m_Instance)
    {
        mutex.lock();

        if (!m_Instance)
        {
            m_Instance = new LocalData;
            m_Instance->initialized = m_Instance->initialize();
        }

        mutex.unlock();
    }

    return m_Instance;
}

bool LocalData::initialize()
{
    db =  QSqlDatabase::addDatabase("QSQLITE");

#ifdef Q_OS_WIN32
    db.setDatabaseName(QCoreApplication::applicationDirPath() + "/quickmon.sqlite");
#else
    db.setDatabaseName("~/.quickmon.sqlite");
#endif

#ifdef QT_DEBUG
    if (db.open())
        qDebug() << "LocalData : initialize :" << db.databaseName() << ": Success";
    else {
        qDebug() << "LocalData : initialize :" << db.databaseName() << ": Error : " << getLastError();
        return false;
    }
#else
    if (!db.open())
        return false;
#endif

    //check existence of database through its hosts table
    QSqlQuery query = createQuery();
    executeQuery("SELECT count(*) FROM sqlite_master WHERE type='table' AND name='hosts';", query);
    if (query.next())
    {
        if ( query.value(0).toInt() <= 0 )
        {
            //create host table
            executeQuery("CREATE TABLE hosts ("
                            "id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                            "name TEXT NOT NULL,"
                            "address TEXT,"
                            "snmpversion INTEGER,"
                            "snmpcommunity TEXT,"
                            "snmpv3seclevel TEXT,"
                            "snmpv3authprotocol TEXT,"
                            "snmpv3authpassphrase TEXT,"
                            "snmpv3privprotocol TEXT,"
                            "snmpv3privpassphrase TEXT"
                         ")", query);

            //add first host
            executeQuery("INSERT INTO hosts ("
                            "name,"
                            "address,"
                            "snmpversion,"
                            "snmpcommunity"
                         ") VALUES ("
                            "'localhost',"
                            "'127.0.0.1',"
                            "2,"
                            "'public'"
                         ")", query);
        }
        return true;
    }
    else
        return false;
}

bool LocalData::isInitialized()
{
    return initialized;
}

QSqlQuery LocalData::createQuery()
{
    return QSqlQuery(db);
}

QSqlQuery LocalData::createQuery(const QString & query)
{
    QSqlQuery returnQuery(db);
    returnQuery.prepare(query);
    return returnQuery;
}

bool LocalData::executeQuery(const QString & query)
{
    return executeQuery(query, createQuery());
}

bool LocalData::executeQuery(const QString & query, QSqlQuery & queryObj)
{
    queryObj.prepare(query);
    return executeQuery(queryObj);
}

bool LocalData::executeQuery(QSqlQuery & queryObj)
{
#ifdef QT_DEBUG
    if (queryObj.exec())
    {
        if (queryObj.isSelect())
            qDebug() << "LocalData : executeQuery :" << queryObj.executedQuery() << ": Success :" << queryObj.size() << "rows returned";
        else
            qDebug() << "LocalData : executeQuery :" << queryObj.executedQuery() << ": Success :" << queryObj.numRowsAffected() << "rows affected";

        return true;
    } else
    {
        qDebug() << "LocalData : executeQuery :" << queryObj.executedQuery() << ": Error : " << getLastError();

        return false;
    }
#else
    return queryObj.exec(query);
#endif
}

QString LocalData::getLastError()
{
    return db.lastError().text();
}


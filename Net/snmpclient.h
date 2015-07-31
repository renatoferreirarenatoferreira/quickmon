#ifndef SNMPCLIENT_H
#define SNMPCLIENT_H

#include <QObject>
#include <QHash>
#include <QMap>
#include <QVariant>
#include <QHostAddress>
#include <QStringListIterator>
#include <QMap>

#include <QDebug>

#include <WinSock2.h> //avoiding the winsock include hell
#define STDCXX_98_HEADERS //fixing snmppp lib configration
#include "snmp_pp/snmp_pp.h"

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

#define SNMPCLIENT_TIMEOUT 2000
#define SNMPCLIENT_RETRIES 3
#define SNMPCLIENT_QUERYTYPE_GET 0
#define SNMPCLIENT_QUERYTYPE_WALK 1

#pragma comment(lib, "SNMP++.lib")
#pragma comment(lib, "libdes.lib")

struct SNMPData {
    //common data
    snmp_version version;
    UdpAddress* address;
    Pdu pdu;
    SnmpTarget* target;
    int returnCount;
    int queryType;
    QStringList OIDs;

    //v1 and v2c data
    CTarget* communityTarget;
    OctetStr community;

    //v3 data
    UTarget* userTarget;
    OctetStr securityName;
    OctetStr authPass;
    OctetStr privPass;
    OctetStr contextName;
    OctetStr contextEngineID;
    long securityLevel;
    long authProtocol;
    long privProtocol;

    //callback
    PVOID listener;

    //return data
    QMap<QString, QString> returnValues;
};

class ISNMPReplyListener
{
    public:
        virtual void receiveSNMPReply(SNMPData* data) = 0;
};

class SNMPClient : public QObject
{
    Q_OBJECT
public:
    static SNMPClient* Instance();
    void addMappings(QString OID, QMap<QString, QVariant> valueMappings);
    QString mapValue(QString OID, QString key);
    void SNMPGet(int version,
                 QHostAddress address,
                 QStringList OIDs,
                 QString community,
                 ISNMPReplyListener* listener);
    void SNMPv3Get(QHostAddress address,
                   QStringList OIDs,
                   QString v3SecLevel,
                   QString v3AuthProtocol,
                   QString v3AuthPassPhrase,
                   QString v3PrivProtocol,
                   QString v3PrivPassPhrase,
                   ISNMPReplyListener* listener);
    void receiveReply(SNMPData* data);

private:
    static SNMPClient* m_Instance;
    explicit SNMPClient(QObject *parent = 0);
    ~SNMPClient();
    Snmp* SNMPv4;
    Snmp* SNMPv6;
    bool supportv4;
    bool supportv6;
    v3MP* v3_MP;
    QHash<QString, QHash<QString, QString>> valueMappings;
    SNMPData* prepareData(int version,
                          QHostAddress address,
                          QStringList OIDs,
                          QString community,
                          QString v3SecLevel,
                          QString v3AuthProtocol,
                          QString v3AuthPassPhrase,
                          QString v3PrivProtocol,
                          QString v3PrivPassPhrase,
                          ISNMPReplyListener* listener);
};

void callback(int reason, Snmp *snmp, Pdu &pdu, SnmpTarget &target, void *cd);

#endif // SNMPCLIENT_H

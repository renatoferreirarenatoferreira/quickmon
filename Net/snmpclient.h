#ifndef SNMPCLIENT_H
#define SNMPCLIENT_H

#include <QObject>
#include <QHash>
#include <QMap>
#include <QVariant>
#include <QHostAddress>
#include <QStringListIterator>
#include <QMap>
#include <QDateTime>

#include <WinSock2.h> //avoiding the winsock include hell
#define STDCXX_98_HEADERS //fixing snmppp lib configration
#include "snmp_pp/snmp_pp.h"

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

#define SNMPCLIENT_MILLISECONDSTIMEOUT 2000
#define SNMPCLIENT_RETRIES 1
#define SNMPCLIENT_QUERYTYPE_GET 0
#define SNMPCLIENT_QUERYTYPE_WALK 1
#define SNMPCLIENT_BULK_MAXREPETITIONS 10
#define SNMPCLIENT_GET_MAXOID 10

#pragma comment(lib, "SNMP++.lib")
#pragma comment(lib, "libdes.lib")

#define SNMP_RESPONSE_SUCCESS 0
#define SNMP_RESPONSE_TIMEOUT 1
#define SNMP_RESPONSE_ERROR 2

#define SNMPVARIABLE_TYPE_OCTETSTRING 0
#define SNMPVARIABLE_TYPE_INTEGER 1
#define SNMPVARIABLE_TYPE_UNSIGNEDINT 2
#define SNMPVARIABLE_TYPE_COUNTER64 5
#define SNMPVARIABLE_TYPE_OTHER 6

struct SNMPVariable
{
    QString OID;
    int type;
    QVariant variantValue;
    qint64 timestamp;
};

struct SNMPData {
    //common data
    snmp_version version;
    UdpAddress* address;
    QList<Pdu*> pduList;
    int currPdu;
    SnmpTarget* target;
    QStringList OIDs;
    QHostAddress requestAddress;
    QMutex* mutex;
    int queryType;
    Oid lastOID;

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
    QList<SNMPVariable> returnVariables;
    int responseStatus;
    QString errorMessage;
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
    SNMPData* SNMPGet(int version,
                      QHostAddress address,
                      QStringList OIDs,
                      QString community,
                      ISNMPReplyListener* listener);
    SNMPData* SNMPv3Get(QHostAddress address,
                        QStringList OIDs,
                        QString v3SecLevel,
                        QString v3AuthProtocol,
                        QString v3AuthPassPhrase,
                        QString v3PrivProtocol,
                        QString v3PrivPassPhrase,
                        ISNMPReplyListener* listener);
    SNMPData* SNMPWalk(int version,
                       QHostAddress address,
                       QString OID,
                       QString community,
                       ISNMPReplyListener* listener);
    SNMPData* SNMPv3Walk(QHostAddress address,
                         QString OID,
                         QString v3SecLevel,
                         QString v3AuthProtocol,
                         QString v3AuthPassPhrase,
                         QString v3PrivProtocol,
                         QString v3PrivPassPhrase,
                         ISNMPReplyListener* listener);
    void receiveReply(SNMPData* data);
    static void stopListening(SNMPData* data);
    Snmp* getSNMPv4();
    Snmp* getSNMPv6();
    static QString readDateAndTime(QString hexString);

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

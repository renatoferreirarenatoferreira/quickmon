#include "snmpclient.h"

SNMPClient* SNMPClient::m_Instance = 0;

SNMPClient::SNMPClient(QObject *parent) : QObject(parent)
{
    Snmp::socket_startup();

    int status;

    this->SNMPv4 = new Snmp(status, 0, false);
    supportv4 = (status == SNMP_CLASS_SUCCESS);
    if (supportv4)
        this->SNMPv4->start_poll_thread(10);

    this->SNMPv6 = new Snmp(status, 0, true);
    supportv6 = (status == SNMP_CLASS_SUCCESS);
    if (supportv6)
        this->SNMPv6->start_poll_thread(10);

    int construct_status;
    this->v3_MP = new v3MP("QuickMon", 1, construct_status);
    this->contextName = OctetStr("");
    this->contextEngineID = OctetStr("");

    //register type returned by SNMP requests
    qRegisterMetaType<QList<SNMPVariable>>("QList<SNMPVariable>");
}

SNMPClient::~SNMPClient()
{
    if (supportv4)
        this->SNMPv4->stop_poll_thread();
    delete this->SNMPv4;

    if (supportv6)
        this->SNMPv6->stop_poll_thread();
    delete this->SNMPv6;

    delete this->v3_MP;

    Snmp::socket_cleanup();
}

SNMPClient* SNMPClient::Instance()
{
    static QMutex mutex;
    if (!m_Instance)
    {
        mutex.lock();

        if (!m_Instance)
        {
            m_Instance = new SNMPClient;
        }

        mutex.unlock();
    }

    return m_Instance;
}

void SNMPClient::addMappings(QString OID, QMap<QString, QVariant> valueMappings)
{
    if (!this->valueMappings.contains(OID))
    {
        QHash<QString, QString> mappings;

        QMapIterator<QString, QVariant> valuesIterator(valueMappings);
        while (valuesIterator.hasNext())
        {
            valuesIterator.next();

            mappings.insert(valuesIterator.key(), valuesIterator.value().toString());
        }

        this->valueMappings.insert(OID, mappings);
    }
}

QString SNMPClient::mapValue(QString OID, QString key)
{
    if (this->valueMappings.contains(OID))
    {
        QHash<QString, QString> mappings = this->valueMappings.value(OID);
        if (mappings.contains(key))
            return mappings.value(key);
    }

    return key;
}

SNMPData* SNMPClient::prepareData(int version,
                                  QHostAddress address,
                                  QStringList OIDs,
                                  QString communityUser,
                                  QString v3SecLevel,
                                  QString v3AuthProtocol,
                                  QString v3AuthPassPhrase,
                                  QString v3PrivProtocol,
                                  QString v3PrivPassPhrase,
                                  ISNMPReplyListener* listener)
{
    SNMPData* returnData = new SNMPData();
    returnData->listener = (PVOID)listener;
    returnData->OIDs = OIDs;
    returnData->requestAddress = address;
    returnData->mutex = new QMutex(QMutex::Recursive);

    //set version parameter
    if (version == 1)
        returnData->version = version1;
    else if (version == 2)
        returnData->version = version2c;
    else if (version == 3)
        returnData->version = version3;

    //destination address
    returnData->address = new UdpAddress(address.toString().toUtf8().constData());
    returnData->address->set_port(SNMP_PP_DEFAULT_SNMP_PORT);

    //create lists of OIDs
    Vb nextVar;
    QStringListIterator OIDsIterator(returnData->OIDs);
    int oidCounter = 0;
    bool createPdu = true;
    Pdu* currPdu;
    while (OIDsIterator.hasNext())
    {
        //create new varbind and add it in array
        if (createPdu)
        {
            currPdu = new Pdu();
            returnData->pduList.append(currPdu);
            createPdu = false;
        }

        //add oid to current varbind
        returnData->lastOID = OIDsIterator.next().toUtf8().constData();
        if (returnData->lastOID.valid())
        {
            nextVar.set_oid(returnData->lastOID);
            *currPdu += nextVar;
            oidCounter++;
        }

        //reset counter
        if (oidCounter == SNMPCLIENT_GET_MAXOID)
        {
            oidCounter = 0;
            createPdu = true;
        }
    }
    returnData->currPdu = 0;

    //set target
    if (version == 1 || version == 2)
    {
        //create target object
        returnData->communityTarget = new CTarget(*returnData->address);

        //configure community based authentication
        returnData->communityTarget->set_version(returnData->version);
        returnData->communityTarget->set_retry(SNMPCLIENT_RETRIES);
        returnData->communityTarget->set_timeout(SNMPCLIENT_MILLISECONDSTIMEOUT/10);
        returnData->communityTarget->set_readcommunity(communityUser.toUtf8().constData());

        returnData->target = returnData->communityTarget;
    } else if (version == 3)
    {
        //create target object
        returnData->userTarget = new UTarget(*returnData->address);

        //set authentication protocol SHA/MD5
        int authProtocol = SNMP_AUTHPROTOCOL_NONE;
        if (v3AuthProtocol == "SHA")
            authProtocol = SNMP_AUTHPROTOCOL_HMACSHA;
        else if (v3AuthProtocol == "MD5")
            authProtocol = SNMP_AUTHPROTOCOL_HMACMD5;

        //set privacy protocol
        int privProtocol = SNMP_PRIVPROTOCOL_NONE;
        if (v3PrivProtocol == "DES")
            privProtocol = SNMP_PRIVPROTOCOL_DES;
        else if (v3PrivProtocol == "3DES")
            privProtocol = SNMP_PRIVPROTOCOL_3DESEDE;
        else if (v3PrivProtocol == "IDEA")
            privProtocol = SNMP_PRIVPROTOCOL_IDEA;
        else if (v3PrivProtocol == "AES128")
            privProtocol = SNMP_PRIVPROTOCOL_AES128;
        else if (v3PrivProtocol == "AES192")
            privProtocol = SNMP_PRIVPROTOCOL_AES192;
        else if (v3PrivProtocol == "AES256")
            privProtocol = SNMP_PRIVPROTOCOL_AES256;

        //configure user based authentication
        this->v3_MP->get_usm()->add_usm_user(communityUser.toUtf8().constData(),
                                             authProtocol,
                                             privProtocol,
                                             v3AuthPassPhrase.toUtf8().constData(),
                                             v3PrivPassPhrase.toUtf8().constData());

        //configure target
        returnData->userTarget->set_version(returnData->version);
        returnData->userTarget->set_retry(SNMPCLIENT_RETRIES);
        returnData->userTarget->set_timeout(SNMPCLIENT_MILLISECONDSTIMEOUT/10);
        returnData->userTarget->set_security_model(SNMP_SECURITY_MODEL_USM);
        returnData->userTarget->set_security_name(communityUser.toUtf8().constData());

        //configure pdu
        for (int i = 0; i < returnData->pduList.size(); ++i)
        {
            Pdu* currPdu = returnData->pduList.at(i);
            currPdu->set_context_name(this->contextName);
            currPdu->set_context_engine_id(this->contextEngineID);

            //set security level
            if (v3SecLevel == "noAuthNoPriv")
                currPdu->set_security_level(SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV);
            else if (v3SecLevel == "authNoPriv")
                currPdu->set_security_level(SNMP_SECURITY_LEVEL_AUTH_NOPRIV);
            else if (v3SecLevel == "authPriv")
                currPdu->set_security_level(SNMP_SECURITY_LEVEL_AUTH_PRIV);
        }

        returnData->target = returnData->userTarget;
    }

    return returnData;
}

SNMPData* SNMPClient::SNMPGet(int version,
                              QHostAddress address,
                              QStringList OIDs,
                              QString community,
                              ISNMPReplyListener* listener)
{
    SNMPData* data = this->prepareData(version, address, OIDs, community, NULL, NULL, NULL, NULL, NULL, listener);
    data->queryType = SNMPCLIENT_QUERYTYPE_GET;

    if (data->address->get_ip_version() == Address::version_ipv4)
        this->SNMPv4->get(*data->pduList.at(data->currPdu), *data->target, callback, data);
    else if (data->address->get_ip_version() == Address::version_ipv6)
        this->SNMPv6->get(*data->pduList.at(data->currPdu), *data->target, callback, data);

    return data;
}

SNMPData* SNMPClient::SNMPv3Get(QHostAddress address,
                                QStringList OIDs,
                                QString userName,
                                QString v3SecLevel,
                                QString v3AuthProtocol,
                                QString v3AuthPassPhrase,
                                QString v3PrivProtocol,
                                QString v3PrivPassPhrase,
                                ISNMPReplyListener* listener)
{
    SNMPData* data = this->prepareData(3, address, OIDs, userName, v3SecLevel, v3AuthProtocol, v3AuthPassPhrase, v3PrivProtocol, v3PrivPassPhrase, listener);
    data->queryType = SNMPCLIENT_QUERYTYPE_GET;

    if (data->address->get_ip_version() == Address::version_ipv4)
        this->SNMPv4->get(*data->pduList.at(data->currPdu), *data->target, callback, data);
    else if (data->address->get_ip_version() == Address::version_ipv6)
        this->SNMPv6->get(*data->pduList.at(data->currPdu), *data->target, callback, data);

    return data;
}

SNMPData* SNMPClient::SNMPWalk(int version,
                               QHostAddress address,
                               QString OID,
                               QString community,
                               ISNMPReplyListener* listener)
{
    QStringList OIDs;
    OIDs.append(OID);

    SNMPData* data = this->prepareData(version, address, OIDs, community, NULL, NULL, NULL, NULL, NULL, listener);
    data->queryType = SNMPCLIENT_QUERYTYPE_WALK;

    if (data->address->get_ip_version() == Address::version_ipv4)
        this->SNMPv4->get_bulk(*data->pduList.at(data->currPdu), *data->target, 0, SNMPCLIENT_BULK_MAXREPETITIONS, callback, data);
    else if (data->address->get_ip_version() == Address::version_ipv6)
        this->SNMPv6->get_bulk(*data->pduList.at(data->currPdu), *data->target, 0, SNMPCLIENT_BULK_MAXREPETITIONS, callback, data);

    return data;
}

SNMPData* SNMPClient::SNMPv3Walk(QHostAddress address,
                                 QString OID,
                                 QString userName,
                                 QString v3SecLevel,
                                 QString v3AuthProtocol,
                                 QString v3AuthPassPhrase,
                                 QString v3PrivProtocol,
                                 QString v3PrivPassPhrase,
                                 ISNMPReplyListener* listener)
{
    QStringList OIDs;
    OIDs.append(OID);

    SNMPData* data = this->prepareData(3, address, OIDs, userName, v3SecLevel, v3AuthProtocol, v3AuthPassPhrase, v3PrivProtocol, v3PrivPassPhrase, listener);
    data->queryType = SNMPCLIENT_QUERYTYPE_WALK;

    if (data->address->get_ip_version() == Address::version_ipv4)
        this->SNMPv4->get_bulk(*data->pduList.at(data->currPdu), *data->target, 0, SNMPCLIENT_BULK_MAXREPETITIONS, callback, data);
    else if (data->address->get_ip_version() == Address::version_ipv6)
        this->SNMPv6->get_bulk(*data->pduList.at(data->currPdu), *data->target, 0, SNMPCLIENT_BULK_MAXREPETITIONS, callback, data);

    return data;
}

void SNMPClient::receiveReply(SNMPData* data)
{
    //callback listener
    data->mutex->lock();
    if (data->listener != NULL)
        ((ISNMPReplyListener*)data->listener)->receiveSNMPReply(data);
    data->listener = NULL;
    data->mutex->unlock();

    //clear data
    for (int i = 0; i < data->pduList.size(); ++i)
        delete data->pduList.at(i);
    data->pduList.clear();
    ////
    if (data->version == version1 || data->version == version2c)
        delete data->communityTarget;
    else if (data->version == version3)
        delete data->userTarget;
    delete data->address;
    delete data->mutex;
    delete data;
}

void SNMPClient::stopListening(SNMPData* data)
{
    data->mutex->lock();
    if (data->listener != NULL)
        data->listener = NULL;
    data->mutex->unlock();
}

Snmp* SNMPClient::getSNMPv4()
{
    return this->SNMPv4;
}

Snmp* SNMPClient::getSNMPv6()
{
    return this->SNMPv6;
}

void callback(int reason, Snmp *snmp, Pdu &pdu, SnmpTarget &target, void *cd)
{
    SNMPData* data = (SNMPData*) cd;

    bool endOfData = false;
    Vb nextVar;
    if (reason == SNMP_CLASS_ASYNC_RESPONSE)
    {
        data->responseStatus = SNMP_RESPONSE_SUCCESS;

        OctetStr octetString;
        Oid nextOid;
        int nextVarSyntax;
        qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
        for ( int i=0; i<pdu.get_vb_count(); i++)
        {
            pdu.get_vb(nextVar,i);
            nextVarSyntax = nextVar.get_syntax();

            if (nextVar.get_exception_status() != SNMP_CLASS_SUCCESS)
            {
                data->responseStatus = SNMP_RESPONSE_ERROR;
                data->errorMessage = Snmp::error_msg(nextVarSyntax);
                break;
            }

            //test if we're still in the tree in case of snmpwalk or if all oid were collected in case of get
            if (data->queryType == SNMPCLIENT_QUERYTYPE_WALK)
            {
                nextVar.get_oid(nextOid);
                if (data->lastOID.nCompare(data->lastOID.len(), nextOid) != 0)
                {
                    endOfData = true;
                    break;
                }
            } else if (data->queryType == SNMPCLIENT_QUERYTYPE_GET)
            {
                if ((data->pduList.size()-1) == data->currPdu)
                    endOfData = true;
            }

            if (nextVarSyntax != sNMP_SYNTAX_ENDOFMIBVIEW)
            {
                SNMPVariable nextReturnVariable;
                nextReturnVariable.OID = nextVar.get_printable_oid();
                nextReturnVariable.timestamp = timestamp;

                if (nextVarSyntax == sNMP_SYNTAX_CNTR32 || nextVarSyntax == sNMP_SYNTAX_UINT32)
                {
                    nextReturnVariable.type = SNMPVARIABLE_TYPE_UNSIGNEDINT;

                    unsigned int rawValue;
                    nextVar.get_value(rawValue);

                    nextReturnVariable.variantValue = QVariant(rawValue);
                } else if (nextVarSyntax == sNMP_SYNTAX_INT32)
                {
                    nextReturnVariable.type = SNMPVARIABLE_TYPE_INTEGER;

                    int rawValue;
                    nextVar.get_value(rawValue);

                    nextReturnVariable.variantValue = QVariant(rawValue);
                } else if (nextVarSyntax == sNMP_SYNTAX_CNTR64)
                {
                    nextReturnVariable.type = SNMPVARIABLE_TYPE_COUNTER64;

                    quint64 rawValue;
                    nextVar.get_value(rawValue);

                    nextReturnVariable.variantValue = QVariant(rawValue);
                } else if (nextVarSyntax == sNMP_SYNTAX_OCTETS)
                {
                    nextReturnVariable.type = SNMPVARIABLE_TYPE_OCTETSTRING;

                    QMap<QString, QVariant> rawValue;

                    nextVar.get_value(octetString);
                    octetString.set_hex_output_type(OctetStr::OutputHex);

                    QString stringValue = octetString.get_printable_clear();
                    rawValue.insert("stringValue", stringValue);

                    QString hexValue = octetString.get_printable_hex();
                    hexValue = hexValue.trimmed();
                    rawValue.insert("hexValue", hexValue);

                    nextReturnVariable.variantValue = rawValue;
                } else {
                    nextReturnVariable.type = SNMPVARIABLE_TYPE_OTHER;
                    QString rawValue = nextVar.get_printable_value();
                    nextReturnVariable.variantValue = QVariant(rawValue);
                }

                data->returnVariables.append(nextReturnVariable);
            }
        }
    } else if (reason == SNMP_CLASS_TIMEOUT)
    {
        data->responseStatus = SNMP_RESPONSE_TIMEOUT;
        data->errorMessage = Snmp::error_msg(reason);
    }
    else
    {
        data->responseStatus = SNMP_RESPONSE_ERROR;
        data->errorMessage = Snmp::error_msg(reason);
    }

    if (data->responseStatus == SNMP_RESPONSE_SUCCESS && data->queryType == SNMPCLIENT_QUERYTYPE_WALK && !endOfData)
    {
        //continue collecting the tree
        (*data->pduList.at(data->currPdu)).set_vblist(&nextVar, 1);
        if (data->address->get_ip_version() == Address::version_ipv4)
            SNMPClient::Instance()->getSNMPv4()->get_bulk(*data->pduList.at(data->currPdu), *data->target, 0, SNMPCLIENT_BULK_MAXREPETITIONS, callback, data);
        else if (data->address->get_ip_version() == Address::version_ipv6)
            SNMPClient::Instance()->getSNMPv6()->get_bulk(*data->pduList.at(data->currPdu), *data->target, 0, SNMPCLIENT_BULK_MAXREPETITIONS, callback, data);
    }
    else if (data->responseStatus == SNMP_RESPONSE_SUCCESS && data->queryType == SNMPCLIENT_QUERYTYPE_GET && !endOfData)
    {
        //collect next group of OIDs
        data->currPdu++;
        if (data->address->get_ip_version() == Address::version_ipv4)
            SNMPClient::Instance()->getSNMPv4()->get(*data->pduList.at(data->currPdu), *data->target, callback, data);
        else if (data->address->get_ip_version() == Address::version_ipv6)
            SNMPClient::Instance()->getSNMPv6()->get(*data->pduList.at(data->currPdu), *data->target, callback, data);
    }
    else
        SNMPClient::Instance()->receiveReply(data);
}

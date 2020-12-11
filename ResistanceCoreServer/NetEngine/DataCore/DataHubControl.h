#ifndef DATAHUBCONTROL_H
#define DATAHUBCONTROL_H


#include "../NetRootHeader.h"
#include "../NetCore/TCPServerManage.h"
#include "Crypto.h"

struct NetTransportData
{
    TCPCell connectData;
    string bodyData;
};

struct ReceiveNavigateData
{
    string keyword;
    void (*DestinationFunction)(TCPCell, string); // (connectData, bodyData)
};

class DataHubControl
{
public:
    DataHubControl();
    ~DataHubControl();

    bool Initialize(TCPServerManage* pTCPServerCore, Crypto* pCrypto);
    bool Destroy();

    void Tick();

public:
    bool PushInputQueue(NetTransportData pushData);
    bool PopInputQueue(NetTransportData* pPopInputData);

    bool PushOutputQueue(NetTransportData pushData);
    bool PopOutputQueue(NetTransportData* pPopOutputData);

    bool TidyInputQueue(TCPCell connectData);

    bool addNewNavigateSet(ReceiveNavigateData navSetData);
    bool modifiedNavigateSet(string targetKey, ReceiveNavigateData navSetData);
    bool deleteMavigateSet(string targetKey);

public:
    static vector<string> StrTokenDivide(const string& data);

private:
    queue<NetTransportData> netInputQueue;
    queue<NetTransportData> netOutputQueue;

    vector<ReceiveNavigateData> receiveNavigateDataList;

    TCPServerManage* pTCPServerCore;
    Crypto* pCrypto;
};


#endif
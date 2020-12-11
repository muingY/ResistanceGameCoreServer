#include "DataHubControl.h"

DataHubControl::DataHubControl()
{
    queue<NetTransportData> empty;
    swap(netInputQueue, empty);
    swap(netOutputQueue, empty);
    receiveNavigateDataList.clear();
    pTCPServerCore = NULL;
}
DataHubControl::~DataHubControl() 
{
    queue<NetTransportData> empty;
    swap(netInputQueue, empty);
    swap(netOutputQueue, empty);
    receiveNavigateDataList.clear();
}

bool DataHubControl::Initialize(TCPServerManage* pTCPServerCore, Crypto* pCrypto) 
{ 
    this->pTCPServerCore = pTCPServerCore;
    this->pCrypto = pCrypto;
    cout << "DataHubControl > DataHubControl Object initialize complete" << endl;
    return true; 
}
bool DataHubControl::Destroy() 
{ 
    cout << "DataHubControl > DataHubControl Object destroy complete" << endl;
    return true; 
}

void DataHubControl::Tick()
{
    // Processing netInputQueue
    if (!netInputQueue.empty())
    {
        NetTransportData inputNetData;
        if (PopInputQueue(&inputNetData))
        {
            bool isKeyFind = false;
            char inputNetDataIP[20];
            inet_ntop(AF_INET, &inputNetData.connectData.tcpHeader.sin_addr.s_addr, inputNetDataIP, sizeof(inputNetDataIP));

            for (ReceiveNavigateData navigateSet : receiveNavigateDataList)
            {
                if (navigateSet.keyword == StrTokenDivide(pCrypto->EncDecryptStr(inputNetData.bodyData)).front())
                {
                    navigateSet.DestinationFunction(inputNetData.connectData, pCrypto->EncDecryptStr(inputNetData.bodyData));
                    isKeyFind = true;
                    cout << "DataHubControl > " << navigateSet.keyword << "(keyword) data IN from " << inputNetDataIP << endl;
                    break;
                }
            }

            if (!isKeyFind)
            {
                cout << "DataHubControl > [Error] Unknown keyword(" << pCrypto->EncDecryptStr(inputNetData.bodyData) << ") data IN from " << inputNetDataIP << endl;
                // Detect abnormal response
                // receiveNavigateDataList.at(0).DestinationFunction(inputNetData.connectData, pCrypto->EncDecryptStr(inputNetData.bodyData));
            }
        }
        else
        {
            cout << "DataHubControl > [Error] PopInputQueue error" << endl;
        }
    }

    // Processting netOutputQueue
    if (!netOutputQueue.empty())
    {
        NetTransportData outputNetData;
        if (PopOutputQueue(&outputNetData))
        {
            char outputNetDataIP[20];
            inet_ntop(AF_INET, &outputNetData.connectData.tcpHeader.sin_addr.s_addr, outputNetDataIP, sizeof(outputNetDataIP));
            
            if (pTCPServerCore->SendData(outputNetData.connectData, pCrypto->EncDecryptStr(outputNetData.bodyData)))
            {
                cout << "DataHubControl > Send data to " << outputNetDataIP << endl; 
            }
            else
            {
                cout << "DataHubControl > [Error] Send data fail to " << outputNetDataIP << endl;
            }
        }
        else
        {
            cout << "DataHubControl > [Error] PopOutputQueue error" << endl;
        }
    }

    // for debug (temp)
    if (netInputQueue.size() > 0) 
    {
        cout << "!!! InputQueueSize: " << netInputQueue.size() << endl;
    }
}

bool DataHubControl::PushInputQueue(NetTransportData pushData)
{
    netInputQueue.push(pushData);
    return true;
}
bool DataHubControl::PopInputQueue(NetTransportData* pPopInputData)
{
    if (netInputQueue.empty())
    {
        return false;
    }
    *pPopInputData = netInputQueue.front();
    netInputQueue.pop();
    return true;
}

bool DataHubControl::PushOutputQueue(NetTransportData pushData)
{
    netOutputQueue.push(pushData);
    return true;
}
bool DataHubControl::PopOutputQueue(NetTransportData* pPopOutputData)
{
    if (netOutputQueue.empty())
    {
        return false;
    }
    *pPopOutputData = netOutputQueue.front();
    netOutputQueue.pop();
    return true;
}

bool DataHubControl::TidyInputQueue(TCPCell connectData)
{
    if (netInputQueue.empty())
    {
        return false;
    }

    for (int i; i < netInputQueue.size(); i++)
    {
        if (netInputQueue.front().connectData == connectData)
        {
            netInputQueue.pop();
        }
    }

    return true;
}

bool DataHubControl::addNewNavigateSet(ReceiveNavigateData navSetData)
{
    receiveNavigateDataList.push_back(navSetData);
    return true;
}
bool DataHubControl::modifiedNavigateSet(string targetKey, ReceiveNavigateData navSetData)
{
    for (ReceiveNavigateData& navigateSet : receiveNavigateDataList)
    {
        if (navigateSet.keyword == targetKey)
        {
            navigateSet = navSetData;
            return true;
        }
    }
    return false;
}
bool DataHubControl::deleteMavigateSet(string targetKey)
{
    if (receiveNavigateDataList.empty())
    {
        return false;
    }
    for (int i = 0; i < receiveNavigateDataList.size(); i++)
    {
        if (receiveNavigateDataList.at(i).keyword == targetKey)
        {
            receiveNavigateDataList.erase(receiveNavigateDataList.begin() + i);
            return true;
        }
    }
    return false;
}

vector<string> DataHubControl::StrTokenDivide(const string& data)
{
    if (data.empty())
    {   
        vector<string> temp;
        temp.push_back(" ");
        return temp;
    }

    vector<string> result;
	string token;
	stringstream ss(data);
	while (ss >> token) 
    {
		result.push_back(token);
	}
	return result;
}
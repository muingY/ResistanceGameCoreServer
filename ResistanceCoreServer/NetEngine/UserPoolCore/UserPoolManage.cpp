#include "UserPoolManage.h"

UserPoolManage::UserPoolManage() {}
UserPoolManage::~UserPoolManage() {}

void UserPoolManage::Initialize(array<UserCell, MAX_CONNECTION>* pUserPool, TCPServerManage* pTCPServer, Crypto* pCrypto)
{
    this->pUserPool = pUserPool;
    this->userPoolReferrerList.fill(false);
    this->pTCPServer = pTCPServer;
    this->pCrypto = pCrypto;
    cout << "UserPoolCore > UserPoolManage initialize" << endl;
}
void UserPoolManage::Destroy()
{
    cout << "UserPoolCore > UserPoolManage destroy" << endl;
}

bool UserPoolManage::AddUserCell(TCPCell newConnectData)
{
    bool isFull = true;
    for (bool bReferrer : userPoolReferrerList)
    {
        if (!bReferrer)
        {
            isFull = false;
            break;
        }
    }
    if (isFull)
    {
        cout << "UserPoolCore > [Error] Can not Access. UserPool is full" << endl;
        pTCPServer->SendData(newConnectData, pCrypto->EncDecryptStr("AccessDenied 1"));
        pTCPServer->Disconnect(&newConnectData);
        pTCPServer->SendData(newConnectData, pCrypto->EncDecryptStr(""));
        return false;
    }

    for (int i = 0; i < MAX_CONNECTION; i++)
    {
        if (!(userPoolReferrerList.at(i)))
        {
            pUserPool->at(i) = {newConnectData, 1};
            userPoolReferrerList.at(i) = true;

            difference_add.push(&pUserPool->at(i));

            break;
        }
    }
    
    return true;
}
bool UserPoolManage::SubReservationUserCell(TCPCell subConnectData)
{
    int posIndex;
    if (!(FindUserWithTCPCell(subConnectData, &posIndex)))
    {
        cout << "UserPoolCore > [Error] Can not sub user. No matching TCPCellData" << endl;
        return false;
    }

    difference_sub.push(&pUserPool->at(posIndex));

    return true;
}
bool UserPoolManage::DelUserCell(TCPCell delConnectData)
{
    int posIndex;
    if (!(FindUserWithTCPCell(delConnectData, &posIndex)))
    {
        cout << "UserPoolCore > [Error] Can not sub user. No matching TCPCellData" << endl;
        return false;
    }

    pUserPool->at(posIndex) = UserCell();
    userPoolReferrerList.at(posIndex) = false;

    return true;
}

bool UserPoolManage::FindUserWithTCPCell(TCPCell targetConnectData, int* positionIndex)
{
    bool bFind = false;
    for (int i = 0; i < MAX_CONNECTION; i++)
    {
        if (userPoolReferrerList.at(i))
        {
            if (pUserPool->at(i).connectData == targetConnectData)
            {
                *positionIndex = i;
                bFind = true;
                break;
            }
        }
    }

    if (!bFind)
    {
        return false;
    }

    return true;
}
bool UserPoolManage::GetUserCellAt(int index, UserCell* pUserCell)
{
    if (!(userPoolReferrerList.at(index)))
    {
        return false;
    }

    *pUserCell = pUserPool->at(index);
    // pUserCell = &pUserPool->at(index);

    return true;
}
int UserPoolManage::GetUserPoolSize()
{
    int connectCount = 0;
    for(bool bReferrer : userPoolReferrerList)
    {
        if (bReferrer)
        {
            connectCount += 1;
        }
    }

    return connectCount;
}
bool UserPoolManage::GetUserSecureLevel(int index, int* secureLevel)
{
    if (!(userPoolReferrerList.at(index)))
    {
        return false;
    }

    *secureLevel = pUserPool->at(index).SecureLevel;

    return true;
}
bool UserPoolManage::SetUserSecureLevel(int index, int secureLevel)
{
    if (!(userPoolReferrerList.at(index)))
    {
        return false;
    }

    pUserPool->at(index).SecureLevel = secureLevel;

    return true;
}
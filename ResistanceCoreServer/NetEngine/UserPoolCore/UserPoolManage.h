#ifndef USERPOOLMANAGE_H
#define USERPOOLMANAGE_H


#include "../NetRootHeader.h"
#include "../NetCore/TCPServerManage.h"
#include "../DataCore/Crypto.h"

/*
[Secure&Status Level]
1 = Just connected
2 = Authenticated (= Logged in)
3 = ...
...
*/

struct UserCell
{
    TCPCell connectData;
    int SecureLevel;

    bool operator==(const UserCell& a)
    {
        if ((connectData == a.connectData) && (SecureLevel == a.SecureLevel))
        {
            return true;
        }
        return false;
    }
};

class UserPoolManage
{
public:
    UserPoolManage();
    ~UserPoolManage();

    void Initialize(array<UserCell, MAX_CONNECTION>* pUserPool, TCPServerManage* pTCPServer, Crypto* pCrypto);
    void Destroy();

    bool AddUserCell(TCPCell newConnectData);
    bool SubReservationUserCell(TCPCell subConnectData);
    bool DelUserCell(TCPCell deleteConnectData);

    bool FindUserWithTCPCell(TCPCell targetConnectData, int* positionIndex);
    bool GetUserCellAt(int index, UserCell* pUserCell);
    int GetUserPoolSize();
    bool GetUserSecureLevel(int index, int* secureLevel);
    bool SetUserSecureLevel(int index, int secureLevel);

public:
    queue<UserCell*> difference_add;
    queue<UserCell*> difference_sub;

private:
    array<UserCell, MAX_CONNECTION>* pUserPool;

public:
    array<bool, MAX_CONNECTION> userPoolReferrerList;

private:
    TCPServerManage* pTCPServer;
    Crypto* pCrypto;
};


#endif
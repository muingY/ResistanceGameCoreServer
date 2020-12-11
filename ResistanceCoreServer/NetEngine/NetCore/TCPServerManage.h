#ifndef TCPINTERFACE_H
#define TCPINTERFACE_H


#include "../NetRootHeader.h"

struct TCPCell
{
    sockaddr_in tcpHeader;
    int tcpSocketID;

    bool operator==(const TCPCell& a)
    {
        bool result = false;
        
        if ((tcpSocketID == a.tcpSocketID) &&
        (tcpHeader.sin_addr.s_addr == a.tcpHeader.sin_addr.s_addr) &&
        (tcpHeader.sin_family == a.tcpHeader.sin_family) &&
        (tcpHeader.sin_port == a.tcpHeader.sin_port))
        {
            result = true;
        }
        else
        {
            result = false;
        }

        return result;
    }
};

struct TCPServer
{
    sockaddr_in serverHeader;
    int serverSocketID;
    int port;
};

class TCPServerManage
{
public:
    TCPServerManage();
    ~TCPServerManage();

    bool Initialize(TCPServer* pMainCore, int port, int bufferLen);
    bool Destroy();

    TCPCell AcceptConnection();
    void Disconnect(TCPCell* tcpCellInfo);

    bool ListenData(TCPCell tcpCellInfo, string* pdata);
    bool SendData(TCPCell tcpCellInfo, string data);
    
private:
    TCPServer* MainCore;

    int bufferLen;
};


#endif
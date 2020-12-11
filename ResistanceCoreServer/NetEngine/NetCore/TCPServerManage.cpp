#include "TCPServerManage.h"

TCPServerManage::TCPServerManage()
{
    this->MainCore = NULL;
    int buffer = 0;
}
TCPServerManage::~TCPServerManage() { }

bool TCPServerManage::Initialize(TCPServer* pMainCore, int port, int bufferLen)
{
    this->MainCore = pMainCore;
    this->bufferLen = bufferLen;

    MainCore->port = port;

    // Create socket
    if ((MainCore->serverSocketID = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        cout << "TCPServer > [Error] Can't open stream socket" << endl;
        exit(0);
    }
    // Initialize MainCore->serverHeader to NULL
    memset(&MainCore->serverHeader, 0x00, sizeof(MainCore->serverHeader));
    // MainCore->serverHeader setting
    MainCore->serverHeader.sin_family = AF_INET;
    MainCore->serverHeader.sin_addr.s_addr = htonl(INADDR_ANY);
    MainCore->serverHeader.sin_port = htons(MainCore->port);
    
    // Call bind
    if (bind(MainCore->serverSocketID, (sockaddr*)&MainCore->serverHeader, sizeof(MainCore->serverHeader)) < 0)
    {
        cout << "TCPServer > [Error] Can't bind local address" << endl;
        exit(0);
    }

    // Socket setting for Listen
    if (listen(MainCore->serverSocketID, 5) < 0)
    {
        cout << "TCPServer > [Error] Can't listening connect" << endl;
        exit(0);
    }

    cout << "TCPServer > TCPServer MainCore initialize complete" << endl;
    return true;
}
bool TCPServerManage::Destroy()
{
    if (MainCore != NULL)
    {
        close(MainCore->serverSocketID);
        MainCore = NULL;
    }
    else
    {
        cout << "TCPServer > [Warning] Can't destroy TCPServer. Server Core does not exist." << endl;
        return false;
    }

    cout << "TCPServer > TCPServer destroy complete" << endl;
    return true;
}

// Have waiting
TCPCell TCPServerManage::AcceptConnection()
{
    TCPCell newClient;

    int len = sizeof(newClient.tcpHeader);
    newClient.tcpSocketID = accept(MainCore->serverSocketID, (sockaddr*)&newClient.tcpHeader, (socklen_t*)&len);
    if (newClient.tcpSocketID < 0)
    {
        cout << "CoreServer > Accept failed someone" << endl;
    }
    char clientIP[20];
    inet_ntop(AF_INET, &newClient.tcpHeader.sin_addr.s_addr, clientIP, sizeof(clientIP));
    cout << "CoreServer > " << clientIP << " client connected" << endl;

    return newClient;
}
void TCPServerManage::Disconnect(TCPCell* tcpCellInfo)
{
    char clientIP[20];
    inet_ntop(AF_INET, &tcpCellInfo->tcpHeader.sin_addr.s_addr, clientIP, sizeof(clientIP));
    close(tcpCellInfo->tcpSocketID);
    cout << "CoreServer > " <<  clientIP << " client disconnected" << endl;
}

// Have waiting
bool TCPServerManage::ListenData(TCPCell tcpCellInfo, string* pdata)
{
    /*if (tcpCellInfo == NULL)
    {
        return false;
    }*/

    pdata->clear();
    char buffer[bufferLen];
    read(tcpCellInfo.tcpSocketID, buffer, 1024);

    // test
    /*
    if (send(tcpCellInfo.tcpSocketID, buffer, bufferLen, 0) == -1)
    {
        cout << "!!!" << endl;
        exit(0);
    }
    */

    *pdata = buffer;
    if (pdata->length() > 0)
    {
        pdata->erase(pdata->size() - 1, pdata->size());
    }

    return true;
}
bool TCPServerManage::SendData(TCPCell tcpCellInfo, string data)
{
    /*if (tcpCellInfo == NULL)
    {
        cout << "TCPServer > [Error] Data send fail" << endl;
        return false;
    }*/

    data = data + "\n";

    char buffer[bufferLen];
    strcpy(buffer, data.c_str());
    int msg_size = data.length();
    write(tcpCellInfo.tcpSocketID, buffer, msg_size);

    return true;
}
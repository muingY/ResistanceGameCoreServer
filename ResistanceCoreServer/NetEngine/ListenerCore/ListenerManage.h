#ifndef LISTENERMANAGE_H
#define LISTENERMANAGE_H


#include "../NetRootHeader.h"
#include "../NetCore/TCPServerManage.h"
#include "../DataCore/DataHubControl.h"
#include "../UserPoolCore/UserPoolManage.h"

struct AcceptProcessData
{
    TCPServerManage* pTCPServer;
    UserPoolManage* pUserPool;
    bool* pProcessHandle;
    pthread_mutex_t* pMutex;
};
struct ListenProcessData
{
    TCPServerManage* pTCPServer;
    DataHubControl* pDataHub;
    UserCell* pUserData;
    bool* pProcessHandle;
    bool* pProcessStatus;
    pthread_mutex_t* pMutex;
};

void* AcceptProcess(void* data);
void* ListenProcess(void* data);

class ListenerManage
{
public:
    ListenerManage();
    ~ListenerManage();

    bool Initialize(TCPServerManage* pTCPServer, DataHubControl* pDataHub, UserPoolManage* pUserPool);
    bool Destroy();
    
    bool AcceptProcessStart();
    bool AcceptProcessStop();

    bool ListenProcessStart();
    bool ListenProcessUpdate();
    bool ListenProcessStop();

private:
    pthread_t acceptThread;
    bool acceptThreadHandle;
    AcceptProcessData acceptThreadData;

    array<pthread_t, MAX_CONNECTION> listenThreadPool;
    array<bool, MAX_CONNECTION> listenThreadPoolStatus;
    array<bool, MAX_CONNECTION> listenThreadPoolHandle;
    array<ListenProcessData, MAX_CONNECTION> listenProcessDataPool;

    pthread_mutex_t acceptProcessMutex;
    pthread_mutex_t listenProcessMutex;

    TCPServerManage* pTCPServer;
    DataHubControl* pDataHub;
    UserPoolManage* pUserPool;
};


#endif
#include "ListenerManage.h"

void* AcceptProcess(void* data)
{
    AcceptProcessData* coreData = static_cast<AcceptProcessData*>(data);

    while (*coreData->pProcessHandle)
    {
        TCPCell newConnectData = coreData->pTCPServer->AcceptConnection();
        pthread_mutex_lock(coreData->pMutex);
        coreData->pUserPool->AddUserCell(newConnectData);
        pthread_mutex_unlock(coreData->pMutex);
    }

    return NULL;
}
void* ListenProcess(void* data)
{
    ListenProcessData* coreData = static_cast<ListenProcessData*>(data);

    *coreData->pProcessStatus = true;
    
    NetTransportData inputNetData;
    inputNetData.connectData = coreData->pUserData->connectData;

    while (*coreData->pProcessHandle)
    {
        coreData->pTCPServer->ListenData(coreData->pUserData->connectData, &inputNetData.bodyData);

        pthread_mutex_lock(coreData->pMutex);
        coreData->pDataHub->PushInputQueue(inputNetData);
        pthread_mutex_unlock(coreData->pMutex);

        // test
        usleep(40);

        char chkBuffer[128] = "ChkConnection";
        if (send(inputNetData.connectData.tcpSocketID, chkBuffer, 128, 0) == -1)
        {
            cout << "ListenProcess > [Warning] Detect abnormal listen process! Terminate thread" << endl;
            *coreData->pProcessHandle = false;

            NetTransportData abnormalInputNetData = inputNetData;
            abnormalInputNetData.bodyData = "AbnormalCommunicationDetection";
            pthread_mutex_lock(coreData->pMutex);
            coreData->pDataHub->PushInputQueue(abnormalInputNetData);
            pthread_mutex_unlock(coreData->pMutex);
        }
    }

    *coreData->pProcessStatus = false;

    return NULL;
}

ListenerManage::ListenerManage() {}
ListenerManage::~ListenerManage() {}

bool ListenerManage::Initialize(TCPServerManage* pTCPServer, DataHubControl* pDataHub, UserPoolManage* pUserPool)
{
    this->pTCPServer = pTCPServer;
    this->pDataHub = pDataHub;
    this->pUserPool = pUserPool;

    acceptThreadHandle = false;

    listenThreadPoolStatus.fill(false);
    listenThreadPoolHandle.fill(false);

    cout << "ListenerManager > ListenerManager initialize" << endl;

    return true;
}
bool ListenerManage::Destroy()
{
    cout << "ListenerManager > ListenerManager destroy" << endl;
    return true;
}

bool ListenerManage::AcceptProcessStart()
{
    pthread_mutex_init(&acceptProcessMutex, NULL);

    acceptThreadHandle = true;

    acceptThreadData.pTCPServer = pTCPServer;
    acceptThreadData.pUserPool = pUserPool;
    acceptThreadData.pProcessHandle = &acceptThreadHandle;
    acceptThreadData.pMutex = &acceptProcessMutex;

    pthread_create(&acceptThread, NULL, AcceptProcess, (void*)&acceptThreadData);

    return true;
}
bool ListenerManage::AcceptProcessStop()
{
    acceptThreadHandle = false;

    pthread_cancel(acceptThread);
    pthread_mutex_destroy(&acceptProcessMutex);

    acceptThreadData = AcceptProcessData();

    return true;
}

bool ListenerManage::ListenProcessStart()
{
    pthread_mutex_init(&listenProcessMutex, NULL);

    return true;
}
bool ListenerManage::ListenProcessUpdate()
{
    if (!pUserPool->difference_add.empty() || !pUserPool->difference_sub.empty())
    {
        if (!pUserPool->difference_add.empty())
        {
            for (int i = 0; i < MAX_CONNECTION; i++)
            {
                if (!listenThreadPoolStatus.at(i))
                {
                    listenProcessDataPool.at(i).pTCPServer = pTCPServer;
                    listenProcessDataPool.at(i).pDataHub = pDataHub;
                    listenProcessDataPool.at(i).pUserData = pUserPool->difference_add.front();
                    listenProcessDataPool.at(i).pProcessHandle = &listenThreadPoolHandle.at(i);
                    listenProcessDataPool.at(i).pProcessStatus = &listenThreadPoolStatus.at(i);
                    listenProcessDataPool.at(i).pMutex = &listenProcessMutex;

                    listenThreadPoolHandle.at(i) = true;
                    pUserPool->difference_add.pop();

                    pthread_create(&listenThreadPool.at(i), NULL, ListenProcess, (void*)&listenProcessDataPool.at(i));
                    pthread_detach(listenThreadPool.at(i));
                    break;
                }
            }
        }

        if (!pUserPool->difference_sub.empty())
        {
            for (int i = 0; i < MAX_CONNECTION; i++)
            {
                if (listenThreadPoolStatus.at(i) || pUserPool->userPoolReferrerList.at(i))
                {
                    if (*pUserPool->difference_sub.front() == *listenProcessDataPool.at(i).pUserData)
                    {
                        // pthread_cancel(listenThreadPool.at(i));
                        *listenProcessDataPool.at(i).pProcessHandle = false;
                        //
                        pUserPool->DelUserCell(pUserPool->difference_sub.front()->connectData);
                        pUserPool->difference_sub.pop();

                        break;
                    }
                }
            }
        }
    }

    return true;
}
bool ListenerManage::ListenProcessStop()
{
    for (int i = 0; i < MAX_CONNECTION; i++)
    {
        if (listenThreadPoolStatus.at(i))
        {
            *listenProcessDataPool.at(i).pProcessHandle = false;
            pthread_cancel(listenThreadPool.at(i));
        }
    }

    pthread_mutex_destroy(&listenProcessMutex);

    return true;
}
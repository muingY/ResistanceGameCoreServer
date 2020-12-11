#ifndef MAINCORE_H
#define MAINCORE_H


#include "ServerRootHeader.h"
#include "DestinationFuncPool.h"

extern void* Extern_MainCore;

void init_keyboard();
void close_keyboard();
int linux_kbhit();
char linux_getch();

struct ActiveUser
{
    int originUserPoolIdx;
    int UserDBidx;
    string id;
    int exp[4];
    int state; // 0:Idle, 1:Belonging, 2:OnPlay
};

struct Room
{
    int roomIdx;
    string roomName;

    array<bool, 4> memberRefList;
    array<string, 4> memberList;
    array<int, 4> memberJobList;

    bool isPlay = false;
    bool isPwd = false; 
    string pwd;

    clock_t generatedTime;
    bool bStartSteady;
};

class MainCore
{
public:
    MainCore();
    ~MainCore();

    bool Initialize();
    bool Tick();
    void Destroy();

public:
    static void GetExpRefinement(int exp, int* level, int* requiredExp, int* ownedExp);

public:
    TCPServerManage* GetpTCPServerManage() { return &TCPServerManager; }
    DBConnectManage* GetpDBConnectManager_read() { return &DBConnectManager_read; }
    DBConnectManage* GetpDBConnectManager_write() { return &DBConnectManager_write; }
    DataHubControl* GetpDataHub() { return &DataHub; }
    UserPoolManage* GetpUserPoolManager() { return &UserPoolManager; }
    ListenerManage* GetpListenerManager() { return &ListenerManager; }

    vector<ActiveUser>* GetpActiveUserPool() { return &ActiveUserPool; }

    array<Room, MAX_CONNECTION>* GetpRoomPool() { return &RoomPool; }
    array<bool, MAX_CONNECTION>* GetpRoomRefPool() { return &RoomRefPool; }

private:
    /* Server Core Object */
    TCPServer TCPServerCore;
    TCPServerManage TCPServerManager;

    DBConnectManage DBConnectManager_read;
    DBConnectManage DBConnectManager_write;
    
    Crypto CryptoCore;
    DataHubControl DataHub;

    array<UserCell, MAX_CONNECTION> UserPool;
    UserPoolManage UserPoolManager;

    ListenerManage ListenerManager;

    /* Game Net Core Vars */
    vector<ActiveUser> ActiveUserPool;
    array<Room, MAX_CONNECTION> RoomPool;
    array<bool, MAX_CONNECTION> RoomRefPool;
};


#endif
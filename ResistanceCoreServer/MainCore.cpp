#include "MainCore.h"

static struct termios initial_settings, new_settings;
static int peek_character = -1;

void init_keyboard()
{
    tcgetattr(0, &initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_lflag &= ~ISIG;
    new_settings.c_cc[VMIN] = 1;
    new_settings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
}
void close_keyboard()
{
    tcsetattr(0, TCSANOW, &initial_settings);
}
int linux_kbhit()
{
    char ch;
    int nread;
    if (peek_character != -1)
        return 1;
                      
    new_settings.c_cc[VMIN] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
    nread = read(0, &ch, 1);
    new_settings.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &new_settings);
    if (nread == 1)
    {
        peek_character = ch;
        return 1;
    }
    return 0;
}
char linux_getch()
{
    char ch;
    if (peek_character != -1)
    {
        ch = peek_character;
        peek_character = -1;
        return ch;
    }
    read(0, &ch, 1);
    return ch;
}

MainCore::MainCore() {}
MainCore::~MainCore() {}

bool MainCore::Initialize()
{
    /* Core object initialize */
    TCPServerManager.Initialize(&TCPServerCore, 3360, 128);

    DBConnectManager_read.Initialize((char*)"(DB ipv4 address)", (char*)"Resistance_ReadOnly", (char*)"(password)", (char*)"(DB name)");
    DBConnectManager_read.DBConnect();
    DBConnectManager_write.Initialize((char*)"(DB ipv4 address)", (char*)"Resistance_WriteOnly", (char*)"(password)", (char*)"(DB name)");
    DBConnectManager_write.DBConnect();

    CryptoCore.Initialize("crepyto key");
    DataHub.Initialize(&TCPServerManager, &CryptoCore);

    UserPoolManager.Initialize(&UserPool, &TCPServerManager, &CryptoCore);

    ListenerManager.Initialize(&TCPServerManager, &DataHub, &UserPoolManager);

    /* Game net core initialize */
    ActiveUserPool.clear();

    /* DataHub setting */
    DataHub.addNewNavigateSet({"AbnormalCommunicationDetection", SystemControlFuncPool::AbnormalCommunicationDetection}); // [0]

    DataHub.addNewNavigateSet({"GetServerState", DestinationFuncPool::Des_GetServerState});
    DataHub.addNewNavigateSet({"ConnectClose", DestinationFuncPool::Des_ConnectClose});
    DataHub.addNewNavigateSet({"CloseChk", DestinationFuncPool::Des_CloseChk});

    DataHub.addNewNavigateSet({"Login", DestinationFuncPool::Des_Login});
    DataHub.addNewNavigateSet({"Logout", DestinationFuncPool::Des_Logout});
    
    DataHub.addNewNavigateSet({"GetUserExp", DestinationFuncPool::Des_GetUserExp});
    DataHub.addNewNavigateSet({"GetEnemyData", DestinationFuncPool::Des_GetEnemyData});
    DataHub.addNewNavigateSet({"GetBossData", DestinationFuncPool::Des_GetBossData});

    DataHub.addNewNavigateSet({"CreateRoom", DestinationFuncPool::Des_CreateRoom});
    DataHub.addNewNavigateSet({"GetRoomList", DestinationFuncPool::Des_GetRoomList});
    DataHub.addNewNavigateSet({"RoomEnter", DestinationFuncPool::Des_RoomEnter});
    DataHub.addNewNavigateSet({"RoomExit", DestinationFuncPool::Des_RoomExit});
    DataHub.addNewNavigateSet({"GetRoomMembers", DestinationFuncPool::Des_GetRoomMember});
    DataHub.addNewNavigateSet({"StartGame", DestinationFuncPool::Des_StartGame});
    DataHub.addNewNavigateSet({"InterruptStartGame", DestinationFuncPool::Des_InterruptStartGame});
    DataHub.addNewNavigateSet({"GetRoomOwnerServer", DestinationFuncPool::Des_GetRoomOwnerServer});
    DataHub.addNewNavigateSet({"RoomOwnerChange", DestinationFuncPool::Des_RoomOwnerCharge});

    /* Startup */
    ListenerManager.AcceptProcessStart();
    ListenerManager.ListenProcessStart();

    init_keyboard();

    return true;
}

bool MainCore::Tick()
{
    // Key input process
    if (linux_kbhit() == 1)
    {
        char keyInput = linux_getch();
        switch (keyInput)
        {
        // shutdown
        case 'q':
            return false;
            break;
        default:
            break;
        }
    }

    /* Server core tick */
    DataHub.Tick();
    ListenerManager.ListenProcessUpdate();

    /* ActiveUserPool control */
    for (int i = 0; i < ActiveUserPool.size(); i++)
    {
        if (!UserPoolManager.userPoolReferrerList.at(ActiveUserPool.at(i).originUserPoolIdx))
        {
            ActiveUserPool.erase(ActiveUserPool.begin() + i);
        }
    }

    /* RoomPool control */
    for (int i = 0; i < RoomRefPool.size(); i++)
    {
        if (RoomRefPool.at(i))
        {
            bool isNoEmpty = false;
            for (int i_ = 0; i_ < 4; i_++)
            {
                if (RoomPool.at(i).memberRefList.at(i_))
                {
                    isNoEmpty = true;
                }
            }
            if (!isNoEmpty)
            {
                RoomRefPool.at(i) = false;
                RoomPool.at(i) = Room();
            }
        }
    }

    usleep(10);

    return true;
}

void MainCore::Destroy()
{
    /* Game net core destroy */
    ActiveUserPool.clear();

    /* Shutdown */
    ListenerManager.AcceptProcessStop();
    ListenerManager.ListenProcessStop();

    /* Core object destroy */
    ListenerManager.Destroy();
    UserPoolManager.Destroy();
    DataHub.Destroy();
    DBConnectManager_read.Destroy();
    DBConnectManager_write.Destroy();
    TCPServerManager.Destroy();

    /* Game net core destroy */
    ActiveUserPool.clear();

    close_keyboard();
}

void MainCore::GetExpRefinement(int exp, int* level, int* requiredExp, int* ownedExp)
{
    static int levelExp[50] = {
        0,
        15,
        45,
        90,
        150,
        225,
        315,
        420,
        540,
        690,
        870,
        1080,
        1320,
        1590,
        1890,
        2250,
        2670,
        3150,
        3690,
        4290,
        4980,
        5760,
        6630,
        7590,
        8640,
        9810,
        11100,
        12510,
        14040,
        15690,
        17505,
        19485,
        21630,
        23940,
        26415,
        29055,
        31860,
        34830,
        37965,
        41265,
        44775,
        48495,
        52425,
        56565,
        60915,
        65520,
        70380,
        75495,
        80865,
        86490,
    };
    static int levelRequiredExp[51] = {
        0,
        15,
        30,
        45,
        60,
        75,
        90,
        105,
        120,
        150,
        180,
        210,
        240,
        270,
        300,
        360,
        420,
        480,
        540,
        600,
        690,
        780,
        870,
        960,
        1050,
        1170,
        1290,
        1410,
        1530,
        1650,
        1815,
        1980,
        2145,
        2310,
        2475,
        2640,
        2805,
        2970,
        3135,
        3300,
        3510,
        3720,
        3930,
        4140,
        4350,
        4605,
        4860,
        5115,
        5370,
        5625,
        999999
    };

    for (int i = 0; i < 50; i++)
    {
        if (exp >= levelExp[i])
        {
            *level = (exp + 1);
            *requiredExp = levelRequiredExp[i + 1];
            *ownedExp = exp - levelExp[i];

            break;
        }
    }
}


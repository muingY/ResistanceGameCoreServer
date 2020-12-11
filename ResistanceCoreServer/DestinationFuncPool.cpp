#include "DestinationFuncPool.h"

/* SystemControlFuncPool */

void SystemControlFuncPool::AbnormalCommunicationDetection(TCPCell connectData, string bodyData)
{
    MainCore* rootMainCore = (MainCore*)Extern_MainCore;

    char chkBuffer[128] = "ChkConnection";
    if (send(connectData.tcpSocketID, chkBuffer, 128, 0) == -1)
    {
        cout << "SystemControl > [Warning] Detect abnormal connection! Remove connection" << endl;

        rootMainCore->GetpUserPoolManager()->SubReservationUserCell(connectData);
        rootMainCore->GetpTCPServerManage()->Disconnect(&connectData);
    }

    return;
}


/* DestinationFuncPool */

void DestinationFuncPool::Des_GetServerState(TCPCell connectData, string bodyData)
{
    MainCore* rootMainCore = (MainCore*)Extern_MainCore;
    
    NetTransportData outputData;
    outputData.connectData = connectData;
    outputData.bodyData = "ServerState 1";
    rootMainCore->GetpDataHub()->PushOutputQueue(outputData);
}

void DestinationFuncPool::Des_ConnectClose(TCPCell connectData, string bodyData)
{
    MainCore* rootMainCore = (MainCore*)Extern_MainCore;

    rootMainCore->GetpUserPoolManager()->SubReservationUserCell(connectData);
    rootMainCore->GetpTCPServerManage()->Disconnect(&connectData);
}

void DestinationFuncPool::Des_CloseChk(TCPCell connectData, string bodyData)
{
    cout << "CoreServer > Client graceful disconnected" << endl;
    return;
}

void DestinationFuncPool::Des_Login(TCPCell connectData, string bodyData)
{
    MainCore* rootMainCore = (MainCore*)Extern_MainCore;

    int user_idx_;
    if (!rootMainCore->GetpUserPoolManager()->FindUserWithTCPCell(connectData, &user_idx_)) { return; }
    int user_secureLevel_;
    rootMainCore->GetpUserPoolManager()->GetUserSecureLevel(user_idx_, &user_secureLevel_);
    if (user_secureLevel_ > 1)
    {
        NetTransportData outputData;
        outputData.connectData = connectData;
        outputData.bodyData = "LoginResult 0";
        rootMainCore->GetpDataHub()->PushOutputQueue(outputData);
        return;
    }

    vector<string> dataTokens = rootMainCore->GetpDataHub()->StrTokenDivide(bodyData);

    bool bID = false;
    bool bPW = false;

    MYSQL_RES* res;
    MYSQL_ROW row;
    res = rootMainCore->GetpDBConnectManager_read()->DBMysqlQuery((char*)"select ID from login;");

    while ((row = mysql_fetch_row(res)) != NULL)
    {
        // cout << (string)row[0] << endl;
        if (dataTokens.at(1) == (string)row[0])
        {
            bID = true;
            break;
        }
    }
    if (!bID)
    {
        NetTransportData outputData;
        outputData.connectData = connectData;
        outputData.bodyData = "LoginResult 0";
        rootMainCore->GetpDataHub()->PushOutputQueue(outputData);
        return;
    }
    mysql_free_result(res);

    int user_idx;
    int user_DBidx = 1;
    res = rootMainCore->GetpDBConnectManager_read()->DBMysqlQuery((char*)"select PW from login;");
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        if (dataTokens.at(2) == (string)row[0])
        {
            //int user_idx;
            if (!rootMainCore->GetpUserPoolManager()->FindUserWithTCPCell(connectData, &user_idx)) { return; }
            rootMainCore->GetpUserPoolManager()->SetUserSecureLevel(user_idx, 2);

            NetTransportData outputData;
            outputData.connectData = connectData;
            outputData.bodyData = "LoginResult 1";
            rootMainCore->GetpDataHub()->PushOutputQueue(outputData);
            
            bPW = true;
            break;
        }
        user_DBidx += 1;
    }
    if (!bPW)
    {
        NetTransportData outputData;
        outputData.connectData = connectData;
        outputData.bodyData = "LoginResult 0";
        rootMainCore->GetpDataHub()->PushOutputQueue(outputData);
        return;
    }
    mysql_free_result(res);

    if (bID && bPW)
    {
        // Add User on MainCore::ActiveUserPool
        ActiveUser newUser;
        newUser.originUserPoolIdx = user_idx;
        newUser.UserDBidx = user_DBidx;
        newUser.id = dataTokens.at(1);
        newUser.state = 0;

        MYSQL_RES* res_idx;
        MYSQL_ROW row_idx;

        res_idx = rootMainCore->GetpDBConnectManager_read()->DBMysqlQuery((char*)"select user_idx, exp1, exp2, exp3, exp4 from user_info;");
        while ((row_idx = mysql_fetch_row(res_idx)) != NULL)
        {
            if (newUser.UserDBidx == atoi(row_idx[0]))
            {
                newUser.exp[0] = atoi(row_idx[1]);
                newUser.exp[1] = atoi(row_idx[2]);
                newUser.exp[2] = atoi(row_idx[3]);
                newUser.exp[3] = atoi(row_idx[4]);
                break;
            }
        }
        mysql_free_result(res_idx);

        rootMainCore->GetpActiveUserPool()->push_back(newUser);
    }
}

void DestinationFuncPool::Des_Logout(TCPCell connectData, string bodyData)
{
    MainCore* rootMainCore = (MainCore*)Extern_MainCore;

    int user_idx;
    if (!rootMainCore->GetpUserPoolManager()->FindUserWithTCPCell(connectData, &user_idx)) { return; }
    int user_secureLevel;
    rootMainCore->GetpUserPoolManager()->GetUserSecureLevel(user_idx, &user_secureLevel);

    if (user_secureLevel < 2)
    {
        NetTransportData outputData;
        outputData.connectData = connectData;
        outputData.bodyData = "LogoutResult 0";
        rootMainCore->GetpDataHub()->PushOutputQueue(outputData);

        for (int i = 0; i < rootMainCore->GetpActiveUserPool()->size(); i++)
        {
            if (rootMainCore->GetpActiveUserPool()->at(i).originUserPoolIdx == user_idx)
            {
                rootMainCore->GetpActiveUserPool()->erase(rootMainCore->GetpActiveUserPool()->begin() + i);
                break;
            }
        }

        return;
    }
    rootMainCore->GetpUserPoolManager()->SetUserSecureLevel(user_idx, 1);
    NetTransportData outputData;
    outputData.connectData = connectData;
    outputData.bodyData = "LogoutResult 1";
    rootMainCore->GetpDataHub()->PushOutputQueue(outputData);
}

void DestinationFuncPool::Des_GetUserExp(TCPCell connectData, string bodyData)
{
    MainCore* rootMainCore = (MainCore*)Extern_MainCore;

    int user_idx;
    if (!rootMainCore->GetpUserPoolManager()->FindUserWithTCPCell(connectData, &user_idx)) { return; }
    int user_secureLevel;
    rootMainCore->GetpUserPoolManager()->GetUserSecureLevel(user_idx, &user_secureLevel);
    if (user_secureLevel < 2)
    {
        return;
    }

    for (int i = 0; i < rootMainCore->GetpActiveUserPool()->size(); i++)
    {
        if (rootMainCore->GetpActiveUserPool()->at(i).originUserPoolIdx == user_idx)
        {
            int level[4];
            int requiredExp[4];
            int ownedExp[4];

            for (int i_ = 0; i_ < 4; i_++)
            {
                MainCore::GetExpRefinement(rootMainCore->GetpActiveUserPool()->at(i).exp[i], &level[i_], &requiredExp[i_], &ownedExp[i_]);
            }

            string outputStr = "UserExp ";
            outputStr += rootMainCore->GetpActiveUserPool()->at(i).id;
            outputStr += " ";
            outputStr += to_string(level[0]);
            outputStr += " ";
            outputStr += to_string(requiredExp[0]);
            outputStr += " ";
            outputStr += to_string(ownedExp[0]);
            outputStr += " ";
            outputStr += to_string(level[1]);
            outputStr += " ";
            outputStr += to_string(requiredExp[1]);
            outputStr += " ";
            outputStr += to_string(ownedExp[1]);
            outputStr += " ";
            outputStr += to_string(level[2]);
            outputStr += " ";
            outputStr += to_string(requiredExp[2]);
            outputStr += " ";
            outputStr += to_string(ownedExp[2]);
            outputStr += " ";
            outputStr += to_string(level[3]);
            outputStr += " ";
            outputStr += to_string(requiredExp[3]);
            outputStr += " ";
            outputStr += to_string(ownedExp[3]);

            NetTransportData outputData;
            outputData.connectData = connectData;
            outputData.bodyData = outputStr;
            rootMainCore->GetpDataHub()->PushOutputQueue(outputData);

            break;
        }
    }
}

void DestinationFuncPool::Des_GetEnemyData(TCPCell connectData, string bodyData)
{
    MainCore* rootMainCore = (MainCore*)Extern_MainCore;

    int user_idx;
    if (!rootMainCore->GetpUserPoolManager()->FindUserWithTCPCell(connectData, &user_idx)) { return; }
    int user_secureLevel;
    rootMainCore->GetpUserPoolManager()->GetUserSecureLevel(user_idx, &user_secureLevel);
    if (user_secureLevel < 2)
    {
        return;
    }

    vector<string> dataTokens = rootMainCore->GetpDataHub()->StrTokenDivide(bodyData);

    MYSQL_RES* res;
    MYSQL_ROW row;
    //string dbQuery = "select * from enemy_info where enemy_idx = " + dataTokens.at(1) + ";";
    //res = rootMainCore->GetpDBConnectManager_read()->DBMysqlQuery((char*)dbQuery.c_str());
    res = rootMainCore->GetpDBConnectManager_read()->DBMysqlQuery((char*)"select * from enemy_info;");

    string result = "EnemyData " + dataTokens.at(1) + " ";
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        if (dataTokens.at(1) == (string)row[0])
        {
            result += (string)row[1];
            result += " ";
            result += (string)row[2];
            result += " ";
            result += (string)row[3];
            result += " ";
            result += (string)row[4];

            NetTransportData outputData;
            outputData.connectData = connectData;
            outputData.bodyData = result;
            rootMainCore->GetpDataHub()->PushOutputQueue(outputData);

            break;
        }
    }

    mysql_free_result(res);
}

void DestinationFuncPool::Des_GetBossData(TCPCell connectData, string bodyData)
{
    MainCore* rootMainCore = (MainCore*)Extern_MainCore;

    int user_idx;
    if (!rootMainCore->GetpUserPoolManager()->FindUserWithTCPCell(connectData, &user_idx)) { return; }
    int user_secureLevel;
    rootMainCore->GetpUserPoolManager()->GetUserSecureLevel(user_idx, &user_secureLevel);
    if (user_secureLevel < 2)
    {
        return;
    }

    vector<string> dataTokens = rootMainCore->GetpDataHub()->StrTokenDivide(bodyData);

    MYSQL_RES* res;
    MYSQL_ROW row;
    // res = rootMainCore->GetpDBConnectManager_read()->DBMysqlQuery((char*)"select ID from login;");
    res = rootMainCore->GetpDBConnectManager_read()->DBMysqlQuery((char*)"select * from boss_info;");

    string result = "BossData " + dataTokens.at(1) + " ";;
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        if (dataTokens.at(1) == (string)row[0])
        {
            result += (string)row[1];
            result += " ";
            result += (string)row[2];
            result += " ";
            result += (string)row[3];
            result += " ";
            result += (string)row[4];

            NetTransportData outputData;
            outputData.connectData = connectData;
            outputData.bodyData = result;
            rootMainCore->GetpDataHub()->PushOutputQueue(outputData);

            break;
        }
    }

    mysql_free_result(res);
}

void DestinationFuncPool::Des_CreateRoom(TCPCell connectData, string bodyData)
{
    MainCore* rootMainCore = (MainCore*)Extern_MainCore;

    int user_idx;
    if (!rootMainCore->GetpUserPoolManager()->FindUserWithTCPCell(connectData, &user_idx)) { return; }
    int user_secureLevel;
    rootMainCore->GetpUserPoolManager()->GetUserSecureLevel(user_idx, &user_secureLevel);
    if (user_secureLevel < 2)
    {
        return;
    }

    vector<string> dataTokens = rootMainCore->GetpDataHub()->StrTokenDivide(bodyData);

    Room newRoom;
    for (int i = 0; i < MAX_CONNECTION; i++)
    {
        if (rootMainCore->GetpRoomRefPool()->at(i) == false)
        {
            newRoom.roomIdx = i;
            break;
        }
    }
    newRoom.roomName = dataTokens.at(1);
    newRoom.memberRefList.at(0) = true;
    newRoom.memberRefList.at(1) = false;
    newRoom.memberRefList.at(2) = false;
    newRoom.memberRefList.at(3) = false;
    newRoom.memberList.at(0) = dataTokens.at(2);
    newRoom.memberJobList.at(0) = -1;
    if (dataTokens.at(3) != "0")
    {
        newRoom.isPwd = true;
        newRoom.pwd = dataTokens.at(3);
    }
    else
    {
        newRoom.pwd = "0";
    }
    newRoom.generatedTime = clock();
    newRoom.bStartSteady = false;

    bool bFind = false;
    for (int i = 0; i < rootMainCore->GetpActiveUserPool()->size(); i++)
    {
        if (rootMainCore->GetpActiveUserPool()->at(i).id == newRoom.memberList.at(0))
        {
            rootMainCore->GetpActiveUserPool()->at(i).state = 1;
            bFind = true;
            break;
        }
    }
    if (!bFind)
    {
        NetTransportData outputData;
        outputData.connectData = connectData;
        outputData.bodyData = "CreateRoomResult 0";
        rootMainCore->GetpDataHub()->PushOutputQueue(outputData);
        return;
    }

    rootMainCore->GetpRoomRefPool()->at(newRoom.roomIdx) = true;
    rootMainCore->GetpRoomPool()->at(newRoom.roomIdx) = newRoom;

    NetTransportData outputData;
    outputData.connectData = connectData;
    outputData.bodyData = "CreateRoomResult 1";
    rootMainCore->GetpDataHub()->PushOutputQueue(outputData);
}

void DestinationFuncPool::Des_GetRoomList(TCPCell connectData, string bodyData)
{
    MainCore* rootMainCore = (MainCore*)Extern_MainCore;

    int user_idx;
    if (!rootMainCore->GetpUserPoolManager()->FindUserWithTCPCell(connectData, &user_idx)) { return; }
    int user_secureLevel;
    rootMainCore->GetpUserPoolManager()->GetUserSecureLevel(user_idx, &user_secureLevel);
    if (user_secureLevel < 2)
    {
        return;
    }

    vector<string> dataTokens = rootMainCore->GetpDataHub()->StrTokenDivide(bodyData);

    for (int i = 0; i < rootMainCore->GetpRoomRefPool()->size(); i++)
    {
        if (rootMainCore->GetpRoomRefPool()->at(i))
        {
            string outputStr = "RoomListFrag ";
            outputStr += to_string(rootMainCore->GetpRoomPool()->at(i).roomIdx);
            outputStr += " ";
            outputStr += rootMainCore->GetpRoomPool()->at(i).roomName;
            outputStr += " ";
            outputStr += to_string(rootMainCore->GetpRoomPool()->at(i).isPlay);
            outputStr += " ";
            int count = 0;
            for (int i_ = 0; i_ < 4; i_++)
            {
                if (rootMainCore->GetpRoomPool()->at(i).memberRefList.at(i_))
                {
                    count += 1;
                }
            }
            outputStr += to_string(count);
            outputStr += " ";
            outputStr += to_string(rootMainCore->GetpRoomPool()->at(i).isPwd);

            NetTransportData outputData;
            outputData.connectData = connectData;
            outputData.bodyData = outputStr;
            rootMainCore->GetpDataHub()->PushOutputQueue(outputData);
        }
    }
}

void DestinationFuncPool::Des_RoomEnter(TCPCell connectData, string bodyData)
{
    MainCore* rootMainCore = (MainCore*)Extern_MainCore;

    int user_idx;
    if (!rootMainCore->GetpUserPoolManager()->FindUserWithTCPCell(connectData, &user_idx)) { return; }
    int user_secureLevel;
    rootMainCore->GetpUserPoolManager()->GetUserSecureLevel(user_idx, &user_secureLevel);
    if (user_secureLevel < 2)
    {
        return;
    }

    vector<string> dataTokens = rootMainCore->GetpDataHub()->StrTokenDivide(bodyData);

    if (!rootMainCore->GetpRoomRefPool()->at(stoi(dataTokens.at(1))))
    {
        NetTransportData outputData;
        outputData.connectData = connectData;
        outputData.bodyData = "RoomEnterResult 0";
        rootMainCore->GetpDataHub()->PushOutputQueue(outputData);
        return;
    }
    bool isIdConform = false;
    for (int i = 0; i < rootMainCore->GetpActiveUserPool()->size(); i++)
    {
        if (rootMainCore->GetpActiveUserPool()->at(i).id == dataTokens.at(2))
        {
            isIdConform = true;
            break;
        }
    }
    if (!isIdConform)
    {
        NetTransportData outputData;
        outputData.connectData = connectData;
        outputData.bodyData = "RoomEnterResult 0";
        rootMainCore->GetpDataHub()->PushOutputQueue(outputData);
        return;
    }
    bool isNotFull = false;
    for (int i = 0; i < 4; i++)
    {
        if (!rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberRefList.at(i))
        {
            isNotFull = true;
            break;
        }
    }
    if (!isNotFull)
    {
        NetTransportData outputData;
        outputData.connectData = connectData;
        outputData.bodyData = "RoomEnterResult 0";
        rootMainCore->GetpDataHub()->PushOutputQueue(outputData);
        return;
    }
    if (rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).isPwd)
    {
        if (rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).pwd != dataTokens.at(3))
        {
            NetTransportData outputData;
            outputData.connectData = connectData;
            outputData.bodyData = "RoomEnterResult 0";
            rootMainCore->GetpDataHub()->PushOutputQueue(outputData);
            return;
        }
    }
    
    for (int i = 0; i < 4; i++)
    {
        if (!rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberRefList.at(i))
        {
            rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberRefList.at(i) = true;
            rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberList.at(i) = dataTokens.at(2);
            rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberJobList.at(i) = -1;
            break;
        }
    }
    NetTransportData outputData;
    outputData.connectData = connectData;
    outputData.bodyData = "RoomEnterResult 1";
    rootMainCore->GetpDataHub()->PushOutputQueue(outputData);
}

void DestinationFuncPool::Des_RoomExit(TCPCell connectData, string bodyData)
{
    MainCore* rootMainCore = (MainCore*)Extern_MainCore;

    int user_idx;
    if (!rootMainCore->GetpUserPoolManager()->FindUserWithTCPCell(connectData, &user_idx)) { return; }
    int user_secureLevel;
    rootMainCore->GetpUserPoolManager()->GetUserSecureLevel(user_idx, &user_secureLevel);
    if (user_secureLevel < 2)
    {
        return;
    }

    vector<string> dataTokens = rootMainCore->GetpDataHub()->StrTokenDivide(bodyData);

    if (rootMainCore->GetpRoomRefPool()->at(stoi(dataTokens.at(1))))
    {
        for (int i = 0; i < 4; i++)
        {
            if (rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberRefList.at(i))
            {
                if (rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberList.at(i) == dataTokens.at(2))
                {
                    rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberRefList.at(i) = false;
                    rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberList.at(i) = "";
                    rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberJobList.at(i) = 0;

                    break;
                }
            }
        }
        if (!rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberRefList.at(0))
        {
            for (int i = 0; i < 4; i++)
            {
                if (rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberRefList.at(i))
                {
                    rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberRefList.at(i) = false;
                    rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberRefList.at(0) = true;
                    rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberList.at(0) = rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberList.at(i);
                    rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberList.at(i) = "";
                    rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberJobList.at(0) = rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberJobList.at(i);
                    rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberJobList.at(i) = 0;

                    break;
                }
            }
        }
    }
}

void DestinationFuncPool::Des_GetRoomMember(TCPCell connectData, string bodyData)
{
    MainCore* rootMainCore = (MainCore*)Extern_MainCore;

    int user_idx;
    if (!rootMainCore->GetpUserPoolManager()->FindUserWithTCPCell(connectData, &user_idx)) { return; }
    int user_secureLevel;
    rootMainCore->GetpUserPoolManager()->GetUserSecureLevel(user_idx, &user_secureLevel);
    if (user_secureLevel < 2)
    {
        return;
    }

    vector<string> dataTokens = rootMainCore->GetpDataHub()->StrTokenDivide(bodyData);

    if (rootMainCore->GetpRoomRefPool()->at(stoi(dataTokens.at(1))))
    {
        string outputStr = "RoomMember";
        for (int i = 0; i < 4; i++)
        {
            if (rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberRefList.at(i))
            {
                outputStr += " ";
                outputStr += rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberList.at(i);
                outputStr += " ";
                outputStr += to_string(rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberJobList.at(i));
            }

            // Unnormal user cut
            bool isNormal = false;
            for (int i_ = 0; i_ < rootMainCore->GetpActiveUserPool()->size(); i_++)
            {
                if (rootMainCore->GetpActiveUserPool()->at(i_).id == rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberList.at(i))
                {
                    isNormal = true;
                    break;
                }
            }
            if (!isNormal)
            {
                rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberRefList.at(i) = false;
                rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberList.at(i) = "";
                rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberJobList.at(i) = -1;
            }
        }
        NetTransportData outputData;
        outputData.connectData = connectData;
        outputData.bodyData = outputStr;
        rootMainCore->GetpDataHub()->PushOutputQueue(outputData);
    }
}

void DestinationFuncPool::Des_StartGame(TCPCell connectData, string bodyData)
{
    MainCore* rootMainCore = (MainCore*)Extern_MainCore;

    int user_idx;
    if (!rootMainCore->GetpUserPoolManager()->FindUserWithTCPCell(connectData, &user_idx)) { return; }
    int user_secureLevel;
    rootMainCore->GetpUserPoolManager()->GetUserSecureLevel(user_idx, &user_secureLevel);
    if (user_secureLevel < 2)
    {
        return;
    }

    vector<string> dataTokens = rootMainCore->GetpDataHub()->StrTokenDivide(bodyData);

    if (rootMainCore->GetpRoomRefPool()->at(stoi(dataTokens.at(1))))
    {
        if (rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberList.at(0) != dataTokens.at(2))
        {
            return;
        }
    }

    rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).bStartSteady = true;
    for (int i = 0; i < 4; i++)
    {
        if (rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberRefList.at(i))
        {
            for (ActiveUser user : *rootMainCore->GetpActiveUserPool())
            {
                if (user.id == rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberList.at(i))
                {
                    UserCell targetUser;
                    rootMainCore->GetpUserPoolManager()->GetUserCellAt(user.originUserPoolIdx, &targetUser);

                    NetTransportData outputData;
                    outputData.connectData = targetUser.connectData;
                    outputData.bodyData = "StartingGame";
                    rootMainCore->GetpDataHub()->PushOutputQueue(outputData);
                }
            }
        }
    }
}

void DestinationFuncPool::Des_InterruptStartGame(TCPCell connectData, string bodyData)
{
    MainCore* rootMainCore = (MainCore*)Extern_MainCore;

    int user_idx;
    if (!rootMainCore->GetpUserPoolManager()->FindUserWithTCPCell(connectData, &user_idx)) { return; }
    int user_secureLevel;
    rootMainCore->GetpUserPoolManager()->GetUserSecureLevel(user_idx, &user_secureLevel);
    if (user_secureLevel < 2)
    {
        return;
    }

    vector<string> dataTokens = rootMainCore->GetpDataHub()->StrTokenDivide(bodyData);

    if (rootMainCore->GetpRoomRefPool()->at(stoi(dataTokens.at(1))) && rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).bStartSteady)
    {
        for (int i = 0; i < 4; i++)
        {
            if (rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberList.at(i) == dataTokens.at(2))
            {
                for (ActiveUser user : *rootMainCore->GetpActiveUserPool())
                {
                    if (user.id == rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberList.at(i))
                    {
                        UserCell targetUser;
                        rootMainCore->GetpUserPoolManager()->GetUserCellAt(user.originUserPoolIdx, &targetUser);

                        NetTransportData outputData;
                        outputData.connectData = targetUser.connectData;
                        outputData.bodyData = "StartingGame";
                        rootMainCore->GetpDataHub()->PushOutputQueue(outputData);
                    }
                }
                break;
            }
        }
    }
}

void DestinationFuncPool::Des_GetRoomOwnerServer(TCPCell connectData, string bodyData)
{
    MainCore* rootMainCore = (MainCore*)Extern_MainCore;

    int user_idx;
    if (!rootMainCore->GetpUserPoolManager()->FindUserWithTCPCell(connectData, &user_idx)) { return; }
    int user_secureLevel;
    rootMainCore->GetpUserPoolManager()->GetUserSecureLevel(user_idx, &user_secureLevel);
    if (user_secureLevel < 2)
    {
        return;
    }

    vector<string> dataTokens = rootMainCore->GetpDataHub()->StrTokenDivide(bodyData);

    if (rootMainCore->GetpRoomRefPool()->at(stoi(dataTokens.at(1))))
    {
        for (int i = 0; i < 4; i++)
        {
            if (rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberList.at(i) == dataTokens.at(2))
            {
                string ownerIp;
                UserCell owerUserData;

                for (auto user : *rootMainCore->GetpActiveUserPool())
                {
                    if (user.id == rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberList.at(0))
                    {
                        rootMainCore->GetpUserPoolManager()->GetUserCellAt(user.originUserPoolIdx, &owerUserData);
                        
                        string msgout = "RoomOwnerServer ";
                        char inputNetDataIP[20];
                        inet_ntop(AF_INET, &owerUserData.connectData.tcpHeader.sin_addr.s_addr, inputNetDataIP, sizeof(inputNetDataIP));
                        ownerIp = inputNetDataIP;
                        msgout += ownerIp;

                        NetTransportData outputData;
                        outputData.connectData = connectData;
                        outputData.bodyData = msgout;
                        rootMainCore->GetpDataHub()->PushOutputQueue(outputData);
                        break;
                    }
                }
                break;
            }
        }
    }
}

void DestinationFuncPool::Des_RoomOwnerCharge(TCPCell connectData, string bodyData)
{
    MainCore* rootMainCore = (MainCore*)Extern_MainCore;

    int user_idx;
    if (!rootMainCore->GetpUserPoolManager()->FindUserWithTCPCell(connectData, &user_idx)) { return; }
    int user_secureLevel;
    rootMainCore->GetpUserPoolManager()->GetUserSecureLevel(user_idx, &user_secureLevel);
    if (user_secureLevel < 2)
    {
        return;
    }

    vector<string> dataTokens = rootMainCore->GetpDataHub()->StrTokenDivide(bodyData);

    if (rootMainCore->GetpRoomRefPool()->at(stoi(dataTokens.at(1))))
    {
        int targetIDIdx;
        bool find = false;

        for (int i = 0; i < 4; i++)
        {
            if (rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberList.at(i) == dataTokens.at(i))
            {
                targetIDIdx = i;
                find = true;
            }
        }

        string idTemp;
        int jobTemp;
        if (find)
        {
            idTemp = rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberList.at(targetIDIdx);
            jobTemp = rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberJobList.at(targetIDIdx);

            rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberList.at(targetIDIdx) = rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberList.at(0);
            rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberJobList.at(targetIDIdx) = rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberJobList.at(0);
            rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberList.at(0) = idTemp;
            rootMainCore->GetpRoomPool()->at(stoi(dataTokens.at(1))).memberJobList.at(0) = jobTemp;
        }
    }
}


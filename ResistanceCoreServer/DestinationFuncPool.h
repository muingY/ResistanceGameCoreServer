#ifndef DESTINATIONFUNCPOOL_H
#define DESTINATIONFUNCPOOL_H


#include "ServerRootHeader.h"
#include "MainCore.h"

extern void* Extern_MainCore;

/* 
Destination Function Rule
: void (*DestinationFunction)(TCPCell, string); 
*/

namespace SystemControlFuncPool
{
    void AbnormalCommunicationDetection(TCPCell connectData, string bodyData);
};

namespace DestinationFuncPool
{
    void Des_GetServerState(TCPCell connectData, string bodyData);
    void Des_ConnectClose(TCPCell connectData, string bodyData);
    void Des_CloseChk(TCPCell connectData, string bodyData);

    void Des_Login(TCPCell connectData, string bodyData);
    void Des_Logout(TCPCell connectData, string bodyData);
    
    void Des_GetUserExp(TCPCell connectData, string bodyData);
    void Des_GetEnemyData(TCPCell connectData, string bodyData);
    void Des_GetBossData(TCPCell connectData, string bodyData);

    void Des_CreateRoom(TCPCell connectData, string bodyData); 
    void Des_GetRoomList(TCPCell connectData, string bodyData); 
    void Des_RoomEnter(TCPCell connectData, string bodyData); 
    void Des_RoomExit(TCPCell connectData, string bodyData); 
    void Des_GetRoomMember(TCPCell connectData, string bodyData); 
    void Des_StartGame(TCPCell connectData, string bodyData); 
    void Des_InterruptStartGame(TCPCell connectData, string bodyData);
    void Des_GetRoomOwnerServer(TCPCell connectData, string bodyData);
    void Des_RoomOwnerCharge(TCPCell connectData, string bodyData);
};

#endif
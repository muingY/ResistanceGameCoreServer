#ifndef DBCONNECTMANAGE_H
#define DBCONNECTMANAGE_H


#include "../NetRootHeader.h"

struct DBUserData
{
    char* server;
    char* user;
    char* password;
    char* database;
};

class DBConnectManage
{
public:
    DBConnectManage();
    ~DBConnectManage();

    bool Initialize(char* server, char* user, char* password, char* database);
    bool Destroy();

    bool DBConnect();
    MYSQL_RES* DBMysqlQuery(char* sqlQuery);

private:
    MYSQL* DBConnectionCore;
    MYSQL_RES* res;
    MYSQL_ROW row;

    DBUserData DBUser;
};


#endif
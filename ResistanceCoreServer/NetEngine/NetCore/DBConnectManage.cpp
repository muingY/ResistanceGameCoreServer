#include "DBConnectManage.h"

DBConnectManage::DBConnectManage()
{
    DBConnectionCore = NULL;
}
DBConnectManage::~DBConnectManage() {}

bool DBConnectManage::Initialize(char* server, char* user, char* password, char* database)
{
    DBUser.server = server;
    DBUser.user = user;
    DBUser.password = password;
    DBUser.database = database;

    return true;
}
bool DBConnectManage::Destroy()
{
    mysql_free_result(res);
    mysql_close(DBConnectionCore);

    return true;
}

bool DBConnectManage::DBConnect()
{
    DBConnectionCore = mysql_init(NULL);

    if (!mysql_real_connect(DBConnectionCore, DBUser.server, DBUser.user, DBUser.password, DBUser.database, 0, NULL, 0))
    {
        cout << "DB > [error] Connection error: " << mysql_error(DBConnectionCore) << endl;
        exit(0);
    }
    cout << "DB > Successfully connected to " << DBUser.database << " database, with " << DBUser.user << " user" << endl;

    return true;
}
MYSQL_RES* DBConnectManage::DBMysqlQuery(char* sqlQuery)
{
    if (mysql_query(DBConnectionCore, sqlQuery))
    {
        cout << "DB > [error] MYSQL query error: " << mysql_error(DBConnectionCore) << endl;
        exit(0);
    }

    return mysql_use_result(DBConnectionCore);
}
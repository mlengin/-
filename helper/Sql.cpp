#include <Windows.h>
#include <string>
#include "common.h"
#include "InlineHook.h"
#include <Shlwapi.h>
#include "offset.h"
#include "resource.h"
#include <CommCtrl.h>
#pragma comment(lib,"Shlwapi.lib")
using namespace std;
string sqlResult = "";

int runSql(string dbName, string sqlStr, sqlite3_callback callBack, char* &sqlErrmsg)
{
	Sqlite3_exec sqlite3_exec = (Sqlite3_exec)(getWeChatWinAddr() + DATABASERUNCALL);
	for (auto& db : getDbHandleList())
	{
		if (StrStrA(db.path, dbName.c_str()))
		{
			sqlResult = "";
			sqlite3_exec(db.handler, (char*)sqlStr.c_str(), callBack, NULL, &sqlErrmsg);
			return 0;
		}
	}
	return 1;
}

INT runSqlCallBack(void* para, int nColumn, char** colValue, char** colName)
{
    char columns[0x100] = { 0 };
    sprintf_s(columns, "nColumns : %d\r\n", nColumn);
    sqlResult = sqlResult + string(columns);
    for (int i = 0; i < nColumn; i++)
    {
        char data[0x1000] = { 0 };
        sprintf_s(data, "%s :%s\r\n", *(colName + i), colValue[i]);
        sqlResult = sqlResult + string(data);
    }
	if (sqlResult == "")
	{
		sqlResult = "查询结果为空，请检查SQL语句是否在正确";
	}
    SetDlgItemTextA(getGlobalHwnd(), SQL_RESULT, sqlResult.c_str());
    return 0;
}
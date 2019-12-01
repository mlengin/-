#include <Windows.h>
#include <string>
#include "common.h"
#include "InlineHook.h"
using namespace std;

int runSql(DWORD dbHandle, string sqlStr, sqlite3_callback callBack, char* &sqlErrmsg)
{
	Sqlite3_exec sqlite3_exec = getSqlExec();
	return sqlite3_exec(dbHandle, (char*)sqlStr.c_str(), callBack, NULL, &sqlErrmsg);
}
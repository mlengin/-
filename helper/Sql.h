#include <Windows.h>
#include <string>
#include "struct.h"
using namespace std;

int runSql(string dbName, string sqlStr, sqlite3_callback callBack, char*& sqlErrmsg);
INT runSqlCallBack(void* para, int nColumn, char** colValue, char** colName);
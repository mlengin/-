#include <Windows.h>
#include <string>
#include "struct.h"
using namespace std;

int runSql(DWORD dbHandle, string sqlStr, sqlite3_callback callBack, char*& sqlErrmsg);
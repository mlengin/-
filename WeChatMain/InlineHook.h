#pragma once
#include <list>
#include "struct.h"
using namespace std;

list<DbHandle> getDbHandleList();
Sqlite3_exec getSqlExec();
void inLineHook();
VOID getContactList();
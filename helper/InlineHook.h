#pragma once
#include <Windows.h>
#include <list>
#include "struct.h"
using namespace std;

list<DbHandle> getDbHandleList();
VOID clearDbHandleList();
void inLineHook();
VOID getContactList();
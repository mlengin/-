#include <Windows.h>
#include <string>
#include <list>
#include "common.h"
#include "offset.h"
#include "struct.h"
#include "resource.h"
#include <CommCtrl.h>
#include "Login.h"
#include <cassert>
#include <Shlwapi.h>
#include "Sql.h"
#pragma comment(lib,"Shlwapi.lib")
using namespace std;

list<DbHandle> DbHandleList;
list<friendStruct> friendList;
Sqlite3_exec sqlite3_exec;

DWORD recieveMsgParam, recieveMsgJmpAddr, databaseJumpAddr;
VOID getContactList();
/**
 * 作者QQ：50728123
 * 交流群：810420984
 * 作者微信：codeByDog
 * 返回数据库句柄list
 * @param
 * @return list<DbHandle>
 */
list<DbHandle> getDbHandleList()
{
	return DbHandleList;
}

Sqlite3_exec getSqlExec()
{
	return sqlite3_exec;
}

INT getContactCallBack(void* para, int nColumn, char** colValue, char** colName)
{
	HWND listHwnd = GetDlgItem(getGlobalHwnd(), FRIEND_LIST);
	LVITEM item = { 0 };
	item.mask = LVIF_TEXT;
	for (int i = 0; i < nColumn; i++)
	{
		if (strcmp(*(colName + i), "UserName") == 0 || strcmp(*(colName + i), "Alias") == 0)
		{
			wchar_t data[0x100] = { 0 };
			swprintf_s(data, L"%S", colValue[i]);
			item.iSubItem = strcmp(*(colName + i), "UserName") == 0 ? 0 : 1;
			item.pszText = data;
			if (strcmp(*(colName + i), "UserName") == 0)
			{
				ListView_InsertItem(listHwnd, &item);
			}
			else
			{
				ListView_SetItem(listHwnd, &item);
			}
		}
		if (strcmp(*(colName + i), "NickName") == 0 || strcmp(*(colName + i), "Remark") == 0)
		{
			char data[200] = { 0 };
			sprintf_s(data, "%s", colValue[i]);
			item.iSubItem = strcmp(*(colName + i), "NickName") == 0 ? 2 : 3;
			item.pszText = UTF8ToUnicode(data);
			ListView_SetItem(listHwnd, &item);
		}
	}
	if (nColumn == 0)
	{
		Sleep(1000);
		HANDLE hookThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)getContactList, NULL, NULL, 0);
		if (hookThread != 0) {
			CloseHandle(hookThread);
		}
	}
	return 0;
}

VOID getContactList()
{
	char* sqlErrmsg = NULL;
	ListView_DeleteAllItems(GetDlgItem(getGlobalHwnd(), FRIEND_LIST));
	for (auto& db : getDbHandleList())
	{
		if (StrStrA(db.path, "MicroMsg.db"))
		{
			int ret = runSql(db.handler, "select * from Contact", getContactCallBack, sqlErrmsg);
		}
	}
}

/**
 * 作者QQ：50728123
 * 交流群：810420984
 * 作者微信：codeByDog
 * 处理拦截到的消息内容
 * @param DWORD esp
 * @return VOID
 */
VOID recieveMessageJump(DWORD esp)
{
	DWORD** msgAddress = (DWORD**)(esp + 0x20);
	recieveMsgStruct* msg = new recieveMsgStruct;
	msg->type = (int)(*((DWORD*)(**msgAddress + 0x30)));
	msg->isSelf = (int)(*((DWORD*)(**msgAddress + 0x34)));
	msg->fromWxid = (wchar_t*)(*((LPVOID*)(**msgAddress + 0x40)));
	msg->content = (wchar_t*)(*((LPVOID*)(**msgAddress + 0x68)));
	msg->senderWxid = (wchar_t*)(*((LPVOID*)(**msgAddress + 0x114)));
	msg->unkonwStr = (wchar_t*)(*((LPVOID*)(**msgAddress + 0x128)));

	HWND listHwnd = GetDlgItem(getGlobalHwnd(), RECIEVE_MSG_LIST);
	LVITEM item = { 0 };
	item.mask = LVIF_TEXT;

	wchar_t type[0x100] = { 0 };
	swprintf_s(type, L"%d", msg->type);
	item.iSubItem = 0;
	item.pszText = type;
	ListView_InsertItem(listHwnd, &item);

	wchar_t isSelf[0x100] = { 0 };
	swprintf_s(isSelf, L"%d", msg->isSelf);
	item.iSubItem = 1;
	item.pszText = isSelf;
	ListView_SetItem(listHwnd, &item);

	item.iSubItem = 2;
	item.pszText = msg->fromWxid;
	ListView_SetItem(listHwnd, &item);

	item.iSubItem = 3;
	item.pszText = msg->senderWxid;
	ListView_SetItem(listHwnd, &item);

	item.iSubItem = 4;
	item.pszText = msg->unkonwStr;
	ListView_SetItem(listHwnd, &item);

	item.iSubItem = 5;
	item.pszText = msg->content;
	ListView_SetItem(listHwnd, &item);
}

/**
 * 作者QQ：50728123
 * 交流群：810420984
 * 作者微信：codeByDog
 * 将获取到的数据库句柄压入list内
 * @param int dbAddress, int dbHandle
 * @return VOID
 */
VOID databaseHwnd(int dbAddress, int dbHandle)
{
	DbHandle Db = { 0 };
	Db.handler = dbHandle;
	sprintf_s(Db.path, "%s", (char*)dbAddress);
	DbHandleList.push_back(Db);
	if (StrStrA(Db.path, "MicroMsg.db"))
	{
		HANDLE hookThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)getContactList, NULL, NULL, 0);
		if (hookThread != 0) {
			CloseHandle(hookThread);
		}
	}

	HWND dbSelect = GetDlgItem(getGlobalHwnd(), DATABASE_SELECT);
	assert(dbSelect);
	wchar_t address[0x100] = { 0 };
	swprintf_s(address, L"%S", (char*)dbAddress);
	wstring dbPath = wstring(address);
	int pos1 = dbPath.find(L"\\Msg\\");
	wstring dbName = dbPath.substr(pos1 + 5);
	SendMessage(dbSelect, CB_ADDSTRING, 0, (LPARAM)dbName.c_str());
}

/**
 * 作者QQ：50728123
 * 交流群：810420984
 * 作者微信：codeByDog
 * 被写入到hook点的接收消息裸函数
 */
__declspec(naked) void recieveMsgDeclspec()
{
	__asm
	{
		mov ecx, recieveMsgParam
		pushad
		push esp
		call recieveMessageJump
		add esp, 4
		popad
		jmp recieveMsgJmpAddr
	}
}

/**
 * 作者QQ：50728123
 * 交流群：810420984
 * 作者微信：codeByDog
 * 被写入到hook点的数据库句柄获取裸函数
 */
__declspec(naked) void databaseDeclspec()
{
	__asm
	{
		mov esi, dword ptr ss : [ebp - 0x14]
		add esp, 0x8
		pushad
		push[ebp - 0x14]
		push[ebp - 0x24]
		call databaseHwnd
		add esp, 8
		popad
		jmp databaseJumpAddr
	}
}

/**
 * 作者QQ：50728123
 * 交流群：810420984
 * 作者微信：codeByDog
 * 使用inLineHook拦截消息和数据库
 * @param
 * @return void
 */
void inLineHook()
{
	//获取模块地址，直到获取为止
	while (true)
	{
		if (getWeChatWinAddr() != 0)
		{
			break;
		}
		Sleep(300);
	}
	sqlite3_exec = (Sqlite3_exec)(getWeChatWinAddr() + DATABASERUNCALL);

	//HOOK接收消息
	DWORD recieveMsgHookAddr = getWeChatWinAddr() + RECEIVEMSGHOOKADDR;
	recieveMsgParam = getWeChatWinAddr() + RECEIVEMSGHOOKPARM;
	recieveMsgJmpAddr = recieveMsgHookAddr + 5;
	BYTE msgJmpCode[5] = { 0xE9 };
	*(DWORD*)&msgJmpCode[1] = (DWORD)recieveMsgDeclspec - recieveMsgHookAddr - 5;
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)recieveMsgHookAddr, msgJmpCode, 5, NULL);
	SetDlgItemText(getGlobalHwnd(), HOOK_MSG_STATUS, L"消息拦截已开启");

	//HOOK数据库句柄
	DWORD hookAddress = getWeChatWinAddr() + DATABASEHOOKADDR;
	databaseJumpAddr = hookAddress + 6;
	BYTE dbJmpCode[6] = { 0xE9 };
	dbJmpCode[6 - 1] = 0x90;
	*(DWORD*)&dbJmpCode[1] = (DWORD)databaseDeclspec - hookAddress - 5;
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)hookAddress, dbJmpCode, 6, NULL);
	SetDlgItemText(getGlobalHwnd(), HOOK_DATABASE_STATUS, L"数据库句柄拦截已开启");
}
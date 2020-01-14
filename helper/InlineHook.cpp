#include <Windows.h>
#include <string>
#include <list>
#include "common.h"
#include "offset.h"
#include "struct.h"
#include "resource.h"
#include <CommCtrl.h>
#include <cassert>
#include <Shlwapi.h>
#include "Sql.h"
#include "AiFun.h"
#pragma comment(lib,"Shlwapi.lib")
using namespace std;

list<DbHandle> DbHandleList;

DWORD recieveMsgCall, recieveMsgJmpAddr, databaseJumpAddr, friendCall, friendJmpAddr;
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

VOID clearDbHandleList()
{
	DbHandleList.clear();
	return;
}


INT getContactCallBack(void* para, int nColumn, char** colValue, char** colName)
{
	HWND listHwnd = GetDlgItem(getGlobalHwnd(), FRIEND_LIST);
	LVITEM item = { 0 };
	item.mask = LVIF_TEXT;
	
	for (int i = 0; i < nColumn; i++)
	{
		char data[1000] = { 0 };
		sprintf_s(data, "%s", colValue[i]);
		item.iSubItem = i;
		item.pszText = UTF8ToUnicode(data);
		if (strcmp(*(colName + i), "UserName") == 0)
		{
			ListView_InsertItem(listHwnd, &item);
		}
		else
		{
			ListView_SetItem(listHwnd, &item);
		}
	}
	return 0;
}

VOID getContactList()
{
	ListView_DeleteAllItems(GetDlgItem(getGlobalHwnd(), FRIEND_LIST));
	char* sqlErrmsg = NULL;
	runSql("MicroMsg.db", "select Contact.UserName,Contact.Alias,Contact.NickName,Contact.Remark,ContactHeadImgUrl.bigHeadImgUrl from Contact LEFT OUTER JOIN ContactHeadImgUrl ON Contact.UserName = ContactHeadImgUrl.usrName", getContactCallBack, sqlErrmsg);
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
	DWORD* msgAddress = (DWORD*)(esp + 0x20);
	recieveMsgStruct* msg = new recieveMsgStruct;
	msg->type = (int)(*((DWORD*)(*msgAddress + 0x128)));
	msg->isSelf = (int)(*((DWORD*)(*msgAddress + 0x12C)));
	msg->fromWxid = (wchar_t*)(*((LPVOID*)(*msgAddress + 0x138)));
	msg->content = (wchar_t*)(*((LPVOID*)(*msgAddress + 0x160)));
	msg->senderWxid = (wchar_t*)(*((LPVOID*)(*msgAddress + 0x210)));
	msg->unkonwStr = (wchar_t*)(*((LPVOID*)(*msgAddress + 0x224)));

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
	item.pszText = msg->content;
	ListView_SetItem(listHwnd, &item);

	char auto_collect_money[0x100] = { 0 };
	GetDlgItemTextA(getGlobalHwnd(), AUTO_COLLECT_MONEY, auto_collect_money, 100);

	char auto_add_user_card[0x100] = { 0 };
	GetDlgItemTextA(getGlobalHwnd(), AUTO_ADD_USER_CARD, auto_add_user_card, 100);

	char auto_agree_new_friend_apply[0x100] = { 0 };
	GetDlgItemTextA(getGlobalHwnd(), AUTO_AGREE_NEW_FRIEND_APPLY, auto_agree_new_friend_apply, 100);

	if (StrStrW(msg->content, L"微信转账") && msg->isSelf == 0 && StrStrA(auto_collect_money, "开启"))
	{
		AutoCollectMoney(msg->fromWxid, msg->content);
	}
	else if (msg->type == 42 && StrStrA(auto_add_user_card, "开启"))
	{
		AutoAddUserFromCard(msg->content);
	}
	else if (msg->type == 37 && StrStrA(auto_agree_new_friend_apply, "开启"))
	{
		AutoAgreeUserRequest(msg->content);
	}
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
		pushad
		push esp
		call recieveMessageJump
		add esp, 4
		popad
		call recieveMsgCall
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



void friendListCall(DWORD esi)
{
	//HWND listHwnd = GetDlgItem(getGlobalHwnd(), FRIEND_LIST);
	//LVITEM item = { 0 };
	//item.mask = LVIF_TEXT;

	//LPVOID wxid = *((LPVOID*)(esi + 0x10));
	//item.iSubItem = 0;
	//item.pszText = (wchar_t*)wxid;
	//ListView_InsertItem(listHwnd, &item);

	//LPVOID account = *((LPVOID*)(esi + 0x44));
	//item.iSubItem = 1;
	//item.pszText = (wchar_t*)account;
	//ListView_SetItem(listHwnd, &item);

	//LPVOID nickname = *((LPVOID*)(esi + 0xE0));
	//item.iSubItem = 2;
	//item.pszText = (wchar_t*)nickname;
	//ListView_SetItem(listHwnd, &item);

	//LPVOID remaker = *((LPVOID*)(esi + 0xCC));
	//item.iSubItem = 3;
	//item.pszText = (wchar_t*)remaker;
	//ListView_SetItem(listHwnd, &item);
}


__declspec(naked) void friendDeclspec()
{
	__asm
	{
		pushad
		push esi
		call friendListCall
		add esp, 4
		popad
		call friendCall
		jmp friendJmpAddr
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

	//HOOK接收消息
	DWORD recieveMsgHookAddr = getWeChatWinAddr() + RECIEVEHOOKADDR;
	recieveMsgCall = getWeChatWinAddr() + RECIEVEHOOKCALL;
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

	//HOOK好友列表
	DWORD friendHookAddr = getWeChatWinAddr() + FRIENDHOOKADDR;
	friendCall = getWeChatWinAddr() + FRIENDHOOKCALL;
	friendJmpAddr = friendHookAddr + 5;
	BYTE friendJmpCode[5] = { 0xE9 };
	*(DWORD*)&friendJmpCode[1] = (DWORD)friendDeclspec - friendHookAddr - 5;
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)friendHookAddr, friendJmpCode, 5, NULL);
}
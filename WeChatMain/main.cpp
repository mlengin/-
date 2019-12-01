#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include "resource.h"
#include "struct.h"
#include "common.h"
#include "MyInfo.h"
#include "Login.h"
#include "InlineHook.h"
#include <CommCtrl.h>
#include <fstream>
#include "SendMsg.h"
#include "Sql.h"
#include <Shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")
using namespace std;
string sqlResult = "";

DWORD WINAPI ShowDialog(
	_In_ HMODULE hModule
);
INT_PTR CALLBACK DialogProc(
	_In_ HWND   hwndDlg,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);
INT runSqlCallBack(void* para, int nColumn, char** colValue, char** colName);

/**
 * 作者QQ：50728123
 * 交流群：810420984
 * 作者微信：codeByDog
 * 入口函数
 * @param HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved
 * @return BOOL
 */
BOOL APIENTRY DllMain(
	HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		HANDLE lThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShowDialog, hModule, NULL, 0);
		if (lThread != 0) {
			CloseHandle(lThread);
		}
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

/**
 * 作者QQ：50728123
 * 交流群：810420984
 * 作者微信：codeByDog
 * 线程启动窗口，防止微信卡顿
 * @param _In_ HMODULE hModule
 * @return DWORD
 */
DWORD WINAPI ShowDialog(
	_In_ HMODULE hModule
)
{
	DialogBox(hModule, MAKEINTRESOURCE(WECHAT_MAIN), NULL, &DialogProc);
	return TRUE;
}

/**
 * 作者QQ：50728123
 * 交流群：810420984
 * 作者微信：codeByDog
 * 显示窗口
 * @param _In_ HWND   hwndDlg,_In_ UINT   uMsg,_In_ WPARAM wParam,_In_ LPARAM lParam
 * @return INT_PTR
 */
INT_PTR CALLBACK DialogProc(
	_In_ HWND   hwndDlg,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	if (uMsg == WM_INITDIALOG)
	{
		setGlobalHwnd(hwndDlg);
		SetDlgItemText(hwndDlg, RECEIVE_WXID_TEXT, L"filehelper");

		SetDlgItemText(hwndDlg, SQL_RESULT, L"hook写入成功，已开始监听数据库句柄");
		SetDlgItemText(hwndDlg, DB_NAME, L"MicroMsg.db");
		SetDlgItemText(hwndDlg, SQL_TEXT, L"select * from sqlite_master");
		//初始化消息接收list
		LV_COLUMN msgPcol = { 0 };
		LPCWSTR msgTitle[] = { L"类型",L"self",L"来源",L"发送者", L"字符串", L"详情" };
		int msgCx[] = {50,50,80,80,50,200};
		msgPcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
		msgPcol.fmt = LVCFMT_LEFT;
		for (unsigned int i = 0; i < size(msgTitle); i++) {
			msgPcol.pszText = (LPWSTR)msgTitle[i];
			msgPcol.cx = msgCx[i];
			ListView_InsertColumn(GetDlgItem(hwndDlg, RECIEVE_MSG_LIST), i, &msgPcol);
		}
		LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(RECIEVE_MSG_LIST);

		//初始化数据库句柄list
		LV_COLUMN dbPcol = { 0 };
		LPCWSTR dbTitle[] = { L"句柄",L"数据库地址" };
		int dbCx[] = { 80,350 };
		dbPcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
		dbPcol.fmt = LVCFMT_LEFT;
		for (unsigned int i = 0; i < size(dbTitle); i++) {
			dbPcol.pszText = (LPWSTR)dbTitle[i];
			dbPcol.cx = dbCx[i];
			ListView_InsertColumn(GetDlgItem(hwndDlg, DATABSE_HANDLE_LIST), i, &dbPcol);
		}

		//初始化好友列表list
		LV_COLUMN friendPcol = { 0 };
		LPCWSTR friendTitle[] = { L"wxid",L"账号",L"昵称",L"备注",L"头像"};
		int friendCx[] = { 80,80,80,80,80 };
		friendPcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
		friendPcol.fmt = LVCFMT_LEFT;
		for (unsigned int i = 0; i < size(friendTitle); i++) {
			friendPcol.pszText = (LPWSTR)friendTitle[i];
			friendPcol.cx = friendCx[i];
			ListView_InsertColumn(GetDlgItem(hwndDlg, FRIEND_LIST), i, &friendPcol);
		}

		HANDLE lThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)getLoginStatus, NULL, NULL, 0);
		if (lThread != 0) {
			CloseHandle(lThread);
		}

		HANDLE hookThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)inLineHook, NULL, NULL, 0);
		if (hookThread != 0) {
			CloseHandle(hookThread);
		}
	}
	if (uMsg == WM_COMMAND)
	{
		if (wParam == GET_MY_INFO)
		{
			if (isLogin() == 0)
			{
				SetDlgItemText(hwndDlg, MY_INFO_TEXT, L"请先登录微信");
			}
			else
			{
				Information* myInfo = GetMyInfo();
				wchar_t str[0x1000] = { 0 };
				swprintf_s(str,
					L"微信ID：%s\r\n账号：%s\r\n昵称：%s\r\n设备：%s\r\n手机号：%s\r\n邮箱：%s\r\n性别：%s\r\n国籍：%s\r\n省份：%s\r\n城市：%s\r\n签名：%s\r\n头像：%s",
					myInfo->wxid,
					myInfo->account,
					myInfo->nickname,
					myInfo->device,
					myInfo->phone,
					myInfo->email,
					myInfo->sex,
					myInfo->nation,
					myInfo->province,
					myInfo->city,
					myInfo->signName,
					myInfo->header);
				SetDlgItemText(hwndDlg, MY_INFO_TEXT, str);
			}
		}
		else if (wParam == AUTO_CLLECT_MONEY || wParam == AUTO_AGREE_NEW_FRIEND_APPLY || wParam == AUTO_DOWNLOAD_IMAGE || wParam == DELETE_FIREND)
		{
			MessageBox(NULL, L"功能开发中，请保持关注，GITHUB持续更新中", L"客官不要急嘛", 0);
		}
		else if (wParam == SEND_TEXT_BTN || wParam == SEND_IMG_BTN || wParam == SEND_ATTACH_BTN)
		{
			if (isLogin() == 0)
			{
				MessageBox(NULL, L"请先登录微信", L"错误", 0);
				return 0;
			}
			wchar_t wxid[0x100] = {0};
			wchar_t content[0x1000] = {0};
			GetDlgItemText(hwndDlg, RECEIVE_WXID_TEXT, wxid, 100);
			GetDlgItemText(hwndDlg, RECEIVE_CONTENT_TEXT, content, 1000);
			if (wParam != SEND_TEXT_BTN && _waccess(content, 0) == -1)
			{
				MessageBox(NULL, L"文件不存在", L"错误", 0);
				return 0;
			}
			switch (wParam)
			{
				case SEND_IMG_BTN:
					SendImageMsg(wxid, content);
					break;
				case SEND_ATTACH_BTN:
					SendAttachMsg(wxid, content);
					break;
				default:
					SendTextMsg(wxid, content);
					break;
			}
		}
		else if (wParam == RUN_SQL_BTN)
		{
			char dbName[0x100] = { 0 };
			GetDlgItemTextA(hwndDlg, DATABASE_SELECT, dbName, 100);
			char sql[0x1000] = { 0 };
			GetDlgItemTextA(hwndDlg, SQL_TEXT, sql, 1000);
			char* sqlErrmsg = NULL;
			sqlResult = "";
			for (auto& db : getDbHandleList())
			{
				if (StrStrA(db.path, dbName))
				{
					int ret = runSql(db.handler, string(sql), runSqlCallBack, sqlErrmsg);
				}
			}
		}
		else if (wParam == GET_CONTACT_LIST)
		{
			getContactList();
		}
	}
	else if (uMsg == WM_CLOSE)
	{
		EndDialog(hwndDlg, 0);
	}
	return FALSE;
}

INT runSqlCallBack(void* para, int nColumn, char** colValue, char** colName)
{
	char columns[0x100] = { 0 };
	sprintf_s(columns, "nColumns : %d\r\n", nColumn);
	sqlResult = sqlResult + string(columns);
	for (int i = 0; i < nColumn; i++)
	{
		char data[0x2048] = { 0 };
		sprintf_s(data, "%s :%s\r\n", *(colName + i), colValue[i]);
		sqlResult = sqlResult + string(data);
	}
	SetDlgItemTextA(getGlobalHwnd(), SQL_RESULT, sqlResult.c_str());

	//for (int i = 0; i < nColumn; i++)
	//{
	//	char name[0x100] = { 0 };
	//	sprintf_s(name, "%s", *(colName + i));
	//	if (strcmp(name, "sql") == 0)
	//	{
	//		char value[0x2048] = { 0 };
	//		sprintf_s(value, "%s\r\n", colValue[i]);
	//		sqlResult = sqlResult + string(value);
	//	}
	//}
	//SetDlgItemTextA(getGlobalHwnd(), SQL_RESULT, sqlResult.c_str());
	return 0;
}
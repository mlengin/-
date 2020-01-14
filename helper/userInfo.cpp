#include "common.h"
#include <Shlwapi.h>
#include "struct.h"
#include "offset.h"
#pragma comment(lib,"Shlwapi.lib")

/**
 * 作者QQ：50728123
 * 交流群：810420984
 * 作者微信：codeByDog
 * 获取当前登录用户的个人信息
 * @param
 * @return Information* (结构体)
 */
Information* getMyInfo()
{
	Information* info = new Information;
	//微信ID
	if (IsBadReadPtr((char*)(*(DWORD*)(getWeChatWinAddr() + MY_WXID)), sizeof(char*)))
	{
		swprintf_s(info->wxid, L"%S", (char*)(getWeChatWinAddr() + MY_WXID));
	}
	else
	{
		swprintf_s(info->wxid, L"%S", (char*)(*(DWORD*)(getWeChatWinAddr() + MY_WXID)));
	}

	//昵称 可能存在汉字字符，所以使用wcscpy_s
	char nickName[0x500] = { 0 };
	if (IsBadReadPtr((char*)(*(DWORD*)(getWeChatWinAddr() + MY_NICKNAME)), sizeof(char*)))
	{
		sprintf_s(nickName, "%s", (char*)(getWeChatWinAddr() + MY_NICKNAME));
	}
	else
	{
		sprintf_s(nickName, "%s", (char*)(*(DWORD*)(getWeChatWinAddr() + MY_NICKNAME)));
	}
	wcscpy_s(info->nickname, wcslen(UTF8ToUnicode(nickName)) + 1, UTF8ToUnicode(nickName));

	//个性签名 可能存在汉字字符，所以使用wcscpy_s
	char signName[0x500] = { 0 };
	if (IsBadReadPtr((char*)(*(DWORD*)(getWeChatWinAddr() + MY_SIGNNAME)), sizeof(char*)))
	{
		sprintf_s(signName, "%s", (char*)(getWeChatWinAddr() + MY_SIGNNAME));
	}
	else
	{
		sprintf_s(signName, "%s", (char*)(*(DWORD*)(getWeChatWinAddr() + MY_SIGNNAME)));
	}
	wcscpy_s(info->signName, wcslen(UTF8ToUnicode(signName)) + 1, UTF8ToUnicode(signName));

	//自定义账号
	swprintf_s(info->account, L"%S", (char*)(getWeChatWinAddr() + MY_ACCOUNT));

	//登陆设备
	swprintf_s(info->device, L"%S", (char*)(getWeChatWinAddr() + MY_DEVICE));

	//手机号
	swprintf_s(info->phone, L"%S", (char*)(getWeChatWinAddr() + MY_MOBILE));

	//邮箱
	swprintf_s(info->email, L"%S", (char*)(getWeChatWinAddr() + MY_EMAIL));

	//性别
	info->sex =(int)(*((DWORD*)(getWeChatWinAddr() + MY_SEX)));

	//国籍
	swprintf_s(info->nation, L"%S", (char*)(getWeChatWinAddr() + MY_NATION));

	//省份
	swprintf_s(info->province, L"%S", (char*)(getWeChatWinAddr() + MY_PROVINCE));

	//城市
	swprintf_s(info->city, L"%S", (char*)(getWeChatWinAddr() + MY_CITY));

	//头像
	swprintf_s(info->bigHeader, L"%S", (char*)(*(DWORD*)(getWeChatWinAddr() + MY_BIGHEAD)));

	return info;
}

VOID getUserInfo(wchar_t* wxid)
{
	struct wxMsg
	{
		wchar_t* wxid;
		int msgLen;
		int buffLen;
	};
	wxMsg id = { 0 };
	id.wxid = wxid;
	id.msgLen = wcslen(wxid);
	id.buffLen = wcslen(wxid) * 2;

	DWORD getUserDetail = getWeChatWinAddr() + 0x518A0;

	DWORD aaax = 0;
	char* pWxid = (char*)&id.wxid;
	__asm
	{
		push pWxid
		call getUserDetail
		mov aaax, eax
	}
	MessageBoxA(NULL, "查询成功", "提示", 0);
}


void UpdateMemberDetails(wchar_t* memberwxid)
{
	//群ID结构体 
	struct RoomIdStruct
	{
		wchar_t* roomid;
		int roomidLen;
		int roomidMaxLen;
		int full = 0;
		int full2 = 0;
	};

	//拿到call的数据地址 
	DWORD dwCall1 = getWeChatWinAddr() + WxUpdateMemberDetails1;
	DWORD dwCall2 = getWeChatWinAddr() + WxUpdateMemberDetails2;
	DWORD dwCall3 = getWeChatWinAddr() + WxUpdateMemberDetails3;
	DWORD dwCall4 = getWeChatWinAddr() + WxUpdateMemberDetails4;
	DWORD dwCall5 = getWeChatWinAddr() + WxUpdateMemberDetails5;
	DWORD dwCall6 = getWeChatWinAddr() + WxUpdateMemberDetails6;

	//dwCall6 = *((DWORD *)dwCall6);
	//DWORD buffUser = dwCall6 + 0xC00;
	//nof[0] = (DWORD)&dwCall6;
	//nof[0xA20] = (DWORD)&dwCall7;
	//nof[0xA24] = (DWORD)&dwCall8;
	//nof[0xA88] = (DWORD)&dwCall9;

	//组装群ID结构体 

	//DWORD * dwCall7=&dwCall6;
	char buff[0xc00] = { 0 };
	char* asmBuff = buff;

	//组装群ID结构体 
	RoomIdStruct roomiddata;
	roomiddata.roomid = memberwxid;
	roomiddata.roomidLen = wcslen(memberwxid) + 1;
	roomiddata.roomidMaxLen = (wcslen(memberwxid) + 1) * 2;

	//组装微信ID结构体 
	char* asmChatRoom = (char*)&roomiddata.roomid;
	//char * asmesiData = (char *)esidata.Call1;

	__asm
	{
		pushad;
		mov edi, asmBuff
		push edi
		sub esp, 0x14
		mov eax, asmChatRoom
		mov ecx, esp
		push eax
		call dwCall1
		call dwCall2
		call dwCall3
		sub esp, 0x3E0
		mov ecx, esp
		push edi
		call dwCall4
		lea esi, dwCall6
		mov ecx, esi
		call dwCall5
		popad;
	}
	MessageBoxA(NULL,"信息更新成功","提示",0);
}

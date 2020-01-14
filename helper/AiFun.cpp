#include <Windows.h>
#include <string>
#include "common.h"
#include "offset.h"
using namespace std;

void CollectMoney(wchar_t* wxid, wchar_t* transferid)
{
	struct CllectMoneyStruct
	{
		wchar_t* ptransferid;
		int transferidLen;
		int transferidMaxLen;
		char full[0x8] = { 0 };
		wchar_t* pwxid;
		int wxidLen;
		int wxidMaxLen;
		char full2[0x8] = { 0 };
	};

	CllectMoneyStruct cllect;
	cllect.ptransferid = transferid;
	cllect.transferidLen = wcslen(transferid) + 1;
	cllect.transferidMaxLen = (wcslen(transferid) + 1) * 2;
	cllect.pwxid = wxid;
	cllect.wxidLen = wcslen(wxid) + 1;
	cllect.wxidMaxLen = (wcslen(wxid) + 1) * 2;

	char* asmBuff = (char*)&cllect.ptransferid;

	DWORD dwCall1 = getWeChatWinAddr() + COLLECTMONEYCALL1;
	DWORD dwCall2 = getWeChatWinAddr() + COLLECTMONEYCALL2;


	__asm
	{
		sub esp, 0x30;
		mov ecx, esp;
		mov eax, asmBuff;
		push eax;
		call dwCall1;
		call dwCall2;
		add esp, 0x30;
	}
}

void AutoCollectMoney(wchar_t* wxid, wstring msg)
{
	int pos1 = msg.find(L"<transferid>");
	int pos2 = msg.find(L"]]></transferid>");
	wstring noneed = L"<transferid><![CDATA[";
	int noneedLen = noneed.length();
	wstring transferid;
	transferid = msg.substr(pos1 + noneedLen, (pos2 - pos1) - noneedLen);
	CollectMoney(wxid, (wchar_t*)transferid.c_str());
}

void AddUserFromWxid(wchar_t* wxid, wchar_t* msg)
{
	DWORD dwParam1 = getWeChatWinAddr() + ADDUSERFROMWXIDPARAM;
	DWORD dwCall1 = getWeChatWinAddr() + ADDUSERFROMWXIDCALL1;
	DWORD dwCall2 = getWeChatWinAddr() + ADDUSERFROMWXIDCALL2;
	DWORD dwCall3 = getWeChatWinAddr() + ADDUSERFROMWXIDCALL3;
	DWORD dwCall4 = getWeChatWinAddr() + ADDUSERFROMWXIDCALL4;
	DWORD dwCall5 = getWeChatWinAddr() + ADDUSERFROMWXIDCALL5;

	struct TextStruct
	{
		wchar_t* pStr;
		int strLen;
		int strMaxLen;

	};

	TextStruct pWxid = { 0 };
	pWxid.pStr = wxid;
	pWxid.strLen = wcslen(wxid) + 1;
	pWxid.strMaxLen = (wcslen(wxid) + 1) * 2;


	TextStruct pMsg = { 0 };
	pMsg.pStr = msg;
	pMsg.strLen = wcslen(msg) + 1;
	pMsg.strMaxLen = (wcslen(msg) + 1) * 2;

	char* asmWxid = (char*)&pWxid.pStr;
	char* asmMsg = (char*)&pMsg.pStr;
	DWORD asmMsgText = (DWORD)pMsg.pStr;
	char buff3[0x100] = { 0 };
	char* buff = buff3;
	__asm
	{
		sub esp, 0x18;
		mov ecx, esp;
		mov dword ptr ss : [ebp - 0xDC] , esp;
		push  dwParam1;
		call dwCall1;
		sub esp, 0x18;
		mov eax, buff;
		mov dword ptr ss : [ebp - 0xE4] , esp;
		mov ecx, esp;
		push eax;
		call dwCall2;
		push 0x6;
		sub esp, 0x14;
		mov ecx, esp;
		mov dword ptr ss : [ebp - 0xE8] , esp;
		push - 0x1;
		mov edi, asmMsgText;
		push edi;
		call dwCall3;
		mov eax, asmMsg;
		push 0x2;
		sub esp, 0x14;
		mov ecx, esp;
		mov dword ptr ss : [ebp - 0xE0] , esp;
		mov ebx, asmWxid;
		push ebx;
		call dwCall4;
		mov ecx, eax;
		call dwCall5;
	}
}

void AddUserFromCard(wchar_t* v1, wchar_t* msg)
{
	DWORD addParam = getWeChatWinAddr() + ADDCARDUSERPARAM;
	DWORD addCall1 = getWeChatWinAddr() + ADDCARDUSERCALL1;
	DWORD addCall2 = getWeChatWinAddr() + ADDCARDUSERCALL2;
	DWORD addCall3 = getWeChatWinAddr() + ADDCARDUSERCALL3;
	DWORD addCall4 = getWeChatWinAddr() + ADDCARDUSERCALL4;
	DWORD addCall5 = getWeChatWinAddr() + ADDCARDUSERCALL5;

	struct TextStruct
	{
		wchar_t* pStr;
		int strLen;
		int strMaxLen;

	};

	TextStruct pV1 = { 0 };
	pV1.pStr = v1;
	pV1.strLen = wcslen(v1) + 1;
	pV1.strMaxLen = (wcslen(v1) + 1) * 2;


	char* asmV1 = (char*)&pV1.pStr;
	char buff3[0x100] = { 0 };
	char* buff = buff3;
	__asm
	{
		sub esp, 0x18;
		mov ecx, esp;
		mov dword ptr ss : [ebp - 0xDC] , esp;
		push  addParam;
		call addCall1;
		sub esp, 0x18;
		mov eax, buff;
		mov dword ptr ss : [ebp - 0xE4] , esp;
		mov ecx, esp;
		push eax;
		call addCall2;
		push 0x11;
		sub esp, 0x14;
		mov ecx, esp;
		mov dword ptr ss : [ebp - 0xE8] , esp;
		push - 0x1;
		mov edi, msg;
		push edi;
		call addCall3;
		push 0x2;
		sub esp, 0x14;
		mov ecx, esp;
		mov dword ptr ss : [ebp - 0xE0] , esp;
		mov ebx, asmV1;
		push ebx;
		call addCall4;
		mov ecx, eax;
		call addCall5;
	}
}

void AutoAddUserFromCard(wstring msg)
{
	int v1strat = msg.find(L"v1_");
	int v1end = msg.find(L"@stranger");
	wstring v1;
	v1 = msg.substr(v1strat, v1end - v1strat + 9);
	AddUserFromCard((wchar_t*)v1.c_str(), (wchar_t*)L"Hi~");
}

void AgreeUserRequest(wchar_t* v1, wchar_t* v2)
{
	struct v1Info
	{
		int fill = 0;
		wchar_t* v1 = 0;
		int v1Len;
		int maxV1Len;
		char fill2[0x41C] = { 0 };
		DWORD v2 = { 0 };
	};

	struct v2Info
	{
		char fill[0x24C] = { 0 };
		DWORD fill3 = 0x25;
		char fill4[0x40] = { 0 };
		wchar_t* v2;
		int v2Len;
		int maxV2Len;
		char fill2[0x8] = { 0 };
	};

	DWORD params = getWeChatWinAddr() + AGREEUSERREQUESTPARAM;
	DWORD callAdd1 = getWeChatWinAddr() + AGREEUSERREQUESTCALL1;
	DWORD callAdd2 = getWeChatWinAddr() + AGREEUSERREQUESTCALL2;
	DWORD callAdd3 = getWeChatWinAddr() + AGREEUSERREQUESTCALL3;
	DWORD callAdd4 = getWeChatWinAddr() + AGREEUSERREQUESTCALL4;

	DWORD* asmP = (DWORD*)params;

	v1Info userInfoV1 = { 0 };
	v2Info userInfoV2 = { 0 };
	userInfoV1.v2 = (DWORD)&userInfoV2.fill;
	userInfoV1.v1 = v1;
	userInfoV1.v1Len = wcslen(v1);
	userInfoV1.maxV1Len = wcslen(v1) * 2;
	userInfoV2.v2 = v2;
	userInfoV2.v2Len = wcslen(v2);
	userInfoV2.maxV2Len = wcslen(v2) * 2;

	char* asmUser = (char*)&userInfoV1.fill;
	char buff[0x14] = { 0 };
	char buff2[0x48] = { 0 };
	char* asmBuff = buff2;

	__asm
	{
		mov ecx, asmUser;
		push 0x6;
		sub esp, 0x14;
		push esp;
		call callAdd1;
		mov ecx, asmUser;
		lea eax, buff;
		push eax;
		call callAdd2;
		mov esi, eax;
		sub esp, 0x8;
		mov ecx, asmP;
		call callAdd3;
		mov ecx, asmBuff;
		mov edx, ecx;
		push edx;
		push eax;
		push esi;
		call callAdd4;
	}
}

void AutoAgreeUserRequest(wstring msg)
{
	int v1strat = msg.find(L"v1_");
	int v1end = msg.find(L"@stranger");
	wstring v1;
	v1 = msg.substr(v1strat, v1end - v1strat + 9);
	int v2strat = msg.find(L"v2_");
	int v2end = msg.rfind(L"@stranger");
	wstring v2;
	v2 = msg.substr(v2strat, v2end - v2strat + 9);
	AgreeUserRequest((wchar_t*)v1.c_str(), (wchar_t*)v2.c_str());
}

void DeleteFriend(wchar_t* wxid)
{
	struct StructWxid
	{
		wchar_t* pWxid;
		DWORD length;
		DWORD maxLength;
		DWORD fill1;
		DWORD fill2;
	};
	StructWxid structWxid = { 0 };
	structWxid.pWxid = wxid;
	structWxid.length = wcslen(wxid);
	structWxid.maxLength = wcslen(wxid) * 2;

	DWORD* asmMsg = (DWORD*)&structWxid.pWxid;
	DWORD dwCallAddr = getWeChatWinAddr() + DELETEFRIEND;

	__asm {
		mov ecx, 0;
		push ecx;
		mov edi, asmMsg;
		push edi;
		call  dwCallAddr;
	}
}
#include <Windows.h>
#include <string>
#include "common.h"
#include "offset.h"
using namespace std;

/**
 * 作者QQ：50728123
 * 交流群：810420984
 * 作者微信：codeByDog
 * 发送文本消息
 * @param wchar_t* wxid, string content
 * @return void
 */
void SendTextMsg(wchar_t* wxid, wchar_t* msg)
{
	struct wxMsg
	{
		wchar_t* pMsg;
		int msgLen;
		int buffLen;
		int fill1 = 0;
		int fill2 = 0;
	};

	DWORD dwSendCallAddr = getWeChatWinAddr() + SENDTEXTADDR;

	wxMsg id = { 0 };
	id.pMsg = wxid;
	id.msgLen = wcslen(wxid);
	id.buffLen = wcslen(wxid) * 2;

	wxMsg text = { 0 };
	text.pMsg = msg;
	text.msgLen = wcslen(msg);
	text.buffLen = wcslen(msg) * 2;

	char* pWxid = (char*)&id.pMsg;
	char* pWxmsg = (char*)&text.pMsg;

	char buff[0x81C] = { 0 };

	__asm {
		mov edx, pWxid;
		push 0x1;
		push 0;
		mov ebx, pWxmsg;
		push ebx;
		lea ecx, buff;
		call dwSendCallAddr;
		add esp, 0xC;
	}
}

/**
 * 作者QQ：50728123
 * 交流群：810420984
 * 作者微信：codeByDog
 * 发送图片文件
 * @param wchar_t* wxid, string content
 * @return void
 */
void SendImageMsg(wchar_t* wxid, wchar_t* filepath)
{
	struct SendImageStruct
	{
		wchar_t* pWxid = nullptr;
		DWORD length = 0;
		DWORD maxLength = 0;
		DWORD fill1 = 0;
		DWORD fill2 = 0;
		wchar_t wxid[1024] = { 0 };
	};

	//组装微信ID的数据结构
	SendImageStruct imagewxid;
	memcpy(imagewxid.wxid, wxid, wcslen(wxid) + 1);
	imagewxid.length = wcslen(wxid) + 1;
	imagewxid.maxLength = wcslen(wxid) * 2;
	imagewxid.pWxid = wxid;

	//组装文件路径的数据结构
	SendImageStruct imagefilepath;
	memcpy(imagefilepath.wxid, filepath, wcslen(filepath) + 1);
	imagefilepath.length = wcslen(filepath) + 1;
	imagefilepath.maxLength = MAX_PATH;
	imagefilepath.pWxid = filepath;

	char buff[0x45C] = { 0 };

	DWORD dwCall1 = getWeChatWinAddr() + SENDIMGADDR1;
	DWORD dwCall2 = getWeChatWinAddr() + SENDIMGADDR2;
	DWORD myEsp = 0;

	__asm
	{
		mov myEsp, esp;

		lea ebx, imagefilepath;
		push ebx;
		lea eax, imagewxid;
		push eax;
		lea eax, buff;
		push eax;
		push 0;
		call dwCall1;
		add esp, 0x4;
		mov ecx, eax;
		call dwCall2;

		mov eax, myEsp;
		mov esp, eax;
	}
}

/**
 * 作者QQ：50728123
 * 交流群：810420984
 * 作者微信：codeByDog
 * 发送附件文件
 * @param wchar_t* wxid, string content
 * @return void
 */
void SendAttachMsg(wchar_t* wxid, wchar_t* filepath)
{
	struct Wxid
	{
		wchar_t* str;
		int strLen = 0;
		int maxLen = 0;
		char file[0x8] = { 0 };
	};

	struct filePath
	{
		wchar_t* str;
		int strLen = 0;
		int maxLen = 0;
		char file[0x18] = { 0 };
	};

	DWORD dwCall1 = getWeChatWinAddr() + SENDATTACHADDR1;
	DWORD dwCall2 = getWeChatWinAddr() + SENDATTACHADDR2;
	DWORD dwCall3 = getWeChatWinAddr() + SENDATTACHADDR3;
	DWORD dwCall4 = getWeChatWinAddr() + SENDATTACHADDR4;
	DWORD dwParams = getWeChatWinAddr() + SENDATTACHPARAM;

	char buff[0x45C] = { 0 };
	Wxid wxidStruct = { 0 };
	wxidStruct.str = wxid;
	wxidStruct.strLen = wcslen(wxid);
	wxidStruct.maxLen = wcslen(wxid) * 2;

	filePath filePathStruct = { 0 };
	filePathStruct.str = filepath;
	filePathStruct.strLen = wcslen(filepath);
	filePathStruct.maxLen = wcslen(filepath) * 2;

	char* pFilePath = (char*)&filePathStruct.str;
	char* pWxid = (char*)&wxidStruct.str;

	__asm {
		push dword ptr ss : [ebp - 0x5C] ;
		sub esp, 0x14;
		mov ecx, esp;
		push - 0x1;
		push dwParams;
		call dwCall1;
		sub esp, 0x14;
		mov ecx, esp;
		push pFilePath;
		call dwCall2;
		sub esp, 0x14;
		mov ecx, esp;
		push pWxid;
		call dwCall2;
		lea eax, buff;
		push eax;
		call dwCall3;
		mov ecx, eax;
		call dwCall4;
	}
}

void SendXmlCard(wchar_t* RecverWxid, wchar_t* xmlData)
{
	struct stringData
	{
		wchar_t* content;
		int strLen;
		int strMaxLen;
	};
	stringData wxid = { 0 };
	wxid.content = RecverWxid;
	wxid.strLen = wcslen(RecverWxid);
	wxid.strMaxLen = wcslen(RecverWxid) * 2;

	stringData xml = { 0 };
	xml.content = xmlData;
	xml.strLen = wcslen(xmlData);
	xml.strMaxLen = wcslen(RecverWxid) * 2;

	char* wxidContent = (char*)&wxid.content;
	char* xmlContent = (char*)&xml.content;
	char buff[0x20C] = { 0 };
	DWORD callAdd = getWeChatWinAddr() + SENDXMLCARD;

	__asm {
		mov eax, xmlContent
		push 0x2A
		mov edx, wxidContent
		push 0x0
		push eax
		lea ecx, buff
		call callAdd
		add esp, 0xC
	}
}

void SendXmlArticle(wchar_t* RecverWxid, wchar_t* FromWxid, wchar_t* xmlData)
{
	struct stringData
	{
		wchar_t* content;
		int strLen;
		int strMaxLen;
	};
	stringData wxid = { 0 };
	wxid.content = RecverWxid;
	wxid.strLen = wcslen(RecverWxid);
	wxid.strMaxLen = wcslen(RecverWxid) * 2;

	stringData fromWxid = { 0 };
	fromWxid.content = FromWxid;
	fromWxid.strLen = wcslen(FromWxid);
	fromWxid.strMaxLen = wcslen(FromWxid) * 2;

	stringData xml = { 0 };
	xml.content = xmlData;
	xml.strLen = wcslen(xmlData);
	xml.strMaxLen = 0x800;

	char* wxidContent = (char*)&wxid.content;
	char* wxidFContent = (char*)&fromWxid.content;
	char* xmlContent = (char*)&xml.content;

	char buff1[0x24] = { 0 };
	char buff2[0x48] = { 0 };
	char buff3[0x248] = { 0 };
	DWORD callAddr = getWeChatWinAddr() + SENDXMLARTICLE;
	__asm {
		lea eax, buff1
		push 0x5
		lea edx, wxidFContent
		push eax
		lea eax, buff2
		push eax
		push xmlContent
		push wxidContent
		lea ecx, buff3
		call callAddr
		add esp, 0x14
	}
}
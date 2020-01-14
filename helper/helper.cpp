#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "resource.h"
#include "userInfo.h"
#include "Login.h"
#include "InlineHook.h"
#include "common.h"
#include <CommCtrl.h>
#include "SendMsg.h"
#include <Shlwapi.h>
#include "Sql.h"
#include "AiFun.h"
#include "chatroom.h"
#pragma comment(lib,"Shlwapi.lib")
using namespace std;

//----------声明 Start----------
BOOL ShowDialog(HMODULE hModule);
INT_PTR CALLBACK DialogProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

//----------声明 Over----------

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
BOOL ShowDialog(HMODULE hModule)
{
    DialogBox(hModule, MAKEINTRESOURCE(DEBUG_MAIN), NULL, &DialogProc);
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
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        setGlobalHwnd(hDlg);
        SetDlgItemText(hDlg, SQL_TEXT, L"select * from sqlite_master");
        SetDlgItemText(hDlg, SQL_RESULT, L"hook写入成功，已开始监听数据库句柄");
        SetDlgItemText(hDlg, RECEIVE_WXID_TEXT, L"filehelper");
        //初始化消息接收list
        LV_COLUMN msgPcol = { 0 };
        LPCWSTR msgTitle[] = { L"类型",L"self",L"来源",L"发送者", L"详情" };
        int msgCx[] = { 40,40,80,80,200 };
        msgPcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
        msgPcol.fmt = LVCFMT_LEFT;
        for (unsigned int i = 0; i < size(msgTitle); i++) {
            msgPcol.pszText = (LPWSTR)msgTitle[i];
            msgPcol.cx = msgCx[i];
            ListView_InsertColumn(GetDlgItem(hDlg, RECIEVE_MSG_LIST), i, &msgPcol);
        }
        LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(RECIEVE_MSG_LIST);

        //初始化好友列表list
        LV_COLUMN friendPcol = { 0 };
        LPCWSTR friendTitle[] = { L"wxid",L"账号",L"昵称",L"备注",L"头像" };
        int friendCx[] = { 80,80,80,80,80 };
        friendPcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
        friendPcol.fmt = LVCFMT_LEFT;
        for (unsigned int i = 0; i < size(friendTitle); i++) {
            friendPcol.pszText = (LPWSTR)friendTitle[i];
            friendPcol.cx = friendCx[i];
            ListView_InsertColumn(GetDlgItem(hDlg, FRIEND_LIST), i, &friendPcol);
        }

        HANDLE lThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)getLoginStatus, NULL, NULL, 0);
        if (lThread != 0) {
            CloseHandle(lThread);
        }

        HANDLE hookThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)inLineHook, NULL, NULL, 0);
        if (hookThread != 0) {
            CloseHandle(hookThread);
        }
        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
    {
        wchar_t RecverWxid[0x500] = { 0 };
        GetDlgItemText(hDlg, RECEIVE_WXID_TEXT, RecverWxid, 500);
        wchar_t RecverContent[0x2000] = { 0 };
        GetDlgItemText(hDlg, RECEIVE_CONTENT_TEXT, RecverContent, 2000);
        switch (wParam)
        {
        case GOTO_QRCODE:
        {
            GotoQrCode();
            break;
        }
        case GET_MY_INFO://获取个人信息
        {
            Information* myInfo = getMyInfo();
            wchar_t str[0x1000] = { 0 };
            swprintf_s(str,
                L"微信ID：%s\r\n账号：%s\r\n昵称：%s\r\n设备：%s\r\n手机号：%s\r\n邮箱：%s\r\n性别：%d\r\n国籍：%s\r\n省份：%s\r\n城市：%s\r\n签名：%s\r\n头像：%s",
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
                myInfo->bigHeader);
            SetDlgItemText(hDlg, MY_INFO_TEXT, str);
            break;
        }
        case GET_CONTACT_LIST://获取联系人
            getContactList();
            break;
        case RUN_SQL_BTN://执行SQL
        {
            SetDlgItemTextA(getGlobalHwnd(), SQL_RESULT, "数据库查询中...");
            char* sqlErrmsg = NULL;
            char dbName[0x100] = { 0 };
            GetDlgItemTextA(hDlg, DATABASE_SELECT, dbName, 100);
            char sql[0x1000] = { 0 };
            GetDlgItemTextA(hDlg, SQL_TEXT, sql, 1000);
            runSql(dbName, sql, runSqlCallBack, sqlErrmsg);
            break;
        }
        case SEND_TEXT_BTN://发送文本
        {
            SendTextMsg(RecverWxid, RecverContent);
            break;
        }
        case SEND_IMG_BTN://发送图片
        {
            SendImageMsg(RecverWxid, RecverContent);
            break;
        }
        case SEND_ATTACH_BTN://发送附件
        {
            SendAttachMsg(RecverWxid, RecverContent);
            break;
        }
        case SEND_USERCARD_BTN://发送好友名片
        {
            //<?xml version="1.0"?><msg bigheadimgurl="http://wx.qlogo.cn/mmhead/ver_1/LiccciblITlIKGcj01r8wFVxqC0IdPPoicpHqFtCjAc2EJ2wyaX4kvhuKCuicGmX6ZaEZzUHiaiahoVquNOO2r1eHAdoJsMam0iatVJOV9X84J7Z3c/0" smallheadimgurl="http://wx.qlogo.cn/mmhead/ver_1/LiccciblITlIKGcj01r8wFVxqC0IdPPoicpHqFtCjAc2EJ2wyaX4kvhuKCuicGmX6ZaEZzUHiaiahoVquNOO2r1eHAdoJsMam0iatVJOV9X84J7Z3c/132" username="这里填写微信id" nickname="111111" fullpy="" shortpy="" alias="" imagestatus="0" scene="17" province="" city="" sign="" sex="1" certflag="0" certinfo="" brandIconUrl="" brandHomeUrl="" brandSubscriptConfigUrl= "" brandFlags="0" regionCode="" />
            SendXmlCard(RecverWxid, RecverContent);
            break;
        }
        case SEND_URL_BTN://发送链接
        {
            wchar_t fromWxid[0x100];
           // swprintf_s(fromWxid, L"%s", L"微信ID");
            SendXmlArticle(RecverWxid, fromWxid, RecverContent);
            break;
        }
        case ADD_NEW_FRIEND://添加好友
            AddUserFromWxid(RecverWxid, RecverContent);
            break;
        case CLEAR_FANS://无痕清粉
            MessageBoxA(NULL,"功能尚未完善","提示",0);
            break;
        case LOGOUT_ACCOUNT://退出登录
            LogoutWeChat();
            break;
        case DELETE_FRIEND://删除好友
            DeleteFriend(RecverWxid);
            break;
        case SEND_CHATROOM_AT_MSG://发送群@消息
        {
            wchar_t atWxid[0x100] = L"微信ID";
            wchar_t nickname[0x100] = L"会编代码的狗";
            SendChatroomAtMsg(RecverWxid, atWxid, nickname, RecverContent);
            break;
        }
        case ADD_CHATROMM_USER://添加群成员
            AddChatroomMember(RecverWxid, RecverContent);
            break;
        case SET_CHATROOM_NAME://设置群聊名称
            SetChatoomName(RecverWxid, RecverContent);
            break;
        case SET_CHATROOM_ANNOUNCEMENT://设置群公告
            SetChatroomAnnouncement(RecverWxid, RecverContent);
            break;
        case DELETE_CHATROOM_USER://删除群成员
            DeleteChatroomMember(RecverWxid, RecverContent);
            break;
        case GET_CHATROOM_USER:
        {
            GetChatroomUser(RecverWxid);
            break;
        }
        case QUIT_CHATROOM://退出群聊
            QuitChatRoom(RecverWxid);
            break;
        case AUTO_COLLECT_MONEY://自动收款
        {
            char str[0x100] = { 0 };
            GetDlgItemTextA(hDlg, AUTO_COLLECT_MONEY, str, 100);
            SetDlgItemTextA(hDlg, AUTO_COLLECT_MONEY, StrStrA(str, "开启") ? "自动收款" : "已开启自动收款");
            break;
        }
        case AUTO_AGREE_NEW_FRIEND_APPLY://自动同意好友申请
        {
            char str[0x100] = { 0 };
            GetDlgItemTextA(hDlg, AUTO_AGREE_NEW_FRIEND_APPLY, str, 100);
            SetDlgItemTextA(hDlg, AUTO_AGREE_NEW_FRIEND_APPLY, StrStrA(str, "开启") ? "自动通过好友请求" : "已开启自动通过好友请求");
            break;
        }
        case AUTO_DOWNLOAD_IMAGE://自动保存聊天图片
        {
            char str[0x100] = { 0 };
            GetDlgItemTextA(hDlg, AUTO_DOWNLOAD_IMAGE, str, 100);
            SetDlgItemTextA(hDlg, AUTO_DOWNLOAD_IMAGE, StrStrA(str, "开启") ? "保存聊天图片" : "已开启保存聊天图片");
            break;
        }
        case AUTO_FOLLOW_GH://自动关注公众号名片
        {
            char str[0x100] = { 0 };
            GetDlgItemTextA(hDlg, AUTO_FOLLOW_GH, str, 100);
            SetDlgItemTextA(hDlg, AUTO_FOLLOW_GH, StrStrA(str, "开启") ? "关注公众号名片" : "已开启关注公众号名片");
            break;
        }
        case AUTO_JOIN_GROUP://自动同意要求入群
        {
            char str[0x100] = { 0 };
            GetDlgItemTextA(hDlg, AUTO_JOIN_GROUP, str, 100);
            SetDlgItemTextA(hDlg, AUTO_JOIN_GROUP, StrStrA(str, "开启") ? "同意进群邀请" : "已开启自动进群邀请");
            break;
        }
        case AUTO_ADD_USER_CARD://自动同意要求入群
        {
            char str[0x100] = { 0 };
            GetDlgItemTextA(hDlg, AUTO_ADD_USER_CARD, str, 100);
            SetDlgItemTextA(hDlg, AUTO_ADD_USER_CARD, StrStrA(str, "开启") ? "添加用户名片" : "已开启自动添加名片好友");
            break;
        }
        default:
            break; 
        }
        break;
    }
    case WM_CLOSE:
        break;
    }
    return (INT_PTR)FALSE;
}
#pragma once

void CollectMoney(wchar_t* wxid, wchar_t* transferid);
void AutoCollectMoney(wchar_t* wxid, wstring msg);

void AddUserFromWxid(wchar_t* wxid, wchar_t* msg);

void AddUserFromCard(wchar_t* v1, wchar_t* msg);
void AutoAddUserFromCard(wstring msg);

void AgreeUserRequest(wchar_t* v1, wchar_t* v2);
void AutoAgreeUserRequest(wstring msg);

void DeleteFriend(wchar_t* wxid);
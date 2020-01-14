#pragma once
void SendTextMsg(wchar_t* wxid, wchar_t* msg);
void SendImageMsg(wchar_t* wxid, wchar_t* filepath);
void SendAttachMsg(wchar_t* wxid, wchar_t* filepath);
void SendXmlCard(wchar_t* RecverWxid, wchar_t* xml);
void SendXmlArticle(wchar_t* RecverWxid, wchar_t* FromWxid, wchar_t* xmlData);
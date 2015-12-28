/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2015
-------------------------------------------------------------------------
-------------------------------------------------------------------------
History:

- 13.03.2015   15:28 : Created by AfroStalin(chernecoff)
- 23.03.2015   15:50 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#ifndef _CryMasterServer_H_
#define _CryMasterServer_H_

typedef UINT_PTR SOCKET;

class CCryMasterServer
{
public:
	CCryMasterServer(){}
	~CCryMasterServer(){}

	const char* Connect();
	const char* Login(const char* login, const char* password);
	const char* Register(const char* login, const char* password, const char* nickname);

	void SendGameServerInfo();

	void SendGlobalChatMessage(const char* message);
	void SendRequest(const char* request, const char* sParam, int iParam);

private:
	void ClientThread(SOCKET ServerSocket);
	int InitWinSock();
};

#endif
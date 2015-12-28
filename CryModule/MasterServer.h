/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
-------------------------------------------------------------------------
History:

- 13.03.2015   15:28 : Created by AfroStalin(chernecoff)
- 23.05.2015   00:50  : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#ifndef _MasterServer_H_
#define _MasterServer_H_

typedef UINT_PTR SOCKET;

class CMasterServer
{
public:
	CMasterServer(){}
	~CMasterServer(){}

	const char* Connect();
	void Disconnect();

	const char* Login(const char* login, const char* password);
	const char* Register(const char* login, const char* password, const char* nickname);

	void SendGameServerInfo();
	void SendGlobalChatMessage(const char* message);
	void SendRequest(const char* request, const char* sParam, int iParam);

	int InitWinSock();

public:
	std::vector <SGameServer> vServers;

	
private:
	void ClientThread(SOCKET ServerSocket);
};

#endif
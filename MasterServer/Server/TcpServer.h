/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
-------------------------------------------------------------------------
History:

- 15.08.2014   21:10 : Created by AfroStalin(chernecoff)
- 30.04.2015   22:56 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/
#ifndef _TCP_SERVER_
#define _TCP_SERVER_

#include <winsock.h>
#include "Packets\RSP.h"

class CTcpServer
{
public:
	CTcpServer();
	~CTcpServer();

	void Start();
	void Free();

	void SendClientStatus(std::string name, EClientStatus status);

private:
	void ServerThread();
	void Update();
	void SetServerStatus();	
	void SendServerInfo();
	void SendGlobalMessage(SMessage message);
	void RemoveGameServer(int id);
	
	int InitWinSock();

	void ClientThread(SClient client);
	void GameServerThread(SGameServer server);
public:
	std::vector <SClient> vClients;
	std::vector <SGameServer> vServers;
	std::mutex mutex;

protected:
	SOCKADDR_IN addr;
	SOCKET sConnect; 
	SOCKET sListen;

	int addrlen;
	int unique_id;
};

#endif
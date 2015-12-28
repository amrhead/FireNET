/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
--------------------------------------------------------------------------
History:

- 15.08.2014   21:10 : Created by AfroStalin(chernecoff)
- 01.06.2015   12:24 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/
#include "StdAfx.h"
#include <time.h>


#pragma warning(disable : 4018)
#pragma warning(disable : 4267)

CTcpServer::CTcpServer()
{
	addrlen = sizeof(addr);
	unique_id = 0;
}

CTcpServer::~CTcpServer()
{
	Free();
}


void CTcpServer::Update()
{
	while (true)
	{
		SetServerStatus();
		Sleep(500);
	}
}


void CTcpServer::Start()
{
	std::thread updateThread(&CTcpServer::Update, this);
	std::thread serverThread(&CTcpServer::ServerThread, this);

	updateThread.detach();
	serverThread.detach();	

	/*
	SGameServer s1, s2, s3, s4, s5, s6, s7, s8, s9, s10;
	s1.id = 0; s1.ip = "81.177.141.21", s1.mapName = "test", s1.port = 64090, s1.serverName = "test0", s1.maxPlayers = 16, s1.currentPlayers = 0, s1.gameRules = "unknown";
	s2.id = 1; s2.ip = "213.61.34.22", s2.mapName = "test", s2.port = 64090, s2.serverName = "test1", s2.maxPlayers = 16, s2.currentPlayers = 0, s2.gameRules = "unknown";
	s3.id = 2; s3.ip = "87.240.131.99", s3.mapName = "test", s3.port = 64090, s3.serverName = "test2", s3.maxPlayers = 16, s3.currentPlayers = 0, s3.gameRules = "unknown";
	s4.id = 3; s4.ip = "213.61.34.22", s4.mapName = "test", s4.port = 64090, s4.serverName = "test3", s4.maxPlayers = 16, s4.currentPlayers = 0, s4.gameRules = "unknown";
	s5.id = 4; s5.ip = "81.177.141.21", s5.mapName = "test", s5.port = 64090, s5.serverName = "test4", s5.maxPlayers = 16, s5.currentPlayers = 0, s5.gameRules = "unknown";

	s5.id = 5; s6.ip = "213.61.34.22", s6.mapName = "test", s6.port = 64090, s6.serverName = "test5", s6.maxPlayers = 16, s6.currentPlayers = 0, s6.gameRules = "unknown";
	s5.id = 6; s7.ip = "81.177.141.21", s7.mapName = "test", s7.port = 64090, s7.serverName = "test6", s7.maxPlayers = 16, s7.currentPlayers = 0, s7.gameRules = "unknown";
	s5.id = 7; s8.ip = "127.0.0.1", s8.mapName = "test", s8.port = 64090, s8.serverName = "test7", s8.maxPlayers = 16, s8.currentPlayers = 0, s8.gameRules = "unknown";
	s5.id = 8; s9.ip = "81.177.141.21", s9.mapName = "test", s9.port = 64090, s9.serverName = "test8", s9.maxPlayers = 16, s9.currentPlayers = 0, s9.gameRules = "unknown";
	s5.id = 9; s10.ip = "127.0.0.1", s10.mapName = "test", s10.port = 64090, s10.serverName = "test9", s10.maxPlayers = 16, s10.currentPlayers = 0, s10.gameRules = "unknown";


	SERVER_LOCK
	gEnv->pServer->vServers.push_back(s1);
	gEnv->pServer->vServers.push_back(s2);
	gEnv->pServer->vServers.push_back(s3);
	gEnv->pServer->vServers.push_back(s4);
	gEnv->pServer->vServers.push_back(s5);

	gEnv->pServer->vServers.push_back(s6);
	gEnv->pServer->vServers.push_back(s7);
	gEnv->pServer->vServers.push_back(s8);
	gEnv->pServer->vServers.push_back(s9);
	gEnv->pServer->vServers.push_back(s10);
	SERVER_UNLOCK
	gEnv->allServers = 10;*/
}

void CTcpServer::Free()
{
	closesocket(sListen);
	WSACleanup();
}

void CTcpServer::ServerThread()
{
	Log(LOG_INFO,"Starting TcpServer...");

	// Init WinSock
	if(InitWinSock() != 0)
	{
		Log(LOG_ERROR,"Error startup WinSock!!!");
		return;
	}

	// Init packet queue
	gEnv->pPacketQueue->Init();

	sListen = socket (AF_INET, SOCK_STREAM, NULL);
	sConnect = socket(AF_INET, SOCK_STREAM, NULL);

	addr.sin_addr.s_addr = inet_addr (gEnv->pSettings->GetConfigValue("Server","ip")); 
	addr.sin_port        = htons (atoi(gEnv->pSettings->GetConfigValue("Server","port")));   
	addr.sin_family      = AF_INET;

	if(bind(sListen, (SOCKADDR*)&addr ,sizeof(addr)) >= 0)
	{
		listen (sListen,64);

		Log(LOG_INFO,"Server started!");
		Log(LOG_INFO,"Server ip '%s'",gEnv->pSettings->GetConfigValue("Server","ip"));
		Log(LOG_INFO,"Server port '%d'", atoi(gEnv->pSettings->GetConfigValue("Server","port")));
		Log(LOG_INFO,"Master server loaded for %d msec.",clock()-(gEnv->startTime));
		Log(LOG_INFO,"Use 'list' command to see all console commands");

		while (true)
		{
			if(sConnect = accept (sListen ,(SOCKADDR*)&addr, &addrlen))
			{
				Log(LOG_INFO,"New incoming connection from '%s' ...",inet_ntoa((in_addr)addr.sin_addr));

				// Sending identification package
				gEnv->pRsp->SendIdentificationPacket(sConnect);

				char *Buffer = new char [2048];
				int size = 0;

				while(size != SOCKET_ERROR)
				{
					if((size = recv (sConnect,Buffer,2048,NULL)) > 0) 
					{
						Log(LOG_DEBUG,"New incoming packet from <%s>...Reading..", inet_ntoa((in_addr)addr.sin_addr));

						SPacket Packet;
						Packet.data = Buffer;
						Packet.size = size;

						EPacketType PacketType = gEnv->pRsp->GetPacketType(Packet);

						switch (PacketType)
						{
						case PACKET_IDENTIFICATION:
							{
								if(gEnv->allPlayers != gEnv->maxPlayers)
								{
									Log(LOG_DEBUG,"Client packet from '%s' accepted! Create new thread...", inet_ntoa((in_addr)addr.sin_addr));

									SClient client;
									client.socket = sConnect;
									client.ip = inet_ntoa((in_addr)addr.sin_addr);

									std::thread clientThread(&CTcpServer::ClientThread, this, client);
									clientThread.detach();
								}

								break;
							}
						case PACKET_GAME_SERVER:
							{
								if(gEnv->allServers != gEnv->maxGameServers)
								{					
									SGameServer server;
									server.socket = sConnect;
									server.ip = inet_ntoa((in_addr)addr.sin_addr);

									bool blockDual = false;

									for(auto it = vServers.begin(); it != vServers.end(); ++it)
									{
										if(it->ip = server.ip)
										{
											// Block dual servers
											bool tmp = !!atoi(gEnv->pSettings->GetConfigValue("Server","block_dual_servers"));

											if(tmp)
											{
												closesocket(server.socket);
												blockDual = true;
												Log(LOG_WARNING,"Block dual connection from game server '%s'",inet_ntoa((in_addr)addr.sin_addr));
											}
										}
									}

									if(!blockDual)
									{
										std::thread gameServerThread(&CTcpServer::GameServerThread, this, server);
										gameServerThread.detach();

										Log(LOG_DEBUG,"Game server packet from '%s' accepted! Create new thread...", inet_ntoa((in_addr)addr.sin_addr));
									}
								}
								break;
							}
						default:
							Log(LOG_WARNING,"Unknown packet from <%s> client...ignoring...",inet_ntoa((in_addr)addr.sin_addr));
							closesocket(sConnect);
							break;
						}

						break;
					}		
				}

				delete[] Buffer;
			}
		}
	}
	else
		Log(LOG_ERROR, "Filed bind socket! Please, check you server.cfg and setup server ip and port.");
}

void CTcpServer::ClientThread(SClient client)
{
	std::vector <SClient>::iterator it;

	char *Buffer = new char [2048];
	int size = 0;
	client.nickname = "Unknown";

	gEnv->allPlayers++;

	mutex.lock(); // Lock
	vClients.push_back(client);
	mutex.unlock(); // Unlock

	Log(LOG_INFO,"Client <%s:%s> connected!",client.nickname.c_str(),client.ip);

	while(size != SOCKET_ERROR)
	{
		if((size = recv (client.socket,Buffer,2048,NULL)) > 0 && size != SOCKET_ERROR)
		{
			gEnv->inPackets++;

			SPacket packet;
			packet.data = Buffer;
			packet.size = size;

			mutex.lock(); // Lock

			for(it = vClients.begin(); it != vClients.end(); ++it)
			{
				if(it->socket == client.socket)  // Get client iterator
				{
					client.socket = it->socket;
					client.ip = it->ip;
					client.login  = it->login;
					client.nickname = it->nickname;
					client.level = it->level;
					client.money = it->money;
					client.xp = it->xp;
					client.banStatus = it->banStatus;
					break;
				}
			}
			

			mutex.unlock(); // Unlock

			SReadPacket Packet;
			Packet.client = client;
			Packet.packet = packet;

			gEnv->pPacketQueue->InsertPacketToRead(Packet);
		}
		else
			break;

		Sleep(1);
	}

	mutex.lock(); // Lock
	for(it = vClients.begin(); it != vClients.end(); ++it)
	{
		if(it->socket == client.socket)
			break;
	}
	vClients.erase(it);
	mutex.unlock(); // Unlock

	Log(LOG_INFO,"Client <%s:%s> dissconected!",client.nickname.c_str(), client.ip);

	if(strcmp(client.nickname.c_str(), "Unknown"))
		SendClientStatus(client.nickname,CLIENT_DISCONNECTED);

	gEnv->allPlayers--;

	delete[] Buffer;
	closesocket(client.socket);	
}

void CTcpServer::GameServerThread(SGameServer server)
{
	std::vector <SGameServer>::iterator it;

	char *Buffer = new char [2048];
	int size = 0;
	server.serverName = "Unknown";
	server.id = unique_id;

	gEnv->allServers++;
	unique_id++;

	mutex.lock(); // Lock
	vServers.push_back(server);
	mutex.unlock(); // Unlock

	Log(LOG_INFO,"Game server <%s:%s> connected!",server.serverName,server.ip);

	while(size != SOCKET_ERROR)
	{
		if((size = recv (server.socket,Buffer,2048,NULL)) > 0 && size != SOCKET_ERROR)
		{
			gEnv->inPackets++;

			SPacket packet;
			packet.data = Buffer;
			packet.size = size;

			mutex.lock(); // Lock

			for(it = vServers.begin(); it != vServers.end(); ++it)
			{
				if(it->socket == server.socket)  // Get iterator
				{
					server.socket = it->socket;
					server.serverName = it->serverName;
					server.mapName = it->mapName;
					server.gameRules = it->gameRules;
					server.ip = it->ip;
					server.currentPlayers = it->currentPlayers;
					server.maxPlayers = it->maxPlayers;
					server.port = it->port;
					break;
				}
			}

			mutex.unlock(); // Unlock

			SReadPacket Packet;
			Packet.server = server;
			Packet.packet = packet;

			gEnv->pPacketQueue->InsertPacketToRead(Packet);
		}
		else
			break;

		Sleep(1);
	}

	mutex.lock(); // Lock
	for(it = vServers.begin(); it != vServers.end(); ++it)
	{
		if(it->socket == server.socket)
		{
			server.serverName = it->serverName;
			server.ip = it->ip;
			break;
		}
	}
	vServers.erase(it);
	mutex.unlock(); // Unlock

	Log(LOG_INFO,"Game server <%s:%s> dissconected!",server.serverName, server.ip);

	if(strcmp(server.serverName,"Unknown"))
		RemoveGameServer(server.id);

	gEnv->allServers--;

	delete[] Buffer;
	closesocket(server.socket);	
}

int CTcpServer::InitWinSock ()
{
	WSAData wsaData ;
	WORD DllVersion = MAKEWORD(2,1); 
	int Val = WSAStartup(DllVersion,&wsaData);

	return Val;
}

void CTcpServer::SendServerInfo()
{
	mutex.lock();

	SMasterServerInfo info;
	info.playersOnline = vClients.size();
	info.gameServersOnline = vServers.size();

	if(vClients.size()>0)
	{
		for (auto conIT=vClients.begin(); conIT!=vClients.end(); ++conIT)
			gEnv->pRsp->SendMasterServerInfo(conIT->socket, info);
	}

	mutex.unlock();
}

void CTcpServer::RemoveGameServer(int id)
{
	mutex.lock();

	if(vClients.size()>0)
	{
		SRequestPacket request;
		request.request = "RemoveGameServer";
		request.iParam  = id;
		request.sParam = "";

		for (auto conIT=vClients.begin(); conIT!=vClients.end(); ++conIT)
			gEnv->pRsp->SendRequest(conIT->socket, request);
	}

	mutex.unlock();
} 

void CTcpServer::SendGlobalMessage(SMessage message)
{
	mutex.lock();

	if(vClients.size()>0)
	{
		for (auto conIT=vClients.begin(); conIT!=vClients.end(); ++conIT)
			gEnv->pRsp->SendMsg(conIT->socket,message);
	}

	mutex.unlock();
} 

void CTcpServer::SendClientStatus(std::string name, EClientStatus status)
{
	SMessage message;
	switch (status)
	{
	case CLIENT_CONNECTED:
		{
			char connectedMsg[128];
			sprintf(connectedMsg, "Player '%s' connected", name.c_str());

			message.area = CHAT_MESSAGE_GLOBAL;
			message.message = connectedMsg;
			SendGlobalMessage(message);

			SendServerInfo();
			break;
		}
	case CLIENT_DISCONNECTED:
		{
			char dissconnectedMsg[128];
			sprintf(dissconnectedMsg, "Player '%s' dissconnected", name.c_str());

			message.area = CHAT_MESSAGE_GLOBAL;
			message.message = dissconnectedMsg;
			SendGlobalMessage(message);

			SendServerInfo();
			break;
		}
	default:
		break;
	}
}

void CTcpServer::SetServerStatus()
{
	char text[256];
	sprintf(text,"Input packets : %d , Output packets : %d, Players online : %d, Game servers online :%d", gEnv->inPackets, gEnv->outPackets, gEnv->allPlayers, gEnv->allServers);
	SendMessage (gEnv->statusBox, WM_SETTEXT, FALSE, reinterpret_cast<LPARAM>(text)); 
}
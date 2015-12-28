/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
-------------------------------------------------------------------------
History:

- 13.03.2015   15:28 : Created by AfroStalin(chernecoff)
- 01.06.2015   11:03 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#include "StdAfx.h"
#include <thread>
#include <winsock.h>

#include "System/Global.h"
#include "MasterServer.h"
#include "Nodes/MsEvents.h"
#include "Tools/md5.h"

SOCKADDR_IN addr;
SOCKET sConnect; 

int CMasterServer::InitWinSock ()
{
	WSAData wsaData ;
	WORD DllVersion = MAKEWORD(2,1); 
	int Val = WSAStartup(DllVersion,&wsaData);

	return Val;
}

void CMasterServer::ClientThread(SOCKET ServerSocket)
{
	char *Buffer = new char [2048];
	int size = 0;

	gClientEnv->bConnected = true;

#if	defined(DEDICATED_SERVER)
	SendGameServerInfo();
#else
	gClientEnv->pRsp->SendIdentificationPacket(sConnect);
#endif

	while(size != SOCKET_ERROR)
	{
		if((size = recv (ServerSocket,Buffer,2048,NULL)) > 0)
		{
			SPacket packet;
			packet.data = Buffer;
			packet.size = size;

			gClientEnv->pPacketQueue->InsertPacketToRead(packet);
		}
	}

	gClientEnv->bConnected = false;
	gEnv->pLog->LogWarning(TITLE "Master server disconnected!");
	SUIArguments args;
	args.AddArgument("@ms_connection_lost");
	CMsEvents::GetInstance()->SendEvent(CMsEvents::eUIGE_Error, args);

	delete[] Buffer;
	WSACleanup();
}

const char* CMasterServer::Connect()
{
	if(InitWinSock() != 0)
	{
		gEnv->pLog->LogWarning(TITLE "Error startup WinSock!!!");
		return "@winsock_error";
	}

	sConnect = socket(AF_INET, SOCK_STREAM, NULL);

	addr.sin_addr.s_addr = inet_addr (gEnv->pConsole->GetCVar("fn_master_server_ip")->GetString());
	addr.sin_port        = htons (gEnv->pConsole->GetCVar("fn_master_server_port")->GetIVal()); 
	addr.sin_family      = AF_INET;

	if(connect (sConnect,(SOCKADDR*)&addr ,sizeof(addr)) !=0)
	{
		gEnv->pLog->LogWarning(TITLE "Server not available!!!");
		WSACleanup();
		return "@ms_not_available";
	}
	else
	{
		int size = 0 ;
		char *Buffer = new char [256];

		if((size = recv (sConnect,Buffer,256,NULL)) > 0)
		{
			SPacket Packet;
			Packet.data = Buffer;
			Packet.size = size;

			EPacketType PacketType = gClientEnv->pRsp->GetPacketType(Packet);

			switch (PacketType)
			{
			case PACKET_IDENTIFICATION:
				{
					std::thread clientThread(&CMasterServer::ClientThread, this, sConnect);
					clientThread.detach();

					gEnv->pLog->LogWarning(TITLE "Connection is established");
					return "@ms_connection_established";
					break;
				}
			default:
				{
					gEnv->pLog->LogWarning(TITLE "Connection refused!!!");
					WSACleanup();
					return "@ms_connection_refused";
					break;
				}
			}
		}

		delete[] Buffer;
	}

	return "@ms_unknown_error";
}

void CMasterServer::Disconnect()
{
	closesocket(sConnect);
	WSACleanup();
}

const char* CMasterServer::Login(const char* login, const char* password)
{
	gEnv->pLog->Log(TITLE "CMasterServer::Login()");

	if(gClientEnv->bConnected)
	{
		MD5 md5;

		SLoginPacket LoginPacket;
		LoginPacket.login = login;
		LoginPacket.password = md5.digestString((char*)password);

		gClientEnv->serverResult = ""; // Clearing server result!

		gClientEnv->pRsp->SendLoginPacket(sConnect,LoginPacket); // Send login packet


		float startTime = gEnv->pTimer->GetAsyncTime().GetSeconds();
		float timeout = startTime + 5; // 5 second

		// Wait answer from server
		while(gEnv->pTimer->GetAsyncTime().GetSeconds() != timeout)
		{
			if(strcmp(gClientEnv->serverResult.c_str(),""))
				return gClientEnv->serverResult.c_str();
		}

		return "login_timeout";
	}

	gEnv->pLog->LogError(TITLE "Master server not connected!");
	return "ms_connection_lost";
}

const char* CMasterServer::Register(const char* login, const char* password, const char* nickname)
{
	gEnv->pLog->Log(TITLE "CMasterServer::Register()");

	if(gClientEnv->bConnected)
	{
		MD5 md5;

		SLoginPacket RegisterPacket;
		RegisterPacket.nickname = nickname;
		RegisterPacket.login = login;
		RegisterPacket.password = md5.digestString((char*)password);

		gClientEnv->serverResult = ""; // Clearing server result!

		gClientEnv->pRsp->SendRegisterPacket(sConnect, RegisterPacket); // Send register packet

		float startTime = gEnv->pTimer->GetAsyncTime().GetSeconds();
		float timeout = startTime + 3; // 3 second

		// Wait answer from server
		while(gEnv->pTimer->GetAsyncTime().GetSeconds() != timeout)
		{
			if(strcmp(gClientEnv->serverResult.c_str(),""))
				return gClientEnv->serverResult.c_str();
		}

		return "register_timeout";
	}

	gEnv->pLog->LogError(TITLE "Master server not connected!");
	return "ms_connection_lost";
}

void CMasterServer::SendGlobalChatMessage(const char* message)
{
	gEnv->pLog->Log(TITLE "CMasterServer::SendGlobalChatMessage()");

	if(gClientEnv->bConnected)
	{
		SMessage Message;
		Message.area = CHAT_MESSAGE_GLOBAL;
		Message.message = message;

		gClientEnv->pRsp->SendMsg(sConnect,Message);
		return;
	}

	gEnv->pLog->LogError(TITLE "Master server not connected!");
}

void CMasterServer::SendRequest(const char* request, const char* sParam, int iParam)
{
	gEnv->pLog->Log(TITLE "CMasterServer::SendRequest()");

	if(gClientEnv->bConnected)
	{
		SRequestPacket Request;
		Request.request = request;
		Request.sParam = sParam;
		Request.iParam = iParam;


		gClientEnv->pRsp->SendRequest(sConnect,Request);
		return;
	}

	gEnv->pLog->LogError(TITLE "Master server not connected!");
}


void CMasterServer::SendGameServerInfo()
{
	CryLogAlways(TITLE "CMasterServer:: Send game server info...");

	if(gClientEnv->bConnected)
	{
		// Get server ip adress 
		ICVar* ip_ = gEnv->pConsole->GetCVar("sv_bind");                  
		gClientEnv->gameServer.ip = ip_->GetString();

		// Get server port
		gClientEnv->gameServer.port = gEnv->pConsole->GetCVar("sv_port")->GetIVal();     

		// Get server name 
		ICVar* pName = gEnv->pConsole->GetCVar("sv_servername");           
		if(pName) gClientEnv->gameServer.serverName = pName->GetString();

		// Get max players
		gClientEnv->gameServer.maxPlayers = gEnv->pConsole->GetCVar("sv_maxplayers")->GetIVal();  

		// Get map name 
		ICVar* map = gEnv->pConsole->GetCVar("sv_map");                    
		gClientEnv->gameServer.mapName = (char*)map->GetString();      

		// Get gamerules
		gClientEnv->gameServer.gameRules = (char*)gEnv->pConsole->GetCVar("sv_gamerules")->GetString(); 

		gClientEnv->pRsp->SendGameServerInfo(sConnect, gClientEnv->gameServer);

		return;
	}

	gEnv->pLog->LogError(TITLE "Master server not connected!");
}
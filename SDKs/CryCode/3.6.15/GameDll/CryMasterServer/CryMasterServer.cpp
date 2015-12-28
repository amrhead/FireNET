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

#include "StdAfx.h"
#include <thread>
#include <winsock.h>

#include "System/Global.h"
#include "CryMasterServer.h"
#include "Nodes/MsEvents.h"
#include "Tools/md5.h"

SOCKADDR_IN addr;
SOCKET sConnect; 

int CCryMasterServer::InitWinSock ()
{
	WSAData wsaData ;
	WORD DllVersion = MAKEWORD(2,1); 
	int Val = WSAStartup(DllVersion,&wsaData);

	return Val;
}

void CCryMasterServer::ClientThread(SOCKET ServerSocket)
{
	char *Buffer = new char [256];
	int size = 0;

	

#if	defined(DEDICATED_SERVER)
	SendGameServerInfo();
#else
	gClientEnv->pRsp->SendIdentificationPacket(sConnect);
#endif

	while(size != SOCKET_ERROR)
	{
		if((size = recv (ServerSocket,Buffer,256,NULL)) > 0)
		{
			SPacket packet;
			packet.data = Buffer;
			packet.size = 256;

			gClientEnv->pPacketQueue->InsertPacketToRead(packet);
		}
	}

	CryWarning(VALIDATOR_MODULE_NETWORK,VALIDATOR_WARNING,"[CryMasterServer] Master server disconnected!");
	SUIArguments args;
	args.AddArgument("@ms_connection_lost");
	CMsEvents::GetInstance()->SendEvent(CMsEvents::eUIGE_Error, args);
	WSACleanup();
}

const char* CCryMasterServer::Connect()
{
	IConsole *pConsole = gEnv->pConsole;

	if(InitWinSock() != 0)
	{
		CryWarning(VALIDATOR_MODULE_NETWORK,VALIDATOR_ERROR,"[CryMasterServer] Error startup WinSock!!!");
		return "@winsock_error";
	}

	sConnect = socket(AF_INET, SOCK_STREAM, NULL);

	addr.sin_addr.s_addr = inet_addr (pConsole->GetCVar("ms_ip")->GetString());
	addr.sin_port        = htons (pConsole->GetCVar("ms_port")->GetIVal()); 
	addr.sin_family      = AF_INET;

	if(connect (sConnect,(SOCKADDR*)&addr ,sizeof(addr)) !=0)
	{
		CryWarning(VALIDATOR_MODULE_NETWORK,VALIDATOR_ERROR,"[CryMasterServer] Server not available!!!");
		WSACleanup();
		return "@ms_not_available";
	}
	else
	{
		int size = 0 ;
		char *Buffer = new char [512];

		if((size = recv (sConnect,Buffer,256,NULL)) > 0)
		{
			SPacket Packet;
			Packet.data = Buffer;
			Packet.size = 256;

			EPacketType PacketType = gClientEnv->pRsp->GetPacketType(Packet);

			switch (PacketType)
			{
			case PACKET_IDENTIFICATION:
				{
					std::thread clientThread(&CCryMasterServer::ClientThread, this, sConnect);
					clientThread.detach();

					CryWarning(VALIDATOR_MODULE_NETWORK,VALIDATOR_COMMENT,"[CryMasterServer] Connection is established");
					return "@ms_connection_established";
					break;
				}
			default:
				{
					CryWarning(VALIDATOR_MODULE_NETWORK,VALIDATOR_ERROR,"[CryMasterServer] Connection refused!!!");
					WSACleanup();
					return "@ms_connection_refused";
					break;
				}
			}
		}
	}

	return "@ms_unknown_error";
}


const char* CCryMasterServer::Login(const char* login, const char* password)
{
	CryLog("[CryMasterServer] CCryMasterServer::Login()");
	MD5 md5;

	SLoginPacket LoginPacket;
	LoginPacket.login = login;
	LoginPacket.password = md5.digestString((char*)password);

	gClientEnv->serverResult = ""; // Clearing server result!

	gClientEnv->pRsp->SendLoginPacket(sConnect,LoginPacket); // Send login packet


	float startTime = gEnv->pTimer->GetAsyncTime().GetSeconds();
	float timeout = startTime + 3; // 3 second

	// Wait answer from server
	while(gEnv->pTimer->GetAsyncTime().GetSeconds() != timeout)
	{
		if(strcmp(gClientEnv->serverResult.c_str(),""))
			return gClientEnv->serverResult.c_str();
	}

	return "login_timeout";
}

const char* CCryMasterServer::Register(const char* login, const char* password, const char* nickname)
{
	CryLog("[CryMasterServer] CCryMasterServer::Register()");
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

void CCryMasterServer::SendGlobalChatMessage(const char* message)
{
	CryLog("[CryMasterServer] CCryMasterServer::SendGlobalChatMessage()");

	SMessage Message;
	Message.area = CHAT_MESSAGE_GLOBAL;
	Message.message = message;

	gClientEnv->pRsp->SendMsg(sConnect,Message);
}

void CCryMasterServer::SendRequest(const char* request, const char* sParam, int iParam)
{
	CryLog("[CryMasterServer] CCryMasterServer::SendRequest()");

	SRequestPacket Request;
	Request.request = request;
	Request.sParam = sParam;
	Request.iParam = iParam;


	gClientEnv->pRsp->SendRequest(sConnect,Request);
}

void CCryMasterServer::SendGameServerInfo()
{
	CryLogAlways("CryMasterServer] CCryMasterServer:: Send game server info...");

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
}
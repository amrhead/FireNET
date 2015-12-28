/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
-------------------------------------------------------------------------
History:

- 26.08.2014   13:49 : Created by AfroStalin(chernecoff)
- 01.06.2015   11:03 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/
#include "StdAfx.h"
#include "../System/Global.h"

#include "Packets.h"
#include "PacketDebugger.h"
#include "RSP.h"


EPacketType CReadSendPacket::GetPacketType (SPacket packet)
{
	gEnv->pLog->Log( TITLE "Read a packet type...");

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);
	p->decodeBlowfish(gClientEnv->bBlowFish);

	/****************************Обязательный блок************************************/
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	const char* version        = p->readString();             // Packet version
	/*********************************************************************************/

	if(version != NULL)
	{
		if(!strcmp(version , gClientEnv->clientVersion))
			return type;
		else
		{
			gEnv->pLog->LogWarning(TITLE "Different packet versions! Ignoring...");
			gEnv->pLog->Log(TITLE "Packet version = %s", version);
			gEnv->pLog->Log(TITLE "Curent version = %s", gClientEnv->clientVersion);
		}
	}

	delete p;
	return (EPacketType)-1;
}

void CReadSendPacket::ReadIdentificationPacket (SPacket packet)
{
	gEnv->pLog->Log(TITLE "Read identification packet...");

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);

	p->decodeBlowfish(gClientEnv->bBlowFish);

	/****************************Обязательный блок************************************/
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	const char* version        = p->readString();             // Packet version
	/*********************************************************************************/

	const char* endBlock = p->readString();                   // Завершающий блок

	if(strcmp(endBlock,EndBlock))
		gEnv->pLog->LogWarning(TITLE "Identification packet damaged!");

	if(gClientEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		gEnv->pLog->Log("Identification packet size = %d", size);
		
		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}
	delete p;
}

SMessage CReadSendPacket::ReadMsg(SPacket packet)
{
	SMessage message;

	gEnv->pLog->Log(TITLE "Read message packet...");

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);

	p->decodeBlowfish(gClientEnv->bBlowFish);

	/****************************Обязательный блок************************************/
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	const char* version        = p->readString();             // Packet version
	/*********************************************************************************/

	message.message    = p->readString();              
	message.area       = (EChatMessageArea)p->readInt();                      

	const char* endBlock = p->readString();

	if(strcmp(endBlock,EndBlock))
	{
		gEnv->pLog->LogWarning(TITLE "Message packet damaged!");

		message.message = "";
		return message;
	}

	if(gClientEnv->bDebugMode)
	{

		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		gEnv->pLog->Log(TITLE "Message packet size = %d",size);
		gEnv->pLog->Log(TITLE "Message packet data = %s", message.message);
		gEnv->pLog->Log(TITLE "Message packet area = %d", message.area);


		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	delete p;
	return message;
}

SRequestPacket CReadSendPacket::ReadRequest(SPacket packet)
{
	SRequestPacket request;

	gEnv->pLog->Log(TITLE "Read request packet...");

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);

	p->decodeBlowfish(gClientEnv->bBlowFish);

	/****************************Обязательный блок************************************/
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	const char* version        = p->readString();             // Packet version
	/*********************************************************************************/


	request.request = p->readString();
	request.sParam = p->readString();
	request.iParam = p->readInt();


	const char* endBlock = p->readString(); 
	if(strcmp(endBlock, EndBlock))
	{
		gEnv->pLog->LogWarning(TITLE "Request packet damaged!");

		request.request = "";
		return request;
	}

	if(gClientEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		gEnv->pLog->Log(TITLE "Request size = %d", size);
		gEnv->pLog->Log(TITLE "Request = %s", request.request);
		gEnv->pLog->Log(TITLE "Request sParam = %s", request.sParam);
		gEnv->pLog->Log(TITLE "Request iParam = %d", request.iParam);

		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	delete p;
	return request;
}

SGameServer CReadSendPacket::ReadGameServerInfo(SPacket packet)
{
	gEnv->pLog->Log(TITLE "Read game server info packet...");

	SGameServer Server;

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);

	p->decodeBlowfish(gClientEnv->bBlowFish);

	/****************************Обязательный блок************************************/
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	const char* version        = p->readString();             // Packet version
	/*********************************************************************************/

	Server.id              = p->readInt();                  // id сервера
	Server.ip              = p->readString();               // ip сервера          
	Server.port            = p->readInt();                  // Порт сервера
	Server.serverName      = p->readString();               // Название сервера
	Server.currentPlayers  = p->readInt();                  // Онлайн на сервере
	Server.maxPlayers      = p->readInt();                  // Максимальное кол-во игроков
	Server.mapName         = p->readString();               // Название карты
	Server.gameRules       = p->readString();               // Режим игры

	const char* endBlock = p->readString();                 // Завершающий блок

	if(strcmp(endBlock,EndBlock))
	{
		gEnv->pLog->LogWarning(TITLE "Game server info packet damaged!");

		Server.port  = 0;
		return Server;
	}

	if(gClientEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		gEnv->pLog->Log(TITLE "Game Server info packet size = %d",size);
		gEnv->pLog->Log(TITLE "Game Server ip = %s",Server.ip);
		gEnv->pLog->Log(TITLE "Game Server port = %d",Server.port);
		gEnv->pLog->Log(TITLE "Game Server name = %s",Server.serverName);
		gEnv->pLog->Log(TITLE "Game Server online = %d",Server.currentPlayers);
		gEnv->pLog->Log(TITLE "Game Server max players = %d",Server.maxPlayers);
		gEnv->pLog->Log(TITLE "Game Server map name = %s",Server.mapName);
		gEnv->pLog->Log(TITLE "Game Server gamerules = %s",Server.gameRules);

	
		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	delete p;
	return Server;
}

void CReadSendPacket::ReadGameServers(SPacket packet)
{
	gEnv->pLog->Log(TITLE "Read game server info packet...");

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);

	p->decodeBlowfish(gClientEnv->bBlowFish);

	/****************************Обязательный блок************************************/
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	const char* version        = p->readString();             // Packet version
	/*********************************************************************************/

	int serversCount = p->readInt();

	for (int i = 0 ; i != serversCount; i++)
	{
		SGameServer Server; SUIArguments args;

		Server.id              = p->readInt();                  // id сервера
		Server.ip              = p->readString();               // ip сервера          
		Server.port            = p->readInt();                  // Порт сервера
		Server.serverName      = p->readString();               // Название сервера
		Server.currentPlayers  = p->readInt();                  // Онлайн на сервере
		Server.maxPlayers      = p->readInt();                  // Максимальное кол-во игроков
		Server.mapName         = p->readString();               // Название карты
		Server.gameRules       = p->readString();               // Режим игры

		if(Server.port)
		{
			char tmp[64];
			sprintf(tmp, "%d/%d", Server.currentPlayers, Server.maxPlayers);

			args.AddArgument(Server.id);
			args.AddArgument(Server.ip);
			args.AddArgument(Server.port);
			args.AddArgument(Server.serverName);
			args.AddArgument(tmp);
			args.AddArgument((const char*)Server.mapName);
			args.AddArgument((const char*)Server.gameRules);
			args.AddArgument(gClientEnv->pPing->Ping(Server.ip));

			CMsEvents::GetInstance()->SendEvent(CMsEvents::eUIGE_GameServerInfo, args);

			gClientEnv->pMasterServer->vServers.push_back(Server);
		}
	}
	

	const char* endBlock = p->readString();                    // Завершающий блок

	if(strcmp(endBlock,EndBlock))
	{
		gEnv->pLog->LogWarning(TITLE "Game servers packet damaged!");
	}

	delete p;
}

SPlayer CReadSendPacket::ReadAccountInfo(SPacket packet)
{
	gEnv->pLog->Log(TITLE "Read account info packet...");

	SPlayer Player;
	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);
	p->decodeBlowfish(gClientEnv->bBlowFish);

	/****************************Обязательный блок************************************/
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	const char* version        = p->readString();             // Packet version
	/*********************************************************************************/

	Player.playerId  = p->readInt();
	Player.nickname  = p->readString();
	Player.xp        = p->readInt();
	Player.level     = p->readInt();
	Player.money     = p->readInt();
	Player.banStatus = !!p->readInt();

	const char* endBlock = p->readString();                   // Завершающий блок

	if(strcmp(endBlock,EndBlock))
	{
		gEnv->pLog->LogWarning(TITLE "Game server info packet damaged!");

		Player.playerId = 0;
		return Player;
	}

	if(gClientEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		gEnv->pLog->Log(TITLE "Account info packet size = %d",size);
		gEnv->pLog->Log(TITLE "Account playerId = %s",Player.playerId);
		gEnv->pLog->Log(TITLE "Account nickname = %s",Player.nickname.c_str());
		gEnv->pLog->Log(TITLE "Account level = %s",Player.level);
		gEnv->pLog->Log(TITLE "Account game cash = %d",Player.money);
		gEnv->pLog->Log(TITLE "Account xp = %d",Player.xp);
		gEnv->pLog->Log(TITLE "Account ban status = %s",Player.banStatus);

		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	delete p;
	return Player;
}

SMasterServerInfo CReadSendPacket::ReadMasterServerInfo(SPacket packet)
{
	gEnv->pLog->Log(TITLE "Read master server info packet...");

	SMasterServerInfo Info;
	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);
	p->decodeBlowfish(gClientEnv->bBlowFish);

	/****************************Обязательный блок************************************/
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	const char* version        = p->readString();             // Packet version
	/*********************************************************************************/

	Info.playersOnline = p->readInt();
	Info.gameServersOnline = p->readInt();

	const char* endBlock = p->readString();                   // Завершающий блок

	if(strcmp(endBlock,EndBlock))
	{
		gEnv->pLog->LogWarning(TITLE "Game server info packet damaged!");

		Info.playersOnline = 0;
		return Info;
	}

	if(gClientEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		gEnv->pLog->Log(TITLE "Master server info packet size = %d",size);
		gEnv->pLog->Log(TITLE "Master server players ammount = %d",Info.playersOnline);
		gEnv->pLog->Log(TITLE "Master server game server ammount = %d",Info.gameServersOnline);


		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	delete p;
	return Info;
}


void CReadSendPacket::ReadConsoleCommandPacket(SPacket packet)
{
	gEnv->pLog->Log(TITLE "Read console command packet...");

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);

	p->decodeBlowfish(gClientEnv->bBlowFish);

	/****************************Обязательный блок************************************/
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	const char* version        = p->readString();             // Packet version
	/*********************************************************************************/

	const char* command = p->readString();
	gEnv->pConsole->ExecuteString(command);

	const char* endBlock = p->readString();                   // Завершающий блок

	if(strcmp(endBlock,EndBlock))
		gEnv->pLog->LogWarning(TITLE "Identification packet damaged!");

	if(gClientEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		gEnv->pLog->Log("Identification packet size = %d", size);
		
		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	delete p;
}
/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014-2015
-------------------------------------------------------------------------

Description: Чтение пакетов
Description: Reading packets
-------------------------------------------------------------------------
History:

- 26.08.2014   13:49 : Created by AfroStalin(chernecoff)
- 30.04.2015   22:56 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/
#include "StdAfx.h"
#include "../System/Global.h"

#include "Packets.h"
#include "PacketDebugger.h"
#include "RSP.h"


EPacketType CReadSendPacket::GetPacketType (SPacket packet)
{
	//CryLog( "[CryMasterServer] Read a packet type...");

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);
	p->decodeBlowfish(gClientEnv->bBlowFish);

	/****************************Обязательный блок************************************/
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	std::string version        = p->readString();             // Packet version
	/*********************************************************************************/

	if(!strcmp(version.c_str() , gClientEnv->clientVersion))
		return type;
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK,VALIDATOR_WARNING,"[CryMasterServer] Different packet versions! Ignoring...");
		CryLog("[CryMasterServer] Packet version = %s", version);
		CryLog("[CryMasterServer] Curent version = %s", gClientEnv->clientVersion);
	}

	return (EPacketType)-1;
}

void CReadSendPacket::ReadIdentificationPacket (SPacket packet)
{
	//CryLog("[CryMasterServer] Read identification packet...");

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);

	p->decodeBlowfish(gClientEnv->bBlowFish);

	/****************************Обязательный блок************************************/
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	std::string version        = p->readString();             // Packet version
	/*********************************************************************************/

	std::string endBlock = p->readString();                   // Завершающий блок

	if(strcmp(endBlock.c_str(),EndBlock))
		CryWarning(VALIDATOR_MODULE_NETWORK,VALIDATOR_WARNING,"[CryMasterServer] Identification packet damaged!");

	if(gClientEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		CryLog("Identification packet size = %d", size);
		
		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

}

SMessage CReadSendPacket::ReadMsg(SPacket packet)
{
	SMessage message;

	//CryLog("[CryMasterServer] Read message packet...");

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);

	p->decodeBlowfish(gClientEnv->bBlowFish);

	/****************************Обязательный блок************************************/
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	std::string version        = p->readString();             // Packet version
	/*********************************************************************************/

	message.message    = p->readString();              
	message.area       = (EChatMessageArea)p->readInt();                      

	std::string endBlock = p->readString();

	if(strcmp(endBlock.c_str(),EndBlock))
	{
		CryWarning(VALIDATOR_MODULE_NETWORK,VALIDATOR_WARNING,"[CryMasterServer] Message packet damaged!");

		message.message = "";
		return message;
	}

	if(gClientEnv->bDebugMode)
	{

		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		CryLog("[CryMasterServer] Message packet size = %d",size);
		CryLog("[CryMasterServer] Message packet data = %s", message.message);
		CryLog("[CryMasterServer] Message packet area = %d", message.area);


		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	return message;
}

SRequestPacket CReadSendPacket::ReadRequest(SPacket packet)
{
	SRequestPacket request;

	//CryLog("[CryMasterServer] Read request packet...");

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);

	p->decodeBlowfish(gClientEnv->bBlowFish);

	/****************************Обязательный блок************************************/
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	std::string version        = p->readString();             // Packet version
	/*********************************************************************************/


	request.request = p->readString();
	request.sParam = p->readString();
	request.iParam = p->readInt();


	std::string endBlock = p->readString(); 
	if(strcmp(endBlock.c_str(), EndBlock))
	{
		CryWarning(VALIDATOR_MODULE_NETWORK,VALIDATOR_WARNING,"[CryMasterServer] Request packet damaged!");

		request.request = "";
		return request;
	}

	if(gClientEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		CryLog("[CryMasterServer] Request size = %d", size);
		CryLog("[CryMasterServer] Request = %s", request.request);
		CryLog("[CryMasterServer] Request sParam = %s", request.sParam);
		CryLog("[CryMasterServer] Request iParam = %d", request.iParam);

		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	return request;
}

SGameServer CReadSendPacket::ReadGameServerInfo(SPacket packet)
{
	//CryLog("[CryMasterServer] Read game server info packet...");

	SGameServer Server;

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);

	p->decodeBlowfish(gClientEnv->bBlowFish);

	/****************************Обязательный блок************************************/
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	std::string version        = p->readString();             // Packet version
	/*********************************************************************************/

	Server.ip              = p->readString();               // ip сервера          
	Server.port            = p->readInt();                  // Порт сервера
	Server.serverName      = p->readString();               // Название сервера
	Server.currentPlayers  = p->readInt();                  // Онлайн на сервере
	Server.maxPlayers      = p->readInt();                  // Максимальное кол-во игроков
	Server.mapName         = p->readString();               // Название карты
	Server.gameRules       = p->readString();               // Режим игры

	std::string endBlock = p->readString();                 // Завершающий блок

	if(strcmp(endBlock.c_str(),EndBlock))
	{
		CryWarning(VALIDATOR_MODULE_NETWORK,VALIDATOR_WARNING,"[CryMasterServer] Game server info packet damaged!");

		Server.port  = 0;
		return Server;
	}

	if(gClientEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		CryLog("[CryMasterServer] Game Server info packet size = %d",size);
		CryLog("[CryMasterServer] Game Server ip = %s",Server.ip);
		CryLog("[CryMasterServer] Game Server port = %d",Server.port);
		CryLog("[CryMasterServer] Game Server name = %s",Server.serverName);
		CryLog("[CryMasterServer] Game Server online = %d",Server.currentPlayers);
		CryLog("[CryMasterServer] Game Server max players = %d",Server.maxPlayers);
		CryLog("[CryMasterServer] Game Server map name = %s",Server.mapName);
		CryLog("[CryMasterServer] Game Server gamerules = %s",Server.gameRules);

	
		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	return Server;
}

SPlayer CReadSendPacket::ReadAccountInfo(SPacket packet)
{
	//CryLog("[CryMasterServer] Read account info packet...");

	SPlayer Player;
	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);
	p->decodeBlowfish(gClientEnv->bBlowFish);

	/****************************Обязательный блок************************************/
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	std::string version        = p->readString();             // Packet version
	/*********************************************************************************/

	Player.playerId  = p->readInt();
	Player.nickname  = p->readString();
	Player.xp        = p->readInt();
	Player.level     = p->readInt();
	Player.money     = p->readInt();
	Player.banStatus = !!p->readInt();

	std::string endBlock = p->readString();                   // Завершающий блок

	if(strcmp(endBlock.c_str(),EndBlock))
	{
		CryWarning(VALIDATOR_MODULE_NETWORK,VALIDATOR_WARNING,"[CryMasterServer] Game server info packet damaged!");

		Player.playerId = 0;
		return Player;
	}

	if(gClientEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		CryLog("[CryMasterServer] Account info packet size = %d",size);
		CryLog("[CryMasterServer] Account playerId = %s",Player.playerId);
		CryLog("[CryMasterServer] Account nickname = %d",Player.nickname);
		CryLog("[CryMasterServer] Account level = %s",Player.level);
		CryLog("[CryMasterServer] Account game cash = %d",Player.money);
		CryLog("[CryMasterServer] Account xp = %d",Player.xp);
		CryLog("[CryMasterServer] Account ban status = %s",Player.banStatus);

		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	return Player;
}


SMasterServerInfo CReadSendPacket::ReadMasterServerInfo(SPacket packet)
{
	//CryLog("[CryMasterServer] Read master server info packet...");

	SMasterServerInfo Info;
	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);
	p->decodeBlowfish(gClientEnv->bBlowFish);

	/****************************Обязательный блок************************************/
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	std::string version        = p->readString();             // Packet version
	/*********************************************************************************/

	Info.playersOnline = p->readInt();
	Info.gameServersOnline = p->readInt();

	std::string endBlock = p->readString();                   // Завершающий блок

	if(strcmp(endBlock.c_str(),EndBlock))
	{
		CryWarning(VALIDATOR_MODULE_NETWORK,VALIDATOR_WARNING,"[CryMasterServer] Game server info packet damaged!");

		Info.playersOnline = 0;
		return Info;
	}

	if(gClientEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		CryLog("[CryMasterServer] Master server info packet size = %d",size);
		CryLog("[CryMasterServer] Master server players ammount = %d",Info.playersOnline);
		CryLog("[CryMasterServer] Master server game server ammount = %d",Info.gameServersOnline);


		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	return Info;
}

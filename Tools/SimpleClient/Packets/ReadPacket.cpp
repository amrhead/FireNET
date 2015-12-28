/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014-2015
-------------------------------------------------------------------------

Description: Чтение пакетов
Description: Reading packets
-------------------------------------------------------------------------
History:

- 26.08.2014   13:49 : Created by AfroStalin(chernecoff)
- 15.03.2015   21:50 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#include "Global.h"

#include "Packets\Packets.h"
#include "Packets\PacketDebugger.h"
#include "Packets\RSP.h"


EPacketType CReadSendPacket::GetPacketType (SPacket packet)
{
	printf( "Read a packet type...\n");

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);
	p->decodeBlowfish(gClientEnv->bBlowFish);

	/****************************Обязательный блок************************************/
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	std::string version        = p->readString();             // Packet version
	/*********************************************************************************/

	if(!strcmp(version.c_str() , gClientEnv->clientVersion.c_str()))
		return type;
	else
	{
		printf("Different packet versions! Ignoring...\n");
		printf("Packet version = %s\n", version.c_str());
		printf("Curent version = %s\n", gClientEnv->clientVersion);
	}

	return (EPacketType)-1;
}

void CReadSendPacket::ReadIdentificationPacket (SPacket packet)
{
	printf("Read identification packet...\n");

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);

	p->decodeBlowfish(gClientEnv->bBlowFish);

	/****************************Обязательный блок************************************/
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	std::string version        = p->readString();             // Packet version
	/*********************************************************************************/

	std::string endBlock = p->readString();                   // Завершающий блок

	if(strcmp(endBlock.c_str(),EndBlock))
		printf("Identification packet damaged!\n");

	if(gClientEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		printf("Identification packet size = %d\n", size);
		
		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

}

#if defined _SERVER
SLoginPacket CReadSendPacket::ReadLoginPacket (SPacket packet)
{
	SLoginPacket loginPacket;

	printf("Read login packet...");

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);

	p->decodeBlowfish(gClientEnv->bBlowFish);

	/****************************Обязательный блок************************************/
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	std::string version        = p->readString();             // Packet version
	/*********************************************************************************/

	loginPacket.login    = p->readString();               
	loginPacket.password = p->readString();    


	std::string endBlock     = p->readString();       

	if(strcmp(endBlock.c_str(),EndBlock))
		Log(LOG_WARNING,"Login packet damaged!");

	if(gClientEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		printf("Login packet size = %d",size);
		printf("Login : %s",loginPacket.login);
		printf("Password : %s", loginPacket.password);
		
		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	return loginPacket;
}

SLoginPacket CReadSendPacket::ReadRegistrationPacket (SPacket packet)
{
	SLoginPacket registerPacket;

	printf("Read registration packet...");

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);

	p->decodeBlowfish(gClientEnv->bBlowFish);

	/****************************Обязательный блок************************************/
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	std::string version        = p->readString();             // Packet version
	/*********************************************************************************/

	registerPacket.login    = p->readString();             
	registerPacket.password = p->readString();                
	registerPacket.nickname = p->readString();                

	std::string endBlock     = p->readString();                // Завершающий блок

	if(strcmp(endBlock.c_str(), EndBlock))
		Log(LOG_WARNING,"Registration packet damaged!");

	if(gClientEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		printf("Registration packet size = %d",size);
		printf("Login : %s",registerPacket.login);
		printf("Password : %s",registerPacket.password);
		printf("Nickname : %s",registerPacket.nickname);

		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	return registerPacket;
}
#endif

SMessage CReadSendPacket::ReadMsg(SPacket packet)
{
	SMessage message;

	printf("Read MSG packet...\n");

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
		printf("MSG packet damaged!");

	if(gClientEnv->bDebugMode)
	{

		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		printf("MSG packet size = %d\n",size);
		printf("MSG packet data = %s\n", message.message);
		printf("MSG packet area = %d\n", message.area);


		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	return message;
}

SRequestPacket CReadSendPacket::ReadRequest(SPacket packet)
{
	SRequestPacket request;

	printf( "Read client request packet...\n");

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
		printf("Client request packet damaged!\n");

	if(gClientEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		printf("Request size = %d\n", size);
		printf("Request = %s\n", request.request);
		printf("Request sParam = %s\n", request.sParam);
		printf("Request iParam = %d\n", request.iParam);

		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	return request;
}

SGameServer CReadSendPacket::ReadGameServerInfo(SPacket packet)
{
	printf("Read game server info packet...\n");

	SGameServer Server;

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);

	p->decodeBlowfish(gClientEnv->bBlowFish);

/****************************Обязательный блок************************************/
	p->readString();                                        // Идентификатор пакета
	p->readInt();                                           // Тип сессии
	p->readString();                                        // Версия клиента или сервера

/*******************************Тело пакета****************************************/

	Server.ip              = p->readString();               // ip сервера          
	Server.port            = p->readInt();                  // Порт сервера
	Server.serverName      = p->readString();               // Название сервера
	Server.currentPlayers  = p->readInt();                  // Онлайн на сервере
	Server.maxPlayers      = p->readInt();                  // Максимальное кол-во игроков
	Server.mapName         = p->readString();               // Название карты
	Server.gameRules       = p->readString();               // Режим игры

	std::string endBlock = p->readString();                 // Завершающий блок

	if(strcmp(endBlock.c_str(),EndBlock))
		printf("Game server info packet damaged!\n");

	if(gClientEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		printf("Game Server info packet size = %d\n",size);
		printf("Game Server ip = %s\n",Server.ip);
		printf("Game Server port = %d\n",Server.port);
		printf("Game Server name = %s\n",Server.serverName);
		printf("Game Server online = %d\n",Server.currentPlayers);
		printf("Game Server max players = %d\n",Server.maxPlayers);
		printf("Game Server map name = %s\n",Server.mapName);
		printf("Game Server gamerules = %s\n",Server.gameRules);

	
		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	return Server;
}
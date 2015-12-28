/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
-------------------------------------------------------------------------
History:

- 26.08.2014   13:49 : Created by AfroStalin(chernecoff)
- 20.06.2015   15:18 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#include "StdAfx.h"
#include <winsock.h>

#include "Packets\Packets.h"
#include "Packets\PacketDebugger.h"
#include "Packets\RSP.h"


EPacketType CReadSendPacket::GetPacketType (SPacket packet)
{
	Log(LOG_DEBUG, "Read a packet type...");

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);
	p->decodeBlowfish(gEnv->bBlowFish);

	// Packet header
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	const char* version = p->readString();                    // Packet version
	//

	if(version != NULL)
	{
		if(!strcmp(version, gEnv->serverVersion))
			return type;
		else
		{
			Log(LOG_WARNING,"Different packet versions! Ignoring...");
			Log(LOG_DEBUG,"Packet version = %s", version);
			Log(LOG_DEBUG,"Curent version = %s", gEnv->serverVersion);
		}
	}

	delete p;
	return (EPacketType)-1;
}

void CReadSendPacket::ReadIdentificationPacket (SPacket packet)
{
	Log(LOG_DEBUG,"Read identification packet...");

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);

	p->decodeBlowfish(gEnv->bBlowFish);

	// Packet header
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	const char* version        = p->readString();             // Packet version
	//

	const char* endBlock = p->readString();                   // End block

	if(strcmp(endBlock,EndBlock))
		Log(LOG_WARNING,"Identification packet damaged!");

	if(gEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		Log(LOG_DEBUG,"Identification packet size = %d", size);
		
		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	delete p;
}

SLoginPacket CReadSendPacket::ReadLoginPacket (SPacket packet)
{
	SLoginPacket loginPacket;

	Log(LOG_DEBUG,"Read login packet...");

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);

	p->decodeBlowfish(gEnv->bBlowFish);

	// Packet header
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	std::string version        = p->readString();             // Packet version
	//

	loginPacket.login    = p->readString();                   // Login
	loginPacket.password = p->readString();                   // Password


	std::string endBlock     = p->readString();               // End block

	if(strcmp(endBlock.c_str(),EndBlock))
		Log(LOG_WARNING,"Login packet damaged!");

	if(gEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		Log(LOG_DEBUG,"Login packet size = %d",size);
		Log(LOG_DEBUG,"Login : %s",loginPacket.login);
		Log(LOG_DEBUG,"Password : %s", loginPacket.password);
		
		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	return loginPacket;
}

SLoginPacket CReadSendPacket::ReadRegistrationPacket (SPacket packet)
{
	SLoginPacket registerPacket;

	Log(LOG_DEBUG,"Read registration packet...");

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);

	p->decodeBlowfish(gEnv->bBlowFish);

	// Packet header
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	std::string version        = p->readString();             // Packet version
	//

	registerPacket.login    = p->readString();                // Login
	registerPacket.password = p->readString();                // Password 
	registerPacket.nickname = p->readString();                // Nickname    

	std::string endBlock     = p->readString();               // End block

	if(strcmp(endBlock.c_str(), EndBlock))
		Log(LOG_WARNING,"Registration packet damaged!");

	if(gEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		Log(LOG_DEBUG,"Registration packet size = %d",size);
		Log(LOG_DEBUG,"Login : %s",registerPacket.login);
		Log(LOG_DEBUG,"Password : %s",registerPacket.password);
		Log(LOG_DEBUG,"Nickname : %s",registerPacket.nickname);

		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	return registerPacket;
}

SMessage CReadSendPacket::ReadMsg(SPacket packet)
{
	SMessage message;

	Log(LOG_DEBUG,"Read MSG packet...");

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);

	p->decodeBlowfish(gEnv->bBlowFish);

	// Packet header
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	const char* version        = p->readString();             // Packet version
	//

	message.message    = p->readString();                     // Message
	message.area       = (EChatMessageArea)p->readInt();      // Message area      

	const char* endBlock = p->readString();

	if(strcmp(endBlock,EndBlock))
		Log(LOG_WARNING,"MSG packet damaged!");

	if(gEnv->bDebugMode)
	{
		Log(LOG_DEBUG,"MSG packet size = %d",packet.size);
		Log(LOG_DEBUG,"MSG packet data = %s", message.message);
		Log(LOG_DEBUG,"MSG packet area = %d", message.area);


		PacketDebugger::Debug(packet.data, packet.size, "ReceivedPacketsDebugg.txt");
	}

	delete p;
	return message;
}

SRequestPacket CReadSendPacket::ReadRequest(SPacket packet)
{
	SRequestPacket request;

	Log(LOG_DEBUG, "Read client request packet...");

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);

	p->decodeBlowfish(gEnv->bBlowFish);

	// Packet header
	EPacketType type = (EPacketType)p->readInt();             // Packet type
	const char* version        = p->readString();             // Packet version
	//


	request.request = p->readString();                        // Request
	request.sParam = p->readString();                         // String param
	request.iParam = p->readInt();                            // Int param


	const char* endBlock = p->readString(); 
	if(strcmp(endBlock, EndBlock))
		Log(LOG_WARNING,"Client request packet damaged!");

	if(gEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		Log(LOG_DEBUG,"Request size = %d", size);
		Log(LOG_DEBUG,"Request = %s", request.request);
		Log(LOG_DEBUG,"Request sParam = %s", request.sParam);
		Log(LOG_DEBUG,"Request iParam = %d", request.iParam);

		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	delete p;
	return request;
}

SGameServer CReadSendPacket::ReadGameServerInfo(SPacket packet)
{
	Log(LOG_DEBUG,"Read game server info packet...");

	SGameServer Server;

	Packet* p = new Packet((const unsigned char*)packet.data, packet.size);

	p->decodeBlowfish(gEnv->bBlowFish);

	// Packet header
	EPacketType type = (EPacketType)p->readInt();           // Packet type
	const char* version        = p->readString();           // Packet version
	//

	Server.id              = p->readInt();                  // id
	Server.ip              = p->readString();               // ip          
	Server.port            = p->readInt();                  // port
	Server.serverName      = p->readString();               // name
	Server.currentPlayers  = p->readInt();                  // players online
	Server.maxPlayers      = p->readInt();                  // max players
	Server.mapName         = p->readString();               // map name
	Server.gameRules       = p->readString();               // game rules

	const char* endBlock = p->readString();                 // End block

	if(strcmp(endBlock,EndBlock))
		Log(LOG_WARNING,"Game server info packet damaged!");

	if(gEnv->bDebugMode)
	{
		int size = p->getPacketSize();
		char* Packet = (char*)p->getBytesPtr();

		Log(LOG_DEBUG,"Game Server info packet size = %d",size);
		Log(LOG_DEBUG,"Game Server ip = %s",Server.ip);
		Log(LOG_DEBUG,"Game Server port = %d",Server.port);
		Log(LOG_DEBUG,"Game Server name = %s",Server.serverName);
		Log(LOG_DEBUG,"Game Server online = %d",Server.currentPlayers);
		Log(LOG_DEBUG,"Game Server max players = %d",Server.maxPlayers);
		Log(LOG_DEBUG,"Game Server map name = %s",Server.mapName);
		Log(LOG_DEBUG,"Game Server gamerules = %s",Server.gameRules);

	
		PacketDebugger::Debug(Packet, size, "ReceivedPacketsDebugg.txt");
	}

	delete p;
	return Server;
}
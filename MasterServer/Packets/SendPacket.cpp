/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
--------------------------------------------------------------------------
History:

- 26.08.2014   13:49 : Created by AfroStalin(chernecoff)
- 22.05.2015   12:24 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/
#include "StdAfx.h"
#include <winsock.h>

#include "Packets\Packets.h"
#include "Packets\PacketDebugger.h"
#include "Packets\RSP.h"

void CReadSendPacket::SendIdentificationPacket(SOCKET Socket)
{
	Log(LOG_DEBUG,"Send identification packet to client...");

	SPacket SPacket;
	Packet* p = new Packet();
	p->create();

	// Header
	p->writeInt(PACKET_IDENTIFICATION);                          // Packet type
	p->writeString(gEnv->serverVersion);                         // Packet version
	// 

	p->writeString(EndBlock);                                    // End block

	p->padPacketTo8ByteLen();
	p->encodeBlowfish(gEnv->bBlowFish);
	p->appendChecksum(false);
	p->appendMore8Bytes();


	int size = p->getPacketSize();
	char* packet = (char*)p->getBytesPtr();

	SPacket.addr = Socket;
	SPacket.data = packet;
	SPacket.size = size;

	gEnv->pPacketQueue->InsertPacketToSend(SPacket);

	if(gEnv->bDebugMode)
	{
		Log(LOG_DEBUG,"Identification packet size = %d",size);

		PacketDebugger::Debug(packet, size, "SendPacketsDebugg.txt");
	}

	gEnv->outPackets++;
}

void CReadSendPacket::SendMsg(SOCKET Socket, SMessage message)
{
	Log(LOG_DEBUG,"Send message packet to client...");

	SPacket SPacket;
	Packet* p = new Packet();
	p->create();

	// Header
	p->writeInt(PACKET_MESSAGE);                                 // Packet type
	p->writeString(gEnv->serverVersion);                         // Version
	//

	p->writeString(message.message);                             // Message
	p->writeInt(message.area);                                   // Message area

	p->writeString(EndBlock);                                    // End block

	p->padPacketTo8ByteLen();
	p->encodeBlowfish(gEnv->bBlowFish);
	p->appendChecksum(false);
	p->appendMore8Bytes();


	int size = p->getPacketSize();
	char* packet = (char*)p->getBytesPtr();

	SPacket.addr = Socket;
	SPacket.data = packet;
	SPacket.size = size;

	gEnv->pPacketQueue->InsertPacketToSend(SPacket);


	if(gEnv->bDebugMode)
	{
		Log(LOG_DEBUG,"Message packet size = %d",size);
		Log(LOG_DEBUG,"Message packet data = %s",message.message);
		Log(LOG_DEBUG,"Message packet type = %d",message.area);

		PacketDebugger::Debug(packet, size, "SendPacketsDebugg.txt");
	}

	gEnv->outPackets++;
}

void CReadSendPacket::SendAccountInfo (SOCKET Socket, SClient player)
{
	Log(LOG_DEBUG,"Send account info to client...");
	SPacket SPacket;
	Packet* p = new Packet();
	p->create();

	// Header
	p->writeInt(PACKET_ACCOUNT);                                 // Packet type
	p->writeString(gEnv->serverVersion);                         // Packet versions
	//

	p->writeInt(player.playerId);                                // Player id
	p->writeString(player.nickname.c_str());                     // Player nickname
	p->writeInt(player.xp);                                      // Player exp
	p->writeInt(player.level);                                   // Player level
	p->writeInt(player.money);                                   // Player money
	p->writeInt(player.banStatus);                               // Player ban status...hm

	p->writeString(EndBlock);                                    // End block

	p->padPacketTo8ByteLen();
	p->encodeBlowfish(gEnv->bBlowFish);
	p->appendChecksum(false);
	p->appendMore8Bytes();


	int size = p->getPacketSize();
	char* packet = (char*)p->getBytesPtr();

	SPacket.addr = Socket;
	SPacket.data = packet;
	SPacket.size = size;

	gEnv->pPacketQueue->InsertPacketToSend(SPacket);

	if(gEnv->bDebugMode)
	{
		Log(LOG_DEBUG,"Account info packet size = %d", size);
		Log(LOG_DEBUG,"Player id = %d", player.playerId);
		Log(LOG_DEBUG,"Player nickname = %s", player.nickname.c_str());
		Log(LOG_DEBUG,"Player xp = %d", player.xp);
		Log(LOG_DEBUG,"Player level = %d", player.level);
		Log(LOG_DEBUG,"Player game money ammount = %d", player.money);
		Log(LOG_DEBUG,"Player ban status = %d", player.banStatus);

		PacketDebugger::Debug(packet, size, "SendPacketsDebugg.txt");
	}

	gEnv->outPackets++;
}

void CReadSendPacket::SendMasterServerInfo (SOCKET Socket, SMasterServerInfo info)
{
	Log(LOG_DEBUG,"Send master server info to client...");

	SPacket SPacket;
	Packet* p = new Packet();
	p->create();

	// Header
	p->writeInt(PACKET_MS_INFO);                                 // Packet type
	p->writeString(gEnv->serverVersion);                         // Packet version
	//

	p->writeInt(info.playersOnline);                             // Player online
	p->writeInt(info.gameServersOnline);                         // Game servers online

	p->writeString(EndBlock);                                    // End block

	p->padPacketTo8ByteLen();
	p->encodeBlowfish(gEnv->bBlowFish);
	p->appendChecksum(false);
	p->appendMore8Bytes();


	int size = p->getPacketSize();
	char* packet = (char*)p->getBytesPtr();

	SPacket.addr = Socket;
	SPacket.data = packet;
	SPacket.size = size;

	gEnv->pPacketQueue->InsertPacketToSend(SPacket);

	if(gEnv->bDebugMode)
	{
		Log(LOG_DEBUG,"Server info packet size = %d",size);
		Log(LOG_DEBUG,"Players onlain = %d",info.playersOnline);
		Log(LOG_DEBUG,"Game servers onlain = %d",info.gameServersOnline);

		PacketDebugger::Debug(packet, size, "SendPacketsDebugg.txt");
	}
	gEnv->outPackets++;
}

void CReadSendPacket::SendGameServerInfo (SOCKET Socket, SGameServer server)
{
	SPacket SPacket;

	Log(LOG_DEBUG,"Send game server info to client...");

	Packet* p = new Packet();

	p->create();

	// Header
	p->writeInt(PACKET_GAME_SERVER);                             // Packet type
	p->writeString(gEnv->serverVersion);                         // Packet version
	//
        
	p->writeInt(server.id);                                      // id
	p->writeString(server.ip);                                   // ip
	p->writeInt(server.port);                                    // port
	p->writeString(server.serverName);                           // server name
	p->writeInt(server.currentPlayers);                          // players online
	p->writeInt(server.maxPlayers);                              // max players
	p->writeString(server.mapName);                              // map name
	p->writeString(server.gameRules);                            // game rules

	p->writeString(EndBlock);                                    // End block

	p->padPacketTo8ByteLen();
	p->encodeBlowfish(gEnv->bBlowFish);
	p->appendChecksum(false);
	p->appendMore8Bytes();


	int size = p->getPacketSize();
	char* packet = (char*)p->getBytesPtr();

	SPacket.addr = Socket;
	SPacket.data = packet;
	SPacket.size = size;

	gEnv->pPacketQueue->InsertPacketToSend(SPacket);

	if(gEnv->bDebugMode)
	{
		Log(LOG_DEBUG,"Game Server info packet size = %d",size);
		Log(LOG_DEBUG,"Game Server ip = %s",server.ip);
		Log(LOG_DEBUG,"Game Server port = %d",server.port);
		Log(LOG_DEBUG,"Game Server name = %s",server.serverName);
		Log(LOG_DEBUG,"Game Server onlain = %d",server.currentPlayers);
		Log(LOG_DEBUG,"Game Server max players = %d",server.maxPlayers);
		Log(LOG_DEBUG,"Game Server map name = %s",server.mapName);
		Log(LOG_DEBUG,"Game Server gamerules = %s",server.gameRules);


		PacketDebugger::Debug(packet, size, "SendPacketsDebugg.txt");

	}
	gEnv->outPackets++;
}

void CReadSendPacket::SendGameServers( SOCKET Socket)
{
	SPacket SPacket;

	Log(LOG_DEBUG,"Send all game servers to client...");

	Packet* p = new Packet();

	p->create();

	// Header
	p->writeInt(PACKET_GAME_SERVERS);                            // Packet type
	p->writeString(gEnv->serverVersion);                         // Packet version
	//

	
	p->writeInt ((int)gEnv->pServer->vServers.size());           // size servers vector
	
	
	for( auto it = gEnv->pServer->vServers.begin(); it != gEnv->pServer->vServers.end(); ++it)
	{
		p->writeInt(it->id); 
		p->writeString(it->ip);                                
		p->writeInt(it->port);                               
		p->writeString(it->serverName);                       
		p->writeInt(it->currentPlayers);                   
		p->writeInt(it->maxPlayers);                     
		p->writeString(it->mapName);                          
		p->writeString(it->gameRules);
	}

	p->writeString(EndBlock);                                    // End block

	p->padPacketTo8ByteLen();
	p->encodeBlowfish(gEnv->bBlowFish);
	p->appendChecksum(false);
	p->appendMore8Bytes();


	int size = p->getPacketSize();
	char* packet = (char*)p->getBytesPtr();

	SPacket.addr = Socket;
	SPacket.data = packet;
	SPacket.size = size;

	gEnv->pPacketQueue->InsertPacketToSend(SPacket);
	gEnv->outPackets++;
}

void CReadSendPacket::SendRequest(SOCKET Socket, SRequestPacket request)
{
	SPacket SPacket;

	Log(LOG_DEBUG,"Send request to client...");

	Packet* p = new Packet();

	p->create();

	// Header
	p->writeInt(PACKET_REQUEST);                                 // Packet type
	p->writeString(gEnv->serverVersion);                         // Packet version
	//

	p->writeString(request.request);                             // Request
	p->writeString(request.sParam);                              // String param
	p->writeInt(request.iParam);                                 // Int param

	p->writeString(EndBlock);                                    // End block

	p->padPacketTo8ByteLen();
	p->encodeBlowfish(gEnv->bBlowFish);
	p->appendChecksum(false);
	p->appendMore8Bytes();


	int size = p->getPacketSize();
	char* packet = (char*)p->getBytesPtr();

	SPacket.addr = Socket;
	SPacket.data = packet;
	SPacket.size = size;

	gEnv->pPacketQueue->InsertPacketToSend(SPacket);

	if(gEnv->bDebugMode)
	{
		Log(LOG_DEBUG,"Request packet size = %d",size);
		Log(LOG_DEBUG,"Request = %s",request.request);
		Log(LOG_DEBUG,"Request sParam = %s", request.sParam);
		Log(LOG_DEBUG,"Request iParam = %d", request.iParam);

		PacketDebugger::Debug(packet, size, "SendPacketsDebugg.txt");
	}
	gEnv->outPackets++;
}
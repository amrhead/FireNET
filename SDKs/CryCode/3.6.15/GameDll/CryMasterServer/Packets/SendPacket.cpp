/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014-2015
-------------------------------------------------------------------------
Description: Sending packets
-------------------------------------------------------------------------
History:

- 26.08.2014   13:49 : Created by AfroStalin(chernecoff)
- 19.03.2015   00:03 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#include "StdAfx.h"

#include "../System/Global.h"
#include "Packets.h"
#include "PacketDebugger.h"
#include "RSP.h"

void CReadSendPacket::SendIdentificationPacket(SOCKET Socket)
{
	//CryLog("[CryMasterServer] Send identification packet...");

	SPacket SPacket;
	Packet* p = new Packet();
	p->create();


	/****************************Обязательный блок************************************/

	p->writeInt(PACKET_IDENTIFICATION);                          // Тип пакета
	p->writeString(gClientEnv->clientVersion);           // Версия пакета

	/*******************************Тело пакета****************************************/

	p->writeString(EndBlock);                                    // Завершающий блок

	p->padPacketTo8ByteLen();
	p->encodeBlowfish(gClientEnv->bBlowFish);
	p->appendChecksum(false);
	p->appendMore8Bytes();


	int size = p->getPacketSize();
	char* packet = (char*)p->getBytesPtr();

	SPacket.addr = Socket;
	SPacket.data = packet;
	SPacket.size = size;

	gClientEnv->pPacketQueue->InsertPacket(SPacket);

	if(gClientEnv->bDebugMode)
	{
		CryLog("[CryMasterServer] Identification packet size = %d",size);

		PacketDebugger::Debug(packet, size, "SendPacketsDebugg.txt");
	}
}

void CReadSendPacket::SendMsg(SOCKET Socket, SMessage message)
{
	//CryLog("[CryMasterServer] Send message packet...");

	SPacket SPacket;
	Packet* p = new Packet();
	p->create();

	/****************************Обязательный блок************************************/

	p->writeInt(PACKET_MESSAGE);                                 // Тип пакета
	p->writeString(gClientEnv->clientVersion);           // Версия пакета

	/*******************************Тело пакета****************************************/

	p->writeString(message.message);
	p->writeInt(message.area);                            

	p->writeString(EndBlock);                                    // Завершающий блок 

	p->padPacketTo8ByteLen();
	p->encodeBlowfish(gClientEnv->bBlowFish);
	p->appendChecksum(false);
	p->appendMore8Bytes();


	int size = p->getPacketSize();
	char* packet = (char*)p->getBytesPtr();

	SPacket.addr = Socket;
	SPacket.data = packet;
	SPacket.size = size;

	gClientEnv->pPacketQueue->InsertPacket(SPacket);


	if(gClientEnv->bDebugMode)
	{
		CryLog("[CryMasterServer] Message packet size = %d",size);
		CryLog("[CryMasterServer] Message packet data = %s",message.message);
		CryLog("[CryMasterServer] Message packet type = %d",message.area);

		PacketDebugger::Debug(packet, size, "SendPacketsDebugg.txt");
	}
}

void CReadSendPacket::SendGameServerInfo (SOCKET Socket, SGameServer server)
{
	SPacket SPacket;

	//CryLog("[CryMasterServer] Send game server info...");

	Packet* p = new Packet();

	p->create();


	/****************************Обязательный блок************************************/

	p->writeInt(PACKET_GAME_SERVER);                     // Тип пакета
	p->writeString(gClientEnv->clientVersion);           // Версия пакета

	/*******************************Тело пакета****************************************/
                                    
	p->writeString(server.ip);                                
	p->writeInt(server.port);                               
	p->writeString(server.serverName);                       
	p->writeInt(server.currentPlayers);                   
	p->writeInt(server.maxPlayers);                     
	p->writeString(server.mapName);                          
	p->writeString(server.gameRules);                     

	p->writeString(EndBlock);                                    // Завершающий блок

	p->padPacketTo8ByteLen();
	p->encodeBlowfish(gClientEnv->bBlowFish);
	p->appendChecksum(false);
	p->appendMore8Bytes();


	int size = p->getPacketSize();
	char* packet = (char*)p->getBytesPtr();

	SPacket.addr = Socket;
	SPacket.data = packet;
	SPacket.size = size;

	gClientEnv->pPacketQueue->InsertPacket(SPacket);

	if(gClientEnv->bDebugMode)
	{
		CryLog("[CryMasterServer] Game Server info packet size = %d",size);
		CryLog("[CryMasterServer] Game Server ip = %s",server.ip);
		CryLog("[CryMasterServer] Game Server port = %d",server.port);
		CryLog("[CryMasterServer] Game Server name = %s",server.serverName);
		CryLog("[CryMasterServer] Game Server onlain = %d",server.currentPlayers);
		CryLog("[CryMasterServer] Game Server max players = %d",server.maxPlayers);
		CryLog("[CryMasterServer] Game Server map name = %s",server.mapName);
		CryLog("[CryMasterServer] Game Server gamerules = %s",server.gameRules);


		PacketDebugger::Debug(packet, size, "SendPacketsDebugg.txt");

	}
}

void CReadSendPacket::SendRequest(SOCKET Socket, SRequestPacket request)
{
	SPacket SPacket;

	//CryLog("[CryMasterServer] Send request...");

	Packet* p = new Packet();

	p->create();


	/****************************Обязательный блок************************************/

	p->writeInt(PACKET_REQUEST);                                 // Тип пакета
	p->writeString(gClientEnv->clientVersion);           // Версия пакета

	/*******************************Тело пакета****************************************/

	p->writeString(request.request);
	p->writeString(request.sParam);
	p->writeInt(request.iParam);

	p->writeString(EndBlock);                                    // Завершающий блок 

	p->padPacketTo8ByteLen();
	p->encodeBlowfish(gClientEnv->bBlowFish);
	p->appendChecksum(false);
	p->appendMore8Bytes();


	int size = p->getPacketSize();
	char* packet = (char*)p->getBytesPtr();

	SPacket.addr = Socket;
	SPacket.data = packet;
	SPacket.size = size;

	gClientEnv->pPacketQueue->InsertPacket(SPacket);

	if(gClientEnv->bDebugMode)
	{
		CryLog("[CryMasterServer] Request packet size = %d",size);
		CryLog("[CryMasterServer] Request = %s",request.request);
		CryLog("[CryMasterServer] Request sParam = %s", request.sParam);
		CryLog("[CryMasterServer] Request iParam = %d", request.iParam);

		PacketDebugger::Debug(packet, size, "SendPacketsDebugg.txt");
	}
}

void CReadSendPacket::SendLoginPacket(SOCKET Socket, SLoginPacket packet)
{
	//CryLog("[CryMasterServer] Send login packet...");

	SPacket SPacket;
	Packet* p = new Packet();
	p->create();


	/****************************Обязательный блок************************************/

	p->writeInt(PACKET_LOGIN);                                   // Тип пакета
	p->writeString(gClientEnv->clientVersion);           // Версия пакета

	/*******************************Тело пакета****************************************/

	p->writeString(packet.login);
	p->writeString(packet.password);

	p->writeString(EndBlock);                                    // Завершающий блок

	p->padPacketTo8ByteLen();
	p->encodeBlowfish(gClientEnv->bBlowFish);
	p->appendChecksum(false);
	p->appendMore8Bytes();


	int size = p->getPacketSize();
	char* Packet = (char*)p->getBytesPtr();

	SPacket.addr = Socket;
	SPacket.data = Packet;
	SPacket.size = size;

	gClientEnv->pPacketQueue->InsertPacket(SPacket);

	if(gClientEnv->bDebugMode)
	{
		CryLog("[CryMasterServer] Login packet size = %d",size);

		PacketDebugger::Debug(Packet, size, "SendPacketsDebugg.txt");
	}
}

void CReadSendPacket::SendRegisterPacket(SOCKET Socket, SLoginPacket packet)
{
	//CryLog("[CryMasterServer] Send register packet...");

	SPacket SPacket;
	Packet* p = new Packet();
	p->create();


	/****************************Обязательный блок************************************/

	p->writeInt(PACKET_REGISTER);                                              // Тип пакета
	p->writeString(gClientEnv->clientVersion);                         // Версия пакета

	/*******************************Тело пакета****************************************/

	p->writeString(packet.login);
	p->writeString(packet.password);
	p->writeString(packet.nickname);

	p->writeString(EndBlock);                                    // Завершающий блок

	p->padPacketTo8ByteLen();
	p->encodeBlowfish(gClientEnv->bBlowFish);
	p->appendChecksum(false);
	p->appendMore8Bytes();


	int size = p->getPacketSize();
	char* Packet = (char*)p->getBytesPtr();

	SPacket.addr = Socket;
	SPacket.data = Packet;
	SPacket.size = size;

	gClientEnv->pPacketQueue->InsertPacket(SPacket);

	if(gClientEnv->bDebugMode)
	{
		CryLog("[CryMasterServer] Register packet size = %d",size);

		PacketDebugger::Debug(Packet, size, "SendPacketsDebugg.txt");
	}
}
/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
--------------------------------------------------------------------------
History:

- 26.08.2014   13:49 : Created by AfroStalin(chernecoff)
- 22.05.2015   08:40 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#include "StdAfx.h"

#include "../System/Global.h"
#include "Packets.h"
#include "PacketDebugger.h"
#include "RSP.h"

void CReadSendPacket::SendIdentificationPacket(SOCKET Socket)
{
	//gEnv->pLog->Log(TITLE "Send identification packet...");

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
		gEnv->pLog->Log(TITLE "Identification packet size = %d",size);

		PacketDebugger::Debug(packet, size, "SendPacketsDebugg.txt");
	}
}

void CReadSendPacket::SendMsg(SOCKET Socket, SMessage message)
{
	//gEnv->pLog->Log(TITLE "Send message packet...");

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
		gEnv->pLog->Log(TITLE "Message packet size = %d",size);
		gEnv->pLog->Log(TITLE "Message packet data = %s",message.message);
		gEnv->pLog->Log(TITLE "Message packet type = %d",message.area);

		PacketDebugger::Debug(packet, size, "SendPacketsDebugg.txt");
	}
}

void CReadSendPacket::SendGameServerInfo (SOCKET Socket, SGameServer server)
{
	SPacket SPacket;

	//gEnv->pLog->Log(TITLE "Send game server info...");

	Packet* p = new Packet();

	p->create();


	/****************************Обязательный блок************************************/

	p->writeInt(PACKET_GAME_SERVER);                     // Тип пакета
	p->writeString(gClientEnv->clientVersion);           // Версия пакета

	/*******************************Тело пакета****************************************/
              
	p->writeInt(0);
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
		gEnv->pLog->Log(TITLE "Game Server info packet size = %d",size);
		gEnv->pLog->Log(TITLE "Game Server ip = %s",server.ip);
		gEnv->pLog->Log(TITLE "Game Server port = %d",server.port);
		gEnv->pLog->Log(TITLE "Game Server name = %s",server.serverName);
		gEnv->pLog->Log(TITLE "Game Server onlain = %d",server.currentPlayers);
		gEnv->pLog->Log(TITLE "Game Server max players = %d",server.maxPlayers);
		gEnv->pLog->Log(TITLE "Game Server map name = %s",server.mapName);
		gEnv->pLog->Log(TITLE "Game Server gamerules = %s",server.gameRules);


		PacketDebugger::Debug(packet, size, "SendPacketsDebugg.txt");

	}
}

void CReadSendPacket::SendRequest(SOCKET Socket, SRequestPacket request)
{
	SPacket SPacket;

	//gEnv->pLog->Log(TITLE "Send request...");

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
		gEnv->pLog->Log(TITLE "Request packet size = %d",size);
		gEnv->pLog->Log(TITLE "Request = %s",request.request);
		gEnv->pLog->Log(TITLE "Request sParam = %s", request.sParam);
		gEnv->pLog->Log(TITLE "Request iParam = %d", request.iParam);

		PacketDebugger::Debug(packet, size, "SendPacketsDebugg.txt");
	}
}

void CReadSendPacket::SendLoginPacket(SOCKET Socket, SLoginPacket packet)
{
	//gEnv->pLog->Log(TITLE "Send login packet...");

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
		gEnv->pLog->Log(TITLE "Login packet size = %d",size);

		PacketDebugger::Debug(Packet, size, "SendPacketsDebugg.txt");
	}
}

void CReadSendPacket::SendRegisterPacket(SOCKET Socket, SLoginPacket packet)
{
	//gEnv->pLog->Log(TITLE "Send register packet...");

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
		gEnv->pLog->Log(TITLE "Register packet size = %d",size);

		PacketDebugger::Debug(Packet, size, "SendPacketsDebugg.txt");
	}
}

void CReadSendPacket::SendConsoleTextPacket(SOCKET Socket, int textType, const char* text)
{
	//gEnv->pLog->Log(TITLE "Send console text packet...");

	SPacket SPacket;
	Packet* p = new Packet();
	p->create();


	/****************************Обязательный блок************************************/

	p->writeInt(PACKET_CONSOLE_TEXT);                    // Тип пакета
	p->writeString(gClientEnv->clientVersion);           // Версия пакета

	/*******************************Тело пакета****************************************/

	p->writeInt(textType);
	p->writeString(text);

	p->writeString(EndBlock);                            // Завершающий блок

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
}
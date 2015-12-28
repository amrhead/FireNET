/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014-2015
-------------------------------------------------------------------------
Description: Sending packets
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

void CReadSendPacket::SendIdentificationPacket(SOCKET Socket)
{
	printf("Send identification packet to client...\n");

	SPacket SPacket;
	Packet* p = new Packet();
	p->create();


	/****************************Обязательный блок************************************/

	p->writeInt(PACKET_IDENTIFICATION);                          // Тип пакета
	p->writeString(gClientEnv->clientVersion.c_str());                         // Версия пакета

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
		printf("Identification packet size = %d\n",size);

		PacketDebugger::Debug(packet, size, "SendPacketsDebugg.txt");
	}

	//gClientEnv->outPackets++;
}

void CReadSendPacket::SendMsg(SOCKET Socket, SMessage message)
{
	printf("Send message packet to client...\n");

	SPacket SPacket;
	Packet* p = new Packet();
	p->create();

	/****************************Обязательный блок************************************/

	p->writeInt(PACKET_MESSAGE);                                 // Тип пакета
	p->writeString(gClientEnv->clientVersion.c_str());                         // Версия пакета

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
		printf("Message packet size = %d\n",size);
		printf("Message packet data = %s\n",message.message);
		printf("Message packet type = %d\n",message.area);

		PacketDebugger::Debug(packet, size, "SendPacketsDebugg.txt");
	}

	//gClientEnv->outPackets++;
}

#if defined _SERVER
void CReadSendPacket::SendAccountInfo (SOCKET Socket, SPlayer player)
{
	printf("Send account info to client...");
	SPacket SPacket;
	Packet* p = new Packet();
	p->create();


	/****************************Обязательный блок************************************/

	p->writeInt(PACKET_ACCOUNT);                                 // Тип пакета
	p->writeString(gClientEnv->clientVersion.c_str());                         // Версия пакета

	/*******************************Тело пакета****************************************/

	p->writeInt(player.playerID);    
	p->writeString(player.nickName.c_str());                        
	p->writeInt(player.level);                                  
	p->writeInt(player.gameCash);                              
	p->writeInt(player.realCash);
	p->writeInt(player.banStatus);

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
		printf("Account info packet size = %d", size);
		printf("Player id = %d", player.playerID);
		printf("Player nickname = %s", player.nickName);
		printf("Player level = %d", player.level);
		printf("Player gamecash ammount = %d", player.gameCash);
		printf("Player realcash ammount = %d", player.realCash);
		printf("Player ban status = %d", player.banStatus);

		PacketDebugger::Debug(packet, size, "SendPacketsDebugg.txt");
	}

	gClientEnv->outPackets++;
}

void CReadSendPacket::SendMasterServerInfo (SOCKET Socket, SMasterServerInfo info)
{
	printf("Send master server info to client...");

	SPacket SPacket;
	Packet* p = new Packet();
	p->create();


	/****************************Обязательный блок************************************/

	p->writeInt(PACKET_MS_INFO);                                 // Тип пакета
	p->writeString(gClientEnv->clientVersion.c_str());                         // Версия пакета

	/*******************************Тело пакета****************************************/

	p->writeInt(info.playersOnline);
	p->writeInt(info.gameServersOnline);

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
		printf("Server info packet size = %d",size);
		printf("Players onlain = %d",info.playersOnline);
		printf("Game servers onlain = %d",info.gameServersOnline);

		PacketDebugger::Debug(packet, size, "SendPacketsDebugg.txt");
	}
	gClientEnv->outPackets++;
}
#endif

void CReadSendPacket::SendGameServerInfo (SOCKET Socket, SGameServer server)
{
	SPacket SPacket;

	printf("Send game server info to client...\n");

	Packet* p = new Packet();

	p->create();


	/****************************Обязательный блок************************************/

	p->writeInt(PACKET_GAME_SERVER);                             // Тип пакета
	p->writeString(gClientEnv->clientVersion.c_str());                         // Версия пакета

	/*******************************Тело пакета****************************************/
                                    
	p->writeString(server.ip);                                
	p->writeInt(server.port);                               
	p->writeString(server.serverName);                       
	p->writeInt(server.currentPlayers);                   
	p->writeInt(server.maxPlayers);                     
	p->writeString(server.mapName.c_str());                          
	p->writeString(server.gameRules.c_str());                     

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
		printf("Game Server info packet size = %d\n",size);
		printf("Game Server ip = %s\n",server.ip);
		printf("Game Server port = %d\n",server.port);
		printf("Game Server name = %s\n",server.serverName);
		printf("Game Server onlain = %d\n",server.currentPlayers);
		printf("Game Server max players = %d\n",server.maxPlayers);
		printf("Game Server map name = %s\n",server.mapName);
		printf("Game Server gamerules = %s\n",server.gameRules);


		PacketDebugger::Debug(packet, size, "SendPacketsDebugg.txt");

	}
	//gClientEnv->outPackets++;
}

void CReadSendPacket::SendRequest(SOCKET Socket, SRequestPacket request)
{
	SPacket SPacket;

	printf("Send request to client...\n");

	Packet* p = new Packet();

	p->create();


	/****************************Обязательный блок************************************/

	p->writeInt(PACKET_REQUEST);                                 // Тип пакета
	p->writeString(gClientEnv->clientVersion.c_str());                         // Версия пакета

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
		printf("Request packet size = %d\n",size);
		printf("Request = %s\n",request.request);
		printf("Request sParam = %s\n", request.sParam);
		printf("Request iParam = %d\n", request.iParam);

		PacketDebugger::Debug(packet, size, "SendPacketsDebugg.txt");
	}
}

#if defined _CLIENT
void CReadSendPacket::SendLoginPacket(SOCKET Socket, SLoginPacket packet)
{
	printf("Send login packet...\n");

	SPacket SPacket;
	Packet* p = new Packet();
	p->create();


	/****************************Обязательный блок************************************/

	p->writeInt(PACKET_LOGIN);                                   // Тип пакета
	p->writeString(gClientEnv->clientVersion.c_str());           // Версия пакета

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
		printf("Login packet size = %d\n",size);

		PacketDebugger::Debug(Packet, size, "SendPacketsDebugg.txt");
	}

	//gClientEnv->outPackets++;
}

void CReadSendPacket::SendRegisterPacket(SOCKET Socket, SLoginPacket packet)
{
	printf("Send register packet...\n");

	SPacket SPacket;
	Packet* p = new Packet();
	p->create();


	/****************************Обязательный блок************************************/

	p->writeInt(PACKET_LOGIN);                                   // Тип пакета
	p->writeString(gClientEnv->clientVersion.c_str());                         // Версия пакета

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
		printf("Register packet size = %d\n",size);

		PacketDebugger::Debug(Packet, size, "SendPacketsDebugg.txt");
	}

	//gClientEnv->outPackets++;
}
#endif
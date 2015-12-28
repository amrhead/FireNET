/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014-2015
-------------------------------------------------------------------------

-------------------------------------------------------------------------
History:

- 10.01.2015   21:34 : Created by AfroStalin(chenecoff)
- 30.04.2015   22:56 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/
#include "StdAfx.h"
#include <thread>
#include <mutex>
#include <winsock.h>

#include "../System/Global.h"
#include "PacketQueue.h"

CPacketQueue::CPacketQueue()
{
	packetsInSendQueue = 0;
	packetsInReadQueue = 0;
}

void CPacketQueue::Init()
{
	CryLog("[CryMasterServer] CPacketQueue::Init()");

	std::thread Thread(&CPacketQueue::Thread, this);
	Thread.detach();
}

void CPacketQueue::InsertPacket(SPacket packet)
{
	send_mutex.lock();
	SendPackets.push_back(packet);
	packetsInSendQueue++;
	send_mutex.unlock();

	//CryLog("[CryMasterServer] CPacketQueue::New packet added to send queue");
}

void CPacketQueue::InsertPacketToRead(SPacket packet)
{
	read_mutex.lock();
	ReadPackets.push_back(packet);
	packetsInReadQueue++;
	read_mutex.unlock();

	//CryLog("[CryMasterServer] CPacketQueue::New packet added to read queue");
}

void CPacketQueue::Thread()
{
	CryLog("[CryMasterServer] Send/Read packets thread started!");

	while(true)
	{	
		send_mutex.lock();
		if(packetsInSendQueue>0)
		{
			//CryLog("[CryMasterServer] Sending packet...");

			SPacket packet;
			std::vector <SPacket>::iterator it;

			it = SendPackets.begin();

			packet.addr = (*it).addr;
			packet.data = (*it).data;
			packet.size = (*it).size;
			
			send(packet.addr,packet.data,packet.size,0);

			SendPackets.erase(it);
			packetsInSendQueue--;
		}
		send_mutex.unlock();

		read_mutex.lock();
		if(packetsInReadQueue>0)
		{
			//CryLog("[CryMasterServer] Reding packet...");

			SPacket Packet;
			SPlayer Player;

			std::vector <SPacket>::iterator it;

			it = ReadPackets.begin();

			Packet.addr =  (*it).addr;
			Packet.data = (*it).data;
			Packet.size = (*it).size;

			EPacketType packetType = gClientEnv->pRsp->GetPacketType(Packet);

			switch (packetType)
			{
			case PACKET_IDENTIFICATION:
				CryLog("[CryMasterServer] Identification packet recived");
				break;
			case PACKET_ACCOUNT:
				{
					CryLog("[CryMasterServer] Account info packet recived");

					gClientEnv->masterPlayer = gClientEnv->pRsp->ReadAccountInfo(Packet);

					SPlayer Player;
					SUIArguments args;

					Player = gClientEnv->masterPlayer;

					if(Player.playerId)
					{
						args.AddArgument(Player.nickname.c_str());
						args.AddArgument(Player.level);
						args.AddArgument(Player.money);
						args.AddArgument(Player.xp);

						CMsEvents::GetInstance()->SendEvent(CMsEvents::eUIGE_AccountInfoResived, args);
					}

					break;
				}
			case PACKET_MESSAGE:
				{
					SMessage Message = gClientEnv->pRsp->ReadMsg(Packet);

					if(strcmp(Message.message,""))
					{

						switch (Message.area)
						{
						case CHAT_MESSAGE_GLOBAL:
							{
								CryLog("[CryMasterServer] Global chat message recived");

								SUIArguments args;
								args.AddArgument(Message.message);
								CMsEvents::GetInstance()->SendEvent(CMsEvents::eUIGE_MsgResived, args);

								break;
							}
						case CHAT_MESSAGE_PRIVATE:
							break;
						case CHAT_MESSAGE_SYSTEM:
							{
								gClientEnv->serverResult = Message.message;
								CryLog("[CryMasterServer] System message recived [%s]", gClientEnv->serverResult);
								break;
							}
						default:
							break;
						}
					}

					break;
				}
			case PACKET_REQUEST:
				CryLog("[CryMasterServer] Request packet recived");
				break;
			case PACKET_MS_INFO:
				{
					CryLog("[CryMasterServer] Master server info packet recived");
					gClientEnv->serverInfo = gClientEnv->pRsp->ReadMasterServerInfo(Packet);

					if(gClientEnv->serverInfo.playersOnline)
					{
						SUIArguments args;
						args.AddArgument(gClientEnv->serverInfo.playersOnline);
						args.AddArgument(gClientEnv->serverInfo.gameServersOnline);
						CMsEvents::GetInstance()->SendEvent(CMsEvents::eUIGE_ServerInfoResived, args);
					}

					break;
				}
			case PACKET_GAME_SERVER:
				{
					CryLog("[CryMasterServer] Game server info packet recived");
					SUIArguments args;
					SGameServer server;
					server = gClientEnv->pRsp->ReadGameServerInfo(Packet);

					if(server.port)
					{
						args.AddArgument(server.ip);
						args.AddArgument(server.port);
						args.AddArgument(server.serverName);
						args.AddArgument("0/0");
						args.AddArgument((const char*)server.mapName);
						args.AddArgument((const char*)server.gameRules);
						args.AddArgument(0);

						CMsEvents::GetInstance()->SendEvent(CMsEvents::eUIGE_GameServerInfo, args);
					}

					break;
				}
			default:
				CryLog("[CryMasterServer] Unknown packet recived...");
				break;
			}

			ReadPackets.erase(it);
			packetsInReadQueue--;
		}		
		read_mutex.unlock();

		Sleep(1);
	}
}
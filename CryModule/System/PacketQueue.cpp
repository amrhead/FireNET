/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
-------------------------------------------------------------------------
History:

- 10.01.2015   21:34 : Created by AfroStalin(chenecoff)
- 15.06.2015   14:15  : Edited by AfroStalin(chernecoff)
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
	gEnv->pLog->Log(TITLE "CPacketQueue::Init()");

	std::thread Thread(&CPacketQueue::Thread, this);
	Thread.detach();
}

void CPacketQueue::InsertPacket(SPacket packet)
{
	send_mutex.lock();
	SendPackets.push_back(packet);
	packetsInSendQueue++;
	send_mutex.unlock();

	//gEnv->pLog->Log("[CryMasterServer] CPacketQueue::New packet added to send queue");
}

void CPacketQueue::InsertPacketToRead(SPacket packet)
{
	read_mutex.lock();
	ReadPackets.push_back(packet);
	packetsInReadQueue++;
	read_mutex.unlock();

	//gEnv->pLog->Log("[CryMasterServer] CPacketQueue::New packet added to read queue");
}

void CPacketQueue::Thread()
{
	gEnv->pLog->Log(TITLE "Send/Read packets thread started!");

	while(!gEnv->pSystem->IsQuitting())
	{	
		// ONLY FOR FREE VERSION. DELETE THIS IF YOU USE FULL LICIENSE
		if(gEnv && gEnv->pRenderer)
		{
			const float white[4] = {1.0f, 1.0f, 1.0f, 0.75f};
			char tmp[64];
			sprintf(tmp,"Powered by : FireNET v.%s", gClientEnv->clientVersion);
			gEnv->pRenderer->Draw2dLabel( 95, 1 , 1.3, white, true, tmp );
		}
		//

		send_mutex.lock();
		if(packetsInSendQueue>0)
		{
			//gEnv->pLog->Log("[CryMasterServer] Sending packet...");

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
			//gEnv->pLog->Log("[CryMasterServer] Reding packet...");

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
				gEnv->pLog->Log(TITLE "Identification packet recived");
				break;
			case PACKET_ACCOUNT:
				{
					gEnv->pLog->Log(TITLE "Account info packet recived");

					gClientEnv->masterPlayer = gClientEnv->pRsp->ReadAccountInfo(Packet);

					SPlayer Player;
					SUIArguments args;

					Player = gClientEnv->masterPlayer;

					if(Player.playerId != 0)
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
								gEnv->pLog->Log(TITLE "Global chat message recived");

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
								gEnv->pLog->Log(TITLE "System message recived [%s]", gClientEnv->serverResult.c_str());
								break;
							}
						default:
							break;
						}
					}

					break;
				}
			case PACKET_REQUEST:
				{
					gEnv->pLog->Log(TITLE "Request packet recived");
					SRequestPacket svRequest = gClientEnv->pRsp->ReadRequest(Packet);

					if(!strcmp(svRequest.request,"RemoveGameServer"))
					{
						SUIArguments args;
						args.AddArgument(svRequest.iParam);
						CMsEvents::GetInstance()->SendEvent(CMsEvents::eUIGE_RemoveServer, args);

						std::vector <SGameServer>::iterator it;
						bool find = false;

						// Delete game server
						for(it = gClientEnv->pMasterServer->vServers.begin(); it!= gClientEnv->pMasterServer->vServers.end(); ++it)
						{
							if(it->id == svRequest.iParam)
							{
								find = true;
								break;
							}
						}

						if(find) gClientEnv->pMasterServer->vServers.erase(it);
					}
					break;
				}
			case PACKET_MS_INFO:
				{
					gEnv->pLog->Log(TITLE "Master server info packet recived");
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
			case PACKET_GAME_SERVERS:
				{
					gEnv->pLog->Log( TITLE "Game servers packet recived");
					gClientEnv->pRsp->ReadGameServers(Packet);

					break;
				}
			case PACKET_CONSOLE_COMMAND:
				{
					gEnv->pLog->Log(TITLE "Console command packet recieved");
					gClientEnv->pRsp->ReadConsoleCommandPacket(Packet);
					break;
				}
			default:
				gEnv->pLog->Log( TITLE "Unknown packet recived...");
				break;
			}

			ReadPackets.erase(it);
			packetsInReadQueue--;
		}		
		read_mutex.unlock();

		CrySleep(1);
	}
}
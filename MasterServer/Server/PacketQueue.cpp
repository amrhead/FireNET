/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
--------------------------------------------------------------------------
History:

- 10.01.2015   21:34 : Created by AfroStalin(chenecoff)
- 22.05.2015   12:24 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#include "StdAfx.h"
#include <winsock.h>

#include "PacketQueue.h"

CPacketQueue::CPacketQueue()
{
	packetsInSendQueue = 0;
	packetsInReadQueue = 0;
}

void CPacketQueue::Init()
{
	Log(LOG_DEBUG,"CPacketQueue::Init()");

	std::thread SendThread(&CPacketQueue::SendThread, this);
	std::thread ReadThread(&CPacketQueue::ReadThread, this);

	SendThread.detach();
	ReadThread.detach();
}

void CPacketQueue::InsertPacketToSend(SPacket packet)
{
	send_mutex.lock();

	SendPackets.push_back(packet);
	packetsInSendQueue++;

	send_mutex.unlock();

	Log(LOG_DEBUG,"CPacketQueue::New packet added to send queue");
}

void CPacketQueue::InsertPacketToRead(SReadPacket packet)
{
	read_mutex.lock();

	ReadPackets.push_back(packet);
	packetsInReadQueue++;

	read_mutex.unlock();

	Log(LOG_DEBUG,"CPacketQueue::New packet added to Read queue");
}

int CPacketQueue::CheckSocket(SOCKET Socket)
{
	SOCKET ERROR_SOCKET = send (Socket,"",0,0);
	return (int)ERROR_SOCKET;
}

void CPacketQueue::SendThread()
{
	while(true)
	{	
		send_mutex.lock();

		if(packetsInSendQueue>0)
		{
			SPacket packet;
			std::vector <SPacket>::iterator it;

			it = SendPackets.begin();

			packet.addr = (*it).addr;
			packet.data = (*it).data;
			packet.size = (*it).size;

			if(CheckSocket(packet.addr) != -1)
				send(packet.addr,packet.data,packet.size,0);
			else
				Log(LOG_DEBUG,"CPacketQueue::SendThread::Dead socket!");

			SendPackets.erase(it);
			packetsInSendQueue--;	
		}

		send_mutex.unlock();

		Sleep(33);
	}
}

void CPacketQueue::ReadThread()
{
	while(true)
	{	
		read_mutex.lock();

		if(packetsInReadQueue>0)
		{
			SClient Client;
			SGameServer GameServer;
			SPacket Packet;
			SClient Player;

			std::vector <SReadPacket>::iterator it;

			it = ReadPackets.begin();

			Packet = (*it).packet;
			Client = (*it).client;
			GameServer = (*it).server;
		
			if(CheckSocket(Client.socket) != -1 || CheckSocket(GameServer.socket) != -1) // If socket aviable start reading
			{
				EPacketType packetType = gEnv->pRsp->GetPacketType(Packet);

				switch (packetType)
				{
				case PACKET_IDENTIFICATION:
					break;
				case PACKET_LOGIN:
					{
						Log(LOG_DEBUG,"Login packet recived");
						Log(LOG_INFO,"Client <%s:%s> trying logining...", Client.nickname.c_str(), Client.ip);

						SLoginPacket loginPacket = gEnv->pRsp->ReadLoginPacket(Packet);

						const char* result = gEnv->pXml->Login(loginPacket.login,loginPacket.password);

						if(!strcmp("PasswordCorrect",result))
						{
							Player = gEnv->pXml->GetUserInfo(loginPacket.login);


							// Block dual autorization
							bool blockDual = false;

							SERVER_LOCK
							for(auto it = gEnv->pServer->vClients.begin(); it != gEnv->pServer->vClients.end(); ++it)
							{
								if(it->playerId == Player.playerId)
								{
									bool tmp = !!atoi(gEnv->pSettings->GetConfigValue("Server","block_dual_players"));

									if(tmp)
									{
										Log(LOG_WARNING, "Block dual authorization from <%s, %s>", Client.nickname.c_str(), Client.ip);
										blockDual = true;

										SMessage Message;
										Message.area = CHAT_MESSAGE_SYSTEM;
										Message.message = "BlockDual";
										gEnv->pRsp->SendMsg(Client.socket,Message);	
									}
									break;
								}
							}
							SERVER_UNLOCK

							if(!blockDual)
							{
								gEnv->pRsp->SendAccountInfo(Client.socket,Player);

								gEnv->pServer->SendClientStatus(Player.nickname, CLIENT_CONNECTED);

								Log(LOG_INFO,"Client <%s:%s> has changed the status to <%s:%s>", Client.nickname.c_str(), Client.ip, Player.nickname.c_str(), Client.ip);


								SERVER_LOCK
								for( auto it = gEnv->pServer->vClients.begin(); it != gEnv->pServer->vClients.end(); ++it)
								{
									if(it->socket == Client.socket)
									{
										it->playerId    = Player.playerId;
										it->login       = loginPacket.login;
										it->nickname    = Player.nickname;	
										it->level       = Player.level;
										it->money       = Player.money;
										it->xp          = Player.xp;
										it->banStatus   = !!Player.banStatus;

										break;
									}
								}
								SERVER_UNLOCK
							}
						}

						SMessage Message;
						Message.area = CHAT_MESSAGE_SYSTEM;
						Message.message = result;
						gEnv->pRsp->SendMsg(Client.socket,Message);	



						break;
					}
				case PACKET_REGISTER:
					{
						Log(LOG_DEBUG,"Register packet recived");

						SLoginPacket loginPacket = gEnv->pRsp->ReadRegistrationPacket(Packet);
						const char* result = gEnv->pXml->Register(loginPacket.login, loginPacket.password, loginPacket.nickname);

						SMessage Message;
						Message.area = CHAT_MESSAGE_SYSTEM;
						Message.message = result;
						gEnv->pRsp->SendMsg(Client.socket,Message);
						break;
					}
				case PACKET_ACCOUNT:
					break;
				case PACKET_MESSAGE:
					{
						Log(LOG_DEBUG,"Message packet recived");
						SMessage clientMsg = gEnv->pRsp->ReadMsg(Packet);

						switch (clientMsg.area)
						{
						case CHAT_MESSAGE_GLOBAL:
							{
								char CompleteMsg[256];
								sprintf(CompleteMsg, "%s : %s", Client.nickname.c_str(), clientMsg.message);

								SERVER_LOCK
								for(auto conIT = gEnv->pServer->vClients.begin(); conIT != gEnv->pServer->vClients.end(); ++conIT)
								{
									SMessage Message;
									Message.area = CHAT_MESSAGE_GLOBAL;
									Message.message = CompleteMsg;

									gEnv->pRsp->SendMsg(conIT->socket,Message);
								}
								SERVER_UNLOCK

								break;
							}
						default:
							break;
						}


						break;
					}
				case PACKET_REQUEST:
					{
						Log(LOG_DEBUG,"Request packet recived");
						SRequestPacket clientRequest = gEnv->pRsp->ReadRequest(Packet);

						if(!strcmp(clientRequest.request,"GetServers"))
						{
							if(gEnv->allServers)
							{
								gEnv->pRsp->SendGameServers(Client.socket);
							}
						}

						if(!strcmp(clientRequest.request,"GetPlayer"))
						{
							Player = gEnv->pXml->GetUserInfo(Client.login);
							gEnv->pRsp->SendAccountInfo(Client.socket,Player);
						}

						if(!strcmp(clientRequest.request,"GetMasterInfo"))
						{
							SERVER_LOCK
							SMasterServerInfo info;
							info.playersOnline = (int)gEnv->pServer->vClients.size();
							info.gameServersOnline = (int)gEnv->pServer->vServers.size();
							SERVER_UNLOCK

							gEnv->pRsp->SendMasterServerInfo(Client.socket,info);
						}

						break;
					}
				case PACKET_MS_INFO:
					break;
				case PACKET_GAME_SERVER:
					{
						Log(LOG_DEBUG,"Game server info recived");
						SGameServer Server = gEnv->pRsp->ReadGameServerInfo(Packet);

						SERVER_LOCK
						for( auto it = gEnv->pServer->vServers.begin(); it != gEnv->pServer->vServers.end(); ++it)
						{
							if(it->socket == GameServer.socket)
							{
								(*it).ip = Server.ip;
								(*it).serverName = Server.serverName;
								(*it).mapName = Server.mapName;
								(*it).gameRules = Server.gameRules;
								(*it).port = Server.port;
								(*it).currentPlayers = Server.currentPlayers;
								(*it).maxPlayers = Server.maxPlayers;

								break;
							}
						}
						SERVER_UNLOCK

						if(strcmp(GameServer.serverName,Server.serverName))
						{
							Log(LOG_INFO,"Game server <%s:%s> has changed the status to <%s:%s>", GameServer.serverName, GameServer.ip, Server.serverName, Server.ip);

							Server.id = GameServer.id;

							SERVER_LOCK
							if(gEnv->pServer->vClients.size()>0)
							{
								for (auto conIT=gEnv->pServer->vClients.begin(); conIT!=gEnv->pServer->vClients.end(); ++conIT)
									gEnv->pRsp->SendGameServerInfo(conIT->socket,Server);
							}
							SERVER_UNLOCK
						}
						else
						{
							Server.id = GameServer.id;
							SERVER_LOCK
							if(gEnv->pServer->vClients.size()>0)
							{
								for (auto conIT=gEnv->pServer->vClients.begin(); conIT!=gEnv->pServer->vClients.end(); ++conIT)
									gEnv->pRsp->SendGameServerInfo(conIT->socket,Server);
							}
							SERVER_UNLOCK
						}

						


						break;
					}
				default:
					break;
				}
			}
			else
				Log(LOG_DEBUG,"CPacketQueue::ReadThread::Dead socket!");

			ReadPackets.erase(it);
			packetsInReadQueue--;	
		}
		read_mutex.unlock();

		Sleep(1);
	}
}


/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014-2015
-------------------------------------------------------------------------

-------------------------------------------------------------------------
History:

- 10.01.2015   21:34 : Created by AfroStalin(chenecoff)
- 15.03.2015   22:57 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#include "Global.h"

#include "PacketQueue.h"

CPacketQueue::CPacketQueue()
{
	packetsInSendQueue = 0;
	packetsInReadQueue = 0;
}

void CPacketQueue::Init()
{
	printf("CPacketQueue::Init()\n");

	std::thread firstSendThread(&CPacketQueue::SendThread, this);
	std::thread secondSendThread(&CPacketQueue::SendThread, this);
	std::thread thirdSendThread(&CPacketQueue::SendThread, this);
	std::thread fourthSendThread(&CPacketQueue::SendThread, this);
	std::thread fifthSendThread(&CPacketQueue::SendThread, this);

	std::thread ReadThread(&CPacketQueue::ReadThread, this);
	ReadThread.detach();


	firstSendThread.detach();
	secondSendThread.detach();
	thirdSendThread.detach();
	fourthSendThread.detach();
	fifthSendThread.detach();
}

void CPacketQueue::InsertPacket(SPacket packet)
{
	send_mutex.lock();

	SendPackets.push_back(packet);
	packetsInSendQueue++;

	send_mutex.unlock();

	printf("CPacketQueue::New packet added to send queue\n");
}

void CPacketQueue::InsertPacketToRead(SPacket packet)
{
	read_mutex.lock();

	ReadPackets.push_back(packet);
	packetsInReadQueue++;

	read_mutex.unlock();

	printf("CPacketQueue::New packet added to Read queue\n");
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

			//Log(LOG_DEBUG,"CPacketQueue::SendThread::Packet in send queue = %d", packetsInSendQueue);

			it = SendPackets.begin();

			packet.addr = (*it).addr;
			packet.data = (*it).data;
			packet.size = (*it).size;

			if(CheckSocket(packet.addr) != -1)
				send(packet.addr,packet.data,packet.size,0);

			SendPackets.erase(it);
			packetsInSendQueue--;

			//Log(LOG_DEBUG,"CPacketQueue::SendThread::Packet is sent!");		
		}

		send_mutex.unlock();

		Sleep(1);
	}
}

void CPacketQueue::ReadThread()
{
	while(true)
	{	
		read_mutex.lock();

		if(packetsInReadQueue>0)
		{
			SPacket Packet;
			SPlayer Player;

			std::vector <SPacket>::iterator it;

			it = ReadPackets.begin();

			Packet.addr =  (*it).addr;
			Packet.data = (*it).data;
			Packet.size = (*it).size;

			int packetType = gClientEnv->pRsp->GetPacketType(Packet);

			switch (packetType)
			{

			case 4: // Message
				{
					SMessage Message = gClientEnv->pRsp->ReadMsg(Packet);

					printf("Global chat message = %s\n", Message.message);
					break;
				}

			case 8: // Request
				{
					break;
				}

			default:
				break;
			}

			ReadPackets.erase(it);
			packetsInReadQueue--;
		}		

		read_mutex.unlock();
	}

	Sleep(1);
}


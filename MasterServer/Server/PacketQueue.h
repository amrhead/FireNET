/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
--------------------------------------------------------------------------
History:

- 10.01.2015   21:34 : Created by AfroStalin(chernecoff)
- 13.03.2015   15:43 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#ifndef _PacketQueue_
#define _PacketQueue_

//#include <winsock.h>
typedef UINT_PTR SOCKET;
#include "TcpServer.h"

struct SReadPacket
{
	SClient client;
	SGameServer server;
	SPacket packet;
};

class CPacketQueue
{
public:
	CPacketQueue();
	~CPacketQueue(void){}

	void Init();
	// Insert packet to send queue
	void InsertPacketToSend(SPacket packet);
	// Inser packet to read queue
	void InsertPacketToRead(SReadPacket packet);

private:
	void SendThread();
	void ReadThread();
	int CheckSocket(SOCKET Socket);

private:
	std::mutex send_mutex;

	int packetsInSendQueue;
	std::vector <SPacket> SendPackets;
	
private:
	std::mutex read_mutex;

	int packetsInReadQueue;
	std::vector <SReadPacket> ReadPackets;

};
#endif
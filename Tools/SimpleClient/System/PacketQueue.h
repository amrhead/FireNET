/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014-2015
-------------------------------------------------------------------------

-------------------------------------------------------------------------
History:

- 10.01.2015   21:34 : Created by AfroStalin(chernecoff)
- 13.03.2015   15:43 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#ifndef _PacketQueue_
#define _PacketQueue_

#include <winsock.h>

class CPacketQueue
{
public:
	CPacketQueue();
	~CPacketQueue(void){}

	void Init();
	// Insert packet to send queue
	void InsertPacket(SPacket packet);
	// Inser packet to read queue
	void InsertPacketToRead(SPacket packet);

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
	std::vector <SPacket> ReadPackets;

};
#endif
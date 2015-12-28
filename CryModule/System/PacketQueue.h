/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
-------------------------------------------------------------------------
History:

- 10.01.2015   21:34 : Created by AfroStalin(chernecoff)
- 23.03.2015   15:50 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#ifndef _PacketQueue_
#define _PacketQueue_

#include <mutex>
typedef UINT_PTR SOCKET;

class CPacketQueue
{
public:
	CPacketQueue();
	~CPacketQueue(){}

	void Init();
	// Insert packet to send queue
	void InsertPacket(SPacket packet);
	// Inser packet to read queue
	void InsertPacketToRead(SPacket packet);

private:
	void Thread();
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
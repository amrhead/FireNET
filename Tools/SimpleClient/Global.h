/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2015
-------------------------------------------------------------------------
-------------------------------------------------------------------------
History:

- 13.03.2015   15:36 : Created by AfroStalin(chernecoff)
- 20.06.2015   17:47 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/
#include "windows.h"
#include "winsock.h"
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <iostream>

#include "Packets\RSP.h"
#include "System\PacketQueue.h"
#include "../../versions.h"

struct SGlobalClient
{
	CPacketQueue* pPacketQueue;
	CReadSendPacket* pRsp;

	std::string clientVersion;

	bool bDebugMode;
	bool bBlowFish;

	inline void Init()
	{
		clientVersion = PACKET_VERSION;

		bDebugMode = false;
		bBlowFish  = false;

		pPacketQueue = new CPacketQueue;
		pRsp         = new CReadSendPacket;
	}
};

extern SGlobalClient* gClientEnv;
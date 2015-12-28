/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2015
-------------------------------------------------------------------------
-------------------------------------------------------------------------
History:

- 13.03.2015   15:36 : Created by AfroStalin(chernecoff)
- 30.04.2015   22:56 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/
#ifndef _CryMasterGlobal_
#define _CryMasterGlobal_

#include "../Packets/RSP.h"
#include "../System/PacketQueue.h"
#include "../CryMasterServer.h"
#include "../Nodes/MsEvents.h"

struct SGlobalClient
{
	CPacketQueue* pPacketQueue;
	CReadSendPacket* pRsp;
	CCryMasterServer* pMasterServer;

	SPlayer masterPlayer;
	SMasterServerInfo serverInfo;
	SGameServer gameServer;

	const char* clientVersion;
	string serverResult;

	bool bDebugMode;
	bool bBlowFish;

	inline void Init()
	{
		clientVersion = "0.0.8a";
		serverResult = "";

		bDebugMode = false;
		bBlowFish  = false;

		pMasterServer = new CCryMasterServer;
		pPacketQueue = new CPacketQueue;
		pRsp         = new CReadSendPacket;

		//
		masterPlayer.playerId = 0;
		masterPlayer.nickname = "Unknown";
		masterPlayer.level = 0;
		masterPlayer.money = 0;
		masterPlayer.xp = 0;
		masterPlayer.banStatus = false;
		//
		serverInfo.playersOnline = 0;
		serverInfo.gameServersOnline = 0;
		//
		gameServer.currentPlayers = 0;
		gameServer.gameRules = "";
		gameServer.ip = "";
		gameServer.mapName = "";
		gameServer.maxPlayers = 0;
		gameServer.port = 0;
		gameServer.serverName = 0;
	}
};

extern SGlobalClient* gClientEnv;

#endif
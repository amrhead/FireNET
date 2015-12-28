/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
-------------------------------------------------------------------------
History:

- 13.03.2015   15:36 : Created by AfroStalin(chernecoff)
- 20.06.2015   12:15  : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/
#ifndef _CryMasterGlobal_
#define _CryMasterGlobal_

#include "../Packets/RSP.h"
#include "../System/PacketQueue.h"
#include "../MasterServer.h"
#include "../Nodes/MsEvents.h"
#include "../Tools/Ping.h"
#include "../versions.h"

#define TITLE "[FireNET] "


struct SGlobalClient
{
	CPacketQueue* pPacketQueue;
	CReadSendPacket* pRsp;
	CMasterServer* pMasterServer;
	CPing* pPing;

	SPlayer masterPlayer;
	SMasterServerInfo serverInfo;
	SGameServer gameServer;

	const char* clientVersion;
	string serverResult;

	bool bDebugMode;
	bool bBlowFish;
	bool bConnected;
	bool bConsoleHelperConnected;

	inline void Init()
	{
		gEnv->pLog->Log(TITLE "Global environment Init()");

		clientVersion = PACKET_VERSION;
		serverResult = "";

		bDebugMode = false;
		bBlowFish  = false;
		bConnected = false;
		bConsoleHelperConnected = false;

		pMasterServer = new CMasterServer;
		pPacketQueue  = new CPacketQueue;
		pRsp          = new CReadSendPacket;
		pPing         = new CPing;

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
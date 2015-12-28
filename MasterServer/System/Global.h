/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
--------------------------------------------------------------------------
History:

- 20.08.2014   23:19 : Created by AfroStalin(chernecoff)
- 13.03.2015   14:15 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------


*************************************************************************/

#ifndef _Global_
#define _Global_

// Server
#include "Server\PacketQueue.h"
#include "Server\TcpServer.h"

// Databases
#include "MySql\MySql.h"
#include "Xml\XmlDatabase.h"

// System
#include "ConsoleCommands.h"
#include "Settings.h"
#include "AppLog.h"

// Packets
#include "Packets\RSP.h"

struct SGlobal
{
	// Class pointers
	CConsoleCommands* pConsole;
	CTcpServer* pServer;
	CXmlDatabase* pXml;
	CSettings* pSettings;
	CMySql* pMySql;
	CReadSendPacket* pRsp;
	CPacketQueue* pPacketQueue;
	CAppLog* pLog;

	// Server variables
	const char* serverVersion;
	int maxPlayers;
	int maxGameServers;

	// Statistic variables
	int startTime;
	int aClients;
	int rClients;
	int sqlCounter;
	int inPackets;
	int outPackets;
	int allPlayers;
	int allServers;

	// Booleans
	bool bUseXml;
	bool bDebugMode;
	bool bBlowFish;

	// Windows handles
	HWND logBox;
	HWND consoleBox;
	HWND statusBox;

	inline void Init()
	{
		startTime = 0;
		aClients = 0;
		rClients = 0;
		sqlCounter = 0;
		inPackets = 0;
		outPackets = 0;
		allPlayers = 0;
		allServers = 0;

		bUseXml = false;

#if defined RELEASE
		bDebugMode = false;
#endif
#if defined DEBUG
		bDebugMode = true;
#endif
		bBlowFish = false;

		pServer      = new CTcpServer;
		pPacketQueue = new CPacketQueue;
		pLog         = new CAppLog;
		pConsole     = new CConsoleCommands;
		pXml         = new CXmlDatabase;
		pSettings    = new CSettings;
		pMySql       = new CMySql;
		pRsp         = new CReadSendPacket;
	}
};

extern struct SGlobal* gEnv;

#define SERVER_LOCK gEnv->pServer->mutex.lock();
#define SERVER_UNLOCK gEnv->pServer->mutex.unlock();

#endif
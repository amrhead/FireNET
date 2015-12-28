/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
---------------------------------------------------------------------------
History:

- 29.08.2014   20:02 : Created by AfroStalin(chernecoff)
- 22.05.2015   12:24 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------


*************************************************************************/

#include "StdAfx.h"

#include "ConsoleCommands.h"
#include "Packets\RSP.h"

#include "time.h"

void CConsoleCommands::Read(const char* command)
{
	if(!strcmp(command,"list")) List();
	if(!strcmp(command,"status")) Status();
	
	if(!strcmp(command,"debug"))
	{
		if(!gEnv->bDebugMode)
		{
			Log(LOG_WARNING,"********************Debug mode********************");
			gEnv->bDebugMode = true;
		}
		else
		{
			Log(LOG_WARNING,"Debug mode disabled.");
			gEnv->bDebugMode = false;
		}
	}

	if(!strcmp(command,"players")) ShowPlayers();
	if(!strcmp(command,"servers")) ShowServers(); 
	if(!strcmp(command,"clear")) SetWindowText(gEnv->logBox, ""); 
	if(!strcmp(command,"exit")) Quit();
	if(!strcmp(command, "test")) Test(); 

	//Log(LOG_WARNING,"Console command <%s> not found!", command);
}

void CConsoleCommands::List()
{
	Log(LOG_WARNING,"********************Command list********************");
	Log(LOG_INFO,"list    -    use 'list' to show all console commands...");
	Log(LOG_INFO,"status  -    use 'status' to show server status...");
	Log(LOG_INFO,"players -    use 'players' to show all connected players...");
	Log(LOG_INFO,"servers -    use 'servers' to show all register game servers...");
	Log(LOG_INFO,"debug   -    use 'debug' to enable or disable debug mode...");
	Log(LOG_INFO,"clear   -    use 'clear' to clear screen...");
	Log(LOG_INFO,"exit    -    use 'exit' to close master server...");
	Log(LOG_WARNING,"****************************************************");
}

void CConsoleCommands::Status()
{
	Log(LOG_WARNING,"********************Server status********************");
	Log(LOG_INFO,"Server working time : %d min.", (clock()-(gEnv->startTime))/60000);
	Log(LOG_INFO,"Number of connected clients : %d", gEnv->allPlayers);
	Log(LOG_INFO,"Number of authorized clients : %d", gEnv->aClients);
	Log(LOG_INFO,"Number of registered clients : %d", gEnv->rClients);
	Log(LOG_INFO,"Number of sql queries : %d", gEnv->sqlCounter);
	Log(LOG_INFO,"Number of all incoming packets : %d", gEnv->inPackets);
	Log(LOG_INFO,"Number of all send packets : %d", gEnv->outPackets);
	Log(LOG_INFO,"Number connected game server : %d",gEnv->allServers);
	Log(LOG_WARNING,"******************************************************");
}


void CConsoleCommands::ShowPlayers()
{
	gEnv->pServer->mutex.lock();

	if(gEnv->pServer->vClients.size()>0)
	{
		Log(LOG_INFO,"-----------------Player list-------------------");
		//
		for (auto it = gEnv->pServer->vClients.begin(); it != gEnv->pServer->vClients.end(); ++it)
		{
			Log(LOG_INFO,"Player '%s' , ip '%s', id '%d'" , (*it).nickname, (*it).ip, (*it).playerId);
		}
		//
		Log(LOG_INFO,"-----------------------------------------------");
	}
	else
		Log(LOG_WARNING,"No connected players!");

	gEnv->pServer->mutex.unlock();
}

void CConsoleCommands::ShowServers()
{
	gEnv->pServer->mutex.lock();

	if(gEnv->pServer->vServers.size()>0)
	{
		Log(LOG_INFO,"-----------------Server list-------------------");
		//
		for (auto it = gEnv->pServer->vServers.begin(); it != gEnv->pServer->vServers.end(); ++it)
			Log(LOG_INFO,"Server '%s' , Ip '%s' , Port '%d', id = '%d' , Online = '%d'", (*it).serverName, (*it).ip, (*it).port, (*it).id, (*it).currentPlayers);
		//
		Log(LOG_INFO,"-----------------------------------------------");
	}
	else
		Log(LOG_WARNING,"No connected game servers!");

	gEnv->pServer->mutex.unlock();
}

void CConsoleCommands::Quit()
{
	Log(LOG_INFO,"Master Server shutdown...");
	exit(1);
}

void CConsoleCommands::Test()
{
	Log(LOG_INFO, "Test sending all game servers");

	for (auto it=gEnv->pServer->vClients.begin(); it!=gEnv->pServer->vClients.end(); ++it)
			gEnv->pRsp->SendGameServers(it->socket);	
}
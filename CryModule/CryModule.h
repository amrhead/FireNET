/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
-------------------------------------------------------------------------
History:

- 15.05.2015   10:08 : Created by AfroStalin(chernecoff)
- 20.06.2015   12:15  : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#ifndef _FIRENET_CRYMODULE_H_
#define _FIRENET_CRYMODULE_H_
#define FIRENET_API extern "C" __declspec(dllexport)
#ifndef SAFESTR
#define SAFESTR(x) (((const char*)x)?((const char*)x):"")
#endif

#include "StdAfx.h"
#include "Nodes/G2FlowBaseNode.h"

#include "System/Global.h"

// CVars
char* fn_master_server_ip = "127.0.0.1";
char* fn_master_server_securityKey = "PLEASE_CHANGE_YOU_MASTER_SERVER_SUPER_STRONG_ENCRYPTION_KEY_HERE";
int fn_master_server_port = 64087;


void ConnectToMasterServer(IConsoleCmdArgs* pArgs){gClientEnv->pMasterServer->Connect();}
void DisconnectFromMasterServer(IConsoleCmdArgs* pArg){gClientEnv->pMasterServer->Disconnect();}

void RegisterCommands()
{
	gEnv->pConsole->AddCommand("fn_ms_connect", ConnectToMasterServer, VF_NULL,"Connect to FireNET - Master server");
	gEnv->pConsole->AddCommand("fn_ms_disconnect", DisconnectFromMasterServer, VF_NULL,"Disconnect from FireNET - Master server");
}

void RegisterCVars()
{
	gEnv->pConsole->RegisterString("fn_master_server_ip", fn_master_server_ip, VF_NULL, "FireNet master server ip addres");
	gEnv->pConsole->RegisterString("fn_master_server_securityKey", fn_master_server_securityKey, VF_NULL, "FireNet maser server security key");
	gEnv->pConsole->RegisterInt("fn_master_server_port",fn_master_server_port,VF_NULL,"FireNet master server port");
}

FIRENET_API void UpdateGameServerInfo(int clientsNumber)
{
	gClientEnv->gameServer.currentPlayers = clientsNumber;
	gClientEnv->pMasterServer->SendGameServerInfo();
}

FIRENET_API void RegisterFlowNodes()
{
	if (IFlowSystem* pFlow= gEnv->pGame->GetIGameFramework()->GetIFlowSystem())
	{
		for ( CG2AutoRegFlowNodeBase* pFactory = CG2AutoRegFlowNodeBase::m_pFirst; pFactory; pFactory = pFactory->m_pNext )
		{
			TFlowNodeTypeId nTypeId = pFlow->RegisterType( pFactory->m_sClassName, pFactory );

			if ( nTypeId != InvalidFlowNodeTypeId )
			{
				gEnv->pLog->Log( TITLE "Flownode class(%s) registered!", SAFESTR( pFactory->m_sClassName ));
			}

			else
			{
				gEnv->pLog->LogError( TITLE "Flownode class(%s) couldn't register!", SAFESTR( pFactory->m_sClassName ) );
			}
		}
	}
	else
	{
		gEnv->pLog->LogError( TITLE "Error register flow nodes!" );
	}
}

FIRENET_API void InitModule(SSystemGlobalEnvironment& gCryEnv)
{
	gEnv = &gCryEnv;

	RegisterCommands();
	RegisterCVars();

	CMsEvents::GetInstance()->Init();
	gClientEnv->Init();

	if(!gEnv->IsEditor())
	{
		gClientEnv->pPacketQueue->Init();
		gClientEnv->pPing->Init();
	}
}

#endif
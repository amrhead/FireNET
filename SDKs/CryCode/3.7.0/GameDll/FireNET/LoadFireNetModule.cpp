/*************************************************************************
Copyright (C), chernecoff@gmail.com , 2015
-------------------------------------------------------------------------
History:

- 15.05.2015   11:28 : Created by AfroStalin(chernecoff)
- 16.05.2015   01:16 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#include "StdAfx.h"
#include "CryLibrary.h"
#include "LoadFireNetModule.h"

CLoadFireNET* pFireNET = new CLoadFireNET;

CLoadFireNET::CLoadFireNET()
{
	hndl = NULL; 
	pInitModule = NULL; 
	pRegisterFlowNodes = NULL; 
	pUpdateGameServerInfo = NULL;
}

CLoadFireNET::~CLoadFireNET()
{
	delete pFireNET;
}

void CLoadFireNET::LoadModule()
{
	CryLog("[" FNM_NAME "] - Start loading module");

	hndl = CryLoadLibrary( FNM_NAME CrySharedLibrayExtension );

	if(hndl)
	{
		pInitModule = (void (*)(SSystemGlobalEnvironment&)) CryGetProcAddress( hndl, "InitModule" );
		pRegisterFlowNodes = (void(*)(void)) CryGetProcAddress( hndl,"RegisterFlowNodes");
		pUpdateGameServerInfo = (void(*)(int)) CryGetProcAddress( hndl,"UpdateGameServerInfo");

		if(pInitModule != NULL && pRegisterFlowNodes != NULL && pUpdateGameServerInfo != NULL)
		{ 
			pInitModule(*gEnv);
			CryLog(FNM_LOADED);
		}
		else
		{
			hndl = NULL;
			CryLog(FNM_ERROR " - POINTERS ERROR");
		}
	}
	else
	{
		FreeDll();
		hndl = NULL;
		CryLog(FNM_ERROR " - ERROR LOADING DLL");
	}
}

void CLoadFireNET::FreeDll()
{
	if(hndl)
		CryFreeLibrary(hndl);
}
/*************************************************************************
Copyright (C), chernecoff@gmail.com , 2015
-------------------------------------------------------------------------
History:

- 15.05.2015   11:28 : Created by AfroStalin(chernecoff)
- 07.05.2015   00:06 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/
#ifndef _LoadFireNetModule_
#define _LoadFireNetModule_
#ifndef _WINDEF_
class HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;
#endif

#include "StdAfx.h"

#if defined(DEDICATED_SERVER)
#define FNM_NAME "FireNET_Dedicated"
#else
#define FNM_NAME "FireNET"
#endif

#define FNM_LOADED "[" FNM_NAME "] " "FireNet module loaded!"
#define FNM_ERROR "[" FNM_NAME "] " "Error loading FireNet module"

class CLoadFireNET
{
public:
	CLoadFireNET();
	~CLoadFireNET();

public:
	void LoadModule();
	void FreeDll();

public:
	HINSTANCE hndl;
	void (*pInitModule)(SSystemGlobalEnvironment&);
	void (*pRegisterFlowNodes)(void);
	void (*pUpdateGameServerInfo)(int);
};

extern CLoadFireNET* pFireNET;

#endif
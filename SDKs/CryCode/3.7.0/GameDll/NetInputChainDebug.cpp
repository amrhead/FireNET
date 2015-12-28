/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2004.
-------------------------------------------------------------------------

Description: Utilities for debugging input synchronization problems

-------------------------------------------------------------------------
History:
-	30:03:2007  : Created by Craig TillerNetInputChainPrint( const char * 

*************************************************************************/

#include "StdAfx.h"
#include "NetInputChainDebug.h"
#include "IEntitySystem.h"
#include "ITextModeConsole.h"

#if ENABLE_NETINPUTCHAINDEBUG
#include <BoostHelpers.h>
#include "CryWindows.h"


EntityId _netinputchain_debugentity = 0;
static int lastFrame = -100;
static int ypos = 0;
static int dump = 0;
static uint64 tstamp;

typedef boost::mpl::vector<float, Vec3> TNetInputValueTypes;
typedef boost::make_variant_over<TNetInputValueTypes>::type TNetInputValue;

static const char * GetEntityName()
{
	IEntity * pEnt = gEnv->pEntitySystem->GetEntity(_netinputchain_debugentity);
	assert(pEnt);
	if (pEnt)
		return pEnt->GetName();
	else
		return "<<unknown>>";
}

static void Put( const char * name, const TNetInputValue& value )
{
	FILE * fout = 0;
	if (dump) fout = fopen("netinput.log", "at");
#if defined(WIN32)
	FILETIME tm;
	GetSystemTimeAsFileTime(&tm);
#endif
	ITextModeConsole * pTMC = gEnv->pSystem->GetITextModeConsole();

	if (lastFrame != gEnv->pRenderer->GetFrameID())
	{
		ypos = 0;
		lastFrame = gEnv->pRenderer->GetFrameID();
#if defined(WIN32)
		tstamp = (uint64(tm.dwHighDateTime)<<32) | tm.dwLowDateTime;
#endif
}
	float white[] = {1,1,1,1};
	char buf[256];

	if (const Vec3 * pVec = boost::get<const Vec3>(&value))
	{
		sprintf(buf, "%s: %f %f %f", name, pVec->x, pVec->y, pVec->z);
		gEnv->pRenderer->Draw2dLabel(10.f, (float)(ypos+=20), 2.f, white, false, "%s", buf);
		if (pTMC)
			pTMC->PutText( 0, ypos/20, buf );
#ifndef _RELEASE
		if (fout) fprintf(fout, "%" PRIu64 " %s %s %f %f %f\n", tstamp, GetEntityName(), name, pVec->x, pVec->y, pVec->z);
#endif
	}
	else if (const float * pFloat = boost::get<const float>(&value))
	{
		sprintf(buf, "%s: %f", name, *pFloat);
		gEnv->pRenderer->Draw2dLabel(10.f, (float)(ypos+=20), 2, white, false, "%s", buf);
		if (pTMC)
			pTMC->PutText( 0, ypos/20, buf );
#ifndef _RELEASE
		if (fout) fprintf(fout, "%" PRIu64 " %s %s %f\n", tstamp, GetEntityName(), name, *pFloat);
#endif
	}
	if (fout)
		fclose(fout);
}

static void OnChangeEntity( ICVar * pVar )
{
	const char * entName = pVar->GetString();
	if (IEntity * pEnt = gEnv->pEntitySystem->FindEntityByName(entName))
		_netinputchain_debugentity = pEnt->GetId();
	else
		_netinputchain_debugentity = 0;
}

void NetInputChainPrint( const char * name, const Vec3& val )
{
	Put(name, TNetInputValue(val));
}

void NetInputChainPrint( const char * name, float val )
{
	Put(name, TNetInputValue(val));
}

void NetInputChainInitCVars()
{
	REGISTER_STRING_CB("net_input_trace", "0", VF_CHEAT, "trace an entities input processing to assist finding sync errors", OnChangeEntity);

	REGISTER_CVAR2("net_input_dump", &dump, 0, VF_CHEAT, "write net_input_trace output to a file (netinput.log)");
}

#else

void NetInputChainInitCVars(){}

#endif

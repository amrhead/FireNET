////////////////////////////////////////////////////////////////////////////
//
//  CryEngine Source File.
//  Copyright (C), Crytek.
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "PlayerProfileImplOrbis.h"

#if defined(ORBIS)

#include <IXml.h>
#include <StringUtils.h>

//------------------------------------------------------------------------
void CPlayerProfileImplOrbis::Release()
{
	delete this;
}

//------------------------------------------------------------------------
void CPlayerProfileImplOrbis::GetMemoryStatistics(ICrySizer * s)
{
	s->Add(*this);
}

//------------------------------------------------------------------------
void CPlayerProfileImplOrbis::InternalMakeFSPath(SUserEntry* pEntry, const char* profileName, string& outPath)
{
	if (pEntry)
	{
		outPath = ("%USER%/Profiles/");
	}
	else
	{
		outPath = PathUtil::GetGameFolder();
		outPath.append("/Libs/Config/Profiles/");

		if (profileName && *profileName)
		{
			outPath.append(profileName);
			outPath.append("/");
		}
	}
}

//------------------------------------------------------------------------
void CPlayerProfileImplOrbis::InternalMakeFSSaveGamePath(SUserEntry* pEntry, const char* profileName, string& outPath, bool bNeedFolder)
{
	assert (pEntry != 0);
	outPath = "%USER%/SaveGames/";
}

#endif
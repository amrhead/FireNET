////////////////////////////////////////////////////////////////////////////
//
//  CryEngine Source File.
//  Copyright (C), Crytek.
//
////////////////////////////////////////////////////////////////////////////

#if defined(ORBIS)

#pragma once

#include "PlayerProfileImplConsole.h"

class CPlayerProfileImplOrbis : public CPlayerProfileImplConsole
{

private:
	// CPlayerProfileImplConsole
	virtual void Release();
	virtual void GetMemoryStatistics(ICrySizer * s);
	// ~CPlayerProfileImplConsole

	// ICommonProfileImpl
	virtual void InternalMakeFSPath(SUserEntry* pEntry, const char* profileName, string& outPath);
	virtual void InternalMakeFSSaveGamePath(SUserEntry* pEntry, const char* profileName, string& outPath, bool bNeedFolder);
	// ~ICommonProfileImpl
};
#endif

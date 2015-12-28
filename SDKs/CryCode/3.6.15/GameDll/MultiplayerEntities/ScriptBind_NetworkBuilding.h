/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014.

-------------------------------------------------------------------------
History:

- 03.12.2014   13:49 : Created by AfroStalin(chernecoff)
- 07.12.2014   22:35 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#ifndef _ScriptBind_NetworkBuilding_h_
#define _ScriptBind_NetworkBuilding_h_

#include <IScriptSystem.h>
#include <ScriptHelpers.h>

#include "NetworkBuilding.h"


class CNetworkBuilding;

class CScriptBind_NetworkBuilding : public CScriptableBase
{
private:
	typedef std::map<EntityId, CNetworkBuilding*> TNetworkBuildingMap;

public:
	CScriptBind_NetworkBuilding(ISystem *pSystem, IGameFramework *pGameFramework);
	virtual ~CScriptBind_NetworkBuilding();

	virtual void GetMemoryUsage(ICrySizer *pSizer) const;
	CNetworkBuilding *GetNetworkBuilding(IFunctionHandler *pH);

	void AttachTo(CNetworkBuilding *pInteractiveObject);
	void Detach(EntityId entityId);

	int CanUse(IFunctionHandler *pH, ScriptHandle userId);
	int StartBuild(IFunctionHandler *pH);

private:
	void RegisterMethods();

	ISystem					*m_pSystem;
	IGameFramework	*m_pGameFrameWork;
	TNetworkBuildingMap m_NetworkBuildingMap;
};

#endif
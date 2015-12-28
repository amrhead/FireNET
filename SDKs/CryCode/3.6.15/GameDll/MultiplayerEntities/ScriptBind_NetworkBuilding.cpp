/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014.

-------------------------------------------------------------------------
History:

- 03.12.2014   13:49 : Created by AfroStalin(chernecoff)
- 07.12.2014   22:35  : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#include "StdAfx.h"
#include "ScriptBind_NetworkBuilding.h"


CScriptBind_NetworkBuilding::CScriptBind_NetworkBuilding( ISystem *pSystem, IGameFramework *pGameFramework )
: m_pSystem(pSystem)
, m_pGameFrameWork(pGameFramework)
{
	Init(pSystem->GetIScriptSystem(), m_pSystem, 1);

	RegisterMethods();
}

CScriptBind_NetworkBuilding::~CScriptBind_NetworkBuilding()
{

}

void CScriptBind_NetworkBuilding::RegisterMethods()
{
#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CScriptBind_NetworkBuilding::

	SCRIPT_REG_TEMPLFUNC(CanUse, "userId");
	SCRIPT_REG_TEMPLFUNC(StartBuild, "");
}

void CScriptBind_NetworkBuilding::AttachTo( CNetworkBuilding *pInteractiveObject )
{
	IScriptTable *pScriptTable = pInteractiveObject->GetEntity()->GetScriptTable();

	if (pScriptTable)
	{
		SmartScriptTable thisTable(m_pSS);

		thisTable->SetValue("__this", ScriptHandle(pInteractiveObject->GetEntityId()));
		thisTable->Delegate(GetMethodsTable());

		pScriptTable->SetValue("networkBuilding", thisTable);
	}
	m_NetworkBuildingMap.insert(TNetworkBuildingMap::value_type(pInteractiveObject->GetEntityId(), pInteractiveObject));
}

void CScriptBind_NetworkBuilding::Detach( EntityId entityId )
{
	m_NetworkBuildingMap.erase(entityId);
}

CNetworkBuilding * CScriptBind_NetworkBuilding::GetNetworkBuilding( IFunctionHandler *pH )
{
	void* pThis = pH->GetThis();

	if (pThis)
	{
		const EntityId objectId = (EntityId)(UINT_PTR)pThis;
		TNetworkBuildingMap::const_iterator cit = m_NetworkBuildingMap.find(objectId);
		if (cit != m_NetworkBuildingMap.end())
		{
			return cit->second;
		}
	}

	return NULL;
}

int CScriptBind_NetworkBuilding::CanUse( IFunctionHandler *pH, ScriptHandle userId )
{
	CNetworkBuilding *pInteractiveObject = GetNetworkBuilding(pH);
	if (pInteractiveObject)
	{
		return pH->EndFunction(pInteractiveObject->CanUse((EntityId)userId.n));
	}
	
	return pH->EndFunction();
}

int CScriptBind_NetworkBuilding::StartBuild(IFunctionHandler *pH)
{
	CNetworkBuilding *pInteractiveObject = GetNetworkBuilding(pH);
	if (pInteractiveObject)
	{
		pInteractiveObject->StartBuild();
	}

	return pH->EndFunction();
}

void CScriptBind_NetworkBuilding::GetMemoryUsage(ICrySizer *pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));	
}
/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014.

-------------------------------------------------------------------------
History:

- 03.12.2014   13:49 : Created by AfroStalin(chernecoff)
- 10.12.2014   21:08 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#include "StdAfx.h"
#include "NetworkBuilding.h"
#include "ScriptBind_NetworkBuilding.h"
#include "WorldState/WorldState.h"

CNetworkBuilding::CNetworkBuilding()
	: m_state(eState_NotUsed)
{
}

CNetworkBuilding::~CNetworkBuilding()
{
	g_pGame->GetNetworkBuildingScriptBind()->Detach(GetEntityId());
}

bool  CNetworkBuilding::Init( IGameObject * pGameObject )
{
	SetGameObject(pGameObject);
	defMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("Materials/special/not_built_building.mtl");

	if (!pGameObject->BindToNetwork())
		return false;

	if (!Reset())
		return false;

	g_pGame->GetNetworkBuildingScriptBind()->AttachTo(this);

	
	
	if(gEnv->bServer)
	{
		ws=CWorldState::GetInstance();
		ReadWorldState();
	}

	GetEntity()->Activate(true);
	return true;
}

void CNetworkBuilding::PostInit( IGameObject * pGameObject )
{
	pGameObject->EnableUpdateSlot(this, 0);
}

bool CNetworkBuilding::GetSettings()
{
	SmartScriptTable entityProperties;
	IScriptTable* pScriptTable = GetEntity()->GetScriptTable();
	if(!pScriptTable || !pScriptTable->GetValue("Properties", entityProperties))
	{
		CryLog("[Network Building] : Error read lua properties !");
		return false;
	}

	//Physics properties
	SmartScriptTable physicProperties;
	if (entityProperties->GetValue("Physics", physicProperties))
	{
		physicProperties->GetValue("iPhysType", phys_type);
		physicProperties->GetValue("fPhysMass", phys_mass);
		physicProperties->GetValue("fPhysDensity", phys_density);
	}

	//Building properties
	SmartScriptTable buildingProperties;
	if (entityProperties->GetValue("Building", buildingProperties))
	{
		// Geometry
		buildingProperties->GetValue("sDefaultModel",default_model);
		buildingProperties->GetValue("sModel_1",model_1);
		buildingProperties->GetValue("sModel_2",model_2);
		buildingProperties->GetValue("sModel_3",model_3);
		buildingProperties->GetValue("sFinishModel",finish_model);
		

		// Materials
		buildingProperties->GetValue("sModel_1_material",Model_1_mat);
		buildingProperties->GetValue("sModel_2_material",Model_2_mat);
		buildingProperties->GetValue("sModel_3_material",Model_3_mat);
		buildingProperties->GetValue("sFinishMaterial",finishMat);
		//
		buildingProperties->GetValue("fBuildTime",build_time);
	}
	return true;
}

bool CNetworkBuilding::Reset()
{
	if(!GetSettings())
		return false;

	entity_name = GetEntity()->GetName();
	fStartTime = 0.f;
	build_status = -1;
	m_state = eState_NotUsed;

	GetEntity()->LoadGeometry(0,default_model);
	GetEntity()->SetMaterial(defMat);
	Physicalize(PE_NONE);

	CHANGED_NETWORK_STATE(this, POSITION_ASPECT);

	return true;
}

bool CNetworkBuilding::ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params )
{
	ResetGameObject();

	CRY_ASSERT_MESSAGE(false, "CCTFFlag::ReloadExtension not implemented");

	return false;
}

bool CNetworkBuilding::GetEntityPoolSignature( TSerialize signature )
{
	CRY_ASSERT_MESSAGE(false, "CCTFFlag::GetEntityPoolSignature not implemented");

	return true;
}

void CNetworkBuilding::Release()
{
	delete this;
}

bool CNetworkBuilding::NetSerialize( TSerialize ser, EEntityAspects aspect, uint8 profile, int pflags )
{
	if (aspect == POSITION_ASPECT)
	{
		if(gEnv->bServer && !gEnv->IsEditor())
		{
			int cur_state = (int)m_state;
			ws->SetInt(entity_name,"BuildStatus",build_status);
			ws->SetInt(entity_name,"EntityState",cur_state);
		}

		EState newState = m_state;
		ser.EnumValue("cur_state", newState, eState_NotUsed, eState_Done);
		ser.Value( "build_status", build_status);

		if (ser.IsWriting())
		{
			//CryLog("CNetworkBuilding::NetSerialize writing !!!");			
		}

		if (ser.IsReading())
		{
			//CryLog("CNetworkBuilding::NetSerialize reading !!!");
			m_state = newState;
			Building(build_status);
		}
	}

	return true;
}

void CNetworkBuilding::Update( SEntityUpdateContext& ctx, int updateSlot )
{
	if (g_pGame->GetIGameFramework()->IsEditing())
	{
		Reset();
		return;
	}

	float curTime = gEnv->pTimer->GetAsyncTime().GetSeconds();
	float newTime = fStartTime+(build_time/4);

	if(build_status!=-1 && gEnv->bServer)
	{
		if(curTime >= newTime && m_state!=eState_Done)
		{
			build_status++;

			Building(build_status);
			fStartTime = curTime;	
		}
	}

	// Test
	//Vec3 test = GetEntity()->GetWorldPos();
	//test.z = test.z+5.f;
	//gEnv->pRenderer->DrawLabel(test, 1.f,GetEntity()->GetName());
}

int CNetworkBuilding::CanUse(EntityId entityId) const
{
	const bool canUse = (m_state == eState_NotUsed);
	if(canUse)
		return 1;
	else
		return 0;
}

void CNetworkBuilding::StartBuild()
{
	if(m_state==eState_NotUsed && gEnv->bServer)
	{
		CryLog("CNetworkBuilding::Building started...");
		m_state = eState_InUse;
		build_status = 0;
		fStartTime = gEnv->pTimer->GetAsyncTime().GetSeconds();

		CHANGED_NETWORK_STATE(this, POSITION_ASPECT);
	}
}

bool CNetworkBuilding::Building(int part)
{
	if(part>0)
	{
		if(part==1)
		{
			CryLog("CNetworkBuilding::Building part 1...");
			// Geometry
			GetEntity()->LoadGeometry(0, model_1);
			// Material
			IMaterial *pMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial(Model_1_mat);
			GetEntity()->SetMaterial(pMat);
			// Physics
			Physicalize((pe_type)phys_type);
		}
		if(part==2)
		{
			CryLog("CNetworkBuilding::Building part 2...");
			GetEntity()->LoadGeometry(0, model_2);

			IMaterial *pMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial(Model_2_mat);
			GetEntity()->SetMaterial(pMat);

			Physicalize((pe_type)phys_type);
		}

		if(part==3)
		{
			CryLog("CNetworkBuilding::Building part 3...");
			GetEntity()->LoadGeometry(0, model_3);

			IMaterial *pMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial(Model_3_mat);
			GetEntity()->SetMaterial(pMat);

			Physicalize((pe_type)phys_type);
		}

		if(part==4)
		{
			CryLog("CNetworkBuilding::Building finish part...");
			GetEntity()->LoadGeometry(0, finish_model);

			IMaterial *pMat = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial(finishMat);
			GetEntity()->SetMaterial(pMat);

			Physicalize((pe_type)phys_type);

			m_state = eState_Done;
			CryLog("CNetworkBuilding::Building finished !");
		}

		if (gEnv->bServer)
		{
			CHANGED_NETWORK_STATE(this, POSITION_ASPECT);	
		}
	}

	return true;
}

void CNetworkBuilding::Physicalize(pe_type phys_type)
{
	SEntityPhysicalizeParams params;
	params.type = phys_type;
	params.density = phys_density;
	params.mass = phys_mass;

	GetEntity()->Physicalize(params);

	if (gEnv->bServer)
	{
		GetGameObject()->SetAspectProfile(eEA_Physics, phys_type);
	}
}

void CNetworkBuilding::ReadWorldState()
{
	if(gEnv->bServer && ws->CheckEntityTag(entity_name) && !gEnv->IsEditor())
	{
		build_status = ws->GetInt(entity_name,"BuildStatus");
		m_state = (EState)ws->GetInt(entity_name,"EntityState");
	}
}
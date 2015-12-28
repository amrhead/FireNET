///////////////////////////////////////////////////////////////////////////
//
//  Crytek Source File.
//  Copyright (C), Crytek Studios, 1999-2014.
// -------------------------------------------------------------------------
//  Created          : 13/05/2014 by Jean Geffroy
//
///////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MannequinObject.h"

#include "IAnimatedCharacter.h"

///////////////////////////////////////////////////////////////////////////
CMannequinObject::CMannequinObject()
	: m_pAnimatedCharacter(NULL)
{
}

///////////////////////////////////////////////////////////////////////////
CMannequinObject::~CMannequinObject()
{
	if (m_pAnimatedCharacter)
	{
		IGameObject* pGameObject = GetGameObject();
		pGameObject->ReleaseExtension("AnimatedCharacter");
	}
}

///////////////////////////////////////////////////////////////////////////
bool CMannequinObject::Init(IGameObject* const pGameObject)
{
	SetGameObject(pGameObject);

	m_pAnimatedCharacter = static_cast<IAnimatedCharacter*>(pGameObject->AcquireExtension("AnimatedCharacter"));

	pGameObject->EnablePrePhysicsUpdate(ePPU_Always);
	pGameObject->EnablePhysicsEvent(true, eEPE_OnPostStepImmediate);

	return true;
}

///////////////////////////////////////////////////////////////////////////
void CMannequinObject::PostInit(IGameObject* const pGameObject)
{
	Reset();
}

///////////////////////////////////////////////////////////////////////////
void CMannequinObject::Reset()
{
	IEntity* const pEntity = GetEntity();

	// Reset AnimatedCharacter
	if (m_pAnimatedCharacter)
	{
		m_pAnimatedCharacter->ResetState();
		m_pAnimatedCharacter->Init(GetGameObject());
		m_pAnimatedCharacter->SetMovementControlMethods(eMCM_Animation, eMCM_Animation);

		if (IActionController* pActionController = m_pAnimatedCharacter->GetActionController())
		{
			pActionController->GetContext().state.Clear();
		}
	}

	// Load character
	if (IScriptTable* pScriptTable = pEntity->GetScriptTable())
	{
		SmartScriptTable propertiesTable;
		if (pScriptTable->GetValue("Properties", propertiesTable))
		{
			const char* modelName = NULL;
			if (propertiesTable->GetValue("objModel", modelName) && modelName)
			{
				const int slot = 0;
				pEntity->LoadCharacter(slot, modelName);
			}
		}
	}

	// Physicalize
	{
		SEntityPhysicalizeParams physicsParams;
		physicsParams.type              = PE_LIVING;
		physicsParams.nSlot             = 0;
		physicsParams.mass              = 80.0f;
		physicsParams.nFlagsOR          = pef_monitor_poststep;

		pe_player_dynamics playerDyn; 
		playerDyn.gravity               = Vec3( 0, 0, 9.81f );
		playerDyn.kAirControl           = 0.9f;
		playerDyn.mass                  = 80;
		playerDyn.minSlideAngle         = 45;
		playerDyn.maxClimbAngle         = 50;
		playerDyn.minFallAngle          = 50;
		playerDyn.maxVelGround          = 16;
		physicsParams.pPlayerDynamics   = &playerDyn;

		pe_player_dimensions playerDim;
		playerDim.heightCollider        = 1.0f;
		playerDim.sizeCollider          = Vec3(0.4f, 0.4f, 0.1f);
		playerDim.heightPivot           = 0.0f;
		playerDim.maxUnproj             = 0.0f;
		playerDim.bUseCapsule           = true;
		physicsParams.pPlayerDimensions = &playerDim;

		pEntity->Physicalize(physicsParams);
	}
}

///////////////////////////////////////////////////////////////////////////
void CMannequinObject::OnScriptEvent( const char* eventName )
{
	assert(eventName != NULL);

	const bool isOnPropertyChangeEvent = 0 == strcmp(eventName, "OnPropertyChange");
	if (isOnPropertyChangeEvent)
	{
		Reset();
	}
}

///////////////////////////////////////////////////////////////////////////
void CMannequinObject::ProcessEvent(SEntityEvent& evt)
{
	switch (evt.event)
	{
	case ENTITY_EVENT_RESET:
		{
			Reset();
		}
		break;

	case ENTITY_EVENT_SCRIPT_EVENT:
		{
			const char* eventName = reinterpret_cast<const char*>(evt.nParam[0]);
			OnScriptEvent(eventName);
		}
		break;
	}
}

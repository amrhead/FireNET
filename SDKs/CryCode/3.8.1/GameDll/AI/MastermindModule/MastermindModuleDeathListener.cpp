// ============================================================================
// ============================================================================
// ============================================================================
// ====
// ==== Mastermind Module - Death Listener
// ====
// ==== An AI system listener that will reports deaths that are relevant to 
// ==== the mastermind.
// ====
//


#include "StdAfx.h"
#include "MastermindModuleDeathListener.h"

#include "Actor.h"
#include "CryAssert.h"

#include "MastermindModule.h"


namespace Mastermind
{


// ============================================================================
// ============================================================================
// ============================================================================
//
// -- MastermindModuleDeathListener -- MastermindModuleDeathListener --
//
// ============================================================================
// ============================================================================
// ============================================================================


MastermindModuleDeathListener::MastermindModuleDeathListener() :
m_MastermindModule(NULL)
{
	assert(gEnv->pAISystem != NULL);
	gEnv->pAISystem->RegisterListener(this);
}


MastermindModuleDeathListener::~MastermindModuleDeathListener()
{
	if (gEnv->pAISystem != NULL)
	{
		gEnv->pAISystem->UnregisterListener(this);
	}
}

// ============================================================================
//	Init
//
//	In:		The MasterMind Module
//
void MastermindModuleDeathListener::Init(MastermindModule *mastermindModule)
{
	assert(mastermindModule != NULL);
	m_MastermindModule = mastermindModule;
}

// ============================================================================
//	Event: An entity died.
//
//	In:		The ID of the dead entity (0 will abort!)
//
void MastermindModuleDeathListener::OnAgentDeath(EntityId deadEntityID)
{
	if (deadEntityID == (EntityId)0)
	{
		return;
	}
	const IEntity* entity = gEnv->pEntitySystem->GetEntity(deadEntityID);
	if (!IsEntityResurrectable(entity))
	{
		return;
	}

	assert(m_MastermindModule != NULL);
	m_MastermindModule->ReportResurrectableEntity(deadEntityID);
}


// ===========================================================================
//	Query if an entity is resurrectable.
//
//	In:		Pointer to the target entity (NULL will abort!)
//
//	Returns:	True if resurrectable; otherwise false.
//
bool MastermindModuleDeathListener::IsEntityResurrectable(const IEntity* entity) const
{
	// Do the cheap checks first.
	if (entity->IsHidden())
	{
		return false;
	}

	// And then the more expensive ones.
	IScriptTable* scriptTable = entity->GetScriptTable();
	if (scriptTable == NULL)
	{
		return false;
	}

	SmartScriptTable props;	
	if (!(scriptTable->GetValue("Properties", props)))
	{
		return false;
	}
	SmartScriptTable propsResurrection;
	if (!(props->GetValue("Resurrection", propsResurrection)))
	{
		return false;
	}	
	bool huskableByMastermind = false;
	if (propsResurrection->GetValue("bHuskableByMastermind", huskableByMastermind))
	{
		if (!huskableByMastermind)
		{
			return false;
		}
	}

	return true;
}


} // namespace Mastermind

// ============================================================================
// ============================================================================
// ============================================================================
// ====
// ==== Mastermind Module
// ====
// ==== This module provides a number of services for the Mastermind agent:
// ====		- Resurrection
// ====
//


// YET2DO
// - Lock-outs on resurrection targets in case there are multiple masterminds.
// - Serialize! (the deflector shield also needs serialization!?)
// - Solve the problem with the grunts' weapons that are not completely detached
//   from the dead bodies.
// - Take care of other nasty possibilities: what happens if the corpse is grabbed or
//   moved or whatever during the resurrection sequence?
// - Handle this with an notification even from the entity system! See:
//	 gEnv->pEntitySystem->AddSink(this, IEntitySystem::OnEvent, 1 << ENTITY_EVENT_XFORM);


#include "StdAfx.h"
#include "MastermindModule.h"

#include "CryAssert.h"

#include "Cry_Math.h"
#include "Cry_GeoDistance.h"

#include <IAIObject.h>
#include <IAIObjectManager.h>
#include <ISystem.h>

#include "IAISystem.h"
#include "GameObjects\GameObject.h"

#include "..\GameAIEnv.h"
#include "..\Agent.h"
#include "..\..\Actor.h"
#include "..\..\Environment\DeflectorShield.h"

#include "GameCVars.h"


namespace Mastermind
{

using namespace primitives;


// ============================================================================
// ============================================================================
// ============================================================================
//
// -- Settings -- Settings -- Settings -- Settings -- Settings -- Settings --
//
// ============================================================================
// ============================================================================
// ============================================================================


// ===[ Resurrection: ]===

// How long it takes to lift the dead bodies from the ground during the 
// resurrection state (>= 0.0f) (in seconds).
static const float g_HuskResurrectionLiftingDuration = 2.0f;

// How fast the dead bodies are lifted from the ground (>= 0.0f) (in meters / second).
static const float g_HuskResurrectionLifingSpeed = 1.0f;

// How long it takes to transform the dead bodies into actual husks (>= 0.0f) (in seconds).
static const float g_HuskResurrectionHuskingDuration = 1.0f;

// How long it takes to pull the husks into formation.
static const float g_HuskResurrectionPullIntoFormationDuration = 0.6f;

// The minimum and maximum amount of husks that are spawned from a _single_
// corpse.
static const int g_HuskResurrectionMinAmount = 5;
static const int g_HuskResurrectionMaxAmount = g_HuskResurrectionMinAmount;

#if !defined(_RELEASE)

// Debug color used during resurrection.
static const ColorF g_HuskResurrectionDebugColor = Col_Cyan;

#endif


// ===[ Formation: ]===

// The maximum smoothing factor for the husk formation. The closer to 1.0f,
// the more exact the positioning will be, but we want to smooth it a bit!
static const float g_HuskFormationMaxSmoothingFactor = 0.95f;


// ===[ Offensive Formation: ]===

// The angular velocity of the husk offensive formation rotation (in radians / second).	
static const float g_HuskFormationOfsAngularVelocity = DEG2RAD(130.0f);

// The offensive formation pivot offset (in meters).
static const Vec3 g_HuskFormationOfsPivotOffset(0.0f, 0.0f, 2.9f);

// The offensive radius around the husk formation pivot position (>= 0.0f) (in meters).
static const float g_HuskFormationOfsRadius = 4.2f;

// The offensive formation plane angle (>= 0.0f) (in radians).
static const float g_HuskFormationOfsPlaneAngle = DEG2RAD(35.0f);


// ===[ Defensive Formation: ]===

// The angular velocity of the husk defensive formation rotation (in radians / second).	
static const float g_HuskFormationDefAngularVelocity = DEG2RAD(-80.0f);

// The defensive formation pivot offset (in meters).
static const Vec3 g_HuskFormationDefPivotOffset(0.0f, 0.0f, 0.5f);

// The defensive radius around the husk formation pivot position (>= 0.0f) (in meters).
static const float g_HuskFormationDefRadius = 5.0f;

// The defensive formation plane angle (>= 0.0f) (in radians).
static const float g_HuskFormationDefPlaneAngle = DEG2RAD(0.0f);


// ===[ Deflector shield: ]===

// The name of the deflector shield game object extension module.
static const char g_DeflectorShieldGameObjectExtensionName[] = "DeflectorShield";


// ===[ Husk perception: ]===


// How long to suppress incoming 'enemy seen' notifications from the Husks 
// after we have just reported it to the Mastermind (this is to prevent
// cascading, because the mastermind also notifies all the other Husks
// of this event).
static const float g_HsuKPerceptionNotifyCascadeDelay = 1.0f;


// ============================================================================
// ============================================================================
// ============================================================================
//
// -- Globals -- Globals -- Globals -- Globals -- Globals -- Globals --
//
// ============================================================================
// ============================================================================
// ============================================================================


// ============================================================================
//	Wrap an angle into its normal domain.
//
//	In:		The angle (in radians). Note: the angle must not be too far outside 
//			normal range!)
//
//	Returns:	The wrapped angle (in the range of [0 .. gf_PI2>) (in radians).
//
float WrapAngle (float angle)
{
	while (angle > 0.0f)
	{
		angle -= gf_PI2;
	}

	while (angle < 0.0f)
	{
		angle += gf_PI2;
	}

	return angle;
}


// ============================================================================
// ============================================================================
// ============================================================================
//
// -- ModuleInstanceEntityInfo -- ModuleInstanceEntityInfo -- ModuleInstanceEntityInfo -- ModuleInstanceEntityInfo -- ModuleInstanceEntityInfo --
//
// ============================================================================
// ============================================================================
// ============================================================================


ModuleInstanceEntityInfo::ModuleInstanceEntityInfo():
	m_EntityID((EntityId)0)
	, m_WorldPos(ZERO)
{
}


ModuleInstanceEntityInfo::ModuleInstanceEntityInfo(const EntityId entityID, const Vec3& worldPos):
	m_EntityID(entityID)
	, m_WorldPos(worldPos)
{
}


// ============================================================================
// ============================================================================
// ============================================================================
//
// -- ResurrectableCandidateEntityInfo -- ResurrectableCandidateEntityInfo --
//
// ============================================================================
// ============================================================================
// ============================================================================


ResurrectableCandidateEntityInfo::ResurrectableCandidateEntityInfo(const EntityId entityID) :
	m_EntityID(entityID)
{
	// Nothing to do here.
}


// ============================================================================
// ============================================================================
// ============================================================================
//
// -- MastermindModuleInstance -- MastermindModuleInstance --
//
// ============================================================================
// ============================================================================
// ============================================================================


MastermindModuleInstance::MastermindModuleInstance() :	
	m_Config()
	, m_ResurrectEntities()
	, m_ResurrectionTargetsSignaledFlag(false)
	, m_ResurrectionState(rsIdle)
	, m_NextResurrectTrackingTime(-1.0f)
	, m_HuskEntityIDs()
	, m_LockCounter(0)
	, m_FormationAngularVelocity(0.0f)
	, m_HuskFormationPlaneAngle(0.0f)
	, m_HuskFormationRadius(0.0f)
	, m_HuskFormationPivotOffset(ZERO)
	, m_HuskFormationRotationAngle(0.0f)
	, m_HuskMoveSmoothingFactor(0.0f)
	, m_ResurrectionTimer(0.0f)
	, m_ShieldedFlag(false)
	, m_ShieldEntityID((EntityId)0)
	, m_FormationMode(fmUndefined)
	, m_EnemySeenNotiftyTime(-1.0f)
	, m_EnemySpotterEntityID((EntityId)0)
	, m_SpottedEntityID((EntityId)0)
	, m_SpottedEnemyPos(ZERO)	
	, m_EnemySeenSuppressTime(-1.0f)	
{
	m_ResurrectEntities.reserve(8);
	m_HuskEntityIDs.reserve(8);
}


MastermindModuleInstance::~MastermindModuleInstance()
{
	// Nothing to do here.
}


// ===========================================================================
//	Setup the mastermind instance.
//
//	In:		The configuration.
//
void MastermindModuleInstance::SetupMastermind(
	const MastermindModuleInstanceConfig& config)
{
	CRY_ASSERT_MESSAGE(config.m_SearchRadius >= 0.0f, "Invalid search radius!");
	CRY_ASSERT_MESSAGE(config.m_ResurrectionRadius >= 0.0f, "Invalid resurrection radius!");
	CRY_ASSERT_MESSAGE(!config.m_ResurrectablesAvailableSignal.empty(), "Invalid resurrectables available signal!");
	CRY_ASSERT_MESSAGE(!config.m_ResurrectablesUnavailableSignal.empty(), "Invalid resurrectables unavailable signal!");

	m_Config = config;

	SelectFormationMode(fmOffsensive);	
}


void MastermindModuleInstance::Update(float frameTime)
{
	// How long to delay between successive resurrection tracking intervals
	// (>= 0.0f) (in seconds).
	const float resurrectTrackingPeriod = 1.0f;

	BaseClass::Update(frameTime);

	// Applying some load-balancing here because the resurrectable queries
	// might be a bit 'costly'.
	const float now = gEnv->pTimer->GetCurrTime();
	if (m_NextResurrectTrackingTime <= now)
	{
		TrackNearbyResurrectableEntities();
		m_NextResurrectTrackingTime = now + resurrectTrackingPeriod;
	}	
	
	// Always send signals as soon as possible.
	if (!IsResurrecting())
	{
		SignalOwnerAboutResurrectionOpportunities();
	}
	else
	{
		ProgressResurrection(frameTime);			
	}
	
	ProgressEnemySpottingNotifications(now);

	// Keep the pose of the shield entity synchronized with that of
	// the owner (note: we even do this even if the shield is
	// invisible just as a safety precaution (so it can never end up
	// at some random place, which may introduce really hard to find
	// bugs)).
	if (m_ShieldEntityID != (EntityId)0)
	{
		AlignShieldWithOwner();
	}	

#if !defined(_RELEASE)
	DebugRender();
#endif // !defined(_RELEASE)
}


void MastermindModuleInstance::Destroy()
{
	KillAllSlaveHusks();

	PurgeShieldEntity();

	BaseClass::Destroy();
}


// ============================================================================
//	Get the closest resurrectable entity.
//
//	Returns:	A pointer to the closest ressurectable entity; otherwise NULL.
//
IEntity* MastermindModuleInstance::GetClosestResurrectEntity() const
{
	if (m_ResurrectEntities.empty())
	{
		return NULL;
	}

	sphere searchArea;
	RetrieveSearchArea(&searchArea);
	
	bool closerFlag;
	float entityDistanceSq;
	float closestEntityDistanceSq = 0.0f;	
	EntityId closestEntityID = (EntityId)0;
	RessurectEntities::const_iterator entityIter = m_ResurrectEntities.begin();
	RessurectEntities::const_iterator entityIterEnd = m_ResurrectEntities.end();
	for (entityIter = m_ResurrectEntities.begin() ; entityIter != entityIterEnd ; ++entityIter)
	{		
		entityDistanceSq = Distance::Point_PointSq(searchArea.center, entityIter->m_WorldPos);
		if (closestEntityID == (EntityId)0)
		{
			closerFlag = true;
		}
		else
		{
			closerFlag = (entityDistanceSq < closestEntityDistanceSq);
		}

		if (closerFlag)
		{
			closestEntityDistanceSq = entityDistanceSq;
			closestEntityID = entityIter->m_EntityID;
		}
	}
	
	return gEnv->pEntitySystem->GetEntity(closestEntityID);
}


// ============================================================================
//	Start the resurrection sequence.
//
void MastermindModuleInstance::StartResurrection()
{
	assert(m_ResurrectionState == rsIdle);
	SetResurrectionState(rsLifting);	
}


// ============================================================================
//	Finalize the resurrection process (and spawn the actually husks).
//
void MastermindModuleInstance::FinalizeResurrection()
{
	switch (m_ResurrectionState)
	{
	case rsIdle:
		break;

	case rsLifting:
	case rsFormingHusks:
	case rsHusksFormed:
		// For now we will allow the actual resurrection to be performed at
		// any moment (makes animation prototyping easier).
		SetResurrectionState(rsSpawnHusks);
		break;

	case rsSpawnHusks:
	case rsPullIntoFormation:
	case rsInFormation:
		break;

	default:
		// We should never get here!
		assert(false);
		break;
	}
}


// ============================================================================
//	Abort the resurrection sequence.
//
void MastermindModuleInstance::AbortResurrection()
{
	KillAllSlaveHusks();
	SetResurrectionState(rsIdle);
}


// ============================================================================
//	Register a slave husk.
//
//	In:		Pointer to the target slave entity (NULL will abort!)
//
void MastermindModuleInstance::RegisterSlaveHusk(IEntity* slaveEntity)
{
	assert(m_LockCounter <= 0);

	if (slaveEntity == NULL)
	{
		return;
	}

	assert(!IsHuskRegisteredAsSlave(slaveEntity->GetId()));
	
	// Report the entity ID of the mastermind in the husk entity as well
	// so that it can properly unregister itself when it dies for example.	
	IScriptTable* scriptTable = slaveEntity->GetScriptTable();
	assert(scriptTable != NULL);	
	HSCRIPTFUNCTION scriptFunction;
	const char luaFuncName[] = "OnRegisteredToMastermind";
	if (scriptTable->GetValue(luaFuncName, scriptFunction))  
	{
		Script::CallMethod(scriptTable, luaFuncName, GetEntityID());
	}

	m_HuskEntityIDs.push_back(slaveEntity->GetId());

	// Signal to the mastermind that he is being protected now.
	if (m_HuskEntityIDs.size() == 1)
	{
		SendSignal(m_Config.m_HusksSpawnedSignal.c_str());
	}	
}


// ============================================================================
//	Unregister a slave husk.
//
//	In:		The entity ID of the target slave husk (0 or an invalid ID will 
//			abort).
//
void MastermindModuleInstance::UnregisterSlaveHusk(EntityId slaveEntityID)
{
	assert(m_LockCounter <= 0);

	SlavedEntityIDs::iterator entityIter = FindSlavedEntityIter(slaveEntityID);
	if (entityIter == m_HuskEntityIDs.end())
	{
		return;
	}

	// TODO: Ugly hack to get rid of the Husks for now (otherwise they will just hang 
	// there in the air). This is just for demo purpose.
	IEntity* entity = gEnv->pEntitySystem->GetEntity(slaveEntityID);
	if (entity != NULL)
	{
		entity->Hide(true);
	}

	m_HuskEntityIDs.erase(entityIter);

	// Signal to the mastermind that he might not be protected anymore.
	if (m_HuskEntityIDs.empty())
	{
		SendSignal(m_Config.m_HusksAllDestroyedSignal.c_str());
	}	
}


// ============================================================================
//	Query if a slave husk has been registered.
//
//	In:		The entity ID of the slave husk (0 will abort!)
//
//	Returns:	True if the hask has been registered as a slave; otherwise false.
//
bool MastermindModuleInstance::IsHuskRegisteredAsSlave(const EntityId entityID)
{
	return (FindSlavedEntityIter(entityID) != m_HuskEntityIDs.end());
}


// ============================================================================
//	Kill all slave husks.
//
void MastermindModuleInstance::KillAllSlaveHusks()
{
	m_LockCounter++;
	KillAllSlaveHusksHelper();
	m_LockCounter--;
}


// ============================================================================
//	Query if the mastermind is/has resurrected any husks.
//
//	Returns:	True if the mastermind is resurrecting; otherwise false.
//
bool MastermindModuleInstance::IsResurrecting() const
{
	return (m_ResurrectionState != rsIdle);
}


// ============================================================================
//	Notification: A slave Husk has just seen a target.
//
//	In:		The entity ID of the target slave husk (0 or an invalid ID will 
//			abort).
//	In:		The entity ID of the seen target (0 or an invalid ID will 
//			abort).
//
void MastermindModuleInstance::NotifySlaveHuskTargetSeen(
	EntityId slaveEntityID, EntityId seenEntityID)
{	
	// Should we ignore this notification to suppress possible cascading?
	if (m_EnemySeenSuppressTime >= 0.0f)
	{
		return;
	}

	SlavedEntityIDs::iterator entityIter = FindSlavedEntityIter(slaveEntityID);
	if (entityIter == m_HuskEntityIDs.end())
	{
		assert(false);
		return;
	}	
	IEntity* seenEntity = gEnv->pEntitySystem->GetEntity(seenEntityID);
	if (seenEntity == NULL)
	{	// Apparently this entity died already or something, so it is
		// safe to ignore it.
		return;
	}

	// If a notification was already pending then don't overwrite it
	// but just update it with the latest coordinate.
	if (m_EnemySeenNotiftyTime < 0.0f)
	{
		const float now = gEnv->pTimer->GetCurrTime();
		m_EnemySeenNotiftyTime = now + m_Config.m_EnemySpottedByHuskNotifyDelay;
	}
	m_EnemySpotterEntityID = slaveEntityID;	
	m_SpottedEntityID = seenEntityID;
	m_SpottedEnemyPos = seenEntity->GetWorldPos();
}


// ============================================================================
//	Select a husk formation by name.
//
//	In:		The husk formation name (case-insensitive) (NULL will abort!)
//
void MastermindModuleInstance::SelectFormationMode(const char *modeName)
{
	FormationMode formationMode = fmUndefined;

	if (modeName != NULL)
	{
		if (stricmp(modeName, "defensive") == 0)
		{
			formationMode = fmDefensive;
		}
		else if (stricmp(modeName, "offensive") == 0)
		{
			formationMode = fmOffsensive;
		}
	}

	if (formationMode == fmUndefined)
	{
		CryLog("MastermindModuleInstance::SelectFormationMode(): Invalid formation mode name: %s!",
			(modeName != NULL) ? modeName : "<NULL>");
		return;
	}

	SelectFormationMode(formationMode);
}


// ============================================================================
//	Select a defensive husk formation.
//
void MastermindModuleInstance::SelectDefensiveFormation()
{
	m_FormationMode = fmDefensive;

	m_FormationAngularVelocity = g_HuskFormationDefAngularVelocity;
	m_HuskFormationPivotOffset = g_HuskFormationDefPivotOffset;
	m_HuskFormationRadius      = g_HuskFormationDefRadius;
	m_HuskFormationPlaneAngle  = g_HuskFormationDefPlaneAngle;
}


// ============================================================================
//	Select an offensive husk formation.
//
void MastermindModuleInstance::SelectOffensiveFormation()
{
	m_FormationMode = fmOffsensive;

	m_FormationAngularVelocity = g_HuskFormationOfsAngularVelocity;
	m_HuskFormationPivotOffset = g_HuskFormationOfsPivotOffset;
	m_HuskFormationRadius      = g_HuskFormationOfsRadius;
	m_HuskFormationPlaneAngle  = g_HuskFormationOfsPlaneAngle;	
}


// ============================================================================
//	Enable the shield (if needed).
//
void MastermindModuleInstance::EnableShield()
{
	if (m_ShieldedFlag)
	{
		return;
	}

	// Create the shield entity if it doesn't already exists yet.
	// Note: we are depending on this function being called from outside the
	// module update loop (so from a behavior for example), so that the module
	// will not complain that entities are making changes to the modules
	// because of spawning.
	IEntity* shieldEntity = GetShieldEntity();
	if (shieldEntity == NULL)
	{		
		shieldEntity = SpawnShieldEntity();
		m_ShieldEntityID = shieldEntity->GetId();
	}
	if (shieldEntity == NULL)
	{
		CryLog("Mastermind: Failed to spawn shield entity!");
		return;
	}

	shieldEntity->Hide(false);	
	m_ShieldedFlag = true;

	AlignShieldWithOwner();
}


// ============================================================================
//	Disable the shield (if needed).
//
void MastermindModuleInstance::DisableShield()
{
	if (!m_ShieldedFlag)
	{
		return;
	}

	IEntity* shieldEntity = GetShieldEntity();
	if (shieldEntity != NULL)
	{
		shieldEntity->Hide(true);
	}
	m_ShieldedFlag = false;
}


// ============================================================================
//	Track all nearby resurrectable entities.
//
void MastermindModuleInstance::TrackNearbyResurrectableEntities()
{
	m_ResurrectEntities.resize(0);

	// For now we will search in a sphere around the instance owner agent's pivot
	// position.
	sphere sphereArea;
	RetrieveSearchArea(&sphereArea);
	const float sphereAreaRadiusSq = sphereArea.r * sphereArea.r;

	const ResurrectableCandidateEntityInfos& candidateEntities = gGameAIEnv.mastermindModule->GetResurrectionCandidates();
	
	ResurrectableCandidateEntityInfos::const_iterator candidateIter;
	ResurrectableCandidateEntityInfos::const_iterator candidateIterEnd = candidateEntities.end();

	for (candidateIter = candidateEntities.begin() ; candidateIter != candidateIterEnd ; ++candidateIter)
	{
		Agent agent(candidateIter->m_EntityID);
		// (note: just make sure the agent still exists, in case the level designers disable parts of the map).
		// TODO: handle this with an notification even from the entity system!
		if (agent.IsValid())
		{		
			if (Distance::Point_PointSq(sphereArea.center, agent.GetEntityPos()) <= sphereAreaRadiusSq)
			{
				if (CanAgentBeResurrectedNow(agent))
				{
					m_ResurrectEntities.push_back(
						ModuleInstanceEntityInfo(
							agent.GetEntityID(),
							agent.GetPos()));
				}
			}
		}
	}
}


// ============================================================================
//	Signal the owner agent if resurrectable targets are nearby or not.
//
void MastermindModuleInstance::SignalOwnerAboutResurrectionOpportunities()
{
	const bool resurrectionTargetsAvailableFlag = !m_ResurrectEntities.empty();

	if (m_ResurrectionTargetsSignaledFlag != resurrectionTargetsAvailableFlag)
	{
		m_ResurrectionTargetsSignaledFlag = resurrectionTargetsAvailableFlag;

		if (resurrectionTargetsAvailableFlag)
		{
			SendSignal(m_Config.m_ResurrectablesAvailableSignal.c_str());
		}
		else
		{
			SendSignal(m_Config.m_ResurrectablesUnavailableSignal.c_str());
		}		
	}
}


// ============================================================================
//	Retrieve the area in which to search for resurrection targets.
//
//	Out:	The search sphere (NULL is invalid!)
//
void MastermindModuleInstance::RetrieveSearchArea(sphere *searchSphere) const
{
	Agent ownerAgent(GetEntityID());
	CRY_ASSERT_MESSAGE(ownerAgent.IsValid(), "Only agents can be registered to the resurrection module!");

	searchSphere->center = ownerAgent.GetPos();
	searchSphere->r = m_Config.m_SearchRadius;
}


// ============================================================================
//	Query if an agent can be resurrected now.
//
//	In:		The agent.
//
//	Returns:	True if the agent is resurrectable now; otherwise false.
//	
bool MastermindModuleInstance::CanAgentBeResurrectedNow(
	const Agent& agent) const
{
	const IActor* actorInterface = agent.GetActor();
	if (actorInterface == NULL)
	{
		return false;
	}
	const CActor* actor = (CActor*)actorInterface;

	const SActorStats *actorStats = actor->GetActorStats();
	if (actorStats == NULL)
	{	// There might be something wrong with the internal state of this actor,
		// so leave it alone just to be sure.
		return false;
	}
	if (actorStats->isGrabbed)
	{	// Safety code.
		return false;
	}	
	if (!(actorStats->isRagDoll))
	{	// We should wait for the entity to be a rag-doll so that we know that it completed
		// its death animation (otherwise we run the risk that a Mastermind starts a 
		// resurrection just immediately after the actor start dieing).
		return false;
	}

	return true;
}


// ============================================================================
//	Perform the actual resurrection (the rest is just animation and fluff).
//
void MastermindModuleInstance::ResurrectAllEntities()
{	
	RessurectEntities::const_iterator entityIter;
	RessurectEntities::const_iterator entityIterEnd = m_ResurrectEntities.end();
	for (entityIter = m_ResurrectEntities.begin() ; entityIter != entityIterEnd ; 
		++entityIter)
	{
		const int husksAmount = g_HuskResurrectionMinAmount + 
			(int)(cry_frand() * (float)(g_HuskResurrectionMaxAmount - g_HuskResurrectionMinAmount));
		ResurrectEntity(gEnv->pEntitySystem->GetEntity(entityIter->m_EntityID),
			husksAmount);
	}
	
	// And immediately signal to the entity that there are currently no more 
	// resurrection targets.
	m_ResurrectEntities.resize(0);
	SignalOwnerAboutResurrectionOpportunities();
}


// ============================================================================
//	Resurrect an entity.
//
//	In,out:		Pointer to the entity (NULL will abort!)
//	In:			The amount of husks to spawn (>= 1).
//
void MastermindModuleInstance::ResurrectEntity(
	IEntity* sourceEntity, int husksAmount /* = 1 */)
{
	EntityId huskEntityID = (EntityId)0;

	if (sourceEntity == NULL)
	{
		return;
	}

	// No-one else should try to resurrect this entity anymore.
	gGameAIEnv.mastermindModule->UnregisterResurrectionCandidate(sourceEntity->GetId());

	int huskIndex;
	for (huskIndex = 0 ; huskIndex < husksAmount ; huskIndex++)
	{
		SpawnHusk(sourceEntity, sourceEntity->GetPos(), sourceEntity->GetRotation());
	}

	
	/* (Keeping this code around just for now because I still need to resolve the issue with the weapons
	   that dead grunts have in their hand won't despawn)
	IActor* actorInterface = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(sourceEntity->GetId());
	if (actorInterface != NULL)
	{	
		IInventory* inventoryInterface = actorInterface->GetInventory();
		if (inventoryInterface != NULL)
		{
			// Note: destroying the inventory does not appear to work instantaneously,
			// so just hide the weapon that the entity was holding.
			EntityId currentItemEntityID = inventoryInterface->GetCurrentItem();
			IEntity* currentItemEntity = gEnv->pEntitySystem->GetEntity(currentItemEntityID);
			if (currentItemEntity != NULL)
			{
				currentItemEntity->Hide(true);
			}

			inventoryInterface->Destroy();
		}		
	}
	*/

	
	// Note: actually removing the entity causes all sorts of glitches in the editor,
	// so it is better to just hide the entity.
	//gEnv->pEntitySystem->RemoveEntity(sourceEntity->GetId());
	sourceEntity->Hide(true);	
	return;
}


// ============================================================================
//	Spawn a husk entity.
//
//	In:		The source entity (from which the husk is 'created' so to speak)
//			(NULL is invalid!)
//	In:		The start position (in world-space).
//	In:		The start orientation (in world-space).
//
void MastermindModuleInstance::SpawnHusk(
	IEntity* sourceEntity, const Vec3& startPos, const Quat& startRotation)
{
	assert(sourceEntity != NULL);

	CryFixedStringT<20> newEntityName;
	newEntityName.Format("Husk%u", gGameAIEnv.mastermindModule->GenerateHuskNameID());

	SEntitySpawnParams spawnParams;		
	spawnParams.bStaticEntityId = false;		
	spawnParams.pArchetype = gEnv->pEntitySystem->LoadEntityArchetype("Characters.Husk.HuskSlave");
	CRY_ASSERT_MESSAGE(spawnParams.pArchetype != NULL, "Unable to load Husk entity archetype!");
	spawnParams.vPosition = startPos;
	spawnParams.qRotation = startRotation;
	spawnParams.sName = newEntityName.c_str();
	spawnParams.pUserData = 0;
	
	IEntity	*newEntity = gEnv->pEntitySystem->SpawnEntity(spawnParams);
	if (newEntity == NULL)
	{
		CRY_ASSERT_MESSAGE(false, "Failed to spawn Husk!");
		return;
	}

	// Let the new husk join the group of the mastermind so that they 
	// share perception information and such.	
	int ownerEntityGroupID = GetEntity()->GetAI()->GetGroupId();
#if !defined(_RELEASE)
	if (ownerEntityGroupID == 0)
	{
		CryLog("Mastermind %s has group ID 0 and thus its Husks cannot join it for shared perception!",
			sourceEntity->GetName());
	}
#endif	
	newEntity->GetAI()->SetGroupId(ownerEntityGroupID);

	RegisterSlaveHusk(newEntity);

#if defined(MASTERMIND_EDITOR_ONLY)	
	gGameAIEnv.mastermindModule->EditorOnlyReportSpawnedEntity(newEntity->GetId());
#endif
}


// ============================================================================
//	Get the iterator to a registered slave husk.
//
//	In:		The entity ID of the slave husk (0 will abort!)
//
//	Returns:	Iterator to the corresponding entry in m_HuskEntityIDs or
//				m_HuskEntityIDs.end() if it could not be found.
//
MastermindModuleInstance::SlavedEntityIDs::iterator MastermindModuleInstance::FindSlavedEntityIter(
	const EntityId entityID)
{
	if (entityID == (EntityId)0)
	{
		return m_HuskEntityIDs.end();
	}
	return std::find(m_HuskEntityIDs.begin(), m_HuskEntityIDs.end(), entityID);
}


// ============================================================================
//	Fixate the resurrect entities' positions.
//
void MastermindModuleInstance::FixateResurrectEntityPositions()
{
	IEntity* entity;
	RessurectEntities::const_iterator entityIter;
	RessurectEntities::const_iterator entityIterEnd = m_ResurrectEntities.end();
	for (entityIter = m_ResurrectEntities.begin() ; entityIter != entityIterEnd ; ++entityIter)
	{
		entity = gEnv->pEntitySystem->GetEntity(entityIter->m_EntityID);
		if (entity != NULL)
		{
			entity->SetPos(entityIter->m_WorldPos);
		}
	}
}


// ============================================================================
//	Remove any invalid entities from the resurrect candidate list.
//
//	This function will basically prune all entities that no longer exist on the
//	world.
//
void MastermindModuleInstance::RemoveInvalidResurrectEntities()
{
	assert(m_LockCounter == 0);

	m_LockCounter++;
	RemoveInvalidResurrectEntitiesHelper();
	m_LockCounter--;
}


// ============================================================================
//	Helper: Remove any invalid entities from the resurrect candidate list.
//
//	This function will basically prune all entities that no longer exist on the
//	world.
//
void MastermindModuleInstance::RemoveInvalidResurrectEntitiesHelper()
{
	RessurectEntities::iterator entityIter = m_ResurrectEntities.begin();
	while (entityIter != m_ResurrectEntities.end())
	{
		if (gEnv->pEntitySystem->GetEntity(entityIter->m_EntityID) == NULL)
		{
			entityIter = m_ResurrectEntities.erase(entityIter);
		}
		else
		{
			++entityIter;
		}
	}
}


// ============================================================================
//	Select a husk formation by its ID.
//
//	In:		The husk formation mode ID (fmUndefined is invalid!)
//
void MastermindModuleInstance::SelectFormationMode(FormationMode formationMode)
{
	switch (formationMode)
	{
	case fmUndefined:
	default:
		// We should never get here!
		assert(false);
		return;

	case fmDefensive:
		SelectDefensiveFormation();		
		break;

	case fmOffsensive:
		SelectOffensiveFormation();
		break;
	}
}


// ============================================================================
//	Update all the husk formation positions.
//
//	In:		The amount of time that has elapsed since the last frame (>= 0.0f)
//			(in seconds).
//
void MastermindModuleInstance::UpdateAllHuskFormationPositions(float frameTime)
{
	m_LockCounter++;
	UpdateAllHuskFormationPositionsHelper(frameTime);
	m_LockCounter--;
}


// ============================================================================
//	Helper: Update all the husk formation positions.
//
//	In:		The amount of time that has elapsed since the last frame (>= 0.0f)
//			(in seconds).
//
void MastermindModuleInstance::UpdateAllHuskFormationPositionsHelper(float frameTime)
{
	IEntity* ownerEntity = GetEntity();
	assert(ownerEntity != NULL);

	// For now the formation will just slowly rotate around the Mastermind.
	m_HuskFormationRotationAngle =
		WrapAngle(m_HuskFormationRotationAngle + (m_FormationAngularVelocity * frameTime));
	
	// Construct a 'plane' in which the husks will be circling.	
	Vec3 rightNormal = ownerEntity->GetRotation().GetColumn0();
	Vec3 backwardNormal = -(ownerEntity->GetRotation().GetColumn1());
	Vec3 upNormal = ownerEntity->GetRotation().GetColumn2();
	Vec3 backDownNormal = backwardNormal - upNormal;
	backDownNormal.SetLength(cry_sinf(m_HuskFormationPlaneAngle));
	Vec3 rotationPlaneNormal = upNormal + backDownNormal;
	rotationPlaneNormal.NormalizeSafe();
	Vec3 planeNormalU = rightNormal;
	Vec3 planeNormalV = rightNormal.Cross(rotationPlaneNormal);

	Vec3 formationPivotPos = ownerEntity->GetWorldPos() + m_HuskFormationPivotOffset;

	size_t formationIndex = 0;
	size_t formationSize = m_HuskEntityIDs.size();
	SlavedEntityIDs::const_iterator huskIter;
	SlavedEntityIDs::const_iterator huskEndIter = m_HuskEntityIDs.end();
	for (huskIter = m_HuskEntityIDs.begin() ; huskIter != huskEndIter ; ++huskIter)
	{
		UpdateHuskFormationPose(
			gEnv->pEntitySystem->GetEntity(*huskIter),
			frameTime,
			formationIndex, formationSize, 
			formationPivotPos, planeNormalU, planeNormalV);
		formationIndex++;
	}
}


// ============================================================================
//	Update the formation pose of a husk.
//
//	In:		Pointer to the husk entity (NULL will abort).
//	In:		The amount of time that has elapsed since the last frame (>= 0.0f)
//			(in seconds).
//	In:		The formation index of this husk (>= 0).
//	In:		The total amount of husks in the formation (>= 0).
//	In:		The pivot position of the formation (in world-space).
//
void MastermindModuleInstance::UpdateHuskFormationPose(
	IEntity* entity, float frameTime,
	size_t formationIndex, size_t formationSize,
	const Vec3& formationPivotPos,
	const Vec3& planeNormalU, const Vec3& planeNormalV)
{
	if (entity == NULL)
	{
		return;
	}	

	// The target position is basically circling around the formation pivot position.
	const float angle = WrapAngle(
		m_HuskFormationRotationAngle + (gf_PI2 * ((float)formationIndex)/((float)formationSize)));
	const Vec3 formationOffset = ((planeNormalU * cry_cosf(angle)) + (planeNormalV * cry_sinf(angle))) * m_HuskFormationRadius;
	const Vec3 destPos = formationPivotPos + formationOffset;
	const Vec3 oldPos = entity->GetPos();
	const Vec3 towardsDestDir = destPos - oldPos;
	const Vec3 newPos = oldPos + (towardsDestDir * frameTime * m_HuskMoveSmoothingFactor);
	
	Quat newOrientation;
	switch (m_FormationMode)
	{
	case fmUndefined:
	default:
		// We should never get here!
		assert(false);
		return;

	case fmDefensive:
		RetrieveDefensiveHuskFormationOrientation(&newOrientation, entity, newPos, formationPivotPos);		
		entity->GetAI()->CastToIAIActor()->SetLookAtDir(newOrientation.GetColumn1());
		break;

	case fmOffsensive:		
		RetrieveOffensiveHuskFormationOrientation(&newOrientation, entity, newPos);
		break;
	}

	//Matrix34 worldMatrix;
	//worldMatrix.Set(Vec3Constants<float>::fVec3_One, newOrientation, newPos);	
	//entity->SetWorldTM(worldMatrix);	
	// For now we'll do it like this because the AI system will turn the Husk
	// (in the XY plane only), and the gun IK will not go all haywire.
	entity->SetPos(newPos);		
}


// ============================================================================
//	Retrieve the defensive formation orientation for a Husk.
//
//	Out:	The resulting orientation.
//	In:		The Husk entity (NULL is invalid!)
//	In:		The new formation position of the Husk (in world-space).
//	In:		The formation pivot position around which the Husks circle (in 
//			world-space).
//	
void MastermindModuleInstance::RetrieveDefensiveHuskFormationOrientation(
	Quat *resultOrientation, 
	const IEntity* entity, 
	const Vec3& newPos, const Vec3& formationPivotPos) const
{
	*resultOrientation = entity->GetRotation();

	// Construct an outwards aimed vector in the XY plane.
	Vec3 bodyDir =  newPos - formationPivotPos;
	bodyDir.z = 0.0f;
	if (bodyDir.NormalizeSafe() <= 0.0f)
	{	// Do not rotate when the Husk is accidentally at the exact same position
		// as the formation pivot (or above/below it).
		return;	
	}

	resultOrientation->SetRotationVDir(bodyDir);
}



// ============================================================================
//	Retrieve the defensive formation orientation for a Husk.
//
//	Out:	The resulting orientation.
//	In:		The Husk entity (NULL is invalid!)
//	In:		The new formation position of the Husk (in world-space).
//	
void MastermindModuleInstance::RetrieveOffensiveHuskFormationOrientation(
	Quat *resultOrientation, 
	IEntity* entity, 
	const Vec3& newPos) const
{
	*resultOrientation = entity->GetRotation();

	IAIObject* aiObject = entity->GetAI();
	assert(aiObject != NULL);	
	IAIActor* aiActor = aiObject->CastToIAIActor();
	if (aiActor == NULL)
	{
		return;
	}
	
	// Always aim for the current attention target.
	IAIObject* attentationTarget = aiActor->GetAttentionTarget();
	if (attentationTarget == NULL)
	{	// If there is no target then just don't rotate.
		return;
	}
	Vec3 bodyDir =  attentationTarget->GetPos() - newPos;	
	if (bodyDir.NormalizeSafe() <= 0.0f)
	{	// Do not rotate when the Husk is accidentally at the exact same position
		// as the target.
		return;	
	}

	resultOrientation->SetRotationVDir(bodyDir);
}


// ============================================================================
//	Helper: Kill all slave husks.
//
void MastermindModuleInstance::KillAllSlaveHusksHelper()
{
	IActorSystem* actorSystem = g_pGame->GetIGameFramework()->GetIActorSystem();
	assert(actorSystem != NULL);
	
	IActor* actorInterface;
	CActor* actor;
	SlavedEntityIDs::const_iterator entityIter;
	SlavedEntityIDs::const_iterator entityIterEnd = m_HuskEntityIDs.end();
	for (entityIter = m_HuskEntityIDs.begin() ; entityIter != entityIterEnd ; ++entityIter)
	{
		actorInterface = actorSystem->GetActor(*entityIter);		
		if (actorInterface != NULL)
		{
			actor = (CActor*)actorInterface;

			ForcedKillActor(actor);			
		}
	}
}


// ============================================================================
//	Forcefully kill an actor.
//
//	In,out:		The actor (NULL will abort!)
//
void MastermindModuleInstance::ForcedKillActor(CActor *actor)
{
	if (actor == NULL)
	{
		return;
	}

	// Note: this doesn't seem to work entirely because it bypasses
	// all the death reactions and such.
	//HitInfo hitInfo;
	//hitInfo.type = g_pGame->GetGameRules()->GetHitTypeId("event");
	//g_pGame->GetGameRules()->KillPlayer(actorInterface, false, true, hitInfo);			

	// This is how the 'Kill' Flow Graph node does it (oh dear).
	const EntityId clientId = actor->GetEntityId();
	//g_pGame->GetIGameFramework()->GetClientActorId();
	HitInfo suicideInfo(clientId, clientId, clientId,
		10000, 0, 0, -1, CGameRules::EHitType::Punish, ZERO, ZERO, ZERO);
	g_pGame->GetGameRules()->ClientHit(suicideInfo);

	// TODO: Ugly hack to get rid of the Husks for now (otherwise they will just hang 
	// there in the air). This is just for demo purpose.
	actor->GetEntity()->Hide(true);
}


// ============================================================================
//	Set the resurrection state.
//
//	In:		The resurrection state.
//
void MastermindModuleInstance::SetResurrectionState(ResurrectionState newState)
{
	if (m_ResurrectionState == newState)
	{
		return;
	}

	m_ResurrectionState = newState;
	switch (newState)
	{
	case rsIdle:
		break;

	case rsLifting:
		m_ResurrectionTimer = g_HuskResurrectionLiftingDuration;
		break;

	case rsFormingHusks:
		m_ResurrectionTimer = g_HuskResurrectionHuskingDuration;
		break;

	case rsHusksFormed:
		// We have to initiate the actual spawning from within the behavior
		// of the mastermind because we cannot do it directly from within
		// this module (this will trigger asserts when spawning a husk
		// because it appears to the module system that the new entity is
		// performing module entries/exits; even though it actually isn't).
		// Anyways, this will also serve as a nice animation synchronization
		// opportunity with the goal-pipe that is currently running on the
		// mastermind.
		break;

	case rsSpawnHusks:
		ResurrectAllEntities();
		SetResurrectionState(rsPullIntoFormation);
		break;

	case rsPullIntoFormation:
		m_HuskMoveSmoothingFactor = 0.0f;
		m_ResurrectionTimer = g_HuskResurrectionPullIntoFormationDuration;
		break;

	case rsInFormation:
		break;

	default:
		// We should never get here!
		assert(false);
		break;
	}
}


// ============================================================================
//	Progress the resurrection sequence.
//
//	In:		The amount of time that has elapsed since the last frame (>= 0.0f)
//			(in seconds).
//
void MastermindModuleInstance::ProgressResurrection(const float frameTime)
{
	RemoveInvalidResurrectEntities();

	m_LockCounter++;
	ProgressResurrectionHelper(frameTime);
	m_LockCounter--;
}


// ============================================================================
//	Helper: Progress the resurrection sequence.
//
//	In:		The amount of time that has elapsed since the last frame (>= 0.0f)
//			(in seconds).
//
void MastermindModuleInstance::ProgressResurrectionHelper(const float frameTime)
{
	switch (m_ResurrectionState)
	{
	case rsIdle:
		break;

	case rsLifting:
		ProgressResurrectionLifting(frameTime);
		break;

	case rsFormingHusks:
		ProgressResurrectionFormingHusks(frameTime);
		break;

	case rsHusksFormed:
		ProgressResurrectionHusksFormed();		
		break;

	case rsSpawnHusks:
		break;

	case rsPullIntoFormation:
		ProgressResurrectionPullIntoFormation(frameTime);
		break;

	case rsInFormation:
		ProgressResurrectionInFormation(frameTime);
		break;

	default:
		// We should never get here!
		assert(false);
		break;
	}
}


// ============================================================================
//	Progress resurrection: Lifting the bodies from the floor.
//
//	In:		The amount of time that has elapsed since the last frame (>= 0.0f)
//			(in seconds).
//
void MastermindModuleInstance::ProgressResurrectionLifting(const float frameTime)
{
	const bool lastFrameFlag = (frameTime >= m_ResurrectionTimer);
	float timeRemaining;
	if (lastFrameFlag)
	{
		timeRemaining = m_ResurrectionTimer;
		m_ResurrectionTimer = 0.0f;
	}
	else
	{
		timeRemaining = frameTime;
		m_ResurrectionTimer -= frameTime;
	}

	const float liftingDistance = g_HuskResurrectionLifingSpeed * frameTime;
	IEntity* entity;
	Vec3 entityPos;
	RessurectEntities::iterator entityIter;
	RessurectEntities::iterator entityIterEnd = m_ResurrectEntities.end();
	for (entityIter = m_ResurrectEntities.begin() ; entityIter != entityIterEnd ; ++entityIter)
	{
		entity = gEnv->pEntitySystem->GetEntity(entityIter->m_EntityID);
		if (entity != NULL)
		{
			entityPos = entity->GetPos() + Vec3(0.0f, 0.0f, liftingDistance);
			entityIter->m_WorldPos = entityPos;						
			entity->SetPos(entityPos);
		}
	}

	if (lastFrameFlag)
	{
		SetResurrectionState(rsFormingHusks);
	}
}


// ============================================================================
//	Progress resurrection: Forming the husks.
//
//	In:		The amount of time that has elapsed since the last frame (>= 0.0f)
//			(in seconds).
//
void MastermindModuleInstance::ProgressResurrectionFormingHusks(const float frameTime)
{
	// This is a bit of an ugly way to keep the bodies up into the air.
	FixateResurrectEntityPositions();

	const bool lastFrameFlag = (frameTime >= m_ResurrectionTimer);
	if (!lastFrameFlag)
	{
		m_ResurrectionTimer -= frameTime;
	}
	else
	{
		SetResurrectionState(rsHusksFormed);			
	}	
}


// ============================================================================
//	Progress resurrection: Husks have been formed.
//
void MastermindModuleInstance::ProgressResurrectionHusksFormed()
{
	// Waiting for the go-ahead signal from the mastermind behavior here.

	// This is a bit of an ugly way to keep the bodies up into the air.
	FixateResurrectEntityPositions();
}


// ============================================================================
//	Progress resurrection: Pulling the husks into formation.
//
//	In:		The amount of time that has elapsed since the last frame (>= 0.0f)
//			(in seconds).
//
void MastermindModuleInstance::ProgressResurrectionPullIntoFormation(const float frameTime)
{
	m_ResurrectionTimer -= frameTime;
	float timePassed = g_HuskResurrectionPullIntoFormationDuration - m_ResurrectionTimer;
	const bool lastFrameFlag = (timePassed >= g_HuskResurrectionPullIntoFormationDuration);
	if (lastFrameFlag)
	{
		timePassed = g_HuskResurrectionPullIntoFormationDuration;
	}
	const float interpol = timePassed / g_HuskResurrectionPullIntoFormationDuration;

	m_HuskMoveSmoothingFactor = interpol * g_HuskFormationMaxSmoothingFactor;

	UpdateAllHuskFormationPositions(frameTime);

	if (lastFrameFlag)
	{
		SetResurrectionState(rsInFormation);
	}
}


// ============================================================================
//	Progress resurrection: Husks are in formation.
//
//	In:		The amount of time that has elapsed since the last frame (>= 0.0f)
//			(in seconds).
//
void MastermindModuleInstance::ProgressResurrectionInFormation(const float frameTime)
{
	if (m_HuskEntityIDs.empty())
	{
		SetResurrectionState(rsIdle);
	}

	UpdateAllHuskFormationPositions(frameTime);
}


// ============================================================================
//	Get the shield entity.
//
//	Returns:	The shield entity or NULL if none exists yet.
//
IEntity* MastermindModuleInstance::GetShieldEntity() const
{
	if (m_ShieldEntityID == (EntityId)0)
	{
		return NULL;
	}
	return gEnv->pEntitySystem->GetEntity(m_ShieldEntityID);
}


// ============================================================================
//	Spawn a shield entity.
//
//	Returns:	The shield entity or NULL if none could be spawned.
//
IEntity* MastermindModuleInstance::SpawnShieldEntity()
{
	assert(m_ShieldEntityID == (EntityId)0);

	IEntity* ownerEntity = GetEntity();

	CryFixedStringT<30> newEntityName;
	newEntityName.Format("MastermindShield%u", gGameAIEnv.mastermindModule->GenerateShieldNameID());

	SEntitySpawnParams spawnParams;	
	spawnParams.nFlags = 0;
	spawnParams.bStaticEntityId = false;			
	spawnParams.pArchetype = gEnv->pEntitySystem->LoadEntityArchetype("Characters.Mastermind.DeflectorShield");
	CRY_ASSERT_MESSAGE(spawnParams.pArchetype != NULL, "Unable to load Mastermind Shield entity archetype!");
	spawnParams.vPosition = ownerEntity->GetPos();
	spawnParams.qRotation = ownerEntity->GetRotation();
	spawnParams.sName = newEntityName.c_str();
	spawnParams.pUserData = 0;

	IEntity	*newEntity = gEnv->pEntitySystem->SpawnEntity(spawnParams);
	CRY_ASSERT_MESSAGE((newEntity != NULL), "Failed to spawn Shield Entity!");

	// Configure the deflector shield.
	CGameObject* gameObject = (CGameObject*)(newEntity->GetProxy(ENTITY_PROXY_USER));
	CDeflectorShield* deflectorShield = (CDeflectorShield*)
		(gameObject->QueryExtension(g_DeflectorShieldGameObjectExtensionName));
	CRY_ASSERT_MESSAGE(deflectorShield != NULL, 
		"Mastermind could not acquire deflector shield game object instance!");

	// Make sure the mastermind and any of its husks can actually shoot 
	// through its own shield.	
	int ownerEntityGroupID = ownerEntity->GetAI()->GetGroupId();
#if !defined(_RELEASE)
	if (ownerEntityGroupID == 0)
	{
		CryLog("Mastermind %s has group ID 0 and thus we cannot properly setup its deflector shield!",
			ownerEntity->GetName());
	}
#endif	
	deflectorShield->SetNonDeflectedOwnerGroup(ownerEntityGroupID);

	// Allow any physical objects to pass through the shield (but deflect any hostile
	// projectiles!) We need to do this because we are positioning the shield by directly
	// manipulating the shield entity's world-space position. The physics system doesn't
	// like that when we suddenly intersect other dynamic entities (such as the mastermind
	// and/or its husks).
	deflectorShield->SetPhysicsCollisionResponse(false);

	return newEntity;
}


// ============================================================================
//	Purge the shield entity (if needed).
//
void MastermindModuleInstance::PurgeShieldEntity()
{
	if (m_ShieldEntityID == (EntityId)0)
	{
		return;
	}

	gEnv->pEntitySystem->RemoveEntity(m_ShieldEntityID);
	m_ShieldEntityID = (EntityId)0;
}


// ============================================================================
//	Align the shield entity with the owner mastermind entity.
//
void MastermindModuleInstance::AlignShieldWithOwner()
{
	IEntity* shieldEntity = GetShieldEntity();
	if (shieldEntity == NULL)
	{
		return;
	}
	const IEntity* ownerEntity = GetEntity();

	Matrix34 worldTransform = ownerEntity->GetWorldTM();

	const Vec3& ownerForwardNormal = worldTransform.GetColumn1();
	const Vec3& ownerUpNormal = worldTransform.GetColumn2();
	worldTransform.SetTranslation(
		worldTransform.GetTranslation() + 
		(ownerForwardNormal * m_Config.m_ShieldForwardOffset) + 
		(ownerUpNormal      * m_Config.m_ShieldUpwardOffset)      );

	shieldEntity->SetWorldTM(worldTransform);
}


// ============================================================================
//	Progress enemy spotting notifications.
//
//	In:		The current time-stamp (>= 0.0f) (in seconds).
//
void MastermindModuleInstance::ProgressEnemySpottingNotifications(const float now)
{
	if (m_EnemySeenSuppressTime >= 0.0f)
	{
		if (m_EnemySeenSuppressTime < now)
		{
			m_EnemySeenSuppressTime = -1.0f;
		}
	}

	if (m_EnemySeenNotiftyTime < 0.0f)
	{
		return;
	}

	// Make sure the entity IDs remain valid (in case it died in the mean
	// time or something).	
	if (m_EnemySpotterEntityID != (EntityId)0)
	{
		if (gEnv->pEntitySystem->GetEntity(m_EnemySpotterEntityID) == NULL)
		{	// Note: we will not suppress the notification itself because 
			// this might be exploitable (e.g.: player repeatedly killing
			// a drone that spotted him without the mastermind ever noticing).
			// The notification will just be 'anonymous' now.
			m_EnemySpotterEntityID = (EntityId)0;
		}
	}
	if (gEnv->pEntitySystem->GetEntity(m_SpottedEntityID) == NULL)
	{	// If the spotted enemy died then just abort the notification
		// (passing in an invalid ID will cause Lua script errors later on!)
		ResetEnemySpottedNotification();
		return;
	}
	
	if (m_EnemySeenNotiftyTime > now)
	{
		return;
	}

	// Deliver the actual notification.
	NotifyMastermindSlaveHuskSawTarget(m_SpottedEntityID, m_SpottedEnemyPos);
	NotifyAllSlaveHuskSawTarget(m_EnemySpotterEntityID, m_SpottedEntityID, m_SpottedEnemyPos);
	ResetEnemySpottedNotification();

	// And impose a delay, just to make sure we don't cascade when other Husks
	// suddenly spot the target as well.
	m_EnemySeenSuppressTime = now + g_HsuKPerceptionNotifyCascadeDelay;
}


// ============================================================================
//	Reset the enemy spotted notification.
//
void MastermindModuleInstance::ResetEnemySpottedNotification()
{
	m_EnemySeenNotiftyTime = -1.0f;
	m_EnemySpotterEntityID = (EntityId)0;
	m_SpottedEntityID = (EntityId)0;
	m_SpottedEnemyPos.zero();
}


// ============================================================================
//	Notify the Mastermind that a target has been seen by one of the Husk slaves.
//
//	In:		The ID of the spotted entity (must be valid!)
//	In:		The position where the target was spotted at (in world-space).
//
void MastermindModuleInstance::NotifyMastermindSlaveHuskSawTarget(
	EntityId seenEntityID, const Vec3& seenEntityPos)
{
	IAISignalExtraData* extraData = gEnv->pAISystem->CreateSignalExtraData();	
	extraData->nID = seenEntityID;
	extraData->point = seenEntityPos;
	SendSignal("OnHuskSawTarget", extraData);
}


// ============================================================================
//	Notify all the slave Husks that a target has been seen by one of the Husk slaves.
//
//	In:		The ID of the spotted entity (must be valid!)
//	In:		The ID of the husk entity that spotted the target (must be valid!)
//	In:		The position where the target was spotted at (in world-space).
//
void MastermindModuleInstance::NotifyAllSlaveHuskSawTarget(
	EntityId spotterHuskEntityID, EntityId seenEntityID, const Vec3& seenEntityPos)
{
	m_LockCounter++;
	NotifyAllSlaveHuskSawTargetHelper(spotterHuskEntityID, seenEntityID, seenEntityPos);
	m_LockCounter--;
}


// ============================================================================
//	Helper: Notify all the slave Husks that a target has been seen by one of 
//	the Husk slaves.
//
//	In:		The ID of the spotted entity (must be valid!)
//	In:		The ID of the husk entity that spotted the target (must be valid!)
//	In:		The position where the target was spotted at (in world-space).
//
void MastermindModuleInstance::NotifyAllSlaveHuskSawTargetHelper(
	EntityId spotterHuskEntityID, EntityId seenEntityID, const Vec3& seenEntityPos)
{
	EntityId huskEntityID;
	SlavedEntityIDs::const_iterator huskIter;
	SlavedEntityIDs::const_iterator huskEndIter = m_HuskEntityIDs.end();
	for (huskIter = m_HuskEntityIDs.begin() ; huskIter != huskEndIter ; ++huskIter)
	{
		huskEntityID = *huskIter;
		if (huskEntityID != spotterHuskEntityID)
		{
			Agent huskAgent(*huskIter);
			IAISignalExtraData* extraData = gEnv->pAISystem->CreateSignalExtraData();	
			extraData->nID = seenEntityID;
			extraData->point = seenEntityPos;
			huskAgent.SetSignal(AISIGNAL_DEFAULT, "OnOtherHuskSawTarget", extraData);
		}
	}
}


#if !defined(_RELEASE)


// ============================================================================
//	Debug rendering.
//
void MastermindModuleInstance::DebugRender()
{	
	IPersistantDebug* debugRenderer = gEnv->pGame->GetIGameFramework()->GetIPersistantDebug();
	assert(debugRenderer != NULL);

	debugRenderer->Begin("MastermindModuleInstanceDebugGraphics", false);


	RessurectEntities::const_iterator entityIter;
	RessurectEntities::const_iterator entityIterEnd;

	// Render lines to all resurrection target (if needed).
	switch (m_ResurrectionState)
	{
	case rsIdle:
	case rsInFormation:
		break;

	case rsLifting:
	case rsFormingHusks:
	case rsHusksFormed:
	case rsSpawnHusks:
	case rsPullIntoFormation:		
		{
			Vec3 abovePivotPos = GetEntity()->GetWorldPos();
			abovePivotPos.z += 1.0f;			
			entityIterEnd = m_ResurrectEntities.end();
			for (entityIter = m_ResurrectEntities.begin() ; entityIter != entityIterEnd ; ++entityIter)
			{
				const IEntity* entity = gEnv->pEntitySystem->GetEntity(entityIter->m_EntityID);
				if (entity != NULL)
				{
					Vec3 dir = entity->GetWorldPos() - abovePivotPos;
					float len = dir.GetLength();			
					debugRenderer->AddCylinder(abovePivotPos + (dir * 0.5f), dir.GetNormalized(), 0.5f, len, g_HuskResurrectionDebugColor, 0.1f);
				}
			}
		}
		break;

	default:
		assert(false);
		break;
	}

	// Specific state animations.
	entityIterEnd = m_ResurrectEntities.end();
	for (entityIter = m_ResurrectEntities.begin() ; entityIter != entityIterEnd ; ++entityIter)
	{
		const IEntity* entity = gEnv->pEntitySystem->GetEntity(entityIter->m_EntityID);
		if (entity != NULL)
		{
			switch (m_ResurrectionState)
			{
			case rsIdle:
				break;

			case rsLifting:
				break;

			case rsFormingHusks:
			case rsHusksFormed:
				{
					const float maxSphereRadius = 0.75f;
					float timePassed = g_HuskResurrectionHuskingDuration - m_ResurrectionTimer;
					debugRenderer->AddSphere(entity->GetPos(), timePassed * maxSphereRadius, g_HuskResurrectionDebugColor, 0.1f);
				}
				break;			

			case rsSpawnHusks:

			case rsPullIntoFormation:
				break;

			case rsInFormation:
				break;

			default:
				assert(false);
				break;
			}
		}
	}
}


#endif // !defined(_RELEASE)


// ============================================================================
// ============================================================================
// ============================================================================
//
// -- MastermindModule -- MastermindModule -- MastermindModule --
//
// ============================================================================
// ============================================================================
// ============================================================================


MastermindModule::MastermindModule() :
	m_HuskNameIDGen(0u)
	, m_ShieldNameIDGen(0u)
	, m_ResurrectEntities()
{	
	m_DeathListener.Init(this);
	m_ResurrectEntities.reserve(16);
}


MastermindModule::~MastermindModule()
{
	// Nothing to do here.
}


void MastermindModule::Reset(bool bUnload)
{
	BaseClass::Reset(bUnload);

#if defined(MASTERMIND_EDITOR_ONLY)
	// We have to manually remove all the husks that were spawned while running 
	// the game in the editor, or else they will automatically become part of 
	// the map data!
	if (gEnv->IsEditor())
	{
		EditorOnlyRemoveAllSpawnedEntities();
	}
#endif

	m_HuskNameIDGen = 0u;
	m_ShieldNameIDGen = 0u;
	m_ResurrectEntities.clear();
}


// ============================================================================
//	Find the mastermind module instance to which a husk entity is registered
//	as a slave.
//
//	In:		The entity ID of the husk (0 will abort).
//
//	Returns:	A pointer to the corresponding module instance or NULL if
//				none could be found.
//
MastermindModuleInstance* MastermindModule::FindInstanceToWhichHuskIsSlaved(EntityId huskEntityID)
{
	if (huskEntityID == (EntityId)0)
	{
		return NULL;
	}
	
	Instances::iterator instanceIter;
	Instances::iterator instanceIterEnd = m_running->end();
	for (instanceIter == m_running->begin() ; instanceIter != instanceIterEnd ; ++instanceIter)
	{
		InstanceID instanceID = instanceIter->second;
		MastermindModuleInstance* instance = GetInstanceFromID(instanceID);		
		if (instance->IsHuskRegisteredAsSlave(huskEntityID))
		{
			return instance;
		}
	}

	return NULL;
}


// ============================================================================
//	Generate a new unique husk name ID.
//
//	Returns:	The new name ID (>= 0).
//
unsigned int MastermindModule::GenerateHuskNameID()
{
	unsigned int newID = m_HuskNameIDGen;
	m_HuskNameIDGen++;
	assert(m_HuskNameIDGen != (0u));	// Too many IDs generated?!
	return newID;
}


// ============================================================================
//	Generate a new unique husk name ID.
//
//	Returns:	The new name ID (>= 0).
//
unsigned int MastermindModule::GenerateShieldNameID()
{
	unsigned int newID = m_ShieldNameIDGen;
	m_ShieldNameIDGen++;
	assert(m_ShieldNameIDGen != (0u));	// Too many IDs generated?!
	return newID;
}


// ============================================================================
//	Get the resurrection candidates.
//
//	Returns:	The resurrection candidates.
//
ResurrectableCandidateEntityInfos& MastermindModule::GetResurrectionCandidates()
{
	return m_ResurrectEntities;
}


// ============================================================================
//	Unregister a resurrection candidate.
//
//	In:		The entity ID of the candidate that should be unregistered.
//
void MastermindModule::UnregisterResurrectionCandidate(EntityId entityID)
{
	ResurrectableCandidateEntityInfos::iterator iter = FindResurrectEntity(entityID);
	if (iter == m_ResurrectEntities.end())
	{
		return;
	}

	m_ResurrectEntities.erase(iter);
}


#if defined(MASTERMIND_EDITOR_ONLY)


// ============================================================================
//	Report that an item that has been spawned and should be removed in the editor
//	when the game is aborted.
//
void MastermindModule::EditorOnlyReportSpawnedEntity(EntityId entityID)
{
	if (entityID != (EntityId)0)
	{
		stl::push_back_unique(m_EditorOnlySpawnedEntityIDs, entityID);
	}	
}


// ============================================================================
//	Remove all spawned entities.
//
void MastermindModule::EditorOnlyRemoveAllSpawnedEntities()
{	
	EntityId entityID;
	IInventory* inventoryInterface;
	EditorOnlySpawnedEntityIDs::const_iterator entityIter;
	EditorOnlySpawnedEntityIDs::const_iterator entityIterEnd = m_EditorOnlySpawnedEntityIDs.end();	
	for (entityIter = m_EditorOnlySpawnedEntityIDs.begin() ; entityIter != entityIterEnd ; ++entityIter)
	{
		entityID = *entityIter;

		IActor* actorInterface = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entityID);
		if (actorInterface != NULL)
		{
			inventoryInterface = actorInterface->GetInventory();
			if (inventoryInterface != NULL)
			{
				inventoryInterface->Destroy();
			}
		}

		gEnv->pEntitySystem->RemoveEntity(entityID);
	}

	m_EditorOnlySpawnedEntityIDs.resize(0);
}


#endif // defined(MASTERMIND_EDITOR_ONLY)


// ============================================================================
//	Report an entity that can potentially be resurrected.
//
//	In:		The entity ID (0 is invalid!)
//
void MastermindModule::ReportResurrectableEntity(EntityId deadEntityID)
{
	assert(deadEntityID != (EntityId)0);

	if (FindResurrectEntity(deadEntityID) != m_ResurrectEntities.end())
	{
		return;
	}

	m_ResurrectEntities.push_back(ResurrectableCandidateEntityInfo(deadEntityID));
}


// ============================================================================
//	Find a resurrectable candidate entity.
//
//	In:		The ID of the target entity (0 will abort!)
//
//	Returns:	The iterator of the found entity in m_ResurrectEntities; or
//				m_ResurrectEntities.end() if it could not be found.
//
ResurrectableCandidateEntityInfos::iterator MastermindModule::FindResurrectEntity(EntityId entityId)
{
	if (entityId == (EntityId)0)
	{
		return m_ResurrectEntities.end();
	}

	ResurrectableCandidateEntityInfos::iterator iter;
	ResurrectableCandidateEntityInfos::iterator iterEnd = m_ResurrectEntities.end();

	for (iter = m_ResurrectEntities.begin() ; iter != iterEnd ; ++iter)
	{
		if (iter->m_EntityID == entityId)
		{
			return iter;
		}
	}

	return m_ResurrectEntities.end();
}


} // namespace Mastermind
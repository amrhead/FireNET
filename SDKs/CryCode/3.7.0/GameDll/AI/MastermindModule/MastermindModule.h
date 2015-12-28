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


#pragma once

#ifndef MastermindModule_h
#define MastermindModule_h

#include "primitives.h"

#include "..\GameAIHelpers.h"

#include "MastermindModuleShared.h"

#include "MastermindModuleDeathListener.h"


// When defined we will compile code that should only be present in the editor
#if !defined(_RELEASE)
#define MASTERMIND_EDITOR_ONLY
#endif


// Forward declarations:
class Agent;
class CActor;


namespace Mastermind
{

// Forward declarations:
class MastermindModuleInstanceConfig;


// Information on an entity that we are resurrecting or controlling as a Husk.
class ModuleInstanceEntityInfo
{
public:
	// The entity ID (or 0 if undefined).
	EntityId							m_EntityID;

	// The position of the entity in world-space.
	Vec3								m_WorldPos;


public:
	ModuleInstanceEntityInfo();
	ModuleInstanceEntityInfo(const EntityId entityID, const Vec3& worldPos);
};


// Interfacing between actors and the Resurrection module they are registered to.
class MastermindModuleInstance : public CGameAIInstanceBase
{
public:
	typedef CGameAIInstanceBase BaseClass;

public:
	MastermindModuleInstance();
	virtual ~MastermindModuleInstance();

	// Life-cycle:
	void								SetupMastermind(const MastermindModuleInstanceConfig& config);
	virtual void                        Update(float frameTime);
	virtual void						Destroy();

	// Resurrection:
	IEntity*							GetClosestResurrectEntity() const;
	void								StartResurrection();
	void								FinalizeResurrection();
	void								AbortResurrection();

	// Slave management:
	void								RegisterSlaveHusk(IEntity* slaveEntity);
	void								UnregisterSlaveHusk(EntityId slaveEntityID);
	bool								IsHuskRegisteredAsSlave(const EntityId entityID);
	void								KillAllSlaveHusks();
	bool								IsResurrecting() const;
	void								NotifySlaveHuskTargetSeen(EntityId slaveEntityID, EntityId seenEntityID);

	// Formation management:
	void								SelectFormationMode(const char *modeName);
	void								SelectDefensiveFormation();
	void								SelectOffensiveFormation();

	// Shield control:
	void								EnableShield();
	void								DisableShield();


private:
	// The various states in the resurrection sequence.
	enum ResurrectionState
	{
		rsIdle = 0,
		rsLifting,				// Lifting the dead bodies from the ground.
		rsFormingHusks,			// Forming the husks.
		rsHusksFormed,			// The husks are formed (visually) and we are waiting for a go-ahead signal
								// from the mastermind entity to perform the actual spawning.
		rsSpawnHusks,			// Actual spawning of the husks (this will remove the bodies).
		rsPullIntoFormation,	// Pulling the husks into the formation
		rsInFormation			// The husks are active and in formation.
	};


	// The various Husk formation modes.
	enum FormationMode
	{
		fmUndefined = 0,		// Only used for debugging and error handling.
		fmDefensive,
		fmOffsensive	
	};


private:
	// The instance configuration.
	MastermindModuleInstanceConfig		m_Config;

	// All the entity IDs of resurrectable agents nearby.
	typedef std::vector<ModuleInstanceEntityInfo> RessurectEntities;
	RessurectEntities					m_ResurrectEntities;

	// True if we signaled the owner that ressurectable targets are nearby;
	// otherwise false.
	bool								m_ResurrectionTargetsSignaledFlag;

	// The current resurrection state (RessurectionState.Idle if not resurrecting).
	ResurrectionState					m_ResurrectionState;

	// The time index at which we may perform the next resurrection tracking.
	float								m_NextResurrectTrackingTime;

	// All the entity IDs of husks that are slaved to the mastermind.
	typedef std::vector<EntityId> SlavedEntityIDs;
	SlavedEntityIDs						m_HuskEntityIDs;

	// We are not allowed to modify the module while this lock counter is > 0.
	// (Is used to make sure that iterators remain valid!)
	int									m_LockCounter;

	// The formation angular velocity (in radians / second) (the sign defines
	// the rotation direction).
	float								m_FormationAngularVelocity;

	// The offset angle of the plane in which the husks are located (>= 0.0f)
	// (in radians) (offset from the up vector).
	float								m_HuskFormationPlaneAngle;

	// The formation radius (>= 0.0f) (in meters).
	float								m_HuskFormationRadius;
	
	// The formation pivot offset (in world-space) (in meters).
	Vec3								m_HuskFormationPivotOffset;

	// The current husk formation rotation angle (in the range of [0.0f .. gf_PI2])
	// (in radians).
	float								m_HuskFormationRotationAngle;

	// The current husk movement smoothing factor (in the range of [0.0f .. 1.0f]).
	float								m_HuskMoveSmoothingFactor;
	
	// A generic resurrection timer (>= 0.0f) (in seconds).
	float								m_ResurrectionTimer;

	// True if the shield is enabled; otherwise false.
	bool								m_ShieldedFlag;

	// The entity ID of the shield (or 0 if none exists).
	EntityId							m_ShieldEntityID;

	// The current Husk formation mode (fmUndefined if not set yet).
	FormationMode						m_FormationMode;

	// The 'enemy seen by Husk' notification time (>= 0.0f).
	// If -1.0f then no notification is currently pending.
	// This will be used to impede a small delay so that the player has
	// time to attack the Mastermind in the back before it will
	// actually turn to face him again.
	float								m_EnemySeenNotiftyTime;

	// The entity ID of the slave Husk that saw the enemy (0 if none).
	EntityId							m_EnemySpotterEntityID;

	// The entity ID of the spotted enemy (0 if none).
	EntityId							m_SpottedEntityID;

	// The position at which the enemy was spotted by a slave Husk
	// (in world-space).
	Vec3								m_SpottedEnemyPos;

	// The time index at which we are allowed to respond to new 
	// 'enemy spotted' events (-1.0f if there is no suppression
	// should be applied).
	float								m_EnemySeenSuppressTime;


private:
	// Resurrectable entity management:
	void								TrackNearbyResurrectableEntities();
	void								SignalOwnerAboutResurrectionOpportunities();
	void								RetrieveSearchArea(primitives::sphere *searchSphere) const;
	bool								CanAgentBeResurrectedNow(const Agent& agent) const;	
	void								ResurrectAllEntities();
	void								ResurrectEntity(IEntity* sourceEntity, int husksAmount = 1);
	void								SpawnHusk(IEntity* sourceEntity, const Vec3& startPos, const Quat& startRotation);
	SlavedEntityIDs::iterator			FindSlavedEntityIter(const EntityId entityID);
	void								FixateResurrectEntityPositions();	
	void                                RemoveInvalidResurrectEntities();
	inline void                         RemoveInvalidResurrectEntitiesHelper();

	// Formation management:
	void								SelectFormationMode(FormationMode formationMode);

	// Slaved husk management:	
	void								UpdateAllHuskFormationPositions(float frameTime);
	inline void							UpdateAllHuskFormationPositionsHelper(float frameTime);
	void								UpdateHuskFormationPose(IEntity* entity, float frameTime, size_t formationIndex, size_t formationSize, const Vec3& formationPivotPos, const Vec3& planeNormalU, const Vec3& planeNormalV);
	void								RetrieveDefensiveHuskFormationOrientation(Quat *resultOrientation, const IEntity* entity, const Vec3& newPos, const Vec3& formationPivotPos) const;
	void								RetrieveOffensiveHuskFormationOrientation(Quat *resultOrientation, IEntity* entity, const Vec3& newPos) const;
	inline void							KillAllSlaveHusksHelper();	
	void								ForcedKillActor(CActor *actor);

	// Resurrection sequence control:
	void								SetResurrectionState(ResurrectionState newState);
	void								ProgressResurrection(const float frameTime);
	inline void							ProgressResurrectionHelper(const float frameTime);
	void								ProgressResurrectionLifting(const float frameTime);
	void								ProgressResurrectionFormingHusks(const float frameTime);
	void								ProgressResurrectionHusksFormed();
	void								ProgressResurrectionPullIntoFormation(const float frameTime);
	void								ProgressResurrectionInFormation(const float frameTime);

	// Shield control:
	IEntity*							GetShieldEntity() const;
	IEntity*							SpawnShieldEntity();
	void								PurgeShieldEntity();
	void								AlignShieldWithOwner();

	// Target tracking:
	void								ProgressEnemySpottingNotifications(const float now);
	void								ResetEnemySpottedNotification();
	void								NotifyMastermindSlaveHuskSawTarget(EntityId seenEntityID, const Vec3& seenEntityPos);
	void								NotifyAllSlaveHuskSawTarget(EntityId spotterHuskEntityID, EntityId seenEntityID, const Vec3& seenEntityPos);
	inline void							NotifyAllSlaveHuskSawTargetHelper(EntityId spotterHuskEntityID, EntityId seenEntityID, const Vec3& seenEntityPos);

#if !defined(_RETAIL)

	// Debugging:
	void								DebugRender();

#endif // !defined(_RETAIL)
};


// In this class we store information on entities that are resurrection candidates.
class ResurrectableCandidateEntityInfo
{
public:
	// The entity ID (or 0 if undefined).
	EntityId							m_EntityID;


public:
	ResurrectableCandidateEntityInfo();
	ResurrectableCandidateEntityInfo(const EntityId entityID);
};


typedef std::vector<ResurrectableCandidateEntityInfo> ResurrectableCandidateEntityInfos;


// The mastermind modules provides various services for the Mastermind agents.
class MastermindModule : public AIModuleWithInstanceUpdate<MastermindModule, MastermindModuleInstance, 16>
{
public:
	typedef AIModuleWithInstanceUpdate<MastermindModule, MastermindModuleInstance, 16> BaseClass;


public:
	MastermindModule();
	virtual ~MastermindModule();
		
	virtual const char*                 GetName() const { return "MastermindModule"; }		

	// Life-cycle:
	virtual void						Reset(bool bUnload);

	// Husk management:
	MastermindModuleInstance*			FindInstanceToWhichHuskIsSlaved(EntityId huskEntityID);
	unsigned int					    GenerateHuskNameID();

	// Shield management:
	unsigned int						GenerateShieldNameID();

	// Resurrection candidate management:
	ResurrectableCandidateEntityInfos&	GetResurrectionCandidates();
	void								UnregisterResurrectionCandidate(EntityId entityID);

#if defined(MASTERMIND_EDITOR_ONLY)

	// Editor only husk management:
	void								EditorOnlyReportSpawnedEntity(EntityId huskEntityID);
	void								EditorOnlyRemoveAllSpawnedEntities();

#endif // defined(MASTERMIND_EDITOR_ONLY)


protected:

	friend class MastermindModuleDeathListener;

	// Death reporting:
	void								ReportResurrectableEntity(EntityId deadEntityID);


private:

	// A unique husk entity name ID generator.
	unsigned int						m_HuskNameIDGen;

	// A unique mastermind shield entity name ID generator.
	unsigned int						m_ShieldNameIDGen;

	// The death listener module.
	MastermindModuleDeathListener		m_DeathListener;

	// The info on all the entities that are potentially resurrectable.	
	ResurrectableCandidateEntityInfos   m_ResurrectEntities;


#if defined(MASTERMIND_EDITOR_ONLY)

	// All the entities that should be removed in the editor when we exit the game mode.
	typedef std::vector<EntityId> EditorOnlySpawnedEntityIDs;
	EditorOnlySpawnedEntityIDs			m_EditorOnlySpawnedEntityIDs;

#endif // defined(MASTERMIND_EDITOR_ONLY)


private:

	// Resurrection candidate management:
	ResurrectableCandidateEntityInfos::iterator FindResurrectEntity(EntityId entityId);	
};


}; // namespace Mastermind


#endif // MastermindModule_h
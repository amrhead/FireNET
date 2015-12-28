/*************************************************************************
	Crytek Source File.
	Copyright (C), Crytek Studios, 2009.
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Description: 
	
	-------------------------------------------------------------------------
	History:
	- 07:09:2009  : Created by Ben Johnson
	- 08:09:2009  : Written by Colin Gulliver

*************************************************************************/

#ifndef _GAME_RULES_MP_DAMAGE_HANDLING_H_
#define _GAME_RULES_MP_DAMAGE_HANDLING_H_

#define MAX_BONES 100
#define MAX_PARTS 16

#if _MSC_VER > 1000
# pragma once
#endif

#include "GameRulesCommonDamageHandling.h"

class CWeapon;

class CGameRulesMPDamageHandling :	public CGameRulesCommonDamageHandling
{
public:
	CGameRulesMPDamageHandling();

	// IGameRulesDamageHandlingModule
	virtual ~CGameRulesMPDamageHandling();

	virtual void Init(XmlNodeRef xml);
	virtual void Update(float frameTime);

	virtual bool SvOnHit(const HitInfo &hitInfo);
	virtual bool SvOnHitScaled(const HitInfo &hitInfo);
	virtual void SvOnExplosion(const ExplosionInfo &explosionInfo, const CGameRules::TExplosionAffectedEntities& affectedEntities);
	virtual void SvOnCollision(const IEntity *entity, const CGameRules::SCollisionHitInfo& colHitInfo);

	virtual void ClProcessHit(Vec3 dir, EntityId shooterId, EntityId weaponId, float damage, uint16 projectileClassId, uint8 hitTypeId);
	// ~IGameRulesDamageHandlingModule

#ifdef SERVER_CHECKS
	virtual void RegisterShotIdentification(EntityId ownerId, CWeapon * pWeapon, uint32 mungedShotId);
	void DoShotValidation( const HitInfo &hitInfo, const HitTypeInfo * pHitTypeInfo, CActor * pShooterActor );

private:
	struct SFireData
	{
		EntityId  shooterId;
		float			fTime;
		std::vector<EntityId> targetsHit;
	};

	struct SUnverifiedFireData
	{
		uint32	shotId;
		float		fTime;
		uint16	uChannelId;
	};

	typedef std::map<uint32, SFireData> TShotHitVerificationMap;
	typedef std::vector<SUnverifiedFireData> TUnverifiedList;
	TShotHitVerificationMap m_shotHitVerificationMap;
	TUnverifiedList					m_unverifiedList;
#endif

protected:
	struct EntityCollisionRecord
	{
		EntityCollisionRecord() { entityID = 0; time = 0.0f; }
		EntityCollisionRecord(EntityId _entityID, float _time) : entityID(_entityID), time(_time) { }

		EntityId entityID;
		float time;
	};
	struct SKickableCarRecord
	{
		EntityId vehicleId;
		EntityId victimId;
		float timer;
	};

protected:
	void InitVehicleDamage(XmlNodeRef vehicleDamage);
	void UpdateKickableCarRecords(float frameTime, float currentTime);
	void InsertKickableCarRecord(EntityId vehicle, EntityId victim);
	SKickableCarRecord* FindKickableCarRecord(EntityId vehicle, EntityId victim);
	float ProcessActorKickedVehicle(IActor* victimActor, EntityId victimId, EntityId kickerId, EntityId vehicleId, float damage, const CGameRules::SCollisionHitInfo& collisionHitInfo);
	float ProcessActorVehicleCollision(IActor* victimActor, EntityId victimId, IVehicle* pVehicle, EntityId vehicleId, float damage, const CGameRules::SCollisionHitInfo& collisionHitInfo, EntityId& kickerId);
	bool IsDead(CActor* actor, IScriptTable* actorScript);
	void ProcessDeath(const HitInfo &hitInfo, CActor& pTarget);
	void ProcessVehicleDeath(const HitInfo &hitInfo);
	float CalcExplosionDamage(IEntity* entity, const ExplosionInfo &explosionInfo, float obstruction);
	float GetFriendlyFireDamage(float damage, const HitInfo &hitInfo, IActor* pActor);

	void StartLocalMeleeScreenFx();
	void StopLocalMeleeScreenFx();
	void UpdateLocalMeleeScreenFx(const float frameTime);

	float CalculateFriendlyFireRatio(EntityId entityId1, EntityId entityId2);
	
	// Collisions
	float GetCollisionMinVelocity(const IEntity *pEntity, const CGameRules::SCollisionHitInfo& colHitInfo);
	float AdjustPlayerCollisionDamage(const IEntity *pEntity, const IEntity *pCollider, const CGameRules::SCollisionHitInfo& colHitInfo, float colInfo_damage);
	//float ProcessPlayerToActorCollision(const IEntity *pEntity, const IEntity *pCollider, const CGameRules::SCollisionHitInfo& colHitInfo, float colInfo_damage);

	void DelegateServerHit(IScriptTable* victimScript, const HitInfo& hit, CActor* pVictimActor);

	typedef stl::hash_map<EntityId, EntityCollisionRecord> EntityCollisionRecords;
	EntityCollisionRecords m_entityCollisionRecords;

	enum { maxKickableCarRecords=16 };
	SKickableCarRecord m_kickableCarRecords[maxKickableCarRecords];
	int m_numKickableCarRecords;

	float m_localMeleeScreenFxTimer;
	float m_entityLastDamageUpdateTimer;

	// Vehicle Damage Settings
	struct SVehicleDamageSettings
	{
		float killSpeed;
	};
	SVehicleDamageSettings m_vehicleDamageSettings;
	typedef std::map<EntityId, uint32> TVehicleBodyDamageMap;
	TVehicleBodyDamageMap m_vehicleBodyDamageMap;

#ifdef SERVER_CHECKS
	int m_checkCounter;
#endif
};

#endif // _GAME_RULES_MP_DAMAGE_HANDLING_H_

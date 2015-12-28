#ifndef _ENERGY_SHIELD_GAME_EFFECT_
#define _ENERGY_SHIELD_GAME_EFFECT_

#pragma once

// Includes
#include "GameEffect.h"
#include "Effects/GameEffectsSystem.h"
#include "CREEnergyShield.h"
#include "Effects/RenderNodes/IGameRenderNode.h"
#include "Audio/AudioSignalPlayer.h"

// Forward declares
struct ILightSource;

//==================================================================================================
// Name: EEnergyShieldHitFlags
// Desc: Energy Shield Hit flags
// Author: James Chilvers
//==================================================================================================
enum EEnergyShieldHitFlags
{
	ESHF_DEFLECTED						= (1<<0),
	ESHF_PROJECTILE						=	(1<<1),
	ESHF_INSIDE_SHIELD				= (1<<2),
	ESHF_EXPLOSIVE						= (1<<3)
};//------------------------------------------------------------------------------------------------

//==================================================================================================
// Name: CEnergyShieldGameEffect
// Desc: Energy Shield Game logic
// Author: James Chilvers
//==================================================================================================
class CEnergyShieldGameEffect : public CGameEffect
{
	DECLARE_TYPE(CEnergyShieldGameEffect, CGameEffect); // Exposes this type for SoftCoding
public:

	CEnergyShieldGameEffect();
	virtual ~CEnergyShieldGameEffect();

	virtual void	Initialise(const SGameEffectParams* pGameEffectParams = NULL);
	virtual void	Release();
	virtual void	SetActive(bool isActive);

	virtual void	Update(float frameTime);

	virtual const char* GetName() const;

	void SetRadius(float newRadius);
	void AddHit(const Vec3& pos,const Vec3& vel,float projectileRadius,uint8 flags = 0);
	void SetPos(const Vec3& newPos);
	void GetPos(Vec3& posOut) const;
	void SetShieldOwnerStatus(EEnergyShieldStatus newOwnerStatus);

#if DEBUG_GAME_FX_SYSTEM
	static void  DebugOnInputEvent(int keyId);
	static void	 DebugDisplay(const Vec2& textStartPos,float textSize,float textYStep);
#endif

	static void LoadStaticData(IItemParamsNode* pRootNode);
	static void ReloadStaticData(IItemParamsNode* pRootNode);
	static void ReleaseStaticData();

private:

	static void ReadColorFromXml(const IItemParamsNode* pColorNode,Vec3* pColorArray);

	void CreateLightSource();
	void ReleaseLightSource();
	void UpdateLightSource(bool bForceUpdate=false);

	void UpdateVisuals();
	void SetAlpha(float newAlpha);
	void UpdateEffectAlpha(float frameTime);

	void SpawnDeflectionParticleEffect(const Vec3& pos, const Vec3& vel);

	void Start();
	void Stop();

	CAudioSignalPlayer							SOFT(m_ambientSound);
	IGameRenderNodePtr							SOFT(m_pRenderNode);
	ILightSource*										SOFT(m_pLightSource);
	EntityId												SOFT(m_entityId);
	float														SOFT(m_radius);
	float														SOFT(m_statusTransitionScale);
	float														SOFT(m_effectAlpha);
	uint8														SOFT(m_energyShieldFlags);
	uint8														SOFT(m_oldOwnerStatus);
	uint8														SOFT(m_newOwnerStatus);
	bool														SOFT(m_bVisualsNeedUpdate);
};//------------------------------------------------------------------------------------------------

//==================================================================================================
// Name: SHologramGameEffectParams
// Desc: Hologram game effect parameters
// Author: James Chilvers
//==================================================================================================
struct SEnergyShieldGameEffectParams : public SGameEffectParams
{
	SEnergyShieldGameEffectParams()
	{
		pos.Set(0.0f,0.0f,0.0f);
		radius = 0.0f;
		entityId = 0;
	}

	Vec3			pos;
	float			radius;
	EntityId	entityId;
};//------------------------------------------------------------------------------------------------

#endif // _ENERGY_SHIELD_GAME_EFFECT_

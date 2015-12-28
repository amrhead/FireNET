//==================================================================================================
// Name: CEnergyShieldGameEffect
// Desc: Energy Shield Game logic
// Author: James Chilvers
//==================================================================================================

// Includes
#include "StdAfx.h"
#include "EnergyShieldGameEffect.h"
#include "ItemParams.h"
#include "Effects/RenderNodes/EnergyShieldGameRenderNode.h"
#include <IRenderAuxGeom.h>

REGISTER_EFFECT_DEBUG_DATA(CEnergyShieldGameEffect::DebugOnInputEvent,CEnergyShieldGameEffect::DebugDisplay,EnergyShield);
REGISTER_DATA_CALLBACKS(CEnergyShieldGameEffect::LoadStaticData,CEnergyShieldGameEffect::ReleaseStaticData,CEnergyShieldGameEffect::ReloadStaticData,EnergyShieldData);

// Soft-coding: 
IMPLEMENT_TYPE(CEnergyShieldGameEffect); // Provides the factory creation and access to reflection

//==================================================================================================
// Name: EEnergyShieldGameEffectFlags
// Desc: Energy Shield Game effect flags
// Author: James Chilvers
//==================================================================================================
enum EEnergyShieldGameEffectFlags
{
	ENERGY_SHIELD_FADING_IN		= (1<<0),
	ENERGY_SHIELD_FADING_OUT	=	(1<<1)
};//------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Desc: Static data
//--------------------------------------------------------------------------------------------------
#if DEBUG_GAME_FX_SYSTEM
	static float s_debugRadius = 5.0f;
#endif

const char* ENERGY_SHIELD_GAME_EFFECT_NAME = "Energy Shield";
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: SEnergyShieldGameEffectData
// Desc: Data loaded from xml to control game effect
//--------------------------------------------------------------------------------------------------
struct SEnergyShieldGameEffectData
{
	struct SLightParams
	{
		SLightParams()
		{
			for(int i=0; i<eMAX_ENERGY_SHIELD_STATUS; i++)
			{
				color[i].Set(0.0f,0.0f,0.0f);
			}
			specular = 0.0f;
			heightOffsetScale = 0.0f;
			radiusScale = 0.0f;
		}

		Vec3	color[eMAX_ENERGY_SHIELD_STATUS];
		float specular;
		float heightOffsetScale;
		float radiusScale;
	};

	SEnergyShieldGameEffectData()
	{
		pDeflectionParticleEffect = NULL;
		deflectionParticleEffectScale = 0.0f;
		statusTransitionSpeed = 1.0f;
		effectAlphaTransitionSpeed = 1.0f;
		ambientSoundID = INVALID_AUDIOSIGNAL_ID;
		startUpSoundID = INVALID_AUDIOSIGNAL_ID;
		explosivePassSoundID = INVALID_AUDIOSIGNAL_ID;
		explosiveBounceSoundID = INVALID_AUDIOSIGNAL_ID;
		playerPassSoundID = INVALID_AUDIOSIGNAL_ID;
		bulletPassSoundID = INVALID_AUDIOSIGNAL_ID;

		isInitialised = false;
	}

	SEnergyShieldGameRenderNodeParams::SStaticParams renderNodeParams;
	SLightParams					lightParams;
	IParticleEffect*			pDeflectionParticleEffect;
	float									deflectionParticleEffectScale;
	float									statusTransitionSpeed;
	float									effectAlphaTransitionSpeed;
	TAudioSignalID				ambientSoundID;
	TAudioSignalID				startUpSoundID;
	TAudioSignalID				explosivePassSoundID;
	TAudioSignalID				explosiveBounceSoundID;
	TAudioSignalID				playerPassSoundID;
	TAudioSignalID				bulletPassSoundID;

	bool							isInitialised;
};
static SEnergyShieldGameEffectData s_energyShieldGEData;
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: CEnergyShieldGameEffect
// Desc: Constructor
//--------------------------------------------------------------------------------------------------
CEnergyShieldGameEffect::CEnergyShieldGameEffect()
{
	GAME_FX_SYSTEM.RegisterGameRenderNode(m_pRenderNode);
	m_pLightSource = NULL;
	m_radius = 0.0f;
	m_bVisualsNeedUpdate = false;
	m_oldOwnerStatus = eENERGY_SHIELD_STATUS_NEUTRAL;
	m_newOwnerStatus = eENERGY_SHIELD_STATUS_NEUTRAL;
	m_statusTransitionScale = 1.0f;
	m_effectAlpha = 0.0f;
	m_energyShieldFlags = 0;
	m_entityId = 0;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: ~CEnergyShieldGameEffect
// Desc: Destructor
//--------------------------------------------------------------------------------------------------
CEnergyShieldGameEffect::~CEnergyShieldGameEffect()
{
	GAME_FX_SYSTEM.UnregisterGameRenderNode(m_pRenderNode);
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: ReadColorFromXml
// Desc: Reads colors from xml
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::ReadColorFromXml(const IItemParamsNode* pColorNode,Vec3* pColorArray)
{
	if(pColorNode && pColorArray)
	{
		pColorNode->GetAttribute("neutral",pColorArray[eENERGY_SHIELD_STATUS_NEUTRAL]);
		pColorNode->GetAttribute("friendly",pColorArray[eENERGY_SHIELD_STATUS_FRIENDLY]);
		pColorNode->GetAttribute("enemy",pColorArray[eENERGY_SHIELD_STATUS_ENEMY]);
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: LoadStaticData
// Desc: Loads static data for effect
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::LoadStaticData(IItemParamsNode* pRootNode)
{
	const IItemParamsNode* pParamNode = pRootNode->GetChild("EnergyShield");

	if(pParamNode)
	{
		SEnergyShieldGameRenderNodeParams::SStaticParams& params = s_energyShieldGEData.renderNodeParams;

		// General params
		pParamNode->GetAttribute("statusTransitionSpeed",s_energyShieldGEData.statusTransitionSpeed);
		pParamNode->GetAttribute("effectAlphaTransitionSpeed",s_energyShieldGEData.effectAlphaTransitionSpeed);

		// Sphere params
		const IItemParamsNode* pSphereNode = pParamNode->GetChild("sphere");
		if(pSphereNode)
		{
			pSphereNode->GetAttribute("maxViewDistance",params.maxViewDistance);
			pSphereNode->GetAttribute("meshSegmentCount",params.xml.sphereMeshSegmentCount);
			params.xml.sphereMeshSegmentCount = max(params.xml.sphereMeshSegmentCount,2);
			params.pSphereMaterial = LoadMaterial(pSphereNode->GetAttribute("material"));
			pSphereNode->GetAttribute("frontFaceAlphaFadeScale",params.xml.frontFaceAlphaFadeScale);
			pSphereNode->GetAttribute("animSpeed",params.xml.sphereAnimSpeed);
			pSphereNode->GetAttribute("noiseAnimSpeed",params.xml.noiseAnimSpeed);
			pSphereNode->GetAttribute("softNoiseAnimSpeed",params.xml.softNoiseAnimSpeed);
			ReadColorFromXml(pSphereNode->GetChild("color"),params.xml.sphereColor);
			ReadColorFromXml(pSphereNode->GetChild("refractionColor"),params.xml.sphereRefractionColor);
			ReadColorFromXml(pSphereNode->GetChild("chromaShiftChannelStrength"),params.xml.chromaShiftChannelStrength);
		}

		// Hit effect params
		const IItemParamsNode* pHitEffectNode = pParamNode->GetChild("hitEffect");
		if(pHitEffectNode)
		{
			pHitEffectNode->GetAttribute("reservedHitEffectCount",params.xml.reservedHitEffectCount);
			pHitEffectNode->GetAttribute("hitEffectMeshSegmentCount",params.xml.hitEffectMeshSegmentCount);
			params.xml.hitEffectMeshSegmentCount = max(params.xml.hitEffectMeshSegmentCount,1);
			pHitEffectNode->GetAttribute("deflectionMeshSegmentCount",params.xml.deflectionMeshSegmentCount);
			params.xml.deflectionMeshSegmentCount = max(params.xml.deflectionMeshSegmentCount,1);
			pHitEffectNode->GetAttribute("hitEffectDuration",params.xml.hitEffectDuration);
			params.xml.hitEffectDuration = max(params.xml.hitEffectDuration,0.001f);
			pHitEffectNode->GetAttribute("deflectionDuration",params.xml.deflectionDuration);
			params.xml.deflectionDuration = max(params.xml.deflectionDuration,0.001f);
			pHitEffectNode->GetAttribute("particleEffectScale",s_energyShieldGEData.deflectionParticleEffectScale);
			pHitEffectNode->GetAttribute("hitWaveRefractionColScale",params.xml.hitWaveRefractionColScale);
			pHitEffectNode->GetAttribute("deflectWaveRefractionColScale",params.xml.deflectWaveRefractionColScale);
			s_energyShieldGEData.pDeflectionParticleEffect = LoadParticleEffect(pHitEffectNode->GetAttribute("particleEffect"));
			params.pHitEffectMaterial = LoadMaterial(pHitEffectNode->GetAttribute("material"));
			ReadColorFromXml(pHitEffectNode->GetChild("startColor"),params.xml.hitEffectStartColor);
			ReadColorFromXml(pHitEffectNode->GetChild("endColor"),params.xml.hitEffectEndColor);
		}

		// Pulse params
		const IItemParamsNode* pPulseNode = pParamNode->GetChild("pulse");
		if(pPulseNode)
		{
			params.pSpherePulseMaterial = LoadMaterial(pPulseNode->GetAttribute("material"));
			pPulseNode->GetAttribute("count",params.xml.pulseCount);
			pPulseNode->GetAttribute("speed",params.xml.pulseSpeed);
			pPulseNode->GetAttribute("radiusScale",params.xml.pulseRadiusScale);
			pPulseNode->GetAttribute("alphaScale",params.xml.pulseAlphaScale);
			pPulseNode->GetAttribute("alphaSaturation",params.xml.pulseAlphaSaturation);
			pPulseNode->GetAttribute("alphaReduction",params.xml.pulseAlphaReduction);
			pPulseNode->GetAttribute("highLightScale",params.xml.pulseHighLightScale);
			ReadColorFromXml(pPulseNode->GetChild("color"),params.xml.pulseColor);
		}

		// Pulse params
		const IItemParamsNode* pLightNode = pParamNode->GetChild("light");
		if(pLightNode)
		{
			pLightNode->GetAttribute("specular",s_energyShieldGEData.lightParams.specular);
			pLightNode->GetAttribute("heightOffsetScale",s_energyShieldGEData.lightParams.heightOffsetScale);
			pLightNode->GetAttribute("radiusScale",s_energyShieldGEData.lightParams.radiusScale);
			ReadColorFromXml(pLightNode->GetChild("color"),s_energyShieldGEData.lightParams.color);
		}

		// Sound fx params
		if(g_pGame)
		{
			CGameAudio* pGameAudio = g_pGame->GetGameAudio();
			if(pGameAudio)
			{
				const IItemParamsNode* pSoundFXNode = pParamNode->GetChild("soundFX");
				if(pSoundFXNode)
				{
					s_energyShieldGEData.ambientSoundID = pGameAudio->GetSignalID(pSoundFXNode->GetAttribute("ambient"));
					s_energyShieldGEData.startUpSoundID = pGameAudio->GetSignalID(pSoundFXNode->GetAttribute("startUp"));
					s_energyShieldGEData.explosivePassSoundID = pGameAudio->GetSignalID(pSoundFXNode->GetAttribute("explosivePass"));
					s_energyShieldGEData.explosiveBounceSoundID = pGameAudio->GetSignalID(pSoundFXNode->GetAttribute("explosiveBounce"));
					s_energyShieldGEData.playerPassSoundID = pGameAudio->GetSignalID(pSoundFXNode->GetAttribute("playerPass"));
					s_energyShieldGEData.bulletPassSoundID = pGameAudio->GetSignalID(pSoundFXNode->GetAttribute("bulletPass"));
				}
			}
		}

		s_energyShieldGEData.isInitialised = true;
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: ReloadStaticData
// Desc: Reloads static data
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::ReloadStaticData(IItemParamsNode* pRootNode)
{
	ReleaseStaticData();
	LoadStaticData(pRootNode);

#if DEBUG_GAME_FX_SYSTEM
	// Data has been reloaded, so re-initialse debug effect with new data
	CEnergyShieldGameEffect* pDebugEnergyShieldEffect = (CEnergyShieldGameEffect*)GAME_FX_SYSTEM.GetDebugEffect(ENERGY_SHIELD_GAME_EFFECT_NAME);
	if(pDebugEnergyShieldEffect && pDebugEnergyShieldEffect->IsFlagSet(GAME_EFFECT_REGISTERED))
	{
		s_energyShieldGEData.renderNodeParams.xml.reloadedData = true;

		SEnergyShieldGameEffectParams params;
		pDebugEnergyShieldEffect->GetPos(params.pos);
		params.radius = s_debugRadius;
		pDebugEnergyShieldEffect->Initialise(&params);
		pDebugEnergyShieldEffect->SetAlpha(0.0f);
		pDebugEnergyShieldEffect->SetAlpha(1.0f);
		pDebugEnergyShieldEffect->UpdateVisuals();

		s_energyShieldGEData.renderNodeParams.xml.reloadedData = false;
	}
#endif
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: ReleaseStaticData
// Desc: Releases static data
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::ReleaseStaticData()
{
	if(s_energyShieldGEData.isInitialised)
	{
		SAFE_RELEASE(s_energyShieldGEData.renderNodeParams.pSphereMaterial);
		SAFE_RELEASE(s_energyShieldGEData.renderNodeParams.pHitEffectMaterial);
		SAFE_RELEASE(s_energyShieldGEData.renderNodeParams.pSpherePulseMaterial);
		SAFE_RELEASE(s_energyShieldGEData.pDeflectionParticleEffect);
		
		s_energyShieldGEData.isInitialised = false;
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: Initialise
// Desc: Initializes game effect
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::Initialise(const SGameEffectParams* pGameEffectParams)
{
	CGameEffect::Initialise(pGameEffectParams);

	if(pGameEffectParams)
	{
		const SEnergyShieldGameEffectParams* pEnergyShieldParams = static_cast<const SEnergyShieldGameEffectParams*>(pGameEffectParams);

		if(m_pRenderNode == NULL)
		{
			m_pRenderNode = CREATE_GAME_FX_SOFT_CODE_INSTANCE(CEnergyShieldGameRenderNode);
		}

		if(m_pRenderNode)
		{
			m_pRenderNode->SetRndFlags(ERF_HIDDEN,true);

			SEnergyShieldGameRenderNodeParams renderNodeParams;
			renderNodeParams.staticParams = s_energyShieldGEData.renderNodeParams;
			renderNodeParams.radius = pEnergyShieldParams->radius;
			renderNodeParams.pos = pEnergyShieldParams->pos;
			m_pRenderNode->SetParams(&renderNodeParams);

			m_pRenderNode->InitialiseGameRenderNode();
		}

		m_radius = pEnergyShieldParams->radius;
		m_entityId = pEnergyShieldParams->entityId;

		CreateLightSource();

		m_ambientSound.SetSignal(s_energyShieldGEData.ambientSoundID);
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: Release
// Desc: Releases game effect
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::Release()
{
	CGameEffect::SetActive(false);

	SAFE_DELETE_GAME_RENDER_NODE(m_pRenderNode);

	ReleaseLightSource();
	
	CGameEffect::Release();
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: CreateLightSource
// Desc: Creates light source
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::CreateLightSource()
{
	if(m_pLightSource == NULL)
	{
		m_pLightSource = gEnv->p3DEngine->CreateLightSource();
		if(m_pLightSource)
		{
			m_pLightSource->SetRndFlags(ERF_HIDDEN,true);

			const bool bForceLightUpdate = true;
			UpdateLightSource(bForceLightUpdate); // Light source must be valid before registering
			gEnv->p3DEngine->RegisterEntity(m_pLightSource);
		}
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: ReleaseLightSource
// Desc: Releases light source
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::ReleaseLightSource()
{
	if(m_pLightSource)
	{
		gEnv->p3DEngine->UnRegisterEntity(m_pLightSource);
		gEnv->p3DEngine->DeleteLightSource(m_pLightSource);
		m_pLightSource = NULL;
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: SetActive
// Desc: Sets active status of effect
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::SetActive(bool isActive)
{
	if(s_energyShieldGEData.isInitialised)
	{
		if(isActive)
		{
			Start();
		}
		else
		{
			Stop();
		}
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: Start
// Desc: Starts effect
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::Start()
{
	if(IsFlagSet(GAME_EFFECT_ACTIVE) == false)
	{
		CGameEffect::SetActive(true);
		SET_FLAG(m_energyShieldFlags,ENERGY_SHIELD_FADING_IN,true);
		SET_FLAG(m_energyShieldFlags,ENERGY_SHIELD_FADING_OUT,false);

		m_ambientSound.Play(m_entityId);

		Vec3 pos(ZERO);
		GetPos(pos);
		CAudioSignalPlayer::JustPlay(s_energyShieldGEData.startUpSoundID,pos);
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: Stop
// Desc: Stops effect
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::Stop()
{
	if(IsFlagSet(GAME_EFFECT_ACTIVE))
	{
		SET_FLAG(m_energyShieldFlags,ENERGY_SHIELD_FADING_IN,false);
		SET_FLAG(m_energyShieldFlags,ENERGY_SHIELD_FADING_OUT,true);

		m_ambientSound.Stop(m_entityId);
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: GetName
// Desc: Gets effect's name
//--------------------------------------------------------------------------------------------------
const char* CEnergyShieldGameEffect::GetName() const
{
	return ENERGY_SHIELD_GAME_EFFECT_NAME;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: Update
// Desc: Updates effect
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::Update(float frameTime)
{
	CGameEffect::Update(frameTime);

	UpdateEffectAlpha(frameTime);
	
	if(m_oldOwnerStatus != m_newOwnerStatus)
	{
		m_bVisualsNeedUpdate = true;
		m_statusTransitionScale += (frameTime * s_energyShieldGEData.statusTransitionSpeed);
		if(m_statusTransitionScale >= 1.0f)
		{
			m_statusTransitionScale = 1.0f;
			m_oldOwnerStatus = m_newOwnerStatus;
		}
	}

	if(m_bVisualsNeedUpdate)
	{
		UpdateVisuals();
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: UpdateLightSource
// Desc: Updates light source
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::UpdateLightSource(bool bForceUpdate)
{
	if(m_pLightSource)
	{
		if((m_effectAlpha > 0.0f) || bForceUpdate)
		{
			Vec3 pos;
			GetPos(pos);

			// Offset height so above ground level
			pos.z += (m_radius * s_energyShieldGEData.lightParams.heightOffsetScale);

			// Lerp color
			const Vec3& oldColor = s_energyShieldGEData.lightParams.color[m_oldOwnerStatus];
			const Vec3& newColor = s_energyShieldGEData.lightParams.color[m_newOwnerStatus];
			ColorF lightColor = Vec3::CreateLerp(oldColor,newColor,m_statusTransitionScale) * m_effectAlpha;

			CDLight lightParams;
			lightParams.SetLightColor(lightColor);
			lightParams.SetSpecularMult(s_energyShieldGEData.lightParams.specular);

			lightParams.m_fRadius = m_radius * s_energyShieldGEData.lightParams.radiusScale;
			lightParams.m_Flags |= DLF_POINT;
			lightParams.m_sName = "Energy Shield Light";

			m_pLightSource->SetLightProperties(lightParams);

			Matrix34 worldMatrix;
			worldMatrix.SetIdentity();
			worldMatrix.SetTranslation(pos);
			m_pLightSource->SetMatrix(worldMatrix);
		}
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: UpdateEffectAlpha
// Desc: Updates effect alpha
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::UpdateEffectAlpha(float frameTime)
{
	if(IS_FLAG_SET(m_energyShieldFlags,ENERGY_SHIELD_FADING_IN))
	{
		float newAlpha = m_effectAlpha + (frameTime*s_energyShieldGEData.effectAlphaTransitionSpeed);
		if(newAlpha >= 1.0f)
		{
			newAlpha = 1.0f;
			SET_FLAG(m_energyShieldFlags,ENERGY_SHIELD_FADING_IN,false);
		}
		SetAlpha(newAlpha);
	}
	else if(IS_FLAG_SET(m_energyShieldFlags,ENERGY_SHIELD_FADING_OUT))
	{
		float newAlpha = m_effectAlpha - (frameTime*s_energyShieldGEData.effectAlphaTransitionSpeed);
		if(newAlpha <= 0.0f)
		{
			newAlpha = 0.0f;
			SET_FLAG(m_energyShieldFlags,ENERGY_SHIELD_FADING_OUT,false);
			CGameEffect::SetActive(false);
		}
		SetAlpha(newAlpha);
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: SetAlpha
// Desc: Sets effect alpha
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::SetAlpha(float newAlpha)
{
	if(m_effectAlpha != newAlpha)
	{
		m_bVisualsNeedUpdate = true;

		const bool bHidden = (newAlpha > 0.0f) ? false : true;

		if(m_pRenderNode)
		{
			m_pRenderNode->SetRndFlags(ERF_HIDDEN,bHidden);
		}
		if(m_pLightSource)
		{
			m_pLightSource->SetRndFlags(ERF_HIDDEN,bHidden);
		}
	}

	m_effectAlpha = newAlpha;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: SetRadius
// Desc: Sets effect radius
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::SetRadius(float newRadius)
{
	m_bVisualsNeedUpdate = true;
	m_radius = newRadius;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: AddHit
// Desc: Adds a hit effect
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::AddHit(const Vec3& pos,const Vec3& vel,float projectileRadius,uint8 flags)
{
	if(m_pRenderNode)
	{
		const bool bIsProjectile = IS_FLAG_SET(flags,ESHF_PROJECTILE);
		if(bIsProjectile)
		{
			SEnergyShieldHitData newHitData;
			newHitData.pos = pos;
			newHitData.spawnTime = gEnv->pTimer->GetCurrTime(ITimer::ETIMER_GAME);
			newHitData.speed = vel.GetLength();
			newHitData.bounceSpeed = vel;
			newHitData.bDeflected = IS_FLAG_SET(flags,ESHF_DEFLECTED);

			SEnergyShieldGameRenderNodeParams renderNodeParams;
			renderNodeParams.pNewHitEffect = &newHitData;
			m_pRenderNode->SetParams(&renderNodeParams);

			if(newHitData.bDeflected)
			{
				SpawnDeflectionParticleEffect(pos,vel);
				CAudioSignalPlayer::JustPlay(s_energyShieldGEData.explosiveBounceSoundID,pos);
			}
			else
			{
				const bool bInSideShield = IS_FLAG_SET(flags,ESHF_INSIDE_SHIELD);
				const bool bExplosive = IS_FLAG_SET(flags,ESHF_EXPLOSIVE);
				TAudioSignalID soundId = INVALID_AUDIOSIGNAL_ID;

				if(bInSideShield && bExplosive)
				{
					soundId = s_energyShieldGEData.explosivePassSoundID;
				}
				else
				{
					soundId = s_energyShieldGEData.bulletPassSoundID;
				}
				CAudioSignalPlayer::JustPlay(soundId,pos);
			}
		}
		else
		{
			CAudioSignalPlayer::JustPlay(s_energyShieldGEData.playerPassSoundID,pos);
		}
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: SpawnDeflectionParticleEffect
// Desc: Spawns deflection particle effect
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::SpawnDeflectionParticleEffect(const Vec3& pos, const Vec3& vel)
{
	if(s_energyShieldGEData.pDeflectionParticleEffect)
	{
		Vec3 dir = vel;
		dir.Normalize();

		const float scale = s_energyShieldGEData.deflectionParticleEffectScale * m_radius;
		const bool bIndependent = false;
		IParticleEmitter* pParticleEmitter = s_energyShieldGEData.pDeflectionParticleEffect->Spawn(bIndependent,IParticleEffect::ParticleLoc(pos, dir, scale));
		if(pParticleEmitter)
		{
			// Use emitter strength to control emitter particle color
			// The only other way to control the color is by have 3 particle effects
			const float friendlyStrength = 0.3333f;
			const float enemyStrength = 0.6666f;
			float emitterStrength = 0.0f;
			if(m_newOwnerStatus == m_oldOwnerStatus)
			{
				switch(m_newOwnerStatus)
				{
					case eENERGY_SHIELD_STATUS_FRIENDLY:
					{
						// Friendly
						emitterStrength = friendlyStrength;
						break;
					}
					case eENERGY_SHIELD_STATUS_ENEMY:
					{
						// Enemy
						emitterStrength = enemyStrength;
						break;
					}
				}
			}
			else
			{
				// Transition between state colors
				emitterStrength = m_statusTransitionScale * friendlyStrength;
				switch(m_newOwnerStatus)
				{
					case eENERGY_SHIELD_STATUS_NEUTRAL:
					{
						if(m_oldOwnerStatus == eENERGY_SHIELD_STATUS_FRIENDLY)
						{
							emitterStrength = friendlyStrength - emitterStrength; // Friendly to Neutral
						}
						else
						{
							emitterStrength = enemyStrength + emitterStrength; // Enemy to Neutral
						}
						break;
					}
					case eENERGY_SHIELD_STATUS_FRIENDLY:
					{
						if(m_oldOwnerStatus == eENERGY_SHIELD_STATUS_ENEMY)
						{
							emitterStrength = enemyStrength - emitterStrength; // Enemy to Friendly
						}
						else
						{
							emitterStrength = emitterStrength; // Neutral to Friendly
						}
						break;
					}
					case eENERGY_SHIELD_STATUS_ENEMY:
					{
						if(m_oldOwnerStatus == eENERGY_SHIELD_STATUS_FRIENDLY)
						{
							emitterStrength = friendlyStrength + emitterStrength; // Friendly to Enemy
						}
						else
						{
							emitterStrength = 1.0f - emitterStrength; // Neutral to Enemy
						}
						break;
					}
				}
			}

			SpawnParams spawnParams;
			pParticleEmitter->GetSpawnParams(spawnParams);
			spawnParams.fStrength = emitterStrength;
			pParticleEmitter->SetSpawnParams(spawnParams);
		}
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: GetPos
// Desc: Gets effect positions
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::GetPos(Vec3& posOut) const
{
	if(m_pRenderNode)
	{
		posOut = m_pRenderNode->GetPos();
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: SetPos
// Desc: Sets effect positions
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::SetPos(const Vec3& newPos)
{
	if(m_pRenderNode)
	{
		// Set matrix
		Matrix34 matrix;
		matrix.SetIdentity();
		matrix.SetTranslation(newPos);
		m_pRenderNode->SetMatrix(matrix);
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: SetShieldOwnerStatus
// Desc: Sets shield owner status
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::SetShieldOwnerStatus( EEnergyShieldStatus newOwnerStatus )
{
	m_oldOwnerStatus = m_newOwnerStatus;
	m_newOwnerStatus = newOwnerStatus;
	m_statusTransitionScale = 0.0f;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: UpdateVisuals
// Desc: Updates visual parts of effect
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::UpdateVisuals()
{
	if(m_pRenderNode)
	{
		SEnergyShieldGameRenderNodeParams renderNodeParams;
		renderNodeParams.radius = m_radius;
		renderNodeParams.pos = m_pRenderNode->GetPos();
		renderNodeParams.oldStatus = m_oldOwnerStatus;
		renderNodeParams.newStatus = m_newOwnerStatus;
		renderNodeParams.statusTransitionScale = m_statusTransitionScale;
		renderNodeParams.effectAlpha = m_effectAlpha;
		m_pRenderNode->SetParams(&renderNodeParams);
	}
	UpdateLightSource();
	m_bVisualsNeedUpdate = false;
}//-------------------------------------------------------------------------------------------------

#if DEBUG_GAME_FX_SYSTEM
//--------------------------------------------------------------------------------------------------
// Name: DebugOnInputEvent
// Desc: Called when input events happen in debug builds
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::DebugOnInputEvent(int keyId)
{
	if(s_energyShieldGEData.isInitialised)
	{
		CEnergyShieldGameEffect* pEnergyShieldEffect = (CEnergyShieldGameEffect*)GAME_FX_SYSTEM.GetDebugEffect(ENERGY_SHIELD_GAME_EFFECT_NAME);

		// Read input
		switch(keyId)
		{
			case eKI_NP_1:
			{
				// Create debug effect for development
				if(pEnergyShieldEffect == NULL)
				{
					pEnergyShieldEffect = CREATE_GAME_FX_SOFT_CODE_INSTANCE(CEnergyShieldGameEffect);
					if(pEnergyShieldEffect)
					{
						SEnergyShieldGameEffectParams params;
						params.pos = gEnv->pRenderer->GetCamera().GetPosition();
						params.radius = s_debugRadius;

						EntityId entityId = 0;
						SEntitySpawnParams entitySpawnParams;
						entitySpawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
						entitySpawnParams.sName = "EnergyShield";
						entitySpawnParams.nFlags = ENTITY_FLAG_NO_PROXIMITY | ENTITY_FLAG_CLIENT_ONLY | ENTITY_FLAG_NO_SAVE;

						IEntity *pEntity=gEnv->pEntitySystem->SpawnEntity(entitySpawnParams);
						if(pEntity)
						{
							pEntity->SetPos(params.pos);
							params.entityId = pEntity->GetId();
						}

						pEnergyShieldEffect->Initialise(&params);
						pEnergyShieldEffect->SetFlag(GAME_EFFECT_DEBUG_EFFECT,true);
					}
				}
				break;
			}
			case eKI_NP_2:
			{
				if(pEnergyShieldEffect && pEnergyShieldEffect->m_entityId!=0)
				{
					gEnv->pEntitySystem->RemoveEntity(pEnergyShieldEffect->m_entityId);
				}

				// Delete effect
				SAFE_DELETE_GAME_EFFECT(pEnergyShieldEffect);
				break;
			}
			case eKI_NP_3:
			{
				// Toggle active status
				if(pEnergyShieldEffect)
				{
					pEnergyShieldEffect->SetActive(!pEnergyShieldEffect->IsFlagSet(GAME_EFFECT_ACTIVE));
				}
				break;
			}
			case eKI_NP_4:
			{
				// Hit Effect test
				if(pEnergyShieldEffect)
				{
					Vec3 pos;
					Vec3 vel(0.0f,0.0f,0.0f);
					float projectileRadius = 1.0f;
					pEnergyShieldEffect->GetPos(pos);
	
					Vec3 randomPosOnSphere;
					randomPosOnSphere.SetRandomDirection();
					randomPosOnSphere *= s_debugRadius;
					pos += randomPosOnSphere;

					pEnergyShieldEffect->AddHit(pos,vel,projectileRadius);
				}
				break;
			}
			case eKI_NP_5:
			{
				// Deflection test
				if(pEnergyShieldEffect)
				{
					Vec3 pos;
					Vec3 vel(0.0f,0.0f,0.0f);
					float projectileRadius = 1.0f;
					pEnergyShieldEffect->GetPos(pos);

					Vec3 randomPosOnSphere;
					randomPosOnSphere.SetRandomDirection();
					vel = randomPosOnSphere;
					randomPosOnSphere *= s_debugRadius;
					pos += randomPosOnSphere;

					pEnergyShieldEffect->AddHit(pos,vel,projectileRadius,ESHF_DEFLECTED);
				}
				break;
			}
			case eKI_NP_6:
			{
				// Extreme hit effect test
				if(pEnergyShieldEffect)
				{
					static uint32 spawnHitCount = 300;
					bool bDeflected = true;
					Vec3 effectPos;
					Vec3 hitPos;
					float projectileRadius = 1.0f;
					pEnergyShieldEffect->GetPos(effectPos);
					Vec3 randomPosOnSphere;

					for(uint32 i=0; i<spawnHitCount; i++)
					{
						bDeflected = (cry_frand() > 0.5f) ? true : false;
						uint8 flags = 0;
						SET_FLAG(flags,ESHF_DEFLECTED,bDeflected);
						randomPosOnSphere.SetRandomDirection();
						randomPosOnSphere *= s_debugRadius;
						hitPos = effectPos + randomPosOnSphere;
						pEnergyShieldEffect->AddHit(hitPos,randomPosOnSphere,projectileRadius,flags);
					}
				}
				break;
			}
			case eKI_NP_7:
			{
				// Set Neutral
				if(pEnergyShieldEffect)
					pEnergyShieldEffect->SetShieldOwnerStatus(eENERGY_SHIELD_STATUS_NEUTRAL);
				break;
			}
			case eKI_NP_8:
			{
				// Set Friendly
				if(pEnergyShieldEffect)
					pEnergyShieldEffect->SetShieldOwnerStatus(eENERGY_SHIELD_STATUS_FRIENDLY);
				break;
			}
			case eKI_NP_9:
			{
				// Set Enemy
				if(pEnergyShieldEffect)
					pEnergyShieldEffect->SetShieldOwnerStatus(eENERGY_SHIELD_STATUS_ENEMY);
				break;
			}
			case eKI_NP_Multiply:
			{
				// Increase shield radius
				s_debugRadius += 0.1f;
				if(pEnergyShieldEffect)
					pEnergyShieldEffect->SetRadius(s_debugRadius);
				break;
			}
			case eKI_NP_Divide:
			{
				// Decrease shield radius
				s_debugRadius = max(s_debugRadius-0.1f,0.0f);
				if(pEnergyShieldEffect)
					pEnergyShieldEffect->SetRadius(s_debugRadius);
				break;
			}
		}
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: DebugDisplay
// Desc: Display when this effect is selected to debug through the game effects system
//--------------------------------------------------------------------------------------------------
void CEnergyShieldGameEffect::DebugDisplay(const Vec2& textStartPos,float textSize,float textYStep)
{
	ColorF textCol(1.0f,1.0f,0.0f,1.0f);
	Vec2 currentTextPos = textStartPos;

	if(s_energyShieldGEData.isInitialised)
	{
		gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&textCol.r,false,"Create: NumPad 1");
		currentTextPos.y += textYStep;
		gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&textCol.r,false,"Destroy: NumPad 2");
		currentTextPos.y += textYStep;
		gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&textCol.r,false,"Toggle Active: NumPad 3");
		currentTextPos.y += textYStep;
		gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&textCol.r,false,"Add random hit: NumPad 4");
		currentTextPos.y += textYStep;
		gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&textCol.r,false,"Add random deflection hit: NumPad 5");
		currentTextPos.y += textYStep;
		gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&textCol.r,false,"Extreme test: NumPad 6");
		currentTextPos.y += textYStep;
		gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&textCol.r,false,"Set Neutral: 7");
		currentTextPos.y += textYStep;
		gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&textCol.r,false,"Set Friendly: 8");
		currentTextPos.y += textYStep;
		gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&textCol.r,false,"Set Enemy: 9");
		currentTextPos.y += textYStep;
		gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&textCol.r,false,"Add/Decrease radius: NumPad */   = %f",s_debugRadius);
	}
	else
	{
		gEnv->pRenderer->Draw2dLabel(currentTextPos.x,currentTextPos.y,textSize,&textCol.r,false,"Effect failed to load data");
	}
}//-------------------------------------------------------------------------------------------------

#endif
// CryEngine Source File.
// Copyright (C), Crytek, 1999-2014.

#include "StdAfx.h"
#include "RuntimeAreaObject.h"

CRuntimeAreaObject::TAudioControlMap CRuntimeAreaObject::m_cAudioControls;

///////////////////////////////////////////////////////////////////////////
CRuntimeAreaObject::CRuntimeAreaObject()
{
}

///////////////////////////////////////////////////////////////////////////
CRuntimeAreaObject::~CRuntimeAreaObject()
{
}

//////////////////////////////////////////////////////////////////////////
bool CRuntimeAreaObject::Init(IGameObject *pGameObject)
{
	SetGameObject(pGameObject);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CRuntimeAreaObject::ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params )
{
	ResetGameObject();

	CRY_ASSERT_MESSAGE(false, "CRuntimeAreaObject::ReloadExtension not implemented");

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CRuntimeAreaObject::GetEntityPoolSignature( TSerialize signature )
{
	CRY_ASSERT_MESSAGE(false, "CRuntimeAreaObject::GetEntityPoolSignature not implemented");

	return true;
}

///////////////////////////////////////////////////////////////////////////
void CRuntimeAreaObject::Release()
{
	// Stop all of the currently playing sounds controlled by this RuntimeAreaObject instance.
	for (TEntitySoundsMap::iterator iEntityData = m_cActiveEntitySounds.begin(),
		iEntityDataEnd = m_cActiveEntitySounds.end(); iEntityData != iEntityDataEnd; ++iEntityData)
	{
		StopEntitySounds(iEntityData->first, iEntityData->second);
	}

	delete this;
}

///////////////////////////////////////////////////////////////////////////
bool CRuntimeAreaObject::NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags)
{
	CRY_ASSERT_MESSAGE(false, "CRuntimeAreaObject::NetSerialize not implemented");

	return false;
}

///////////////////////////////////////////////////////////////////////////
void CRuntimeAreaObject::ProcessEvent(SEntityEvent& entityEvent)
{
	switch (entityEvent.event)
	{
	case ENTITY_EVENT_ENTERAREA:
		{
			EntityId const nEntityID = static_cast<EntityId>(entityEvent.nParam[0]);
			
			IEntity* const pEntity = gEnv->pEntitySystem->GetEntity(nEntityID);
			if ((pEntity != NULL) && ((pEntity->GetFlagsExtended() & ENTITY_FLAG_EXTENDED_CAN_COLLIDE_WITH_MERGED_MESHES) != 0))
			{
				TAudioParameterMap cNewEntityParamMap;

				UpdateParameterValues(pEntity, cNewEntityParamMap);

				m_cActiveEntitySounds.insert(
					std::pair<EntityId, TAudioParameterMap>(static_cast<EntityId>(nEntityID), cNewEntityParamMap));
			}

			break;
		}
	case ENTITY_EVENT_LEAVEAREA:
		{
			EntityId const nEntityID = static_cast<EntityId>(entityEvent.nParam[0]);

			TEntitySoundsMap::iterator iFoundPair = m_cActiveEntitySounds.find(nEntityID);
			if (iFoundPair != m_cActiveEntitySounds.end())
			{
				StopEntitySounds(iFoundPair->first, iFoundPair->second);
				m_cActiveEntitySounds.erase(nEntityID);
			}

			break;
		}
	case ENTITY_EVENT_MOVEINSIDEAREA:
		{
			EntityId const nEntityID = static_cast<EntityId>(entityEvent.nParam[0]);
			TEntitySoundsMap::iterator iFoundPair = m_cActiveEntitySounds.find(nEntityID);

			if (iFoundPair != m_cActiveEntitySounds.end())
			{
				IEntity* const pEntity = gEnv->pEntitySystem->GetEntity(nEntityID);
				if ((pEntity != NULL) && ((pEntity->GetFlagsExtended() & ENTITY_FLAG_EXTENDED_CAN_COLLIDE_WITH_MERGED_MESHES) != 0))
				{
					UpdateParameterValues(pEntity, iFoundPair->second);
				}
			}

			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void CRuntimeAreaObject::GetMemoryUsage(ICrySizer *pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
}

//////////////////////////////////////////////////////////////////////////
void CRuntimeAreaObject::UpdateParameterValues(IEntity* const pEntity, TAudioParameterMap& rParamMap)
{
	static float const fParamEpsilon = 0.001f;
	static float const fMaxDensity = 256.0f;

	IEntityAudioProxy* const pAudioProxy = static_cast<IEntityAudioProxy*>(pEntity->CreateProxy(ENTITY_PROXY_AUDIO).get());
	if (pAudioProxy != NULL)
	{
		ISurfaceType*	aSurfaceTypes[MMRM_MAX_SURFACE_TYPES];
		memset(aSurfaceTypes, 0x0, sizeof(aSurfaceTypes));

		float aDensities[MMRM_MAX_SURFACE_TYPES];
		memset(aDensities, 0x0, sizeof(aDensities));

		gEnv->p3DEngine->GetIMergedMeshesManager()->QueryDensity(pEntity->GetPos(), aSurfaceTypes, aDensities);

		for (int i = 0; i < MMRM_MAX_SURFACE_TYPES && (aSurfaceTypes[i] != NULL); ++i)
		{
			float const fNewParamValue = aDensities[i]/256.0f;
			TSurfaceCRC const nSurfaceCrc = CCrc32::ComputeLowercase(aSurfaceTypes[i]->GetName());

			TAudioParameterMap::iterator iSoundPair = rParamMap.find(nSurfaceCrc);
			if (iSoundPair == rParamMap.end())
			{	
				if (fNewParamValue > 0.0f)
				{
					// The sound for this surface is not yet playing on this entity, needs to be started.
					TAudioControlMap::const_iterator const iAudioControls = m_cAudioControls.find(nSurfaceCrc);
					if (iAudioControls != m_cAudioControls.end())
					{
						SAudioControls const& rAudioControls = iAudioControls->second;

						pAudioProxy->SetRtpcValue(rAudioControls.nRtpcID, fNewParamValue);
						pAudioProxy->ExecuteTrigger(rAudioControls.nTriggerID, eLSM_None);

						rParamMap.insert(
							std::pair<TSurfaceCRC, SAreaSoundInfo>(
							nSurfaceCrc,
							SAreaSoundInfo(rAudioControls, fNewParamValue)));
					}
				}
			}
			else
			{
				SAreaSoundInfo& oSoundInfo = iSoundPair->second;
				if (fabs_tpl(fNewParamValue - oSoundInfo.fParameter) >= fParamEpsilon)
				{
					oSoundInfo.fParameter = fNewParamValue;
					pAudioProxy->SetRtpcValue(oSoundInfo.oAudioControls.nRtpcID, oSoundInfo.fParameter);
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void CRuntimeAreaObject::StopEntitySounds(EntityId const nEntityID, TAudioParameterMap& rParamMap)
{
	IEntity* const pEntity = gEnv->pEntitySystem->GetEntity(nEntityID);
	if (pEntity != NULL)
	{
		IEntityAudioProxy* const pAudioProxy = static_cast<IEntityAudioProxy*>(pEntity->CreateProxy(ENTITY_PROXY_AUDIO).get());
		if (pAudioProxy != NULL)
		{
			for (TAudioParameterMap::const_iterator iSoundPair = rParamMap.begin(), iSoundPairEnd = rParamMap.end(); iSoundPair != iSoundPairEnd; ++iSoundPair)
			{
				pAudioProxy->StopTrigger(iSoundPair->second.oAudioControls.nTriggerID);
				pAudioProxy->SetRtpcValue(iSoundPair->second.oAudioControls.nRtpcID, 0.0f);
			}

			rParamMap.clear();
		}
	}
}

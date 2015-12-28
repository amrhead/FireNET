////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2011.
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "ICryMannequin.h"
#include <Mannequin/Serialization.h>
#include <CryExtension/Impl/ClassWeaver.h>

/*
	TEMPORARY_SOUND_FLAGS wraps soundFlags which are used to specify either
	an audio should be triggered on the local player or not.
	This game specific data should be moved to the game once a Wwise switch
	is setup, so that only one audio trigger is needed and Wwise sort it
	internally out which sound to play.
*/
#define TEMPORARY_SOUND_FLAGS

class CAudioContext : public IProceduralContext
{
private:

	typedef IProceduralContext BaseClass;

public:

	PROCEDURAL_CONTEXT(CAudioContext, "AudioContext", 0xC6C087F64CE14854, 0xADCA544D252834BD);
	
	virtual void Initialise(IEntity &entity, IActionController &actionController)
	{
		BaseClass::Initialise(entity, actionController);

		m_pIEntityAudioProxy = crycomponent_cast<IEntityAudioProxyPtr> (entity.CreateProxy(ENTITY_PROXY_AUDIO));
	}

	virtual void Update(float timePassed)
	{
	}

	void ExecuteAudioTrigger(TAudioControlID const nID, bool playFacial)
	{
		if (m_pIEntityAudioProxy != NULL)
		{
			m_pIEntityAudioProxy->ExecuteTrigger(nID, playFacial ? eLSM_MatchAnimationToSoundName : eLSM_None);
		}
	}

	void StopAudioTrigger(TAudioControlID const nID)
	{
		if (m_pIEntityAudioProxy != NULL)
		{
			m_pIEntityAudioProxy->StopTrigger(nID);
		}
	}

	void SetAudioObjectPos(QuatT const& rOffset)
	{
		if (m_pIEntityAudioProxy != NULL)
		{
			m_pIEntityAudioProxy->SetAuxAudioProxyOffset(SATLWorldPosition(Matrix34(IDENTITY, rOffset.t), ZERO));
		}
	}

private:

	IEntityAudioProxyPtr m_pIEntityAudioProxy;
};

CAudioContext::CAudioContext() : m_pIEntityAudioProxy()
{
}

CAudioContext::~CAudioContext()
{
}

CRYREGISTER_CLASS(CAudioContext);

#if defined(TEMPORARY_SOUND_FLAGS)
typedef enum
{
	ePSF_LocalPlayerOnly		= BIT(0),		//only play this sound if we are the local player entity
	ePSF_ExcludeLocalPlayer		= BIT(1),		//don't play this sound if we are the local player
} ePlaySoundFlags;
#endif

struct SAudioParams : public IProceduralParams
{
	SAudioParams()
		: radius(0.f)
		, isVoice(false)
		, playFacial(false)
#if defined(TEMPORARY_SOUND_FLAGS)
		, soundFlags(0)
#endif
	{
	}

	virtual void Serialize(Serialization::IArchive& ar)
	{
		ar(Serialization::Decorators::SoundName<TProcClipString>(startTrigger), "StartTrigger", "Start Trigger");
		ar(Serialization::Decorators::SoundName<TProcClipString>(stopTrigger), "StopTrigger", "Stop Trigger");
		ar(Serialization::Decorators::JointName<SProcDataCRC>(attachmentJoint), "AttachmentJoint", "Attachment Joint");
		ar(radius, "Radius", "AI Radius");
		ar(isVoice, "IsVoice", "Is Voice");
		ar(playFacial, "PlayFacial", "Play Facial");
#if defined(TEMPORARY_SOUND_FLAGS)
		ar(soundFlags, "SoundFlags", "Sound Flags");
#endif
	}

	virtual void GetExtraDebugInfo(StringWrapper& extraInfoOut) const OVERRIDE
	{
		extraInfoOut = startTrigger.c_str();
	}

	TProcClipString startTrigger;
	TProcClipString stopTrigger;
	SProcDataCRC attachmentJoint;
	float radius;
	bool synchStop;
	bool forceStopOnExit;
	bool isVoice;
	bool playFacial;

#if defined(TEMPORARY_SOUND_FLAGS)
	uint32 soundFlags;
#endif
};

class CProceduralClipAudio : public TProceduralContextualClip<CAudioContext, SAudioParams>
{
public:
	CProceduralClipAudio()
		: m_nAudioControlIDStart(INVALID_AUDIO_CONTROL_ID)
		, m_nAudioControlIDStop(INVALID_AUDIO_CONTROL_ID)
	{
	}

private:
	struct SAudioParamInfo
	{
		SAudioParamInfo() : paramIndex(-1), paramCRC(0), paramValue(-1.f) {}
		SAudioParamInfo(int _index, uint32 _crc, float _value) : paramIndex(_index), paramCRC(_crc), paramValue(_value) {}

		int    paramIndex;
		uint32 paramCRC;
		float  paramValue;
	};

	typedef std::vector<SAudioParamInfo> TAudioParamVec;

public:
	virtual void OnEnter(float blendTime, float duration, const SAudioParams &params)
	{
#if defined(TEMPORARY_SOUND_FLAGS)
		if (params.soundFlags)
		{
			bool bIsLocalPlayer = false;

			if (IGameFramework* gameFramework = gEnv->pGame->GetIGameFramework())
			{
				if (gameFramework->GetClientActorId() == m_entity->GetId())
				{
					bIsLocalPlayer = true;
				}
			}

			if ((params.soundFlags & ePSF_LocalPlayerOnly) && !bIsLocalPlayer)
			{
				return;
			}

			if ((params.soundFlags & ePSF_ExcludeLocalPlayer) && bIsLocalPlayer)
			{
				return;
			}
		}
#endif

		m_referenceJointID = -1;
		bool const playFacial = params.playFacial;
		bool const isVoice    = playFacial || params.isVoice;

		bool bIsSilentPlaybackMode = gEnv->IsEditor() && gEnv->pGame->GetIGameFramework()->GetMannequinInterface().IsSilentPlaybackMode();

		ICharacterInstance const* const pCharacterInstance = m_scope->GetCharInst();

		if (pCharacterInstance != NULL)
		{
			m_referenceJointID = pCharacterInstance->GetIDefaultSkeleton().GetJointIDByCRC32(params.attachmentJoint.ToUInt32());
		}

		QuatT const soundOffset = GetBoneAbsLocationByID(m_referenceJointID);

		if (!bIsSilentPlaybackMode)
		{
			if (!params.startTrigger.empty())
			{
				gEnv->pAudioSystem->GetAudioTriggerID(params.startTrigger.c_str(), m_nAudioControlIDStart);

				if (m_nAudioControlIDStart != INVALID_AUDIO_CONTROL_ID)
				{
					m_context->ExecuteAudioTrigger(m_nAudioControlIDStart, playFacial);
				}
			}

			if (!params.stopTrigger.empty())
			{
				gEnv->pAudioSystem->GetAudioTriggerID(params.stopTrigger.c_str(), m_nAudioControlIDStop);
			}
		}
	}

	virtual void OnExit(float blendTime)
	{
		if (m_nAudioControlIDStop != INVALID_AUDIO_CONTROL_ID)
		{
			m_context->ExecuteAudioTrigger(m_nAudioControlIDStop, false);
		}
		else if (m_nAudioControlIDStart != INVALID_AUDIO_CONTROL_ID)
		{
			m_context->StopAudioTrigger(m_nAudioControlIDStart);
		}

		m_nAudioControlIDStart	= INVALID_AUDIO_CONTROL_ID;
		m_nAudioControlIDStop		= INVALID_AUDIO_CONTROL_ID;
	}

	virtual void Update(float timePassed)
	{
		UpdateSoundParams();

		UpdateSoundPosition();
	}

private:

	void UpdateSoundParams()
	{
		if (!m_oAudioParams.empty())
		{
			const size_t numParams = m_oAudioParams.size();
			for(size_t i = 0; i < numParams; i++)
			{
				float paramValue = 0.f;
				if(GetParam(m_oAudioParams[i].paramCRC, paramValue) && (m_oAudioParams[i].paramValue != paramValue))
				{
					REINST("update RTPCs")
					//pSound->SetParam(m_soundParams[i].paramIndex, paramValue);
					m_oAudioParams[i].paramValue = paramValue;
				}
			}
		}
	}

	void UpdateSoundPosition()
	{
		if(m_referenceJointID < 0)
			return;

		m_context->SetAudioObjectPos(GetBoneAbsLocationByID(m_referenceJointID));
	}

	QuatT GetBoneAbsLocationByID( const int jointID )
	{
		ICharacterInstance* pCharacterInstance = m_scope->GetCharInst();
		if ((pCharacterInstance != NULL) && (jointID >= 0))
		{
			return pCharacterInstance->GetISkeletonPose()->GetAbsJointByID(jointID);
		}

		return QuatT(ZERO, IDENTITY);
	}

	TAudioParamVec m_oAudioParams;

	int  m_referenceJointID;

	TAudioControlID m_nAudioControlIDStart;
	TAudioControlID m_nAudioControlIDStop;
};

typedef CProceduralClipAudio CProceduralClipPlaySound;
REGISTER_PROCEDURAL_CLIP(CProceduralClipPlaySound, "PlaySound");
REGISTER_PROCEDURAL_CLIP(CProceduralClipAudio, "Audio");

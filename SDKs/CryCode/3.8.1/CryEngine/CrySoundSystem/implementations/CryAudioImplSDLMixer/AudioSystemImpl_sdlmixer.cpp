// CryEngine Source File.
// Copyright (C), Crytek, 1999-2014.

#include "stdafx.h"
#include "AudioSystemImpl_sdlmixer.h"
#include "AudioSystemImplCVars_sdlmixer.h"
#include <CryFile.h>
#include <CryPath.h>
#include "SDLMixerSoundEngineUtil.h"
#include <IAudioSystem.h>
#include "SDLMixerSoundEngineTypes.h"

// SDL / SDL Mixer
#include <SDL.h>
#include <SDL_rwops.h>
#include <SDL_mixer.h>

char const* const	CAudioSystemImpl_sdlmixer::ms_sSDLFileTag = "SDLMixerSample";
char const* const	CAudioSystemImpl_sdlmixer::ms_sSDLCommonAttribute = "sdl_name";
char const* const	CAudioSystemImpl_sdlmixer::ms_sSDLEventIdTag = "event_id";
char const* const	CAudioSystemImpl_sdlmixer::ms_sSDLEventTag = "SDLMixerEvent";
char const* const	CAudioSystemImpl_sdlmixer::ms_sSDLSoundLibraryPath = "/sounds/sdlmixer/";
char const* const	CAudioSystemImpl_sdlmixer::ms_sSDLEventTypeTag = "event_type";
char const* const	CAudioSystemImpl_sdlmixer::ms_sSDLEventPanningEnabledTag = "enable_panning";
char const* const	CAudioSystemImpl_sdlmixer::ms_sSDLEventAttenuationEnabledTag = "enable_distance_attenuation";
char const* const	CAudioSystemImpl_sdlmixer::ms_sSDLEventAttenuationMinDistanceTag = "attenuation_dist_min";
char const* const	CAudioSystemImpl_sdlmixer::ms_sSDLEventAttenuationMaxDistanceTag = "attenuation_dist_max";
char const* const	CAudioSystemImpl_sdlmixer::ms_sSDLEventVolumeTag = "volume";
char const* const	CAudioSystemImpl_sdlmixer::ms_sSDLEventLoopCountTag = "loop_count";

void EndEventCallback(TAudioEventID nEventID)
{
	SAudioRequest oRequest;
	SAudioCallbackManagerRequestData<eACMRT_REPORT_FINISHED_EVENT> oRequestData(nEventID, true);
	oRequest.nFlags	= eARF_THREAD_SAFE_PUSH;
	oRequest.pData	= &oRequestData;
	gEnv->pAudioSystem->PushRequest(oRequest);
}

CAudioSystemImpl_sdlmixer::CAudioSystemImpl_sdlmixer()
{
	m_sGameFolder = PathUtil::GetGameFolder();
	if (m_sGameFolder.empty())
	{
		CryFatalError("<Audio - SDLMixer>: Needs a valid game folder to proceed!");
	}

#if defined(INCLUDE_SDLMIXER_IMPL_PRODUCTION_CODE)
	m_sFullImplString = "SDL Mixer 2.0 (";
	m_sFullImplString += m_sGameFolder + PathUtil::RemoveSlash(ms_sSDLSoundLibraryPath) + ")";
#endif // INCLUDE_SDLMIXER_IMPL_PRODUCTION_CODE

#if defined(WIN32) || defined(WIN64)
	m_nMemoryAlignment = 16;
#elif defined(MAC)
	m_nMemoryAlignment = 16;
#elif defined(LINUX) && !defined(ANDROID)
	m_nMemoryAlignment = 16;
#elif defined(IOS)
	m_nMemoryAlignment = 16;
#elif defined(ANDROID)
	m_nMemoryAlignment = 16;
#else
#error "Undefined platform."
#endif
}

CAudioSystemImpl_sdlmixer::~CAudioSystemImpl_sdlmixer()
{
}

void CAudioSystemImpl_sdlmixer::Update(float const fUpdateIntervalMS)
{
	SDLMixer::SoundEngine::Update();
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::Init()
{
	if (SDLMixer::SoundEngine::Init())
	{
		SDLMixer::SoundEngine::RegisterEventFinishedCallback(EndEventCallback);
		return eARS_SUCCESS;
	}

	return eARS_FAILURE;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::ShutDown()
{
	return eARS_SUCCESS;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::Release()
{
	SDLMixer::SoundEngine::Release();
	POOL_FREE(this);

	// Freeing Memory Pool Memory again
	uint8* pMemSystem = g_SDLMixerImplMemoryPool.Data();
	g_SDLMixerImplMemoryPool.UnInitMem();

	if (pMemSystem)
	{
		delete[](uint8*)(pMemSystem);
	}

	g_SDLMixerImplCVars.UnregisterVariables();

	return eARS_SUCCESS;
}

void CAudioSystemImpl_sdlmixer::OnAudioSystemRefresh()
{
	SDLMixer::SoundEngine::Refresh();
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::OnLoseFocus()
{
	SDLMixer::SoundEngine::Pause();
	return eARS_SUCCESS;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::OnGetFocus()
{
	SDLMixer::SoundEngine::Resume();
	return eARS_SUCCESS;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::MuteAll()
{
	SDLMixer::SoundEngine::Mute();
	return eARS_SUCCESS;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::UnmuteAll()
{
	SDLMixer::SoundEngine::UnMute();
	return eARS_SUCCESS;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::StopAllSounds()
{
	SDLMixer::SoundEngine::Stop();
	return eARS_SUCCESS;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::RegisterAudioObject(IATLAudioObjectData* const pObjectData, char const* const sObjectName)
{
	SSDLMixerAudioObjectData* const pSDLMixerObject = static_cast<SSDLMixerAudioObjectData* const>(pObjectData);

	if (pSDLMixerObject)
	{
		SDLMixer::SoundEngine::RegisterAudioObject(pSDLMixerObject);
#if defined(INCLUDE_SDLMIXER_IMPL_PRODUCTION_CODE)
		m_idToName[pSDLMixerObject->nObjectID] = sObjectName;
#endif
		return eARS_SUCCESS;
	}
	return eARS_FAILURE;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::RegisterAudioObject(IATLAudioObjectData* const pObjectData)
{
	SSDLMixerAudioObjectData* const pSDLMixerObject = static_cast<SSDLMixerAudioObjectData* const>(pObjectData);
	if (pSDLMixerObject)
	{
		SDLMixer::SoundEngine::RegisterAudioObject(pSDLMixerObject);
		return eARS_SUCCESS;
	}
	return eARS_FAILURE;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::UnregisterAudioObject(IATLAudioObjectData* const pObjectData)
{
	SSDLMixerAudioObjectData* const pSDLMixerObject = static_cast<SSDLMixerAudioObjectData* const>(pObjectData);
	if (pSDLMixerObject)
	{
		SDLMixer::SoundEngine::UnregisterAudioObject(pSDLMixerObject);
		return eARS_SUCCESS;
	}
	return eARS_FAILURE;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::ResetAudioObject(IATLAudioObjectData* const pObjectData)
{
	return eARS_SUCCESS;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::UpdateAudioObject(IATLAudioObjectData* const pObjectData)
{
	return eARS_SUCCESS;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::PrepareTriggerSync(IATLAudioObjectData* const pAudioObjectData, IATLTriggerImplData const* const pTriggerData)
{
	return eARS_SUCCESS;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::UnprepareTriggerSync(IATLAudioObjectData* const pAudioObjectData, IATLTriggerImplData const* const pTriggerData)
{
	return eARS_SUCCESS;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::PrepareTriggerAsync(IATLAudioObjectData* const pAudioObjectData, IATLTriggerImplData const* const pTriggerData, IATLEventData* const pEventData)
{
	return eARS_SUCCESS;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::UnprepareTriggerAsync(IATLAudioObjectData* const pAudioObjectData, IATLTriggerImplData const* const pTriggerData, IATLEventData* const pEventData)
{
	return eARS_SUCCESS;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::ActivateTrigger(IATLAudioObjectData* const pAudioObjectData, IATLTriggerImplData const* const pTriggerData, IATLEventData* const pEventData)
{
	EAudioRequestStatus eResult = eARS_FAILURE;

	SSDLMixerAudioObjectData* const pSDLAudioObjectData = static_cast<SSDLMixerAudioObjectData* const>(pAudioObjectData);
	SSDLMixerEventStaticData const* const pSDLEventStaticData = static_cast<SSDLMixerEventStaticData const* const>(pTriggerData);
	SSDLMixerEventInstanceData* const pSDLEventInstanceData = static_cast<SSDLMixerEventInstanceData* const>(pEventData);

	if ((pSDLAudioObjectData != NPTR) && (pSDLEventStaticData != NPTR) && (pSDLEventInstanceData != NPTR))
	{
		if (SDLMixer::SoundEngine::ExecuteEvent(pSDLAudioObjectData, pSDLEventStaticData, pSDLEventInstanceData))
		{
			eResult = eARS_SUCCESS;
		}
	}
	return eResult;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::StopEvent(IATLAudioObjectData* const pAudioObjectData, IATLEventData const* const pEventData)
{
	EAudioRequestStatus eResult = eARS_FAILURE;
	SSDLMixerEventInstanceData const* const pSDLEventInstanceData = static_cast<SSDLMixerEventInstanceData const* const>(pEventData);

	if (pSDLEventInstanceData != NPTR)
	{
		if (SDLMixer::SoundEngine::StopEvent(pSDLEventInstanceData))
		{
			eResult = eARS_SUCCESS;
		}
	}
	return eResult;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::StopAllEvents(IATLAudioObjectData* const pAudioObjectData)
{
	return eARS_SUCCESS;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::SetPosition(IATLAudioObjectData* const pAudioObjectData, SATLWorldPosition const& oWorldPosition)
{
	SSDLMixerAudioObjectData* const pSDLMixerObject = static_cast<SSDLMixerAudioObjectData* const>(pAudioObjectData);
	if (pSDLMixerObject)
	{
		SDLMixer::SoundEngine::SetAudioObjectPosition(pSDLMixerObject, oWorldPosition);
		return eARS_SUCCESS;
	}
	return eARS_FAILURE;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::SetRtpc(IATLAudioObjectData* const pAudioObjectData, IATLRtpcImplData const* const pRtpcData, float const fValue)
{
	return eARS_SUCCESS;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::SetSwitchState(IATLAudioObjectData* const pAudioObjectData, IATLSwitchStateImplData const* const pSwitchStateData)
{
	return eARS_SUCCESS;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::SetObstructionOcclusion(IATLAudioObjectData* const pAudioObjectData, float const fObstruction, float const fOcclusion)
{
	return eARS_SUCCESS;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::SetEnvironment(IATLAudioObjectData* const pAudioObjectData, IATLEnvironmentImplData const* const pEnvironmentImplData, float const fAmount)
{
	return eARS_SUCCESS;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::SetListenerPosition(IATLListenerData* const pListenerData, SATLWorldPosition const& oNewPosition)
{
	SATLListenerData* const pListener = static_cast<SATLListenerData* const>(pListenerData);
	if (pListener)
	{
		SDLMixer::SoundEngine::SetListenerPosition(pListener->nListenerID, oNewPosition);
		return eARS_SUCCESS;
	}
	return eARS_FAILURE;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::RegisterInMemoryFile(SATLAudioFileEntryInfo* const pAudioFileEntry)
{
	EAudioRequestStatus eResult = eARS_FAILURE;

	if (pAudioFileEntry != NPTR)
	{
		SSDLMixerAudioFileEntryData* const pFileData = static_cast<SSDLMixerAudioFileEntryData*>(pAudioFileEntry->pImplData);

		if (pFileData != NPTR)
		{
			pFileData->nSampleID = SDLMixer::SoundEngine::LoadSample(pAudioFileEntry->pFileData, pAudioFileEntry->nSize, pAudioFileEntry->sFileName);
			eResult = eARS_SUCCESS;
		}
		else
		{
			g_SDLMixerImplLogger.Log(eALT_ERROR, "Invalid AudioFileEntryData passed to the SDL Mixer implementation of RegisterInMemoryFile");
		}
	}

	return eResult;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::UnregisterInMemoryFile(SATLAudioFileEntryInfo* const pAudioFileEntry)
{
	EAudioRequestStatus eResult = eARS_FAILURE;

	if (pAudioFileEntry != NPTR)
	{
		SSDLMixerAudioFileEntryData* const pFileData = static_cast<SSDLMixerAudioFileEntryData*>(pAudioFileEntry->pImplData);

		if (pFileData != NPTR)
		{
			SDLMixer::SoundEngine::UnloadSample(pFileData->nSampleID);
			eResult = eARS_SUCCESS;
		}
		else
		{
			g_SDLMixerImplLogger.Log(eALT_ERROR, "Invalid AudioFileEntryData passed to the SDL Mixer implementation of UnregisterInMemoryFile");
		}
	}

	return eResult;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::ParseAudioFileEntry(XmlNodeRef const pAudioFileEntryNode, SATLAudioFileEntryInfo* const pFileEntryInfo)
{
	EAudioRequestStatus eResult = eARS_FAILURE;

	if ((_stricmp(pAudioFileEntryNode->getTag(), ms_sSDLFileTag) == 0) && (pFileEntryInfo != NPTR))
	{
		char const* const sFileName = pAudioFileEntryNode->getAttr(ms_sSDLCommonAttribute);

		// Currently the SDLMixer Implementation does not support localized files.
		pFileEntryInfo->bLocalized = false;

		if (sFileName != NPTR && sFileName[0] != '\0')
		{
			pFileEntryInfo->sFileName = sFileName;
			pFileEntryInfo->nMemoryBlockAlignment	= m_nMemoryAlignment;
			POOL_NEW(SSDLMixerAudioFileEntryData, pFileEntryInfo->pImplData);

			eResult = eARS_SUCCESS;
		}
		else
		{
			pFileEntryInfo->sFileName = NPTR;
			pFileEntryInfo->nMemoryBlockAlignment	= 0;
			pFileEntryInfo->pImplData = NPTR;
		}
	}

	return eResult;
}

void CAudioSystemImpl_sdlmixer::DeleteAudioFileEntryData(IATLAudioFileEntryData* const pOldAudioFileEntryData)
{
	POOL_FREE(pOldAudioFileEntryData);
}

char const* const CAudioSystemImpl_sdlmixer::GetAudioFileLocation(SATLAudioFileEntryInfo* const pFileEntryInfo)
{
	static CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH> sPath;
	sPath = m_sGameFolder.c_str();
	sPath += ms_sSDLSoundLibraryPath;

	return sPath.c_str();
}

IATLTriggerImplData const* CAudioSystemImpl_sdlmixer::NewAudioTriggerImplData(XmlNodeRef const pAudioTriggerNode)
{
	SSDLMixerEventStaticData* pNewTriggerImpl = NPTR;
	if (_stricmp(pAudioTriggerNode->getTag(), ms_sSDLEventTag) == 0)
	{
		char const* const sFileName = pAudioTriggerNode->getAttr(ms_sSDLCommonAttribute);
		SDLMixer::TSDLMixerID sampleID = SDLMixer::GetIDFromString(sFileName);

		SDLMixer::TSDLMixerID eventID = sampleID;
		if (pAudioTriggerNode->haveAttr(ms_sSDLEventIdTag))
		{
			pAudioTriggerNode->getAttr(ms_sSDLEventIdTag, eventID);
		}

		pNewTriggerImpl = SDLMixer::SoundEngine::CreateEventData(eventID);
		if (pNewTriggerImpl)
		{
			pNewTriggerImpl->samples.push_back(sampleID);

			pNewTriggerImpl->bStartEvent = (_stricmp(pAudioTriggerNode->getAttr(ms_sSDLEventTypeTag), "stop") != 0);
			pNewTriggerImpl->bPanningEnabled = (_stricmp(pAudioTriggerNode->getAttr(ms_sSDLEventPanningEnabledTag), "true") == 0);
			bool bAttenuationEnabled = (_stricmp(pAudioTriggerNode->getAttr(ms_sSDLEventAttenuationEnabledTag), "true") == 0);
			if (bAttenuationEnabled)
			{
				pAudioTriggerNode->getAttr(ms_sSDLEventAttenuationMinDistanceTag, pNewTriggerImpl->fAttenuationMinDistance);
				pAudioTriggerNode->getAttr(ms_sSDLEventAttenuationMaxDistanceTag, pNewTriggerImpl->fAttenuationMaxDistance);
			}
			else
			{
				pNewTriggerImpl->fAttenuationMinDistance = -1.0f;
				pNewTriggerImpl->fAttenuationMaxDistance = -1.0f;
			}

			// Translate decibel to normalized value.
			static const int nMaxVolume = 128;
			float fVolume = 0.0f;
			pAudioTriggerNode->getAttr(ms_sSDLEventVolumeTag, fVolume);
			pNewTriggerImpl->nVolume = static_cast<int>(pow_tpl(10.0f, fVolume / 20.0f) * nMaxVolume);

			pAudioTriggerNode->getAttr(ms_sSDLEventLoopCountTag, pNewTriggerImpl->nLoopCount);
		}
	}

	return pNewTriggerImpl;
}

void CAudioSystemImpl_sdlmixer::DeleteAudioTriggerImplData(IATLTriggerImplData const* const pOldTriggerImplData)
{
	SSDLMixerEventStaticData const* const pSDLEventStaticData = static_cast<SSDLMixerEventStaticData const* const>(pOldTriggerImplData);
	if(pSDLEventStaticData)
	{
		SDLMixer::SoundEngine::StopEvents(pSDLEventStaticData->nEventID);
	}
	POOL_FREE_CONST(pOldTriggerImplData);
}

IATLRtpcImplData const* CAudioSystemImpl_sdlmixer::NewAudioRtpcImplData(XmlNodeRef const pAudioRtpcNode)
{
	SATLRtpcImplData_sdlmixer* pNewRtpcImpl = NPTR;
	POOL_NEW(SATLRtpcImplData_sdlmixer, pNewRtpcImpl)();
	return pNewRtpcImpl;
}

void CAudioSystemImpl_sdlmixer::DeleteAudioRtpcImplData(IATLRtpcImplData const* const pOldRtpcImplData)
{
	POOL_FREE_CONST(pOldRtpcImplData);
}

IATLSwitchStateImplData const* CAudioSystemImpl_sdlmixer::NewAudioSwitchStateImplData(XmlNodeRef const pAudioSwitchStateImplNode)
{
	SATLSwitchStateImplData_sdlmixer* pNewSwitchImpl = NPTR;
	POOL_NEW(SATLSwitchStateImplData_sdlmixer, pNewSwitchImpl)();
	return pNewSwitchImpl;
}

void CAudioSystemImpl_sdlmixer::DeleteAudioSwitchStateImplData(IATLSwitchStateImplData const* const pOldAudioSwitchStateImplNode)
{
	POOL_FREE_CONST(pOldAudioSwitchStateImplNode);
}

IATLEnvironmentImplData const* CAudioSystemImpl_sdlmixer::NewAudioEnvironmentImplData(XmlNodeRef const pAudioEnvironmentNode)
{
	SATLEnvironmentImplData_sdlmixer* pNewEnvironmentImpl = NPTR;
	POOL_NEW(SATLEnvironmentImplData_sdlmixer, pNewEnvironmentImpl)();
	return pNewEnvironmentImpl;
}

void CAudioSystemImpl_sdlmixer::DeleteAudioEnvironmentImplData(IATLEnvironmentImplData const* const pOldEnvironmentImplData)
{
	POOL_FREE_CONST(pOldEnvironmentImplData);
}

IATLAudioObjectData* CAudioSystemImpl_sdlmixer::NewGlobalAudioObjectData(TAudioObjectID const nObjectID)
{
	POOL_NEW_CREATE(SSDLMixerAudioObjectData, pNewObject)(nObjectID, true);
	return pNewObject;
}

IATLAudioObjectData* CAudioSystemImpl_sdlmixer::NewAudioObjectData(TAudioObjectID const nObjectID)
{
	POOL_NEW_CREATE(SSDLMixerAudioObjectData, pNewObject)(nObjectID, false);
	return pNewObject;
}

void CAudioSystemImpl_sdlmixer::DeleteAudioObjectData(IATLAudioObjectData* const pOldObjectData)
{
	POOL_FREE(pOldObjectData);
}

IATLListenerData* CAudioSystemImpl_sdlmixer::NewDefaultAudioListenerObjectData()
{
	POOL_NEW_CREATE(SATLListenerData, pNewObject)(0);
	return pNewObject;
}

IATLListenerData* CAudioSystemImpl_sdlmixer::NewAudioListenerObjectData(uint const nIndex)
{
	POOL_NEW_CREATE(SATLListenerData, pNewObject)(nIndex);
	return pNewObject;
}

void CAudioSystemImpl_sdlmixer::DeleteAudioListenerObjectData(IATLListenerData* const pOldListenerData)
{
	POOL_FREE(pOldListenerData);
}

IATLEventData* CAudioSystemImpl_sdlmixer::NewAudioEventData(TAudioEventID const nEventID)
{
	POOL_NEW_CREATE(SSDLMixerEventInstanceData, pNewEvent)(nEventID);
	return pNewEvent;
}

void CAudioSystemImpl_sdlmixer::DeleteAudioEventData(IATLEventData* const pOldEventData)
{
	POOL_FREE(pOldEventData);
}

void CAudioSystemImpl_sdlmixer::ResetAudioEventData(IATLEventData* const pEventData)
{
	SSDLMixerEventInstanceData* const pEventInstanceData = static_cast<SSDLMixerEventInstanceData*>(pEventData);

	if (pEventInstanceData != NPTR)
	{
		pEventInstanceData->Reset();
	}
}

void CAudioSystemImpl_sdlmixer::SetLanguage(char const* const sLanguage)
{
	//TODO
}

char const* const CAudioSystemImpl_sdlmixer::GetImplementationNameString() const
{
#if defined(INCLUDE_SDLMIXER_IMPL_PRODUCTION_CODE)
	return m_sFullImplString.c_str();
#endif // INCLUDE_SDLMIXER_IMPL_PRODUCTION_CODE
	return NPTR;
}

void CAudioSystemImpl_sdlmixer::GetMemoryInfo(SAudioImplMemoryInfo& oMemoryInfo) const
{
	oMemoryInfo.nPrimaryPoolSize = g_SDLMixerImplMemoryPool.MemSize();
	oMemoryInfo.nPrimaryPoolUsedSize = oMemoryInfo.nPrimaryPoolSize - g_SDLMixerImplMemoryPool.MemFree();
	oMemoryInfo.nPrimaryPoolAllocations = g_SDLMixerImplMemoryPool.FragmentCount();
	oMemoryInfo.nBucketUsedSize					= g_SDLMixerImplMemoryPool.GetSmallAllocsSize();
	oMemoryInfo.nBucketAllocations			= g_SDLMixerImplMemoryPool.GetSmallAllocsCount();
	oMemoryInfo.nSecondaryPoolSize				= 0;
	oMemoryInfo.nSecondaryPoolUsedSize		= 0;
	oMemoryInfo.nSecondaryPoolAllocations	= 0;
}
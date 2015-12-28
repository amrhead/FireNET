// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef AUDIO_SYSTEM_IMPL_SDLMIXER_H_INCLUDED
#define AUDIO_SYSTEM_IMPL_SDLMIXER_H_INCLUDED

#include <IAudioSystemImplementation.h>
#include "SDLMixerSoundEngine.h"

class CAudioSystemImpl_sdlmixer : public IAudioSystemImplementation
{
public:

	CAudioSystemImpl_sdlmixer();
	virtual ~CAudioSystemImpl_sdlmixer();

	// IAudioSystemImplementation
	VIRTUAL void Update(float const fUpdateIntervalMS);

	VIRTUAL EAudioRequestStatus Init();
	VIRTUAL EAudioRequestStatus ShutDown();
	VIRTUAL EAudioRequestStatus Release();
	VIRTUAL void OnAudioSystemRefresh();
	VIRTUAL EAudioRequestStatus OnLoseFocus();
	VIRTUAL EAudioRequestStatus OnGetFocus();
	VIRTUAL EAudioRequestStatus MuteAll();
	VIRTUAL EAudioRequestStatus UnmuteAll();
	VIRTUAL EAudioRequestStatus StopAllSounds();

	VIRTUAL EAudioRequestStatus RegisterAudioObject(IATLAudioObjectData* const pObjectData, char const* const sObjectName);
	VIRTUAL EAudioRequestStatus	RegisterAudioObject(IATLAudioObjectData* const pObjectData);
	VIRTUAL EAudioRequestStatus	UnregisterAudioObject(IATLAudioObjectData* const pObjectData);
	VIRTUAL EAudioRequestStatus ResetAudioObject(IATLAudioObjectData* const pObjectData);
	VIRTUAL EAudioRequestStatus UpdateAudioObject(IATLAudioObjectData* const pObjectData);
	VIRTUAL EAudioRequestStatus PrepareTriggerSync(IATLAudioObjectData* const pAudioObjectData, IATLTriggerImplData const* const pTriggerData);
	VIRTUAL EAudioRequestStatus UnprepareTriggerSync(IATLAudioObjectData* const pAudioObjectData, IATLTriggerImplData const* const pTriggerData);
	VIRTUAL EAudioRequestStatus PrepareTriggerAsync(IATLAudioObjectData* const pAudioObjectData, IATLTriggerImplData const* const pTriggerData, IATLEventData* const pEventData);
	VIRTUAL EAudioRequestStatus	UnprepareTriggerAsync(IATLAudioObjectData* const pAudioObjectData, IATLTriggerImplData const* const pTriggerData, IATLEventData* const pEventData);
	VIRTUAL EAudioRequestStatus ActivateTrigger(IATLAudioObjectData* const pAudioObjectData, IATLTriggerImplData const* const pTriggerData, IATLEventData* const pEventData);
	VIRTUAL EAudioRequestStatus StopEvent(IATLAudioObjectData* const pAudioObjectData, IATLEventData const* const pEventData);
	VIRTUAL EAudioRequestStatus StopAllEvents(IATLAudioObjectData* const pAudioObjectData);
	VIRTUAL EAudioRequestStatus SetPosition(IATLAudioObjectData* const pAudioObjectData, SATLWorldPosition const& oWorldPosition);
	VIRTUAL EAudioRequestStatus	SetRtpc(IATLAudioObjectData* const pAudioObjectData, IATLRtpcImplData const* const pRtpcData, float const fValue);
	VIRTUAL EAudioRequestStatus SetSwitchState(IATLAudioObjectData* const pAudioObjectData, IATLSwitchStateImplData const* const pSwitchStateData);
	VIRTUAL EAudioRequestStatus SetObstructionOcclusion(IATLAudioObjectData* const pAudioObjectData, float const fObstruction, float const fOcclusion);
	VIRTUAL EAudioRequestStatus SetEnvironment(IATLAudioObjectData* const pAudioObjectData, IATLEnvironmentImplData const* const pEnvironmentImplData, float const fAmount);
	VIRTUAL EAudioRequestStatus SetListenerPosition(IATLListenerData* const pListenerData, SATLWorldPosition const& oNewPosition);
	VIRTUAL EAudioRequestStatus RegisterInMemoryFile(SATLAudioFileEntryInfo* const pAudioFileEntry);
	VIRTUAL EAudioRequestStatus	UnregisterInMemoryFile(SATLAudioFileEntryInfo* const pAudioFileEntry);
	VIRTUAL EAudioRequestStatus ParseAudioFileEntry(XmlNodeRef const pAudioFileEntryNode, SATLAudioFileEntryInfo* const pFileEntryInfo);

	VIRTUAL void DeleteAudioFileEntryData(IATLAudioFileEntryData* const pOldAudioFileEntryData);
	VIRTUAL char const* const	GetAudioFileLocation(SATLAudioFileEntryInfo* const pFileEntryInfo);
	VIRTUAL IATLTriggerImplData const* NewAudioTriggerImplData(XmlNodeRef const pAudioTriggerNode);
	VIRTUAL void DeleteAudioTriggerImplData(IATLTriggerImplData const* const pOldTriggerImplData);
	VIRTUAL IATLRtpcImplData const* NewAudioRtpcImplData(XmlNodeRef const pAudioRtpcNode);
	VIRTUAL void DeleteAudioRtpcImplData(IATLRtpcImplData const* const pOldRtpcImplData);
	VIRTUAL IATLSwitchStateImplData const* NewAudioSwitchStateImplData(XmlNodeRef const pAudioSwitchStateImplNode);
	VIRTUAL void DeleteAudioSwitchStateImplData(IATLSwitchStateImplData const* const pOldAudioSwitchStateImplNode);
	VIRTUAL IATLEnvironmentImplData const* NewAudioEnvironmentImplData(XmlNodeRef const pAudioEnvironmentNode);
	VIRTUAL void DeleteAudioEnvironmentImplData(IATLEnvironmentImplData const* const pOldEnvironmentImplData);
	VIRTUAL IATLAudioObjectData* NewGlobalAudioObjectData(TAudioObjectID const nObjectID);
	VIRTUAL IATLAudioObjectData* NewAudioObjectData(TAudioObjectID const nObjectID);
	VIRTUAL void DeleteAudioObjectData(IATLAudioObjectData* const pOldObjectData);
	VIRTUAL IATLListenerData* NewDefaultAudioListenerObjectData();
	VIRTUAL IATLListenerData* NewAudioListenerObjectData(uint const nIndex);
	VIRTUAL void DeleteAudioListenerObjectData(IATLListenerData* const pOldListenerData);
	VIRTUAL IATLEventData* NewAudioEventData(TAudioEventID const nEventID);
	VIRTUAL void DeleteAudioEventData(IATLEventData* const pOldEventData);
	VIRTUAL void ResetAudioEventData(IATLEventData* const pEventData);
	VIRTUAL void SetLanguage(char const* const sLanguage);
	VIRTUAL char const* const GetImplementationNameString() const;
	VIRTUAL void GetMemoryInfo(SAudioImplMemoryInfo& oMemoryInfo) const;
	// ~IAudioSystemImplementation

private:

	static char const* const	ms_sSDLFileTag;
	static char const* const	ms_sSDLEventTag;
	static char const* const	ms_sSDLCommonAttribute;
	static char const* const	ms_sSDLSoundLibraryPath;
	static char const* const	ms_sSDLEventTypeTag;
	static char const* const	ms_sSDLEventPanningEnabledTag;
	static char const* const	ms_sSDLEventAttenuationEnabledTag;
	static char const* const	ms_sSDLEventAttenuationMinDistanceTag;
	static char const* const	ms_sSDLEventAttenuationMaxDistanceTag;
	static char const* const	ms_sSDLEventVolumeTag;
	static char const* const	ms_sSDLEventLoopCountTag;

	string m_sGameFolder;
	size_t m_nMemoryAlignment;

#if defined(INCLUDE_SDLMIXER_IMPL_PRODUCTION_CODE)
	std::map<TAudioObjectID, string> m_idToName;
	CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH> m_sFullImplString;
#endif // INCLUDE_SDLMIXER_IMPL_PRODUCTION_CODE

};

#endif // AUDIO_SYSTEM_IMPL_SDLMIXER_H_INCLUDED

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
	virtual void Update(float const fUpdateIntervalMS);

	virtual EAudioRequestStatus Init();
	virtual EAudioRequestStatus ShutDown();
	virtual EAudioRequestStatus Release();
	virtual void OnAudioSystemRefresh();
	virtual EAudioRequestStatus OnLoseFocus();
	virtual EAudioRequestStatus OnGetFocus();
	virtual EAudioRequestStatus MuteAll();
	virtual EAudioRequestStatus UnmuteAll();
	virtual EAudioRequestStatus StopAllSounds();

	virtual EAudioRequestStatus RegisterAudioObject(IATLAudioObjectData* const pObjectData, char const* const sObjectName);
	virtual EAudioRequestStatus	RegisterAudioObject(IATLAudioObjectData* const pObjectData);
	virtual EAudioRequestStatus	UnregisterAudioObject(IATLAudioObjectData* const pObjectData);
	virtual EAudioRequestStatus ResetAudioObject(IATLAudioObjectData* const pObjectData);
	virtual EAudioRequestStatus UpdateAudioObject(IATLAudioObjectData* const pObjectData);
	virtual EAudioRequestStatus PrepareTriggerSync(IATLAudioObjectData* const pAudioObjectData, IATLTriggerImplData const* const pTriggerData);
	virtual EAudioRequestStatus UnprepareTriggerSync(IATLAudioObjectData* const pAudioObjectData, IATLTriggerImplData const* const pTriggerData);
	virtual EAudioRequestStatus PrepareTriggerAsync(IATLAudioObjectData* const pAudioObjectData, IATLTriggerImplData const* const pTriggerData, IATLEventData* const pEventData);
	virtual EAudioRequestStatus	UnprepareTriggerAsync(IATLAudioObjectData* const pAudioObjectData, IATLTriggerImplData const* const pTriggerData, IATLEventData* const pEventData);
	virtual EAudioRequestStatus ActivateTrigger(IATLAudioObjectData* const pAudioObjectData, IATLTriggerImplData const* const pTriggerData, IATLEventData* const pEventData);
	virtual EAudioRequestStatus StopEvent(IATLAudioObjectData* const pAudioObjectData, IATLEventData const* const pEventData);
	virtual EAudioRequestStatus StopAllEvents(IATLAudioObjectData* const pAudioObjectData);
	virtual EAudioRequestStatus SetPosition(IATLAudioObjectData* const pAudioObjectData, SATLWorldPosition const& oWorldPosition);
	virtual EAudioRequestStatus	SetRtpc(IATLAudioObjectData* const pAudioObjectData, IATLRtpcImplData const* const pRtpcData, float const fValue);
	virtual EAudioRequestStatus SetSwitchState(IATLAudioObjectData* const pAudioObjectData, IATLSwitchStateImplData const* const pSwitchStateData);
	virtual EAudioRequestStatus SetObstructionOcclusion(IATLAudioObjectData* const pAudioObjectData, float const fObstruction, float const fOcclusion);
	virtual EAudioRequestStatus SetEnvironment(IATLAudioObjectData* const pAudioObjectData, IATLEnvironmentImplData const* const pEnvironmentImplData, float const fAmount);
	virtual EAudioRequestStatus SetListenerPosition(IATLListenerData* const pListenerData, SATLWorldPosition const& oNewPosition);
	virtual EAudioRequestStatus RegisterInMemoryFile(SATLAudioFileEntryInfo* const pAudioFileEntry);
	virtual EAudioRequestStatus	UnregisterInMemoryFile(SATLAudioFileEntryInfo* const pAudioFileEntry);
	virtual EAudioRequestStatus ParseAudioFileEntry(XmlNodeRef const pAudioFileEntryNode, SATLAudioFileEntryInfo* const pFileEntryInfo);

	virtual void DeleteAudioFileEntryData(IATLAudioFileEntryData* const pOldAudioFileEntryData);
	virtual char const* const	GetAudioFileLocation(SATLAudioFileEntryInfo* const pFileEntryInfo);
	virtual IATLTriggerImplData const* NewAudioTriggerImplData(XmlNodeRef const pAudioTriggerNode);
	virtual void DeleteAudioTriggerImplData(IATLTriggerImplData const* const pOldTriggerImplData);
	virtual IATLRtpcImplData const* NewAudioRtpcImplData(XmlNodeRef const pAudioRtpcNode);
	virtual void DeleteAudioRtpcImplData(IATLRtpcImplData const* const pOldRtpcImplData);
	virtual IATLSwitchStateImplData const* NewAudioSwitchStateImplData(XmlNodeRef const pAudioSwitchStateImplNode);
	virtual void DeleteAudioSwitchStateImplData(IATLSwitchStateImplData const* const pOldAudioSwitchStateImplNode);
	virtual IATLEnvironmentImplData const* NewAudioEnvironmentImplData(XmlNodeRef const pAudioEnvironmentNode);
	virtual void DeleteAudioEnvironmentImplData(IATLEnvironmentImplData const* const pOldEnvironmentImplData);
	virtual IATLAudioObjectData* NewGlobalAudioObjectData(TAudioObjectID const nObjectID);
	virtual IATLAudioObjectData* NewAudioObjectData(TAudioObjectID const nObjectID);
	virtual void DeleteAudioObjectData(IATLAudioObjectData* const pOldObjectData);
	virtual IATLListenerData* NewDefaultAudioListenerObjectData();
	virtual IATLListenerData* NewAudioListenerObjectData(uint const nIndex);
	virtual void DeleteAudioListenerObjectData(IATLListenerData* const pOldListenerData);
	virtual IATLEventData* NewAudioEventData(TAudioEventID const nEventID);
	virtual void DeleteAudioEventData(IATLEventData* const pOldEventData);
	virtual void ResetAudioEventData(IATLEventData* const pEventData);
	virtual void SetLanguage(char const* const sLanguage);
	virtual char const* const GetImplementationNameString() const;
	virtual void GetMemoryInfo(SAudioImplMemoryInfo& oMemoryInfo) const;
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
	static char const* const	ms_sSDLEventIdTag;

	string m_sGameFolder;
	size_t m_nMemoryAlignment;

#if defined(INCLUDE_SDLMIXER_IMPL_PRODUCTION_CODE)
	std::map<TAudioObjectID, string> m_idToName;
	CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH> m_sFullImplString;
#endif // INCLUDE_SDLMIXER_IMPL_PRODUCTION_CODE

};

#endif // AUDIO_SYSTEM_IMPL_SDLMIXER_H_INCLUDED

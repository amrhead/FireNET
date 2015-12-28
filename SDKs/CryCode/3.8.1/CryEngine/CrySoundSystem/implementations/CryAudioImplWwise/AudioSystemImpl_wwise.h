// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef AUDIO_SYSTEM_IMPL_WWISE_H_INCLUDED
#define AUDIO_SYSTEM_IMPL_WWISE_H_INCLUDED

#include "FileIOHandler_wwise.h"
#include "ATLEntities_wwise.h"
#include <IAudioSystemImplementation.h>

#if defined(WIN64)
#define WWISE_USE_OCULUS
#endif // WIN64

class CAudioSystemImpl_wwise : public IAudioSystemImplementation
{
public:
	
	CAudioSystemImpl_wwise();
	virtual ~CAudioSystemImpl_wwise();

	// IAudioSystemImplementation
	virtual void																	Update(float const fUpdateIntervalMS);

	virtual EAudioRequestStatus										Init();
	virtual EAudioRequestStatus										ShutDown();
	virtual EAudioRequestStatus										Release();
	virtual EAudioRequestStatus										OnLoseFocus();
	virtual EAudioRequestStatus										OnGetFocus();
	virtual EAudioRequestStatus										MuteAll();
	virtual EAudioRequestStatus										UnmuteAll();
	virtual EAudioRequestStatus										StopAllSounds();

	virtual EAudioRequestStatus										RegisterAudioObject(
																									IATLAudioObjectData* const pObjectData, 
																									char const* const sObjectName);
	virtual EAudioRequestStatus										RegisterAudioObject(IATLAudioObjectData* const pObjectData);
	virtual EAudioRequestStatus										UnregisterAudioObject(IATLAudioObjectData* const pObjectData);
	virtual EAudioRequestStatus										ResetAudioObject(IATLAudioObjectData* const pObjectData);
	virtual EAudioRequestStatus										UpdateAudioObject(IATLAudioObjectData* const pObjectData);

	virtual EAudioRequestStatus										PrepareTriggerSync(
																									IATLAudioObjectData* const pAudioObjectData,
																									IATLTriggerImplData const* const pTriggerDataData);
	virtual EAudioRequestStatus										UnprepareTriggerSync(
																									IATLAudioObjectData* const pAudioObjectData,
																									IATLTriggerImplData const* const pTriggerData);
	virtual EAudioRequestStatus										PrepareTriggerAsync(
																									IATLAudioObjectData* const pAudioObjectData,
																									IATLTriggerImplData const* const pTriggerData,
																									IATLEventData* const pEventData);
	virtual EAudioRequestStatus										UnprepareTriggerAsync(
																									IATLAudioObjectData* const pAudioObjectData,
																									IATLTriggerImplData const* const pTriggerData,
																									IATLEventData* const pEventData);
	virtual EAudioRequestStatus										ActivateTrigger(
																									IATLAudioObjectData* const pAudioObjectData,
																									IATLTriggerImplData const* const pTriggerData,
																									IATLEventData* const pEventData);
	virtual EAudioRequestStatus										StopEvent(
																									IATLAudioObjectData* const pAudioObjectData,
																									IATLEventData const* const pEventData);
	virtual EAudioRequestStatus										StopAllEvents(
																									IATLAudioObjectData* const pAudioObjectData);
	virtual EAudioRequestStatus										SetPosition(
																									IATLAudioObjectData* const pAudioObjectData,
																									SATLWorldPosition const& sWorldPosition);
	virtual EAudioRequestStatus										SetEnvironment(
																									IATLAudioObjectData* const pAudioObjectData,
																									IATLEnvironmentImplData const* const pEnvironmentImplData,
																									float const fAmount);
	virtual EAudioRequestStatus										SetRtpc(
																									IATLAudioObjectData* const pAudioObjectData,
																									IATLRtpcImplData const* const pRtpcData,
																									float const fValue);
	virtual EAudioRequestStatus										SetSwitchState(
																									IATLAudioObjectData* const pAudioObjectData,
																									IATLSwitchStateImplData const* const pSwitchStateData);
	virtual EAudioRequestStatus										SetObstructionOcclusion(
																									IATLAudioObjectData* const pAudioObjectData,
																									float const fObstruction,
																									float const fOcclusion);
	virtual EAudioRequestStatus										SetListenerPosition(
																									IATLListenerData* const pListenerData,
																									SATLWorldPosition const& oNewPosition);

	virtual EAudioRequestStatus										RegisterInMemoryFile(SATLAudioFileEntryInfo* const pAudioFileEntry);
	virtual EAudioRequestStatus										UnregisterInMemoryFile(SATLAudioFileEntryInfo* const pAudioFileEntry);

	virtual EAudioRequestStatus										ParseAudioFileEntry(XmlNodeRef const pAudioFileEntryNode, SATLAudioFileEntryInfo* const pFileEntryInfo);
	virtual void																	DeleteAudioFileEntryData(IATLAudioFileEntryData* const pOldAudioFileEntryData);
	virtual char const* const											GetAudioFileLocation(SATLAudioFileEntryInfo* const pFileEntryInfo);

	virtual SATLTriggerImplData_wwise const*			NewAudioTriggerImplData(XmlNodeRef const pAudioTriggerNode);
	virtual void																	DeleteAudioTriggerImplData(IATLTriggerImplData const* const pOldTriggerImplData);

	virtual SATLRtpcImplData_wwise const*					NewAudioRtpcImplData(XmlNodeRef const pAudioRtpcNode);
	virtual void																	DeleteAudioRtpcImplData(IATLRtpcImplData const* const pOldRtpcImplData);

	virtual SATLSwitchStateImplData_wwise const*	NewAudioSwitchStateImplData(XmlNodeRef const pAudioSwitchStateImplNode);
	virtual void																	DeleteAudioSwitchStateImplData(IATLSwitchStateImplData const* const pOldAudioSwitchStateImplNode);

	virtual SATLEnvironmentImplData_wwise const*	NewAudioEnvironmentImplData(XmlNodeRef const pAudioEnvironmentNode);
	virtual void																	DeleteAudioEnvironmentImplData(IATLEnvironmentImplData const* const pOldEnvironmentImplData);

	virtual SATLAudioObjectData_wwise*						NewGlobalAudioObjectData(TAudioObjectID const nObjectID);
	virtual SATLAudioObjectData_wwise*						NewAudioObjectData(TAudioObjectID const nObjectID);
	virtual void																	DeleteAudioObjectData(IATLAudioObjectData* const pOldObjectData);

	virtual SATLListenerData_wwise*								NewDefaultAudioListenerObjectData();
	virtual SATLListenerData_wwise*								NewAudioListenerObjectData(uint const nIndex);
	virtual void																	DeleteAudioListenerObjectData(IATLListenerData* const pOldListenerData);

	virtual SATLEventData_wwise*									NewAudioEventData(TAudioEventID const nEventID);
	virtual void																	DeleteAudioEventData(IATLEventData* const pOldEventData);
	virtual void																	ResetAudioEventData(IATLEventData* const pEventData);

	virtual void																	OnAudioSystemRefresh();
	virtual void																	SetLanguage(char const* const sLanguage);

	// Below data is only used when INCLUDE_WWISE_IMPL_PRODUCTION_CODE is defined!
	virtual char const* const											GetImplementationNameString() const;
	virtual void																	GetMemoryInfo(SAudioImplMemoryInfo& oMemoryInfo) const;
	// ~IAudioSystemImplementation

private:

	static char const* const	sWwiseEventTag;
	static char const* const	sWwiseRtpcTag;
	static char const* const	sWwiseSwitchTag;
	static char const* const	sWwiseStateTag;
	static char const* const	sWwiseRtpcSwitchTag;
	static char const* const	sWwiseFileTag;
	static char const* const	sWwiseAuxBusTag;
	static char const* const	sWwiseValueTag;

	static char const* const	sWwiseNameAttribute;
	static char const* const	sWwiseValueAttribute;
	static char const* const	sWwiseMutiplierAttribute;
	static char const* const	sWwiseShiftAttribute;
	static char const* const	sWwiseLocalisedAttribute;

	struct SEnvPairCompare
	{
		bool operator() (std::pair<AkAuxBusID, float> const& oPair1, std::pair<AkAuxBusID, float> const& oPair2) const;
	};

	SATLSwitchStateImplData_wwise const*	ParseWwiseSwitchOrState(XmlNodeRef pNode,	EWwiseSwitchType eType);
	SATLSwitchStateImplData_wwise const*	ParseWwiseRtpcSwitch(XmlNodeRef pNode);
	void																	ParseRtpcImpl(XmlNodeRef pNode, AkRtpcID& rAkRtpcID, float& rMult, float& rShift);
	EAudioRequestStatus										PrepUnprepTriggerSync(
																					IATLTriggerImplData const* const pTriggerData,
																					bool bPrepare);
	EAudioRequestStatus										PrepUnprepTriggerAsync(
																					IATLTriggerImplData const* const pTriggerData,
																					IATLEventData* const pEventData,
																					bool bPrepare);

	EAudioRequestStatus										PostEnvironmentAmounts(IATLAudioObjectData* const pAudioObjectData);

	AkGameObjectID												m_nDummyGameObjectID;
	AkBankID															m_nInitBankID;
	CFileIOHandler_wwise									m_oFileIOHandler;

	CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH>	m_sRegularSoundBankFolder;
	CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH>	m_sLocalizedSoundBankFolder;

#if !defined(WWISE_FOR_RELEASE)
	bool m_bCommSystemInitialized;
#endif // !WWISE_FOR_RELEASE

#if defined(INCLUDE_WWISE_IMPL_PRODUCTION_CODE)
	CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH> m_sFullImplString;
#endif // INCLUDE_WWISE_IMPL_PRODUCTION_CODE

#if defined(WWISE_USE_OCULUS)
	void* m_pOculusSpatializerLibrary;
#endif // WWISE_USE_OCULUS
};

#endif // AUDIO_SYSTEM_IMPL_WWISE_H_INCLUDED

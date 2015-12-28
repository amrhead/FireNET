// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef IAUDIOSYSTEMIMPLEMENTATION_H_INCLUDED
#define IAUDIOSYSTEMIMPLEMENTATION_H_INCLUDED

#include <ATLEntityData.h>
#include <IAudioInterfacesCommonData.h>
#include <IXml.h>

//////////////////////////////////////////////////////////////////////////
// <title EAudioRequestStatus>
// Summary:
//      An enum that lists possible statuses of an AudioRequest. 
//			Used as a return type for many function used by the AudioSystem internally, 
//			and also for most of the IAudioSystemImplementation calls 
//////////////////////////////////////////////////////////////////////////
enum EAudioRequestStatus ATL_ENUM_TYPE
{
	eARS_NONE														= 0,
	eARS_SUCCESS												= 1,
	eARS_PARTIAL_SUCCESS								= 2,
	eARS_FAILURE												= 3,
	eARS_PENDING												= 4,
	eARS_FAILURE_INVALID_OBJECT_ID			= 5,
	eARS_FAILURE_INVLAID_CONTROL_ID			= 6,
	eARS_FAILURE_INVLAID_REQUEST				= 7,
};

//////////////////////////////////////////////////////////////////////////
// <title BoolToARS>
// Summary:
//      A utility function to convert a boolean value to an EAudioRequestStatus
// Arguments:
//      bResult - a boolean value
// Returns:
//			eARS_SUCCESS if bResult is true, eARS_FAILURE if bResult is false
//////////////////////////////////////////////////////////////////////////
inline EAudioRequestStatus BoolToARS(bool bResult)
{
	return bResult ? eARS_SUCCESS : eARS_FAILURE;
}

//////////////////////////////////////////////////////////////////////////
// <title IAudioSystemImplementation>
// Summary:
//			The CryEngine AdioTranslationLayer uses this interface to communicate with an audio middleware
//////////////////////////////////////////////////////////////////////////
struct IAudioSystemImplementation
{
	//DOC-IGNORE-BEGIN
	virtual ~IAudioSystemImplementation(){}
	//DOC-IGNORE-END

	//////////////////////////////////////////////////////////////////////////
	// <title Update>
	// Summary:
	//      Update all of the internal components that require regular updates, update the audio middleware state.
	// Arguments:
	//      fUpdateIntervalMS - Time since the last call to Update in milliseconds.
	// Returns:
	//			void
	//////////////////////////////////////////////////////////////////////////
	virtual void Update(float const fUpdateIntervalMS) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title Init>
	// Summary:
	//      Initialize all internal components and the audio middleware.
	// Returns:
	//      eARS_SUCCESS if the initialization was successful, eARS_FAILURE otherwise.
	// See Also:
	//			ShutDown
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus Init() = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title ShutDown>
	// Summary:
	//      Shuts down all of the internal components and the audio middleware. 
	// Note: After a call to ShutDown(), the system can still be reinitialized by calling Init().
	// Returns:
	//      eARS_SUCCESS if the shutdown was successful, eARS_FAILURE otherwise.
	// See Also:
	//			Release, Init
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus ShutDown() = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title Release>
	// Summary:
	//      Frees all of the resources used by the class and destroys the instance. This action is not reversible. 
	// Returns:
	//      eARS_SUCCESS if the action was successful, eARS_FAILURE otherwise.
	// See Also:
	//			ShutDown, Init
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus Release() = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title OnAudioSystemRefresh>
	// Summary:
	//      Perform a "hot restart" of the audio middleware. Reset all of the internal data.
	// See Also:
	//			Release, Init
	//////////////////////////////////////////////////////////////////////////
	virtual void OnAudioSystemRefresh() = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title OnLoseFocus>
	// Summary:
	//      This method is called every time the main Game (or Editor) window loses focus. 
	// Returns:
	//      eARS_SUCCESS if the action was successful, eARS_FAILURE otherwise.
	// See Also:
	//			OnGetFocus
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus OnLoseFocus() = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title OnGetFocus>
	// Summary:
	//      This method is called every time the main Game (or Editor) window gets focus. 
	// Returns:
	//      eARS_SUCCESS if the action was successful, eARS_FAILURE otherwise.
	// See Also:
	//			OnLoseFocus
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus OnGetFocus() = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title MuteAll>
	// Summary:
	//      Mute all sounds, after this call there should be no audio coming from the audio middleware.
	// Returns:
	//      eARS_SUCCESS if the action was successful, eARS_FAILURE otherwise.
	// See Also:
	//			UnmuteAll, StopAllSounds
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus MuteAll() = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title UnmuteAll>
	// Summary:
	//      Restore the audio output of the audio middleware after a call to MuteAll().
	// Returns:
	//      eARS_SUCCESS if the action was successful, eARS_FAILURE otherwise.
	// See Also:
	//			MuteAll
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus UnmuteAll() = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title StopAllSounds>
	// Summary:
	//      Stop all currently playing sounds. Has no effect on anything triggered after this method is called.
	// Returns:
	//      eARS_SUCCESS if the action was successful, eARS_FAILURE otherwise.
	// See Also:
	//			MuteAll
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus StopAllSounds() = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title RegisterAudioObject>
	// Summary:
	//      Register an audio object with the audio middleware. An object needs to be registered for one to set position, execute triggers on it,
	//			or set Rtpcs and switches. This version of the method is meant be used in debug builds.
	// Arguments:
	//      pObjectData - implementation-specific data needed by the audio middleware and the 
	//										AudioSystemImplementation code to manage the AudioObject being registered
	//			sObjectName - the AudioObject name shown in debug info
	// Returns:
	//      eARS_SUCCESS if the object has been registered, eARS_FAILURE if the registration failed
	// See Also:
	//			UnregisterAudioObject, ResetAudioObject
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus RegisterAudioObject(IATLAudioObjectData* const pObjectData, char const* const sObjectName) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title RegisterAudioObject>
	// Summary:
	//      Register an audio object with the audio middleware. An object needs to be registered for one to set position, execute triggers on it,
	//			or set Rtpcs and switches. This version of the method is meant to be used in the release builds.
	// Arguments:
	//      pObjectData - implementation-specific data needed by the audio middleware and the 
	//										AudioSystemImplementation code to manage the AudioObject being registered
	// Returns:
	//      eARS_SUCCESS if the object has been registered, eARS_FAILURE if the registration failed
	// See Also:
	//			UnregisterAudioObject, ResetAudioObject
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus	RegisterAudioObject(IATLAudioObjectData* const pObjectData) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title UnregisterAudioObject>
	// Summary:
	//      Unregister an audio object with the audio middleware. After this action executing triggers, setting position, states or rtpcs
	//			should have no effect on the object referenced by pObjectData.
	// Arguments:
	//      pObjectData - implementation-specific data needed by the audio middleware and the 
	//										AudioSystemImplementation code to manage the AudioObject being unregistered
	// Returns:
	//      eARS_SUCCESS if the object has been unregistered, eARS_FAILURE if the un-registration failed
	// See Also:
	//			RegisterAudioObject, ResetAudioObject
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus	UnregisterAudioObject(IATLAudioObjectData* const pObjectData) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title ResetAudioObject>
	// Summary:
	//      Clear out the object data and resets it so that the object can be returned to the pool of available Audio Object
	//			for reuse.
	// Arguments:
	//      pObjectData - implementation-specific data needed by the audio middleware and the 
	//										AudioSystemImplementation code to manage the AudioObject being reset
	// Returns:
	//      eARS_SUCCESS if the object has been reset, eARS_FAILURE otherwise
	// See Also:
	//			RegisterAudioObject, UnregisterAudioObject
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus ResetAudioObject(IATLAudioObjectData* const pObjectData) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title UpdateAudioObject>
	// Summary:
	//      Performs actions that need to be executed regularly on an AudioObject.			
	// Arguments:
	//      pObjectData - implementation-specific data needed by the audio middleware and the 
	//										AudioSystemImplementation code to manage the AudioObject being updated
	// Returns:
	//      eARS_SUCCESS if the object has been reset, eARS_FAILURE otherwise
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus UpdateAudioObject(IATLAudioObjectData* const pObjectData) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title PrepareTriggerSync>
	// Summary:
	//      Load the metadata and media needed by the audio middleware to execute the ATLTriggerImpl described by pTriggerData.
	//			Preparing Triggers manually is only necessary if their data have not been loaded via PreloadRequests.
	// Arguments:
	//      pAudioObjectData - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to manage the AudioObject on which the ATLTriggerImpl is prepared
	//			pTriggerData		 - implementation-specific data for the ATLTriggerImpl being prepared
	// Returns:
	//      eARS_SUCCESS if the the data was successfully loaded, eARS_FAILURE otherwise
	// See Also:
	//			UnprepareTriggerSync, PrepareTriggerAsync, UnprepareTriggerAsync
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus PrepareTriggerSync(
		IATLAudioObjectData* const pAudioObjectData,
		IATLTriggerImplData const* const pTriggerData) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title UnprepareTriggerSync>
	// Summary:
	//      Release the metadata and media needed by the audio middleware to execute the ATLTriggerImpl described by pTriggerData
	//			Un-preparing Triggers manually is only necessary if their data are not managed via PreloadRequests.
	// Arguments:
	//      pAudioObjectData - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to manage the AudioObject on which the ATLTriggerImpl is un-prepared
	//			pTriggerData		 - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to handle the ATLTriggerImpl being unprepared
	// Returns:
	//      eARS_SUCCESS if the the data was successfully unloaded, eARS_FAILURE otherwise
	// See Also:
	//			PrepareTriggerSync, PrepareTriggerAsync, UnprepareTriggerAsync
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus UnprepareTriggerSync(
		IATLAudioObjectData* const pAudioObjectData,
		IATLTriggerImplData const* const pTriggerData) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title PrepareTriggerAsync>
	// Summary:
	//      Load the metadata and media needed by the audio middleware to execute the ATLTriggerImpl described by pTriggerData 
	//			asynchronously. An active event that references pEventData is created on the corresponding AudioObject. 
	//			The callback called once the loading is done must report that this event is finished.
	//			Preparing Triggers manually is only necessary if their data have not been loaded via PreloadRequests.
	// Arguments:
	//      pAudioObjectData - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to manage the AudioObject on which the ATLTriggerImpl is prepared
	//			pTriggerData		 - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to handle the ATLTriggerImpl being prepared
	//			pEventData			 - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to manage the ATLEvent corresponding to the 
	//												 loading process started
	// Returns:
	//      eARS_SUCCESS if the the request was successfully sent to the audio middleware, eARS_FAILURE otherwise
	// See Also:
	//			 UnprepareTriggerAsync, PrepareTriggerSync, UnprepareTriggerSync
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus PrepareTriggerAsync(
		IATLAudioObjectData* const pAudioObjectData,
		IATLTriggerImplData const* const pTriggerData,
		IATLEventData* const pEventData) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title UnprepareTriggerAsync>
	// Summary:
	//      Unload the metadata and media needed by the audio middleware to execute the ATLTriggerImpl described by pTriggerData 
	//			asynchronously. An active event that references pEventData is created on the corresponding AudioObject. 
	//			The callback called once the unloading is done must report that this event is finished.
	//			Un-preparing Triggers manually is only necessary if their data are not managed via PreloadRequests.
	// Arguments:
	//      pAudioObjectData - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to manage the AudioObject on which the ATLTriggerImpl is un-prepared
	//			pTriggerData		 - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to handle the ATLTriggerImpl
	//			pEventData			 - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to manage the ATLEvent corresponding to the 
	//												 unloading process started
	// Returns:
	//      eARS_SUCCESS if the the request was successfully sent to the audio middleware, eARS_FAILURE otherwise
	// See Also:
	//			 PrepareTriggerAsync, PrepareTriggerSync, UnprepareTriggerSync
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus	UnprepareTriggerAsync(
		IATLAudioObjectData* const pAudioObjectData,
		IATLTriggerImplData const* const pTriggerData,
		IATLEventData* const pEventData) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title ActivateTrigger>
	// Summary:
	//      Activate an audio-middleware-specific ATLTriggerImpl
	// Arguments:
	//      pAudioObjectData - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to manage the AudioObject on which the ATLTriggerImpl should be executed
	//			pTriggerData		 - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to handle the ATLTriggerImpl
	//			pEventData			 - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to manage the ATLEvent corresponding to the 
	//												 process started by the activation of ATLTriggerImpl
	// Returns:
	//      eARS_SUCCESS if the ATLTriggerImpl has been successfully activated by the audio middleware, eARS_FAILURE otherwise
	// See Also:
	//			SetRtpc, SetSwitchState, StopEvent
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus ActivateTrigger(
		IATLAudioObjectData* const pAudioObjectData,
		IATLTriggerImplData const* const pTriggerData,
		IATLEventData* const pEventData) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title StopEvent>
	// Summary:
	//      Stop and ATLEvent active on an AudioObject
	// Arguments:
	//      pAudioObjectData - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to manage the AudioObject on which the 
	//												 specified ATLEvent is active
	//			pEventData			 - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to identify the ATLEvent that needs to be stopped
	// Returns:
	//      eARS_SUCCESS if the ATLEvent has been successfully stopped, eARS_FAILURE otherwise
	// See Also:
	//			 StopAllEvents, PrepareTriggerAsync, UnprepareTriggerAsync, ActivateTrigger
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus StopEvent(
		IATLAudioObjectData* const pAudioObjectData,
		IATLEventData const* const pEventData) = 0;
	 
	//////////////////////////////////////////////////////////////////////////
	// <title StopAllEvents>
	// Summary:
	//      Stop all ATLEvents currently active on an AudioObject
	// Arguments:
	//      pAudioObjectData - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to manage the AudioObject on which the 
	//												 ATLEvent should be stopped
	// Returns:
	//      eARS_SUCCESS if all of the ATLEvents have been successfully stopped, eARS_FAILURE otherwise
	// See Also:
	//			 StopEvent, PrepareTriggerAsync, UnprepareTriggerAsync, ActivateTrigger
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus StopAllEvents(
		IATLAudioObjectData* const pAudioObjectData) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title SetPosition>
	// Summary:
	//      Set the world position of an AudioObject inside the audio middleware
	// Arguments:
	//      pAudioObjectData - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to manage the AudioObject whose position is being set
	//			oWorldPosition	 - a struct containing all of the geometric information about the AudioObject
	//												 necessary to correctly play the audio produced on it
	// Returns:
	//      eARS_SUCCESS if the AudioObject's position has been successfully set, eARS_FAILURE otherwise
	// See Also:
	//			 StopEvent, PrepareTriggerAsync, UnprepareTriggerAsync, ActivateTrigger
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus SetPosition(
		IATLAudioObjectData* const pAudioObjectData,
		SATLWorldPosition const& oWorldPosition) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title SetRtpc>
	// Summary:
	//      Set the ATLRtpcImpl to the specified value on the provided AudioObject
	// Arguments:
	//      pAudioObjectData - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to manage the AudioObject on which the ATLRtpcImpl is being set
	//			pRtpcData				 - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to handle the ATLRtpcImpl which is being set
	//			fValue					 - the value to be set
	// Returns:
	//      eARS_SUCCESS if the provided value has been successfully set on the passed ATLRtpcImpl, eARS_FAILURE otherwise
	// See Also:
	//			 ActivateTrigger, SetSwitchState, SetEnvironment, SetPosition
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus	SetRtpc(
		IATLAudioObjectData* const pAudioObjectData,
		IATLRtpcImplData const* const pRtpcData,
		float const fValue) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title SetSwitchState>
	// Summary:
	//      Set the ATLSwitchStateImpl on the given AudioObject
	// Arguments:
	//      pAudioObjectData - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to manage the AudioObject on which the ATLSwitchStateImpl is being set
	//			pSwitchStateData - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to handle the ATLSwitchStateImpl which is being set
	// Returns:
	//      eARS_SUCCESS if the provided ATLSwitchStateImpl has been successfully set, eARS_FAILURE otherwise
	// See Also:
	//			 ActivateTrigger, SetRtpc, SetEnvironment, SetPosition
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus SetSwitchState(
		IATLAudioObjectData* const pAudioObjectData,
		IATLSwitchStateImplData const* const pSwitchStateData) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title SetObstructionOcclusion>
	// Summary:
	//      Set the provided Obstruction and Occlusion values on the given AudioObject
	// Arguments:
	//      pAudioObjectData - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to manage the AudioObject on which the Obstruction and Occlusion is being set
	//			fObstruction		 - the obstruction value to be set; it describes how much the direct sound path from the AudioObject to the Listener is obstructed
	//			fOcclusion			 - the occlusion value to be set; it describes how much all sound paths (direct and indirect) are obstructed
	// Returns:
	//      eARS_SUCCESS if the provided the values been successfully set, eARS_FAILURE otherwise
	// See Also:
	//			 SetEnvironment
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus SetObstructionOcclusion(
		IATLAudioObjectData* const pAudioObjectData,
		float const fObstruction,
		float const fOcclusion) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title SetEnvironment>
	// Summary:
	//      Set the provided value for the specified ATLEnvironmentImpl on the given AudioObject
	// Arguments:
	//      pAudioObjectData		 - implementation-specific data needed by the audio middleware and the 
	//														 AudioSystemImplementation code to manage the AudioObject on which the ATLEnvironmentImpl value is being set
	//			pEnvironmentImplData - implementation-specific data needed by the audio middleware and the 
	//														 AudioSystemImplementation code to identify and use the ATLEnvironmentImpl being set
	//			fValue							 - the fade value for the provided ATLEnvironmentImpl, 0.0f means no effect at all, 1.0f corresponds to the full effect
	// Returns:
	//      eARS_SUCCESS if the provided the value has been successfully set, eARS_FAILURE otherwise
	// See Also:
	//			 SetObstructionOcclusion
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus SetEnvironment(
		IATLAudioObjectData* const pAudioObjectData,
		IATLEnvironmentImplData const* const pEnvironmentImplData,
		float const fAmount) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title SetPosition>
	// Summary:
	//      Set the world position of an AudioListener inside the audio middleware
	// Arguments:
	//      pListenerData		 - implementation-specific data needed by the audio middleware and the 
	//												 AudioSystemImplementation code to manage the AudioListener whose position is being set
	//			oWorldPosition	 - a struct containing the necessary geometric information about the AudioListener position
	// Returns:
	//      eARS_SUCCESS if the AudioListener's position has been successfully set, eARS_FAILURE otherwise
	// See Also:
	//			SetPosition
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus SetListenerPosition(
		IATLListenerData* const pListenerData,
		SATLWorldPosition const& oNewPosition) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title RegisterInMemoryFile>
	// Summary:
	//      Inform the audio middleware about the memory location of a preloaded audio-data file
	// Arguments:
	//      pAudioFileEntry - ATL-specific information describing the resources used by the preloaded file being reported
	// Returns:
	//      eARS_SUCCESS if the audio middleware is able to use the preloaded file, eARS_FAILURE otherwise
	// See Also:
	//			UnregisterInMemoryFile
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus RegisterInMemoryFile(SATLAudioFileEntryInfo* const pAudioFileEntry) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title UnregisterInMemoryFile>
	// Summary:
	//      Inform the audio middleware that the memory containing the preloaded audio-data file should no longer be used
	// Arguments:
	//      pAudioFileEntry - ATL-specific information describing the resources used by the preloaded file being invalidated
	// Returns:
	//      eARS_SUCCESS if the audio middleware was able to unregister the preloaded file supplied, eARS_FAILURE otherwise
	// See Also:
	//			RegisterInMemoryFile
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus	UnregisterInMemoryFile(SATLAudioFileEntryInfo* const pAudioFileEntry) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title ParseAudioFileEntry>
	// Summary:
	//      Parse the implementation-specific XML node that represents an AudioFileEntry, fill the fields of the struct 
	//			referenced by pFileEntryInfo with the data necessary to correctly access and store the file's contents in memory.
	//			Create an object implementing IATLAudioFileEntryData to hold implementation-specific data about the file and store a pointer to it in a member of pFileEntryInfo
	// Arguments:
	//      pAudioFileEntryNode	 - an XML node containing the necessary information about the file
	//      pFileEntryInfo			 - a pointer to the struct containing the data used by the ATL to load the file into memory
	// Returns:
	//      eARS_SUCCESS if the XML node was parsed successfully, eARS_FAILURE otherwise
	//////////////////////////////////////////////////////////////////////////
	virtual EAudioRequestStatus ParseAudioFileEntry(
		XmlNodeRef const pAudioFileEntryNode,
		SATLAudioFileEntryInfo* const pFileEntryInfo) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title DeleteAudioFileEntryData>
	// Summary:
	//      Free the memory and potentially other resources used by the supplied IATLAudioFileEntryData instance.
	//			Normally, an IATLAudioFileEntryData instance is created by ParseAudioFileEntry() and a pointer is stored in a member of SATLAudioFileEntryInfo.
	// Arguments:
	//      pOldAudioFileEntryData - pointer to the object implementing IATLAudioFileEntryData to be discarded 
	// See Also:
	//			ParseAudioFileEntry
	//////////////////////////////////////////////////////////////////////////
	virtual void DeleteAudioFileEntryData(IATLAudioFileEntryData* const pOldAudioFileEntryData) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title GetAudioFileLocation>
	// Summary:
	//      Get the full path to the folder containing the file described by the pFileEntryInfo
	// Arguments:
	//      pFileEntryInfo - ATL-specific information describing the file whose location is being queried
	// Returns:
	//      A C-string containing the path to the folder where the file corresponding to the pFileEntryInfo is stored
	//////////////////////////////////////////////////////////////////////////
	virtual char const* const GetAudioFileLocation(SATLAudioFileEntryInfo* const pFileEntryInfo) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title NewAudioTriggerImplData>
	// Summary:
	//      Parse the implementation-specific XML node that represents an ATLTriggerImpl, return a pointer to the data needed for identifying
	//			and using this ATLTriggerImpl instance inside the AudioSystemImplementation
	// Arguments:
	//      pAudioTriggerNode - an XML node corresponding to the new ATLTriggerImpl to be created
	// Returns:
	//      IATLRtpcImplData const* pointer to the audio implementation-specific data needed by the audio middleware and the 
	//			AudioSystemImplementation code to use the corresponding ATLTriggerImpl; NULL if the new AudioTriggerImplData instance was not created
	// See Also:
	//			DeleteAudioTriggerImplData
	//////////////////////////////////////////////////////////////////////////
	virtual IATLTriggerImplData const* NewAudioTriggerImplData(XmlNodeRef const pAudioTriggerNode) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title DeleteAudioTriggerImplData>
	// Summary:
	//      Free the memory and potentially other resources used by the supplied IATLTriggerImplData instance
	// Arguments:
	//      pOldTriggerImplData - pointer to the object implementing IATLTriggerImplData to be discarded 
	// See Also:
	//			NewAudioTriggerImplData
	//////////////////////////////////////////////////////////////////////////
	virtual void DeleteAudioTriggerImplData(IATLTriggerImplData const* const pOldTriggerImplData) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title NewAudioRtpcImplData>
	// Summary:
	//      Parse the implementation-specific XML node that represents an ATLRtpcImpl, return a pointer to the data needed for identifying
	//			and using this ATLRtpcImpl instance inside the AudioSystemImplementation
	// Arguments:
	//      pAudioRtpcNode - an XML node corresponding to the new ATLRtpcImpl to be created
	// Returns:
	//      IATLRtpcImplData const* pointer to the audio implementation-specific data needed by the audio middleware and the 
	//			AudioSystemImplementation code to use the corresponding ATLRtpcImpl; NULL if the new AudioTriggerImplData instance was not created
	// See Also:
	//			DeleteAudioRtpcImplData
	//////////////////////////////////////////////////////////////////////////
	virtual IATLRtpcImplData const* NewAudioRtpcImplData(XmlNodeRef const pAudioRtpcNode) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title DeleteAudioRtpcImplData>
	// Syntax: 
	//			DeleteAudioRtpcImplData(pOldRtpcImplData)
	// Summary:
	//      Free the memory and potentially other resources used by the supplied IATLRtpcImplData instance
	// Arguments:
	//      pOldRtpcImplData - pointer to the object implementing IATLRtpcImplData to be discarded 
	// See Also:
	//			NewAudioRtpcImplData
	//////////////////////////////////////////////////////////////////////////
	virtual void DeleteAudioRtpcImplData(IATLRtpcImplData const* const pOldRtpcImplData) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title NewAudioSwitchStateImplData>
	// Summary:
	//      Parse the implementation-specific XML node that represents an ATLSwitchStateImpl, return a pointer to the data needed for identifying
	//			and using this ATLSwitchStateImpl instance inside the AudioSystemImplementation
	// Arguments:
	//      pAudioSwitchStateImplNode - an XML node corresponding to the new ATLSwitchStateImpl to be created
	// Returns:
	//      IATLRtpcImplData const* pointer to the audio implementation-specific data needed by the audio middleware and the 
	//			AudioSystemImplementation code to use the corresponding ATLSwitchStateImpl; NULL if the new AudioTriggerImplData instance was not created
	// See Also:
	//			DeleteAudioSwitchStateImplData
	//////////////////////////////////////////////////////////////////////////
	virtual IATLSwitchStateImplData const* NewAudioSwitchStateImplData(XmlNodeRef const pAudioSwitchStateImplNode) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title DeleteAudioRtpcImplData>
	// Syntax: 
	//			DeleteAudioRtpcImplData(pOldAudioSwitchStateImplNode)
	// Summary:
	//      Free the memory and potentially other resources used by the supplied IATLSwitchStateImplData instance
	// Arguments:
	//      pOldAudioSwitchStateImplNode - pointer to the object implementing IATLSwitchStateImplData to be discarded 
	// See Also:
	//			NewAudioSwitchStateImplData
	//////////////////////////////////////////////////////////////////////////
	virtual void DeleteAudioSwitchStateImplData(IATLSwitchStateImplData const* const pOldAudioSwitchStateImplNode) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title NewAudioEnvironmentImplData>
	// Summary:
	//      Parse the implementation-specific XML node that represents an ATLEnvironmentImpl, return a pointer to the data needed for identifying
	//			and using this ATLEnvironmentImpl instance inside the AudioSystemImplementation
	// Arguments:
	//      pAudioEnvironmentNode - an XML node corresponding to the new ATLEnvironmentImpl to be created
	// Returns:
	//      IATLEnvironmentImplData const* pointer to the audio implementation-specific data needed by the audio middleware and the 
	//			AudioSystemImplementation code to use the corresponding ATLEnvironmentImpl; NULL if the new IATLEnvironmentImplData instance was not created
	// See Also:
	//			DeleteAudioEnvironmentImplData
	//////////////////////////////////////////////////////////////////////////
	virtual IATLEnvironmentImplData const* NewAudioEnvironmentImplData(XmlNodeRef const pAudioEnvironmentNode) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title DeleteAudioEnvironmentImplData>
	// Syntax: 
	//			DeleteAudioEnvironmentImplData(pOldEnvironmentImplData)
	// Summary:
	//      Free the memory and potentially other resources used by the supplied IATLEnvironmentImplData instance
	// Arguments:
	//      pOldEnvironmentImplData - pointer to the object implementing IATLEnvironmentImplData to be discarded 
	// See Also:
	//			NewAudioEnvironmentImplData
	//////////////////////////////////////////////////////////////////////////
	virtual void DeleteAudioEnvironmentImplData(IATLEnvironmentImplData const* const pOldEnvironmentImplData) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title NewGlobalAudioObjectData>
	// Summary:
	//			Create an object implementing IATLAudioObjectData that stores all of the data needed by the AudioSystemImplementation
	//			to identify and use the GlobalAudioObject with the provided AudioObjectID
	// Arguments:
	//      nObjectID - the AudioObjectID to be used for the new GlobalAudioObject
	// Returns:
	//      IATLAudioObjectData* pointer to the audio implementation-specific data needed by the audio middleware and the 
	//			AudioSystemImplementation code to use the corresponding GlobalAudioObject; NULL if the new IATLAudioObjectData instance was not created
	// See Also:
	//			DeleteAudioObjectData
	//////////////////////////////////////////////////////////////////////////
	virtual IATLAudioObjectData* NewGlobalAudioObjectData(TAudioObjectID const nObjectID) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title NewAudioObjectData>
	// Summary:
	//			Create an object implementing IATLAudioObjectData that stores all of the data needed by the AudioSystemImplementation
	//			to identify and use the AudioObject with the provided AudioObjectID. Return a pointer to that object.
	// Arguments:
	//      nObjectID - the AudioObjectID to be used for the new AudioObject
	// Returns:
	//      IATLAudioObjectData* pointer to the audio implementation-specific data needed by the audio middleware and the 
	//			AudioSystemImplementation code to use the corresponding GlobalAudioObject; NULL if the new IATLAudioObjectData instance was not created
	// See Also:
	//			DeleteAudioObjectData
	//////////////////////////////////////////////////////////////////////////
	virtual IATLAudioObjectData* NewAudioObjectData(TAudioObjectID const nObjectID) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title DeleteAudioObjectData>
	// Summary:
	//      Free the memory and potentially other resources used by the supplied IATLAudioObjectData instance
	// Arguments:
	//      pOldObjectData - pointer to the object implementing IATLAudioObjectData to be discarded 
	// See Also:
	//			NewAudioObjectData, NewGlobalAudioObjectData
	//////////////////////////////////////////////////////////////////////////
	virtual void DeleteAudioObjectData(IATLAudioObjectData* const pOldObjectData) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title NewDefaultAudioListenerObjectData>
	// Summary:
	//			Create an object implementing IATLListenerData that stores all of the data needed by the AudioSystemImplementation
	//			to identify and use the DefaultAudioListener. Return a pointer to that object.
	// Returns:
	//      IATLListenerData* pointer to the audio implementation-specific data needed by the audio middleware and the 
	//			AudioSystemImplementation code to use the DefaultAudioListener; NULL if the new IATLListenerData instance was not created
	// See Also:
	//			DeleteAudioListenerObjectData
	//////////////////////////////////////////////////////////////////////////
	virtual IATLListenerData* NewDefaultAudioListenerObjectData() = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title NewAudioListenerObjectData>
	// Summary:
	//			Create an object implementing IATLListenerData that stores all of the data needed by the AudioSystemImplementation
	//			to identify and use an AudioListener. Return a pointer to that object.
	// Arguments:
	//      nIndex - index of the created AudioListener in the array of audio listeners available in the audio middleware
	// Returns:
	//      IATLListenerData* pointer to the audio implementation-specific data needed by the audio middleware and the 
	//			AudioSystemImplementation code to use the corresponding AudioListener; NULL if the new IATLListenerData instance was not created
	// See Also:
	//			DeleteAudioListenerObjectData
	//////////////////////////////////////////////////////////////////////////
	virtual IATLListenerData* NewAudioListenerObjectData(uint const nIndex) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title DeleteAudioListenerObjectData>
	// Summary:
	//      Free the memory and potentially other resources used by the supplied IATLListenerData instance
	// Arguments:
	//      pOldListenerData - pointer to the object implementing IATLListenerData to be discarded 
	// See Also:
	//			NewDefaultAudioListenerObjectData, NewAudioListenerObjectData
	//////////////////////////////////////////////////////////////////////////	
	virtual void DeleteAudioListenerObjectData(IATLListenerData* const pOldListenerData) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title NewAudioEventData>
	// Summary:
	//			Create an object implementing IATLEventData that stores all of the data needed by the AudioSystemImplementation
	//			to identify and use an AudioEvent. Return a pointer to that object.
	// Arguments:
	//      nEventID - AudioEventID to be used for the newly created AudioEvent
	// Returns:
	//      IATLEventData* pointer to the audio implementation-specific data needed by the audio middleware and the 
	//			AudioSystemImplementation code to use the corresponding AudioEvent; NULL if the new IATLEventData instance was not created
	// See Also:
	//			DeleteAudioEventData, ResetAudioEventData
	//////////////////////////////////////////////////////////////////////////
	virtual IATLEventData* NewAudioEventData(TAudioEventID const nEventID) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title DeleteAudioEventData>
	// Summary:
	//      Free the memory and potentially other resources used by the supplied IATLEventData instance
	// Arguments:
	//      pOldEventData - pointer to the object implementing IATLEventData to be discarded 
	// See Also:
	//			NewAudioEventData, ResetAudioEventData
	//////////////////////////////////////////////////////////////////////////	
	virtual void DeleteAudioEventData(IATLEventData* const pOldEventData) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title ResetAudioEventData>
	// Syntax: 
	//			ResetAudioEventData(pEventData)
	// Summary:
	//      Reset all the members of an IATLEventData instance without releasing the memory, so that the
	//			instance can be returned to the pool to be reused.	
	// Arguments:
	//      pEventData - pointer to the object implementing IATLEventData to be reset 
	// See Also:
	//			NewAudioEventData, DeleteAudioEventData
	//////////////////////////////////////////////////////////////////////////
	virtual void ResetAudioEventData(IATLEventData* const pEventData) = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title SetLanguage>
	// Syntax: 
	//			SetLanguage("english")
	// Summary:
	//      Informs the audio middleware that the localized sound banks and streamed files need to use 
	//			a different language. NOTE: this function DOES NOT unload or reload the currently loaded audio files
	// Arguments:
	//      sLanguage - a C-string representing the CryEngine language 
	// See Also:
	//			GetAudioFileLocation
	//////////////////////////////////////////////////////////////////////////
	virtual void SetLanguage(char const* const sLanguage) = 0;

	//////////////////////////////////////////////////////////////////////////
	// NOTE: The methods below are ONLY USED when INCLUDE_AUDIO_PRODUCTION_CODE is defined!
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// <title GetImplementationNameString>
	// Summary:
	//			Return a string describing the audio middleware used. This string is printed on 
	//			the first line of the AudioDebug header shown on the screen whenever s_DrawAudioDebug is not 0
	// Returns:
	//      A zero-terminated C-string with the description of the audio middleware used by this AudioSystemImplementation.
	//////////////////////////////////////////////////////////////////////////
	virtual char const* const GetImplementationNameString() const = 0;

	//////////////////////////////////////////////////////////////////////////
	// <title GetMemoryInfo>
	// Summary:
	//			Fill in the oMemoryInfo describing the current memory usage of this AudioSystemImplementation. 
	//			This data gets displayed in the AudioDebug header shown on the screen whenever s_DrawAudioDebug is not 0
	// Arguments:
	//			oMemoryInfo - a reference to an instance of SAudioImplMemoryInfo 
	//////////////////////////////////////////////////////////////////////////
	virtual void GetMemoryInfo(SAudioImplMemoryInfo& oMemoryInfo) const = 0;
};
#endif // IAUDIOSYSTEMIMPLEMENTATION_H_INCLUDED

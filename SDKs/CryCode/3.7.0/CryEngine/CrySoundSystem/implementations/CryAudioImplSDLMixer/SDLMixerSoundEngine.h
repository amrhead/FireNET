// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef SDLMIXER_SOUND_ENGINE_H_INCLUDED
#define SDLMIXER_SOUND_ENGINE_H_INCLUDED

#include "SDLMixerSoundEngineTypes.h"

namespace SDLMixer
{
	namespace SoundEngine
	{
		typedef void (*TFnEventCallback)(TAudioEventID);

		// Global events
		bool Init();
		void Release();
		void Refresh();
		void Update();

		void Pause();
		void Resume();

		void Mute();
		void UnMute();
		void Stop();

		// Load / Unload samples
		const TSDLMixerID LoadSample(const string& sSampleFilePath);
		const TSDLMixerID LoadSample(void* pMemory, const size_t nSize, const string& sSamplePath);
		void UnloadSample(const TSDLMixerID nID);

		// Events
		SSDLMixerEventStaticData* CreateEventData(const TSDLMixerID nEventID);
		bool ExecuteEvent(SSDLMixerAudioObjectData* const pAudioObject, SSDLMixerEventStaticData const* const pEventStaticData, SSDLMixerEventInstanceData* const pEventInstance);
		bool StopEvent(SSDLMixerAudioObjectData* const pAudioObject, SSDLMixerEventInstanceData const* const pEventInstance);

		// Listeners
		bool SetListenerPosition(const TSDLMixerID nListenerID, const SATLWorldPosition& position);

		// Audio Objects
		bool RegisterAudioObject(SSDLMixerAudioObjectData* pAudioObjectData);
		bool UnregisterAudioObject(SSDLMixerAudioObjectData* pAudioObjectData);
		bool SetAudioObjectPosition(SSDLMixerAudioObjectData* pAudioObjectData, const SATLWorldPosition& position);

		// Callbacks
		void RegisterEventFinishedCallback(TFnEventCallback pCallbackFunction);
	}
}

#endif // SDLMIXER_SOUND_ENGINE_H_INCLUDED

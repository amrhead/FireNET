// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef SDLMIXER_SOUND_ENGINE_TYPES_H_INCLUDED
#define SDLMIXER_SOUND_ENGINE_TYPES_H_INCLUDED

#include <IAudioSystemImplementation.h>
#include <STLSoundAllocator.h>

namespace SDLMixer
{
	typedef uint TSDLMixerID;
	extern const TSDLMixerID SDL_MIXER_INVALID_ID;
}

typedef std::set<int, std::less<int>, STLSoundAllocator<int> > TChannelSet;
typedef std::vector<SDLMixer::TSDLMixerID> TSampleList;

struct SSDLMixerEventStaticData : public IATLTriggerImplData
{
	explicit SSDLMixerEventStaticData(const TSampleList& eventSamples, const SDLMixer::TSDLMixerID nID)
		: samples(eventSamples)
		, nEventID(nID)
		, fAttenuationMinDistance(0)
		, fAttenuationMaxDistance(100)
		, nVolume(128)
		, nLoopCount(1)
		, bPanningEnabled(true)
		, bStartEvent(true)
	{}
	const TSampleList samples;
	const SDLMixer::TSDLMixerID nEventID;
	float fAttenuationMinDistance;
	float fAttenuationMaxDistance;
	int nVolume;
	int nLoopCount;
	bool bPanningEnabled;
	bool bStartEvent;
};

struct SSDLMixerEventInstanceData : public IATLEventData
{
	explicit SSDLMixerEventInstanceData(TAudioEventID const nPassedID)
		: nEventID(nPassedID)
		, pStaticData(NPTR)
	{}

	void Reset()
	{
		channels.clear();
		pStaticData = NPTR;
	}
	const TAudioEventID nEventID;
	TChannelSet channels;
	const SSDLMixerEventStaticData* pStaticData;
};
typedef std::set<SSDLMixerEventInstanceData*, std::less<SSDLMixerEventInstanceData*>, STLSoundAllocator<SSDLMixerEventInstanceData*> > TEventInstanceSet;

struct SSDLMixerAudioObjectData : public IATLAudioObjectData
{
	SSDLMixerAudioObjectData(TAudioObjectID nID, bool bIsGlobal)
		: nObjectID(nID)
		, bGlobal(bIsGlobal)
		, bPositionChanged(false) {}

	const TAudioObjectID nObjectID;
	SATLWorldPosition position;
	TEventInstanceSet events;
	bool bGlobal;
	bool bPositionChanged;
};

struct SATLListenerData : public IATLListenerData
{
	explicit SATLListenerData(const SDLMixer::TSDLMixerID nID)
		: nListenerID(nID)
	{}

	const SDLMixer::TSDLMixerID nListenerID;
	SATLWorldPosition g_listenerPosition;
};

struct SSDLMixerAudioFileEntryData : public IATLAudioFileEntryData
{
	SSDLMixerAudioFileEntryData()
	{}

	SDLMixer::TSDLMixerID nSampleID;
};

#endif // SDLMIXER_SOUND_ENGINE_TYPES_H_INCLUDED

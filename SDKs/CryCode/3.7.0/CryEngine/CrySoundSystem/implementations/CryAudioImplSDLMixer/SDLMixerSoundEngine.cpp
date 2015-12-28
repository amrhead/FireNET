#include "stdafx.h"
#include "SDLMixerSoundEngine.h"
#include "SDLMixerSoundEngineUtil.h"
#include <CryFile.h>
#include <CryPath.h>

#include <SDL.h>
#include <SDL_rwops.h>
#include <SDL_mixer.h>

#define SDL_MIXER_NUM_CHANNELS 512
#define SDL_MIXER_PROJECT_PATH "/sounds/sdlmixer/"

namespace SDLMixer
{
	static const int g_nSupportedFormats = MIX_INIT_OGG;
	static const int g_nNumMixChannels = SDL_MIXER_NUM_CHANNELS;

	static const TSDLMixerID SDL_MIXER_INVALID_ID = 0;
	const int g_nSampleRate = 48000;
	const int g_nBufferSize = 4096;

	namespace SoundEngine
	{
		// Samples
		string g_sampleDataRootDir;
		typedef std::map<TSDLMixerID, Mix_Chunk*> TSampleDataMap;
		TSampleDataMap g_sampleData;

		typedef std::map<TSDLMixerID, string> TSampleNameMap;
		TSampleNameMap g_sampleNames;

		// Channels
		struct SChannelData
		{
			SSDLMixerAudioObjectData* pAudioObject;
		};
		SChannelData g_channels[SDL_MIXER_NUM_CHANNELS];

		enum EChannelFinishedRequestQueueID
		{
			eCFRQID_ONE	= 0,
			eCFRQID_TWO	= 1,
			eCFRQID_COUNT
		};
		typedef std::deque<int> TChannelFinishedRequests;
		TChannelFinishedRequests g_channelFinishedRequests[eCFRQID_COUNT];
		CryCriticalSection g_channelFinishedCriticalSection;

		// Loaded Event Configuration
		std::map<TSDLMixerID, TSampleList> g_eventsConfiguration;

		// Audio Objects
		typedef std::vector<SSDLMixerAudioObjectData*> TAudioObjectList;
		TAudioObjectList g_audioObjects;

		// Listeners
		SATLWorldPosition g_listenerPosition;
		bool g_bListenerPosChanged;

		int g_nVolume;

		TFnEventCallback g_fnEventFinishedCallback;

		void RegisterEventFinishedCallback(TFnEventCallback pCallbackFunction)
		{
			g_fnEventFinishedCallback = pCallbackFunction;
		}

		void EventFinishedPlaying(TAudioEventID nEventID)
		{
			if (g_fnEventFinishedCallback)
			{
				g_fnEventFinishedCallback(nEventID);
			}
		}

		void ProcessChannelFinishedRequests(TChannelFinishedRequests& queue)
		{
			if (!queue.empty())
			{
				TChannelFinishedRequests::const_iterator requestsIt = queue.begin();
				const TChannelFinishedRequests::const_iterator requestsEnd = queue.end();
				for (; requestsIt != requestsEnd; ++requestsIt)
				{
					const int nChannel = *requestsIt;
					SSDLMixerAudioObjectData* pAudioObject = g_channels[nChannel].pAudioObject;
					if (pAudioObject)
					{
						TEventInstanceSet::iterator eventsIt = pAudioObject->events.begin();
						const TEventInstanceSet::iterator eventsEnd = pAudioObject->events.end();
						for (; eventsIt != eventsEnd; ++eventsIt)
						{
							SSDLMixerEventInstanceData* pEventInstance = *eventsIt;
							if (pEventInstance)
							{
								TChannelSet::const_iterator channelIt = std::find(pEventInstance->channels.begin(), pEventInstance->channels.end(), nChannel);
								if (channelIt != pEventInstance->channels.end())
								{
									pEventInstance->channels.erase(channelIt);
									if (pEventInstance->channels.empty())
									{
										pAudioObject->events.erase(eventsIt);
										EventFinishedPlaying(pEventInstance->nEventID);
									}
									break;
								}
							}
						}
					}
				}
				queue.clear();
			}
		}

		void ChannelFinishedPlaying(int nChannel)
		{
			if (nChannel >= 0 && nChannel < g_nNumMixChannels)
			{
				CryAutoLock<CryCriticalSection> autoLock(g_channelFinishedCriticalSection);
				g_channelFinishedRequests[eCFRQID_ONE].push_back(nChannel);
			}
		}

		void LoadMetadata(const string& sSDLMixerAssetPath)
		{
			g_sampleDataRootDir = PathUtil::GetPath(sSDLMixerAssetPath)  + "/";
			g_eventsConfiguration.clear();
			_finddata_t fd;
			ICryPak* pCryPak = gEnv->pCryPak;
			intptr_t handle = pCryPak->FindFirst(sSDLMixerAssetPath + "*.*", &fd);
			if (handle != -1)
			{
				do
				{
					const string sName = fd.name;
					if (sName != "." && sName != ".." && !sName.empty())
					{

						if (sName.find(".wav") != string::npos || sName.find(".ogg") != string::npos /*|| sName.find(".mp3") != string::npos*/)
						{
							// For now there's a 1 to 1 mapping between sample files and events
							g_sampleNames[GetIDFromFilePath(sName)] = sName;
							TSDLMixerID nID = SDLMixer::GetIDFromString(sName);
							std::vector<TSDLMixerID> samples;
							samples.push_back(nID);
							g_eventsConfiguration[nID] = samples;
						}
					}
				}
				while (pCryPak->FindNext(handle, &fd) >= 0);
				pCryPak->FindClose(handle);
			}

		}

		bool Init()
		{
			if (SDL_Init(SDL_INIT_AUDIO) < 0)
			{
				g_SDLMixerImplLogger.Log(eALT_ERROR, "SDL::SDL_Init() returned: %s", SDL_GetError());
				return false;
			}

			int loadedFormats = Mix_Init(g_nSupportedFormats);
			if ((loadedFormats & g_nSupportedFormats) != g_nSupportedFormats)
			{
				g_SDLMixerImplLogger.Log(eALT_ERROR, "SDLMixer::Mix_Init() failed to init support for format flags %d with error \"%s\"", g_nSupportedFormats, Mix_GetError());
				return false;
			}

			if (Mix_OpenAudio(g_nSampleRate, MIX_DEFAULT_FORMAT, 2, g_nBufferSize) < 0)
			{
				g_SDLMixerImplLogger.Log(eALT_ERROR, "SDLMixer::Mix_OpenAudio() failed to init the SDL Mixer API with error \"%s\"", Mix_GetError());
				return false;
			}

			Mix_AllocateChannels(g_nNumMixChannels);

			g_nVolume = Mix_Volume(-1, -1);

			Mix_ChannelFinished(ChannelFinishedPlaying);

			LoadMetadata(PathUtil::GetGameFolder() + SDL_MIXER_PROJECT_PATH);
			g_bListenerPosChanged = false;

			return true;
		}

		void FreeAllSampleData()
		{
			Mix_HaltChannel(-1);
			TSampleDataMap::const_iterator it = g_sampleData.begin();
			TSampleDataMap::const_iterator end = g_sampleData.end();
			for (; it != end; ++it)
			{
				Mix_FreeChunk(it->second);
			}
			g_sampleData.clear();
		}

		void Release()
		{
			FreeAllSampleData();
			g_eventsConfiguration.clear();
			g_audioObjects.clear();
			Mix_Quit();
			Mix_CloseAudio();
			SDL_Quit();
		}

		void Refresh()
		{
			FreeAllSampleData();
			g_eventsConfiguration.clear();
			LoadMetadata(PathUtil::GetGameFolder() + SDL_MIXER_PROJECT_PATH);
		}

		bool LoadSampleImpl(const TSDLMixerID nID, const string& sSamplePath)
		{
			Mix_Chunk* pSample = Mix_LoadWAV(sSamplePath.c_str());
			if (pSample != NPTR)
			{
				g_sampleData[nID] = pSample;
				g_sampleNames[nID] = sSamplePath;
			}
			else
			{
				g_SDLMixerImplLogger.Log(eALT_ERROR, "SDL Mixer failed to load sample %s. Error: \"%s\"", sSamplePath, Mix_GetError());
				return false;
			}
			return true;
		}

		const TSDLMixerID LoadSample(const string& sSampleFilePath)
		{
			const TSDLMixerID nID = GetIDFromFilePath(PathUtil::GetFile(sSampleFilePath));
			if (stl::find_in_map(g_sampleData, nID, NPTR) == NPTR)
			{
				if (!LoadSampleImpl(nID, sSampleFilePath))
				{
					return SDL_MIXER_INVALID_ID;
				}
			}
			return nID;
		}

		const TSDLMixerID LoadSample(void* pMemory, const size_t nSize, const string& sSamplePath)
		{
			const TSDLMixerID nID = GetIDFromFilePath(sSamplePath);
			Mix_Chunk* pSample = stl::find_in_map(g_sampleData, nID, NPTR);
			if (pSample != NPTR)
			{
				Mix_FreeChunk(pSample);
				g_SDLMixerImplLogger.Log(eALT_WARNING, "Loading sample %s which had already been loaded", sSamplePath);
			}
			SDL_RWops* pData = SDL_RWFromMem(pMemory, nSize);
			if (pData)
			{
				Mix_Chunk* pSample = Mix_LoadWAV_RW(pData, 0);
				if (pSample != NPTR)
				{
					g_sampleData[nID] = pSample;
					g_sampleNames[nID] = sSamplePath;
					return nID;
				}
				else
				{
					g_SDLMixerImplLogger.Log(eALT_ERROR, "SDL Mixer failed to load sample. Error: \"%s\"", Mix_GetError());
				}
			}
			else
			{
				g_SDLMixerImplLogger.Log(eALT_ERROR, "SDL Mixer failed to transform the audio data. Error: \"%s\"", SDL_GetError());
			}
			return SDL_MIXER_INVALID_ID;
		}

		void UnloadSample(const TSDLMixerID nID)
		{
			Mix_Chunk* pSample = stl::find_in_map(g_sampleData, nID, NPTR);
			if (pSample != NPTR)
			{
				Mix_FreeChunk(pSample);
				stl::member_find_and_erase(g_sampleData, nID);
			}
			else
			{
				g_SDLMixerImplLogger.Log(eALT_ERROR, "Could not find sample with id %d", nID);
			}
		}

		void Pause()
		{
			Mix_Pause(-1);
			Mix_PauseMusic();
		}

		void Resume()
		{
			Mix_Resume(-1);
			Mix_ResumeMusic();
		}

		void Stop()
		{
			Mix_HaltChannel(-1);
		}

		void Mute()
		{
			g_nVolume = Mix_Volume(-1, 0);
		}

		void UnMute()
		{
			Mix_Volume(-1, g_nVolume);
		}

		void SetChannelPosition(SSDLMixerEventInstanceData* const pEventInstance, const int channelID, const float fDistance, const float fAngle)
		{
			static const uint8 nSDLMaxDistance = 255;
			const float fMin = pEventInstance->pStaticData->fAttenuationMinDistance;
			const float fMax = pEventInstance->pStaticData->fAttenuationMaxDistance;
			if (fMin <= fMax)
			{
				uint8 nDistance = 0;
				if (fMax >= 0.0f && fDistance > fMin)
				{
					if (fMin != fMax)
					{
						const float fFinalDistance = fDistance - fMin;
						const float fRange = fMax - fMin;
						nDistance = static_cast<uint8>((std::min((fFinalDistance / fRange), 1.0f) * nSDLMaxDistance) + 0.5f);
					}
					else
					{
						nDistance = nSDLMaxDistance;
					}
				}
				//Temp code, to be reviewed during the SetChannelPosition rewrite:
				Mix_SetDistance(channelID, nDistance);

				if (pEventInstance->pStaticData->bPanningEnabled)
				{
					//Temp code, to be reviewed during the SetChannelPosition rewrite:
					float const fAbsAngle = fabs(fAngle);
					float const fFrontAngle = (fAngle > 0.0f ? 1.0f : -1.0f) * (fAbsAngle > 90.0f ? 180.f - fAbsAngle : fAbsAngle);
					float const fRightVolume = (fFrontAngle + 90.0f) / 180.0f;
					float const fLeftVolume = 1.0f - fRightVolume;
					Mix_SetPanning(channelID,
					               static_cast<uint8>(255.0f * fLeftVolume),
					               static_cast<uint8>(255.0f * fRightVolume));
				}
			}
			else
			{
				g_SDLMixerImplLogger.Log(eALT_ERROR, "The minimum attenuation distance value is higher than the maximum");
			}
		}

		bool ExecuteEvent(SSDLMixerAudioObjectData* const pAudioObject, SSDLMixerEventStaticData const* const pEventStaticData, SSDLMixerEventInstanceData* const pEventInstance)
		{
			bool bSuccess = false;

			if (pAudioObject && pEventStaticData && pEventInstance)
			{
				if (pEventStaticData->bStartEvent) // start playing samples
				{
					pEventInstance->pStaticData = pEventStaticData;
					size_t const nSize = pEventStaticData->samples.size();
					for (size_t i = 0; i < nSize; ++i)
					{
						Mix_Chunk* pSample = stl::find_in_map(g_sampleData, pEventStaticData->samples[i], NPTR);
						if (pSample == NPTR)
						{
							// Trying to play sample that hasn't been loaded yet, load it in place
							// NOTE: This should be avoided as it can cause lag in audio playback
							const string sSampleName = g_sampleNames[pEventStaticData->samples[i]];
							g_SDLMixerImplLogger.Log(eALT_WARNING, "Using sample %s without pre-loading it first, consider using a preload request to avoid lag in playback.", sSampleName);
							if (LoadSampleImpl(GetIDFromFilePath(sSampleName), g_sampleDataRootDir + sSampleName))
							{
								pSample = stl::find_in_map(g_sampleData, pEventStaticData->samples[i], NPTR);
							}
							if (pSample == NPTR)
							{
								return false;
							}
						}


						int nLoopCount = pEventStaticData->nLoopCount;
						if (nLoopCount > 0)
						{
							// For SDL Mixer 0 loops means play only once, 1 loop play twice, etc ...
							--nLoopCount;
						}
						int nChannelID = Mix_PlayChannel(-1, pSample, nLoopCount);
						if (nChannelID >= 0)
						{
							Mix_Volume(nChannelID, pEventStaticData->nVolume);

							// Get distance and angle from the listener to the audio object
							float fDistance = 0.0f;
							float fAngle = 0.0f;
							GetDistanceAngleToObject(g_listenerPosition, pAudioObject->position, fDistance, fAngle);
							SetChannelPosition(pEventInstance, nChannelID, fDistance, fAngle);

							g_channels[nChannelID].pAudioObject = pAudioObject;
							pAudioObject->events.insert(pEventInstance);
							pEventInstance->channels.insert(nChannelID);
						}
						else
						{
							g_SDLMixerImplLogger.Log(eALT_ERROR, "Could not play sample. Error: %s", Mix_GetError());
						}
					}
					bSuccess = nSize > 0;
				}
				else // stop event in audio object
				{
					TEventInstanceSet::const_iterator eventIt = pAudioObject->events.begin();
					TEventInstanceSet::const_iterator eventEnd = pAudioObject->events.end();
					for (; eventIt != eventEnd; ++eventIt)
					{
						SSDLMixerEventInstanceData* pEventInstance = *eventIt;
						if (pEventInstance)
						{
							if (pEventInstance->pStaticData->nEventID == pEventStaticData->nEventID)
							{
								StopEvent(pAudioObject, pEventInstance);
							}
						}
					}
				}
			}

			return bSuccess;
		}

		bool SetListenerPosition(const TSDLMixerID nListenerID, const SATLWorldPosition& position)
		{
			g_listenerPosition = position;
			g_bListenerPosChanged = true;
			return true;
		}

		bool RegisterAudioObject(SSDLMixerAudioObjectData* pAudioObjectData)
		{
			if (pAudioObjectData)
			{
				g_audioObjects.push_back(pAudioObjectData);
				return true;
			}
			return false;
		}

		bool UnregisterAudioObject(SSDLMixerAudioObjectData* pAudioObjectData)
		{
			if (pAudioObjectData)
			{
				stl::find_and_erase(g_audioObjects, pAudioObjectData);
				return true;
			}
			return false;
		}

		bool SetAudioObjectPosition(SSDLMixerAudioObjectData* pAudioObjectData, const SATLWorldPosition& position)
		{
			if (pAudioObjectData)
			{
				pAudioObjectData->position = position;
				pAudioObjectData->bPositionChanged = true;
				return true;
			}
			return false;
		}

		bool StopEvent(SSDLMixerAudioObjectData* const pAudioObject, SSDLMixerEventInstanceData const* const pEventInstance)
		{
			if (pEventInstance)
			{
				// need to make a copy because the callback
				// registered with Mix_ChannelFinished can edit the list
				TChannelSet channels = pEventInstance->channels;
				TChannelSet::const_iterator channelIt = channels.begin();
				TChannelSet::const_iterator channelEnd = channels.end();
				for (; channelIt != channelEnd; ++channelIt)
				{
					Mix_HaltChannel(*channelIt);
				}
				return true;
			}
			return false;
		}

		SSDLMixerEventStaticData* CreateEventData(const TSDLMixerID nEventID)
		{
			SSDLMixerEventStaticData* pNewTriggerImpl = NPTR;
			if (nEventID != SDLMixer::SDL_MIXER_INVALID_ID)
			{
				POOL_NEW(SSDLMixerEventStaticData, pNewTriggerImpl)(stl::find_in_map_ref(g_eventsConfiguration, nEventID, std::vector<TSDLMixerID>()), nEventID);
			}
			else
			{
				assert(false);
			}
			return pNewTriggerImpl;
		}

		void Update()
		{
			ProcessChannelFinishedRequests(g_channelFinishedRequests[eCFRQID_TWO]);
			{
				CryAutoLock<CryCriticalSection> oAutoLock(g_channelFinishedCriticalSection);
				g_channelFinishedRequests[eCFRQID_ONE].swap(g_channelFinishedRequests[eCFRQID_TWO]);
			}

			TAudioObjectList::const_iterator audioObjectIt = g_audioObjects.begin();
			const TAudioObjectList::const_iterator audioObjectEnd = g_audioObjects.end();
			for (; audioObjectIt != audioObjectEnd; ++audioObjectIt)
			{
				SSDLMixerAudioObjectData* pAudioObject = *audioObjectIt;
				if (pAudioObject && (pAudioObject->bPositionChanged || g_bListenerPosChanged))
				{
					// Get distance and angle from the listener to the audio object
					float fDistance = 0.0f;
					float fAngle = 0.0f;
					GetDistanceAngleToObject(g_listenerPosition, pAudioObject->position, fDistance, fAngle);
					const uint8 nSDLMaxDistance = 255;

					TEventInstanceSet::const_iterator eventIt = pAudioObject->events.begin();
					const TEventInstanceSet::const_iterator eventEnd = pAudioObject->events.end();
					for (; eventIt != eventEnd; ++eventIt)
					{
						SSDLMixerEventInstanceData* pEventInstance = *eventIt;
						if (pEventInstance && pEventInstance->pStaticData)
						{
							TChannelSet::const_iterator channelIt = pEventInstance->channels.begin();
							const TChannelSet::const_iterator channelEnd = pEventInstance->channels.end();
							for (; channelIt != channelEnd; ++channelIt)
							{
								SetChannelPosition(pEventInstance, *channelIt, fDistance, fAngle);
							}
						}
					}
					pAudioObject->bPositionChanged = false;
				}
			}
			g_bListenerPosChanged = false;
		}
	}
}
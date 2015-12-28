// CryEngine Source File.
// Copyright (C), Crytek, 1999-2014.

#include "stdafx.h"
#include "AudioSystemImpl_sdlmixer.h"
#include "AudioSystemImplCVars_sdlmixer.h"
#include <IAudioSystemImplementation.h>
#include <SoundAllocator.h>
#include <IAudioSystem.h>
#include <platform_impl.h>
#include <IEngineModule.h>
#include <CryExtension/ICryFactory.h>
#include <CryExtension/Impl/ClassWeaver.h>

CSoundAllocator				g_SDLMixerImplMemoryPool;
CAudioLogger					g_SDLMixerImplLogger;
CAudioSystemImplCVars g_AudioImplCVars;

//////////////////////////////////////////////////////////////////////////
class CEngineModule_CryAudioImplSDL : public IEngineModule
{
	CRYINTERFACE_SIMPLE(IEngineModule)
	CRYGENERATE_SINGLETONCLASS(CEngineModule_CryAudioImplSDL, "AudioModule_CryAudioImpl", 0x8030c0d1905b4031, 0xa3785a8b53125f3f)

	//////////////////////////////////////////////////////////////////////////
	virtual const char *GetName() {return "CryAudioImplSDLMixer";}
	virtual const char *GetCategory() {return "CryAudio";}

	//////////////////////////////////////////////////////////////////////////
	virtual bool Initialize(SSystemGlobalEnvironment &env,const SSystemInitParams &initParams)
	{
		bool bSuccess = false;

		// initialize memory pools

		MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "SDL Mixer Audio Implementation Memory Pool Primary");
		size_t const nPoolSize		= g_AudioImplCVars.m_nPrimaryPoolSize << 10;
		uint8* const pPoolMemory	= new uint8[nPoolSize];
		g_SDLMixerImplMemoryPool.InitMem(nPoolSize, pPoolMemory, "SDL Mixer Implementation Audio Pool");

		POOL_NEW_CREATE(CAudioSystemImpl_sdlmixer, pImpl);

		if (pImpl	!= NPTR)
		{
			g_SDLMixerImplLogger.Log(eALT_ALWAYS, "<Audio> CryAudioImplSDLMixer loaded");

			env.pAudioSystem->Initialize(pImpl);

			bSuccess = true;
		}
		else
		{
			g_SDLMixerImplLogger.Log(eALT_ALWAYS, "<Audio> CryAudioImplSDLMixer failed to load");
		}

		return bSuccess;
	}
};

CRYREGISTER_SINGLETON_CLASS(CEngineModule_CryAudioImplSDL)

CEngineModule_CryAudioImplSDL::CEngineModule_CryAudioImplSDL()
{
	g_AudioImplCVars.RegisterVariables();
}

CEngineModule_CryAudioImplSDL::~CEngineModule_CryAudioImplSDL()
{
}

#include <CrtDebugStats.h>

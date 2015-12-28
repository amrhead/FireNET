// CryEngine Source File.
// Copyright (C), Crytek, 1999-2014.

#include "stdafx.h"
#include "AudioSystemImplCVars_sdlmixer.h"
#include <IConsole.h>

//////////////////////////////////////////////////////////////////////////
CAudioSystemImplCVars::CAudioSystemImplCVars()
	:	m_nPrimaryPoolSize(0)
{
}

//////////////////////////////////////////////////////////////////////////
CAudioSystemImplCVars::~CAudioSystemImplCVars()
{
}

//////////////////////////////////////////////////////////////////////////
void CAudioSystemImplCVars::RegisterVariables()
{
#if defined(WIN32) || defined(WIN64)
	m_nPrimaryPoolSize		= 128<<10;	// 128 MiB
#elif defined(MAC)
	m_nPrimaryPoolSize		= 128<<10;	// 128 MiB
#elif defined(LINUX)
	m_nPrimaryPoolSize		= 128<<10;	// 128 MiB
#elif defined(IOS)
	m_nPrimaryPoolSize		= 8<<10;		// 8 MiB
#elif defined(ANDROID)
	m_nPrimaryPoolSize		= 8<<10;		// 8 MiB
#else
#error "Undefined platform."
#endif

	REGISTER_CVAR2("s_SDLMixerPrimaryPoolSize", &m_nPrimaryPoolSize, m_nPrimaryPoolSize, VF_REQUIRE_APP_RESTART,
		"Specifies the size (in KiB) of the memory pool to be used by the SDL Mixer audio system implementation.\n"
		"Usage: s_SDLMixerPrimaryPoolSize [0/...]\n"
		"Default PC: 131072 (128 MiB), MAC: 131072 (128 MiB), Linux: 131072 (128 MiB), IOS: 8192 (8 MiB), Android: 8192 (8 MiB)\n");
}

//////////////////////////////////////////////////////////////////////////
void CAudioSystemImplCVars::UnregisterVariables()
{
	IConsole* const pConsole = gEnv->pConsole;
	assert(pConsole);

	pConsole->UnregisterVariable("s_SDLMixerPrimaryPoolSize");
}

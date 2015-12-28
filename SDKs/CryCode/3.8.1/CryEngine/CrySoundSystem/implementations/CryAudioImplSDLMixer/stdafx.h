// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#if !defined(AFX_STDAFX_H__1E4C7FAC_937A_454F_BF0E_GRHBD056EEE5__INCLUDED_)
#define AFX_STDAFX_H__1E4C7FAC_937A_454F_BF0E_GRHBD056EEE5__INCLUDED_

#include <CryModuleDefs.h>
#include <platform.h>
#include <StlUtils.h>
#include <ProjectDefines.h>
#include <SoundAllocator.h>

#if !defined(_RELEASE)
// Define this to enable logging via CAudioLogger.
// We disable logging for Release builds
#define ENABLE_AUDIO_LOGGING
#endif // _RELEASE

#include <AudioLogger.h>

extern CSoundAllocator g_SDLMixerImplMemoryPool;
extern CAudioLogger g_SDLMixerImplLogger;

#define AUDIO_ALLOCATOR_MEMORY_POOL g_SDLMixerImplMemoryPool
#include <STLSoundAllocator.h>

#if !defined(_RELEASE)
#define INCLUDE_SDLMIXER_IMPL_PRODUCTION_CODE
#endif // _RELEASE

// Win32 or Win64
//////////////////////////////////////////////////////////////////////////
#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

// Win32
//////////////////////////////////////////////////////////////////////////
#if defined(WIN32) && !defined(WIN64)
#endif // Win32

// Win64 (Note: WIN32 is automatically defined as well!)
//////////////////////////////////////////////////////////////////////////
#if defined(WIN32) && defined(WIN64)
#endif // Win64

// Mac
//////////////////////////////////////////////////////////////////////////
#if defined(MAC)
#endif

// Android
//////////////////////////////////////////////////////////////////////////
#if defined(ANDROID)
#endif

// iOS
//////////////////////////////////////////////////////////////////////////
#if defined(IOS)
#endif

// Linux
//////////////////////////////////////////////////////////////////////////
#if defined(LINUX) && !defined(ANDROID)
#endif // LINUX

#endif // AFX_STDAFX_H__1E4C7FAC_937A_454F_BF0E_GRHBD056EEE5__INCLUDED_

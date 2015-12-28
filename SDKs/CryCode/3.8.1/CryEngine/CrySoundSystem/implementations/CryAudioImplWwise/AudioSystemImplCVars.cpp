// CryEngine Source File.
// Copyright (C), Crytek, 1999-2014.

#include "stdafx.h"
#include "AudioSystemImplCVars.h"
#include <IConsole.h>

//////////////////////////////////////////////////////////////////////////
CAudioWwiseImplCVars::CAudioWwiseImplCVars()
	: m_nPrimaryMemoryPoolSize(0)
	, m_nSecondaryMemoryPoolSize(0)
	, m_nPrepareEventMemoryPoolSize(0)
	, m_nStreamManagerMemoryPoolSize(0)
	, m_nStreamDeviceMemoryPoolSize(0)
	, m_nSoundEngineDefaultMemoryPoolSize(0)
	, m_nCommandQueueMemoryPoolSize(0)
	, m_nLowerEngineDefaultPoolSize(0)
#if defined(INCLUDE_WWISE_IMPL_PRODUCTION_CODE)
	, m_nEnableCommSystem(0)
	, m_nEnableOutputCapture(0)
	, m_nMonitorMemoryPoolSize(0)
	, m_nMonitorQueueMemoryPoolSize(0)
#endif // INCLUDE_WWISE_IMPL_PRODUCTION_CODE
{
}

//////////////////////////////////////////////////////////////////////////
CAudioWwiseImplCVars::~CAudioWwiseImplCVars()
{
}

//////////////////////////////////////////////////////////////////////////
void CAudioWwiseImplCVars::RegisterVariables()
{
#if defined(WIN32) || defined(WIN64)
	m_nPrimaryMemoryPoolSize						= 128<<10;	// 128 MiB
	m_nSecondaryMemoryPoolSize					= 0;
	m_nPrepareEventMemoryPoolSize				= 2<<10;		// 2 MiB
	m_nStreamManagerMemoryPoolSize			= 64;				// 64 KiB;
	m_nStreamDeviceMemoryPoolSize				= 2<<10;		// 2 MiB
	m_nSoundEngineDefaultMemoryPoolSize	= 8<<10;		// 8 MiB
	m_nCommandQueueMemoryPoolSize				= 256;			// 256 KiB
	m_nLowerEngineDefaultPoolSize				= 16<<10;		// 16 MiB
#if defined(INCLUDE_WWISE_IMPL_PRODUCTION_CODE)
	m_nMonitorMemoryPoolSize						= 256;			// 256 KiB
	m_nMonitorQueueMemoryPoolSize				= 64;				// 64 KiB
#endif // INCLUDE_WWISE_IMPL_PRODUCTION_CODE
#elif defined(DURANGO)
	m_nPrimaryMemoryPoolSize						= 128<<10;	// 128 MiB
	m_nSecondaryMemoryPoolSize					= 32<<10;		// 32 MiB
	m_nPrepareEventMemoryPoolSize				= 2<<10;		// 2 MiB
	m_nStreamManagerMemoryPoolSize			= 64;				// 64 KiB;
	m_nStreamDeviceMemoryPoolSize				= 2<<10;		// 2 MiB
	m_nSoundEngineDefaultMemoryPoolSize	= 8<<10;		// 8 MiB
	m_nCommandQueueMemoryPoolSize				= 256;			// 256 KiB
	m_nLowerEngineDefaultPoolSize				= 16<<10;		// 16 MiB
#if defined(INCLUDE_WWISE_IMPL_PRODUCTION_CODE)
	m_nMonitorMemoryPoolSize						= 256;			// 256 KiB
	m_nMonitorQueueMemoryPoolSize				= 64;				// 64 KiB
#endif // INCLUDE_WWISE_IMPL_PRODUCTION_CODE
#elif defined(ORBIS)
	m_nPrimaryMemoryPoolSize						= 128<<10;	// 128 MiB
	m_nSecondaryMemoryPoolSize					= 0;
	m_nPrepareEventMemoryPoolSize				= 2<<10;		// 2 MiB
	m_nStreamManagerMemoryPoolSize			= 64;				// 64 KiB;
	m_nStreamDeviceMemoryPoolSize				= 2<<10;		// 2 MiB
	m_nSoundEngineDefaultMemoryPoolSize	= 8<<10;		// 8 MiB
	m_nCommandQueueMemoryPoolSize				= 256;			// 256 KiB
	m_nLowerEngineDefaultPoolSize				= 16<<10;		// 16 MiB
#if defined(INCLUDE_WWISE_IMPL_PRODUCTION_CODE)
	m_nMonitorMemoryPoolSize						= 256;			// 256 KiB
	m_nMonitorQueueMemoryPoolSize				= 64;				// 64 KiB
#endif // INCLUDE_WWISE_IMPL_PRODUCTION_CODE
#elif defined(MAC)
	m_nPrimaryMemoryPoolSize						= 128<<10;	// 128 MiB
	m_nSecondaryMemoryPoolSize					= 0;
	m_nPrepareEventMemoryPoolSize				= 2<<10;		// 2 MiB
	m_nStreamManagerMemoryPoolSize			= 64;				// 64 KiB;
	m_nStreamDeviceMemoryPoolSize				= 2<<10;		// 2 MiB
	m_nSoundEngineDefaultMemoryPoolSize	= 8<<10;		// 8 MiB
	m_nCommandQueueMemoryPoolSize				= 256;			// 256 KiB
	m_nLowerEngineDefaultPoolSize				= 16<<10;		// 16 MiB
#if defined(INCLUDE_WWISE_IMPL_PRODUCTION_CODE)
	m_nMonitorMemoryPoolSize						= 256;			// 256 KiB
	m_nMonitorQueueMemoryPoolSize				= 64;				// 64 KiB
#endif // INCLUDE_WWISE_IMPL_PRODUCTION_CODE
#elif defined(LINUX) && !defined(ANDROID)
	m_nPrimaryMemoryPoolSize						= 128<<10;	// 128 MiB
	m_nSecondaryMemoryPoolSize					= 0;
	m_nPrepareEventMemoryPoolSize				= 2<<10;		// 2 MiB
	m_nStreamManagerMemoryPoolSize			= 64;				// 64 KiB;
	m_nStreamDeviceMemoryPoolSize				= 2<<10;		// 2 MiB
	m_nSoundEngineDefaultMemoryPoolSize	= 8<<10;		// 8 MiB
	m_nCommandQueueMemoryPoolSize				= 256;			// 256 KiB
	m_nLowerEngineDefaultPoolSize				= 16<<10;		// 16 MiB
#if defined(INCLUDE_WWISE_IMPL_PRODUCTION_CODE)
	m_nMonitorMemoryPoolSize						= 256;			// 256 KiB
	m_nMonitorQueueMemoryPoolSize				= 64;				// 64 KiB
#endif // INCLUDE_WWISE_IMPL_PRODUCTION_CODE
#elif defined(iOS)
	m_nPrimaryMemoryPoolSize						= 8<<10;		// 8 MiB
	m_nSecondaryMemoryPoolSize					= 0;
	m_nPrepareEventMemoryPoolSize				= 2<<10;		// 2 MiB
	m_nStreamManagerMemoryPoolSize			= 64;				// 64 KiB;
	m_nStreamDeviceMemoryPoolSize				= 2<<10;		// 2 MiB
	m_nSoundEngineDefaultMemoryPoolSize	= 8<<10;		// 8 MiB
	m_nCommandQueueMemoryPoolSize				= 256;			// 256 KiB
	m_nLowerEngineDefaultPoolSize				= 16<<10;		// 16 MiB
#if defined(INCLUDE_WWISE_IMPL_PRODUCTION_CODE)
	m_nMonitorMemoryPoolSize						= 256;			// 256 KiB
	m_nMonitorQueueMemoryPoolSize				= 64;				// 64 KiB
#endif // INCLUDE_WWISE_IMPL_PRODUCTION_CODE
#elif defined(ANDROID)
	m_nPrimaryMemoryPoolSize						= 32<<10;		// 32 MiB on ANDROID
	m_nSecondaryMemoryPoolSize					= 0;
	m_nPrepareEventMemoryPoolSize				= 2<<10;		// 2 MiB
	m_nStreamManagerMemoryPoolSize			= 64;				// 64 KiB;
	m_nStreamDeviceMemoryPoolSize				= 2<<10;		// 2 MiB
	m_nSoundEngineDefaultMemoryPoolSize	= 8<<10;		// 8 MiB
	m_nCommandQueueMemoryPoolSize				= 256;			// 256 KiB
	m_nLowerEngineDefaultPoolSize				= 16<<10;		// 16 MiB
#if defined(INCLUDE_WWISE_IMPL_PRODUCTION_CODE)
	m_nMonitorMemoryPoolSize						= 256;			// 256 KiB
	m_nMonitorQueueMemoryPoolSize				= 64;				// 64 KiB
#endif // INCLUDE_WWISE_IMPL_PRODUCTION_CODE
#else
#error "Undefined platform."
#endif

	REGISTER_CVAR2("s_WwisePrimaryPoolSize", &m_nPrimaryMemoryPoolSize, m_nPrimaryMemoryPoolSize, VF_REQUIRE_APP_RESTART,
		"Specifies the size (in KiB) of the memory pool to be used by the Wwise audio system implementation.\n"
		"Usage: s_WwisePrimaryPoolSize [0/...]\n"
		"Default PC: 131072 (128 MiB), XboxOne: 131072 (128 MiB), PS4: 131072 (128 MiB), MAC: 131072 (128 MiB), Linux: 131072 (128 MiB), iOS: 8192 (8 MiB), Android: 32768 (32 MiB)\n");

	REGISTER_CVAR2("s_WwiseSecondaryPoolSize", &m_nSecondaryMemoryPoolSize, m_nSecondaryMemoryPoolSize, VF_REQUIRE_APP_RESTART,
		"Specifies the size (in KiB) of the memory pool to be used by the Wwise audio system implementation.\n"
		"Usage: s_WwiseSecondaryPoolSize [0/...]\n"
		"Default PC: 0, XboxOne: 32768 (32 MiB), PS4: 0, MAC: 0, Linux: 0, iOS: 0, Android: 0\n");

	REGISTER_CVAR2("s_WwisePrepareEventMemoryPoolSize", &m_nPrepareEventMemoryPoolSize, m_nPrepareEventMemoryPoolSize, VF_REQUIRE_APP_RESTART,
		"Specifies the size (in KiB) of the Wwise prepare event memory pool.\n"
		"Usage: s_WwisePrepareEventMemoryPoolSize [0/...]\n"
		"Default PC: 2048 (2 MiB), XboxOne: 2048 (2 MiB), PS4: 2048 (2 MiB), MAC: 2048 (2 MiB), Linux: 2048 (2 MiB), iOS: 2048 (2 MiB), Android: 2048 (2 MiB)\n");

	REGISTER_CVAR2("s_WwiseStreamManagerMemoryPoolSize", &m_nStreamManagerMemoryPoolSize, m_nStreamManagerMemoryPoolSize, VF_REQUIRE_APP_RESTART,
		"Specifies the size (in KiB) of the Wwise stream manager memory pool.\n"
		"Usage: s_WwiseStreamManagerMemoryPoolSize [0/...]\n"
		"Default PC: 64, XboxOne: 64, PS4: 64, MAC: 64, Linux: 64, iOS: 64, Android: 64\n");

	REGISTER_CVAR2("s_WwiseStreamDeviceMemoryPoolSize", &m_nStreamDeviceMemoryPoolSize, m_nStreamDeviceMemoryPoolSize, VF_REQUIRE_APP_RESTART,
		"Specifies the size (in KiB) of the Wwise stream device memory pool.\n"
		"Usage: s_WwiseStreamDeviceMemoryPoolSize [0/...]\n"
		"Default PC: 2048 (2 MiB), XboxOne: 2048 (2 MiB), PS4: 2048 (2 MiB), MAC: 2048 (2 MiB), Linux: 2048 (2 MiB), iOS: 2048 (2 MiB), Android: 2048 (2 MiB)\n");

	REGISTER_CVAR2("s_WwiseSoundEngineDefaultMemoryPoolSize", &m_nSoundEngineDefaultMemoryPoolSize, m_nSoundEngineDefaultMemoryPoolSize, VF_REQUIRE_APP_RESTART,
		"Specifies the size (in KiB) of the Wwise sound engine default memory pool.\n"
		"Usage: s_WwiseSoundEngineDefaultMemoryPoolSize [0/...]\n"
		"Default PC: 8192 (8 MiB), XboxOne: 8192 (8 MiB), PS4: 8192 (8 MiB), MAC: 8192 (8 MiB), Linux: 8192 (8 MiB), iOS: 8192 (8 MiB), Android: 8192 (8 MiB)\n");

	REGISTER_CVAR2("s_WwiseCommandQueueMemoryPoolSize", &m_nCommandQueueMemoryPoolSize, m_nCommandQueueMemoryPoolSize, VF_REQUIRE_APP_RESTART,
		"Specifies the size (in KiB) of the Wwise command queue memory pool.\n"
		"Usage: s_WwiseCommandQueueMemoryPoolSize [0/...]\n"
		"Default PC: 256, XboxOne: 256, PS4: 256, MAC: 256, Linux: 256, iOS: 256, Android: 256\n");

	REGISTER_CVAR2("s_WwiseLowerEngineDefaultPoolSize", &m_nLowerEngineDefaultPoolSize, m_nLowerEngineDefaultPoolSize, VF_REQUIRE_APP_RESTART,
		"Specifies the size (in KiB) of the Wwise lower engine memory pool.\n"
		"Usage: s_WwiseLowerEngineDefaultPoolSize [0/...]\n"
		"Default PC: 16384 (16 MiB), XboxOne: 16384 (16 MiB), PS4: 16384 (16 MiB), MAC: 16384 (16 MiB), Linux: 16384 (16 MiB), iOS: 16384 (16 MiB), Android: 16384 (16 MiB)\n");

#if defined(INCLUDE_WWISE_IMPL_PRODUCTION_CODE)
	REGISTER_CVAR2("s_WwiseEnableCommSystem", &m_nEnableCommSystem, 0, VF_REQUIRE_APP_RESTART,
		"Specifies whether Wwise should initialize using its Comm system or not.\n"
		"This cvar is only available in non-release builds.\n"
		"Usage: s_WwiseEnableCommSystem [0/1]\n"
		"Default: 0 (off)\n");

	REGISTER_CVAR2("s_WwiseEnableOutputCapture", &m_nEnableOutputCapture, 0, VF_REQUIRE_APP_RESTART,
		"Allows for capturing the output audio to a wav file.\n"
		"This cvar is only available in non-release builds.\n"
		"Usage: s_WwiseEnableOutputCapture [0/1]\n"
		"Default: 0 (off)\n");

	REGISTER_CVAR2("s_WwiseMonitorMemoryPoolSize", &m_nMonitorMemoryPoolSize, m_nMonitorMemoryPoolSize, VF_REQUIRE_APP_RESTART,
		"Specifies the size (in KiB) of the Wwise monitor memory pool.\n"
		"Usage: s_WwiseMonitorMemoryPoolSize [0/...]\n"
		"Default PC: 256, XboxOne: 256, PS4: 256, MAC: 256, Linux: 256, iOS: 256, Android: 256\n");

	REGISTER_CVAR2("s_WwiseMonitorQueueMemoryPoolSize", &m_nMonitorQueueMemoryPoolSize, m_nMonitorQueueMemoryPoolSize, VF_REQUIRE_APP_RESTART,
		"Specifies the size (in KiB) of the Wwise monitor queue memory pool.\n"
		"Usage: s_WwiseMonitorQueueMemoryPoolSize [0/...]\n"
		"Default PC: 64, XboxOne: 64, PS4: 64, MAC: 64, Linux: 64, iOS: 64, Android: 64\n");
#endif // INCLUDE_WWISE_IMPL_PRODUCTION_CODE
}

//////////////////////////////////////////////////////////////////////////
void CAudioWwiseImplCVars::UnregisterVariables()
{
	IConsole* const pConsole = gEnv->pConsole;
	assert(pConsole);

	pConsole->UnregisterVariable("s_WwisePrimaryPoolSize");
	pConsole->UnregisterVariable("s_WwiseSecondaryPoolSize");
	pConsole->UnregisterVariable("s_WwisePrepareEventMemoryPoolSize");
	pConsole->UnregisterVariable("s_WwiseStreamManagerMemoryPoolSize");
	pConsole->UnregisterVariable("s_WwiseStreamDeviceMemoryPoolSize");
	pConsole->UnregisterVariable("s_WwiseSoundEngineDefaultMemoryPoolSize");
	pConsole->UnregisterVariable("s_WwiseCommandQueueMemoryPoolSize");
	pConsole->UnregisterVariable("s_WwiseLowerEngineDefaultPoolSize");

#if defined(INCLUDE_WWISE_IMPL_PRODUCTION_CODE)
	pConsole->UnregisterVariable("s_WwiseEnableCommSystem");
	pConsole->UnregisterVariable("s_WwiseEnableOutputCapture");
	pConsole->UnregisterVariable("s_WwiseMonitorMemoryPoolSize");
	pConsole->UnregisterVariable("s_WwiseMonitorQueueMemoryPoolSize");
#endif // INCLUDE_WWISE_IMPL_PRODUCTION_CODE
}

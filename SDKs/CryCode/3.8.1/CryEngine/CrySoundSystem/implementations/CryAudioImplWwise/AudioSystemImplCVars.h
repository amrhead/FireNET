// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef AUDIO_WWISE_IMPL_CVARS_H_INCLUDED
#define AUDIO_WWISE_IMPL_CVARS_H_INCLUDED

class CAudioWwiseImplCVars
{
public:

	CAudioWwiseImplCVars();
	~CAudioWwiseImplCVars();

	void	RegisterVariables();
	void	UnregisterVariables();

	int		m_nPrimaryMemoryPoolSize;
	int		m_nSecondaryMemoryPoolSize;
	int		m_nPrepareEventMemoryPoolSize;
	int		m_nStreamManagerMemoryPoolSize;
	int		m_nStreamDeviceMemoryPoolSize;
	int		m_nSoundEngineDefaultMemoryPoolSize;
	int		m_nCommandQueueMemoryPoolSize;
	int		m_nLowerEngineDefaultPoolSize;

#if defined(INCLUDE_WWISE_IMPL_PRODUCTION_CODE)
	int		m_nEnableCommSystem;
	int		m_nEnableOutputCapture;
	int		m_nMonitorMemoryPoolSize;
	int		m_nMonitorQueueMemoryPoolSize;
#endif // INCLUDE_WWISE_IMPL_PRODUCTION_CODE

private:

	CAudioWwiseImplCVars(CAudioWwiseImplCVars const&);						// Copy protection
	CAudioWwiseImplCVars& operator=(CAudioWwiseImplCVars const&);	// Copy protection
};

extern CAudioWwiseImplCVars g_AudioWwiseImplCVars;

#endif // AUDIO_WWISE_IMPL_CVARS_H_INCLUDED

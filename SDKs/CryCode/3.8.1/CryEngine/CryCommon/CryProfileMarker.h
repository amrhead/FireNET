////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   CryProfileMarker.h
//  Version:     v1.00
//  Created:     11/12/2002 by Christopher Bolte.
//  Compilers:   Visual Studio.NET
//  Description: Platform Independent Profiling Marker
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#if !defined(CRY_PROFILE_MARKER_H_)
#define CRY_PROFILE_MARKER_H_

#include <IJobManager.h>
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// forward declaration for Implementation functions, these will be different 
// per platform/implementation
// TODO: Move the implementations later to it's own Platform Specific Library
namespace CryProfile {
	namespace detail {

		void SetThreadName( const char *pName );
		void SetProfilingEvent( const char *pName );
		void PushProfilingMarker( const char *pName );
		void PopProfilingMarker();

	} // namespace detail
} // namespace CryProfile


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Public Interface for use of Profile Marker
namespace CryProfile
{
	//////////////////////////////////////////////////////////////////////////
	// Tell the Profiling tool the name of the calling Thread
	inline void SetThreadName( const char *pName, ... ) 
	{ 
		va_list args;
		va_start(args, pName);

		// Format threaD name
		const int cBufferSize = 64;
		char threadName[cBufferSize];
		const int cNumCharsNeeded = vsnprintf(threadName, cBufferSize, pName, args);
		if(cNumCharsNeeded > cBufferSize - 1 || cNumCharsNeeded < 0 )
		{
			threadName[cBufferSize-1] = '\0'; // The WinApi only null terminates if strLen < bufSize
			CryWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "ProfileEvent: Thread name \"%s\" has been truncated. Max characters allowed: %i. ", pName, cBufferSize);
		}

		// Set thread name
		CryProfile::detail::SetThreadName(pName); 
		va_end(args);
	}

	//////////////////////////////////////////////////////////////////////////
	// Set a Profiling Event, this represents a single time, no duration event in the profiling Tool
	inline void SetProfilingEvent( const char *pName, ... ) 
	{ 
		va_list args;
		va_start(args, pName);

		// Format event name
		const int cBufferSize = 256;
		char eventName[cBufferSize];
		const int cNumCharsNeeded = vsnprintf(eventName, cBufferSize, pName, args);
		if(cNumCharsNeeded > cBufferSize - 1 || cNumCharsNeeded < 0 )
		{
			eventName[cBufferSize-1] = '\0'; // The WinApi only null terminates if strLen < bufSize
			CryWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "ProfileEvent: Event name \"%s\" has been truncated. Max characters allowed: %i. ", pName, cBufferSize);
		}

		// Set event
		CryProfile::detail::SetProfilingEvent(eventName); 
		va_end(args);
	}

	//////////////////////////////////////////////////////////////////////////
	// direct push/pop for profiling markers
	inline void PushProfilingMarker( const char *pName, ... ) 
	{ 
		va_list args;
		va_start(args, pName);

		// Format marker name
		const int cBufferSize = 256;
		char markerName[cBufferSize];
		const int cNumCharsNeeded = vsnprintf(markerName, cBufferSize, pName, args);
		if(cNumCharsNeeded > cBufferSize - 1 || cNumCharsNeeded < 0 )
		{
			markerName[cBufferSize-1] = '\0'; // The WinApi only null terminates if strLen < bufSize
			CryWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "ProfileEvent: Marker name \"%s\" has been truncated. Max characters allowed: %i. ", pName, cBufferSize);
		}

		// Set marker
		gEnv->pJobManager->PushProfilingMarker(markerName); 
		CryProfile::detail::PushProfilingMarker(markerName); 
		va_end(args);
	}

	//////////////////////////////////////////////////////////////////////////
	inline void PopProfilingMarker() { gEnv->pJobManager->PopProfilingMarker();CryProfile::detail::PopProfilingMarker(); }

	//////////////////////////////////////////////////////////////////////////
	// class to define a profile scope, to represent time events in profile tools
	class CScopedProfileMarker
	{
	public:
		inline CScopedProfileMarker( const char *pName, ... ) 
		{ 
			va_list args;
			va_start(args, pName);

			// Format event name
			const int cBufferSize = 256;
			char markerName[cBufferSize];
			const int cNumCharsNeeded = vsnprintf(markerName, cBufferSize, pName, args);
			if(cNumCharsNeeded > cBufferSize - 1 || cNumCharsNeeded < 0 )
			{
				markerName[cBufferSize-1] = '\0'; // The WinApi only null terminates if strLen < bufSize
				CryWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "ProfileEvent: Marker name \"%s\" has been truncated. Max characters allowed: %i. ", pName, cBufferSize);
			}

			// Set marker
			CryProfile::PushProfilingMarker(markerName);  
			va_end(args);
		}
		inline ~CScopedProfileMarker() { CryProfile::PopProfilingMarker(); }
	};

} // namespace CryProfile

// Util Macro to create scoped profiling markers
#define CRYPROFILE_CONCAT_(a,b) a ## b
#define CRYPROFILE_CONCAT(a,b) CRYPROFILE_CONCAT_(a,b)
#define CRYPROFILE_SCOPE_PROFILE_MARKER( ... ) CryProfile::CScopedProfileMarker CRYPROFILE_CONCAT( __scopedProfileMarker, __LINE__ )(__VA_ARGS__);
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// For Release Builds, provide emtpy inline variants of all profile functions
#if !defined(ENABLE_PROFILING_CODE) || defined(SANDBOX_EXPORTS) || defined(PLUGIN_EXPORTS)
inline void CryProfile::detail::SetThreadName( const char *pName ){}
inline void CryProfile::detail::SetProfilingEvent( const char *pName ){}
inline void CryProfile::detail::PushProfilingMarker( const char *pName ){}
inline void CryProfile::detail::PopProfilingMarker(){}
//For static builds
#elif defined(_LIB)
inline void CryProfile::detail::SetThreadName( const char *pName ){}
inline void CryProfile::detail::SetProfilingEvent( const char *pName ){}
inline void CryProfile::detail::PushProfilingMarker( const char *pName ){}
inline void CryProfile::detail::PopProfilingMarker(){}
#endif

#endif // CRY_PROFILE_MARKER_H_

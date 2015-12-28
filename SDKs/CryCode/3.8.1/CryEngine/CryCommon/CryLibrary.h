#ifndef CRYLIBRARY_H__
#define CRYLIBRARY_H__

/*!
	CryLibrary
	
	Convenience-Macros which abstract the use of DLLs/shared libraries in a platform independent way.
	A short explanation of the different macros follows:
	
	CrySharedLibrarySupported:
		This macro can be used to test if the current active platform supports shared library calls. The default
		value is false. This gets redefined if a certain platform (WIN32 or LINUX) is desired.

	CrySharedLibraryPrefix:
		The default prefix which will get prepended to library names in calls to CryLoadLibraryDefName
		(see below).

	CrySharedLibraryExtension:
		The default extension which will get appended to library names in calls to CryLoadLibraryDefName
		(see below).

	CryLoadLibrary(libName):
		Loads a shared library.

	CryLoadLibraryDefName(libName):
		Loads a shared library. The platform-specific default library prefix and extension are appended to the libName.
		This allows writing of somewhat platform-independent library loading code and is therefore the function
		which should be used most of the time, unless some special extensions are used (e.g. for plugins).
	
	CryGetProcAddress(libHandle, procName):
		Import function from the library presented by libHandle.
		
	CryFreeLibrary(libHandle):
		Unload the library presented by libHandle.
	
	HISTORY:
		03.03.2004 MarcoK
			- initial version
			- added to CryPlatform
*/

#include <stdio.h>

#if defined(WIN32)  || defined(DURANGO)
#ifdef WIN32
	#if !defined(WIN32_LEAN_AND_MEAN)
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <CryWindows.h>
	#define CryLoadLibrary(libName) ::LoadLibraryA(libName)
#elif defined(DURANGO)
	//For Durango
	extern HMODULE DurangoLoadLibrary( const char *libName );
	#define CryLoadLibrary(libName) DurangoLoadLibrary(libName)
#endif //WIN32
	#define CrySharedLibrarySupported true
	#define CrySharedLibraryPrefix ""
	#define CrySharedLibraryExtension ".dll"
	#define CryGetProcAddress(libHandle, procName) ::GetProcAddress((HMODULE)(libHandle), procName)
	#define CryFreeLibrary(libHandle) ::FreeLibrary((HMODULE)(libHandle))
#elif ((defined(LINUX) || defined(APPLE)))
	#include <dlfcn.h>
	#include <stdlib.h>
	#include "platform.h"

	// for compatibility with code written for windows
	#define CrySharedLibrarySupported true
	#define CrySharedLibraryPrefix "lib"
#if defined(APPLE)	
	#define CrySharedLibraryExtension ".dylib"
#else   
	#define CrySharedLibraryExtension ".so"
#endif	

	#define CryGetProcAddress(libHandle, procName) ::dlsym(libHandle, procName)
	#define CryFreeLibrary(libHandle) ::dlclose(libHandle)
	#define HMODULE void*
	static const char* gEnvName("MODULE_PATH");

	static const char* GetModulePath()
	{
		return getenv(gEnvName);
	}

	static void SetModulePath(const char* pModulePath)
	{
		setenv(gEnvName, pModulePath?pModulePath:"",true);
	}

	static HMODULE CryLoadLibrary(const char* libName, bool bLazy = false, bool bInModulePath = true)
	{
		//[K01]: linux version
		if (bInModulePath)
		{
			char path[_MAX_PATH];
			const char* modulePath = GetModulePath();
			sprintf(path, "%s/%s", modulePath ? modulePath : ".", libName);
			libName = path;
		}
#if defined(LINUX) && !defined(ANDROID)
		return ::dlopen(libName, (bLazy ? RTLD_LAZY : RTLD_NOW) | RTLD_DEEPBIND);
#else
		return ::dlopen(libName, bLazy ? RTLD_LAZY : RTLD_NOW);
#endif
	}
#else
#define CrySharedLibrarySupported false
#define CrySharedLibraryPrefix ""
#define CrySharedLibraryExtension ""
#define CryLoadLibrary(libName) NULL
#define CryGetProcAddress(libHandle, procName) NULL
#define CryFreeLibrary(libHandle)
#define GetModuleHandle(x)	0
#endif 
#define CryLibraryDefName(libName) CrySharedLibraryPrefix libName CrySharedLibraryExtension
#define CryLoadLibraryDefName(libName) CryLoadLibrary(CryLibraryDefName(libName))

#endif //CRYLIBRARY_H__

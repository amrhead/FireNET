////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   CryMemoryManager_impl.h
//  Version:     v1.00
//  Created:     27/7/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: Provides implementation for CryMemoryManager globally defined functions.
//               This file included only by platform_impl.h, do not include it directly in code!
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
 
#ifndef __CryMemoryManager_impl_h__
#define __CryMemoryManager_impl_h__
#pragma once

#ifdef _LIB
	#include <ISystem.h> // <> required for Interfuscator
#endif

#include "CryLibrary.h"

#define DLL_ENTRY_CRYMALLOC "CryMalloc"
#define DLL_ENTRY_CRYFREE "CryFree"
#define DLL_ENTRY_CRYREALLOC "CryRealloc"
#define DLL_ENTRY_CRYGETMEMSIZE "CryGetMemSize"
#define DLL_ENTRY_CRYCRTMALLOC "CrySystemCrtMalloc"
#define DLL_ENTRY_CRYCRTFREE "CrySystemCrtFree"
#define DLL_ENTRY_CRYCRTSIZE "CrySystemCrtSize"
#define DLL_ENTRY_GETMEMMANAGER "CryGetIMemoryManagerInterface"

//////////////////////////////////////////////////////////////////////////
// _PoolHelper definition.
//////////////////////////////////////////////////////////////////////////
struct _CryMemoryManagerPoolHelper
{
  typedef void *(*FNC_CryMalloc)(size_t size, size_t& allocated, size_t alignment);
	typedef void *(*FNC_CryCrtMalloc)(size_t size);
  typedef void *(*FNC_CryRealloc)(void *memblock, size_t size, size_t& allocated, size_t& oldsize, size_t alignment);
  typedef size_t (*FNC_CryFree)(void *p,size_t alignment);
	typedef size_t (*FNC_CryCrtFree)(void *p);
	typedef size_t (*FNC_CryCrtSize)(void *p);
	typedef void (*FNC_CryGetIMemoryManagerInterface)( void **p );
	typedef void *(*FNC_CryCrtRealloc)(void *p, size_t size);

  static volatile LONG allocatedMemory;
  static volatile LONG freedMemory;
  static volatile LONG requestedMemory;
  static volatile int numAllocations;

//	typedef size_t (*FNC_CryFree)(void);
	typedef size_t (*FNC_CryGetMemSize)(void *p, size_t);
	typedef int (*FNC_CryStats)(char *buf);
	
	static FNC_CryMalloc _CryMalloc;
	static FNC_CryRealloc _CryRealloc;
	static FNC_CryFree _CryFree;
	static FNC_CryGetMemSize _CryGetMemSize;
	static FNC_CryCrtMalloc _CryCrtMalloc;
	static FNC_CryCrtFree _CryCrtFree;
	static FNC_CryCrtRealloc _CryCrtRealloc;
	static FNC_CryCrtSize _CryCrtSize;
	static FNC_CryGetIMemoryManagerInterface _CryGetIMemoryManagerInterface;


	static int m_bInitialized;

	static void Init()
	{
		if (m_bInitialized)
			return;

		m_bInitialized = 1;
    allocatedMemory = 0;
    freedMemory = 0;
    requestedMemory = 0;
    numAllocations = 0;

#ifndef _LIB
		HMODULE hMod;
		int iter;
#if defined(LINUX) || defined(APPLE)
		for(iter=0,hMod=::dlopen(NULL, RTLD_LAZY); hMod; iter++)
#elif defined(DURANGO)
		// HMODULE set to NULL should take the main executable where allocator functions should be defined, 
		// but needs to be tested. GetModuleHandle() is not available on Metro style apps.
		hMod = CryLoadLibrary("CrySystem.dll"); 
		for(iter=0; iter < 2; iter++)
#else //LINUX MAC
		for(iter=0,hMod=GetModuleHandle(0); hMod; iter++)
#endif //LINUX MAC
		{
			_CryMalloc=(FNC_CryMalloc)CryGetProcAddress(hMod,DLL_ENTRY_CRYMALLOC);
			_CryRealloc=(FNC_CryRealloc)CryGetProcAddress(hMod,DLL_ENTRY_CRYREALLOC);
			_CryFree=(FNC_CryFree)CryGetProcAddress(hMod,DLL_ENTRY_CRYFREE);
			_CryGetMemSize=(FNC_CryGetMemSize)CryGetProcAddress(hMod,DLL_ENTRY_CRYGETMEMSIZE);
			_CryCrtMalloc=(FNC_CryCrtMalloc)CryGetProcAddress(hMod,DLL_ENTRY_CRYCRTMALLOC);
			_CryCrtSize=(FNC_CryCrtSize)CryGetProcAddress(hMod,DLL_ENTRY_CRYCRTSIZE);
			_CryCrtFree=(FNC_CryCrtFree)CryGetProcAddress(hMod,DLL_ENTRY_CRYCRTFREE);
			_CryGetIMemoryManagerInterface=(FNC_CryGetIMemoryManagerInterface)CryGetProcAddress(hMod,DLL_ENTRY_GETMEMMANAGER);

			if ((_CryMalloc && _CryRealloc && _CryFree && _CryGetMemSize && _CryCrtMalloc && _CryCrtFree && _CryCrtSize && _CryGetIMemoryManagerInterface) || iter==1)
				break;

			hMod = CryLoadLibraryDefName("CrySystem");
			if (!hMod) break;
		}
		if (!hMod || !_CryMalloc || !_CryRealloc || !_CryFree || !_CryGetMemSize ||! _CryCrtMalloc || !_CryCrtFree || !_CryCrtSize || !_CryGetIMemoryManagerInterface)
		{
#ifdef WIN32
			MessageBox(NULL, "Could not access " CryLibraryDefName("CrySystem") " (check working directory)", "Memory Manager", MB_OK);
#else
			if (!hMod)
			{
#if defined(DURANGO)
				OutputDebugStringA("Could not access " CryLibraryDefName("CrySystem") " (check working directory)");
#else
				OutputDebugString("Could not access " CryLibraryDefName("CrySystem") " (check working directory)");
#endif
			}
			else
			{
#if defined(DURANGO)
				OutputDebugStringA("Could not get Memory Functions in " CryLibraryDefName("CrySystem"));
#else
				OutputDebugString("Could not get Memory Functions in " CryLibraryDefName("CrySystem"));
#endif
			}
#endif
			exit(1);
		};
#else //_LIB
		_CryMalloc=CryMalloc;
		_CryRealloc=CryRealloc;
		_CryFree=CryFree;
		_CryGetMemSize=CryGetMemSize;
		_CryCrtMalloc=CrySystemCrtMalloc;
		_CryCrtRealloc=CrySystemCrtRealloc;
		_CryCrtFree=(FNC_CryCrtFree)CrySystemCrtFree;
		_CryCrtSize=(FNC_CryCrtSize)CrySystemCrtSize;
		_CryGetIMemoryManagerInterface=(FNC_CryGetIMemoryManagerInterface)CryGetIMemoryManagerInterface;
#endif //_LIB
	}

	static void FakeAllocation( long size )
	{
		if (!m_bInitialized)
			Init();
		CryInterlockedExchangeAdd(&allocatedMemory, size);
		CryInterlockedExchangeAdd(&requestedMemory, size);
		GetISystem()->GetIMemoryManager()->FakeAllocation( size );
	}

	//////////////////////////////////////////////////////////////////////////
	static IMemoryManager* GetIMemoryManager()
	{
		if (!m_bInitialized)
			Init();
		void *ptr = 0;
		#ifdef _LIB
			CryGetIMemoryManagerInterface((void**)&ptr);
		#else
		if (_CryGetIMemoryManagerInterface)
				_CryGetIMemoryManagerInterface((void**)&ptr);
		#endif
		return (IMemoryManager*)ptr;
	}
 
	static void GetMemoryInfo( CryModuleMemoryInfo *pMmemInfo )
	{
		pMmemInfo->allocated = allocatedMemory;
		pMmemInfo->freed = freedMemory;
		pMmemInfo->requested = requestedMemory;
		pMmemInfo->num_allocations = numAllocations;
#ifdef CRY_STRING
		pMmemInfo->CryString_allocated = string::_usedMemory(0) + wstring::_usedMemory(0);
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	// Local version of allocations, does memory counting per module.
	//////////////////////////////////////////////////////////////////////////
  static ILINE void* Malloc(size_t size,size_t alignment )
	{
		if (!m_bInitialized)
			Init();

		size_t allocated;
		void * p = _CryMalloc( size, allocated, alignment );

    CryInterlockedExchangeAdd(&allocatedMemory, allocated);
    CryInterlockedExchangeAdd(&requestedMemory, size);
    CryInterlockedIncrement(&numAllocations);

		return p;
	}

	//////////////////////////////////////////////////////////////////////////
	static ILINE void*	Realloc(void *memblock,size_t size, size_t alignment )	
	{
		if (!m_bInitialized)
			Init();

		size_t allocated,oldsize;
		void * p=  _CryRealloc( memblock,size,allocated,oldsize,alignment );

    CryInterlockedExchangeAdd(&allocatedMemory, allocated);
		CryInterlockedExchangeAdd(&requestedMemory,size);
		CryInterlockedIncrement(&numAllocations);

    CryInterlockedExchangeAdd(&freedMemory,oldsize);

		return p;
    }
			
	//////////////////////////////////////////////////////////////////////////
	static ILINE size_t Free( void *memblock,size_t alignment )
	{
		size_t freed = 0;
		if (memblock != 0)
		{
			freed = _CryFree( memblock,alignment );
      CryInterlockedExchangeAdd(&freedMemory, freed);
		}
		return freed;
	}

  static ILINE size_t	MemSize(void* ptr, size_t sz)
	{
		if (!m_bInitialized)
			Init();

		size_t realSize = _CryGetMemSize( ptr, sz );

		return realSize;
	}
};

//////////////////////////////////////////////////////////////////////////
// Static variables.
//////////////////////////////////////////////////////////////////////////
volatile LONG _CryMemoryManagerPoolHelper::allocatedMemory;
volatile LONG _CryMemoryManagerPoolHelper::freedMemory;
volatile LONG _CryMemoryManagerPoolHelper::requestedMemory;
volatile int  _CryMemoryManagerPoolHelper::numAllocations;

_CryMemoryManagerPoolHelper::FNC_CryMalloc _CryMemoryManagerPoolHelper::_CryMalloc = NULL;
_CryMemoryManagerPoolHelper::FNC_CryGetMemSize _CryMemoryManagerPoolHelper::_CryGetMemSize = NULL;
_CryMemoryManagerPoolHelper::FNC_CryRealloc _CryMemoryManagerPoolHelper::_CryRealloc = NULL;
_CryMemoryManagerPoolHelper::FNC_CryFree _CryMemoryManagerPoolHelper::_CryFree = NULL;
_CryMemoryManagerPoolHelper::FNC_CryCrtMalloc _CryMemoryManagerPoolHelper::_CryCrtMalloc = NULL;
_CryMemoryManagerPoolHelper::FNC_CryCrtRealloc _CryMemoryManagerPoolHelper::_CryCrtRealloc = NULL;
_CryMemoryManagerPoolHelper::FNC_CryCrtFree _CryMemoryManagerPoolHelper::_CryCrtFree = NULL;
_CryMemoryManagerPoolHelper::FNC_CryCrtSize _CryMemoryManagerPoolHelper::_CryCrtSize = NULL;
_CryMemoryManagerPoolHelper::FNC_CryGetIMemoryManagerInterface _CryMemoryManagerPoolHelper::_CryGetIMemoryManagerInterface = NULL;

int _CryMemoryManagerPoolHelper::m_bInitialized = 0;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#  if !defined(NOT_USE_CRY_MEMORY_MANAGER)
//////////////////////////////////////////////////////////////////////////
void* CryModuleMalloc( size_t size ) throw()
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CryMalloc);
	void* ret = _CryMemoryManagerPoolHelper::Malloc(size,0);
	MEMREPLAY_SCOPE_ALLOC(ret, size, 0);
	return ret;
};

//////////////////////////////////////////////////////////////////////////
void* CryModuleRealloc( void *ptr,size_t size )  throw()
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CryMalloc);
	void* ret = _CryMemoryManagerPoolHelper::Realloc(ptr,size, 0);
	MEMREPLAY_SCOPE_REALLOC(ptr, ret, size, 0);
	return ret;
};

//////////////////////////////////////////////////////////////////////////
void CryModuleFree( void *ptr ) throw()
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CryMalloc);
	_CryMemoryManagerPoolHelper::Free(ptr,0);
	MEMREPLAY_SCOPE_FREE(ptr);
};

size_t CryModuleMemSize( void* ptr, size_t sz) throw()
{
	return _CryMemoryManagerPoolHelper::MemSize(ptr, sz);
}

//////////////////////////////////////////////////////////////////////////
void* CryModuleMemalign( size_t size, size_t alignment ) throw()
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CryMalloc);
	void* ret = _CryMemoryManagerPoolHelper::Malloc(size, alignment);
	MEMREPLAY_SCOPE_ALLOC(ret, size, alignment);
	return ret;
};

void CryModuleMemalignFree( void *ptr ) throw()
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CryMalloc);
	_CryMemoryManagerPoolHelper::Free(ptr,1000); // Free with alignment
	MEMREPLAY_SCOPE_FREE(ptr);
};

void* CryModuleCalloc(size_t a, size_t b) throw()
{ 
	void* p = CryModuleMalloc(a*b);memset(p, 0, a*b);
	return p;
}

//////////////////////////////////////////////////////////////////////////
void* CryModuleReallocAlign( void *ptr, size_t size, size_t alignment )  throw()
{
	MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CryMalloc);
	void* ret = _CryMemoryManagerPoolHelper::Realloc(ptr,size, alignment);
	MEMREPLAY_SCOPE_REALLOC(ptr, ret, size, alignment);
	return ret;
};
#endif //!defined(NOT_USE_CRY_MEMORY_MANAGER)

//////////////////////////////////////////////////////////////////////////
void CryModuleGetMemoryInfo( CryModuleMemoryInfo *pMemInfo )
{
  _CryMemoryManagerPoolHelper::GetMemoryInfo(pMemInfo);
};
//////////////////////////////////////////////////////////////////////////
void CryGetMemoryInfoForModule(CryModuleMemoryInfo * pInfo)
{
	_CryMemoryManagerPoolHelper::GetMemoryInfo(pInfo);
};

void *CryCrtMalloc(size_t size)
{
	_CryMemoryManagerPoolHelper::Init();
	return _CryMemoryManagerPoolHelper::_CryCrtMalloc(size);
}

size_t CryCrtFree(void *p)
{
	return _CryMemoryManagerPoolHelper::_CryCrtFree(p);
};

size_t CryCrtSize(void *p)
{
	return _CryMemoryManagerPoolHelper::_CryCrtSize(p);
};

// Redefine new & delete for entire module.
#if !defined(NOT_USE_CRY_MEMORY_MANAGER)
	#if !defined(_LIB) && !defined(NEW_OVERRIDEN)
			PREFAST_SUPPRESS_WARNING(28251)
			void * __cdecl operator new   (size_t size)
			{
				MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtNew);
#if defined(CRY_FORCE_MALLOC_NEW_ALIGN)
				void* ret = CryModuleMemalign(size, TARGET_DEFAULT_ALIGN);
#else
				void* ret = CryModuleMalloc(size);
#endif
				MEMREPLAY_SCOPE_ALLOC(ret, size, 0);
				return ret;
			}
			PREFAST_SUPPRESS_WARNING(28251)
			void * __cdecl operator new[] (size_t size)
			{
				MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtNewArray);
#if defined(CRY_FORCE_MALLOC_NEW_ALIGN)
				void* ret = CryModuleMemalign(size, TARGET_DEFAULT_ALIGN);
#else
				void* ret = CryModuleMalloc(size);
#endif
				MEMREPLAY_SCOPE_ALLOC(ret, size, 0);
				return ret;
            }

#if defined(LINUX)
#    if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)
#        define NO_EXCEPTIONS _GLIBCXX_USE_NOEXCEPT
#    else
#        define NO_EXCEPTIONS noexcept
#    endif
#elif defined(APPLE)
#        define NO_EXCEPTIONS _NOEXCEPT
#else
#        define NO_EXCEPTIONS
#endif

            void __cdecl operator delete  (void *p) NO_EXCEPTIONS
			{
				MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtNew);
#if defined(CRY_FORCE_MALLOC_NEW_ALIGN)
				CryModuleMemalignFree(p);
#else
				CryModuleFree(p);
#endif
				MEMREPLAY_SCOPE_FREE(p);
            }
            void __cdecl operator delete[]  (void *p) NO_EXCEPTIONS
			{
				MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtNewArray);
#if defined(CRY_FORCE_MALLOC_NEW_ALIGN)
				CryModuleMemalignFree(p);
#else
				CryModuleFree(p);
#endif
				MEMREPLAY_SCOPE_FREE(p);
            }

#undef NO_EXCEPTIONS

	#endif //!defined(_LIB) && !defined(NEW_OVERRIDEN)
#endif // !defined(NOT_USE_CRY_MEMORY_MANAGER)

//////////////////////////////////////////////////////////////////////////
#ifndef MEMMAN_STATIC
IMemoryManager *CryGetIMemoryManager()
{
	static IMemoryManager* memMan = 0;
	if (!memMan)
		memMan = _CryMemoryManagerPoolHelper::GetIMemoryManager();
	return memMan;
}
#endif //!defined(_LIB)

// ~memReplay

#if !defined(NOT_USE_CRY_MEMORY_MANAGER)
    #if defined(_LIB) && !defined(NEW_OVERRIDEN)
            void * __cdecl operator new   (size_t size)
            {
                MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtNew);
                void* ret = CryModuleMalloc(size);
                MEMREPLAY_SCOPE_ALLOC(ret, size, 0);
                return ret;
            }

            void * __cdecl operator new[] (size_t size)
            {
                MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtNewArray);
                void* ret = CryModuleMalloc(size);
                MEMREPLAY_SCOPE_ALLOC(ret, size, 0);
                return ret;
            }

            void __cdecl operator delete  (void *p) CRYMM_THROW
            {
                MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtNew);
                CryModuleFree(p);
                MEMREPLAY_SCOPE_FREE(p);
            }

            void __cdecl operator delete[](void *p) CRYMM_THROW
            {
                MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtNewArray);
                CryModuleFree(p);
                MEMREPLAY_SCOPE_FREE(p);
            }

            #ifdef ORBIS
            void* operator new (_CSTD size_t size, const std::nothrow_t &nothrow) CRYMM_THROW
            {
                MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtNew);
                void* ret = CryModuleMalloc(size);
                MEMREPLAY_SCOPE_ALLOC(ret, size, 0);
                return ret;
            }

            void operator delete(void *p, const std::nothrow_t&) CRYMM_THROW
            {
                MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtNew);
                CryModuleFree(p);
                MEMREPLAY_SCOPE_FREE(p);
            }

            void operator delete[](void *p, const std::nothrow_t&) CRYMM_THROW
            {
                MEMREPLAY_SCOPE(EMemReplayAllocClass::C_UserPointer, EMemReplayUserPointerClass::C_CrtNewArray);
                CryModuleFree(p);
                MEMREPLAY_SCOPE_FREE(p);
            }
            #endif // defined(ORBIS)
    #endif // defined(_LIB) && !defined(NEW_OVERRIDEN)
#endif // !defined(NOT_USE_CRY_MEMORY_MANAGER)


#endif // __CryMemoryManager_impl_h__

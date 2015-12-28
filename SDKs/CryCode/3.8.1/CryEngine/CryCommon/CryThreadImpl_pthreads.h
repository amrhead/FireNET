/////////////////////////////////////////////////////////////////////////////
//
// Crytek Source File
// Copyright (C), Crytek Studios, 2001-2006.
//
// History:
// Jun 20, 2006: Created by Sascha Demetrio
//
/////////////////////////////////////////////////////////////////////////////

#include "CryThread_pthreads.h"

#if PLATFORM_SUPPORTS_THREADLOCAL
THREADLOCAL CrySimpleThreadSelf
	*CrySimpleThreadSelf::m_Self = NULL;
#else
TLS_DEFINE(CrySimpleThreadSelf*, g_CrySimpleThreadSelf)
#endif


//////////////////////////////////////////////////////////////////////////
// CryEvent(Timed) implementation
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CryEventTimed::Reset()
{
	m_lockNotify.Lock();
	m_flag = false;
	m_lockNotify.Unlock();
}

//////////////////////////////////////////////////////////////////////////
void CryEventTimed::Set()
{
	m_lockNotify.Lock();
	m_flag = true;
	m_cond.Notify();
	m_lockNotify.Unlock();
}

//////////////////////////////////////////////////////////////////////////
void CryEventTimed::Wait()
{
	m_lockNotify.Lock();
	if (!m_flag)
		m_cond.Wait(m_lockNotify);
	m_flag	=	false;
	m_lockNotify.Unlock();
}

//////////////////////////////////////////////////////////////////////////
bool CryEventTimed::Wait( const uint32 timeoutMillis )
{
	bool bResult = true;
	m_lockNotify.Lock();
	if (!m_flag)
		bResult = m_cond.TimedWait(m_lockNotify,timeoutMillis);
	m_flag	=	false;
	m_lockNotify.Unlock();
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////
// CryCriticalSection implementation
///////////////////////////////////////////////////////////////////////////////
typedef CryLockT<CRYLOCK_RECURSIVE> TCritSecType;

void  CryDeleteCriticalSection( void *cs )
{
	delete ((TCritSecType *)cs);
}

void  CryEnterCriticalSection( void *cs )
{
	((TCritSecType*)cs)->Lock();
}

bool  CryTryCriticalSection( void *cs )
{
	return false;
}

void  CryLeaveCriticalSection( void *cs )
{
	((TCritSecType*)cs)->Unlock();
}

void  CryCreateCriticalSectionInplace(void* pCS)
{
	new (pCS) TCritSecType;
}

void CryDeleteCriticalSectionInplace( void *)
{
}

void* CryCreateCriticalSection()
{
	return (void*) new TCritSecType;
}

//////////////////////////////////////////////////////////////////////////
// LINUX64 Implementation of Lockless Single Linked List
//////////////////////////////////////////////////////////////////////////
#if defined(LINUX64) || defined(MAC) || defined(IOS_SIMULATOR) && !defined (ORBIS)
	typedef __uint128_t uint128;

	//////////////////////////////////////////////////////////////////////////
	// Implementation for Linux64 with gcc using __int128_t
	//////////////////////////////////////////////////////////////////////////
	void CryInterlockedPushEntrySList( SLockFreeSingleLinkedListHeader& list,  SLockFreeSingleLinkedListEntry &element )
	{
		uint64 curSetting[2];
		uint64 newSetting[2];
		uint64 newPointer = (uint64)&element;
		do 
		{
			curSetting[0] = (uint64)list.pNext;
			curSetting[1] = list.salt;
			element.pNext = (SLockFreeSingleLinkedListEntry*)curSetting[0];
			newSetting[0] = newPointer; // new pointer
			newSetting[1] = curSetting[1]+1; // new salt
		}
		// while (false == __sync_bool_compare_and_swap( (volatile uint128*)&list.pNext,*(uint128*)&curSetting[0],*(uint128*)&newSetting[0] ));
	   while (0 == _InterlockedCompareExchange128((volatile int64*)&list.pNext, (int64)newSetting[1], (int64)newSetting[0], (int64*)&curSetting[0])); 
	}

	//////////////////////////////////////////////////////////////////////////
	void* CryInterlockedPopEntrySList(  SLockFreeSingleLinkedListHeader& list )
	{
		uint64 curSetting[2];
		uint64 newSetting[2];
		do 
		{
			curSetting[1]=list.salt;
			curSetting[0]=(uint64)list.pNext;
			if (curSetting[0]==0)
				return NULL;
			newSetting[0] = *(uint64*)curSetting[0]; // new pointer
			newSetting[1] = curSetting[1]+1; // new salt
		}
		//while (false == __sync_bool_compare_and_swap( (volatile uint128*)&list.pNext,*(uint128*)&curSetting[0],*(uint128*)&newSetting[0] ));
	   while (0 == _InterlockedCompareExchange128((volatile int64*)&list.pNext, (int64)newSetting[1], (int64)newSetting[0], (int64*)&curSetting[0])); 
		return (void*)curSetting[0];
	}

	//////////////////////////////////////////////////////////////////////////
	void CryInitializeSListHead(SLockFreeSingleLinkedListHeader& list)
	{
		list.salt = 0;
		list.pNext = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void* CryInterlockedFlushSList(SLockFreeSingleLinkedListHeader& list)
	{
		uint64 curSetting[2];
		uint64 newSetting[2];
		uint64 newSalt;
		uint64 newPointer;
		do 
		{
			curSetting[1]=list.salt;
			curSetting[0]=(uint64)list.pNext;
			if (curSetting[0]==0)
				return NULL;
			newSetting[0] = 0;
			newSetting[1] = curSetting[1]+1;
		}
	//	while (false == __sync_bool_compare_and_swap( (volatile uint128*)&list.pNext,*(uint128*)&curSetting[0],*(uint128*)&newSetting[0] ));
	   while (0 == _InterlockedCompareExchange128((volatile int64*)&list.pNext, (int64)newSetting[1], (int64)newSetting[0], (int64*)&curSetting[0])); 
		return (void*)curSetting[0];
	}
	//////////////////////////////////////////////////////////////////////////
#elif defined(LINUX32)
	//////////////////////////////////////////////////////////////////////////
	// Implementation for Linux32 with gcc using uint64
	//////////////////////////////////////////////////////////////////////////
	void CryInterlockedPushEntrySList( SLockFreeSingleLinkedListHeader& list,  SLockFreeSingleLinkedListEntry &element )
	{
		uint32 curSetting[2];
		uint32 newSetting[2];
		uint32 newPointer = (uint32)&element;
		do 
		{
			curSetting[0] = (uint32)list.pNext;
			curSetting[1] = list.salt;
			element.pNext = (SLockFreeSingleLinkedListEntry*)curSetting[0];
			newSetting[0] = newPointer; // new pointer
			newSetting[1] = curSetting[1]+1; // new salt
		}
		while (false == __sync_bool_compare_and_swap( (volatile uint64*)&list.pNext,*(uint64*)&curSetting[0],*(uint64*)&newSetting[0] ));
	}

	//////////////////////////////////////////////////////////////////////////
	void* CryInterlockedPopEntrySList(  SLockFreeSingleLinkedListHeader& list )
	{
		uint32 curSetting[2];
		uint32 newSetting[2];
		do 
		{
			curSetting[1]=list.salt;
			curSetting[0]=(uint32)list.pNext;
			if (curSetting[0]==0)
				return NULL;
			newSetting[0] = *(uint32*)curSetting[0]; // new pointer
			newSetting[1] = curSetting[1]+1; // new salt
		}
		while (false == __sync_bool_compare_and_swap( (volatile uint64*)&list.pNext,*(uint64*)&curSetting[0],*(uint64*)&newSetting[0] ));
		return (void*)curSetting[0];
	}

	//////////////////////////////////////////////////////////////////////////
	void CryInitializeSListHead(SLockFreeSingleLinkedListHeader& list)
	{
		list.salt = 0;
		list.pNext = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void* CryInterlockedFlushSList(SLockFreeSingleLinkedListHeader& list)
	{
		uint32 curSetting[2];
		uint32 newSetting[2];
		uint32 newSalt;
		uint32 newPointer;
		do 
		{
			curSetting[1]=list.salt;
			curSetting[0]=(uint32)list.pNext;
			if (curSetting[0]==0)
				return NULL;
			newSetting[0] = 0;
			newSetting[1] = curSetting[1]+1;
		}
		while (false == __sync_bool_compare_and_swap( (volatile uint64*)&list.pNext,*(uint64*)&curSetting[0],*(uint64*)&newSetting[0] ));
		return (void*)curSetting[0];
	}
	//////////////////////////////////////////////////////////////////////////
#endif

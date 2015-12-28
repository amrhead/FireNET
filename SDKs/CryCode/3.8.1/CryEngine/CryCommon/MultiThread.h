////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2005.
// -------------------------------------------------------------------------
//  File name:   MultiThread.h
//  Version:     v1.00
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __MultiThread_h__
#define __MultiThread_h__
#pragma once

#if defined(APPLE) || defined(LINUX)
#include <sched.h>
#endif

#include "CryAssert.h"

#define WRITE_LOCK_VAL (1<<16)

//as PowerPC operates via cache line reservation, lock variables should reside ion their own cache line
template <class T>
struct SAtomicVar
{
	T val;

	inline operator T()const{return val;}
	inline operator T()volatile const{return val;}
	inline void operator =(const T& rV){val = rV;return *this;}
	inline void Assign(const T& rV){val = rV;}
	inline void Assign(const T& rV)volatile{val = rV;}
	inline T* Addr() {return &val;}
	inline volatile T* Addr() volatile {return &val;}

	inline bool operator<(const T& v)const{return val < v;}
	inline bool operator<(const SAtomicVar<T>& v)const{return val < v.val;}
	inline bool operator>(const T& v)const{return val > v;}
	inline bool operator>(const SAtomicVar<T>& v)const{return val > v.val;}
	inline bool operator<=(const T& v)const{return val <= v;}
	inline bool operator<=(const SAtomicVar<T>& v)const{return val <= v.val;}
	inline bool operator>=(const T& v)const{return val >= v;}
	inline bool operator>=(const SAtomicVar<T>& v)const{return val >= v.val;}
	inline bool operator==(const T& v)const{return val == v;}
	inline bool operator==(const SAtomicVar<T>& v)const{return val == v.val;}
	inline bool operator!=(const T& v)const{return val != v;}
	inline bool operator!=(const SAtomicVar<T>& v)const{return val != v.val;}
	inline T operator*(const T& v)const{return val * v;}
	inline T operator/(const T& v)const{return val / v;}
	inline T operator+(const T& v)const{return val + v;}
	inline T operator-(const T& v)const{return val - v;}

	inline bool operator<(const T& v)volatile const{return val < v;}
	inline bool operator<(const SAtomicVar<T>& v)volatile const{return val < v.val;}
	inline bool operator>(const T& v)volatile const{return val > v;}
	inline bool operator>(const SAtomicVar<T>& v)volatile const{return val > v.val;}
	inline bool operator<=(const T& v)volatile const{return val <= v;}
	inline bool operator<=(const SAtomicVar<T>& v)volatile const{return val <= v.val;}
	inline bool operator>=(const T& v)volatile const{return val >= v;}
	inline bool operator>=(const SAtomicVar<T>& v)volatile const{return val >= v.val;}
	inline bool operator==(const T& v)volatile const{return val == v;}
	inline bool operator==(const SAtomicVar<T>& v)volatile const{return val == v.val;}
	inline bool operator!=(const T& v)volatile const{return val != v;}
	inline bool operator!=(const SAtomicVar<T>& v)volatile const{return val != v.val;}
	inline T operator*(const T& v)volatile const{return val * v;}
	inline T operator/(const T& v)volatile const{return val / v;}
	inline T operator+(const T& v)volatile const{return val + v;}
	inline T operator-(const T& v)volatile const{return val - v;}
};

typedef SAtomicVar<int> TIntAtomic;
typedef SAtomicVar<unsigned int> TUIntAtomic;
typedef SAtomicVar<float> TFloatAtomic;

#define __add_db16cycl__ NIntrinsics::YieldFor16Cycles();

	void CrySpinLock(volatile int *pLock,int checkVal,int setVal);
  void CryReleaseSpinLock	(volatile int*, int);

	LONG   CryInterlockedIncrement( int volatile *lpAddend );
	LONG   CryInterlockedDecrement( int volatile *lpAddend );
	LONG   CryInterlockedExchangeAdd(LONG volatile * lpAddend, LONG Value);
	LONG	 CryInterlockedCompareExchange(LONG volatile * dst, LONG exchange, LONG comperand);
	void*	 CryInterlockedCompareExchangePointer(void* volatile * dst, void* exchange, void* comperand);
  void*	 CryInterlockedExchangePointer       (void* volatile * dst, void* exchange);

	void*  CryCreateCriticalSection();
  void   CryCreateCriticalSectionInplace(void*);
	void   CryDeleteCriticalSection( void *cs );
  void   CryDeleteCriticalSectionInplace( void *cs );
	void   CryEnterCriticalSection( void *cs );
	bool   CryTryCriticalSection( void *cs );
	void   CryLeaveCriticalSection( void *cs );

#ifdef ORBIS
	LONG   CryInterlockedExchange(LONG volatile *addr, LONG exchange);
	#define InterlockedExchange CryInterlockedExchange
#endif

ILINE void CrySpinLock(volatile int *pLock,int checkVal,int setVal)
{ 
#ifdef _CPU_X86
# ifdef __GNUC__
	int val;
	__asm__ __volatile__ (
		"0:     mov %[checkVal], %%eax\n"
		"       lock cmpxchg %[setVal], (%[pLock])\n"
		"       jnz 0b"
		: "=m" (*pLock)
		: [pLock] "r" (pLock), "m" (*pLock),
		  [checkVal] "m" (checkVal),
		  [setVal] "r" (setVal)
		: "eax", "cc", "memory"
		);
# else //!__GNUC__
	__asm
	{
		mov edx, setVal
		mov ecx, pLock
Spin:
		// Trick from Intel Optimizations guide
#ifdef _CPU_SSE
		pause
#endif 
		mov eax, checkVal
		lock cmpxchg [ecx], edx
		jnz Spin
	}
# endif //!__GNUC__
#else // !_CPU_X86
#if defined(ORBIS)
	uint64 loops = 0;
	while (sceAtomicCompareAndSwap32((volatile int32_t*)pLock, (int32_t)checkVal, (int32_t)setVal)!=checkVal)
	{
		_mm_pause();

		if (!(++loops & 0x7F))
		{
			sceKernelUsleep(1); // give threads with other prio chance to run
		}
		else if(!(loops & 0x3F))
		{
			scePthreadYield(); // give threads with same prio chance to run
		}
	}

# elif defined(APPLE) || defined(LINUX)
//    register int val;
//	__asm__ __volatile__ (
//		"0:     mov %[checkVal], %%eax\n"
//		"       lock cmpxchg %[setVal], (%[pLock])\n"
//		"       jnz 0b"
//		: "=m" (*pLock)
//		: [pLock] "r" (pLock), "m" (*pLock),
//		  [checkVal] "m" (checkVal),
//		  [setVal] "r" (setVal)
//		: "eax", "cc", "memory"
//		);
	//while(CryInterlockedCompareExchange((volatile long*)pLock,setVal,checkVal)!=checkVal) ;
	uint loops = 0;
	while ( __sync_val_compare_and_swap((volatile int32_t*)pLock, (int32_t)checkVal, (int32_t)setVal) != checkVal)
	{
#	if !defined (ANDROID)
		_mm_pause();
#	endif 

		if(!(++loops & 0x7F))
		{
			usleep(1); // give threads with other prio chance to run
		}
		else if (!(loops & 0x3F))
		{
			sched_yield(); // give threads with same prio chance to run
		}		
	}
# else
	// NOTE: The code below will fail on 64bit architectures!
	uint loops = 0;
	while(_InterlockedCompareExchange((volatile LONG*)pLock,setVal,checkVal)!=checkVal) 
	{
		_mm_pause();
	}
# endif
#endif
}

ILINE void CryReleaseSpinLock(volatile int *pLock,int setVal)
{ 
  *pLock = setVal;
}

//////////////////////////////////////////////////////////////////////////
#if defined(APPLE) || defined(LINUX64)
// Fixes undefined reference to CryInterlockedAdd(unsigned long volatile*, long) on
// Mac and linux.
ILINE void CryInterLockedAdd(volatile LONG *pVal, LONG iAdd)
{
    (void) CryInterlockedExchangeAdd(pVal,iAdd); 
}


/*
ILINE void CryInterLockedAdd(volatile unsigned long *pVal, long iAdd)
{
    long r;
	__asm__ __volatile__ (
	#if defined(LINUX64) || defined(MAC)  // long is 64 bits on amd64.
		"lock ; xaddq %0, (%1) \n\t"
	#else
		"lock ; xaddl %0, (%1) \n\t"
	#endif
		: "=r" (r)
		: "r" (pVal), "0" (iAdd)
		: "memory"
	);
    (void) r;
}*/
/*ILINE void CryInterlockedAdd(volatile size_t *pVal, ptrdiff_t iAdd) {
   //(void)CryInterlockedExchangeAdd((volatile long*)pVal,(long)iAdd);
    (void) __sync_fetch_and_add(pVal,iAdd);
}*/

#endif
ILINE void CryInterlockedAdd(volatile int *pVal, int iAdd)
{
#ifdef _CPU_X86
# ifdef __GNUC__
	__asm__ __volatile__ (
		"        lock add %[iAdd], (%[pVal])\n"
		: "=m" (*pVal)
		: [pVal] "r" (pVal), "m" (*pVal), [iAdd] "r" (iAdd)
		);
# else
	__asm
	{
		mov edx, pVal
		mov eax, iAdd
		lock add [edx], eax
	}
# endif
#else
	// NOTE: The code below will fail on 64bit architectures!
#if defined(_WIN64)
  _InterlockedExchangeAdd((volatile LONG*)pVal,iAdd);
# elif defined(ORBIS)
	sceAtomicAdd32((int32_t*)pVal, (int32_t)iAdd);
#elif defined(APPLE) || defined(LINUX)
  CryInterlockedExchangeAdd((volatile LONG*)pVal,iAdd);
#elif defined(APPLE)
	OSAtomicAdd32(iAdd, (volatile LONG*)pVal);
#else
  InterlockedExchangeAdd((volatile LONG*)pVal,iAdd);
#endif

#endif
}

ILINE void CryInterlockedAddSize(volatile size_t* pVal, ptrdiff_t iAdd)
{
#if defined(PLATFORM_64BIT)
#if defined(_WIN64)
  _InterlockedExchangeAdd64((volatile __int64*)pVal,iAdd);
#elif defined(ORBIS)
	sceAtomicAdd64((int64_t*)pVal, (int64_t)iAdd);
#elif defined(WIN32) || defined(DURANGO)
  InterlockedExchangeAdd64((volatile LONG64*)pVal,iAdd);
#elif defined(APPLE) || defined(LINUX)
  (void)__sync_fetch_and_add((int64_t*)pVal, (int64_t)iAdd);
#else
	int64 x, n;
	do
	{
		x = (int64)*pVal;
		n = x + iAdd;
	}
	while (CryInterlockedCompareExchange64((volatile int64*)pVal, n, x) != x);
#endif
#else
	CryInterlockedAdd((volatile int*)pVal, (int)iAdd);
#endif
}



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// CryInterlocked*SList Function, these are specialized C-A-S
// functions for single-linked lists which prevent the A-B-A problem there
// there are implemented in the platform specific CryThread_*.h files
// TODO clean up the interlocked function the same was the CryThread_* header are

//TODO somehow get their real size on WIN (without including windows.h...)
//NOTE: The sizes are verifyed at compile-time in the implementation functions, but this is still ugly
#if defined(WIN64) || defined(DURANGO) 
_MS_ALIGN(16)
#elif defined(WIN32)
_MS_ALIGN(8)
#endif
struct SLockFreeSingleLinkedListEntry
{
	SLockFreeSingleLinkedListEntry * volatile pNext;
}
#if defined(LINUX32)
_ALIGN(8)
#elif defined(APPLE) || defined(LINUX64)
_ALIGN(16)
#endif
;

#if defined(WIN64) || defined(DURANGO)
_MS_ALIGN(16)
#elif defined(WIN32)
_MS_ALIGN(8)
#endif
struct SLockFreeSingleLinkedListHeader
{
	SLockFreeSingleLinkedListEntry * volatile pNext;
#if /*defined(_CPU_ARM) ||*/ defined(_CPU_MIPS)
	// Only need "salt" on platforms using CAS
#elif defined(APPLE) || defined(LINUX) || defined(ORBIS)
	// If pointers 32bit, salt should be as well.  Otherwise we get 4 bytes of padding between pNext and salt and CAS operations fail
#if defined(PLATFORM_64BIT)
	volatile uint64 salt;
#else
	volatile uint32 salt;
#endif
#endif
}
#if defined(ORBIS)
__attribute__ ((aligned(16)))
#elif defined(LINUX32)
_ALIGN(8)
#elif defined(APPLE) || defined(LINUX64)
_ALIGN(16)
#endif
;


// push a element atomically onto a single linked list
void CryInterlockedPushEntrySList( SLockFreeSingleLinkedListHeader& list, SLockFreeSingleLinkedListEntry &element );

// push a element atomically from a single linked list
void* CryInterlockedPopEntrySList(  SLockFreeSingleLinkedListHeader& list );

// initialzied the lock-free single linked list
void CryInitializeSListHead(SLockFreeSingleLinkedListHeader& list);
  
// flush the whole list
void* CryInterlockedFlushSList(SLockFreeSingleLinkedListHeader& list);

ILINE void CryReadLock(volatile int *rw, bool yield)
{
	CryInterlockedAdd(rw,1);
#ifdef NEED_ENDIAN_SWAP
	volatile char *pw=(volatile char*)rw+1;
#else
	volatile char *pw=(volatile char*)rw+2;	
#endif

	uint64 loops = 0;
	for(;*pw;) 
	{
		if (yield)
		{
#	if !defined(ANDROID)
			_mm_pause();
#	endif

			if(!(++loops & 0x7F))
			{
				// give other threads with other prio right to run
#if defined (ORBIS)
				sceKernelUsleep(1);
#elif defined (LINUX)
				usleep(1);
#endif				
			}
			else if (!(loops & 0x3F))
			{
				// give threads with same prio chance to run
#if defined (ORBIS)
				scePthreadYield();
#elif defined (LINUX)
				sched_yield();
#endif				
			}		
		}
	}
}

ILINE void CryReleaseReadLock(volatile int* rw)
{
	CryInterlockedAdd(rw,-1);
}

ILINE void CryWriteLock(volatile int* rw)
{
	CrySpinLock(rw,0,WRITE_LOCK_VAL);
}

ILINE void CryReleaseWriteLock(volatile int* rw)
{
	CryInterlockedAdd(rw,-WRITE_LOCK_VAL);
}

//////////////////////////////////////////////////////////////////////////
struct ReadLock
{
	ILINE ReadLock(volatile int &rw)
	{
		CryInterlockedAdd(prw=&rw,1);
#ifdef NEED_ENDIAN_SWAP
		volatile char *pw=(volatile char*)&rw+1; for(;*pw;);
#else
		volatile char *pw=(volatile char*)&rw+2; for(;*pw;);
#endif
	}
	ILINE ReadLock(volatile int &rw, bool yield)
	{
		CryReadLock(prw=&rw, yield);
	}
	~ReadLock()
	{
		CryReleaseReadLock(prw);
	}
private:
	volatile int *prw;
};

struct ReadLockCond
{
	ILINE ReadLockCond(volatile int &rw,int bActive)
	{
		if (bActive)
		{
			CryInterlockedAdd(&rw,1);
			bActivated = 1;
#ifdef NEED_ENDIAN_SWAP
			volatile char *pw=(volatile char*)&rw+1; for(;*pw;);
#else
			volatile char *pw=(volatile char*)&rw+2; for(;*pw;);
#endif
		}
		else
		{
			bActivated = 0;
		}
		prw = &rw; 
	}
	void SetActive(int bActive=1) { bActivated = bActive; }
	void Release() { CryInterlockedAdd(prw,-bActivated); }
	~ReadLockCond()
	{
		CryInterlockedAdd(prw,-bActivated);
	}

private:
	volatile int *prw;
	int bActivated;
};

//////////////////////////////////////////////////////////////////////////
struct WriteLock
{
	ILINE WriteLock(volatile int &rw) { CryWriteLock(&rw); prw=&rw; }
	~WriteLock() { CryReleaseWriteLock(prw); }
private:
	volatile int *prw;
};

//////////////////////////////////////////////////////////////////////////
struct WriteAfterReadLock
{
	ILINE WriteAfterReadLock(volatile int &rw) { CrySpinLock(&rw,1,WRITE_LOCK_VAL+1); prw=&rw; }
	~WriteAfterReadLock() { CryInterlockedAdd(prw,-WRITE_LOCK_VAL); }
private:
	volatile int *prw;
};

//////////////////////////////////////////////////////////////////////////
struct WriteLockCond
{
	ILINE WriteLockCond(volatile int &rw,int bActive=1)
	{
		if (bActive)
			CrySpinLock(&rw,0,iActive=WRITE_LOCK_VAL);
		else 
			iActive = 0;
		prw = &rw; 
	}
	ILINE WriteLockCond() { prw=&(iActive=0); }
	~WriteLockCond() { 
		CryInterlockedAdd(prw,-iActive); 
	}
	void SetActive(int bActive=1) { iActive = -bActive & WRITE_LOCK_VAL; }
	void Release() { CryInterlockedAdd(prw,-iActive); }
	volatile int *prw;
	int iActive;
};


#if defined(LINUX) || defined(APPLE)
ILINE int64 CryInterlockedCompareExchange64( volatile int64 *addr, int64 exchange, int64 comperand )
{
    return __sync_val_compare_and_swap(addr,comperand,exchange);
    // This is OK, because long is signed int64 on Linux x86_64
	//return CryInterlockedCompareExchange((volatile long*)addr, (long)exchange, (long)comperand);
}

ILINE int64 CryInterlockedExchange64( volatile int64 *addr, int64 exchange)
{
	__sync_synchronize();
	return __sync_lock_test_and_set(addr, exchange);
}
#else
ILINE int64 CryInterlockedCompareExchange64( volatile int64 *addr, int64 exchange, int64 compare )
{
	// forward to system call
#if defined(ORBIS)
	return sceAtomicCompareAndSwap64((volatile int64_t*)addr, (int64_t)compare, (int64_t)exchange);
#else
	return _InterlockedCompareExchange64( (volatile int64*)addr, exchange,compare);
#endif
}
#endif

//////////////////////////////////////////////////////////////////////////
#if defined(EXCLUDE_PHYSICS_THREAD) 
	 ILINE void SpinLock(volatile int *pLock,int checkVal,int setVal) { *(int*)pLock=setVal; } 
	 ILINE void AtomicAdd(volatile int *pVal, int iAdd) {	*(int*)pVal+=iAdd; }
	 ILINE void AtomicAdd(volatile unsigned int *pVal, int iAdd) { *(unsigned int*)pVal+=iAdd; }

   ILINE void JobSpinLock(volatile int *pLock,int checkVal,int setVal) { CrySpinLock(pLock,checkVal,setVal); } 
#else
	ILINE void SpinLock(volatile int *pLock,int checkVal,int setVal) { CrySpinLock(pLock,checkVal,setVal); } 
	ILINE void AtomicAdd(volatile int *pVal, int iAdd) {	CryInterlockedAdd(pVal,iAdd); }
	ILINE void AtomicAdd(volatile unsigned int *pVal, int iAdd) { CryInterlockedAdd((volatile int*)pVal,iAdd); }

  ILINE void JobSpinLock(volatile int *pLock,int checkVal,int setVal) { SpinLock(pLock,checkVal,setVal); } 
#endif

ILINE void JobAtomicAdd(volatile int *pVal, int iAdd) {	CryInterlockedAdd(pVal,iAdd); }
ILINE void JobAtomicAdd(volatile unsigned int *pVal, int iAdd) { CryInterlockedAdd((volatile int*)pVal,iAdd); }


#endif // __MultiThread_h__

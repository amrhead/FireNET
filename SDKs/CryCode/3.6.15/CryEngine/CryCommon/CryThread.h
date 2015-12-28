/////////////////////////////////////////////////////////////////////////////
//
// Crytek Source File
// Copyright (C), Crytek Studios, 2001-2006.
//
// Description: Public include file for the multi-threading API.
//
// History:
// Jun 20, 2006: Created by Sascha Demetrio
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _CryThread_h_
#define _CryThread_h_

// Include basic multithread primitives.
#include "MultiThread.h"
#include "BitFiddling.h"
//////////////////////////////////////////////////////////////////////////
// Lock types:
//
// CRYLOCK_FAST
//   A fast potentially (non-recursive) mutex.
// CRYLOCK_RECURSIVE
//   A recursive mutex.
//////////////////////////////////////////////////////////////////////////
enum CryLockType
{
	CRYLOCK_FAST = 1,
	CRYLOCK_RECURSIVE = 2,
};

#define CRYLOCK_HAVE_FASTLOCK 1

void CryThreadSetName( threadID nThreadId,const char *sThreadName );
const char* CryThreadGetName( threadID nThreadId );

/////////////////////////////////////////////////////////////////////////////
//
// Primitive locks and conditions.
//
// Primitive locks are represented by instance of class CryLockT<Type> 
//
//
template<CryLockType Type> class CryLockT
{
	/* Unsupported lock type. */
};

//////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////
typedef CryLockT<CRYLOCK_RECURSIVE>   CryCriticalSection;
typedef CryLockT<CRYLOCK_FAST>        CryCriticalSectionNonRecursive;
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
// CryAutoCriticalSection implements a helper class to automatically
// lock critical section in constructor and release on destructor.
//
//////////////////////////////////////////////////////////////////////////
template<class LockClass> class CryAutoLock
{
private:
	LockClass *m_pLock;

	CryAutoLock();
	CryAutoLock(const CryAutoLock<LockClass>&);
	CryAutoLock<LockClass>& operator = (const CryAutoLock<LockClass>&);

public:
	CryAutoLock(LockClass &Lock) : m_pLock(&Lock) { m_pLock->Lock(); }
	CryAutoLock(const LockClass &Lock) : m_pLock(const_cast<LockClass*>(&Lock)) { m_pLock->Lock(); }
	~CryAutoLock() { m_pLock->Unlock(); }
};

//////////////////////////////////////////////////////////////////////////
//
// CryOptionalAutoLock implements a helper class to automatically
// lock critical section (if needed) in constructor and release on destructor.
//
//////////////////////////////////////////////////////////////////////////
template<class LockClass> class CryOptionalAutoLock
{
private:
	LockClass *m_Lock;
	bool m_bLockAcquired;

	CryOptionalAutoLock();
	CryOptionalAutoLock(const CryOptionalAutoLock<LockClass>&);
	CryOptionalAutoLock<LockClass>& operator = (const CryOptionalAutoLock<LockClass>&);

public:
	CryOptionalAutoLock(LockClass &Lock, bool acquireLock) : m_Lock(&Lock), m_bLockAcquired(false)
	{
		if (acquireLock)
		{
			Acquire();
		}
	}
	~CryOptionalAutoLock()
	{
		Release();
	}
	void Release()
	{
		if(m_bLockAcquired)
		{
			m_Lock->Unlock();
			m_bLockAcquired = false;
		}
	}
	void Acquire()
	{
		if(!m_bLockAcquired)
		{
			m_Lock->Lock();
			m_bLockAcquired = true;
		}
	}
};

//////////////////////////////////////////////////////////////////////////
//
// CryAutoSet implements a helper class to automatically
// set and reset value in constructor and release on destructor.
//
//////////////////////////////////////////////////////////////////////////
template<class ValueClass> class CryAutoSet
{
private:
	ValueClass* m_pValue;

	CryAutoSet();
	CryAutoSet(const CryAutoSet<ValueClass>&);
	CryAutoSet<ValueClass>& operator = (const CryAutoSet<ValueClass>&);

public:
	CryAutoSet(ValueClass &value) : m_pValue(&value) { *m_pValue = (ValueClass)1; }
	~CryAutoSet() { *m_pValue = (ValueClass)0; }
};

//////////////////////////////////////////////////////////////////////////
//
// Auto critical section is the most commonly used type of auto lock.
//
//////////////////////////////////////////////////////////////////////////
typedef CryAutoLock<CryCriticalSection> CryAutoCriticalSection;

#define AUTO_LOCK_T(Type, lock) PREFAST_SUPPRESS_WARNING(6246); CryAutoLock<Type> __AutoLock(lock)
#define AUTO_LOCK(lock)				AUTO_LOCK_T(CryCriticalSection, lock)
#define AUTO_LOCK_CS(csLock) CryAutoCriticalSection __AL__##csLock(csLock)

/////////////////////////////////////////////////////////////////////////////
//
// Threads.

// Base class for runnable objects.
//
// A runnable is an object with a Run() and a Cancel() method.  The Run()
// method should perform the runnable's job.  The Cancel() method may be
// called by another thread requesting early termination of the Run() method.
// The runnable may ignore the Cancel() call, the default implementation of
// Cancel() does nothing.
class CryRunnable
{
public:
	virtual ~CryRunnable() { }
	virtual void Run() = 0;
	virtual void Cancel() { }
};

// Class holding information about a thread.
//
// A reference to the thread information can be obtained by calling GetInfo()
// on the CrySimpleThread (or derived class) instance.
//
// NOTE:
// If the code is compiled with NO_THREADINFO defined, then the GetInfo()
// method will return a reference to a static dummy instance of this
// structure.  It is currently undecided if NO_THREADINFO will be defined for
// release builds!
//
// PS3 NOTE: Make sure the same NO_THREADINFO setting is used on SPU and PPU.
// On SPU the 'm_Name' field is replaced by a dummy field.
struct CryThreadInfo
{
#ifndef __SPU__
	// The symbolic name of the thread.
	//
	// You may set this name directly or through the SetName() method of
	// CrySimpleThread (or derived class).
	string m_Name;
#else
        // Placeholder (for data layout compatibility).
        char _m_Name[sizeof(string)]
          __attribute__ ((aligned (__alignof__ (string))));
#endif

	// A thread identification number.
	// The number is unique but architecture specific.  Do not assume anything
	// about that number except for being unique.
	//
	// This field is filled when the thread is started (i.e. before the Run()
	// method or thread routine is called).  It is advised that you do not
	// change this number manually.
	uint32 m_ID;
};

// Simple thread class.
//
// CrySimpleThread is a simple wrapper around a system thread providing
// nothing but system-level functionality of a thread.  There are two typical
// ways to use a simple thread:
//
// 1. Derive from the CrySimpleThread class and provide an implementation of
//    the Run() (and optionally Cancel()) methods.
// 2. Specify a runnable object when the thread is started.  The default
//    runnable type is CryRunnable.
//
// The Runnable class specfied as the template argument must provide Run()
// and Cancel() methods compatible with the following signatures:
//
//   void Runnable::Run();
//   void Runnable::Cancel();
//
// If the Runnable does not support cancellation, then the Cancel() method
// should do nothing.
//
// The same instance of CrySimpleThread may be used for multiple thread
// executions /in sequence/, i.e. it is valid to re-start the thread by
// calling Start() after the thread has been joined by calling WaitForThread().
template<class Runnable = CryRunnable> class CrySimpleThread;

// Standard thread class.
//
// The class provides a lock (mutex) and an associated condition variable.  If
// you don't need the lock, then you should used CrySimpleThread instead of
// CryThread.
template<class Runnable = CryRunnable> class CryThread;

///////////////////////////////////////////////////////////////////////////////
// Include architecture specific code.
#if defined(PS3)
#include <CryThread_ps3.h>
#elif defined(LINUX) || defined(ORBIS)
#include <CryThread_pthreads.h>
#elif defined(APPLE)
#include <CryThread_pthreads.h>
#elif defined(WIN32) || defined(WIN64)

#include <CryThread_windows.h>

#elif defined(XENON) || defined(DURANGO)
// Note: After verifying one of the new CryThread implementations for
// Win32/Win64, the XENON should switch to the same implementation.
#include <CryThread_windows.h>
#else
// Put other platform specific includes here!
#include <CryThread_dummy.h>
#endif

#if !defined _CRYTHREAD_CONDLOCK_GLITCH
typedef CryLockT<CRYLOCK_RECURSIVE> CryMutex;
#endif // !_CRYTHREAD_CONDLOCK_GLITCH

// The the architecture specific code does not define a class CryRWLock, then
// a default implementation is provided here.
#if !defined _CRYTHREAD_HAVE_RWLOCK && !defined _CRYTHREAD_CONDLOCK_GLITCH
class CryRWLock
{

	CryCriticalSection m_lockExclusiveAccess;
	CryCriticalSection m_lockSharedAccessComplete;
	CryConditionVariable m_condSharedAccessComplete;

	int m_nSharedAccessCount;
	int m_nCompletedSharedAccessCount;
	bool m_bExclusiveAccess;

	CryRWLock(const CryRWLock &);
	CryRWLock &operator= (const CryRWLock &);

	void AdjustSharedAccessCount()
	{
		m_nSharedAccessCount -= m_nCompletedSharedAccessCount;
		m_nCompletedSharedAccessCount = 0;
	}

public:
	CryRWLock()
		: m_nSharedAccessCount(0),
			m_nCompletedSharedAccessCount(0),
			m_bExclusiveAccess(false)
	{ }

	void RLock()
	{
		m_lockExclusiveAccess.Lock();
		if (++m_nSharedAccessCount == INT_MAX)
		{
			m_lockSharedAccessComplete.Lock();
			AdjustSharedAccessCount();
			m_lockSharedAccessComplete.Unlock();
		}
		m_lockExclusiveAccess.Unlock();
	}

	bool TryRLock()
	{
		if (!m_lockExclusiveAccess.TryLock())
			return false;
		if (++m_nSharedAccessCount == INT_MAX)
		{
			m_lockSharedAccessComplete.Lock();
			AdjustSharedAccessCount();
			m_lockSharedAccessComplete.Unlock();
		}
		m_lockExclusiveAccess.Unlock();
		return true;
	}

	void RUnlock()
	{
		Unlock();
	}

	void WLock()
	{
		m_lockExclusiveAccess.Lock();
		m_lockSharedAccessComplete.Lock();
		assert(!m_bExclusiveAccess);
		AdjustSharedAccessCount();
		if (m_nSharedAccessCount > 0)
		{
			m_nCompletedSharedAccessCount -= m_nSharedAccessCount;
			do
			{
				m_condSharedAccessComplete.Wait(m_lockSharedAccessComplete);
			}
			while (m_nCompletedSharedAccessCount < 0);
			m_nSharedAccessCount = 0;
		}
		m_bExclusiveAccess = true;
	}

	bool TryWLock()
	{
		if (!m_lockExclusiveAccess.TryLock())
			return false;
		if (!m_lockSharedAccessComplete.TryLock())
		{
			m_lockExclusiveAccess.Unlock();
			return false;
		}
		assert(!m_bExclusiveAccess);
		AdjustSharedAccessCount();
		if (m_nSharedAccessCount > 0)
		{
			m_lockSharedAccessComplete.Unlock();
			m_lockExclusiveAccess.Unlock();
			return false;
		}
		else
			m_bExclusiveAccess = true;
		return true;
	}

	void WUnlock()
	{
		Unlock();
	}

	void Unlock()
	{
		if (!m_bExclusiveAccess)
		{
			m_lockSharedAccessComplete.Lock();
			if (++m_nCompletedSharedAccessCount == 0)
				m_condSharedAccessComplete.NotifySingle();
			m_lockSharedAccessComplete.Unlock();
		}
		else
		{
			m_bExclusiveAccess = false;
			m_lockSharedAccessComplete.Unlock();
			m_lockExclusiveAccess.Unlock();
		}
	}
};
#endif // !defined _CRYTHREAD_HAVE_RWLOCK

// Thread class.
//
// CryThread is an extension of CrySimpleThread providing a lock (mutex) and a
// condition variable per instance.
template<class Runnable> class CryThread
	: public CrySimpleThread<Runnable>
{
	CryMutex m_Lock;
	CryConditionVariable m_Cond;

	CryThread(const CryThread<Runnable>&);
	void operator = (const CryThread<Runnable>&);

public:
	CryThread() { }
	void Lock() { m_Lock.Lock(); }
	bool TryLock() { return m_Lock.TryLock(); }
	void Unlock() { m_Lock.Unlock(); }
	void Wait() { m_Cond.Wait(m_Lock); }
	// Timed wait on the associated condition.
	//
	// The 'milliseconds' parameter specifies the relative timeout in
	// milliseconds.  The method returns true if a notification was received and
	// false if the specified timeout expired without receiving a notification.
	//
	// UNIX note: the method will _not_ return if the calling thread receives a
	// signal.  Instead the call is re-started with the _original_ timeout
	// value.  This misfeature may be fixed in the future.
	bool TimedWait(uint32 milliseconds)
	{
		return m_Cond.TimedWait(m_Lock, milliseconds);
	}
	void Notify() { m_Cond.Notify(); }
	void NotifySingle() { m_Cond.NotifySingle(); }
	CryMutex &GetLock() { return m_Lock; }
};

//////////////////////////////////////////////////////////////////////////
//
// Sync primitive for multiple reads and exclusive locking change access
// 
//	Desc:
//		Useful in case if you have rarely modified object that needs 
//		to be read quite often from different threads but still 
//		need to be exclusively modified sometimes
//	Debug functionality:
//		Can be used for debug-only lock calls, which verify that no
//		simultaneous access is attempted. 
//		Use the bDebug argument of LockRead or LockModify,
//		or use the DEBUG_READLOCK or DEBUG_MODIFYLOCK macros.
//		There is no overhead in release builds, if you use the macros,
//		and the lock definition is inside #ifdef _DEBUG.
//////////////////////////////////////////////////////////////////////////

class CryReadModifyLock
{
public:
	CryReadModifyLock() 
		: m_modifyCount(0), m_readCount(0)
	{
		SetDebugLocked(false);
	}

	bool LockRead(bool bTry = false, cstr strDebug = 0, bool bDebug = false) const
	{
		if (!WriteLock(bTry, bDebug, strDebug))			// wait until write unlocked
			return false;
		CryInterlockedIncrement(&m_readCount);			// increment read counter
		m_writeLock.Unlock();
		return true;
	}
	void UnlockRead() const
	{
		SetDebugLocked(false);
		const int counter = CryInterlockedDecrement(&m_readCount);		 // release read
		assert(counter >= 0);
		if (m_writeLock.TryLock())
			m_writeLock.Unlock();
		else
			if (counter == 0 && m_modifyCount)
				m_ReadReleased.Set();										// signal the final read released
	}
	bool LockModify(bool bTry = false, cstr strDebug = 0, bool bDebug = false) const
	{
		if (!WriteLock(bTry, bDebug, strDebug))
			return false;
		CryInterlockedIncrement(&m_modifyCount);		// increment write counter (counter is for nested cases)
		while (m_readCount) 
			m_ReadReleased.Wait();										// wait for all threads finish read operation
		return true;
	}
	void UnlockModify() const
	{
		SetDebugLocked(false);
		int counter = CryInterlockedDecrement(&m_modifyCount);		// decrement write counter
		assert(counter >= 0);
		m_writeLock.Unlock();												// release exclusive lock
	}

protected:
	mutable volatile int          m_readCount;
	mutable volatile int          m_modifyCount;
	mutable CryEvent              m_ReadReleased;
	mutable CryCriticalSection    m_writeLock;
	mutable bool									m_debugLocked;
	mutable const char*						m_debugLockStr;

	void SetDebugLocked(bool b, const char* str = 0) const
	{
#ifdef _DEBUG
		m_debugLocked = b;
		m_debugLockStr = str;
#endif
	}

	bool WriteLock(bool bTry, bool bDebug, const char* strDebug) const
	{
		if (!m_writeLock.TryLock())
		{
#ifdef _DEBUG
			assert(!m_debugLocked);
			assert(!bDebug);
#endif
			if (bTry)
				return false;
			m_writeLock.Lock();
		}
#ifdef _DEBUG
		if (!m_readCount && !m_modifyCount)					// not yet locked
			SetDebugLocked(bDebug, strDebug);
#endif
		return true;
	}
};

// Auto-locking classes.
template<class T, bool bDEBUG = false>
class AutoLockRead
{
protected:
	const T& m_lock;
public:
	AutoLockRead(const T& lock, cstr strDebug = 0) 
		: m_lock(lock) { m_lock.LockRead(bDEBUG, strDebug, bDEBUG); }
	~AutoLockRead()	
		{ m_lock.UnlockRead(); }
};

template<class T, bool bDEBUG = false>
class AutoLockModify
{
protected:
	const T& m_lock;
public:
	AutoLockModify(const T& lock, cstr strDebug = 0) 
		: m_lock(lock) { m_lock.LockModify(bDEBUG, strDebug, bDEBUG); }
	~AutoLockModify()	
		{ m_lock.UnlockModify(); }
};

// On PS3, __FUNC__ isn't a char*, it provides some code to mimic
// the behaivor of __FUNC__ of other plattforms, but it wasn't threadsafe
// so we use __PRETTY_FUNCTION__ for PS3, also we don't want the runtime overhead
// of the macro emulation
// AUTO_READLOCK_PROT is special as it maps to a modify lock for PS3 always	
#if defined(PS3)
	#define AUTO_READLOCK(p) AutoLockRead<CryReadModifyLock> __readlock##__LINE__(p, __PRETTY_FUNCTION__)
	#define AUTO_READLOCK_PROT(p) AutoLockModify<CryReadModifyLock> __readlock_prot##__LINE__(p, __PRETTY_FUNCTION__)
	#define AUTO_MODIFYLOCK(p) AutoLockModify<CryReadModifyLock> __modifylock##__LINE__(p, __PRETTY_FUNCTION__)
#else
	#define AUTO_READLOCK(p) PREFAST_SUPPRESS_WARNING(6246) AutoLockRead<CryReadModifyLock> __readlock##__LINE__(p, __FUNC__)
	#define AUTO_READLOCK_PROT(p) PREFAST_SUPPRESS_WARNING(6246) AutoLockRead<CryReadModifyLock> __readlock_prot##__LINE__(p, __FUNC__)
	#define AUTO_MODIFYLOCK(p) PREFAST_SUPPRESS_WARNING(6246) AutoLockModify<CryReadModifyLock> __modifylock##__LINE__(p, __FUNC__)
#endif

#if defined(_DEBUG) && !defined(PS3) // Disabled for PS3 as it's debug only and has high overhead.
	#define DEBUG_READLOCK(p) AutoLockRead<CryReadModifyLock> __readlock##__LINE__(p, __FUNC__)
	#define DEBUG_MODIFYLOCK(p) AutoLockModify<CryReadModifyLock> __modifylock##__LINE__(p, __FUNC__)
#else
	#define DEBUG_READLOCK(p)
	#define DEBUG_MODIFYLOCK(p)
#endif

// producer consumer queue implementations, but here instead of MultiThread_Container.h
// since they requiere platform specific code, and including windows.h in a very common
// header file leads to all kinds of problems
namespace CryMT
{
	//////////////////////////////////////////////////////////////////////////
	// Producer/Consumer Queue for 1 to 1 thread communication
	// Realized with only volatile variables and memory barriers
	// *warning* this producer/consumer queue is only thread safe in a 1 to 1 situation
	// and doesn't provide any yields or similar to prevent spinning
	//////////////////////////////////////////////////////////////////////////
	template<typename T>
	class SingleProducerSingleConsumerQueue : public CryMT::detail::SingleProducerSingleConsumerQueueBase
	{
	public:
		SingleProducerSingleConsumerQueue();			
		~SingleProducerSingleConsumerQueue();
		
		void Init( size_t nSize );		

		void Push( const T &rObj );
		void Pop( T *pResult );

	private:
		T *m_arrBuffer;
		uint32 m_nBufferSize;
	
		volatile uint32 m_nProducerIndex _ALIGN(16);
		volatile uint32 m_nComsumerIndex _ALIGN(16);
	} _ALIGN(128);

	///////////////////////////////////////////////////////////////////////////////
	template<typename T>
	inline SingleProducerSingleConsumerQueue<T>::SingleProducerSingleConsumerQueue() :
			m_arrBuffer(NULL), m_nBufferSize(0), m_nProducerIndex(0), m_nComsumerIndex(0) 
	{}
	
	///////////////////////////////////////////////////////////////////////////////
	template<typename T>
	inline SingleProducerSingleConsumerQueue<T>::~SingleProducerSingleConsumerQueue()
	{
		CryModuleMemalignFree(m_arrBuffer);			
		m_nBufferSize = 0;
	}

	///////////////////////////////////////////////////////////////////////////////
	template<typename T>
	inline void SingleProducerSingleConsumerQueue<T>::Init( size_t nSize )
	{
		assert(m_arrBuffer == NULL);			
		assert(m_nBufferSize == 0);
		assert((nSize&(nSize-1)) == 0);

		m_arrBuffer = alias_cast<T*>( CryModuleMemalign( nSize * sizeof(T), 16 ) );			
		m_nBufferSize = nSize;				
	}

	///////////////////////////////////////////////////////////////////////////////
	template<typename T>
	inline void SingleProducerSingleConsumerQueue<T>::Push( const T &rObj )
	{
		assert(m_arrBuffer != NULL);
		assert(m_nBufferSize != 0);
#if defined(__SPU__)
		uint64 startTime = rdtsc();
		uint64 endTime = startTime - 10 * 79800; // 10ms
		char queueBuffer[sizeof(SingleProducerSingleConsumerQueue)] _ALIGN(128);
		SingleProducerSingleConsumerQueue *pSPUQueue = alias_cast<SingleProducerSingleConsumerQueue*>(queueBuffer);
		do
		{
			__spu_mfc_load_cacheline(pSPUQueue, this);

			// busy-loop if queue is full			
			if(pSPUQueue->m_nProducerIndex - pSPUQueue->m_nComsumerIndex == pSPUQueue->m_nBufferSize)			
				continue;

			// got a element
			break;

		}while(true);

		T *pTargetAddress = &pSPUQueue->m_arrBuffer[pSPUQueue->m_nProducerIndex % pSPUQueue->m_nBufferSize];

		// fast path if object aligned is 16 or larger		
		if( alignof(T) >= 16 )
		{
			memtransfer_to_main(pTargetAddress,&rObj,sizeof(T),0);
			memtransfer_sync(0);
		}
		else
		{
			char objectBuffer[sizeof(T) + 128] _ALIGN(128);
			uint32 eaTarget = alias_cast<uint32>(pTargetAddress);
			uint32 eaTargetAligned = eaTarget & ~127;
			uint32 eaTargetOffset = eaTarget & 127;
			// copy to SPU local temp buffer to ensure same offset as in main memory
			memcpy( SPU_LOCAL_PTR(&objectBuffer[eaTargetOffset]), SPU_LOCAL_PTR(&rObj), sizeof(T) );

			memtransfer_to_main(pTargetAddress,&objectBuffer[eaTargetOffset],sizeof(T),0);
			memtransfer_sync(0);
		}

		// publish updated
		uint32 nUpdatedProducerIndex _ALIGN(16) = pSPUQueue->m_nProducerIndex + 1;
		memtransfer_to_main(&m_nProducerIndex, &nUpdatedProducerIndex, 16, 0 );
		memtransfer_sync(0);
#else
		SingleProducerSingleConsumerQueueBase::Push((void*)&rObj, m_nProducerIndex, m_nComsumerIndex, m_nBufferSize, m_arrBuffer, sizeof(T));
#endif
	}

	///////////////////////////////////////////////////////////////////////////////
	template<typename T>
	inline void SingleProducerSingleConsumerQueue<T>::Pop( T *pResult )
	{
		assert(m_arrBuffer != NULL);
		assert(m_nBufferSize != 0);
#if defined(__SPU__)
		char queueBuffer[sizeof(SingleProducerSingleConsumerQueue)] _ALIGN(128);
		SingleProducerSingleConsumerQueue *pSPUQueue = alias_cast<SingleProducerSingleConsumerQueue*>(queueBuffer);

		do
		{
			__spu_mfc_load_cacheline(pSPUQueue, this);

			// busy-loop if queue is empty			
			if(pSPUQueue->m_nProducerIndex - pSPUQueue->m_nComsumerIndex == 0)
				continue;

			// got a element
			break;

		}while(true);

		T *pSourceAddress = &pSPUQueue->m_arrBuffer[pSPUQueue->m_nComsumerIndex % pSPUQueue->m_nBufferSize];

		// fast path if object aligenment is 16 or larger
		if( alignof(T) >= 16 )
		{
			memtransfer_from_main(pResult,pSourceAddress,sizeof(T),0);
			memtransfer_sync(0);
		}
		else
		{
			char objectBuffer[sizeof(T) + 128] _ALIGN(128);
			uint32 eaSource = alias_cast<uint32>(pSourceAddress);
			uint32 eaSourceAligned = eaSource & ~127;
			uint32 eaSourceOffset = eaSource & 127;
			
			memtransfer_from_main(&objectBuffer[eaSourceOffset],pSourceAddress,sizeof(T),0);
			memtransfer_sync(0);

			// copy from SPU local temp buffer to ensure same offset as in main memory
			memcpy( SPU_LOCAL_PTR(pResult), SPU_LOCAL_PTR(&objectBuffer[eaSourceOffset]), sizeof(T) );
		}

		// publish updated consumer pointer
		uint32 nUpdatedConsumerIndex _ALIGN(16) = pSPUQueue->m_nComsumerIndex + 1;
		memtransfer_to_main(&m_nComsumerIndex, &nUpdatedConsumerIndex, 16, 0 );
		memtransfer_sync(0);
#else
		SingleProducerSingleConsumerQueueBase::Pop(pResult, m_nProducerIndex, m_nComsumerIndex, m_nBufferSize, m_arrBuffer,sizeof(T));
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// Producer/Consumer Queue for N to 1 thread communication
	// lockfree implemenation, to copy with multiple producers, 
	// a internal producer refcount is managed, the queue is empty
	// as soon as there are no more producers and no new elements
	//////////////////////////////////////////////////////////////////////////
	template<typename T>
	class N_ProducerSingleConsumerQueue : public CryMT::detail::N_ProducerSingleConsumerQueueBase
	{
	public:
		N_ProducerSingleConsumerQueue();
		~N_ProducerSingleConsumerQueue();

		void Init( size_t nSize );

		void Push( const T &rObj );
		bool Pop( T *pResult );
		
		// needs to be called before using, assumes that there is at least one producer
		// so the first one doesn't need to call AddProducer, but he has to deregister itself
		void SetRunningState();

		// to correctly track when the queue is empty(and no new jobs will be added), refcount the producer
		void AddProducer();
		void RemoveProducer();

	private:
		T*								m_arrBuffer;
		volatile uint32*	m_arrStates;
		uint32						m_nBufferSize;
	
		volatile uint32 m_nProducerIndex;
		volatile uint32 m_nComsumerIndex;
		volatile uint32 m_nRunning;
		volatile uint32 m_nProducerCount;
	} _ALIGN(128);

	///////////////////////////////////////////////////////////////////////////////
	template<typename T>
	inline N_ProducerSingleConsumerQueue<T>::N_ProducerSingleConsumerQueue() :
		m_arrBuffer(NULL), m_arrStates(NULL), m_nBufferSize(0), m_nProducerIndex(0), m_nComsumerIndex(0), m_nRunning(0), m_nProducerCount(0) 
	{}

	///////////////////////////////////////////////////////////////////////////////
	template<typename T>
	inline N_ProducerSingleConsumerQueue<T>::~N_ProducerSingleConsumerQueue()
	{
		CryModuleMemalignFree(m_arrBuffer);
		CryModuleMemalignFree( (void*)m_arrStates);
		m_nBufferSize = 0;
	}
		
	///////////////////////////////////////////////////////////////////////////////
	template<typename T>
	inline void N_ProducerSingleConsumerQueue<T>::Init( size_t nSize )
	{
		assert(m_arrBuffer == NULL);
		assert(m_arrStates == NULL);
		assert(m_nBufferSize == 0);
		assert((nSize&(nSize-1)) == 0);

		m_arrBuffer = alias_cast<T*>( CryModuleMemalign( nSize * sizeof(T), 16 ) );
		m_arrStates = alias_cast<uint32*>( CryModuleMemalign( nSize * sizeof(uint32), 16 ) );
		memset((void*)m_arrStates, 0, sizeof(uint32)*nSize);
		m_nBufferSize = nSize;			
	}

	///////////////////////////////////////////////////////////////////////////////
	template<typename T>
	inline void N_ProducerSingleConsumerQueue<T>::SetRunningState()
	{		
#if !defined(_RELEASE)
		if(m_nRunning == 1)
			__debugbreak();
#endif
		m_nRunning = 1;
		m_nProducerCount = 1;
	}

	///////////////////////////////////////////////////////////////////////////////
	template<typename T>
	inline void N_ProducerSingleConsumerQueue<T>::AddProducer()
	{
		assert( m_arrBuffer != NULL );
		assert( m_arrStates != NULL );
		assert( m_nBufferSize != 0 );

#if defined(__SPU__)
		char queueBuffer[sizeof(N_ProducerSingleConsumerQueue)] _ALIGN(128);
		N_ProducerSingleConsumerQueue *pSPUQueue = alias_cast<N_ProducerSingleConsumerQueue*>(queueBuffer);
		do
		{
			__spu_mfc_load_cacheline(pSPUQueue, this);
			pSPUQueue->m_nProducerCount += 1;
			
			if( __spu_mfc_store_cacheline() )			
				break;
		}while(1);
#else
#if !defined(_RELEASE)
		if(m_nRunning == 0)
			__debugbreak();
#endif
		CryInterlockedIncrement((volatile int*)&m_nProducerCount);
#endif		
	}
	
	///////////////////////////////////////////////////////////////////////////////
	template<typename T>
	inline void N_ProducerSingleConsumerQueue<T>::RemoveProducer( )
	{
		assert( m_arrBuffer != NULL );
		assert( m_arrStates != NULL );
		assert( m_nBufferSize != 0 );

#if defined(__SPU__)
		char queueBuffer[sizeof(N_ProducerSingleConsumerQueue)] _ALIGN(128);
		N_ProducerSingleConsumerQueue *pSPUQueue = alias_cast<N_ProducerSingleConsumerQueue*>(queueBuffer);

		do
		{
			__spu_mfc_load_cacheline(pSPUQueue, this);
			pSPUQueue->m_nProducerCount -= 1;
			
			if(pSPUQueue->m_nProducerCount == 0)
				pSPUQueue->m_nRunning = 0;
				
			if( __spu_mfc_store_cacheline() )			
				break;
		}while(1);
#else	
#if !defined(_RELEASE)
		if(m_nRunning == 0)
			__debugbreak();
#endif
		if( CryInterlockedDecrement((volatile int*)&m_nProducerCount) == 0 )
			m_nRunning = 0;
#endif			
	}	

	///////////////////////////////////////////////////////////////////////////////
	template<typename T>
	inline void N_ProducerSingleConsumerQueue<T>::Push( const T &rObj )
	{
		assert( m_arrBuffer != NULL );
		assert( m_arrStates != NULL );
		assert( m_nBufferSize != 0 );
#if !defined(_RELEASE)
		if(m_nRunning == 0)
			__debugbreak();
#endif
#if defined(__SPU__)		
		char queueBuffer[sizeof(N_ProducerSingleConsumerQueue)] _ALIGN(128);
		N_ProducerSingleConsumerQueue *pSPUQueue = alias_cast<N_ProducerSingleConsumerQueue*>(queueBuffer);

		uint64 startTime = rdtsc();
		uint64 endTime = startTime - 10 * 79800; // 10ms
		uint32 nProducerIndex = 0;
		do
		{			
			__spu_mfc_load_cacheline(pSPUQueue, this);

			// busy-loop if queue is full			
			if(pSPUQueue->m_nProducerIndex - pSPUQueue->m_nComsumerIndex == pSPUQueue->m_nBufferSize)
			{
				if( rdtsc() < endTime )
				{
					uint32 nSizeToAlloc = sizeof(SFallbackList) + sizeof(T) - 1;
					SFallbackList *pFallbackEntry = (SFallbackList *)CryModuleMemalign(nSizeToAlloc,128);
					memcpy(pFallbackEntry->object, &rObj, sizeof(T));
					__spu_flush_cache_range( ((char*)pFallbackEntry) + 128, sizeof(T));
					CryInterlockedPushEntrySList( fallbackList,  pFallbackEntry->nextEntry );
					return;
				}
				continue;
			}

			nProducerIndex = pSPUQueue->m_nProducerIndex;
			pSPUQueue->m_nProducerIndex += 1;
			
			if( __spu_mfc_store_cacheline() )			
				break;

		}while(true);

		T *pTargetAddress = &pSPUQueue->m_arrBuffer[nProducerIndex % pSPUQueue->m_nBufferSize];

		// fast path if object aligned is 16 or larger		
		if( alignof(T) >= 16 )
		{
			memtransfer_to_main(pTargetAddress,&rObj,sizeof(T),0);
			memtransfer_sync(0);
		}
		else
		{
			char objectBuffer[sizeof(T) + 128] _ALIGN(128);
			uint32 eaTarget = alias_cast<uint32>(pTargetAddress);
			uint32 eaTargetAligned = eaTarget & ~127;
			uint32 eaTargetOffset = eaTarget & 127;
			// copy to SPU local temp buffer to ensure same offset as in main memory
			memcpy( SPU_LOCAL_PTR(&objectBuffer[eaTargetOffset]), SPU_LOCAL_PTR(&rObj), sizeof(T) );
			memtransfer_to_main(pTargetAddress,&objectBuffer[eaTargetOffset],sizeof(T),0);
			memtransfer_sync(0);
		}

		// publish state change 
		char buffer[128] _ALIGN(128);
		uint32 eaState = (uint32)&pSPUQueue->m_arrStates[nProducerIndex % pSPUQueue->m_nBufferSize];
		uint32 eaStateAligned = eaState & ~127;
		uint32 eaStateOffset = eaState & 127;
		uint32 *pState = (uint32*)&buffer[eaStateOffset];
		*pState = 1;
		memtransfer_to_main( SPU_MAIN_PTR((void*)eaState), &buffer[eaStateOffset], 4, 0 );
		memtransfer_sync(0);
#else
		CryMT::detail::N_ProducerSingleConsumerQueueBase::Push((void*)&rObj, m_nProducerIndex, m_nComsumerIndex, m_nRunning, m_arrBuffer, m_nBufferSize, sizeof(T), m_arrStates);
#endif
	}

	///////////////////////////////////////////////////////////////////////////////
	template<typename T>
	inline bool N_ProducerSingleConsumerQueue<T>::Pop( T *pResult )
	{
		assert( m_arrBuffer != NULL );
		assert( m_arrStates != NULL );
		assert( m_nBufferSize != 0 );
#if defined(__SPU__)
		snPause(); //no supported yet
		return false;
#else
		return CryMT::detail::N_ProducerSingleConsumerQueueBase::Pop(pResult, m_nProducerIndex, m_nComsumerIndex, m_nRunning, m_arrBuffer, m_nBufferSize, sizeof(T), m_arrStates);
#endif
	}

} //namespace CryMT

// Include all multithreading containers.
#include "MultiThread_Containers.h"
#ifdef XENON
#	include <Xenon_MT.h>
#endif

#endif

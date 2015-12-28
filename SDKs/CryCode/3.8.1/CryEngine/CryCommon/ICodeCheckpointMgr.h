////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2010.
// -------------------------------------------------------------------------
//  Created:     08/04/2010 by Will W (based on work by Matthew J)
//  Description: Interface to track code checkpoint registration
// -------------------------------------------------------------------------
//  History: Created by Will Wilson based on work by Matthew Jack.
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ICODE_CHECKPOINT_MGR_H_
#define __ICODE_CHECKPOINT_MGR_H_

#pragma once

// Forward declarations
class CCodeCheckpoint;

// Not for release
#ifndef _RELEASE

	#define CODECHECKPOINT_ENABLED

#endif

/// Interface for a global manager of code coverage checkpoints. Must be thread safe.
struct ICodeCheckpointMgr
{
	// <interfuscator:shuffle>
	virtual ~ICodeCheckpointMgr(){}
	/// Used by code checkpoints to register themselves with the manager.
	virtual void RegisterCheckpoint(CCodeCheckpoint* pCheckpoint) = 0;

	/// Performs a (possibly) expensive lookup by name for a given checkpoint index. Returns index to blank record if not found.
	virtual size_t GetCheckpointIndex(const char* name) = 0;

	/// Performs a cheap lookup by direct index, will return NULL if checkpoint has not yet been registered.
	virtual const CCodeCheckpoint* GetCheckpoint(size_t checkpointIdx) const = 0;

	/// Performs cheap lookup for checkpoint name, will always return a valid name for a valid input index.
	virtual const char* GetCheckPointName(size_t checkpointIdx) const = 0;

	/// Returns the total number of checkpoints
	virtual size_t GetTotalCount() const = 0;

	/// Returns the total number of checkpoints that have been hit at least once.
	virtual size_t GetTotalEncountered() const = 0;

	/// Frees this instance from memory
	virtual void Release() = 0;
	// </interfuscator:shuffle>
};

/// Inline helper class used for registration and update of code checkpoint state.
class CCodeCheckpoint
{
public:
	CCodeCheckpoint(const char* name)
		: m_hitCount(),
			m_name(name)
	{
		ICodeCheckpointMgr* pMgr = gEnv->pCodeCheckpointMgr;

		if (pMgr)
			pMgr->RegisterCheckpoint(this);
	}

	ILINE void Hit()	{ ++m_hitCount; }
	void Reset()			{ m_hitCount = 0; }

	uint32 HitCount() const		{ return m_hitCount; }
	const char* Name() const	{ return m_name; }

private:
	uint32			m_hitCount;		/// How many times has this been hit (uint32 gives 49 days @ 1000 hits/second)
	const char* m_name;				/// Name for the checkpoint. Should be descriptive and unique but not function name.
};

/**
* The checkpoint macro, resolves to nothing on release builds.
* Best usage will include redefining the macro to prefix the name with the module.
* Do not use method names automatically - if code is moved, renamed, copy/pasted, 
* it's more useful to preserve the label.
*/
#ifdef CODECHECKPOINT_ENABLED

	#define CODECHECKPOINT(x) \
		do \
		{ \
			static CCodeCheckpoint s_checkpoint##x(#x); \
			s_checkpoint##x.Hit(); \
		} \
		while (false)

#else

	#define CODECHECKPOINT(x) ((void)(0))

#endif	// !defined CODECHECKPOINT_ENABLED

#endif // __ICODE_CHECKPOINT_MGR_H_

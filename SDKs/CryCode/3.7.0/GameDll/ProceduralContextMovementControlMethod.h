// (c) 2001 - 2012 Crytek GmbH.
#ifndef __PROCEDURAL_CONTEXT_MOVEMENT_CONTROL_METHOD__H__
#define __PROCEDURAL_CONTEXT_MOVEMENT_CONTROL_METHOD__H__

#include <ICryMannequin.h>

#include <CryExtension/CryCreateClassInstance.h>
#include <CryExtension/Impl/ClassWeaver.h>
#include <CryExtension/Impl/ICryFactoryRegistryImpl.h>
#include <CryExtension/Impl/RegFactoryNode.h>

#include <IAnimatedCharacter.h>

#include "ProceduralContextHelpers.h"


#define PROCEDURAL_CONTEXT_MOVEMENT_CONTROL_METHOD_NAME "ProceduralContextMCM"


class CProceduralContextMovementControlMethod
	: public IProceduralContext
{
public:
	PROCEDURAL_CONTEXT( CProceduralContextMovementControlMethod, PROCEDURAL_CONTEXT_MOVEMENT_CONTROL_METHOD_NAME, 0x80140507bdc64be4, 0xa24190f322270e82 );

	// IProceduralContext
	virtual void Update( float timePassedSeconds );
	// ~IProceduralContext

	uint32 RequestMovementControlMethod( const EMovementControlMethod horizontal, const EMovementControlMethod vertical );
	void CancelRequest( const uint32 requestId );

private:
	IAnimatedCharacter* GetAnimatedCharacter() const;

private:
	struct SMCMRequest
	{
		uint32 id;
		EMovementControlMethod horizontal;
		EMovementControlMethod vertical;
	};

	typedef ProceduralContextHelpers::CRequestList< SMCMRequest > TMCMRequestList;
	TMCMRequestList m_requestList;
};


#endif

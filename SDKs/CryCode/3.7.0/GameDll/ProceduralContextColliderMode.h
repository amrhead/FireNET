// (c) 2001 - 2012 Crytek GmbH.
#ifndef __PROCEDURAL_CONTEXT_COLLIDER_MODE__H__
#define __PROCEDURAL_CONTEXT_COLLIDER_MODE__H__

#include <ICryMannequin.h>

#include <CryExtension/CryCreateClassInstance.h>
#include <CryExtension/Impl/ClassWeaver.h>
#include <CryExtension/Impl/ICryFactoryRegistryImpl.h>
#include <CryExtension/Impl/RegFactoryNode.h>

#include <IAnimatedCharacter.h>

#include "ProceduralContextHelpers.h"


#define PROCEDURAL_CONTEXT_COLLIDER_MODE_NAME "ProceduralContextColliderMode"


class CProceduralContextColliderMode
	: public IProceduralContext
{
public:
	PROCEDURAL_CONTEXT( CProceduralContextColliderMode, PROCEDURAL_CONTEXT_COLLIDER_MODE_NAME, 0x2857e483964b45e4, 0x8e9e6a481db8c166 );

	// IProceduralContext
	virtual void Update( float timePassedSeconds );
	// ~IProceduralContext

	uint32 RequestColliderMode( const EColliderMode colliderMode );
	void CancelRequest( const uint32 requestId );

private:
	IAnimatedCharacter* GetAnimatedCharacter() const;

private:
	struct SColliderModeRequest
	{
		uint32 id;
		EColliderMode mode;
	};

	typedef ProceduralContextHelpers::CRequestList< SColliderModeRequest > TColliderModeRequestList;
	TColliderModeRequestList m_requestList;
};


#endif

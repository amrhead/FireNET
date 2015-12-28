/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2006-2008.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: Central manager for synchronized animations on multiple actors 

-------------------------------------------------------------------------
History:
- August 5th, 2008: Created by Michelle Martin
- February 5th, 2009: Moved to CryAction by David Ramos
*************************************************************************/
#ifndef __COOPERATIVEANIMATIONMANAGER_H__
#define __COOPERATIVEANIMATIONMANAGER_H__

#include <ICooperativeAnimationManager.h>
#include "CooperativeAnimation.h"

struct IAnimatedCharacter;

class CCooperativeAnimationManager : public ICooperativeAnimationManager
{
public:
	CCooperativeAnimationManager() {}
	virtual ~CCooperativeAnimationManager();

	// update function for every frame
	// dt is the time passed since last frame
	VIRTUAL void Update( float dt );

	VIRTUAL void Reset();

	//! Start a new Cooperative animation
	//! Returns true if the animation was started
	//! If one of the specified actors is already part of a cooperative animation,
	//! the function will return false.
	VIRTUAL bool StartNewCooperativeAnimation(TCharacterParams &characterParams, const SCooperativeAnimParams &generalParams);

	//! Start a new Cooperative animation
	//! Returns true if the animation was started
	//! If one of the specified actors is already part of a cooperative animation,
	//! the function will return false.
	VIRTUAL bool StartNewCooperativeAnimation(SCharacterParams &params1, SCharacterParams &params2, const SCooperativeAnimParams &generalParams);

	//! This neat function allows to abuse the Cooperative Animation System for just a single
	//! character - taking advantage of the exact positioning
	VIRTUAL bool	StartExactPositioningAnimation(const SCharacterParams &params, const SCooperativeAnimParams &generalParams);

	//! Stops all cooperative animations on this actor
	//! Returns true if there was at least one animation stopped, false otherwise.
	VIRTUAL bool	StopCooperativeAnimationOnActor(IAnimatedCharacter* pActor);

	//! Same as above, but only stops cooperative animations that include both actors
	VIRTUAL bool	StopCooperativeAnimationOnActor(IAnimatedCharacter* pActor1, IAnimatedCharacter* pActor2);

	VIRTUAL bool IsActorBusy( const IAnimatedCharacter* pActor, const CCooperativeAnimation *pIgnoreAnimation = NULL ) const;
	VIRTUAL bool IsActorBusy( const EntityId entID ) const;

private:

	// list of active ongoing coop-animations
	typedef std::vector<CCooperativeAnimation*> TActiveAnimations;
	TActiveAnimations		 m_activeAnimations;
};

#endif //__COOPERATIVEANIMATIONMANAGER_H__

#ifndef __PROCEDURAL_CONTEXT_AIM__H__
#define __PROCEDURAL_CONTEXT_AIM__H__

#include <ICryMannequin.h>

#include <ICryAnimation.h>
#include <IAnimationPoseModifier.h>

#include <CryExtension/CryCreateClassInstance.h>
#include <CryExtension/Impl/ClassWeaver.h>
#include <CryExtension/Impl/ICryFactoryRegistryImpl.h>
#include <CryExtension/Impl/RegFactoryNode.h>

#include "ProceduralContextHelpers.h"

#define PROCEDURAL_CONTEXT_AIM_NAME "ProceduralContextAim"

class CProceduralContextAim
: public IProceduralContext
{
public:
	PROCEDURAL_CONTEXT( CProceduralContextAim, PROCEDURAL_CONTEXT_AIM_NAME, 0x4a5625bb01d149c6, 0xb5632cf301b58e38 );

	// IProceduralContext
	virtual void Initialise( IEntity& entity, IActionController& actionController );
	virtual void Update( float timePassedSeconds );
	// ~IProceduralContext

	void UpdateGameAimingRequest( const bool aimRequest );
	void UpdateProcClipAimingRequest( const bool aimRequest );

	void UpdateGameAimTarget( const Vec3& aimTarget );

	void SetBlendInTime( const float blendInTime );
	void SetBlendOutTime( const float blendOutTime );

	uint32 RequestPolarCoordinatesSmoothingParameters( const Vec2& maxSmoothRateRadiansPerSecond, const float smoothTimeSeconds );
	void CancelPolarCoordinatesSmoothingParameters( const uint32 requestId );

private:
	void InitialisePoseBlenderAim();
	void InitialiseGameAimTarget();

	void UpdatePolarCoordinatesSmoothingParameters();

private:
	IAnimationPoseBlenderDir* m_pPoseBlenderAim;

	bool m_gameRequestsAiming;
	bool m_procClipRequestsAiming;
	Vec3 m_gameAimTarget;

	Vec2 m_defaultPolarCoordinatesMaxSmoothRateRadiansPerSecond;
	float m_defaultPolarCoordinatesSmoothTimeSeconds;
	struct SPolarCoordinatesSmoothingParametersRequest
	{
		uint32 id;
		Vec2 maxSmoothRateRadiansPerSecond;
		float smoothTimeSeconds;
	};

	typedef ProceduralContextHelpers::CRequestList< SPolarCoordinatesSmoothingParametersRequest > TPolarCoordinatesSmoothingParametersRequestList;
	TPolarCoordinatesSmoothingParametersRequestList m_polarCoordinatesSmoothingParametersRequestList;
};


#endif

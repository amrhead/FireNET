//////////////////////////////////////////////////////////////////////////// 
// 
//  CryEngine Source File. 
//  Copyright (C), Crytek, 1999-2012. 
// ------------------------------------------------------------------------- 
//  File name:   IMovementSystem.h
//  Version:     v1.00 
//  Created:     2011-12-20 by Jonas Johansson. 
//  Description: Interface for the AI Movement System.
// ------------------------------------------------------------------------- 
//  History: 
// 
//////////////////////////////////////////////////////////////////////////// 

#pragma once

#ifndef IMovementSystem_h
#define IMovementSystem_h

#include <IAgent.h>
#include <MovementStyle.h>

struct MovementRequest;
struct MovementRequestID;
struct MovementRequestStatus;
struct MNMPathRequest;
class INavPath;
class IPathFollower;
struct SShape;
struct PathFollowResult;
struct OffMeshLink_SmartObject;
class CSmartObject;

namespace Movement
{
	struct IPlanner;

	enum PathfinderState
	{
		StillFinding,
		FoundPath,
		CouldNotFindPath,
	};
}

typedef Functor1<const Vec3&> SetOutputVelocityFunction;
typedef Functor1<MNMPathRequest&> PathRequestFunction;
typedef Functor0wRet<Movement::PathfinderState> CheckOnPathfinderStateFunction;
typedef Functor0wRet<INavPath*> GetPathFunction;
typedef Functor0wRet<IPathFollower*> GetPathFollowerFunction;

struct MovementActorCallbacks
{
	MovementActorCallbacks()
		: queuePathRequestFunction( NULL )
		, checkOnPathfinderStateFunction( NULL )
		, getPathFunction( NULL )
		, getPathFollowerFunction( NULL )
	{}

	PathRequestFunction queuePathRequestFunction;
	CheckOnPathfinderStateFunction checkOnPathfinderStateFunction;
	GetPathFunction getPathFunction;
	GetPathFollowerFunction getPathFollowerFunction;
};

struct IMovementActorAdapter
{
	virtual ~IMovementActorAdapter() {}

	virtual void OnMovementPlanProduced() = 0;

	virtual Vec3 GetPhysicsPosition() = 0;
	virtual Vec3 GetVelocity() = 0;
	virtual Vec3 GetMoveDirection() = 0;
	virtual Vec3 GetAnimationBodyDirection() = 0;
	virtual EActorTargetPhase GetActorPhase() = 0;
	virtual void SetMovementOutputValue(const PathFollowResult& result) = 0;
	virtual void SetBodyTargetDirection(const Vec3& direction)  = 0;
	virtual void ResetMovementContext() = 0;
	virtual void ClearMovementState() = 0;
	virtual void ResetBodyTarget() = 0;
	virtual void ResetActorTargetRequest() = 0;
	virtual bool IsMoving() = 0;

	virtual void RequestExactPosition(const SAIActorTargetRequest* request, const bool lowerPrecision) = 0;

	virtual bool IsClosestToUseTheSmartObject(const OffMeshLink_SmartObject& smartObjectLink) = 0;
	virtual bool PrepareNavigateSmartObject(CSmartObject* pSmartObject, OffMeshLink_SmartObject* pSmartObjectLink) = 0;
	virtual void InvalidateSmartObjectLink(CSmartObject* pSmartObject, OffMeshLink_SmartObject* pSmartObjectLink) = 0;

	virtual void SetInCover( const bool inCover ) = 0;
	virtual void UpdateCoverLocations() = 0;
	virtual void InstallInLowCover(const bool inCover) = 0;
	virtual void SetupCoverInformation() = 0;
	virtual bool IsInCover() = 0;

	virtual bool GetDesignedPath( SShape& pathShape ) = 0;
	virtual void CancelRequestedPath() = 0;
	virtual void ConfigurePathfollower(const MovementStyle& style) = 0;

	virtual void SetActorPath( const MovementStyle& style, const INavPath& navPath ) = 0;
	virtual void SetActorStyle( const MovementStyle& style, const INavPath& navPath ) = 0;
	virtual void SetStance(const MovementStyle::Stance stance) = 0;

	virtual boost::shared_ptr<Vec3> CreateLookTarget() = 0;
	virtual void SetLookTimeOffset(float lookTimeOffset) = 0;
	virtual void UpdateLooking(float updateTime, boost::shared_ptr<Vec3> lookTarget, const bool targetReachable, const float pathDistanceToEnd, const Vec3& followTargetPosition, const MovementStyle& style) = 0;
};

struct IMovementSystem
{
	virtual ~IMovementSystem() {}

	//
	// Register your entity to the movement system and provides the necessary information for the movement system to fulfill his work
	//
	virtual void RegisterEntity(const EntityId entityId, MovementActorCallbacks callbacksConfiguration, IMovementActorAdapter& adapter) = 0;

	//
	// Unregister your entity from the movement system to stop using its services
	//
	virtual void UnregisterEntity(const EntityId entityId) = 0;

	//
	// Ask the movement system to satisfy your request.
	//
	// The movement system will contact you via the callback to inform
	// you whether the request has been satisfied or not.
	//
	// If you submit a callback you are responsible to to call CancelRequest
	// before the callback becomes invalid.
	//

	virtual MovementRequestID QueueRequest(const MovementRequest& request) = 0;

	//
	// Tell the movement system you are no longer interested in getting
	// your request satisfied. Note that this doesn't necessarily mean
	// that the actor will stop, it simply means that you no longer care
	// about the outcome.
	//

	virtual void CancelRequest(const MovementRequestID& id) = 0;

	//
	// Get information about the current status of a request.
	// You'll see if it's in queue, path finding, or
	// what block of a plan it's currently executing.
	//

	virtual void GetRequestStatus(const MovementRequestID& id, MovementRequestStatus& status) const = 0;

	//
	// This resets the movement system to it's initial state.
	//
	// - The internal data will be cleaned out and reset.
	// - No callbacks will be called during reset process.
	//

	virtual void Reset() = 0;

	//
	// This is where the movement system will do it's work.
	//

	virtual void Update(float updateTime) = 0;
};

#endif // IMovementSystem_h
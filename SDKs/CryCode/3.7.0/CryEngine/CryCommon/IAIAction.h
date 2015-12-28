#ifndef __IAIACTION_H__
#define __IAIACTION_H__

#pragma once


struct IGoalPipe;
struct IFlowGraph;


///////////////////////////////////////////////////
// IAIAction references an Action Flow Graph -
// ordered sequence of elementary actions which
// will be executed to "use" a smart object
///////////////////////////////////////////////////
struct IAIAction
{
	/// Event names used to inform listeners of an action when the action's state changes
	enum EActionEvent
	{
		ActionEnd = 0,
		ActionStart,
		ActionCancel,
		ActionAbort,
	};

	struct IAIActionListener
	{

		virtual ~IAIActionListener(){}
		virtual void OnActionEvent(EActionEvent event) = 0;

	};

	virtual ~IAIAction(){}
	// returns the unique name of this AI Action
	virtual const char* GetName() const = 0;

	// traverses all nodes of the underlying flow graph and checks for any nodes that are incompatible when being run in the context of an IAIAction
	virtual bool TraverseAndValidateAction(EntityId idOfUser) const = 0;

	// returns the goal pipe which executes this AI Action
	virtual IGoalPipe* GetGoalPipe() const = 0;

	// returns the Flow Graph associated to this AI Action
	virtual IFlowGraph* GetFlowGraph() const = 0;

	// returns the User entity associated to this AI Action
	virtual IEntity* GetUserEntity() const = 0;

	// returns the Object entity associated to this AI Action
	virtual IEntity* GetObjectEntity() const = 0;

	// returns true if action is active and marked as high priority
	virtual bool IsHighPriority() const { return false; }

	// ends execution of this AI Action
	virtual void EndAction() = 0;

	// cancels execution of this AI Action
	virtual void CancelAction() = 0;

	// aborts execution of this AI Action - will start clean up procedure
	virtual bool AbortAction() = 0;

	// marks this AI Action as modified
	virtual void Invalidate() = 0;

	virtual bool IsSignaledAnimation() const = 0;
	virtual bool IsExactPositioning() const = 0;
	virtual const char* GetAnimationName() const = 0;

	virtual const Vec3& GetAnimationPos() const = 0;
	virtual const Vec3& GetAnimationDir() const = 0;
	virtual bool IsUsingAutoAssetAlignment() const = 0;

	virtual const Vec3& GetApproachPos() const = 0;

	virtual float GetStartWidth() const = 0;
	virtual float GetStartArcAngle() const = 0;
	virtual float GetDirectionTolerance() const = 0;

	virtual void RegisterListener(IAIActionListener* eventListener, const char* name)=0;
	virtual void UnregisterListener(IAIActionListener* eventListener)=0;

};


#endif

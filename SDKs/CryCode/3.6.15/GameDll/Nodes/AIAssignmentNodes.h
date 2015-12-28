#pragma once

#ifndef AIAssignmentNodes_h
#define AIAssignmentNodes_h

#include "G2FlowBaseNode.h"
#include "AI/Assignment.h"

//////////////////////////////////////////////////////////////////////////

class AssignmentFlowNodeBase 
	: public CFlowBaseNode<eNCT_Singleton>
{
public:
	virtual int GetSetPortNumber() const = 0;
	virtual Assignment SpecifyAssignment(SActivationInfo* pActInfo) = 0;

	void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo);

protected:

	void DispatchAssignment( Assignment assignment, IEntity* entity );
};

//////////////////////////////////////////////////////////////////////////

class ClearAssignmentFlowNode
	: public AssignmentFlowNodeBase
{
public:
	enum InputPort
	{
		InputPort_Set,
	};

	ClearAssignmentFlowNode(SActivationInfo* pActInfo)	{}

	virtual void GetConfiguration( SFlowNodeConfig& config );
	virtual void GetMemoryUsage( ICrySizer * sizer ) const { sizer->Add(*this); }

	int GetSetPortNumber() const { return InputPort_Set; }
	Assignment SpecifyAssignment(SActivationInfo* pActInfo);
};

//////////////////////////////////////////////////////////////////////////

class DefendAreaAssignmentFlowNode
	: public AssignmentFlowNodeBase
{
public:
	enum InputPort
	{
		InputPort_Set,
		InputPort_Position,
		InputPort_Radius,
	};

	DefendAreaAssignmentFlowNode(SActivationInfo* pActInfo)	{}

	virtual void GetConfiguration( SFlowNodeConfig& config );
	virtual void GetMemoryUsage( ICrySizer * sizer ) const { sizer->Add(*this); }

	int GetSetPortNumber() const { return InputPort_Set; }
	Assignment SpecifyAssignment(SActivationInfo* pActInfo);
};

//////////////////////////////////////////////////////////////////////////

class HoldPositionAssignmentFlowNode
	: public AssignmentFlowNodeBase
{
public:
	enum InputPort
	{
		InputPort_Set,
		InputPort_Position,
		InputPort_Radius,
		InputPort_Direction,
		InputPort_UseCover,
	};

	HoldPositionAssignmentFlowNode(SActivationInfo* pActInfo)	{}

	virtual void GetConfiguration( SFlowNodeConfig& config );
	virtual void GetMemoryUsage( ICrySizer * sizer ) const { sizer->Add(*this); }

	int GetSetPortNumber() const { return InputPort_Set; }
	Assignment SpecifyAssignment(SActivationInfo* pActInfo);
};

//////////////////////////////////////////////////////////////////////////

class CombatMoveAssignmentFlowNode
	: public AssignmentFlowNodeBase
{
public:
	enum InputPort
	{
		InputPort_Set,
		InputPort_Position,
		InputPort_UseCover,
	};

	CombatMoveAssignmentFlowNode(SActivationInfo* pActInfo)	{}

	virtual void GetConfiguration( SFlowNodeConfig& config );
	virtual void GetMemoryUsage( ICrySizer * sizer ) const { sizer->Add(*this); }

	int GetSetPortNumber() const { return InputPort_Set; }
	Assignment SpecifyAssignment(SActivationInfo* pActInfo);
};

//////////////////////////////////////////////////////////////////////////


#endif //AIAssignmentNodes_h

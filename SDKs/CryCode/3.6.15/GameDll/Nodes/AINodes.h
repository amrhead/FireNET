#pragma once

#ifndef AINodes_H
#define AINodes_H

#include "G2FlowBaseNode.h"
#include "AI/Assignment.h"

class CFlowNode_BattleFrontControl : public CFlowBaseNode<eNCT_Instanced>
{
public:
	enum EInputPorts
	{
		EIP_Group,
		EIP_Position,
		EIP_Activate,
		EIP_Deactivate,
	};

	static const SInputPortConfig inputPorts[];

	CFlowNode_BattleFrontControl(SActivationInfo * activationInformation)
		: m_enabled(false)
	{
	}

	virtual void GetConfiguration(SFlowNodeConfig& config);
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo *activationInformation);
	virtual void GetMemoryUsage(ICrySizer * sizer) const;
	virtual IFlowNodePtr Clone(SActivationInfo *pActInfo) { return new CFlowNode_BattleFrontControl(pActInfo); }

private:
	bool m_enabled;
};

class CFlowNode_SetTurretFaction : public CFlowBaseNode<eNCT_Singleton>
{
public:
	enum InputPort
	{
		InputPort_Faction,
		InputPort_Set,
	};

	CFlowNode_SetTurretFaction(SActivationInfo* /*pActInfo*/)
	{
	}

	virtual void GetConfiguration(SFlowNodeConfig& config);
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo);
	virtual void GetMemoryUsage(ICrySizer * sizer) const { sizer->Add(*this); }

private:
	void SetFaction(SActivationInfo* pActInfo);
	class CGunTurret* GetTurret(IEntity& entity) const;
};


class CFlowNode_AICorpses : public CFlowBaseNode<eNCT_Singleton>
{
public:
	enum InputPort
	{
		InputPort_CleanCorpses,
	};

	CFlowNode_AICorpses(SActivationInfo* /*pActInfo*/)
	{
	}

	virtual void GetConfiguration(SFlowNodeConfig& config);
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo);

	virtual void GetMemoryUsage(ICrySizer * sizer) const;

};


#endif // AINodes_H

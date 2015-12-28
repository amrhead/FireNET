#ifndef __FLOWLOGNODE_H__
#define __FLOWLOGNODE_H__

#pragma once

#include "IFlowSystem.h"

class CFlowLogNode : public IFlowNode
{
public:
	CFlowLogNode();

	// IFlowNode
	virtual void AddRef();
	virtual void Release();
	virtual IFlowNodePtr Clone( SActivationInfo * );
	virtual void GetConfiguration( SFlowNodeConfig& );
	virtual void ProcessEvent( EFlowEvent event, SActivationInfo * );
	virtual bool SerializeXML( SActivationInfo *, const XmlNodeRef&, bool );
	virtual void Serialize(SActivationInfo *, TSerialize ser);
	virtual void PostSerialize(SActivationInfo *){}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}
	// ~IFlowNode

private:
	int m_refs;
};

#endif

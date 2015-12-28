/********************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2011.
-------------------------------------------------------------------------
File name:   FlowModuleNodes.h
$Id$
Description: Flowgraph Nodes for module usage

-------------------------------------------------------------------------
History:
- 03/04/11   : Sascha Hoba - Kevin Kirst 

*********************************************************************/

#ifndef _FLOWMODULENODES_H_
#define _FLOWMODULENODES_H_

#include <IFlowGraphModuleManager.h>
#include "../Nodes/FlowBaseNode.h"

//////////////////////////////////////////////////////////////////////////
// 
//	Dynamically generated nodes based on a module's inputs/outputs
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Start node factory
//////////////////////////////////////////////////////////////////////////

class CFlowModuleStartNodeFactory : public IFlowNodeFactory
{
public:
	CFlowModuleStartNodeFactory(TModuleId moduleId);
	~CFlowModuleStartNodeFactory();

	virtual void AddRef() { m_nRefCount++; }
	virtual void Release() { if (0 == --m_nRefCount) delete this; }
	virtual IFlowNodePtr Create( IFlowNode::SActivationInfo* );

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		SIZER_SUBCOMPONENT_NAME(s, "CFlowModuleStartNodeFactory");
		s->AddObject(this, sizeof(*this) );
		s->AddObject(m_outputs);	
	}

	void GetConfiguration( SFlowNodeConfig& );

	void Reset();

	virtual bool AllowOverride() const OVERRIDE {return true;}

private:
	friend class CFlowModuleStartNode;

	TModuleId m_moduleId;
	int m_nRefCount;
	std::vector<SOutputPortConfig> m_outputs;
};

//////////////////////////////////////////////////////////////////////////
// and the actual Start node class built by the factory above
//////////////////////////////////////////////////////////////////////////

class CFlowModuleStartNode : public CFlowBaseNode<eNCT_Instanced>
{
public:
	CFlowModuleStartNode( CFlowModuleStartNodeFactory *pClass, SActivationInfo *pActInfo );
	virtual ~CFlowModuleStartNode()
	{
	}
	virtual IFlowNodePtr Clone( SActivationInfo *pActInfo );
	virtual void GetConfiguration( SFlowNodeConfig& );
	virtual void ProcessEvent( EFlowEvent event, SActivationInfo * pActInfo );

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	void OnActivate(TModuleParams const& params);
	void OnCancel();

private:

	enum EOutputPorts
	{
		eOP_Start = 0,
		eOP_Update,
		eOP_Cancel,
		eOP_Param1,
		// ...
	};

	void Serialize( SActivationInfo * pActInfo, TSerialize ser ) {}

	_smart_ptr<CFlowModuleStartNodeFactory> m_pClass;

	SActivationInfo m_actInfo;

	bool m_bStarted;
};

//////////////////////////////////////////////////////////////////////////
// Return node factory
//////////////////////////////////////////////////////////////////////////

class CFlowModuleReturnNodeFactory : public IFlowNodeFactory
{
public:
	CFlowModuleReturnNodeFactory(TModuleId moduleId);
	~CFlowModuleReturnNodeFactory();

	virtual void AddRef() { m_nRefCount++; }
	virtual void Release() { if (0 == --m_nRefCount) delete this; }
	virtual IFlowNodePtr Create( IFlowNode::SActivationInfo* );

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		SIZER_SUBCOMPONENT_NAME(s, "CFlowModuleReturnNodeFactory");
		s->AddObject(this, sizeof(*this) );
		s->AddObject(m_inputs);	
	}

	void GetConfiguration( SFlowNodeConfig& );
	
	void Reset();

	virtual bool AllowOverride() const OVERRIDE {return true;}

private:
	friend class CFlowModuleReturnNode;

	TModuleId m_moduleId;
	int m_nRefCount;
	std::vector<SInputPortConfig> m_inputs;
};

//////////////////////////////////////////////////////////////////////////
// and the actual Return node class built by the factory above
//////////////////////////////////////////////////////////////////////////

class CFlowModuleReturnNode : public CFlowBaseNode<eNCT_Instanced>
{
public:
	CFlowModuleReturnNode( CFlowModuleReturnNodeFactory *pClass, SActivationInfo *pActInfo, TModuleId moduleId );
	virtual ~CFlowModuleReturnNode()
	{
	}
	virtual IFlowNodePtr Clone( SActivationInfo *pActInfo );
	virtual void GetConfiguration( SFlowNodeConfig& );
	virtual void ProcessEvent( EFlowEvent event, SActivationInfo * pActInfo );

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	void OnActivate(bool success);

	void SetModuleId(TModuleId id, TModuleInstanceId instanceId) { m_ownerId = id; m_instanceId = instanceId; }

private:

	enum EInputPorts
	{
		eIP_Success = 0,
		eIP_Cancel,
		eIP_Param1,
		// ...
	};

	void Serialize( SActivationInfo * pActInfo, TSerialize ser ) {}

	_smart_ptr<CFlowModuleReturnNodeFactory> m_pClass;

	SActivationInfo m_actInfo;
	TModuleId m_ownerId;
	TModuleInstanceId m_instanceId;
};

//////////////////////////////////////////////////////////////////////////
// Call module node factory
//////////////////////////////////////////////////////////////////////////

class CFlowModuleCallNodeFactory : public IFlowNodeFactory
{
public:
	CFlowModuleCallNodeFactory(TModuleId moduleId);
	~CFlowModuleCallNodeFactory();

	virtual void AddRef() { m_nRefCount++; }
	virtual void Release() { if (0 == --m_nRefCount) delete this; }
	virtual IFlowNodePtr Create( IFlowNode::SActivationInfo* );

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		SIZER_SUBCOMPONENT_NAME(s, "CFlowModuleCallNodeFactory");
		s->AddObject(this, sizeof(*this) );
		s->AddObject(m_outputs);	
	}

	void GetConfiguration( SFlowNodeConfig& );

	void Reset();

	virtual bool AllowOverride() const OVERRIDE {return true;}

private:
	friend class CFlowModuleCallNode;

	TModuleId m_moduleId;
	int m_nRefCount;
	std::vector<SInputPortConfig> m_inputs;
	std::vector<SOutputPortConfig> m_outputs;
};

//////////////////////////////////////////////////////////////////////////
// and the actual Call node class built by the factory above
//////////////////////////////////////////////////////////////////////////

class CFlowModuleCallNode : public CFlowBaseNode<eNCT_Instanced>
{
public:
	CFlowModuleCallNode( CFlowModuleCallNodeFactory *pClass, SActivationInfo *pActInfo );
	virtual ~CFlowModuleCallNode()
	{
	}
	virtual IFlowNodePtr Clone( SActivationInfo *pActInfo );
	virtual void GetConfiguration( SFlowNodeConfig& );
	virtual void ProcessEvent( EFlowEvent event, SActivationInfo * pActInfo );

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	void OnReturn(bool bSuccess, TModuleParams const& params);

private:

	enum EInputPort
	{
		eIP_Call = 0,
		eIP_Instanced,
		eIP_Cancel,
		eIP_InstanceId,
		eIP_Param1,
		// ...
	};
	enum EOutputPort
	{
		eOP_OnCall = 0,
		eOP_Success,
		eOP_Canceled,
		eOP_Param1,
		// ...
	};

	void Serialize( SActivationInfo * pActInfo, TSerialize ser ) {}

	_smart_ptr<CFlowModuleCallNodeFactory> m_pClass;

	SActivationInfo m_actInfo;

	TModuleInstanceId m_instanceId;
};

//////////////////////////////////////////////////////////////////////////
// helper class to bind user id to a module instance id (e.g. to map entity id to a module id)
//////////////////////////////////////////////////////////////////////////
class CFlowModuleUserIdNode : public CFlowBaseNode<eNCT_Singleton>
{
public:
	CFlowModuleUserIdNode( SActivationInfo * pActInfo ) {}
	virtual ~CFlowModuleUserIdNode() {}

	virtual void GetMemoryUsage(ICrySizer * s) const { s->Add(*this); }
	virtual void GetConfiguration( SFlowNodeConfig &config );
	virtual void ProcessEvent( EFlowEvent event,SActivationInfo *pActInfo );

	static void RemoveModuleInstance(TModuleInstanceId id);

private:
	enum EInputs
	{
		eI_Get = 0,
		eI_Set,
		eI_UserId,
		eI_ModuelId,
	};
	enum EOutputs
	{
		eO_ModuleId = 0,
	};

	static std::map<int, TModuleInstanceId> m_ids;
};
#endif //_FLOWMODULENODES_H_

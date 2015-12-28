// Copyright 2001-2015 Crytek GmbH. All rights reserved.

#ifndef DYNAMICRESPONSESYSTEM_PROXY_H_
#define DYNAMICRESPONSESYSTEM_PROXY_H_

#include <IDynamicResponseSystem.h>

//////////////////////////////////////////////////////////////////////////
// Description:
//    Implements dynamic response proxy class for entity.
//////////////////////////////////////////////////////////////////////////
class CDynamicResponseProxy : public IEntityDynamicResponseProxy
{
public:
	CDynamicResponseProxy();
	virtual ~CDynamicResponseProxy();

	//IComponent override
	virtual void Initialize(SComponentInitializer const& init);

	//////////////////////////////////////////////////////////////////////////
	// IEntityProxy interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual	void ProcessEvent(SEntityEvent& event);
	virtual EEntityProxy GetType() { return ENTITY_PROXY_DYNAMICRESPONSE; }
	virtual void Release();
	virtual void Done() {};
	virtual	void Update(SEntityUpdateContext& ctx);
	virtual bool Init(IEntity* pEntity, SEntitySpawnParams& params) { return true; }
	virtual void Reload(IEntity* pEntity, SEntitySpawnParams& params);
	virtual void SerializeXML(XmlNodeRef& entityNode, bool bLoading);
	virtual bool NeedSerialize();
	virtual void Serialize(TSerialize ser);
	virtual bool GetSignature(TSerialize signature);
	virtual void GetMemoryUsage(ICrySizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}
	//////////////////////////////////////////////////////////////////////////
	
	//////////////////////////////////////////////////////////////////////////
	// IEntityDynamicResponseProxy interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void QueueSignal(const char* pSignalName, DRS::IVariableCollection* pSignalContext /* = 0 */, float delayBeforeFiring /* = 0.0f */, bool autoReleaseCollection = true);	
	virtual DRS::IVariableCollection* GetLocalVariableCollection() const;
	virtual DRS::IResponseActor* GetResponseActor() const;
	//////////////////////////////////////////////////////////////////////////

private:
	DRS::IResponseActor* m_pResponseActor;
};

#endif // DYNAMICRESPONSESYSTEM_PROXY_H_

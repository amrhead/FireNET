//  Copyright (C), Crytek Studios, 2001-2014

#ifndef __ENTITYATTRIBUTESPROXY_H__
#define __ENTITYATTRIBUTESPROXY_H__

#pragma once

#include "EntitySystem.h"
#include "IEntityAttributesProxy.h"
#include "IEntityClass.h"
#include "IEntityProxy.h"
#include "ISerialize.h"

//////////////////////////////////////////////////////////////////////////
// Description:
//    Proxy for storage of entity attributes.
//////////////////////////////////////////////////////////////////////////
class CEntityAttributesProxy : public IEntityAttributesProxy
{
public:

	// IComponent
	virtual void ProcessEvent(SEntityEvent& event) override;
	virtual void Initialize(SComponentInitializer const& inititializer) override;
	// ~IComponent

	// IEntityProxy
	virtual EEntityProxy GetType() override;
	virtual void Release() override;
	virtual void Done() override;
	virtual void Update(SEntityUpdateContext& context) override;
	virtual bool Init(IEntity* pEntity, SEntitySpawnParams& params) override;
	virtual void Reload(IEntity* pEntity, SEntitySpawnParams& params) override;
	virtual void SerializeXML(XmlNodeRef &entityNodeXML, bool loading) override;
	virtual void Serialize(TSerialize serialize) override;
	virtual bool NeedSerialize() override;
	virtual bool GetSignature(TSerialize signature) override;
	virtual void GetMemoryUsage(ICrySizer* pSizer) const override;
	// ~IEntityProxy

	// IEntityAttributesProxy
	virtual void SetAttributes(const TEntityAttributeArray& attributes) override;
	virtual TEntityAttributeArray& GetAttributes() override;
	virtual const TEntityAttributeArray& GetAttributes() const override;
	// ~IEntityAttributesProxy

private:

	TEntityAttributeArray	m_attributes;
};

DECLARE_BOOST_POINTERS(CEntityAttributesProxy)

#endif //__ENTITYATTRIBUTESPROXY_H__
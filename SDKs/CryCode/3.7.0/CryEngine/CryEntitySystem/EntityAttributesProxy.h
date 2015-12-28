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
	VIRTUAL void ProcessEvent(SEntityEvent& event) OVERRIDE;
	VIRTUAL void Initialize(SComponentInitializer const& inititializer) OVERRIDE;
	// ~IComponent

	// IEntityProxy
	VIRTUAL EEntityProxy GetType() OVERRIDE;
	VIRTUAL void Release() OVERRIDE;
	VIRTUAL void Done() OVERRIDE;
	VIRTUAL	void Update(SEntityUpdateContext& context) OVERRIDE;
	VIRTUAL bool Init(IEntity* pEntity, SEntitySpawnParams& params) OVERRIDE;
	VIRTUAL void Reload(IEntity* pEntity, SEntitySpawnParams& params) OVERRIDE;
	VIRTUAL void SerializeXML(XmlNodeRef &entityNodeXML, bool loading) OVERRIDE;
	VIRTUAL void Serialize(TSerialize serialize) OVERRIDE;
	VIRTUAL bool NeedSerialize() OVERRIDE;
	VIRTUAL bool GetSignature(TSerialize signature) OVERRIDE;
	VIRTUAL void GetMemoryUsage(ICrySizer* pSizer) const OVERRIDE;
	// ~IEntityProxy

	// IEntityAttributesProxy
	VIRTUAL void SetAttributes(const TEntityAttributeArray& attributes) OVERRIDE;
	VIRTUAL TEntityAttributeArray& GetAttributes() OVERRIDE;
	VIRTUAL const TEntityAttributeArray& GetAttributes() const OVERRIDE;
	// ~IEntityAttributesProxy

private:

	TEntityAttributeArray	m_attributes;
};

DECLARE_BOOST_POINTERS(CEntityAttributesProxy)

#endif //__ENTITYATTRIBUTESPROXY_H__
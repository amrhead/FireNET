// Copyright 2001-2015 Crytek GmbH. All rights reserved.

#include "stdafx.h"
#include "DynamicResponseProxy.h"
#include <IDynamicResponseSystem.h>
#include <ISerialize.h>


//////////////////////////////////////////////////////////////////////////
CDynamicResponseProxy::CDynamicResponseProxy()
	: m_pResponseActor(NULL)
{
}

//////////////////////////////////////////////////////////////////////////
CDynamicResponseProxy::~CDynamicResponseProxy()
{
}

//////////////////////////////////////////////////////////////////////////
void CDynamicResponseProxy::Initialize(SComponentInitializer const& init)
{
	m_pResponseActor = gEnv->pDynamicResponseSystem->CreateResponseActor(init.m_pEntity->GetName(), init.m_pEntity->GetId());
	assert(m_pResponseActor);
	m_pResponseActor->GetLocalVariables()->SetVariableValue("Name", init.m_pEntity->GetName());
}

//////////////////////////////////////////////////////////////////////////
void CDynamicResponseProxy::Reload(IEntity* pEntity, SEntitySpawnParams& params)
{
}

//////////////////////////////////////////////////////////////////////////
void CDynamicResponseProxy::Release()
{
	gEnv->pDynamicResponseSystem->ReleaseResponseActor(m_pResponseActor);
	m_pResponseActor = NULL;
	delete this;
}

//////////////////////////////////////////////////////////////////////////
void CDynamicResponseProxy::Update(SEntityUpdateContext& ctx)
{
}

//////////////////////////////////////////////////////////////////////////
void CDynamicResponseProxy::ProcessEvent(SEntityEvent& event)
{
	if (event.event==ENTITY_EVENT_RESET)
	{		
		m_pResponseActor->GetLocalVariables()->SetVariableValue("Name", m_pResponseActor->GetName());
	}
}

//////////////////////////////////////////////////////////////////////////
bool CDynamicResponseProxy::NeedSerialize()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CDynamicResponseProxy::GetSignature(TSerialize signature)
{
	signature.BeginGroup("DynamicResponseProxy");
	signature.EndGroup();
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CDynamicResponseProxy::Serialize(TSerialize ser)
{

}

//////////////////////////////////////////////////////////////////////////
void CDynamicResponseProxy::SerializeXML(XmlNodeRef& entityNode, bool bLoading)
{

}

//////////////////////////////////////////////////////////////////////////
void CDynamicResponseProxy::QueueSignal(const char* pSignalName, DRS::IVariableCollection* pSignalContext /* = 0 */, float delayBeforeFiring /* = 0.0f */, bool autoReleaseCollection)
{
	gEnv->pDynamicResponseSystem->QueueSignal(pSignalName, m_pResponseActor, pSignalContext, delayBeforeFiring, (pSignalContext) ? autoReleaseCollection : false);
}

//////////////////////////////////////////////////////////////////////////
DRS::IVariableCollection* CDynamicResponseProxy::GetLocalVariableCollection() const
{
	assert(m_pResponseActor && "Proxy without Actor detected...");
	return m_pResponseActor->GetLocalVariables();
}

//////////////////////////////////////////////////////////////////////////
DRS::IResponseActor* CDynamicResponseProxy::GetResponseActor() const
{
	return m_pResponseActor;
}


//////////////////////////////////////////////////////////////////////////

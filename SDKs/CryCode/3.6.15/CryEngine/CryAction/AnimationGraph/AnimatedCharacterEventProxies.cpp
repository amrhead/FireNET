// CryEngine Source File.
// Copyright (C), Crytek, 1999-2014.


#include "StdAfx.h"
#include "AnimatedCharacterEventProxies.h"
#include "AnimatedCharacter.h"

#include "CryActionCVars.h"

#include <IComponent.h>

#include <CryExtension/Impl/ClassWeaver.h>
#include <CryExtension/CryCreateClassInstance.h>

//////////////////////////////////////////////////////////////////////////

CAnimatedCharacterComponent_Base::CAnimatedCharacterComponent_Base()
	: m_pAnimCharacter(NULL)
{
}

void CAnimatedCharacterComponent_Base::Initialize( const SComponentInitializer& init )
{
	m_piEntity = init.m_pEntity;
	m_pAnimCharacter = static_cast<const SComponentInitializerAnimChar&> (init).m_pAnimCharacter;
	
	RegisterEvent( ENTITY_EVENT_PREPHYSICSUPDATE, IComponent::EComponentFlags_Enable );
}

void CAnimatedCharacterComponent_Base::ProcessEvent( SEntityEvent &event )
{
	switch( event.event )
	{
	case ENTITY_EVENT_PREPHYSICSUPDATE:
		OnPrePhysicsUpdate( event.fParam[0] );
		break;
	}
}

//////////////////////////////////////////////////////////////////////////

CAnimatedCharacterComponent_PrepareAnimatedCharacterForUpdate::CAnimatedCharacterComponent_PrepareAnimatedCharacterForUpdate()
	: m_queuedRotation( IDENTITY )
	, m_hasQueuedRotation( false )
{
}

void CAnimatedCharacterComponent_PrepareAnimatedCharacterForUpdate::OnPrePhysicsUpdate( float )
{
	CRY_ASSERT( m_pAnimCharacter );
	
	if ( CAnimationGraphCVars::Get().m_useQueuedRotation && m_hasQueuedRotation )
	{
		m_piEntity->SetRotation( m_queuedRotation, ENTITY_XFORM_USER|ENTITY_XFORM_NOT_REREGISTER );
		ClearQueuedRotation();
	}

	m_pAnimCharacter->PrepareAnimatedCharacterForUpdate();
}

IComponent::ComponentEventPriority CAnimatedCharacterComponent_PrepareAnimatedCharacterForUpdate::GetEventPriority( const int eventID ) const
{
	CRY_ASSERT( m_pAnimCharacter );

	switch( eventID )
	{
	case ENTITY_EVENT_PREPHYSICSUPDATE:
		{
			int priority = ENTITY_PROXY_LAST - ENTITY_PROXY_USER + EEntityEventPriority_PrepareAnimatedCharacterForUpdate;

			if( m_pAnimCharacter->IsClient() )
			{
				// we want the client StartAnimProc to happen after both CActor and GenMoveRequest.
				priority += EEntityEventPriority_Client;
			}

			return priority;
		}
	}
	return( ENTITY_PROXY_LAST - ENTITY_PROXY_USER );
}

//////////////////////////////////////////////////////////////////////////

void CAnimatedCharacterComponent_StartAnimProc::OnPrePhysicsUpdate( float elapsedTime )
{
	CRY_ASSERT( m_pAnimCharacter );

	m_pAnimCharacter->PrepareAndStartAnimProc();
}

IComponent::ComponentEventPriority CAnimatedCharacterComponent_StartAnimProc::GetEventPriority( const int eventID ) const
{
	CRY_ASSERT( m_pAnimCharacter );

	switch( eventID )
	{
	case ENTITY_EVENT_PREPHYSICSUPDATE:
		{
			int priority = ENTITY_PROXY_LAST - ENTITY_PROXY_USER + EEntityEventPriority_StartAnimProc;

			if( m_pAnimCharacter->IsClient() )
			{
				// we want the client StartAnimProc to happen after both CActor and GenMoveRequest.
				priority += EEntityEventPriority_Client;
			}
			return priority;
		}
	}

	return( ENTITY_PROXY_LAST - ENTITY_PROXY_USER );
}

//////////////////////////////////////////////////////////////////////////

void CAnimatedCharacterComponent_GenerateMoveRequest::OnPrePhysicsUpdate( float elapsedTime )
{
	CRY_ASSERT( m_pAnimCharacter );

	m_pAnimCharacter->GenerateMovementRequest();
}

IComponent::ComponentEventPriority CAnimatedCharacterComponent_GenerateMoveRequest::GetEventPriority( const int eventID ) const
{
	CRY_ASSERT( m_pAnimCharacter );

	switch( eventID )
	{
	case ENTITY_EVENT_PREPHYSICSUPDATE:
		{
			int priority = ENTITY_PROXY_LAST - ENTITY_PROXY_USER + EEntityEventPriority_AnimatedCharacter;

			if( m_pAnimCharacter->IsClient() )
			{
				priority += EEntityEventPriority_Client;
			}
			return priority;
		}
	}

	return( ENTITY_PROXY_LAST - ENTITY_PROXY_USER );
}

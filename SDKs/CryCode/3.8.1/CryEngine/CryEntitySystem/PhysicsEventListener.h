////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   PhysicsEventListener.h
//  Version:     v1.00
//  Created:     18/8/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __PhysicsEventListener_h__
#define __PhysicsEventListener_h__
#pragma once

// forward declaration.
class  CEntitySystem;
struct IPhysicalWorld;

//////////////////////////////////////////////////////////////////////////
class CPhysicsEventListener
{
public:
	CPhysicsEventListener( CEntitySystem *pEntitySystem,IPhysicalWorld *pPhysics );
	~CPhysicsEventListener();

	static int OnBBoxOverlap( const EventPhys *pEvent );
	static int OnStateChange( const EventPhys *pEvent );
	static int OnPostStep( const EventPhys *pEvent );
	static int OnUpdateMesh( const EventPhys *pEvent );
	static int OnCreatePhysEntityPart( const EventPhys *pEvent );
	static int OnRemovePhysEntityParts( const EventPhys *pEvent );
	static int OnRevealPhysEntityPart( const EventPhys *pEvent );
	static int OnPreUpdateMesh( const EventPhys *pEvent );
	static int OnPreCreatePhysEntityPart( const EventPhys *pEvent );
	static int OnCollision( const EventPhys *pEvent );
	static int OnJointBreak( const EventPhys *pEvent );
	static int OnPostPump( const EventPhys *pEvent );

	void RegisterPhysicCallbacks();
	void UnregisterPhysicCallbacks();

private:
	static CEntity* GetEntity( void *pForeignData,int iForeignData );
	static CEntity* GetEntity( IPhysicalEntity *pPhysEntity );

	CEntitySystem *m_pEntitySystem;
	IPhysicalWorld *m_pPhysics;

	struct PhysVisAreaUpdate
	{
		PhysVisAreaUpdate(IRenderNode *pRndNode, IPhysicalEntity *pEntity) { m_pRndNode=pRndNode; m_pEntity=pEntity; }
		IRenderNode *m_pRndNode;
		IPhysicalEntity *m_pEntity;
	};
	static std::vector<PhysVisAreaUpdate> m_physVisAreaUpdateVector;
};

#endif // __PhysicsEventListener_h__


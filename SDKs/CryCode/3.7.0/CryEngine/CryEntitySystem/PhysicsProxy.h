////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   PhysicsProxy.h
//  Version:     v1.00
//  Created:     25/5/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __PhysicsProxy_h__
#define __PhysicsProxy_h__
#pragma once

// forward declarations.
struct SEntityEvent;
struct IPhysicalEntity;
struct IPhysicalWorld;

//////////////////////////////////////////////////////////////////////////
// Description:
//    Implements base physical proxy class for entity.
//////////////////////////////////////////////////////////////////////////
class CPhysicalProxy : public IEntityPhysicalProxy
{
public:
	enum EFlags
	{
		CHARACTER_SLOT_MASK     = 0x000F, // Slot Id, of physicalized character.
		// When set Physical proxy will ignore incoming xform events from the entity.
		// Needed to prevent cycle change, when physical entity change entity xform and recieve back an event about entity xform.
		FLAG_IGNORE_XFORM_EVENT = 0x0010,
		FLAG_IGNORE_BUOYANCY    = 0x0020,
		FLAG_PHYSICS_DISABLED   = 0x0040,
		FLAG_SYNC_CHARACTER     = 0x0080,
		FLAG_WAS_HIDDEN					= 0x0100,
		FLAG_PHYS_CHARACTER     = 0x0200,
		FLAG_PHYS_AWAKE_WHEN_VISIBLE   = 0x0400,
		FLAG_ATTACH_CLOTH_WHEN_VISIBLE = 0x0800,
		FLAG_POS_EXTRAPOLATED   = 0x1000,
		FLAG_DISABLE_ENT_SERIALIZATION = 0x2000,
		FLAG_PHYSICS_REMOVED   = 0x4000,
	};

	CPhysicalProxy();
	~CPhysicalProxy() {};
	CEntity* GetEntity() const { return m_pEntity; };

	//////////////////////////////////////////////////////////////////////////
	// IEntityEvent interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void Initialize( const SComponentInitializer& init );
	virtual	void ProcessEvent( SEntityEvent &event );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// IEntityProxy interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual EEntityProxy GetType() { return ENTITY_PROXY_PHYSICS; }
	virtual void Release();
	virtual void Done();
	virtual	void Update( SEntityUpdateContext &ctx );
	virtual bool Init( IEntity *pEntity,SEntitySpawnParams &params ) { return true; }
	virtual void Reload( IEntity *pEntity,SEntitySpawnParams &params );
	virtual void SerializeXML( XmlNodeRef &entityNode,bool bLoading );
	virtual void Serialize( TSerialize ser );
	virtual bool NeedSerialize();
	VIRTUAL void SerializeTyped( TSerialize ser, int type, int flags );
	virtual bool GetSignature( TSerialize signature );
	VIRTUAL void EnableNetworkSerialization(bool enable);
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// IEntityPhysicalProxy interface.
	//////////////////////////////////////////////////////////////////////////
	VIRTUAL void GetLocalBounds( AABB &bbox );
	VIRTUAL void GetWorldBounds( AABB &bbox );
		
	VIRTUAL void Physicalize( SEntityPhysicalizeParams &params );
	VIRTUAL IPhysicalEntity* GetPhysicalEntity() const { return m_pPhysicalEntity; }
	VIRTUAL void EnablePhysics( bool bEnable );
	VIRTUAL bool IsPhysicsEnabled() const;
	VIRTUAL void AddImpulse( int ipart, const Vec3 &pos,const Vec3 &impulse,bool bPos,float fAuxScale, float fPushScale = 1.0f );

	VIRTUAL void SetTriggerBounds( const AABB &bbox );
	VIRTUAL void GetTriggerBounds( AABB &bbox );

  VIRTUAL int GetPartId0() { return m_partId0; }
	VIRTUAL void IgnoreXFormEvent(bool ignore) { SetFlags( ignore ? (m_nFlags|FLAG_IGNORE_XFORM_EVENT) : (m_nFlags&(~FLAG_IGNORE_XFORM_EVENT))); }
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Called from physics events.
	//////////////////////////////////////////////////////////////////////////
	// Called back by physics PostStep event for the owned physical entity.
	void OnPhysicsPostStep(EventPhysPostStep *pEvent=0);
	void AttachToPhysicalEntity( IPhysicalEntity *pPhysEntity );
	void CreateRenderGeometry( int nSlot,IGeometry *pFromGeom, bop_meshupdate *pLastUpdate=0 );
	void OnContactWithEntity( CEntity *pEntity );
	void OnCollision(CEntity *pTarget, int matId, const Vec3 &pt, const Vec3 &n, const Vec3 &vel, const Vec3 &targetVel, int partId, float mass);
	//////////////////////////////////////////////////////////////////////////

	void SetFlags( int nFlags ) { m_nFlags = nFlags; };
	uint32 GetFlags() const { return m_nFlags; };
	bool CheckFlags( uint32 nFlags ) const { return (m_nFlags&nFlags) == nFlags; }
	void UpdateSlotGeometry(int nSlot,IStatObj *pStatObjNew=0,float mass=-1.0f,int bNoSubslots=1);
	void AssignPhysicalEntity( IPhysicalEntity *pPhysEntity, int nSlot=-1 );

	VIRTUAL bool PhysicalizeFoliage( int iSlot );
	VIRTUAL void DephysicalizeFoliage( int iSlot );
	VIRTUAL IFoliage *GetFoliage( int iSlot );

	int AddSlotGeometry( int nSlot,SEntityPhysicalizeParams &params, int bNoSubslots=1 );
	void RemoveSlotGeometry( int nSlot );

	void SetPartId0(int partId0) { m_partId0 = partId0; }

	void MovePhysics(CPhysicalProxy *dstPhysics);

	virtual void GetMemoryUsage(ICrySizer *pSizer )const;
	void ReattachSoftEntityVtx( IPhysicalEntity *pAttachToEntity, int nAttachToPart );

# if !defined(_RELEASE)
  static void EnableValidation();
  static void DisableValidation();
# endif 
	
protected:
	IPhysicalWorld* PhysicalWorld() const { return gEnv->pPhysicalWorld; }
	void OnEntityXForm( SEntityEvent &event );
	void OnChangedPhysics( bool bEnabled );
	void DestroyPhysicalEntity( bool bDestroyCharacters=true, int iMode=0 );

	void PhysicalizeSimple( SEntityPhysicalizeParams &params );
	void PhysicalizeLiving( SEntityPhysicalizeParams &params );
	void PhysicalizeParticle( SEntityPhysicalizeParams &params );
	void PhysicalizeSoft( SEntityPhysicalizeParams &params );
	void AttachSoftVtx( IRenderMesh *pRM, IPhysicalEntity *pAttachToEntity, int nAttachToPart );
	void PhysicalizeArea( SEntityPhysicalizeParams &params );
	bool PhysicalizeGeomCache( SEntityPhysicalizeParams &params );
	bool PhysicalizeCharacter( SEntityPhysicalizeParams &params );
	bool ConvertCharacterToRagdoll( SEntityPhysicalizeParams &params, const Vec3 &velInitial );

	void CreatePhysicalEntity( SEntityPhysicalizeParams &params );
	phys_geometry* GetSlotGeometry( int nSlot );
	void SyncCharacterWithPhysics();
	
	void MoveChildPhysicsParts(IPhysicalEntity* pSrcAdam, CEntity* pChild, pe_action_move_parts& amp, uint64 usedRanges);

	//////////////////////////////////////////////////////////////////////////
	// Handle colliders.
	//////////////////////////////////////////////////////////////////////////
	void AddCollider( EntityId id );
	void RemoveCollider( EntityId id );
	void CheckColliders();
	//////////////////////////////////////////////////////////////////////////

	IPhysicalEntity* QueryPhyscalEntity( IEntity *pEntity ) const;
	CEntity* GetCEntity( IPhysicalEntity* pPhysEntity );

	void ReleasePhysicalEntity();
	void ReleaseColliders();

	uint32 m_nFlags;

	CEntity *m_pEntity;

	 // Pointer to physical object.
	IPhysicalEntity *m_pPhysicalEntity;

	//////////////////////////////////////////////////////////////////////////
	//! List of colliding entities, used by all triggers.
	//! When entity is first added to this list it is considered as entering
	//! to proximity, when it erased from it it is leaving proximity.
	typedef std::set<EntityId> ColliderSet;
	struct Colliders {
		IPhysicalEntity *m_pPhysicalBBox;  // Pointer to physical bounding box (optional).
		ColliderSet colliders;
	};
	Colliders *m_pColliders;
	int m_partId0;	// partid in physics = slot# + m_partsidOffs
	float m_timeLastSync;
};

DECLARE_COMPONENT_POINTERS( CPhysicalProxy );

#endif // __PhysicsProxy_h__


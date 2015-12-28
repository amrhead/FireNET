////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2005.
// -------------------------------------------------------------------------
//  File name:   BreakableManager.h
//  Version:     v1.00
//  Created:     7/6/2005 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __BreakableManager_h__
#define __BreakableManager_h__
#pragma once

#include "IBreakableManager.h"

class CEntitySystem;
class CEntity;
struct GeomRef;
//////////////////////////////////////////////////////////////////////////
//
// BreakableManager manager handles all the code for breaking/destroying entity geometry.
//
//////////////////////////////////////////////////////////////////////////
class CBreakableManager : public IBreakableManager
{
public:
	CBreakableManager( CEntitySystem *pEntitySystem );

	// actual breaking function.
	void BreakIntoPieces( GeomRef &geoOrig, const Matrix34 &srcObjTM, 
												IStatObj *pPiecesObj, const Matrix34 &piecesObjTM, 
												BreakageParams const& Breakage,int nMatLayers );

	//////////////////////////////////////////////////////////////////////////
	// IBreakableManager implementation
	//////////////////////////////////////////////////////////////////////////
	VIRTUAL void BreakIntoPieces( IEntity *pEntity, int nOrigSlot, int nPiecesSlot, BreakageParams const& Breakage );

	// Breakable Plane 
	VIRTUAL EProcessImpactResult ProcessPlaneImpact(const SProcessImpactIn& in, SProcessImpactOut& out);
	VIRTUAL void ExtractPlaneMeshIsland(const SExtractMeshIslandIn& in, SExtractMeshIslandOut& out);
	VIRTUAL void GetPlaneMemoryStatistics(void* pPlaneRaw, ICrySizer* pSizer);
	VIRTUAL bool IsGeometryBreakable(IPhysicalEntity* pEntity, IStatObj* pStatObj, IMaterial* pMaterial);

	// Check if this stat object can shatter.
	// Check if its materials support shattering.
	VIRTUAL bool CanShatter( IStatObj *pStatObj );
	VIRTUAL bool CanShatterEntity( IEntity *pEntity,int nSlot=-1 );

	// Attach the effect & params specified by material of object in slot.
	VIRTUAL void AttachSurfaceEffect( IEntity* pEntity, int nSlot, const char* sType, SpawnParams const& paramsIn, uint uEmitterFlags = 0 );
	virtual void CreateSurfaceEffect( IStatObj *pStatObj,const Matrix34 &tm,const char* sType );
	//////////////////////////////////////////////////////////////////////////

	bool CanShatterRenderMesh( IRenderMesh *pMesh,IMaterial *pMtl );

	VIRTUAL void AddBreakEventListener(IBreakEventListener * pListener);
	VIRTUAL void RemoveBreakEventListener(IBreakEventListener * pListener);

	VIRTUAL void EntityDrawSlot(CEntity * pEntity, int32 slot, int32 flags);

	VIRTUAL ISurfaceType* GetFirstSurfaceType( IStatObj *pStatObj );
	VIRTUAL ISurfaceType* GetFirstSurfaceType( ICharacterInstance *pCharacter );

	VIRTUAL void ReplayRemoveSubPartsEvent( const EventPhysRemoveEntityParts *pRemoveEvent );

	void HandlePhysicsCreateEntityPartEvent( const EventPhysCreateEntityPart *pCreateEvent );
	void HandlePhysicsRemoveSubPartsEvent( const EventPhysRemoveEntityParts *pRemoveEvent );
	void HandlePhysicsRevealSubPartEvent( const EventPhysRevealEntityPart *pRevealEvent );
	int  HandlePhysics_UpdateMeshEvent( const EventPhysUpdateMesh *pEvent );

	void FakePhysicsEvent( EventPhys * pEvent );
	
  // Resets broken objects
  void ResetBrokenObjects();

	// Freeze/unfreeze render node.
	void FreezeRenderNode( IRenderNode *pRenderNode,bool bEnable );

	void CreateObjectAsParticles( IStatObj *pStatObj,IPhysicalEntity *pPhysEnt, IBreakableManager::SCreateParams &createParams );
	VIRTUAL IEntity* CreateObjectAsEntity( IStatObj *pStatObj,IPhysicalEntity *pPhysEnt, IPhysicalEntity *pSrcPhysEnt, IBreakableManager::SCreateParams &createParams, bool bCreateSubstProxy=false );
	bool CheckForPieces(IStatObj *pStatObjSrc, IStatObj::SSubObject *pSubObj, const Matrix34 &worldTM, int nMatLayers, IPhysicalEntity *pPhysEnt);
	IStatObj::SSubObject *CheckSubObjBreak(IStatObj *pStatObj, IStatObj::SSubObject *pSubObj, const EventPhysCreateEntityPart *epcep);

	VIRTUAL const IBreakableManager::SBrokenObjRec * GetPartBrokenObjects(int& brokenObjectCount) { brokenObjectCount = m_brokenObjs.size(); return &m_brokenObjs[0]; }

	VIRTUAL void GetBrokenObjectIndicesForCloning(int32 * pPartRemovalIndices, int32& iNumPartRemovalIndices,
																								int32 * pOutIndiciesForCloning, int32& iNumEntitiesForCloning,
																								const EventPhysRemoveEntityParts * BreakEvents);
	VIRTUAL void ClonePartRemovedEntitiesByIndex(	int32 * pBrokenObjectIndices, int32 iNumBrokenObjectIndices,
																								EntityId * pOutClonedEntities, int32& iNumClonedBrokenEntities,
																								const EntityId * pRecordingEntities, int32 iNumRecordingEntities,
																								SRenderNodeCloneLookup& nodeLookup);

	VIRTUAL void HideBrokenObjectsByIndex( const int32 * pBrokenObjectIndices, const int32 iNumBrokenObjectIndices);
	VIRTUAL void UnhidePartRemovedObjectsByIndex( const int32 * pPartRemovalIndices, const int32 iNumPartRemovalIndices, const EventPhysRemoveEntityParts * pBreakEvents);
	VIRTUAL void ApplySinglePartRemovalFromEventIndex(int32 iPartRemovalEventIndex, const SRenderNodeCloneLookup& renderNodeLookup, const EventPhysRemoveEntityParts * pBreakEvents);
	VIRTUAL void ApplyPartRemovalsUntilIndexToObjectList( int32 iFirstEventIndex, const SRenderNodeCloneLookup& renderNodeLookup, const EventPhysRemoveEntityParts * pBreakEvents);

private:
	void CreateObjectCommon( IStatObj *pStatObj,IPhysicalEntity *pPhysEnt, IBreakableManager::SCreateParams &createParams );
	
	void ApplyPartBreakToClonedObjectFromEvent(const SRenderNodeCloneLookup& renderNodeLookup, const EventPhysRemoveEntityParts& OriginalEvent);

	// Remove Parts
	bool RemoveStatObjParts( IStatObj* &pStatObj );

	CEntitySystem *m_pEntitySystem;
	IBreakEventListener *m_pBreakEventListener;
  //////////////////////////////////////////////////////////////////////////

  std::vector<IBreakableManager::SBrokenObjRec> m_brokenObjs;
  std::vector<IPhysicalEntity*> m_brokenObjsParticles;
};

#endif //__BreakableManager_h__

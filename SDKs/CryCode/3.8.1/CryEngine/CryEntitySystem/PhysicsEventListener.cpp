////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   PhysicsEventListener.cpp
//  Version:     v1.00
//  Created:     18/8/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PhysicsEventListener.h"
#include "IBreakableManager.h"
#include "EntityObject.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "EntityCVars.h"
#include "BreakableManager.h"

#include <IPhysics.h>
#include <IParticles.h>
#include <ParticleParams.h>
#include "IAISystem.h"

#include <ICodeCheckpointMgr.h>

std::vector<CPhysicsEventListener::PhysVisAreaUpdate> CPhysicsEventListener::m_physVisAreaUpdateVector;

//////////////////////////////////////////////////////////////////////////
CPhysicsEventListener::CPhysicsEventListener( CEntitySystem *pEntitySystem,IPhysicalWorld *pPhysics )
{
	assert(pEntitySystem);
	assert(pPhysics);
	m_pEntitySystem = pEntitySystem;

	m_pPhysics = pPhysics;

	RegisterPhysicCallbacks();
}

//////////////////////////////////////////////////////////////////////////
CPhysicsEventListener::~CPhysicsEventListener()
{
	UnregisterPhysicCallbacks();

	m_pPhysics->SetPhysicsEventClient(NULL);
}

//////////////////////////////////////////////////////////////////////////
CEntity* CPhysicsEventListener::GetEntity( IPhysicalEntity *pPhysEntity )
{
	assert(pPhysEntity);
	CEntity *pEntity = (CEntity*)pPhysEntity->GetForeignData(PHYS_FOREIGN_ID_ENTITY);
	return pEntity;
}

//////////////////////////////////////////////////////////////////////////
CEntity* CPhysicsEventListener::GetEntity( void *pForeignData,int iForeignData )
{
	if (PHYS_FOREIGN_ID_ENTITY == iForeignData)
	{
		return (CEntity*)pForeignData;
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
int CPhysicsEventListener::OnPostStep( const EventPhys *pEvent )
{
	EventPhysPostStep *pPostStep = (EventPhysPostStep*)pEvent;
	CEntity *pCEntity = GetEntity(pPostStep->pForeignData,pPostStep->iForeignData);
	IRenderNode *pRndNode = 0;
	if (pCEntity)
	{
		CPhysicalProxy *pPhysProxy = pCEntity->GetPhysicalProxy();
		if (pPhysProxy)// && pPhysProxy->GetPhysicalEntity())
			pPhysProxy->OnPhysicsPostStep(pPostStep);
		pRndNode = pCEntity->GetRenderProxy();
	}
	else if (pPostStep->iForeignData == PHYS_FOREIGN_ID_ROPE)
	{
		IRopeRenderNode *pRenderNode = (IRopeRenderNode*)pPostStep->pForeignData;
		if (pRenderNode)
		{
			pRenderNode->OnPhysicsPostStep();
		}
		pRndNode = pRenderNode;
	}

	if (pRndNode)
	{
		pe_params_flags pf;
		int bInvisible=0,bFaraway=0;
		int bEnableOpt = CVar::es_UsePhysVisibilityChecks;
		float dist = (pRndNode->GetBBox().GetCenter()-GetISystem()->GetViewCamera().GetPosition()).len2();
		float maxDist = pPostStep->pEntity->GetType()!=PE_SOFT ? CVar::es_MaxPhysDist:CVar::es_MaxPhysDistCloth;
		bInvisible = bEnableOpt & (isneg(pRndNode->GetDrawFrame()+10-gEnv->pRenderer->GetFrameID()) | isneg(sqr(maxDist)-dist));
		if (pRndNode->m_nInternalFlags & IRenderNode::WAS_INVISIBLE ^ (-bInvisible & IRenderNode::WAS_INVISIBLE))
		{
			pf.flagsAND = ~pef_invisible;
			pf.flagsOR = -bInvisible & pef_invisible;
			pPostStep->pEntity->SetParams(&pf);
			(pRndNode->m_nInternalFlags &= ~IRenderNode::WAS_INVISIBLE) |= -bInvisible & IRenderNode::WAS_INVISIBLE;
		}
		if (gEnv->p3DEngine->GetWaterLevel()!=WATER_LEVEL_UNKNOWN)
		{
			// Deferred updating ignore ocean flag as the Jobs are busy updating the octree at this point
			m_physVisAreaUpdateVector.push_back(PhysVisAreaUpdate(pRndNode, pPostStep->pEntity));
		}
		bFaraway = bEnableOpt & isneg(sqr(maxDist)+
			bInvisible*(sqr(CVar::es_MaxPhysDistInvisible)-sqr(maxDist)) - dist);
		if (bFaraway && !(pRndNode->m_nInternalFlags & IRenderNode::WAS_FARAWAY)) 
		{
			pe_params_foreign_data pfd;
			pPostStep->pEntity->GetParams(&pfd);
			bFaraway &= -(pfd.iForeignFlags & PFF_UNIMPORTANT)>>31;
		}
		if ((-bFaraway & IRenderNode::WAS_FARAWAY) != (pRndNode->m_nInternalFlags & IRenderNode::WAS_FARAWAY))
		{
			pe_params_timeout pto;
			pto.timeIdle = 0;
			pto.maxTimeIdle = bFaraway*CVar::es_FarPhysTimeout;
			pPostStep->pEntity->SetParams(&pto);
			(pRndNode->m_nInternalFlags &= ~IRenderNode::WAS_FARAWAY) |= -bFaraway & IRenderNode::WAS_FARAWAY;
		}
	}

	return 1;
}

int CPhysicsEventListener::OnPostPump( const EventPhys *pEvent )
{
	if (gEnv->p3DEngine->GetWaterLevel()!=WATER_LEVEL_UNKNOWN)
	{
		for (std::vector<PhysVisAreaUpdate>::iterator it=m_physVisAreaUpdateVector.begin(), end=m_physVisAreaUpdateVector.end(); it!=end; ++it)
		{
			IRenderNode *pRndNode=it->m_pRndNode;
			int bInsideVisarea = pRndNode->GetEntityVisArea()!=0;
			if (pRndNode->m_nInternalFlags & IRenderNode::WAS_IN_VISAREA ^ (-bInsideVisarea & IRenderNode::WAS_IN_VISAREA))
			{
				pe_params_flags pf;
				pf.flagsAND = ~pef_ignore_ocean;
				pf.flagsOR = -bInsideVisarea & pef_ignore_ocean;
				it->m_pEntity->SetParams(&pf);
				(pRndNode->m_nInternalFlags &= ~IRenderNode::WAS_IN_VISAREA) |= -bInsideVisarea & IRenderNode::WAS_IN_VISAREA;
			}
		}
		m_physVisAreaUpdateVector.clear();
	}
	return 1;
}

//////////////////////////////////////////////////////////////////////////
int CPhysicsEventListener::OnBBoxOverlap( const EventPhys *pEvent )
{
	EventPhysBBoxOverlap *pOverlap = (EventPhysBBoxOverlap*)pEvent;

	CEntity *pCEntity = GetEntity(pOverlap->pForeignData[0],pOverlap->iForeignData[0]);
	CEntity *pCEntityTrg = GetEntity(pOverlap->pForeignData[1],pOverlap->iForeignData[1]);
	if (pCEntity && pCEntityTrg)
	{
		CPhysicalProxy *pPhysProxySrc = pCEntity->GetPhysicalProxy();
		if (pPhysProxySrc)
			pPhysProxySrc->OnContactWithEntity( pCEntityTrg );

		CPhysicalProxy *pPhysProxyTrg = pCEntityTrg->GetPhysicalProxy();
		if (pPhysProxyTrg)
			pPhysProxyTrg->OnContactWithEntity( pCEntity );

	}
	return 1;
}

//////////////////////////////////////////////////////////////////////////
int CPhysicsEventListener::OnStateChange( const EventPhys *pEvent )
{
	EventPhysStateChange *pStateChange = (EventPhysStateChange*)pEvent;
	CEntity *pCEntity = GetEntity(pStateChange->pForeignData,pStateChange->iForeignData);
	if (pCEntity)
	{
		EEntityUpdatePolicy policy = (EEntityUpdatePolicy)pCEntity->m_eUpdatePolicy;
		// If its update depends on physics, physics state defines if this entity is to be updated.
		if (policy == ENTITY_UPDATE_PHYSICS || policy == ENTITY_UPDATE_PHYSICS_VISIBLE)
		{
			int nNewSymClass = pStateChange->iSimClass[1];
//			int nOldSymClass = pStateChange->iSimClass[0];
			if (nNewSymClass == SC_ACTIVE_RIGID)
			{
				// Should activate entity if physics is awaken.
				pCEntity->Activate(true);
			}
			else if (nNewSymClass == SC_SLEEPING_RIGID)
			{
				// Entity must go to sleep.
				pCEntity->Activate(false);
				//CallStateFunction(ScriptState_OnStopRollSlideContact, "roll");
				//CallStateFunction(ScriptState_OnStopRollSlideContact, "slide");
			}
		}
		int nOldSymClass = pStateChange->iSimClass[0];
		if (nOldSymClass == SC_ACTIVE_RIGID)
		{
			SEntityEvent event(ENTITY_EVENT_PHYSICS_CHANGE_STATE);
			event.nParam[0] = 1;
			pCEntity->SendEvent(event);
			if (pStateChange->timeIdle>=CVar::es_FarPhysTimeout)
			{
				pe_status_dynamics sd;
				if (pStateChange->pEntity->GetStatus(&sd) && sd.submergedFraction>0)
				{
					pCEntity->SetFlags(pCEntity->GetFlags() | ENTITY_FLAG_SEND_RENDER_EVENT);
					pCEntity->GetPhysicalProxy()->SetFlags(pCEntity->GetPhysicalProxy()->GetFlags() | CPhysicalProxy::FLAG_PHYS_AWAKE_WHEN_VISIBLE);
				}
			}
		}
		else if (nOldSymClass == SC_SLEEPING_RIGID)
		{
			SEntityEvent event(ENTITY_EVENT_PHYSICS_CHANGE_STATE);
			event.nParam[0] = 0;
			pCEntity->SendEvent(event);
		}
	}
	return 1;
}

//////////////////////////////////////////////////////////////////////////
#include <IDeferredCollisionEvent.h>

class CDeferredMeshUpdatePrep : public IDeferredPhysicsEvent 
{
public:
	CDeferredMeshUpdatePrep() { m_status=0; m_pStatObj=0; m_id=-1; m_pMesh=0; }
	IStatObj *m_pStatObj;
	int m_id;
	IGeometry *m_pMesh,*m_pMeshSkel;
	Matrix34 m_mtxSkelToMesh;
	volatile int m_status;

	virtual void Start() {}
	virtual int Result(EventPhys*) { return 0; }
	virtual void Sync() {}
	virtual bool HasFinished() { return m_status>1; }
	virtual DeferredEventType GetType() const { return PhysCallBack_OnCollision; }
	virtual EventPhys* PhysicsEvent() { return 0; }

	virtual void OnUpdate() { 
		if (m_id) 
			m_pStatObj->GetIndexedMesh(true); 
		else
		{
			mesh_data *md = (mesh_data*)m_pMeshSkel->GetData();
			IStatObj *pDeformedStatObj = m_pStatObj->SkinVertices(md->pVertices, m_mtxSkelToMesh);
			m_pMesh->SetForeignData(pDeformedStatObj,0);
		}
		m_pStatObj->Release(); 
		if (m_threadTaskInfo.m_pThread)
			gEnv->pSystem->GetIThreadTaskManager()->UnregisterTask(this);
		m_status = 2; 
	}
	virtual void Stop() {}

	virtual SThreadTaskInfo* GetTaskInfo() { return &m_threadTaskInfo; }
	SThreadTaskInfo m_threadTaskInfo;	
};

int g_lastReceivedEventId=1,g_lastExecutedEventId=1;
CDeferredMeshUpdatePrep g_meshPreps[16];

int CPhysicsEventListener::OnPreUpdateMesh( const EventPhys *pEvent )
{
	EventPhysUpdateMesh *pepum = (EventPhysUpdateMesh*)pEvent;
	IStatObj *pStatObj;
	if (pepum->iReason==EventPhysUpdateMesh::ReasonDeform || !(pStatObj=(IStatObj*)pepum->pMesh->GetForeignData()))
		return 1;

	if (pepum->idx<0)
		pepum->idx = pepum->iReason==EventPhysUpdateMesh::ReasonDeform ? 0:++g_lastReceivedEventId;

	bool bDefer = false;
	if (g_lastExecutedEventId < pepum->idx-1)
		bDefer = true;
	else {
		int i,j;
		for(i=sizeof(g_meshPreps)/sizeof(g_meshPreps[0])-1,j=-1; 
				i>=0 && (!g_meshPreps[i].m_status || (pepum->idx ? (g_meshPreps[i].m_id!=pepum->idx) : (g_meshPreps[i].m_pMesh!=pepum->pMesh))); i--)
			j += i+1 & (g_meshPreps[i].m_status-1 & j)>>31;
		if (i>=0)
		{
			if (g_meshPreps[i].m_status==2)
				g_meshPreps[i].m_status = 0;
			else
				bDefer = true;
		}	else if (pepum->iReason==EventPhysUpdateMesh::ReasonDeform || !pStatObj->GetIndexedMesh(false))
		{
			if (j>=0)
			{	
				(g_meshPreps[j].m_pStatObj = pStatObj)->AddRef();
				g_meshPreps[j].m_pMesh = pepum->pMesh;
				g_meshPreps[j].m_pMeshSkel = pepum->pMeshSkel;
				g_meshPreps[j].m_mtxSkelToMesh = pepum->mtxSkelToMesh;
				g_meshPreps[j].m_id = pepum->idx;
				g_meshPreps[j].m_status = 1;
				gEnv->p3DEngine->GetDeferredPhysicsEventManager()->DispatchDeferredEvent(&g_meshPreps[j]); 
			}
			bDefer = true;
		}
	}
	if (bDefer)
	{
		gEnv->pPhysicalWorld->AddDeferredEvent(EventPhysUpdateMesh::id, pepum);
		return 0;
	}

	if (pepum->idx>0)
		g_lastExecutedEventId = pepum->idx;
	return 1;
};

int CPhysicsEventListener::OnPreCreatePhysEntityPart( const EventPhys *pEvent )
{
	EventPhysCreateEntityPart *pepcep = (EventPhysCreateEntityPart*)pEvent;
	if (!pepcep->pMeshNew)
		return 1;
	if (pepcep->idx<0)
		pepcep->idx = ++g_lastReceivedEventId;
	if (g_lastExecutedEventId < pepcep->idx-1)
	{
		gEnv->pPhysicalWorld->AddDeferredEvent(EventPhysCreateEntityPart::id, pepcep);
		return 0;
	}
	g_lastExecutedEventId = pepcep->idx;
	return 1;
}


//////////////////////////////////////////////////////////////////////////
int CPhysicsEventListener::OnUpdateMesh( const EventPhys *pEvent )
{
	((CBreakableManager*)GetIEntitySystem()->GetBreakableManager())->HandlePhysics_UpdateMeshEvent((EventPhysUpdateMesh*)pEvent);
	return 1;

	/*EventPhysUpdateMesh *pUpdateEvent = (EventPhysUpdateMesh*)pEvent;
	CEntity *pCEntity;
	Matrix34 mtx;
	int iForeignData = pUpdateEvent->pEntity->GetiForeignData();
	void *pForeignData = pUpdateEvent->pEntity->GetForeignData(iForeignData);
	IFoliage *pSrcFoliage=0;
	uint8 nMatLayers = 0;

	bool bNewEntity = false;

	if (iForeignData==PHYS_FOREIGN_ID_ENTITY)
	{
		pCEntity = (CEntity*)pForeignData;
		if (!pCEntity || !pCEntity->GetPhysicalProxy())
			return 1;
		if (pCEntity->GetRenderProxy())
		{
			nMatLayers = pCEntity->GetRenderProxy()->GetMaterialLayers();
			pCEntity->GetRenderProxy()->ExpandCompoundSlot0();
		}
	} else if (iForeignData==PHYS_FOREIGN_ID_STATIC)
	{
		CBreakableManager::SCreateParams createParams;

		CBreakableManager* pBreakMgr = (CBreakableManager*)GetIEntitySystem()->GetBreakableManager();

		IRenderNode *pRenderNode = (IRenderNode*)pUpdateEvent->pForeignData;
		IStatObj *pStatObj = pRenderNode->GetEntityStatObj(0, 0, &mtx);
		
		createParams.fScale = mtx.GetColumn(0).len();
		createParams.pSrcStaticRenderNode = pRenderNode;
		createParams.worldTM = mtx;
		createParams.nMatLayers = pRenderNode->GetMaterialLayers();
		createParams.pCustomMtl = pRenderNode->GetMaterial();

		if (pStatObj)
		{
			pCEntity = pBreakMgr->CreateObjectAsEntity( pStatObj,pUpdateEvent->pEntity,createParams );
			bNewEntity = true;
		}
		
		pSrcFoliage = pRenderNode->GetFoliage();
	}	else
	{
		return 1;
	}

	//if (pUpdateEvent->iReason==EventPhysUpdateMesh::ReasonExplosion)
	//	pCEntity->AddFlags(ENTITY_FLAG_MODIFIED_BY_PHYSICS);

	if (pCEntity)
	{
		pCEntity->GetPhysicalProxy()->DephysicalizeFoliage(0);
		IStatObj *pDeformedStatObj = gEnv->p3DEngine->UpdateDeformableStatObj(pUpdateEvent->pMesh,pUpdateEvent->pLastUpdate,pSrcFoliage);
		pCEntity->GetRenderProxy()->SetSlotGeometry( pUpdateEvent->partid,pDeformedStatObj );

		//pCEntity->GetPhysicalProxy()->CreateRenderGeometry( 0,pUpdateEvent->pMesh, pUpdateEvent->pLastUpdate );
		pCEntity->GetPhysicalProxy()->PhysicalizeFoliage(0);
		
		if (bNewEntity)
		{
			SEntityEvent entityEvent(ENTITY_EVENT_PHYS_BREAK);
			pCEntity->SendEvent( entityEvent );
		}
	}
	
	return 1;*/
}

//////////////////////////////////////////////////////////////////////////
int CPhysicsEventListener::OnCreatePhysEntityPart( const EventPhys *pEvent )
{
	EventPhysCreateEntityPart *pCreateEvent = (EventPhysCreateEntityPart*)pEvent;

	//////////////////////////////////////////////////////////////////////////
	// Let Breakable manager handle creation of the new entity part.
	//////////////////////////////////////////////////////////////////////////
	CBreakableManager* pBreakMgr = (CBreakableManager*)GetIEntitySystem()->GetBreakableManager();
	pBreakMgr->HandlePhysicsCreateEntityPartEvent( pCreateEvent );
	return 1;


	/*if (pCreateEvent->pMeshNew)
		pPhysProxy->CreateRenderGeometry( 0,pCreateEvent->pMeshNew, pCreateEvent->pLastUpdate );
	else 
	{
		int i,nSrcParts,nParts,nSuccParts=0,nSubobj,*slots,slotsbuf[16];
		IStatObj *pSrcStatObj,*pStatObj,*pStatObjDummy;
		pe_params_part pp,pp1;

		nParts = pCreateEvent->pEntNew->GetStatus(&pe_status_nparts());
		nSubobj = pSrcEntity->GetStatObj(0)->GetSubObjectCount();
		slots = nSubobj<=sizeof(slots)/sizeof(slots[0]) ? slotsbuf : new int[nSubobj];
		for(pp.ipart=0; pp.ipart<nParts; pp.ipart++) 
		{
			pCreateEvent->pEntNew->GetParams(&pp);
			slots[pp.ipart] = pp.partid;
			pp1.ipart = pp1.partid = pp.ipart;
			pCreateEvent->pEntNew->SetParams(&pp1);
		}

		pSrcStatObj = pSrcEntity->GetStatObj(0)->SeparateSubobjects(pStatObj, slots,nParts, false);

		if (pCreateEvent->partidSrc) 
		{ // the last removed part in the session, flush removed parts from the source statobj and update phys ids
			nSubobj = pSrcStatObj->GetSubObjectCount();
			for(i=nParts=0;i<nSubobj;i++) if (pSrcStatObj->GetSubObject(i)->nType==STATIC_SUB_OBJECT_MESH)
				slots[i] = nParts++;
			nSrcParts = pCreateEvent->pEntity->GetStatus(&pe_status_nparts());
			for(pp.ipart=i=0; pp.ipart<nSrcParts; pp.ipart++)
			{
				pCreateEvent->pEntity->GetParams(&pp);
				pp1.ipart = pp.ipart; pp1.partid = slots[pp.partid];
				pCreateEvent->pEntity->SetParams(&pp1);
			}
			pSrcStatObj = pSrcStatObj->SeparateSubobjects(pStatObjDummy, 0,0, true);
		}

		pSrcEntity->SetStatObj( pSrcStatObj,0 );
		pEntity->SetStatObj( pStatObj,0 );

		if (slots!=slotsbuf)
			delete slots;
	}*/


	//if (bNewEnt && pEntity->GetPhysicalProxy()->PhysicalizeFoliage(0) && pSrcEntity)
		//pSrcEntity->GetPhysicalProxy()->DephysicalizeFoliage(0);

	return 1;
}

//////////////////////////////////////////////////////////////////////////
int CPhysicsEventListener::OnRemovePhysEntityParts( const EventPhys *pEvent )
{
	EventPhysRemoveEntityParts *pRemoveEvent = (EventPhysRemoveEntityParts*)pEvent;

	CBreakableManager* pBreakMgr = (CBreakableManager*)GetIEntitySystem()->GetBreakableManager();
	pBreakMgr->HandlePhysicsRemoveSubPartsEvent( pRemoveEvent );

	return 1;
}

//////////////////////////////////////////////////////////////////////////
int CPhysicsEventListener::OnRevealPhysEntityPart( const EventPhys *pEvent )
{
	EventPhysRevealEntityPart *pRevealEvent = (EventPhysRevealEntityPart*)pEvent;

	CBreakableManager* pBreakMgr = (CBreakableManager*)GetIEntitySystem()->GetBreakableManager();
	pBreakMgr->HandlePhysicsRevealSubPartEvent( pRevealEvent );

	return 1;
}

//////////////////////////////////////////////////////////////////////////
CEntity *MatchPartId(CEntity *pent,int partid)
{
	if (pent->GetPhysicalProxy() && (unsigned int)(partid-pent->GetPhysicalProxy()->GetPartId0())<1000u)
		return pent;
	CEntity *pMatch;
	for(int i=pent->GetChildCount()-1;i>=0;i--)	if (pMatch=MatchPartId((CEntity*)pent->GetChild(i),partid))
		return pMatch;
	return 0;
}

IEntity *CEntity::UnmapAttachedChild(int &partId)
{
	CEntity *pChild;
	if (partId>=PARTID_LINKED && (pChild=MatchPartId(this,partId)))
	{
		partId -= pChild->GetPhysicalProxy()->GetPartId0();
		return pChild;
	}
	return this;
}

int CPhysicsEventListener::OnCollision( const EventPhys *pEvent )
{
	EventPhysCollision *pCollision = (EventPhysCollision *)pEvent;
	SEntityEvent event;
	CEntity *pEntitySrc = GetEntity(pCollision->pForeignData[0], pCollision->iForeignData[0]), *pChild,
					*pEntityTrg = GetEntity(pCollision->pForeignData[1], pCollision->iForeignData[1]);

 	if (pEntitySrc && pCollision->partid[0]>=PARTID_LINKED && (pChild=MatchPartId(pEntitySrc,pCollision->partid[0])))
	{
		pEntitySrc = pChild;
		pCollision->pForeignData[0] = pEntitySrc;
		pCollision->iForeignData[0] = PHYS_FOREIGN_ID_ENTITY;
		pCollision->partid[0] -= pEntitySrc->GetPhysicalProxy()->GetPartId0();
	}

	if (pEntitySrc)
	{
 		if (pEntityTrg && pCollision->partid[1]>=PARTID_LINKED && (pChild=MatchPartId(pEntityTrg,pCollision->partid[1])))
		{
			pEntityTrg = pChild;
			pCollision->pForeignData[1] = pEntityTrg;
			pCollision->iForeignData[1] = PHYS_FOREIGN_ID_ENTITY;
			pCollision->partid[1] -= pEntityTrg->GetPhysicalProxy()->GetPartId0();
		}

		CPhysicalProxy *pPhysProxySrc = pEntitySrc->GetPhysicalProxy();
		if (pPhysProxySrc)
			pPhysProxySrc->OnCollision(pEntityTrg, pCollision->idmat[1], pCollision->pt, pCollision->n, pCollision->vloc[0], pCollision->vloc[1], pCollision->partid[1], pCollision->mass[1]);

		if (pEntityTrg)
		{
			CPhysicalProxy *pPhysProxyTrg = pEntityTrg->GetPhysicalProxy();
			if (pPhysProxyTrg)
				pPhysProxyTrg->OnCollision(pEntitySrc, pCollision->idmat[0], pCollision->pt, -pCollision->n, pCollision->vloc[1], pCollision->vloc[0], pCollision->partid[0], pCollision->mass[0]);
		}
	}

	event.event = ENTITY_EVENT_COLLISION;
	event.nParam[0] = (INT_PTR)pEvent;
	event.nParam[1] = 0;
	if (pEntitySrc)
		pEntitySrc->SendEvent(event);
	event.nParam[1] = 1;
	if (pEntityTrg)
		pEntityTrg->SendEvent(event);

	return 1;
}

//////////////////////////////////////////////////////////////////////////
int CPhysicsEventListener::OnJointBreak( const EventPhys *pEvent )
{
	EventPhysJointBroken *pBreakEvent = (EventPhysJointBroken*)pEvent;
	CEntity *pCEntity = 0;
	IStatObj *pStatObj = 0;
	IRenderNode *pRenderNode = 0;
	Matrix34A nodeTM;

	// Counter for feature test setup
	CODECHECKPOINT(physics_on_joint_break);

	bool bShatter = false;
	switch (pBreakEvent->iForeignData[0])
	{
	case PHYS_FOREIGN_ID_ROPE:
		{
			IRopeRenderNode *pRopeRenderNode = (IRopeRenderNode*)pBreakEvent->pForeignData[0];
			if (pRopeRenderNode)
			{
				EntityId id = (EntityId)pRopeRenderNode->GetEntityOwner();
				pCEntity = (CEntity*)g_pIEntitySystem->GetEntityFromID(id);
			}
		}
		break;
	case PHYS_FOREIGN_ID_ENTITY:
		pCEntity = (CEntity*)pBreakEvent->pForeignData[0];
		break;
	case PHYS_FOREIGN_ID_STATIC:
		{
			pRenderNode = ((IRenderNode*)pBreakEvent->pForeignData[0]);
			pStatObj = pRenderNode->GetEntityStatObj(0,0,&nodeTM);
			bShatter = pRenderNode->GetMaterialLayers() & MTL_LAYER_FROZEN;
		}
	}
	//GetEntity(pBreakEvent->pForeignData[0],pBreakEvent->iForeignData[0]);
	if (pCEntity)
	{
		SEntityEvent event;
		event.event = ENTITY_EVENT_PHYS_BREAK;
		event.nParam[0] = (INT_PTR)pEvent;
		event.nParam[1] = 0;
		pCEntity->SendEvent(event);
		pStatObj = pCEntity->GetStatObj(ENTITY_SLOT_ACTUAL);

		if (pCEntity->GetRenderProxy())
		{
			bShatter = pCEntity->GetRenderProxy()->GetMaterialLayersMask() & MTL_LAYER_FROZEN;
		}
	}

	IStatObj *pStatObjEnt = pStatObj;
	if (pStatObj)
		while(pStatObj->GetCloneSourceObject())
			pStatObj = pStatObj->GetCloneSourceObject();

	if (pStatObj && pStatObj->GetFlags()&STATIC_OBJECT_COMPOUND)
	{
		Matrix34 tm = Matrix34::CreateTranslationMat(pBreakEvent->pt);
		IStatObj *pObj1=0;
//		IStatObj *pObj2=0;
		Vec3 axisx = pBreakEvent->n.GetOrthogonal().normalized();
		tm.SetRotation33(Matrix33(axisx,pBreakEvent->n^axisx,pBreakEvent->n).T());

		IStatObj::SSubObject *pSubObject1 = pStatObj->GetSubObject(pBreakEvent->partid[0]);
		if (pSubObject1)
			pObj1 = pSubObject1->pStatObj;
		//IStatObj::SSubObject *pSubObject2 = pStatObj->GetSubObject(pBreakEvent->partid[1]);
		//if (pSubObject2)
			//pObj2 = pSubObject2->pStatObj;

		const char *sEffectType = (!bShatter) ? SURFACE_BREAKAGE_TYPE("joint_break") : SURFACE_BREAKAGE_TYPE("joint_shatter");
		CBreakableManager* pBreakMgr = (CBreakableManager*)GetIEntitySystem()->GetBreakableManager();
		if (pObj1)
			pBreakMgr->CreateSurfaceEffect( pObj1,tm,sEffectType );
		//if (pObj2)
			//pBreakMgr->CreateSurfaceEffect( pObj2,tm,sEffectType );
	}

	IStatObj::SSubObject *pSubObj;

	if (pStatObj && (pSubObj=pStatObj->GetSubObject(pBreakEvent->idJoint)) && 
			pSubObj->nType==STATIC_SUB_OBJECT_DUMMY && !strncmp(pSubObj->name,"$joint",6))
	{
		const char *ptr;

		if (ptr=strstr(pSubObj->properties,"effect"))
		{
		
			for (ptr += 6; *ptr && (*ptr == ' ' || *ptr == '='); ptr++);
			if (*ptr)
			{
				char strEff[256];
				const char* const peff = ptr;
				while (*ptr && *ptr != '\n') ++ptr;
				cry_strcpy(strEff, peff, (size_t)(ptr - peff));
				IParticleEffect* pEffect = gEnv->pParticleManager->FindEffect(strEff);
				pEffect->Spawn(true, IParticleEffect::ParticleLoc(pBreakEvent->pt, pBreakEvent->n));
			}
		}

		if (ptr=strstr(pSubObj->properties,"breaker"))
		{
			if (!(pStatObjEnt->GetFlags() & STATIC_OBJECT_GENERATED))
			{
				pStatObjEnt = pStatObjEnt->Clone(false,false,true);
				pStatObjEnt->SetFlags( pStatObjEnt->GetFlags()|STATIC_OBJECT_GENERATED );
				if (pCEntity)
					pCEntity->SetStatObj(pStatObjEnt,ENTITY_SLOT_ACTUAL,false);
				else if (pRenderNode)
				{
					IBreakableManager::SCreateParams createParams;
					createParams.pSrcStaticRenderNode = pRenderNode;
					createParams.fScale = nodeTM.GetColumn(0).len();
					createParams.nSlotIndex = 0;
					createParams.worldTM = nodeTM;
					createParams.nMatLayers = pRenderNode->GetMaterialLayers();
					createParams.nRenderNodeFlags = pRenderNode->GetRndFlags();
					createParams.pCustomMtl = pRenderNode->GetMaterial();
					createParams.nEntityFlagsAdd = ENTITY_FLAG_MODIFIED_BY_PHYSICS;
					createParams.pName = pRenderNode->GetName();
					((CBreakableManager*)GetIEntitySystem()->GetBreakableManager())->CreateObjectAsEntity( 
						pStatObjEnt,pBreakEvent->pEntity[0], pBreakEvent->pEntity[0], createParams,true );
				}
			}
			
			IStatObj::SSubObject* pSubObj1;
			const char* piecesStr;
			for (int i = 0; i < 2; i++)
				if ((pSubObj = pStatObjEnt->GetSubObject(pBreakEvent->partid[i])) && pSubObj->pStatObj &&
						(piecesStr = strstr(pSubObj->properties, "pieces=")) && (pSubObj1 = pStatObj->FindSubObject(piecesStr + 7)) &&
						pSubObj1->pStatObj)
				{
					pSubObj->pStatObj->Release();
					(pSubObj->pStatObj = pSubObj1->pStatObj)->AddRef();
				}
			pStatObjEnt->Invalidate(false);
		}
	}

	return 1;
}

void CPhysicsEventListener::RegisterPhysicCallbacks()
{
	if (m_pPhysics)
	{
		m_pPhysics->AddEventClient( EventPhysStateChange::id,OnStateChange,1 );
		m_pPhysics->AddEventClient( EventPhysBBoxOverlap::id,OnBBoxOverlap,1 );
		m_pPhysics->AddEventClient( EventPhysPostStep::id,OnPostStep,1 );
		m_pPhysics->AddEventClient( EventPhysUpdateMesh::id,OnUpdateMesh,1 );
		m_pPhysics->AddEventClient( EventPhysUpdateMesh::id,OnUpdateMesh,0 );
		m_pPhysics->AddEventClient( EventPhysCreateEntityPart::id,OnCreatePhysEntityPart,1 );
		m_pPhysics->AddEventClient( EventPhysCreateEntityPart::id,OnCreatePhysEntityPart,0 );
		m_pPhysics->AddEventClient( EventPhysRemoveEntityParts::id,OnRemovePhysEntityParts,1 );
		m_pPhysics->AddEventClient( EventPhysRevealEntityPart::id,OnRevealPhysEntityPart,1 );
		m_pPhysics->AddEventClient( EventPhysCollision::id,OnCollision,1,1000.0f );
		m_pPhysics->AddEventClient( EventPhysJointBroken::id,OnJointBreak,1 );
		m_pPhysics->AddEventClient( EventPhysPostPump::id,OnPostPump,1 );
	}
}

void CPhysicsEventListener::UnregisterPhysicCallbacks()
{
	if (m_pPhysics)
	{
		m_pPhysics->RemoveEventClient( EventPhysStateChange::id,OnStateChange,1 );
		m_pPhysics->RemoveEventClient( EventPhysBBoxOverlap::id,OnBBoxOverlap,1 );
		m_pPhysics->RemoveEventClient( EventPhysPostStep::id,OnPostStep,1 );
		m_pPhysics->RemoveEventClient( EventPhysUpdateMesh::id,OnUpdateMesh,1 );
		m_pPhysics->RemoveEventClient( EventPhysUpdateMesh::id,OnUpdateMesh,0 );
		m_pPhysics->RemoveEventClient( EventPhysCreateEntityPart::id,OnCreatePhysEntityPart,1 );
		m_pPhysics->RemoveEventClient( EventPhysCreateEntityPart::id,OnCreatePhysEntityPart,0 );
		m_pPhysics->RemoveEventClient( EventPhysRemoveEntityParts::id,OnRemovePhysEntityParts,1 );
		m_pPhysics->RemoveEventClient( EventPhysRevealEntityPart::id,OnRevealPhysEntityPart,1 );
		m_pPhysics->RemoveEventClient( EventPhysCollision::id,OnCollision,1 );
		m_pPhysics->RemoveEventClient( EventPhysJointBroken::id,OnJointBreak,1 );
		m_pPhysics->RemoveEventClient( EventPhysPostPump::id,OnPostPump,1 );
		stl::free_container(m_physVisAreaUpdateVector);
	}
}

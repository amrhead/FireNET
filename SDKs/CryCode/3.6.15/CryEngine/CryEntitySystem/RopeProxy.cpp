////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   RopeProxy.cpp
//  Version:     v1.00
//  Created:     25/5/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RopeProxy.h"
#include "Entity.h"
#include "EntityObject.h"
#include "EntitySystem.h"
#include "ISerialize.h"

//////////////////////////////////////////////////////////////////////////
CRopeProxy::CRopeProxy()
	:	m_pEntity(NULL)
	, m_pRopeRenderNode(NULL)
{
}

//////////////////////////////////////////////////////////////////////////
CRopeProxy::~CRopeProxy()
{
}

//////////////////////////////////////////////////////////////////////////
void CRopeProxy::Initialize( const SComponentInitializer& init )
{
	m_pEntity = (CEntity*)init.m_pEntity;
	m_pRopeRenderNode = (IRopeRenderNode*)gEnv->p3DEngine->CreateRenderNode(eERType_Rope);
	m_pRopeRenderNode->SetEntityOwner( m_pEntity->GetId() );
	m_nSegmentsOrg = -1;
}

//////////////////////////////////////////////////////////////////////////
void CRopeProxy::Reload( IEntity *pEntity,SEntitySpawnParams &params )
{
	m_pEntity = (CEntity*)pEntity;

	assert(m_pRopeRenderNode);
	if (m_pRopeRenderNode)
	{
		m_pRopeRenderNode->SetEntityOwner( pEntity->GetId() );
		m_pRopeRenderNode->SetMatrix( pEntity->GetWorldTM() );

		if (pEntity->IsHidden())
		{
			m_pRopeRenderNode->SetRndFlags( m_pRopeRenderNode->GetRndFlags() | ERF_HIDDEN );
		}
		else
		{	
			m_pRopeRenderNode->SetRndFlags( m_pRopeRenderNode->GetRndFlags() & (~ERF_HIDDEN) );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CRopeProxy::Release()
{
	// Delete physical entity from physical world.
	if (m_pRopeRenderNode)
	{
		gEnv->p3DEngine->DeleteRenderNode(m_pRopeRenderNode);
		m_pRopeRenderNode = 0;
	}
	delete this;
}

//////////////////////////////////////////////////////////////////////////
void CRopeProxy::Update( SEntityUpdateContext &ctx )
{
}

//////////////////////////////////////////////////////////////////////////
void CRopeProxy::ProcessEvent( SEntityEvent &event )
{
	switch (event.event)
	{
	case ENTITY_EVENT_XFORM:
#ifndef SEG_WORLD
		if (m_pRopeRenderNode)
			m_pRopeRenderNode->SetMatrix( m_pEntity->GetWorldTM() );
#endif
		break;
	case ENTITY_EVENT_HIDE:
		if (m_pRopeRenderNode)
		{
			m_pRopeRenderNode->SetRndFlags( m_pRopeRenderNode->GetRndFlags() | ERF_HIDDEN );
			if (m_pRopeRenderNode->GetPhysics())
				gEnv->pPhysicalWorld->DestroyPhysicalEntity(m_pRopeRenderNode->GetPhysics(), 1);
		}
		break;
	case ENTITY_EVENT_UNHIDE:
		if (m_pRopeRenderNode)
		{
			m_pRopeRenderNode->SetRndFlags( m_pRopeRenderNode->GetRndFlags() & (~ERF_HIDDEN) );
			if (m_pRopeRenderNode->GetPhysics())
				gEnv->pPhysicalWorld->DestroyPhysicalEntity(m_pRopeRenderNode->GetPhysics(), 2);
		}
		break;
	case ENTITY_EVENT_ATTACH:	
		break;
	case ENTITY_EVENT_DETACH:
		break;
	case ENTITY_EVENT_COLLISION:
		break;
	case ENTITY_EVENT_LEVEL_LOADED:
		// Relink physics.
		if (m_pRopeRenderNode)
			m_pRopeRenderNode->LinkEndPoints();
		break;
	case ENTITY_EVENT_MATERIAL:
		if (m_pRopeRenderNode)
		{
			IMaterial *pMtl = (IMaterial*)(event.nParam[0]);
			m_pRopeRenderNode->SetMaterial( pMtl );
		}
		break;
	case ENTITY_EVENT_RESET:
		if (m_pRopeRenderNode && m_nSegmentsOrg>=0) 
		{
			IRopeRenderNode::SRopeParams params = m_pRopeRenderNode->GetParams();
			params.nNumSegments = m_nSegmentsOrg;
			params.fTextureTileV = m_texTileVOrg;
			m_pRopeRenderNode->SetParams(params);
			m_nSegmentsOrg = -1;
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
void CRopeProxy::PreserveParams()
{
	if (m_pRopeRenderNode && m_nSegmentsOrg<0) 
	{
		const IRopeRenderNode::SRopeParams &params = m_pRopeRenderNode->GetParams();
		m_nSegmentsOrg = params.nNumSegments;
		m_texTileVOrg = params.fTextureTileV;
	}	
}

//////////////////////////////////////////////////////////////////////////
bool CRopeProxy::NeedSerialize()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CRopeProxy::GetSignature( TSerialize signature )
{
	signature.BeginGroup("RopeProxy");
	signature.EndGroup();
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CRopeProxy::Serialize( TSerialize ser )
{
	if (m_pRopeRenderNode && m_pRopeRenderNode->GetPhysics())
		if (ser.IsReading())
		{
			m_pRopeRenderNode->GetPhysics()->SetStateFromSnapshot(ser);
			m_pRopeRenderNode->OnPhysicsPostStep();
		} else
			m_pRopeRenderNode->GetPhysics()->GetStateSnapshot(ser);
	/*
	ser.BeginGroup("PhysicsProxy");
	if (m_pPhysicalEntity)
	{
		if (ser.IsReading())
		{
			if (CVar::pAllowInterpolation->GetIVal() && m_pInterpolator && ser.ShouldCommitValues())
			{
				m_pInterpolator->PreSynchronize( m_pPhysicalEntity );
				m_pPhysicalEntity->SetStateFromSnapshot( ser, 0 );
				m_pInterpolator->PostSynchronize( m_pPhysicalEntity );
			}
			else
			{
				m_pPhysicalEntity->SetStateFromSnapshot( ser, 0 );
			}
		}
		else
		{
			m_pPhysicalEntity->GetStateSnapshot( ser );
		}
	}

	if (ser.GetSerializationTarget()!=eST_Network) // no folieage over network for now.
	{
		CEntityObject *pSlot = m_pEntity->GetSlot(0);
		if (pSlot && pSlot->pStatObj && pSlot->pFoliage)
			pSlot->pFoliage->Serialize(ser);

		if (pSlot && pSlot->pCharacter && !pSlot->pCharacter->GetISkeleton()->GetCharacterPhysics())
			for(int i=0;pSlot->pCharacter->GetISkeleton()->GetCharacterPhysics(i);i++)
				if (ser.IsReading())
					pSlot->pCharacter->GetISkeleton()->GetCharacterPhysics(i)->SetStateFromSnapshot(ser);
				else
					pSlot->pCharacter->GetISkeleton()->GetCharacterPhysics(i)->GetStateSnapshot(ser);
	}

	ser.EndGroup();
	*/
}

//////////////////////////////////////////////////////////////////////////
inline void RopeParamsToXml( IRopeRenderNode::SRopeParams &rp,XmlNodeRef &node,bool bLoad )
{
	if (bLoad)
	{
		// Load.
		node->getAttr( "flags",rp.nFlags );
		node->getAttr( "radius",rp.fThickness );
		node->getAttr( "anchor_radius",rp.fAnchorRadius );
		node->getAttr( "num_seg",rp.nNumSegments );
		node->getAttr( "num_sides",rp.nNumSides );
		node->getAttr( "radius",rp.fThickness );
		node->getAttr( "texu",rp.fTextureTileU );
		node->getAttr( "texv",rp.fTextureTileV );
		node->getAttr( "ph_num_seg",rp.nPhysSegments );
		node->getAttr( "ph_sub_vtxs",rp.nMaxSubVtx );
		node->getAttr( "mass",rp.mass );
		node->getAttr( "tension",rp.tension );
		node->getAttr( "friction",rp.friction );
		node->getAttr( "friction_pull",rp.frictionPull );
		node->getAttr( "wind",rp.wind );
		node->getAttr( "wind_var",rp.windVariance );
		node->getAttr( "air_resist",rp.airResistance );
		node->getAttr( "water_resist",rp.waterResistance );
		node->getAttr( "joint_lim",rp.jointLimit );
		node->getAttr( "max_force",rp.maxForce );
		node->getAttr( "max_iters",rp.nMaxIters );
		node->getAttr( "max_time_step",rp.maxTimeStep );
		node->getAttr( "stiffness",rp.stiffness );
		node->getAttr( "hardness",rp.hardness );
		node->getAttr( "damping",rp.damping );
		node->getAttr( "sleep_speed",rp.sleepSpeed );
	}
	else
	{
		// Save.
		node->setAttr( "flags",rp.nFlags );
		node->setAttr( "radius",rp.fThickness );
		node->setAttr( "anchor_radius",rp.fAnchorRadius );
		node->setAttr( "num_seg",rp.nNumSegments );
		node->setAttr( "num_sides",rp.nNumSides );
		node->setAttr( "radius",rp.fThickness );
		node->setAttr( "texu",rp.fTextureTileU );
		node->setAttr( "texv",rp.fTextureTileV );
		node->setAttr( "ph_num_seg",rp.nPhysSegments );
		node->setAttr( "ph_sub_vtxs",rp.nMaxSubVtx );
		node->setAttr( "mass",rp.mass );
		node->setAttr( "tension",rp.tension );
		node->setAttr( "friction",rp.friction );
		node->setAttr( "friction_pull",rp.frictionPull );
		node->setAttr( "wind",rp.wind );
		node->setAttr( "wind_var",rp.windVariance );
		node->setAttr( "air_resist",rp.airResistance );
		node->setAttr( "water_resist",rp.waterResistance );
		node->setAttr( "joint_lim",rp.jointLimit );
		node->setAttr( "max_force",rp.maxForce );
		node->setAttr( "max_iters",rp.nMaxIters );
		node->setAttr( "max_time_step",rp.maxTimeStep );
		node->setAttr( "stiffness",rp.stiffness );
		node->setAttr( "hardness",rp.hardness );
		node->setAttr( "damping",rp.damping );
		node->setAttr( "sleepSpeed",rp.sleepSpeed );
	}
}

//////////////////////////////////////////////////////////////////////////
void CRopeProxy::SerializeXML( XmlNodeRef &entityNode,bool bLoading )
{
	IRopeRenderNode::SRopeParams ropeParams = m_pRopeRenderNode->GetParams();
	if (bLoading)
	{
		uint32 nMaterialLayers = 0;
		if (entityNode->getAttr("MatLayersMask",nMaterialLayers))
		{
			m_pRopeRenderNode->SetMaterialLayers( nMaterialLayers );
		}

		XmlNodeRef ropeNode = entityNode->findChild("Rope");
		if (ropeNode)
		{
			RopeParamsToXml( ropeParams,ropeNode,bLoading );
			m_pRopeRenderNode->SetParams( ropeParams );

			XmlNodeRef pointsNode = ropeNode->findChild("Points");
			if (pointsNode)
			{
				DynArray<Vec3> points;
				points.resize(pointsNode->getChildCount());
				for (int i = 0,num = pointsNode->getChildCount(); i < num; i++)
				{
					XmlNodeRef pnt = pointsNode->getChild(i);
					Vec3 p;
					pnt->getAttr( "Pos",p );
					points[i] = p;
				}
				m_pRopeRenderNode->SetMatrix( m_pEntity->GetWorldTM() );
				m_pRopeRenderNode->SetPoints( &points[0],points.size() );
			}

			// Sound related
			XmlNodeRef const xmlNodeSound = ropeNode->findChild("Sound");
			if (xmlNodeSound)
			{
				char const* pcName = NULL;
				int unsigned nNumSegmentToAttachTo = 0;
				float fOffset = 0.0f;
				xmlNodeSound->getAttr( "Name",&pcName);
				xmlNodeSound->getAttr( "SegmentToAttachTo",nNumSegmentToAttachTo );
				xmlNodeSound->getAttr( "Offset",fOffset );
				m_pRopeRenderNode->SetRopeSound( pcName, nNumSegmentToAttachTo, fOffset );
			}
		}
	}
	else
	{
		// No saving.
		//XmlNodeRef ropeNode = entityNode->newChild("Rope");
		//RopeParamsToXml( ropeParams,ropeNode,bLoading );
	}
}

#include UNIQUE_VIRTUAL_WRAPPER(IEntityRopeProxy)

////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   SubstitutionProxy.cpp
//  Version:     v1.00
//  Created:     7/6/2005 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SubstitutionProxy.h"
#include "Entity.h"
#include "ISerialize.h"

void CSubstitutionProxy::Done() 
{
	// Substitution proxy does not need to be restored if entity system is being rested.
	if (m_pSubstitute && !g_pIEntitySystem->m_bReseting)
	{
		//gEnv->pLog->Log("CRYSIS-3502: CSubstitutionProxy::Done: Ptr=%d", (int)m_pSubstitute);
		//gEnv->pLog->Log("CRYSIS-3502: CSubstitutionProxy::Done: %s", m_pSubstitute->GetName());
		//gEnv->pLog->Log("CRYSIS-3502: CSubstitutionProxy::Done: Pos=(%.2f,%.2f,%.2f)", m_pSubstitute->GetPos().x, m_pSubstitute->GetPos().y, m_pSubstitute->GetPos().z);
		gEnv->p3DEngine->RegisterEntity(m_pSubstitute);
		m_pSubstitute->Physicalize(true);
		AABB WSBBox = m_pSubstitute->GetBBox();
		static ICVar *e_on_demand_physics(gEnv->pConsole->GetCVar("e_OnDemandPhysics")),
								 *e_on_demand_maxsize(gEnv->pConsole->GetCVar("e_OnDemandMaxSize"));
		if (m_pSubstitute->GetPhysics() && e_on_demand_physics && e_on_demand_physics->GetIVal() &&
				e_on_demand_maxsize && max(WSBBox.max.x-WSBBox.min.x,WSBBox.max.y-WSBBox.min.y)<=e_on_demand_maxsize->GetFVal())
			gEnv->pPhysicalWorld->AddRefEntInPODGrid(m_pSubstitute->GetPhysics(), &WSBBox.min);
		m_pSubstitute = 0;
	}
}

void CSubstitutionProxy::SetSubstitute(IRenderNode *pSubstitute) 
{ 
	m_pSubstitute = pSubstitute; 
	//gEnv->pLog->Log("CRYSIS-3502: CSubstitutionProxy::SetSubstitute: Ptr=%d", (int)m_pSubstitute);
	//gEnv->pLog->Log("CRYSIS-3502: CSubstitutionProxy::SetSubstitute: %s", m_pSubstitute->GetName());
	//gEnv->pLog->Log("CRYSIS-3502: CSubstitutionProxy::SetSubstitute: Pos=(%.2f,%.2f,%.2f)", m_pSubstitute->GetPos().x, m_pSubstitute->GetPos().y, m_pSubstitute->GetPos().z);
}

//////////////////////////////////////////////////////////////////////////
void CSubstitutionProxy::Reload( IEntity *pEntity,SEntitySpawnParams &params )
{
	m_pSubstitute = 0;
}

//////////////////////////////////////////////////////////////////////////
bool CSubstitutionProxy::NeedSerialize()
{
	return m_pSubstitute != 0;
};

//////////////////////////////////////////////////////////////////////////
bool CSubstitutionProxy::GetSignature( TSerialize signature )
{
	signature.BeginGroup("SubstitutionProxy");
	signature.EndGroup();
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CSubstitutionProxy::Serialize( TSerialize ser )
{
	Vec3 center,pos;
	if (ser.IsReading())
	{
		if (!m_pSubstitute)
		{
			ser.Value("SubstCenter", center);
			ser.Value("SubstPos", pos);
			IPhysicalEntity **pents;
			m_pSubstitute = 0;
			int i = gEnv->pPhysicalWorld->GetEntitiesInBox(center-Vec3(0.05f),center+Vec3(0.05f),pents,ent_static);
			for(--i; i>=0 && !((m_pSubstitute = (IRenderNode*)pents[i]->GetForeignData(PHYS_FOREIGN_ID_STATIC)) &&
				(m_pSubstitute->GetPos()-pos).len2()<sqr(0.03f) && 
				(m_pSubstitute->GetBBox().GetCenter()-center).len2()<sqr(0.03f)); i--);
			if (i<0)
				m_pSubstitute = 0;
			else if (m_pSubstitute)
			{
				//gEnv->pLog->Log("CRYSIS-3502: CSubstitutionProxy::Serialize: Ptr=%d", (int)m_pSubstitute);
				//gEnv->pLog->Log("CRYSIS-3502: CSubstitutionProxy::Serialize: %s", m_pSubstitute->GetName());
				//gEnv->pLog->Log("CRYSIS-3502: CSubstitutionProxy::Serialize: Pos=(%.2f,%.2f,%.2f)", m_pSubstitute->GetPos().x, m_pSubstitute->GetPos().y, m_pSubstitute->GetPos().z);

				m_pSubstitute->Dephysicalize();
				gEnv->p3DEngine->UnRegisterEntityAsJob(m_pSubstitute);
			}
		}
	} else
	{
		if (m_pSubstitute)
		{
			ser.Value("SubstCenter", center=m_pSubstitute->GetBBox().GetCenter());
			ser.Value("SubstPos", pos=m_pSubstitute->GetPos());
		}
	}
}

#include UNIQUE_VIRTUAL_WRAPPER(IEntitySubstitutionProxy)

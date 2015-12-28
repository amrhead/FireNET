/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2005.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: Implements a base class for vehicle flow node

-------------------------------------------------------------------------
History:
- 12:12:2005: Created by Mathieu Pinard

*************************************************************************/
#include "StdAfx.h"
#include "CryAction.h"
#include "IVehicleSystem.h"
#include "IFlowSystem.h"
#include "FlowSystem/Nodes/FlowBaseNode.h"
#include "FlowVehicleBase.h"

//------------------------------------------------------------------------
void CFlowVehicleBase::Init(SActivationInfo* pActivationInfo)
{
	m_nodeID = pActivationInfo->myID;
	m_pGraph = pActivationInfo->pGraph;

	if (IEntity* pEntity = pActivationInfo->pEntity)
	{
		IVehicleSystem* pVehicleSystem = CCryAction::GetCryAction()->GetIVehicleSystem();
		CRY_ASSERT(pVehicleSystem);

		if (pVehicleSystem->GetVehicle(pEntity->GetId()))
			m_vehicleId = pEntity->GetId();
	}
	else
		m_vehicleId = 0;
}

//------------------------------------------------------------------------
void CFlowVehicleBase::Delete()
{
	if (IVehicle* pVehicle = GetVehicle())
		pVehicle->UnregisterVehicleEventListener(this);
}

//------------------------------------------------------------------------
void CFlowVehicleBase::GetConfiguration(SFlowNodeConfig& nodeConfig)
{
}

//------------------------------------------------------------------------
void CFlowVehicleBase::ProcessEvent(EFlowEvent flowEvent, SActivationInfo* pActivationInfo)
{
	if (flowEvent == eFE_SetEntityId)
	{
		IEntity* pEntity = pActivationInfo->pEntity;

		if (pEntity)
		{
			IVehicleSystem* pVehicleSystem = CCryAction::GetCryAction()->GetIVehicleSystem();
			CRY_ASSERT(pVehicleSystem);

			if (pEntity->GetId() != m_vehicleId)
			{
				if (IVehicle* pVehicle = GetVehicle())
					pVehicle->UnregisterVehicleEventListener(this);

				m_vehicleId = 0;
			}
			
			if (IVehicle* pVehicle = pVehicleSystem->GetVehicle(pEntity->GetId()))
			{
				pVehicle->RegisterVehicleEventListener(this, "FlowVehicleBase");
				m_vehicleId = pEntity->GetId();
			}
		}
		else
		{
			if (IVehicle* pVehicle = GetVehicle())
				pVehicle->UnregisterVehicleEventListener(this);
		}
	}
}

//------------------------------------------------------------------------
void CFlowVehicleBase::Serialize(SActivationInfo* pActivationInfo, TSerialize ser)
{
}

//------------------------------------------------------------------------
void CFlowVehicleBase::OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params)
{
	if (event == eVE_VehicleDeleted)
		m_vehicleId = 0;
}

//------------------------------------------------------------------------
IVehicle* CFlowVehicleBase::GetVehicle()
{
	if (!m_vehicleId)
		return NULL;

	return CCryAction::GetCryAction()->GetIVehicleSystem()->GetVehicle(m_vehicleId);
}

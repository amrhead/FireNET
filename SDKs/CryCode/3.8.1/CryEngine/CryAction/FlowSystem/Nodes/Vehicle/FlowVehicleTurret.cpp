/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2005.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: Implements a flow node to vehicle turrets

-------------------------------------------------------------------------
History:
- 12:12:2005: Created by Mathieu Pinard

*************************************************************************/
#include "StdAfx.h"
#include "CryAction.h"
#include "IVehicleSystem.h"
#include "VehicleSystem/Vehicle.h"
#include "VehicleSystem/VehicleSeat.h"
#include "VehicleSystem/VehicleSeatActionRotateTurret.h"
#include "IFlowSystem.h"
#include "FlowSystem/Nodes/FlowBaseNode.h"
#include "FlowVehicleTurret.h"

//------------------------------------------------------------------------
IFlowNodePtr CFlowVehicleTurret::Clone(SActivationInfo* pActivationInfo)
{
	IFlowNode* pNode = new CFlowVehicleTurret(pActivationInfo);
	return pNode;
}

//------------------------------------------------------------------------
void CFlowVehicleTurret::GetConfiguration(SFlowNodeConfig& nodeConfig)
{
	CFlowVehicleBase::GetConfiguration(nodeConfig);

	static const SInputPortConfig pInConfig[] = 
	{
		InputPortConfig_AnyType("Trigger", _HELP("trigger the turret to be rotated")),
		InputPortConfig<int>("seatId", _HELP("id of the seat which can rotate the turret")),
		InputPortConfig<Vec3>("aimAngles", _HELP("aiming angles for the turret")),
		InputPortConfig<Vec3>("aimPos", _HELP("world position at which we want to aim the turret")),
		{0}
	};

	static const SOutputPortConfig pOutConfig[] = 
	{
		{0}
	};

	nodeConfig.sDescription = _HELP("Handle vehicle turret");
	nodeConfig.nFlags |= EFLN_TARGET_ENTITY;
	nodeConfig.pInputPorts = pInConfig;
	nodeConfig.pOutputPorts = pOutConfig;
	nodeConfig.SetCategory(EFLN_APPROVED);
}

//------------------------------------------------------------------------
void CFlowVehicleTurret::ProcessEvent(EFlowEvent flowEvent, SActivationInfo* pActivationInfo)
{
	CFlowVehicleBase::ProcessEvent(flowEvent, pActivationInfo);

	if (flowEvent == eFE_SetEntityId)
	{
		m_pActionRotateTurret = NULL;
	}

	if (flowEvent == eFE_Initialize || flowEvent == eFE_Activate)
	{
		if (IsPortActive(pActivationInfo, IN_SEATID))
		{
			m_seatId = GetPortInt(pActivationInfo, IN_SEATID);

			if (IVehicle* pVehicle = GetVehicle())
			{
				if (CVehicleSeat* pSeat = static_cast<CVehicleSeat*>(pVehicle->GetSeatById(m_seatId)))
				{
					TVehicleSeatActionVector& seatAction = pSeat->GetSeatActions();
					for (TVehicleSeatActionVector::iterator ite = seatAction.begin(); ite != seatAction.end(); ++ite)
					{
						IVehicleSeatAction* pSeatAction = ite->pSeatAction;
						if (m_pActionRotateTurret = CAST_VEHICLEOBJECT(CVehicleSeatActionRotateTurret, pSeatAction))
						{
							SetAimPos();
							break;
						}
					}
				}
			}
		}

		if (IsPortActive(pActivationInfo, IN_AIMANGLES))
		{
			//...
		}
		else if (IsPortActive(pActivationInfo, IN_AIMPOS))
		{
			m_aimPos = GetPortVec3(pActivationInfo, IN_AIMPOS);
		}

		if (IsPortActive(pActivationInfo, IN_TRIGGER))
		{
			SetAimPos();
		}
	}
}

//------------------------------------------------------------------------
void CFlowVehicleTurret::Serialize(SActivationInfo* pActivationInfo, TSerialize ser)
{
	CFlowVehicleBase::Serialize(pActivationInfo, ser);
}

//------------------------------------------------------------------------
void CFlowVehicleTurret::OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params)
{
	CFlowVehicleBase::OnVehicleEvent(event, params);
}

//------------------------------------------------------------------------
void CFlowVehicleTurret::SetAimPos()
{
	if (m_pActionRotateTurret && !m_aimPos.IsZero())
	{
		m_pActionRotateTurret->SetAimGoal(m_aimPos);
	}
}



REGISTER_FLOW_NODE( "Vehicle:VehicleTurret", CFlowVehicleTurret);

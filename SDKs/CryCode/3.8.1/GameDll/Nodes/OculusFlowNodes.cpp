/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2015.
-------------------------------------------------------------------------
Description:
- This node includes different generic VR functionality
History:
- 02.02.2015   Created by Dario Sancho
*************************************************************************/

#include "StdAfx.h"

#include "Nodes/G2FlowBaseNode.h"
#include <IHMDDevice.h>
#include "ICryAnimation.h"
#include <IHMDDevice.h>
#include <IHMDManager.h>

class CVRTools : public CFlowBaseNode<eNCT_Singleton>
{
	enum INPUTS
	{
		EIP_RecenterPose = 0
	};

	enum OUTPUTS
	{
		EOP_Done = 0,
		EOP_Triggered,
		EOP_Failed,
	};

public:
	CVRTools( SActivationInfo * pActInfo )
	{
	}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	virtual void GetConfiguration( SFlowNodeConfig &config )
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig_Void( "RecentrePose",_HELP("Resets the tracking origin to the headset's current location, and sets the yaw origin to the current headset yaw value") ),
			{0}
		};
		static const SOutputPortConfig out_config[] = {
			OutputPortConfig_AnyType("Done", _HELP("The selected operation has been acknoledge. This output will always get triggered.") ),
			OutputPortConfig_AnyType("Triggered", _HELP("The selected operation has been triggered.") ),
			OutputPortConfig_AnyType("Failed", _HELP("The selected operation did not work as expected (e.g. the VR operation was not supported).") ),
			{0}
		};
		config.sDescription = _HELP( "Various VR-specific utils" );
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.SetCategory(EFLN_APPROVED);
	}

	virtual void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
			{
				if(IsPortActive(pActInfo, EIP_RecenterPose))
				{
					bool triggered = false;
					IHMDManager * pHmDManager = gEnv->pSystem->GetHMDManager();
					IHMDDevice* pDev = pHmDManager->GetHMDDevice();
					if (pDev && pHmDManager->IsStereoSetupOk())
					{
						const HMDTrackingState& sensorState = pDev->GetTrackingState();
						if (sensorState.CheckStatusFlags(HS_IsUsable))
						{
							pDev->RecenterPose();
							triggered = true;
						}
					}
					SAFE_RELEASE(pDev);

					ActivateOutput(pActInfo, triggered ? EOP_Triggered : EOP_Failed, true);
					ActivateOutput(pActInfo, EOP_Done, true);
				}
			}
			break;
		}
	}
};

class CVRGetCameraAngles : public CFlowBaseNode<eNCT_Singleton>
{
	enum INPUTS
	{
		EIP_Read = 0,
	};

	enum OUTPUTS
	{
		EOP_Done = 0,
		EOP_Roll,
		EOP_Pitch,
		EOP_Yaw
	};

public:
	CVRGetCameraAngles( SActivationInfo * pActInfo )
	{
	}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}

	virtual void GetConfiguration( SFlowNodeConfig &config )
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig<float>( "Read",_HELP("Read the camera angles (Roll,Pitch,Yaw)") ),
			{0}
		};
		static const SOutputPortConfig out_config[] = {
			OutputPortConfig_AnyType("Done", _HELP("The selected operation has been acknoledge. This output will always get triggered.") ),
			OutputPortConfig_AnyType("Roll", _HELP("Camera Roll.") ),
			OutputPortConfig_AnyType("Pitch", _HELP("Camera Pitch;") ),
			OutputPortConfig_AnyType("Yaw", _HELP("Camera Yaw;") ),
			{0}
		};
		config.sDescription = _HELP( "Various VR-specific utils" );
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.SetCategory(EFLN_APPROVED);
	}

	virtual void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
	{
		switch (event)
		{
		case eFE_Activate:
			{
				if(IsPortActive(pActInfo, EIP_Read))
				{
					if (IRenderer * pRenderer = gEnv->pRenderer)
					{
						const CCamera &rCam = pRenderer->GetCamera();
						const Ang3 angles = rCam.GetAngles();
						ActivateOutput(pActInfo, EOP_Roll, angles.z);
						ActivateOutput(pActInfo, EOP_Pitch, angles.y);
						ActivateOutput(pActInfo, EOP_Yaw, angles.x);

						ActivateOutput(pActInfo, EOP_Done, true);
					}
				}
			}
			break;
		}
	}
};


REGISTER_FLOW_NODE( "VR:Tools", CVRTools);
REGISTER_FLOW_NODE( "VR:GetCameraAngles", CVRGetCameraAngles);

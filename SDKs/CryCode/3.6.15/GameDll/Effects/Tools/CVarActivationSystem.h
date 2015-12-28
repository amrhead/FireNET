#ifndef _CVAR_ACTIVATION_SYSTEM_
#define _CVAR_ACTIVATION_SYSTEM_

#pragma once

//==================================================================================================
// Name: SCVarParam
// Desc: CVar param used in the cvar activation system
// Author: James Chilvers
//==================================================================================================
struct SCVarParam
{
	SCVarParam()
	{
		cvar = NULL;
		activeValue = 0.0f;
		originalValue = 0.0f;
	}

	ICVar*	cvar;
	float		activeValue;
	float		originalValue;
};//------------------------------------------------------------------------------------------------

//==================================================================================================
// Name: CCVarActivationSystem
// Desc: Simple data driven system to activate cvars
// Author: James Chilvers
//==================================================================================================
class CCVarActivationSystem
{
public:
	CCVarActivationSystem(){}
	~CCVarActivationSystem(){}

	SC_API void Initialise(const IItemParamsNode* cvarListXmlNode); // Uses the xml node name for the cvar, and activeValue attribute
																													 // eg <cl_fov activeValue="85"/>

	SC_API void Release();

	SC_API void StoreCurrentValues();					// This stores the current values in the SCVarParam, and the CVar will be set to these when
																						// SetCVarsActive(false) is called 

	SC_API void SetCVarsActive(bool isActive);	// Uses active value when set true, and original value when set false

private:
	PodArray<SCVarParam>		m_cvarParam;
};//------------------------------------------------------------------------------------------------

#endif // _CVAR_ACTIVATION_SYSTEM_

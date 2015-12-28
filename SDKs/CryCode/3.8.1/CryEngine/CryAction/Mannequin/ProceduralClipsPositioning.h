#ifndef __PROCEDURAL_CLIPS_POSITIONING__H__
#define __PROCEDURAL_CLIPS_POSITIONING__H__

#include <ICryMannequin.h>

struct SProceduralClipPosAdjustTargetLocatorParams 
	: public IProceduralParams
{
	SProceduralClipPosAdjustTargetLocatorParams()
	{
	}

	virtual void Serialize(Serialization::IArchive& ar);

	virtual void GetExtraDebugInfo(StringWrapper& extraInfoOut) const override
	{
		extraInfoOut = targetScopeName.c_str();
	}

	SProcDataCRC targetScopeName;
	TProcClipString targetJointName;
	SProcDataCRC targetStateName;
};

#endif
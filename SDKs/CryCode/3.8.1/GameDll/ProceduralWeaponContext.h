#pragma once

#ifndef _PROCEDURAL_WEAPON_CONTEXT_H_
#define _PROCEDURAL_WEAPON_CONTEXT_H_

#include "ICryMannequin.h"
#include "ProceduralWeaponAnimation.h"



class CProceduralWeaponAnimationContext : public IProceduralContext
{
private:
	struct SParams
	{
		SParams();
		SParams(IDefaultSkeleton& pSkeleton);

		int m_weaponTargetIdx;
		int m_leftHandTargetIdx;
		int m_rightBlendIkIdx;
	};

public:
	PROCEDURAL_CONTEXT(CProceduralWeaponAnimationContext, "ProceduralWeaponAnimationContext", 0xDF1D02E05F4048A1, 0xBC7759DCC568AA7F);

	virtual void Update(float timePassed);
	void SetAimDirection(Vec3 direction);
	void Initialize(IScope* pScope);
	void Finalize();
	CProceduralWeaponAnimation& GetWeaponSway() {return m_weaponSway;}

private:
	SParams m_params;
	IAnimationOperatorQueuePtr m_pPoseModifier;
	IScope* m_pScope;

	CProceduralWeaponAnimation m_weaponSway;
	Vec3 m_aimDirection;
	int m_instanceCount;
};



#endif

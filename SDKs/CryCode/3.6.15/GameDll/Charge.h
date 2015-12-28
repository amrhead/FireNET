/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2004.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: Single-shot Fire Mode Implementation

-------------------------------------------------------------------------
History:
- 11:9:2004   15:00 : Created by M�rcio Martins

*************************************************************************/
#ifndef __CHARGE_H__
#define __CHARGE_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include "EntityUtility/EntityEffects.h"
#include "Automatic.h"

class CCharge :
	public CAutomatic
{
public:
	CRY_DECLARE_GTI(CCharge);

	CCharge();
	virtual ~CCharge();

	virtual void Update(float frameTime, uint32 frameId);
	virtual void GetMemoryUsage(ICrySizer * s) const;

	virtual void Activate(bool activate);

	virtual void StartFire();
	virtual void StopFire();

	virtual bool Shoot(bool resetAnimation, bool autoreload, bool isRemote=false);

	virtual void ChargeEffect(bool attach);
	virtual void ChargedShoot();
	
protected:

	int							m_charged;
	bool						m_charging;
	float						m_chargeTimer;
	bool						m_autoreload;

	EntityEffects::TAttachedEffectId m_chargeEffectId;
	float						m_chargedEffectTimer;
};

#endif //__CHARGE_H__

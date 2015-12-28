/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2004.
-------------------------------------------------------------------------
$Id$
$DateTime$

-------------------------------------------------------------------------
History:
- 5:5:2006   15:26 : Created by M�rcio Martins

*************************************************************************/
#include "StdAfx.h"
#include "Game.h"
#include "Chaff.h"
#include "GameRules.h"
#include <IEntitySystem.h>
#include <IGameTokens.h>

#include "WeaponSystem.h"

VectorSet<CChaff*> CChaff::s_chaffs;

//------------------------------------------------------------------------
CChaff::CChaff()
{
	s_chaffs.insert(this);
	if(s_chaffs.size()>MAX_SPAWNED_CHAFFS)
	{
		if(s_chaffs[0]!=this)
			s_chaffs[0]->Destroy();
		else
			s_chaffs[1]->Destroy(); //Just in case...??
	}
}

//------------------------------------------------------------------------
CChaff::~CChaff()
{
	s_chaffs.erase(this);
}

//------------------------------------------------------------------------

void CChaff::HandleEvent(const SGameObjectEvent &event)
{
	CProjectile::HandleEvent(event);
}

Vec3 CChaff::GetPosition(void)
{
	return m_last;
}
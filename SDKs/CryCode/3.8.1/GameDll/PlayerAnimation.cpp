/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2010.
-------------------------------------------------------------------------
Description: Helper interface for player animation control

-------------------------------------------------------------------------
History:
- 08.9.11: Created by Tom Berry

*************************************************************************/

#include "StdAfx.h"

#include "PlayerAnimation.h"

SMannequinPlayerParams PlayerMannequin;

void InitPlayerMannequin( IActionController* pActionController )
{
	CRY_ASSERT( g_pGame );
	IGameFramework* pGameFramework = g_pGame->GetIGameFramework();
	CRY_ASSERT( pGameFramework );
	CMannequinUserParamsManager& mannequinUserParams = pGameFramework->GetMannequinInterface().GetMannequinUserParamsManager();

	mannequinUserParams.RegisterParams< SMannequinPlayerParams >( pActionController, &PlayerMannequin );
}


// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.


#ifndef __CET_LEVELLOADING_H__
#define __CET_LEVELLOADING_H__

#pragma once

void AddPrepareLevelLoad( IContextEstablisher * pEst, EContextViewState state );
void AddLoadLevel( IContextEstablisher * pEst, EContextViewState state, bool ** pStarted );
void AddLoadingComplete( IContextEstablisher * pEst, EContextViewState state, bool * pStarted );
void AddResetAreas( IContextEstablisher * pEst, EContextViewState state );
void AddLockResources( IContextEstablisher * pEst, EContextViewState stateBegin, EContextViewState stateEnd, CGameContext * pGameContext );
void AddActionEvent( IContextEstablisher * pEst, EContextViewState state, const SActionEvent& event );

#endif

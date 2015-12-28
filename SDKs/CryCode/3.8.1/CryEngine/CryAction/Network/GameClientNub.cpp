/*************************************************************************
  Crytek Source File.
  Copyright (C), Crytek Studios, 2001-2004.
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  
 -------------------------------------------------------------------------
  History:
  - 24:8:2004   10:58 : Created by M�rcio Martins

*************************************************************************/
#include "StdAfx.h"
#include "CryAction.h"
#include "GameContext.h"
#include "GameClientNub.h"
#include "GameClientChannel.h"

CGameClientNub::~CGameClientNub()
{
	delete m_pClientChannel;
}

void CGameClientNub::Release()
{
	// don't delete because it's not dynamically allocated
}

SCreateChannelResult CGameClientNub::CreateChannel(INetChannel *pChannel, const char *pRequest)
{
	if (pRequest && !gEnv->pNetwork->IsRebroadcasterEnabled())
	{
		GameWarning("CGameClientNub::CreateChannel: pRequest is non-null, it should not be");
		CRY_ASSERT(false);
		SCreateChannelResult res(eDC_GameError);
		cry_strcpy(res.errorMsg, "CGameClientNub::CreateChannel: pRequest is non-null, it should not be");
		return res;
	}

	if (m_pClientChannel)
	{
		if (gEnv->pNetwork->IsRebroadcasterEnabled())
		{
			// A client channel has already been set up so this must be a rebroadcaster (peer to peer) channel
			pChannel->SetPeer(m_pGameContext->GetNetContext(), true);
			SCreateChannelResult res(reinterpret_cast<IGameChannel*>(pChannel));
			return res;
		}
		else
		{
			GameWarning("CGameClientNub::CreateChannel: m_pClientChannel is non-null, it should not be");
			CRY_ASSERT(false);
			SCreateChannelResult res(eDC_GameError);
			cry_strcpy(res.errorMsg, "CGameClientNub::CreateChannel: m_pClientChannel is non-null, it should not be");
			return res;
		}
	}

	if (CCryAction::GetCryAction()->IsGameSessionMigrating())
	{
		pChannel->SetMigratingChannel(true);
	}

	m_pClientChannel = new CGameClientChannel(pChannel, m_pGameContext, this);

  ICVar* pPass = gEnv->pConsole->GetCVar("sv_password");
  if(pPass && gEnv->bMultiplayer)
    pChannel->SetPassword(pPass->GetString());

	return SCreateChannelResult(m_pClientChannel);
}

void CGameClientNub::FailedActiveConnect(EDisconnectionCause cause, const char * description)
{
	GameWarning("Failed connecting to server: %s", description);
  CCryAction::GetCryAction()->OnActionEvent(SActionEvent(eAE_connectFailed,int(cause),description));
}

void CGameClientNub::Disconnect( EDisconnectionCause cause, const char * msg )
{
	if (m_pClientChannel) 
		m_pClientChannel->GetNetChannel()->Disconnect( cause, msg );
}

void CGameClientNub::ClientChannelClosed()
{
	CRY_ASSERT( m_pClientChannel );
	m_pClientChannel = NULL;
}

void CGameClientNub::GetMemoryUsage(ICrySizer * s) const
{
	s->Add(*this);
	if (m_pClientChannel)
		m_pClientChannel->GetMemoryStatistics(s);
}

// CryEngine Source File.
// Copyright (C), Crytek, 1999-2014.


#include "StdAfx.h"
#include "CET_NetConfig.h"
#include "GameContext.h"
#include "GameClientChannel.h"
#include "GameClientNub.h"
#include "GameServerChannel.h"
#include "GameServerNub.h"
#include "NetHelpers.h"

/*
 * Established context
 */

class CCET_EstablishedContext : public CCET_Base
{
public:
	CCET_EstablishedContext(int token) : m_token(token) {}

	const char * GetName() { return "EstablishedContext"; }

	EContextEstablishTaskResult OnStep( SContextEstablishState& state )
	{
		CCryAction::GetCryAction()->GetGameContext()->GetNetContext()->EstablishedContext( m_token );
		return eCETR_Ok;
	}

private:
	int m_token;
};

void AddEstablishedContext( IContextEstablisher * pEst, EContextViewState state, int token )
{
	pEst->AddTask( state, new CCET_EstablishedContext(token) );
}

/*
 * Declare witness
 */

class CCET_DeclareWitness : public CCET_Base
{
public:
	const char * GetName() { return "DeclareWitness"; }

	EContextEstablishTaskResult OnStep( SContextEstablishState& state )
	{
		if (CGameClientNub * pNub = CCryAction::GetCryAction()->GetGameClientNub())
		{
			if (CGameClientChannel * pChannel = pNub->GetGameClientChannel())
			{
				if (IActor * pActor = CCryAction::GetCryAction()->GetClientActor())
				{
					pChannel->GetNetChannel()->DeclareWitness( pActor->GetEntityId() );
					return eCETR_Ok;
				}
			}
		}
		return eCETR_Failed;
	}
};

void AddDeclareWitness( IContextEstablisher * pEst, EContextViewState state )
{
	pEst->AddTask( state, new CCET_DeclareWitness );
}

/*
 * Add rebroadcaster connection
 */

class CCET_PopulateRebroadcaster : public CCET_Base
{
public:
	const char* GetName() { return "PopulateRebroadcaster"; }

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		CGameServerNub* pNub = CCryAction::GetCryAction()->GetGameServerNub();
		if (pNub != NULL)
		{
			gEnv->pNetwork->AddRebroadcasterConnection(state.pSender, pNub->GetChannelId(state.pSender));
			return eCETR_Ok;
		}
		return eCETR_Failed;
	}
};

void AddPopulateRebroadcaster(IContextEstablisher* pEst, EContextViewState state)
{
	pEst->AddTask(state, new CCET_PopulateRebroadcaster());
}

/*
 * Delegate authority to player
 */

class CCET_DelegateAuthority : public CCET_Base
{
public:
	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		EntityId entityId = GetEntity(state);
		if (!entityId || !gEnv->pEntitySystem->GetEntity(entityId))
			return eCETR_Failed;
		CCryAction::GetCryAction()->GetGameContext()->GetNetContext()->DelegateAuthority( entityId, state.pSender );
		return eCETR_Ok;
	}

private:
	virtual EntityId GetEntity(SContextEstablishState& state) = 0;
};

class CCET_DelegateAuthority_ToClientActor : public CCET_DelegateAuthority
{
public:
	const char * GetName() { return "DelegateAuthorityToClientActor"; }

private:
	EntityId GetEntity(SContextEstablishState& state)
	{
		if (!state.pSender)
			return 0;
		CGameChannel *pGameChannel=static_cast<CGameChannel *>(state.pSender->GetGameChannel());
		if(pGameChannel)
			return pGameChannel->GetPlayerId();
		return 0;
	}
};

void AddDelegateAuthorityToClientActor(IContextEstablisher * pEst, EContextViewState state)
{
	pEst->AddTask( state, new CCET_DelegateAuthority_ToClientActor() );
}

/*
 * Clear player ids
 */

class CCET_ClearPlayerIds : public CCET_Base
{
public:
	const char * GetName() { return "ClearPlayerIds"; }

	EContextEstablishTaskResult OnStep( SContextEstablishState& state )
	{
		gEnv->pGame->OnClearPlayerIds();
		if (CGameServerNub * pNub = CCryAction::GetCryAction()->GetGameServerNub())
		{
			TServerChannelMap * pMap = pNub->GetServerChannelMap();
			for (TServerChannelMap::iterator it = pMap->begin(); it != pMap->end(); ++it)
				it->second->ResetPlayerId();
		}
		if (CGameClientNub * pCNub = CCryAction::GetCryAction()->GetGameClientNub())
		{
			if (CGameChannel * pChannel = pCNub->GetGameClientChannel())
				pChannel->ResetPlayerId();
		}
		return eCETR_Ok;
	}
};

void AddClearPlayerIds( IContextEstablisher * pEst, EContextViewState state )
{
	pEst->AddTask( state, new CCET_ClearPlayerIds() );
}

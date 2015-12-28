/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2008.
-------------------------------------------------------------------------
History:
- 23:1:2008   Created by Benito G.R. - Refactor'd from John N. ScreenEffects.h/.cpp

*************************************************************************/

#include "StdAfx.h"
#include "BlendedEffect.h"

#include "Player.h"

//-------------FOV EFFECT-------------------------------

CFOVEffect::CFOVEffect(float goalFOV)
: m_currentFOV (0.0f)
, m_startFOV(0.0f)
{
	m_goalFOV = goalFOV;
}

//---------------------------------
void CFOVEffect::Init()
{
	//TODO: Do not modify actor params this way...
	IActor *pClientActor = gEnv->pGame->GetIGameFramework()->GetClientActor();
	if (pClientActor)
	{
		CPlayer *pPlayer = (CPlayer *)pClientActor;
		m_startFOV = pPlayer->GetActorParams().viewFoVScale;
		m_currentFOV = m_startFOV;
	}
}

//---------------------------------
void CFOVEffect::Update(float point)
{
	m_currentFOV = (point * (m_goalFOV - m_startFOV)) + m_startFOV;
	IActor *pClientActor = gEnv->pGame->GetIGameFramework()->GetClientActor();
	if (pClientActor)
	{
		CPlayer *pPlayer = (CPlayer *)pClientActor;
		pPlayer->GetActorParams().viewFoVScale = m_currentFOV;
	}
}

//---------------------------------
void CFOVEffect::Reset()
{
	IActor *pClientActor = gEnv->pGame->GetIGameFramework()->GetClientActor();
	if (pClientActor)
	{
		CPlayer *pPlayer = (CPlayer *)pClientActor;
		pPlayer->GetActorParams().viewFoVScale = m_goalFOV;
	}
}

//-------------------POST PROCESS FX--------------------

CPostProcessEffect::CPostProcessEffect(string paramName, float goalVal)
: m_startVal(0.0f)
, m_currentVal(0.0f)
{
	m_paramName = paramName;
	m_goalVal = goalVal;
}

//---------------------------------
void CPostProcessEffect::Init()
{
	gEnv->p3DEngine->GetPostEffectParam(m_paramName, m_currentVal);
	m_startVal = m_currentVal;
}

//---------------------------------
void CPostProcessEffect::Update(float point)
{
	m_currentVal = (point * (m_goalVal - m_startVal)) + m_startVal;
	gEnv->p3DEngine->SetPostEffectParam(m_paramName, m_currentVal);
	
	//const float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	//gEnv->pRenderer->Draw2dLabel(50.0f, 50.0f, 2.0f, white, false, "Post Effect: %s Value = %.3f", m_paramName.c_str(), m_currentVal);
}

//---------------------------------
void CPostProcessEffect::Reset()
{
	gEnv->p3DEngine->SetPostEffectParam(m_paramName, m_goalVal);
}

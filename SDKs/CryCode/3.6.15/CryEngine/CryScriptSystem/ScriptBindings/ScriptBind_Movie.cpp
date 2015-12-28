////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   ScriptBind_Movie.cpp
//  Version:     v1.00
//  Created:     8/7/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "ScriptBind_Movie.h"
#include <ISystem.h>
#include <IMovieSystem.h>

//////////////////////////////////////////////////////////////////////////
CScriptBind_Movie::CScriptBind_Movie(IScriptSystem *pScriptSystem, ISystem *pSystem)
{
	CScriptableBase::Init(pScriptSystem,pSystem);
	m_pSystem=pSystem;
	m_pMovieSystem=gEnv->pMovieSystem;
	SetGlobalName( "Movie" );

#undef SCRIPT_REG_CLASSNAME 
#define SCRIPT_REG_CLASSNAME &CScriptBind_Movie::

	SCRIPT_REG_TEMPLFUNC(PlaySequence,"sSequenceName");
	SCRIPT_REG_TEMPLFUNC(StopSequence,"sSequenceName");
  SCRIPT_REG_TEMPLFUNC(AbortSequence,"sSequenceName");
	SCRIPT_REG_FUNC(StopAllSequences);
	SCRIPT_REG_FUNC(StopAllCutScenes);
	SCRIPT_REG_FUNC(PauseSequences);
	SCRIPT_REG_FUNC(ResumeSequences);
}

//////////////////////////////////////////////////////////////////////////
CScriptBind_Movie::~CScriptBind_Movie()
{
}

/*! Start a sequence
		@param pszName Name of sequence
*/
//////////////////////////////////////////////////////////////////////////
int CScriptBind_Movie::PlaySequence( IFunctionHandler *pH,const char *sSequenceName )
{
	IGame *pGame=gEnv->pGame;
	if (!pGame) 
	{
		// can this happen?
		return pH->EndFunction();
	}

	bool bResetFx=true;
	if (pH->GetParamCount()==2)
		pH->GetParam(2, bResetFx);

	IMovieSystem *movieSys = gEnv->pMovieSystem;
	if (movieSys != NULL)
		movieSys->PlaySequence(sSequenceName,NULL,bResetFx,false);

	return pH->EndFunction();
}

/*! Stop a sequence
		@param pszName Name of sequence
*/
//////////////////////////////////////////////////////////////////////////
int CScriptBind_Movie::StopSequence( IFunctionHandler *pH,const char* sSequenceName)
{
	if (m_pMovieSystem != NULL)
		m_pMovieSystem->StopSequence(sSequenceName);
	return pH->EndFunction();
}
/*! Abort a sequence
@param pszName Name of sequence
*/
//////////////////////////////////////////////////////////////////////////
int CScriptBind_Movie::AbortSequence( IFunctionHandler *pH,const char* sSequenceName)
{
  bool bLeaveTime=false;
  if (pH->GetParamCount()==2)
    pH->GetParam(2, bLeaveTime);
  //
  IAnimSequence *seq = m_pMovieSystem->FindSequence(sSequenceName);
  if (seq)
  {
    if (m_pMovieSystem != NULL)
      m_pMovieSystem->AbortSequence(seq,bLeaveTime);
  }
  return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptBind_Movie::StopAllSequences(IFunctionHandler *pH)
{
	if (m_pMovieSystem != NULL)
		m_pMovieSystem->StopAllSequences();
	return pH->EndFunction();
}
 
//////////////////////////////////////////////////////////////////////////
int CScriptBind_Movie::StopAllCutScenes(IFunctionHandler *pH)
{
	IGame *pGame=gEnv->pGame;
	if (!pGame) 
	{
		// can this happen?
		return pH->EndFunction();		
	}

	//pGame->StopCurrentCutscene();
	//if (m_pMovieSystem != NULL)
	//  m_pMovieSystem->StopAllCutScenes();
	gEnv->pMovieSystem->StopAllCutScenes();

	return pH->EndFunction();
}

int CScriptBind_Movie::PauseSequences(IFunctionHandler *pH)
{
	if (m_pMovieSystem != NULL)
		m_pMovieSystem->Pause();
	return pH->EndFunction();
}

int CScriptBind_Movie::ResumeSequences(IFunctionHandler *pH)
{
	if (m_pMovieSystem != NULL)
		m_pMovieSystem->Resume();
	return pH->EndFunction();
}

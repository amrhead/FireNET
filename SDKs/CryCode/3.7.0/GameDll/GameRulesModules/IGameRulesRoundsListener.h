/*************************************************************************
	Crytek Source File.
	Copyright (C), Crytek Studios, 2009.
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Description: 
		Interface for a class that receives events when games rounds serialise, etc.
	-------------------------------------------------------------------------
	History:
	- 23:11:2009  : Created by Thomas Houghton

*************************************************************************/

#ifndef _IGAME_RULES_ROUNDS_LISTENER_H_
#define _IGAME_RULES_ROUNDS_LISTENER_H_

#if _MSC_VER > 1000
# pragma once
#endif

class IGameRulesRoundsListener
{
public:
	virtual ~IGameRulesRoundsListener() {}

	virtual void OnRoundStart() = 0;
	virtual void OnRoundEnd() = 0;
	virtual void OnSuddenDeath() = 0;
	virtual void ClRoundsNetSerializeReadState(int newState, int curState) = 0;
	virtual void OnRoundAboutToStart() = 0;

};

#endif // _IGAME_RULES_ROUNDS_LISTENER_H_

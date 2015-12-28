/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2009.
-------------------------------------------------------------------------
Description: 
Listener interface for player updates.
-------------------------------------------------------------------------
History:
- 2:12:2009	Created by Adam Rutkowski
*************************************************************************/
#ifndef __PLAYERUPDATELISTENER_H__
#define __PLAYERUPDATELISTENER_H__

struct IPlayerUpdateListener
{
	virtual ~IPlayerUpdateListener(){}
	virtual void Update(float fFrameTime) = 0;
};

#endif //__PLAYERUPDATELISTENER_H__

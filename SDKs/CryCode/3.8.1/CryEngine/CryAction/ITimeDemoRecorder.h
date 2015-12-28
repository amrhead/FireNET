/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2015.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description:
Provides interface for TimeDemo (CryAction)
Can attach listener to perform game specific stuff on Record/Play
-------------------------------------------------------------------------
History:
- May 06, 2015:	Created by Pascal Kross

*************************************************************************/

#ifndef __ITIMEDEMORECORDER_H__
#define __ITIMEDEMORECORDER_H__

#pragma once


struct ITimeDemoListener
{
	virtual ~ITimeDemoListener(){}
	virtual void OnRecord(bool bEnable) = 0;
	virtual void OnPlayback(bool bEnable) = 0;
};

struct ITimeDemoRecorder
{
	virtual ~ITimeDemoRecorder(){}
	virtual void RegisterListener(ITimeDemoListener* pListener) = 0;
	virtual void UnregisterListener(ITimeDemoListener* pListener) = 0;
};


#endif //__ITIMEDEMORECORDER_H__

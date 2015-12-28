/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2006.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description:	Input implementation for Linux using SDL
-------------------------------------------------------------------------
History:
- Jun 09,2006:	Created by Sascha Demetrio

*************************************************************************/
#ifndef __LINUXINPUT_H__
#define __LINUXINPUT_H__
#pragma once

#ifdef USE_LINUXINPUT

#include "BaseInput.h"
#include "InputDevice.h"

class CSDLPadManager;
class CSDLMouse;
struct ILog;

#if !defined(ANDROID) && !defined(IOS)
#define SDL_USE_HAPTIC_FEEDBACK
#endif

class CLinuxInput : public CBaseInput
{
public:
	CLinuxInput(ISystem *pSystem); 

	virtual bool Init();
	virtual void ShutDown();
	virtual void Update(bool focus);
	virtual bool GrabInput(bool bGrab);

private:
	ISystem *m_pSystem;
	ILog *m_pLog;
	CSDLPadManager* m_pPadManager;
  CSDLMouse* m_pMouse;
};

class CLinuxInputDevice : public CInputDevice
{
public:
	CLinuxInputDevice(CLinuxInput& input, const char* deviceName);
	virtual ~CLinuxInputDevice();

	CLinuxInput& GetLinuxInput() const;
protected:
    void PostEvent(SInputSymbol *pSymbol, unsigned keyMod = ~0);
private:
	CLinuxInput &m_linuxInput;
	
};

struct ILog;
struct ICVar;

#endif // USE_LINUXINPUT

#endif // __LINUXINPUT_H__

// vim:ts=2:sw=2:tw=78


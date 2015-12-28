/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2006.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description:	Keyboard for Windows/DirectX
-------------------------------------------------------------------------
History:
- Dec 05,2005:	Major rewrite by Marco Koegler

*************************************************************************/

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__
#pragma once

#ifdef USE_DXINPUT

#include "DXInputDevice.h"

class		CDXInput;
struct	SInputSymbol;

class CKeyboard : public CDXInputDevice
{
	struct SScanCode
	{
		char		lc;	// lowercase
		char		uc;	// uppercase
		char		ac;	// alt gr
		char		cl;	// caps lock (differs slightly from uppercase)
	};
public:
	CKeyboard(CDXInput& input);

	// IInputDevice overrides
	virtual int GetDeviceIndex() const { return 0; }	//Assume only one keyboard
	virtual bool Init();
	virtual void Update(bool bFocus);
	virtual bool SetExclusiveMode(bool value);
	virtual void ClearKeyState();
	virtual char GetInputCharAscii(const SInputEvent& event);
	virtual const wchar_t* GetOSKeyName(const SInputEvent& event);
	virtual void OnLanguageChange();
	// ~IInputDevice

public:	
	unsigned char Event2ASCII(const SInputEvent& event);
	unsigned char ToAscii(unsigned int vKeyCode, unsigned int k, unsigned char sKState[256]) const;
	wchar_t ToUnicode(unsigned int vKeyCode, unsigned int k, unsigned char sKState[256]) const;

protected:
	void	SetupKeyNames();
	void	ProcessKey(uint32 devSpecId, bool pressed);

private:
	static SScanCode			m_scanCodes[256];
	static SInputSymbol*	Symbol[256];

};

#endif // USE_DXINPUT

#endif // __KEYBOARD_H__

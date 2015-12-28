/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2006.
-------------------------------------------------------------------------
$Id$
$DateTime$
-------------------------------------------------------------------------
History:
- Jan 31,2001:	Created by Marco Corbetta
- Dec 01,2005:	Major rewrite by Marco Koegler
- Dec 05,2005:	Rename CInput to CDXInput ... now platform-specific
- Dec 18,2005:	Many refinements and abstraction

TODO:
- GetOSKeyName functionality needs to be restored
- investigate XInput slowdown
- get initial keystate when gaining focus
- Events for (dis)connect
- DirectX support (joystick)
- "Interpretation" Layer
- action-combinations ("xi_a + xi_b")

*************************************************************************/
#include "StdAfx.h"
#include "DXInput.h"

#ifdef USE_DXINPUT

#include <ILog.h>
#include <ISystem.h>
#include <IHardwareMouse.h>
#include <IRenderer.h>
#include "Keyboard.h"
#include "Mouse.h"
#include "XInputDevice.h"
#include "OrbisPadWin.h"
#include "ICmdLine.h"
#include "UnicodeFunctions.h"

CDXInput* CDXInput::This = 0;

CDXInput::CDXInput(ISystem *pSystem, HWND hwnd) : CBaseInput()
{ 
	assert(!This && "CDXInput has been instantiated more than once");

	m_hwnd = hwnd;
	This = this;

	pSystem->RegisterWindowMessageHandler(this);
};

CDXInput::~CDXInput()
{
	gEnv->pSystem->UnregisterWindowMessageHandler(this);

	m_hwnd = NULL;
	This = NULL;
}

bool CDXInput::Init()
{
//	gEnv->pLog->Log("DXInput::Init()\n");
	CBaseInput::Init();

	gEnv->pLog->Log("Initializing DirectInput\n");

	HRESULT hr = DirectInput8Create(GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pDI, 0); 

	if (FAILED(hr)) 
	{
		gEnv->pLog->Log("Cannot initialize DirectInput\n");		
		return false;
	}			

	// add keyboard and mouse
	if (!AddInputDevice(new CKeyboard(*this))) return false;

	if (GetISystem()->GetICmdLine()->FindArg(eCLAT_Pre, "nomouse") == NULL)
	{
		if (!AddInputDevice(new CMouse(*this))) return false;
	}

	// add xinput controllers devices
	for (int i= 0; i < 4; ++i)
	{
		if (!AddInputDevice(new CXInputDevice(*this, i))) return false;
	}

	// add Orbis pad
#ifdef USE_ORBISPAD_WIN
	{
		IInputDevice *pOrbisPad = CreateOrbisPadWin(GetISystem(), *this, 0);
		if (pOrbisPad)
		{
			if (!AddInputDevice(pOrbisPad)) return false;
		}
	}
#endif // #if CRY_USE_ORBISPADWIN

	ClearKeyState();
	return true;	
}

void CDXInput::Update(bool bFocus)
{
	CBaseInput::Update(bFocus);
}


void CDXInput::ShutDown()
{
	gEnv->pLog->Log("DXInput Shutdown\n");

	CBaseInput::ShutDown();

	m_pDI->Release();
	m_pDI= 0;

	delete this;
}

void CDXInput::ClearKeyState()
{
	CBaseInput::ClearKeyState();
	unsigned char sKState[256] = {0};
	wchar_t buff[8];

	// reset DeadKey buffer in windows ... for reference why this is done by calling ToUnicode()
	// see http://blogs.msdn.com/michkap/archive/2005/01/19/355870.aspx
	::ToUnicode('A', DIK_A, sKState, buff, 8, 0);
}

void CDXInput::SetExclusiveMode( EInputDeviceType deviceType, bool exclusive, void *pUser )
{
	if (pUser && (m_hwnd != (HWND)pUser))
	{
		m_hwnd = (HWND)pUser;
	}

	CBaseInput::SetExclusiveMode(deviceType, exclusive, pUser);
}

bool CDXInput::HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	// Check for text input character
	// Note: Text input is separate from "normal" input through CryInput
	uint32 input = 0;
	switch (message)
	{
	case WM_UNICHAR:
		if (wParam == UNICODE_NOCHAR)
		{
			// Indicate compatibility with 32-bit characters
			*pResult = TRUE;
			return true;
		}
		// Fall through intended
	case WM_CHAR:
		input = (uint32)wParam;
		break;

	case WM_KEYDOWN:
		// CTRL+V for clipboard paste support
		if (wParam == 'V')
		{
			const bool bWasDown = (lParam & (1 << 30)) != 0;
			const bool bControl = (GetKeyState(VK_CONTROL) & (1 << 15)) != 0;
			if (!bWasDown && bControl)
			{
				if (OpenClipboard(NULL) != 0)
				{
					wstring data;
					const HANDLE wideData = GetClipboardData(CF_UNICODETEXT);
					if (wideData)
					{
						const LPCWSTR pWideData = (LPCWSTR)GlobalLock(wideData);
						if (pWideData)
						{
							// Note: This conversion is just to make sure we discard malicious or malformed data
							Unicode::ConvertSafe<Unicode::eErrorRecovery_Discard>(data, pWideData);
							GlobalUnlock(wideData);
						}
					}
					CloseClipboard();

					for (Unicode::CIterator<wstring::const_iterator> it(data.begin(), data.end()); it != data.end(); ++it)
					{
						const uint32 cp = *it;
						if (cp != '\r')
						{
							SUnicodeEvent evt(cp);
							PostUnicodeEvent(evt);
						}
					}

					if (wideData)
					{
						*pResult = 0;
						return true;
					}
				}
			}
		}
		// Fall through intended
	default:
		return false;
	}

	// Handle the character input
	*pResult = 0;
	if (input)
	{
		if (IS_HIGH_SURROGATE(input))
		{
			m_highSurrogate = input;
			return true; // Note: This discards subsequent high surrogate's (an encoding error)
		}
		else if (IS_LOW_SURROGATE(input))
		{
			if (m_highSurrogate)
			{
				input &= 0x3FF;
				input |= ((m_highSurrogate & 0x3FF) << 10) | 0x10000;
			}
			else
			{
				// Note: This discards low surrogate without preceding high-surrogate (an encoding error)
				return true;
			}
		}
		m_highSurrogate = 0;
		if (input < 0x110000) // Note: This discards code-points outside the valid range (should never be posted)
		{
			SUnicodeEvent evt(input);
			PostUnicodeEvent(evt);
		}
	}
	return true;
}


#endif //USE_DXINPUT

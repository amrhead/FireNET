// CryEngine Source File.
// Copyright (C), Crytek, 1999-2015.
// Low-level platform specific window message helper
#ifndef _CRY_WINDOW_MESSAGE_HANDLER_H_
#define _CRY_WINDOW_MESSAGE_HANDLER_H_
#include "platform.h"
#if defined(WIN32)
#include <windows.h>

// Summary:
//		Window message handler for Windows OS
struct IWindowMessageHandler
{
	// Summary:
	//		The low-level pre-process message handler for Windows
	//		This is called before TranslateMessage/DispatchMessage (which will eventually end up in the HandleMessage handler)
	//		Typically, do not implement this function, unless you need to see messages before the IME can touch it
	virtual void PreprocessMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {}

	// Summary:
	//		The low-level window message handler for Windows
	//		The return value specifies if the implementation wants to modify the message handling result
	//		When returning true, the desired result value should be written through the pResult pointer
	//		When returning false, the value stored through pResult (if any) is ignored
	//		If more than one implementation write different results, the behavior is undefined
	//		If none of the implementations write any result, the default OS result will be used instead
	//		In general, return false if the handler doesn't care about the message, or only uses it for informational purposes
	virtual bool HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pResult) { return false; }
};
#else
// Summary:
//		Dummy window message handler
//		This is used for platforms that don't use window message handlers
struct IWindowMessageHandler
{
};
#endif
#endif

////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2012.
// -------------------------------------------------------------------------
//  File name:   CryWindows.h
//  Version:     v1.00
//  Created:     02/05/2012 by James Chilvers.
//  Compilers:   Visual Studio.NET
//  Description: Specific header to handle Windows.h include
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef _CRY_WINDOWS_H_
#define _CRY_WINDOWS_H_

#if defined(WIN32) || defined(WIN64) 

#ifndef FULL_WINDOWS_HEADER
	#define WIN32_LEAN_AND_MEAN
#endif

// Do not use WINDOWS SDK min/max
#define NOMINMAX

#include <windows.h>

#undef min
#undef max
#undef GetCommandLine
#undef GetObject
#undef PlaySound
#undef GetClassName
#undef DrawText
#undef GetCharWidth
#undef GetUserName
#undef LoadLibrary

#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN
#undef FULL_WINDOWS_HEADER

#endif

#endif // _CRY_WINDOWS_H_

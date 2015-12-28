////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2013.
// -------------------------------------------------------------------------
//  File name:   CryPlatform.h
//  Version:     v1.00
//  Created:     31/01/2013 by Christopher Bolte.
//  Compilers:   Visual Studio.NET
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef _CRY_PLATFORM_DARWIN_H_
#define _CRY_PLATFORM_DARWIN_H_


////////////////////////////////////////////////////////////////////////////
// check that we are allowed to be included
#if !defined(CRYPLATFROM_ALLOW_DETAIL_INCLUDES)
#	error Please include CryPlatfrom.h instead of this private implementation header
#endif


////////////////////////////////////////////////////////////////////////////
// size and alignment settings for platfrom specific primities
////////////////////////////////////////////////////////////////////////////
// Interlocked singled linked list settings
#define CRYPLATFORM_INTERLOCKEDSLIST_HEADER_SIZE				16
#define CRYPLATFORM_INTERLOCKEDSLIST_HEADER_ALIGNMENT		16
#define CRYPLATFORM_INTERLOCKEDSLIST_ELEMENT_SIZE				8
#define CRYPLATFORM_INTERLOCKEDSLIST_ELEMENT_ALIGNMENT	16


////////////////////////////////////////////////////////////////////////////
// macros for platfrom specific functionality which cannot be expressed as a C++ function
////////////////////////////////////////////////////////////////////////////
// mark a structure as aligned
#define ALIGN_STRUCTURE( alignment, structure ) structure __attribute__ ((aligned(alignment)))
// cause a the debugger to break if executed
//#define DEBUG_BREAK do { __debugbreak(); } while(0)


////////////////////////////////////////////////////////////////////////////
// util macros for __DETAIL__LINK_THIRD_PARTY_LIBRARY and __DETAIL__LINK_SYSTEM_PARTY_LIBRARY
// SNC is a little strange with macros and pragmas
// the lib pragma requieres a string literal containing a escaped string literal
// eg _Pragma ("comment (lib, \"<lib>\")")
#define __DETAIL__CREATE_PRAGMA(x) _Pragma(CRY_CREATE_STRING(x))

////////////////////////////////////////////////////////////////////////////
// Create a string from an Preprocessor macro or a literal text
#define CRY_DETAIL_CREATE_STRING( string ) #string
#define CRY_CREATE_STRING( string ) CRY_DETAIL_CREATE_STRING(string)

////////////////////////////////////////////////////////////////////////////
#define __DETAIL__LINK_THIRD_PARTY_LIBRARY( name ) \
	__DETAIL__CREATE_PRAGMA("comment(lib, \"" CRY_CREATE_STRING(CODE_BASE_FOLDER) CRY_CREATE_STRING(name) "\")")

////////////////////////////////////////////////////////////////////////////
#define __DETAIL__LINK_SYSTEM_PARTY_LIBRARY( name ) \
	__DETAIL__CREATE_PRAGMA("comment(lib, \"" CRY_CREATE_STRING(name) "\")")

#endif // _CRY_PLATFORM_DARWIN_H_
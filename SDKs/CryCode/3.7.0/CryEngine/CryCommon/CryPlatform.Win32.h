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
#ifndef _CRY_PLATFORM_WIN32_H_
#define _CRY_PLATFORM_WIN32_H_


////////////////////////////////////////////////////////////////////////////
// check that we are allowed to be included
#if !defined(CRYPLATFROM_ALLOW_DETAIL_INCLUDES)
#	error Please include CryPlatfrom.h instead of this private implementation header
#endif


////////////////////////////////////////////////////////////////////////////
// size and alignment settings for platfrom specific primities
////////////////////////////////////////////////////////////////////////////
// Interlocked singled linked list settings
#define CRYPLATFORM_INTERLOCKEDSLIST_HEADER_SIZE				4
#define CRYPLATFORM_INTERLOCKEDSLIST_HEADER_ALIGNMENT		8
#define CRYPLATFORM_INTERLOCKEDSLIST_ELEMENT_SIZE				4
#define CRYPLATFORM_INTERLOCKEDSLIST_ELEMENT_ALIGNMENT	4


////////////////////////////////////////////////////////////////////////////
// macros for platfrom specific functionality which cannot be expressed as a C++ function
////////////////////////////////////////////////////////////////////////////
// mark a structure as aligned
#define ALIGN_STRUCTURE( alignment, structure ) __declspec(align(alignment)) structure

////////////////////////////////////////////////////////////////////////////
// Create a string from an Preprocessor macro or a literal text
#define CRY_DETAIL_CREATE_STRING( string ) #string
#define CRY_CREATE_STRING( string ) CRY_DETAIL_CREATE_STRING(string)
#define RESOLVE_MACRO(x) x

////////////////////////////////////////////////////////////////////////////
#define __DETAIL__LINK_THIRD_PARTY_LIBRARY( name ) \
	__pragma( message( __FILE__ "(" CRY_CREATE_STRING(__LINE__) "): Including SDK Library: " CRY_CREATE_STRING(name) )) \
	__pragma( comment( lib, RESOLVE_MACRO(CODE_BASE_FOLDER) CRY_CREATE_STRING(name)) )

////////////////////////////////////////////////////////////////////////////
#define __DETAIL__LINK_SYSTEM_PARTY_LIBRARY( name ) \
	__pragma( message( __FILE__ "(" CRY_CREATE_STRING(__LINE__) "): Including System Library: " CRY_CREATE_STRING(name) )) \
	__pragma( comment( lib, CRY_CREATE_STRING(name)) )

#endif // _CRY_PLATFORM_WIN32_H_

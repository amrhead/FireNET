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
#ifndef _CRY_PLATFORM_FORWARD_DECLARATIONS_H_
#define _CRY_PLATFORM_FORWARD_DECLARATIONS_H_

////////////////////////////////////////////////////////////////////////////
// check that we are allowed to be included
#if !defined(CRYPLATFROM_ALLOW_DETAIL_INCLUDES)
#	error Please include CryPlatfrom.h instead of this private implementation header
#endif


////////////////////////////////////////////////////////////////////////////
// util macro to create opaque objects which a specific size and alignment
#define DEFINE_OPAQUE_TYPE( name, size, alignment ) ALIGN_STRUCTURE( alignment, struct name { private: char pad[size]; } )


////////////////////////////////////////////////////////////////////////////
// Multithreading primitive functions
////////////////////////////////////////////////////////////////////////////
namespace CryMT {
	
	////////////////////////////////////////////////////////////////////////////
	// structure forward declarations
	struct SInterlockedSListHeader;
	struct SInterlockedSListElement;

	////////////////////////////////////////////////////////////////////////////
	// function forward declarations
	namespace detail
	{
		////////////////////////////////////////////////////////////////////////////
		// interlocked single linked list functions
		void InterlockedSListInitialize( SInterlockedSListHeader *pHeader );	
		void InterlockedSListFlush( SInterlockedSListHeader *pHeader );
		void InterlockedSListPush( SInterlockedSListHeader *pHeader, SInterlockedSListElement *pElement );
		SInterlockedSListElement* InterlockedSListPop( SInterlockedSListHeader *pHeader );

	} // namespace detail

} // namespace CryMT

////////////////////////////////////////////////////////////////////////////
#endif // _CRY_PLATFORM_FORWARD_DECLARATIONS_H_
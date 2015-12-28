////////////////////////////////////////////////////////////////////////////////////////////
//
//  CryEngine Source File.
//  Copyright (C), Crytek, 1999-2014.
// ----------------------------------------------------------------------------------------
//  File name:   IGameFrameworkExtension.h
//  Description: Interface to create game framework extension
//
//////////////////////////////////////////////////////////////////////////////////////////// 

#ifndef __IGameFrameworkExtension_h__
#define __IGameFrameworkExtension_h__
#pragma once

#include <CryExtension/ICryUnknown.h>

struct IGameFramework;

struct IGameFrameworkExtensionCreator : public ICryUnknown
{
	CRYINTERFACE_DECLARE(IGameFrameworkExtensionCreator, 0x86197E35AD024DA8, 0xA8D483A98F424FFD);

	// Description
	//  Creates an extension and returns the interface to it (extension interface must derivate for ICryUnknown) 
	// Parameters
	//  pIGameFramework - Pointer to game framework interface, so the new extension can be registered against it
	// Return
	//  ICryUnknown pointer to just created extension (it can be safely casted with cryinterface_cast< > to the corresponding interface)
	virtual ICryUnknown* Create( IGameFramework* pIGameFramework ) = 0;
};
DECLARE_BOOST_POINTERS(IGameFrameworkExtensionCreator);

#endif //__IGameFrameworkExtension_h__
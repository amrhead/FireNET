// Copyright (c) 2012 Crytek GmbH
// Authors: Evgeny Andreeshchev, Alexander Kotliar
// Based on: Yasli - the serialization library.

#pragma once
#include "IArchive.h"
#include "IClassFactory.h"
#include "STL.h"
#include "ClassFactory.h"
#include "Strings.h"

namespace Serialization{

inline bool Serialize(Serialization::IArchive& ar, Serialization::TypeNameWithFactory& value, const char* name, const char* label)
{
	if(!ar(value.registeredName, name))
		return false;

	if (ar.IsInput()){
		const TypeDescription* desc = value.factory->descriptionByRegisteredName(value.registeredName.c_str());
		if(!desc){
			ar.Error(value, "Unable to read TypeID: unregistered type name: \'%s\'", value.registeredName.c_str());
			value.registeredName.clear();
			return false;
		}
	}
	return true;
}

}


// Copyright (c) 2012 Crytek GmbH
// Authors: Evgeny Andreeshchev, Alexander Kotliar
// Based on: Yasli - the serialization library.
#pragma once

#include "Serializer.h"
#include "IClassFactory.h"
#include "ClassFactory.h"

// IArchive.h is supposed to be pre-included

namespace Serialization {

inline bool SStruct::operator()(IArchive& ar) const{
	YASLI_ESCAPE(serializeFunc_ && object_, return false);
	return serializeFunc_(object_, ar);
}

inline bool SStruct::operator()(IArchive& ar, const char* name, const char* label) const{
	return ar(*this, name, label);
}


inline void IPointer::Serialize(IArchive& ar) const
{
	const bool noEmptyNames = ar.GetCaps(IArchive::NO_EMPTY_NAMES);
	const char* const typePropertyName = noEmptyNames ? "type" : "";
	const char* const dataPropertyName = noEmptyNames ? "data" : "";

	TypeID baseTypeID = baseType();
	const char* oldRegisteredName = registeredTypeName();
	if (!oldRegisteredName)
		oldRegisteredName = "";
	IClassFactory* factory = this->factory();

	if(ar.IsOutput()){
		if(oldRegisteredName[0] != '\0'){
			TypeNameWithFactory pair(oldRegisteredName, factory);
			if(ar(pair, typePropertyName)){
				ar(serializer(), dataPropertyName);
			}
			else
				ar.Warning(pair, "Unable to write typeID!");
		}
	}
	else{
		TypeNameWithFactory pair("", factory);
		if(!ar(pair, typePropertyName)){
			if(oldRegisteredName[0] != '\0'){
				create(""); // 0
			}
			return;
		}

		if(oldRegisteredName[0] != '\0' && (pair.registeredName.empty() || (pair.registeredName != oldRegisteredName)))
			create(""); // 0

		if(!pair.registeredName.empty()){
			if(!get())
				create(pair.registeredName.c_str());
			ar(serializer(), dataPropertyName);
		}
	}	
}

}
// vim:sw=4 ts=4:

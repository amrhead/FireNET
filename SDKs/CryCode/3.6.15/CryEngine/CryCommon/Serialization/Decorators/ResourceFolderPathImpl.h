#pragma once

#include "Serialization/IArchive.h"

namespace Serialization
{

inline bool Serialize(Serialization::IArchive& ar, Serialization::ResourceFolderPath& value, const char* name, const char* label)
{
	if (ar.IsEdit())
		return ar(Serialization::SStruct::ForEdit(value), name, label);
	else
		return ar(*value.path, name, label);
}

}
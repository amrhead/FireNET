#pragma once

#include "LocalFrame.h"
#include "Serialization/IArchive.h"
#include "Serialization/MathImpl.h"

namespace Serialization
{

inline void LocalPosition::Serialize(Serialization::IArchive& ar)
{
	ar(value->x, "x", "^");
	ar(value->y, "y", "^");
	ar(value->z, "z", "^");
}

inline void LocalOrientation::Serialize(Serialization::IArchive& ar)
{
	ar(Serialization::AsAng3(*value), "q", "^");
}

inline void LocalFrame::Serialize(Serialization::IArchive& ar)
{
	ar(*position, "t", "<T");
	ar(AsAng3(*rotation), "q", "<R");
}

inline bool Serialize(Serialization::IArchive& ar, Serialization::LocalPosition& value, const char* name, const char* label)
{
	if (ar.IsEdit())
		return ar(Serialization::SStruct(value), name, label);
	else
		return ar(*value.value, name, label);
}

inline bool Serialize(Serialization::IArchive& ar, Serialization::LocalOrientation& value, const char* name, const char* label)
{
	if (ar.IsEdit())
		return ar(Serialization::SStruct(value), name, label);
	else
		return ar(*value.value, name, label);
}

inline bool Serialize(Serialization::IArchive& ar, Serialization::LocalFrame& value, const char* name, const char* label)
{
	if (ar.IsEdit())
		return ar(Serialization::SStruct(value), name, label);
	else
	{
		QuatT t(*value.rotation, *value.position);
		if (!ar(t, name, label))
			return false;
		if (ar.IsInput())
		{
			*value.position = t.t;
			*value.rotation = t.q;
		}
		return true;
	}
}

}

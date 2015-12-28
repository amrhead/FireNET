// Copyright (c) 2013 Crytek GmbH
#pragma once

#include <Serialization/IArchive.h>
#include <Serialization/Decorators/Range.h>

template<typename T>
inline bool Serialize(Serialization::IArchive& ar, Color_tpl<T>& c, const char* name, const char* label);

namespace Serialization
{
	struct Vec3AsColor
	{
		Vec3& v;
		Vec3AsColor(Vec3& v) : v(v) {} 

		void Serialize(Serialization::IArchive& ar)
		{
			ar(Range(v.x, 0.0f, 1.0f), "r", "^");
			ar(Range(v.y, 0.0f, 1.0f), "g", "^");
			ar(Range(v.z, 0.0f, 1.0f), "b", "^");
		}
	};

	inline bool Serialize(Serialization::IArchive& ar, Vec3AsColor& c, const char* name, const char* label)
	{
		if (ar.IsEdit())
			return ar(Serialization::SStruct(c), name, label);
		else
		{
			typedef float(*Array)[3];
			return ar(*((Array)&c.v.x), name, label);
		}
	}
}

#include "ColorImpl.h"

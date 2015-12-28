// Copyright (c) 2013 Crytek GmbH
#pragma once

#include "Color.h"
#include "Cry_Color.h"

//////////////////////////////////////////////////////////////////////////
template<typename T>
struct SerializableColor_tpl : Color_tpl<T>
{
	static float ColorRangeMin(float) { return 0.0f; }
	static float ColorRangeMax(float) { return 1.0f; }
	static unsigned char ColorRangeMin(unsigned char) { return 0; }
	static unsigned char ColorRangeMax(unsigned char) { return 255; }

	void Serialize(Serialization::IArchive& ar)
	{
		ar(Serialization::Range(Color_tpl<T>::r, ColorRangeMin(Color_tpl<T>::r), ColorRangeMax(Color_tpl<T>::r)), "r", "^");
		ar(Serialization::Range(Color_tpl<T>::g, ColorRangeMin(Color_tpl<T>::g), ColorRangeMax(Color_tpl<T>::g)), "g", "^");
		ar(Serialization::Range(Color_tpl<T>::b, ColorRangeMin(Color_tpl<T>::b), ColorRangeMax(Color_tpl<T>::b)), "b", "^");
		ar(Serialization::Range(Color_tpl<T>::a, ColorRangeMin(Color_tpl<T>::a), ColorRangeMax(Color_tpl<T>::a)), "a", "^");
	}
};

template<typename T>
bool Serialize(Serialization::IArchive& ar, Color_tpl<T>& c, const char* name, const char* label)
{
	if (ar.IsEdit())
		return Serialize(ar, static_cast<SerializableColor_tpl<T>&>(c), name, label);
	else
	{
		typedef T(&Array)[4];
		return ar((Array)c, name, label);
	}
}

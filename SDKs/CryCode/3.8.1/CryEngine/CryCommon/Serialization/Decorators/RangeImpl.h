// Copyright (c) 2012 Crytek GmbH
#pragma once

namespace Serialization
{

template<class T>
bool Serialize(IArchive& ar, RangeDecorator<T>& value, const char* name, const char* label)
{
	if (ar.IsEdit())
	{
		if (!ar(SStruct::ForEdit(value), name, label))
			return false;
	}
	else if (!ar(*value.value, name, label))
		return false;

	if (ar.IsInput())
	{
		if (*value.value < value.hardMin)
			*value.value = value.hardMin;
		if (*value.value > value.hardMax)
			*value.value = value.hardMax;
	}
	return true;
}

}

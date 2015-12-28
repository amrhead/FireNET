// Copyright (c) 2014 Crytek GmbH
// Authors: Sebastien Laurent
// Based on: Yasli - the serialization library.

#pragma once

#include "CryFixedString.h"

#include "Serialization/Serializer.h"

namespace Serialization
{
	class IArchive;
}

// Note : if you are looking for the CryStringT serialization, it is handled in Serialization/STL.h

template< size_t N >
bool Serialize(Serialization::IArchive& ar, CryFixedStringT< N >& value, const char* name, const char* label);

template< size_t N >
bool Serialize(Serialization::IArchive& ar, CryStackStringT< char, N >& value, const char* name, const char* label);

template< size_t N >
bool Serialize(Serialization::IArchive& ar, CryStackStringT< wchar_t, N >& value, const char* name, const char* label);

#include "Serialization/CryStringsImpl.h"

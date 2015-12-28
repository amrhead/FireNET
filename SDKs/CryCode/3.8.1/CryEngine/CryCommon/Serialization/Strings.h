// Copyright (c) 2012 Crytek GmbH
// Authors: Evgeny Andreeshchev, Alexander Kotliar
// Based on: Yasli - the serialization library.
#pragma once

#ifdef SERIALIZATION_STANDALONE
#include <string>
namespace Serialization {
	using std::string;
	using std::wstring;
}
#else
#include <platform.h>

namespace Serialization {
#ifdef RESOURCE_COMPILER
typedef CryStringLocalT<char> string;
typedef CryStringLocalT<wchar_t> wstring;
#else
typedef CryStringT<char> string;
typedef CryStringT<wchar_t> wstring;
#endif
}
#endif

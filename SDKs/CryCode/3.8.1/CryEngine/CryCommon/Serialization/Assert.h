// Copyright (c) 2012 Crytek GmbH
// Authors: Evgeny Andreeshchev, Alexander Kotliar
// Based on: Yasli - the serialization library.

#pragma once 
#ifdef SERIALIZATION_STANDALONE
#include <assert.h>
#else
#include <platform.h>
#endif

#ifdef YASLI_ASSERT
# undef YASLI_ASSERT
#endif

#ifdef YASLI_VERIFY
# undef YASLI_VERIFY
#endif

#ifdef YASLI_ESCAPE
# undef YASLI_ESCAPE
#endif

#ifdef SERIALIZATION_STANDALONE
#define YASLI_ASSERT(x) assert(x)
#define YASLI_ASSERT_STR(x, str) assert(x && str)
#define YASLI_ESCAPE(x, action) if(!(x)) { YASLI_ASSERT(0 && #x); action; };
#else
#define YASLI_ASSERT(x) CRY_ASSERT(x)
#define YASLI_ASSERT_STR(x, str) CRY_ASSERT_MESSAGE(x, str)
#define YASLI_ESCAPE(x, action) if(!(x)) { YASLI_ASSERT(0 && #x); action; };
#endif

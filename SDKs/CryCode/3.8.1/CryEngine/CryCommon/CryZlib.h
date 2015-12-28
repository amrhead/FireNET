#pragma once

// Include this file instead of including zlib.h directly 
// because zconf.h (included by zlib.h) defines WINDOWS and WIN32 - those
// definitions conflict with CryEngine's definitions.

#if defined(CRY_TMP_DEFINED_WINDOWS) || defined(CRY_TMP_DEFINED_WIN32)
#	error CRY_TMP_DEFINED_WINDOWS and/or CRY_TMP_DEFINED_WIN32 already defined
#endif

#if defined(WINDOWS)
#	define CRY_TMP_DEFINED_WINDOWS 1
#endif
#if defined(WIN32)
#	define CRY_TMP_DEFINED_WIN32 1
#endif

#include <zlib.h>

#if !defined(CRY_TMP_DEFINED_WINDOWS)
#  undef WINDOWS
#endif
#undef CRY_TMP_DEFINED_WINDOWS
#if !defined(CRY_TMP_DEFINED_WIN32)
#	undef WIN32
#endif
#undef CRY_TMP_DEFINED_WIN32

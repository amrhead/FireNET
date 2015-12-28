// Generic inline implementations of miscellaneous string manipulation utilities,
// including path manipulation.
//
// Created by Sergiy Migdalskiy on 10th Feb 2003
// Modified by Sergey Sokov in January 2015 
// Copyright (c) 1999-2015 Crytek.

#ifndef _CRY_ENGINE_STRING_UTILS_HDR_
#define _CRY_ENGINE_STRING_UTILS_HDR_

#pragma once

#include "CryString.h"
#include <algorithm>  // std::replace, std::min
#include "UnicodeFunctions.h"
#include "UnicodeIterator.h"

#if !defined(RESOURCE_COMPILER)
#	include "CryCrc32.h"
#endif

#if defined(LINUX) || defined(APPLE)
#	include <ctype.h>
#endif

namespace CryStringUtils
{
	// Convert a single ASCII character to lower case, this is compatible with the standard "C" locale (ie, only A-Z).
	inline char toLowerAscii(char c)
	{
		return (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;
	}

	// Convert a single ASCII character to upper case, this is compatible with the standard "C" locale (ie, only A-Z).
	inline char toUpperAscii(char c)
	{
		return (c >= 'a' && c <= 'z') ? (c - 'a' + 'A') : c;
	}
}

// cry_strXXX() and CryStringUtils_Internal::strXXX():
//
// The functions copy characters from src to dst one by one until any of
// the following conditions is met:
//   1) the end of the destination buffer (minus one character) is reached.
//   2) the end of the source buffer is reached.
//   3) zero character is found in the source buffer.
//
// When any of 1), 2), 3) happens, the functions write the terminating zero
// character to the destination buffer and return.
//
// The functions guarantee writing the terminating zero character to the
// destination buffer (if the buffer can fit at least one character).
//
// The functions return false when a null pointer is passed or when
// clamping happened (i.e. when the end of the destination buffer is
// reached but the source has some characters left).

namespace CryStringUtils_Internal
{

template <class TChar>
inline bool strcpy_with_clamp(TChar* const dst, size_t const dst_size_in_bytes, const TChar* const src, size_t const src_size_in_bytes)
{
	COMPILE_TIME_ASSERT(sizeof(TChar) == sizeof(char) || sizeof(TChar) == sizeof(wchar_t));

	if (!dst || dst_size_in_bytes < sizeof(TChar))
	{
		return false;
	}

	if (!src || src_size_in_bytes < sizeof(TChar))
	{
		dst[0] = 0;
		return src != 0;  // we return true for non-null src without characters
	}

	const size_t src_n = src_size_in_bytes / sizeof(TChar);
	const size_t n = (std::min)(dst_size_in_bytes / sizeof(TChar) - 1, src_n);

	for (size_t i = 0; i < n; ++i)
	{
		dst[i] = src[i];
		if (!src[i])
		{
			return true;
		}
	}

	dst[n] = 0;
	return n >= src_n || src[n] == 0;
}

template <class TChar>
inline bool strcat_with_clamp(TChar* const dst, size_t const dst_size_in_bytes, const TChar* const src, size_t const src_size_in_bytes)
{
	COMPILE_TIME_ASSERT(sizeof(TChar) == sizeof(char) || sizeof(TChar) == sizeof(wchar_t));

	if (!dst || dst_size_in_bytes < sizeof(TChar))
	{
		return false;
	}

	const size_t dst_n = dst_size_in_bytes / sizeof(TChar) - 1;

	size_t dst_len = 0;
	while (dst_len < dst_n && dst[dst_len])
	{
		++dst_len;
	}

	if (!src || src_size_in_bytes < sizeof(TChar))
	{
		dst[dst_len] = 0;
		return src != 0;  // we return true for non-null src without characters
	}

	const size_t src_n = src_size_in_bytes / sizeof(TChar);
	const size_t n = (std::min)(dst_n - dst_len, src_n);
	TChar* dst_ptr = &dst[dst_len];

	for (size_t i = 0; i < n; ++i)
	{
		*dst_ptr++ = src[i];
		if (!src[i])
		{
			return true;
		}
	}

	*dst_ptr = 0;
	return n >= src_n || src[n] == 0;
}

// Compares characters as case-sensitive, locale agnostic.
template<class CharType>
struct SCharComparatorCaseSensitive
{
	static bool IsEqual(CharType a, CharType b)
	{
		return a == b;
	}
};

// Compares characters as case-insensitive, uses the standard "C" locale.
template<class CharType>
struct SCharComparatorCaseInsensitive
{
	static bool IsEqual(CharType a, CharType b)
	{
		if (a < 0x80 && b < 0x80)
		{
			a = (CharType)CryStringUtils::toLowerAscii(char(a));
			b = (CharType)CryStringUtils::toLowerAscii(char(b));
		}
		return a == b;
	}
};

// Template for wildcard matching, UCS code-point aware.
// Can be used for ASCII and Unicode (UTF-8/UTF-16/UTF-32), but not for ANSI.
// ? will match exactly one code-point.
// * will match zero or more code-points.
// Note: function moved here from CryCommonTools.
template <template<class> class CharComparator, class CharType>
static inline bool MatchesWildcards_Tpl(const CharType* pStr, const CharType* pWild)
{
	const CharType* savedStr = 0;
	const CharType* savedWild = 0;

	while ((*pStr) && (*pWild != '*')) 
	{
		if (!CharComparator<CharType>::IsEqual(*pWild, *pStr) && (*pWild != '?'))
		{
			return false;
		}

		// We need special handling of '?' for Unicode
		if (*pWild == '?' && *pStr > 127)
		{
			Unicode::CIterator<const CharType *> utf(pStr, pStr + 4);
			if (utf.IsAtValidCodepoint())
			{
				pStr = (++utf).GetPosition();
				--pStr;
			}
		}

		++pWild;
		++pStr;
	}

	while (*pStr) 
	{
		if (*pWild == '*') 
		{
			if (!*++pWild) 
			{
				return true;
			}
			savedWild = pWild;
			savedStr = pStr + 1;
		} 
		else if (CharComparator<CharType>::IsEqual(*pWild, *pStr) || (*pWild == '?')) 
		{
			// We need special handling of '?' for Unicode
			if (*pWild == '?' && *pStr > 127)
			{
				Unicode::CIterator<const CharType *> utf(pStr, pStr + 4);
				if (utf.IsAtValidCodepoint())
				{
					pStr = (++utf).GetPosition();
					--pStr;
				}
			}

			++pWild;
			++pStr;
		} 
		else 
		{
			pWild = savedWild;
			pStr = savedStr++;
		}
	}

	while (*pWild == '*') 
	{
		++pWild;
	}

	return *pWild == 0;
}

} // namespace CryStringUtils_Internal


inline bool cry_strcpy(char* const dst, size_t const dst_size_in_bytes, const char* const src)
{
	return CryStringUtils_Internal::strcpy_with_clamp<char>(dst, dst_size_in_bytes, src, (size_t)-1);
}

inline bool cry_strcpy(char* const dst, size_t const dst_size_in_bytes, const char* const src, size_t const src_size_in_bytes)
{
	return CryStringUtils_Internal::strcpy_with_clamp<char>(dst, dst_size_in_bytes, src, src_size_in_bytes);
}

template <size_t SIZE_IN_CHARS>
inline bool cry_strcpy(char (&dst)[SIZE_IN_CHARS], const char* const src)
{
	return CryStringUtils_Internal::strcpy_with_clamp<char>(dst, SIZE_IN_CHARS, src, (size_t)-1);
}

template <size_t SIZE_IN_CHARS>
inline bool cry_strcpy(char (&dst)[SIZE_IN_CHARS], const char* const src, size_t const src_size_in_bytes)
{
	return CryStringUtils_Internal::strcpy_with_clamp<char>(dst, SIZE_IN_CHARS, src, src_size_in_bytes);
}


inline bool cry_wstrcpy(wchar_t* const dst, size_t const dst_size_in_bytes, const wchar_t* const src)
{
	return CryStringUtils_Internal::strcpy_with_clamp<wchar_t>(dst, dst_size_in_bytes, src, (size_t)-1);
}

inline bool cry_wstrcpy(wchar_t* const dst, size_t const dst_size_in_bytes, const wchar_t* const src, size_t const src_size_in_bytes)
{
	return CryStringUtils_Internal::strcpy_with_clamp<wchar_t>(dst, dst_size_in_bytes, src, src_size_in_bytes);
}

template <size_t SIZE_IN_WCHARS>
inline bool cry_wstrcpy(wchar_t (&dst)[SIZE_IN_WCHARS], const wchar_t* const src)
{
	return CryStringUtils_Internal::strcpy_with_clamp<wchar_t>(dst, SIZE_IN_WCHARS * sizeof(wchar_t), src, (size_t)-1);
}

template <size_t SIZE_IN_WCHARS>
inline bool cry_wstrcpy(wchar_t (&dst)[SIZE_IN_WCHARS], const wchar_t* const src, size_t const src_size_in_bytes)
{
	return CryStringUtils_Internal::strcpy_with_clamp<wchar_t>(dst, SIZE_IN_WCHARS * sizeof(wchar_t), src, src_size_in_bytes);
}


inline bool cry_strcat(char* const dst, size_t const dst_size_in_bytes, const char* const src)
{
	return CryStringUtils_Internal::strcat_with_clamp<char>(dst, dst_size_in_bytes, src, (size_t)-1);
}

inline bool cry_strcat(char* const dst, size_t const dst_size_in_bytes, const char* const src, size_t const src_size_in_bytes)
{
	return CryStringUtils_Internal::strcat_with_clamp<char>(dst, dst_size_in_bytes, src, src_size_in_bytes);
}

template <size_t SIZE_IN_CHARS>
inline bool cry_strcat(char (&dst)[SIZE_IN_CHARS], const char* const src)
{
	return CryStringUtils_Internal::strcat_with_clamp<char>(dst, SIZE_IN_CHARS, src, (size_t)-1);
}

template <size_t SIZE_IN_CHARS>
inline bool cry_strcat(char (&dst)[SIZE_IN_CHARS], const char* const src, size_t const src_size_in_bytes)
{
	return CryStringUtils_Internal::strcat_with_clamp<char>(dst, SIZE_IN_CHARS, src, src_size_in_bytes);
}


inline bool cry_wstrcat(wchar_t* const dst, size_t const dst_size_in_bytes, const wchar_t* const src)
{
	return CryStringUtils_Internal::strcat_with_clamp<wchar_t>(dst, dst_size_in_bytes, src, (size_t)-1);
}

inline bool cry_wstrcat(wchar_t* const dst, size_t const dst_size_in_bytes, const wchar_t* const src, size_t const src_size_in_bytes)
{
	return CryStringUtils_Internal::strcat_with_clamp<wchar_t>(dst, dst_size_in_bytes, src, src_size_in_bytes);
}

template <size_t SIZE_IN_WCHARS>
inline bool cry_wstrcat(wchar_t (&dst)[SIZE_IN_WCHARS], const wchar_t* const src)
{
	return CryStringUtils_Internal::strcat_with_clamp<wchar_t>(dst, SIZE_IN_WCHARS * sizeof(wchar_t), src, (size_t)-1);
}

template <size_t SIZE_IN_WCHARS>
inline bool cry_wstrcat(wchar_t (&dst)[SIZE_IN_WCHARS], const wchar_t* const src, size_t const src_size_in_bytes)
{
	return CryStringUtils_Internal::strcat_with_clamp<wchar_t>(dst, SIZE_IN_WCHARS * sizeof(wchar_t), src, src_size_in_bytes);
}


namespace CryStringUtils
{

enum
{
	CRY_DEFAULT_HASH_SEED = 40503, // This is a large 16 bit prime number (perfect for seeding)
};

// Removes the extension from the file path by truncating the string.
// Returns a pointer to the removed extension (if found, without the .), or NULL otherwise.
// Note: If the file has multiple extensions, only the last extension is removed.
// This function is Unicode agnostic and locale agnostic.
inline char* StripFileExtension (char * szFilePath)
{
	for (char* p = szFilePath + (int)strlen(szFilePath)-1; p >= szFilePath; --p)
	{
		switch(*p)
		{
		case ':':
		case '/':
		case '\\':
			// we've reached a path separator - it means there's no extension in this name
			return NULL;
		case '.':
			// there's an extension in this file name
			*p = '\0';
			return p+1;
		}
	}
	// it seems the file name is a pure name, without path or extension
	return NULL;
}
 

// Returns the parent directory of the given file or directory.
// The returned path is WITHOUT the trailing slash.
// If the input path has a trailing slash, it's ignored.
// nGeneration - is the number of parents to scan up.
// Note: A drive specifier (if any) will always be kept (Windows-specific).
// Note: If the specified path does not contain enough folders to satisfy the request, an empty string is returned.
// This function is Unicode agnostic and locale agnostic.
template <class StringCls>
StringCls GetParentDirectory (const StringCls& strFilePath, int nGeneration = 1)
{
	for (const char* p = strFilePath.c_str() + strFilePath.length() - 2; // -2 is for the possible trailing slash: there always must be some trailing symbol which is the file/directory name for which we should get the parent
		p >= strFilePath.c_str();
		--p)
	{
		switch (*p)
		{
		case ':':
			return StringCls (strFilePath.c_str(), p);
			break;
		case '/':
		case '\\':
			// we've reached a path separator - return everything before it.
			if (!--nGeneration)
				return StringCls(strFilePath.c_str(), p);
			break;
		}
	};
	// it seems the file name is a pure name, without path or extension
	return StringCls();
}

// Converts all ASCII characters to lower case.
// Note: Any non-ASCII characters are left unchanged.
// This function is ASCII-only and locale agnostic.
inline string toLower (const string& str)
{
	string temp = str;

#ifndef NOT_USE_CRY_STRING
	temp.MakeLower();
#else
	std::transform(temp.begin(),temp.end(),temp.begin(),toLowerAscii); // STL MakeLower
#endif

	return temp;
}

// Converts all ASCII characters to upper case.
// Note: Any non-ASCII characters are left unchanged.
// This function is ASCII-only and locale agnostic.
inline string toUpper (const string& str)
{
	string temp = str;

#ifndef NOT_USE_CRY_STRING
	temp.MakeUpper();
#else
	std::transform(temp.begin(),temp.end(),temp.begin(),toUpperAscii); // STL MakeLower
#endif

	return temp;
}

// Searches and returns the pointer to the extension of the given file.
// If no extension is found, the function returns a pointer to the terminating NULL (ie, empty string).
// Note: Do not pass a full path, since the function does not account for drives and directories.
// This function is Unicode agnostic and locale agnostic.
inline const char* FindExtension (const char* szFileName)
{
	const char* szEnd = szFileName + (int)strlen(szFileName);
	for (const char* p = szEnd-1; p >= szFileName; --p)
		if (*p == '.')
			return p+1;

	return szEnd;
}

// Searches and returns the pointer to the file name in the given file path.
// Note: This function assumes the provided path contains a filename, if it doesn't it returns the parent directory (or an empty string if the path ends with a slash).
// This function is Unicode agnostic and locale agnostic.
inline const char* FindFileNameInPath (const char* szFilePath)
{
	for (const char* p = szFilePath + (int)strlen(szFilePath)-1; p >= szFilePath; --p)
		if (*p == '\\' || *p == '/')
			return p+1;
	return szFilePath;
}

// Works like strstr, but is case-insensitive.
// This function does not perform Unicode collation and uses the current CRT locale to perform case conversion.
inline const char* stristr(const char* szString, const char* szSubstring)
{
	int nSuperstringLength = (int)strlen(szString);
	int nSubstringLength = (int)strlen(szSubstring);

	for (int nSubstringPos = 0; nSubstringPos <= nSuperstringLength - nSubstringLength; ++nSubstringPos)
	{
		if (strnicmp(szString+nSubstringPos, szSubstring, nSubstringLength) == 0)
			return szString+nSubstringPos;
	}
	return NULL;
}

#ifndef NOT_USE_CRY_STRING

// Converts \ to / and replaces ASCII characters to lower-case (A-Z only).
// This function is ASCII-only and Unicode agnostic.
inline void UnifyFilePath(stack_string& strPath)
{
	strPath.replace('\\', '/');
	strPath.MakeLower();
}

template<size_t SIZE>
inline void UnifyFilePath(CryStackStringT<char,SIZE>& strPath)
{
	strPath.replace('\\', '/');
	strPath.MakeLower();
}

inline void UnifyFilePath(string& strPath)
{
	strPath.replace('\\', '/');
	strPath.MakeLower();
}

#endif

// Converts the number to a string.
// These functions are Unicode agnostic and locale agnostic (integral) or uses the current CRT locale (float, vector, quat, matrix).
inline string toString(unsigned nNumber)
{
	char szNumber[16];
	sprintf (szNumber, "%u", nNumber);
	return szNumber;
}

inline string toString(signed int nNumber)
{
	char szNumber[16];
	sprintf (szNumber, "%d", nNumber);
	return szNumber;
}

inline string toString(float nNumber)
{
	char szNumber[128];
	sprintf(szNumber, "%f", nNumber);
	return szNumber;
}

inline string toString(bool nNumber)
{
	char szNumber[4];
	sprintf (szNumber, "%i", (int)nNumber);
	return szNumber;
}

#ifdef MATRIX_H
inline string toString(const Matrix44& m)
{
	char szBuf[0x200];
	sprintf (szBuf, "{%g,%g,%g,%g}{%g,%g,%g,%g}{%g,%g,%g,%g}{%g,%g,%g,%g}",
		m(0,0),m(0,1),m(0,2),m(0,3),
		m(1,0),m(1,1),m(1,2),m(1,3),
		m(2,0),m(2,1),m(2,2),m(2,3),
		m(3,0),m(3,1),m(3,2),m(3,3));
	return szBuf;
}
#endif

#ifdef _CRYQUAT_H
inline string toString(const CryQuat& q)
{
	char szBuf[0x100];
	sprintf (szBuf, "{%g,{%g,%g,%g}}", q.w, q.v.x, q.v.y, q.v.z);
	return szBuf;
}
#endif

#ifdef VECTOR_H
inline string toString(const Vec3& v)
{
	char szBuf[0x80];
	sprintf (szBuf, "{%g,%g,%g}", v.x, v.y, v.z);
	return szBuf;
}
#endif


// Does the same as strstr, but the szString is allowed to be no more than the specified size.
// This function is Unicode agnostic and locale agnostic.
inline const char* strnstr (const char* szString, const char* szSubstring, int nSuperstringLength)
{
	int nSubstringLength = (int)strlen(szSubstring);
	if (!nSubstringLength)
		return szString;

	for (int nSubstringPos = 0; szString[nSubstringPos] && nSubstringPos < nSuperstringLength - nSubstringLength; ++nSubstringPos)
	{
		if (strncmp(szString+nSubstringPos, szSubstring, nSubstringLength) == 0)
			return szString+nSubstringPos;
	}
	return NULL;
}


// Finds the string in the array of strings.
// Returns its 0-based index or -1 if not found.
// Comparison is case-sensitive.
// The string array end is demarked by the NULL value.
// This function is Unicode agnostic (but no Unicode collation is performed for equality test) and locale agnostic.
inline int findString(const char* szString, const char* arrStringList[])
{
	for (const char** p = arrStringList; *p; ++p)
	{
		if (0 == strcmp(*p, szString))
			return (int)(p - arrStringList);
	}
	return -1; // string was not found
}


// Cuts the string and adds leading ... if it's longer than specified maximum length.
// This function is ASCII-only and locale agnostic.
inline string cutString (const string& strPath, unsigned nMaxLength)
{
	if (strPath.length() > nMaxLength && nMaxLength > 3)
		return string("...") + string (strPath.c_str() + strPath.length() - (nMaxLength - 3));
	else
		return strPath;
}


// Returns true if the string matches the wildcard.
// Supports wildcard ? (matches one code-point) and * (matches zero or more code-points).
// This function is Unicode aware and locale agnostic.
// Note: ANSI input is not supported, ASCII is fine since it's a subset of UTF-8.
inline bool MatchWildcard (const char* szString, const char* szWildcard)
{
	return CryStringUtils_Internal::MatchesWildcards_Tpl<CryStringUtils_Internal::SCharComparatorCaseSensitive>(szString, szWildcard);
}

// Returns true if the string matches the wildcard.
// Supports wildcard ? (matches one code-point) and * (matches zero or more code-points).
// This function is Unicode aware and uses the "C" locale for case comparison.
// Note: ANSI input is not supported, ASCII is fine since it's a subset of UTF-8.
inline bool MatchWildcardIgnoreCase(const char* szString, const char* szWildcard)
{
	return CryStringUtils_Internal::MatchesWildcards_Tpl<CryStringUtils_Internal::SCharComparatorCaseInsensitive>(szString, szWildcard);
}

#if !defined(RESOURCE_COMPILER)

// calculates a hash value for a given string
inline uint32 CalculateHash(const char *str)
{
	return CCrc32::Compute(str);
}

// calculates a hash value for the lower case version of a given string
inline uint32 CalculateHashLowerCase(const char *str)
{
	return CCrc32::ComputeLowercase(str);
}

// This function is Unicode agnostic and locale agnostic.
inline uint32 HashStringSeed(const char* string, const uint32 seed)
{
	// A string hash taken from the FRD/Crysis2 (game) code with low probability of clashes
	// Recommend you use the CRY_DEFAULT_HASH_SEED (see HashString)
	const char*     p;
	uint32 hash = seed;
	for (p = string; *p != '\0'; p++)
	{
		hash += *p;
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash;
}

// This function is ASCII-only and uses the standard "C" locale for case conversion.
inline uint32 HashStringLowerSeed(const char* string, const uint32 seed)
{
	// computes the hash of 'string' converted to lower case
	// also see the comment in HashStringSeed
	const char*     p;
	uint32 hash = seed;
	for (p = string; *p != '\0'; p++)
	{
		hash += toLowerAscii(*p);
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash;
}

// This function is Unicode agnostic and locale agnostic.
inline uint32 HashString(const char* string)
{
	return HashStringSeed(string, CRY_DEFAULT_HASH_SEED);
}

// This function is ASCII-only and uses the standard "C" locale for case conversion.
inline uint32 HashStringLower(const char* string)
{
	return HashStringLowerSeed(string, CRY_DEFAULT_HASH_SEED);
}
#endif

// Converts all ASCII characters in a string to lower case - avoids memory allocation.
// This function is ASCII-only (Unicode remains unchanged) and uses the "C" locale for case conversion (A-Z only).
inline void toLowerInplace(string& str)
{
#ifndef NOT_USE_CRY_STRING
	str.MakeLower();
#else
	std::transform(str.begin(),str.end(),str.begin(),toLowerAscii); // STL MakeLower
#endif
}

// Converts all characters in a null-terminated string to lower case - avoids memory allocation.
// This function is ASCII-only (Unicode remains unchanged) and uses the "C" locale for case conversion (A-Z only).
inline void toLowerInplace(char* str)
{
	for (char *s = str; *s != 0; s++)
	{
		*s = toLowerAscii(*s);
	}
}


#ifndef NOT_USE_CRY_STRING

// Converts a wide string (can be UTF-16 or UTF-32 depending on platform) to UTF-8.
// This function is Unicode aware and locale agnostic.
template <typename T>
inline void WStrToUTF8(const wchar_t* str, T& dstr)
{
	string utf8;
	Unicode::Convert(utf8, str);

	// Note: This function expects T to have assign(ptr, len) function
	dstr.assign(utf8.c_str(), utf8.length()); 
}

// Converts a wide string (can be UTF-16 or UTF-32 depending on platform) to UTF-8.
// This function is Unicode aware and locale agnostic.
inline string WStrToUTF8(const wchar_t* str)
{
	return Unicode::Convert<string>(str);
}

// Converts an UTF-8 string to wide string (can be UTF-16 or UTF-32 depending on platform).
// This function is Unicode aware and locale agnostic.
template <typename T>
inline void UTF8ToWStr(const char* str, T& dstr)
{
	wstring wide;
	Unicode::Convert(wide, str);

	// Note: This function expects T to have assign(ptr, len) function
	dstr.assign(wide.c_str(), wide.length()); 
}

// Converts an UTF-8 string to wide string (can be UTF-16 or UTF-32 depending on platform).
// This function is Unicode aware and locale agnostic.
inline wstring UTF8ToWStr(const char* str)
{
	return Unicode::Convert<wstring>(str);
}

#endif // NOT_USE_CRY_STRING

// the type used to parse a yes/no string
enum YesNoType
{
	nYNT_Yes,
	nYNT_No,
	nYNT_Invalid
};

// Parse the yes/no string.
// This function only accepts ASCII input, on Unicode content will return nYNT_Invalid.
inline YesNoType toYesNoType(const char* szString)
{
	if (!stricmp(szString, "yes")
		|| !stricmp(szString, "enable")
		|| !stricmp(szString, "true")
		|| !stricmp(szString, "1"))
		return nYNT_Yes;
	if (!stricmp(szString, "no")
		|| !stricmp(szString, "disable")
		|| !stricmp(szString, "false")
		|| !stricmp(szString, "0"))
		return nYNT_No;
	return nYNT_Invalid;
}

// This function checks if the provided filename is valid.
// This function only accepts ASCII input, on Unicode content will return false.
inline bool IsValidFileName(const char* fileName)
{
	size_t i = 0;
	for (;;)
	{
		const char c = fileName[i++];
		if (c == 0)
		{
			return true;
		}
		if (!((c >= '0' && c <= '9')
			|| (c >= 'A' && c <= 'Z')
			|| (c >= 'a' && c <= 'z')
			|| c == '.' || c == '-' || c == '_'))
		{
			return false;
		}
	}
}


/**************************************************************************
*_splitpath() - split a path name into its individual components
*
*Purpose:
*       to split a path name into its individual components
*
*Entry:
*       path  - pointer to path name to be parsed
*       drive - pointer to buffer for drive component, if any
*       dir   - pointer to buffer for subdirectory component, if any
*       fname - pointer to buffer for file base name component, if any
*       ext   - pointer to buffer for file name extension component, if any
*
*Exit:
*       drive - pointer to drive string.  Includes ':' if a drive was given.
*       dir   - pointer to subdirectory string. Includes leading and trailing '/' or '\', if any.
*       fname - pointer to file base name
*       ext   - pointer to file extension, if any.  Includes leading '.'.
*
*******************************************************************************/
ILINE void portable_splitpath ( const char *path, char *drive,	char *dir, char *fname,	char *ext	)
{
	char *p;
	char *last_slash = NULL, *dot = NULL;
	unsigned len;
	PREFAST_ASSUME(path);

	/* we assume that the path argument has the following form, where any
	* or all of the components may be missing.
	*
	*  <drive><dir><fname><ext>
	*
	* and each of the components has the following expected form(s)
	*
	*  drive:
	*  0 to _MAX_DRIVE-1 characters, the last of which, if any, is a
	*  ':'
	*  dir:
	*  0 to _MAX_DIR-1 characters in the form of an absolute path
	*  (leading '/' or '\') or relative path, the last of which, if
	*  any, must be a '/' or '\'.  E.g -
	*  absolute path:
	*      \top\next\last\     ; or
	*      /top/next/last/
	*  relative path:
	*      top\next\last\  ; or
	*      top/next/last/
	*  Mixed use of '/' and '\' within a path is also tolerated
	*  fname:
	*  0 to _MAX_FNAME-1 characters not including the '.' character
	*  ext:
	*  0 to _MAX_EXT-1 characters where, if any, the first must be a
	*  '.'
	*
	*/

	/* extract drive letter and :, if any */

	if ((strlen(path) >= (_MAX_DRIVE - 2)) && (*(path + _MAX_DRIVE - 2) == (':'))) {
		if (drive) {
			cry_strcpy(drive, _MAX_DRIVE, path);
		}
		path += _MAX_DRIVE - 1;
	}
	else if (drive) {
		*drive = ('\0');
	}

	/* extract path string, if any.  Path now points to the first character
	* of the path, if any, or the filename or extension, if no path was
	* specified.  Scan ahead for the last occurence, if any, of a '/' or
	* '\' path separator character.  If none is found, there is no path.
	* We will also note the last '.' character found, if any, to aid in
	* handling the extension.
	*/

	for (last_slash = NULL, p = (char *)path; *p; p++) {
		if (*p == ('/') || *p == ('\\'))
			/* point to one beyond for later copy */
			last_slash = p + 1;
		else if (*p == ('.'))
			dot = p;
	}

	if (last_slash) {

		/* found a path - copy up through last_slash or max. characters
		* allowed, whichever is smaller
		*/

		if (dir) {
			len = (std::min)((unsigned int)(((char *)last_slash - (char *)path) / sizeof(char)),(unsigned int)(_MAX_DIR - 1));
			memcpy(dir, path, len);
			*(dir + len) = ('\0');
		}
		path = last_slash;
	}
	else if (dir) {

		/* no path found */

		*dir = ('\0');
	}

	/* extract file name and extension, if any.  Path now points to the
	* first character of the file name, if any, or the extension if no
	* file name was given.  Dot points to the '.' beginning the extension,
	* if any.
	*/

	if (dot && (dot >= path)) {
		/* found the marker for an extension - copy the file name up to
		* the '.'.
		*/
		if (fname) {
			len = (std::min)((unsigned int)(((char *)dot - (char *)path) / sizeof(char)),(unsigned int)(_MAX_FNAME - 1));
			memcpy(fname, path, len);
			*(fname + len) = ('\0');
		}
		/* now we can get the extension - remember that p still points
		* to the terminating nul character of path.
		*/
		if (ext) {
			len = (std::min)((unsigned int)(((char *)p - (char *)dot) / sizeof(char)),(unsigned int)(_MAX_EXT - 1));
			memcpy(ext, dot, len);
			*(ext + len) = ('\0');
		}
	}
	else {
		/* found no extension, give empty extension and copy rest of
		* string into fname.
		*/
		if (fname) {
			len = (std::min)((unsigned int)(((char *)p - (char *)path) / sizeof(char)),(unsigned int)(_MAX_FNAME - 1));
			memcpy(fname, path, len);
			*(fname + len) = ('\0');
		}
		if (ext) {
			*ext = ('\0');
		}
	}
}


/**************************************************************************
*void _makepath() - build path name from components
*
*Purpose:
*       create a path name from its individual components
*
*Entry:
*       char *path  - pointer to buffer for constructed path
*       char *drive - pointer to drive component, may or may not contain trailing ':'
*       char *dir   - pointer to subdirectory component, may or may not include leading and/or trailing '/' or '\' characters
*       char *fname - pointer to file base name component
*       char *ext   - pointer to extension component, may or may not contain a leading '.'.
*
*Exit:
*       path - pointer to constructed path name
*
*******************************************************************************/
ILINE void portable_makepath ( char *path, const char *drive, const char *dir, const char *fname, const char *ext )
{
	const char *p;

	/* we assume that the arguments are in the following form (although we
	* do not diagnose invalid arguments or illegal filenames (such as
	* names longer than 8.3 or with illegal characters in them)
	*
	*  drive:
	*      A           ; or
	*      A:
	*  dir:
	*      \top\next\last\     ; or
	*      /top/next/last/     ; or
	*      either of the above forms with either/both the leading
	*      and trailing / or \ removed.  Mixed use of '/' and '\' is
	*      also tolerated
	*  fname:
	*      any valid file name
	*  ext:
	*      any valid extension (none if empty or null )
	*/

	/* copy drive */

	if (drive && *drive) {
		*path++ = *drive;
		*path++ = (':');
	}

	/* copy dir */

	if ((p = dir) && *p) {
		do {
			*path++ = *p++;
		}
		while (*p);
		if (*(p-1) != '/' && *(p-1) != ('\\')) {
			*path++ = ('\\');
		}
	}

	/* copy fname */

	if (p = fname) {
		while (*p) {
			*path++ = *p++;
		}
	}

	/* copy ext, including 0-terminator - check to see if a '.' needs
	* to be inserted.
	*/

	if (p = ext) {
		if (*p && *p != ('.')) {
			*path++ = ('.');
		}
		while (*path++ = *p++)
			;
	}
	else {
		/* better add the 0-terminator */
		*path = ('\0');
	}
}

} // namespace CryStringUtils

#endif

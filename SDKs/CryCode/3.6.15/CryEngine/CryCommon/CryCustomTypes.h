//////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2007.
// -------------------------------------------------------------------------
//  File name:   CryCustomTypes.h
//  Created:     2009-10-23 by Scott.
//  Description: Derived CTypeInfos for structs, enums, etc.
//               Compressed numerical types, and associated TypeInfos.
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#ifndef _CRY_CUSTOM_TYPES_H
#define _CRY_CUSTOM_TYPES_H

#include "CryTypeInfo.h"
#include "CryFixedString.h"

#pragma warning(push)
#pragma warning(disable: 4800)

#define STATIC_CONST(T, name, val) \
	static inline T name()  { static T t = val; return t; }

#define ARRAY_COUNT(arr)		(sizeof(arr) / sizeof *(arr))
#define ARRAY_VAR(arr)			ArrayT(&(arr)[0], (int)ARRAY_COUNT(arr))
	
//---------------------------------------------------------------------------
// String helper function.

template<class T>
inline bool HasString(const T& val, FToString flags, const void* def_data = 0)
{
	if (flags.SkipDefault)
	{
		if (val == (def_data ? *(const T*)def_data : T()))
			return false;
	}
	return true;
}

float NumToFromString(float val, int digits, bool floating, char buffer[], int buf_size);

template<class T>
string NumToString(T val, int min_digits, int max_digits, bool floating)
{
	char buffer[64];
	float f(val);
	for (int digits = min_digits; digits < max_digits; digits++)
	{
		if (T(NumToFromString(f, digits, floating, buffer, 64)) == val)
			break;
	}
	return buffer;
}

//---------------------------------------------------------------------------
// TypeInfo for structs

struct CStructInfo: CTypeInfo
{
	CStructInfo( cstr name, size_t size, size_t align, Array<CVarInfo> vars = Array<CVarInfo>(), Array<CTypeInfo const*> templates = Array<CTypeInfo const*>() );
	virtual bool IsType( CTypeInfo const& Info ) const;
	virtual string ToString(const void* data, FToString flags = 0, const void* def_data = 0) const;
	virtual bool FromString(void* data, cstr str, FFromString flags = 0) const;
	virtual bool ToValue(const void* data, void* value, const CTypeInfo& typeVal) const;
	virtual bool FromValue(void* data, const void* value, const CTypeInfo& typeVal) const;
	virtual bool ValueEqual(const void* data, const void* def_data) const;
	virtual void SwapEndian(void* pData, size_t nCount, bool bWriting) const;
	virtual void GetMemoryUsage(ICrySizer* pSizer, void const* data) const;

	virtual const CVarInfo* NextSubVar(const CVarInfo* pPrev, bool bRecurseBase = false) const;
	virtual const CVarInfo* FindSubVar(cstr name) const;

	virtual CTypeInfo const* const* NextTemplateType(CTypeInfo const* const* pPrev) const
	{
		pPrev = pPrev ? pPrev+1 : TemplateTypes.begin();
		return pPrev < TemplateTypes.end() ? pPrev : 0;
	}

protected:
	Array<CVarInfo>						Vars;
	CryStackStringT<char, 16>	EndianDesc;				// Encodes instructions for endian swapping.
	bool											HasBitfields;
	Array<CTypeInfo const*>		TemplateTypes;

	void MakeEndianDesc();
	size_t AddEndianDesc(cstr desc, size_t dim, size_t elem_size);
	bool IsCompatibleType( CTypeInfo const& Info ) const;
};

//---------------------------------------------------------------------------
// Template TypeInfo for base types, using global To/FromString functions.

template<class T>
struct TTypeInfo: CTypeInfo
{
	TTypeInfo( cstr name )
		: CTypeInfo( name, sizeof(T), alignof(T) )
	{}

	virtual bool ToValue(const void* data, void* value, const CTypeInfo& typeVal) const
	{
		if (&typeVal == this)
			return *(T*)value = *(const T*)data, true;
		return false;
	}
	virtual bool FromValue(void* data, const void* value, const CTypeInfo& typeVal) const
	{
		if (&typeVal == this)
			return *(T*)data = *(const T*)value, true;
		return false;
	}

	virtual string ToString(const void* data, FToString flags = 0, const void* def_data = 0) const
	{
		if (!HasString(*(const T*)data, flags, def_data))
			return string();
		return ::ToString(*(const T*)data);
	}
	virtual bool FromString(void* data, cstr str, FFromString flags = 0) const
	{
		if (!*str)
		{
			if (!flags.SkipEmpty)
				*(T*)data = T();
			return true;
		}
		return ::FromString(*(T*)data, str);
	}
	virtual bool ValueEqual(const void* data, const void* def_data = 0) const
	{
		return *(const T*)data == (def_data ? *(const T*)def_data : T());
	}

	virtual void GetMemoryUsage(ICrySizer* pSizer, void const* data) const
	{}
};

//---------------------------------------------------------------------------
// Template TypeInfo for modified types (e.g. compressed, range-limited)

template<class T, class S>
struct TProxyTypeInfo: CTypeInfo
{
	TProxyTypeInfo( cstr name )
		: CTypeInfo( name, sizeof(S), alignof(S) )
	{}

	virtual bool IsType( CTypeInfo const& Info ) const		
		{ return &Info == this || ValTypeInfo().IsType(Info); }

	virtual bool ToValue(const void* data, void* value, const CTypeInfo& typeVal) const
	{
		if (&typeVal == this)
		{
			*(S*)value = *(const S*)data;
			return true;
		}
		T val = T(*(const S*)data);
		return ValTypeInfo().ToValue(&val, value, typeVal);
	}
	virtual bool FromValue(void* data, const void* value, const CTypeInfo& typeVal) const
	{
		if (&typeVal == this)
		{
			*(S*)data = *(const S*)value;
			return true;
		}
		T val;
		if (ValTypeInfo().FromValue(&val, value, typeVal))
		{
			*(S*)data = S(val);
			return true;
		}
		return false;
	}

	virtual string ToString(const void* data, FToString flags = 0, const void* def_data = 0) const
	{
		T val = T(*(const S*)data);
		T def_val = def_data ? T(*(const S*)def_data) : T();
		return ValTypeInfo().ToString(&val, flags, &def_val);
	}
	virtual bool FromString(void* data, cstr str, FFromString flags = 0) const
	{
		T val;
		if (!*str)
		{
			if (!flags.SkipEmpty)
				*(S*)data = S();
			return true;
		}
		if (!TypeInfo(&val).FromString(&val, str))
			return false;
		*(S*)data = S(val);
		return true;
	}

	// Forward additional TypeInfo functions.
	virtual bool GetLimit(ENumericLimit eLimit, float& fVal) const
		{ return ValTypeInfo().GetLimit(eLimit, fVal); }
	virtual cstr EnumElem(uint nIndex) const
		{ return ValTypeInfo().EnumElem(nIndex); }

protected:

	static const CTypeInfo& ValTypeInfo()
		{ return TypeInfo((T*)0); }
};

//---------------------------------------------------------------------------
// Customisation for string.

template<> 
inline string TTypeInfo<string>::ToString(const void* data, FToString flags, const void* def_data) const
{
	const string& val = *(const string*)data;
	if (def_data && flags.SkipDefault)
	{
		if (val == *(const string*)def_data)
			return string();
	}
	return val;
}

template<> 
inline bool TTypeInfo<string>::FromString(void* data, cstr str, FFromString flags) const
{
	if (!*str && flags.SkipEmpty)
		return true;
	*(string*)data = str;
	return true;
}

template<> 
void TTypeInfo<string>::GetMemoryUsage(ICrySizer* pSizer, void const* data) const;

//---------------------------------------------------------------------------
//
// TypeInfo for small integer types.


#pragma warning(disable: 4554) // BOGUS WARNING: check operator precedence for possible error; use parentheses to clarify precedence

template<class T> struct TIntTraits
{
	static const bool bSIGNED
		= T(-1) < T(0);

	static const T nMIN_FACTOR
		= bSIGNED ? T(-1) : T(0);

	static const size_t nPOS_BITS
		= sizeof(T) * 8 - bSIGNED;

	static const T nMIN
		= bSIGNED ? T (T(1) << T(sizeof(T)*8-1)) : T(0);

	static const T nMAX
		= ~nMIN;
};

template<class D, class S>
inline bool ConvertInt( D& dest, S src)
{
	if (TIntTraits<D>::nPOS_BITS < TIntTraits<S>::nPOS_BITS)
		src = clamp_tpl(src, S(TIntTraits<D>::nMIN), S(TIntTraits<D>::nMAX));
	else if (TIntTraits<D>::bSIGNED < TIntTraits<S>::bSIGNED)
		src = max(src, S(0));

	dest = D(src);
	assert(S(dest) == src);
	return true;
}

template<class D>
inline bool ConvertInt( D& dest, const void* src, const CTypeInfo& typeSrc )
{
	if (typeSrc.IsType<int>())
	{
		switch (typeSrc.Size)
		{
			case 1: return ConvertInt(dest, *(const int8*)src);
			case 2: return ConvertInt(dest, *(const int16*)src);
			case 4: return ConvertInt(dest, *(const int32*)src);
			case 8: return ConvertInt(dest, *(const int64*)src);
		}
	}
	else if (typeSrc.IsType<uint>())
	{
		switch (typeSrc.Size)
		{
			case 1: return ConvertInt(dest, *(const uint8*)src);
			case 2: return ConvertInt(dest, *(const uint16*)src);
			case 4: return ConvertInt(dest, *(const uint32*)src);
			case 8: return ConvertInt(dest, *(const uint64*)src);
		}
	}
	return false;
}

template<class S>
inline bool ConvertInt( void* dest, const CTypeInfo& typeDest, S src )
{
	if (typeDest.IsType<int>())
	{
		switch (typeDest.Size)
		{
			case 1: return ConvertInt(*(int8*)dest, src);
			case 2: return ConvertInt(*(int16*)dest, src);
			case 4: return ConvertInt(*(int32*)dest, src);
			case 8: return ConvertInt(*(int64*)dest, src);
		}
	}
	else if (typeDest.IsType<uint>())
	{
		switch (typeDest.Size)
		{
			case 1: return ConvertInt(*(uint8*)dest, src);
			case 2: return ConvertInt(*(uint16*)dest, src);
			case 4: return ConvertInt(*(uint32*)dest, src);
			case 8: return ConvertInt(*(uint64*)dest, src);
		}
	}
	return false;
}

template<class T>
struct TIntTypeInfo: TTypeInfo<T>
{
	TIntTypeInfo( cstr name )
		: TTypeInfo<T>(name)
	{}

	virtual bool IsType( CTypeInfo const& Info ) const		
		{ return &Info == this || &Info == ( TIntTraits<T>::bSIGNED ? &TypeInfo((int*)0) : &TypeInfo((uint*)0) ); }

	virtual bool GetLimit(ENumericLimit eLimit, float& fVal) const
	{
		if (eLimit == eLimit_Min)
			return fVal = float(TIntTraits<T>::nMIN), true;
		if (eLimit == eLimit_Max)
			return fVal = float(TIntTraits<T>::nMAX), true;
		if (eLimit == eLimit_Step)
			return fVal = 1.f, true;
		return false;
	}

	// Override to allow int conversion
	virtual bool FromValue(void* data, const void* value, const CTypeInfo& typeVal) const
		{ return ConvertInt(*(T*)data, value, typeVal); }
	virtual bool ToValue(const void* data, void* value, const CTypeInfo& typeVal) const
		{ return ConvertInt(value, typeVal, *(const T*)data); }
};

//---------------------------------------------------------------------------
// Store any type, such as an enum, in a small int.

template<class T, int nMIN = INT_MIN, int nMAX = INT_MAX, int nDEFAULT = 0>
struct TRangedType
{
	typedef TRangedType<T, nMIN, nMAX, nDEFAULT> TThis;

	TRangedType(T init = T(nDEFAULT))
		: m_Val(init)
	{ 
		CheckRange(m_Val);
	}

	operator T() const
		{ return m_Val; }

	CUSTOM_STRUCT_INFO(CCustomInfo)

protected:
	T		m_Val;

	static bool HasMin()
		{ return nMIN > INT_MIN; }
	static bool HasMax()
		{ return nMAX < INT_MAX; }

	static bool CheckRange(T& val)
	{
		if (HasMin() && val < T(nMIN))
		{
			val = T(nMIN);
			return false;
		}
		else if (HasMax() && val > T(nMAX))
		{
			val = T(nMAX);
			return false;
		}
		return true;
	}

	// Adaptor TypeInfo for specifying limits, and implementing range checking
	struct CCustomInfo: TProxyTypeInfo<T, TThis>
	{
		CCustomInfo()
			: TProxyTypeInfo<T, TThis>( ::TypeInfo((T*)0).Name )
		{}

		virtual bool GetLimit(ENumericLimit eLimit, float& fVal) const
		{
			if (eLimit == eLimit_Min && HasMin())
				return fVal = T(nMIN), true;
			if (eLimit == eLimit_Max && HasMax())
				return fVal = T(nMAX), true;
			return ::TypeInfo((T*)0).GetLimit(eLimit, fVal);
		}
	};
};

//---------------------------------------------------------------------------
// Store any type, such as an enum, in a small int.

template<class T, class S = uint8, int nOffset = 0, int nDefault = nOffset>
struct TSmall
{
	typedef TSmall<T,S,nOffset,nDefault> TThis;
	typedef T		TValue;

	inline TSmall(T val = T(nDefault))
	{
		set(val);
	}
	void set(T val = T(nDefault))
	{
		m_Val = S(val) - S(nOffset);
		assert(+*this == val);
	}

	inline operator T() const
		{ return T(T(m_Val) + nOffset); }
	inline T operator +() const
		{ return T(T(m_Val) + nOffset); }

	CUSTOM_STRUCT_INFO(CCustomInfo)

protected:
	S		m_Val;

	struct CCustomInfo: TProxyTypeInfo<T, TThis>
	{
		CCustomInfo()
			: TProxyTypeInfo<T, TThis>("TSmall<>")
		{}

		virtual bool GetLimit(ENumericLimit eLimit, float& fVal) const
		{
			if (eLimit == eLimit_Min)
				return fVal = float(TIntTraits<S>::nMIN + nOffset), true;
			if (eLimit == eLimit_Max)
				return fVal = float(TIntTraits<S>::nMAX + nOffset), true;
			if (eLimit == eLimit_Step)
				return fVal = 1.f, true;
			return false;
		}
	};
};

//---------------------------------------------------------------------------
// Quantise a float linearly in an int.
template<class S, int nLIMIT, S nQUANT = TIntTraits<S>::nMAX, bool bTRUNC = false>
struct TFixed
{
	typedef float TValue;

	inline TFixed()
		: m_Store(0)
	{}

	inline TFixed(float fIn)
	{
		float fStore = ToStore(fIn);
		fStore = clamp_tpl(fStore, float(TIntTraits<S>::nMIN_FACTOR * nQUANT), float(nQUANT));
		m_Store = bTRUNC ? S(fStore) : fStore < 0.f ? S(fStore - 0.5f) : S(fStore + 0.5f);
	}

	// Conversion.
	inline operator float() const
		{ return FromStore(m_Store); }
	inline float operator +() const
		{ return FromStore(m_Store); }
	inline bool operator !() const
		{ return !m_Store; }

	inline bool operator ==(const TFixed& x) const
		{ return m_Store == x.m_Store; }
	inline bool operator ==(float x) const
		{ return m_Store == TFixed(x); }
	inline S GetStore() const
		{ return m_Store; }

	static S GetMaxStore()
		{ return nQUANT; }
	static float GetMaxValue()
		{ return float(nLIMIT); }

	CUSTOM_STRUCT_INFO(CCustomInfo)

protected:
	S		m_Store;

	typedef TFixed<S, nLIMIT, nQUANT, bTRUNC> TThis;

	static const int nMAX = nLIMIT;
	static const int nMIN = TIntTraits<S>::nMIN_FACTOR * nLIMIT;

	static inline float ToStore(float f)
		{ return f * float(nQUANT) / float(nLIMIT); }
	static inline float FromStore(float f)
		{ return f * float(nLIMIT) / float(nQUANT); }

	// TypeInfo implementation.
	struct CCustomInfo: TProxyTypeInfo<float, TThis>
	{
		CCustomInfo()
			: TProxyTypeInfo<float, TThis>("TFixed<>")
		{}

		virtual bool GetLimit(ENumericLimit eLimit, float& fVal) const
		{
			if (eLimit == eLimit_Min)
				return fVal = float(nMIN), true;
			if (eLimit == eLimit_Max)
				return fVal = float(nMAX), true;
			if (eLimit == eLimit_Step)
				return fVal = FromStore(1.f), true;
			return false;
		}

		// Override ToString: Limit to significant digits.
		virtual string ToString(const void* data, FToString flags = 0, const void* def_data = 0) const
		{
			if (!HasString(*(const S*)data, flags, def_data))
				return string();
			static int digits = int_ceil( log10f(float(nQUANT)) );
			return NumToString(*(const TFixed*)data, 1, digits+3, true);
		}
	};
};


// Define the canonical float-to-byte quantisation.
typedef TFixed<uint8,1>	UnitFloat8;

//---------------------------------------------------------------------------
// A floating point number, with templated storage size (and sign), and number of exponent bits
template<class S, int nEXP_BITS>
struct TFloat
{
	typedef float TValue;

	ILINE TFloat()
		: m_Store(0)
	{}

	ILINE TFloat(float fIn)
		: m_Store(FromFloat(fIn))
	{}

	ILINE operator float() const
		{ return ToFloat(m_Store); }
	ILINE float operator +() const
		{ return ToFloat(m_Store); }

	ILINE bool operator !() const
		{ return !m_Store; }

	ILINE bool operator ==(TFloat x) const
		{ return m_Store == x.m_Store; }
	ILINE bool operator ==(float x) const
		{ return float(*this) == x; }

	inline TFloat& operator *=(float x)
		{ return *this = *this * x; }

	ILINE uint32 partial_float_conversion() const
		{ return (0 == m_Store)?0:ToFloatCore( m_Store ); }

	STATIC_CONST(float, fMAX, ToFloat(TIntTraits<S>::nMAX));
	STATIC_CONST(float, fPOS_MIN, ToFloat(1 << nMANT_BITS));
	STATIC_CONST(float, fMIN, -fMAX() * (float)TIntTraits<S>::bSIGNED);

	CUSTOM_STRUCT_INFO(CCustomInfo)

protected:
	S		m_Store;

	typedef TFloat<S,nEXP_BITS> TThis;

	static const S nBITS = sizeof(S)*8;
	static const S nSIGN = TIntTraits<S>::bSIGNED;
	static const S nMANT_BITS = nBITS - nEXP_BITS - nSIGN;
	static const S nSIGN_MASK = S(nSIGN << (nBITS-1));
	static const S nMANT_MASK = (S(1) << nMANT_BITS) - 1;
	static const S nEXP_MASK = ~S(nMANT_MASK | nSIGN_MASK);
	static const int nEXP_MAX = 1 << (nEXP_BITS-1),
	                 nEXP_MIN = 1 - nEXP_MAX;

	STATIC_CONST(float, fROUNDER, 1.f + fPOS_MIN()*0.5f);

	static inline S FromFloat(float fIn)
	{
		COMPILE_TIME_ASSERT(sizeof(S) <= 4);
		COMPILE_TIME_ASSERT(nEXP_BITS > 0 && nEXP_BITS <= 8 && nEXP_BITS < sizeof(S)*8-4);

		// Clamp to allowed range.
		float fClamped = clamp_tpl(fIn * fROUNDER(), fMIN(), fMAX());

		// Bit shift to convert from IEEE float32.
		uint32 uBits = *(const uint32*)&fClamped;
		
		// Convert exp.
		int32 iExp = (uBits >> 23) & 0xFF;
		iExp -= 127 + nEXP_MIN;
		IF (iExp < 0,0)
			// Underflow.
			return 0;

		// Reduce mantissa.
		uint32 uMant = uBits >> (23 - nMANT_BITS);

		S bits = (uMant & nMANT_MASK)
					 | (iExp << nMANT_BITS)
					 | ((uBits >> (32-nBITS)) & nSIGN_MASK);

		#ifdef _DEBUG
			fIn = clamp_tpl(fIn, fMIN(), fMAX());
			float fErr = fabs(ToFloat(bits) - fIn);
			float fMaxErr = fabs(fIn) / float(1<<nMANT_BITS);
			assert(fErr <= fMaxErr);
		#endif

		return bits;
	}

	static inline uint32 ToFloatCore(S bits)
	{
		// Extract FP components.
		uint32 uBits = bits & nMANT_MASK,
			uExp = (bits & ~nSIGN_MASK) >> nMANT_BITS,
			uSign = bits & nSIGN_MASK;

		// Shift to 32-bit.
		uBits <<= 23 - nMANT_BITS;
		uBits |= (uExp + 127 + nEXP_MIN) << 23;
		uBits |= uSign << (32-nBITS);

		return uBits;
	}

	static ILINE float ToFloat(S bits)
	{
		IF (bits == 0, 0)
			return 0.f;

		uint32 uBits = ToFloatCore( bits );
		return *(float*)&uBits;
	}

	// TypeInfo implementation.
	struct CCustomInfo: TProxyTypeInfo<float, TThis>
	{
		CCustomInfo()
			: TProxyTypeInfo<float, TThis>("TFloat<>") 
		{}

		virtual bool GetLimit(ENumericLimit eLimit, float& fVal) const
		{
			if (eLimit == eLimit_Min)
				return fVal = fMIN(), true;
			if (eLimit == eLimit_Max)
				return fVal = fMAX(), true;
			if (eLimit == eLimit_Step)
				return fVal = fPOS_MIN(), true;
			return false;
		}

		// Override ToString: Limit to significant digits.
		virtual string ToString(const void* data, FToString flags = 0, const void* def_data = 0) const
		{
			if (!HasString(*(const S*)data, flags, def_data))
				return string();
			static int digits = int_ceil( log10f(1<<nMANT_BITS) );
			return NumToString(*(const TFloat*)data, 1, digits+3, true);
		}
	};
};

// Canonical float16 types, with range ~= 64K.
typedef TFloat<int16,5>		SFloat16;
typedef TFloat<uint16,5>	UFloat16;

template<typename T>
ILINE T partial_float_cast( const SFloat16& s ) { return static_cast<T>( s.partial_float_conversion() ); }

template<typename T>
ILINE T partial_float_cast( const UFloat16& u ) { return static_cast<T>( u.partial_float_conversion() ); }

#ifdef _DEBUG

// Classes for unit tests.
template<class T2, class T>
void TestValues(T val)
{
	T2 val2 = val;
	T2 val2c;

	bool b = TypeInfo(&val2c).FromValue(&val2c, val);
	assert(b);
	assert(val2 == val2c);
	b = TypeInfo(&val2c).ToValue(&val2c, val);
	assert(b);
	assert(val2 == T2(val));
}

template<class T2, class T>
void TestTypes(T val)
{
	T2 val2 = val;
	string s = TypeInfo(&val2).ToString(&val2);
	bool b = TypeInfo(&val).FromString(&val, s);
	assert(b);
	assert(val2 == T2(val));

	TestValues<T2>(val);
}

template<class T>
void TestType(T val)
{
	TestTypes<T>(val);
}

#endif // _DEBUG

//---------------------------------------------------------------------------
// TypeInfo for enums

//---------------------------------------------------------------------------
template<class TEnumDef>
struct TEnumInfo: CTypeInfo
{
	typedef int64 TValue;

	const TEnumDef& def() const
		{ return static_cast<const TEnumDef&>(*this); }

	TEnumInfo( cstr name, size_t size, size_t align )
		: CTypeInfo(name, size, align) {}

	// TypeInfo implementation
	virtual bool IsType( CTypeInfo const& Info ) const		
		{ return &Info == this || &Info == &TypeInfo((int*)0); }

	virtual cstr EnumElem(uint nIndex) const
	{
		if (nIndex < def().Count())
		{
			cstr name = def().Name(nIndex);
			if (*name && *name != '_')
				return name;
		}
		return 0;
	}

	virtual bool ToValue(const void* data, void* value, const CTypeInfo& typeVal) const;
	virtual bool FromValue(void* data, const void* value, const CTypeInfo& typeVal) const;
	virtual string ToString(const void* data, FToString flags, const void* def_data) const;
	virtual bool FromString(void* data, cstr str, FFromString flags) const;

protected:

	TValue ReadInt(const void* data) const
	{
		TValue val;
		ConvertInt(val, data, *this);
		return val;
	}
	bool WriteInt(void* data, TValue val) const
	{
		return ConvertInt(data, *this, val);
	}
};

//---------------------------------------------------------------------------
// TypeInfo for regular enums

struct CSimpleEnumInfo: TEnumInfo<CSimpleEnumInfo>
{
	CSimpleEnumInfo( cstr name, size_t size, size_t align, Array<cstr> names, char* enum_str );

	// TEnumDef implementations
	ILINE uint Count() const
		{ return asNames.size(); }
	ILINE static uint Value(uint i)
		{ return i; }
	ILINE cstr Name(uint i) const
		{ return asNames[i]; }
	ILINE bool MatchName(uint i, cstr str) const
	{
		cstr name = asNames[i];
		if (*name == '_')
			name++;
		return strcmpi(name, str) == 0;
	}
	ILINE cstr ToName(TValue value) const
	{
		if (value >= 0 && value < Count())
			return asNames[check_cast<int>(value)];
		return 0;
	}

protected:

	Array<cstr>		asNames;
};

// Helper class for implementing testing expression:  if (enumvar().eValue)
struct EnumTester
{
	ILINE operator bool() const
		{ return this == 0; }
	ILINE bool operator !() const
		{ return this != 0; }
};

// Define a regular small enum with TypeInfo

#define DEFINE_ENUM(EType, ...) \
	struct EType \
	{ \
		enum E { __VA_ARGS__, _count }; \
		typedef uint8 TInt; \
		TInt Value; \
		ILINE EType(E val = E(0)) : Value(val) {} \
		ILINE operator E() const { return E(Value); } \
		ILINE E operator +() const { return E(Value); } \
		ILINE static uint Count() { return _count; } \
		struct STester { EnumTester __VA_ARGS__; }; \
		STester const& operator ()() const { return *(STester const*)(-Value * sizeof(EnumTester)); } \
		static const CSimpleEnumInfo& TypeInfo() { \
			static cstr names[_count]; \
			static char enum_str[] = #__VA_ARGS__ ; \
			static CSimpleEnumInfo info( #EType, sizeof(EType), alignof(EType), Array<cstr>(&names[0], _count), enum_str ); \
			return info; \
		} \
	};

// Usage:
//	EOP(var,=) eValue;		// assign
//	EOP(var,>=) eValue		// compare
#define EOP(var, op)		(var) op (var).

//---------------------------------------------------------------------------
// TypeInfo for irregular enums

struct CEnumInfo: TEnumInfo<CEnumInfo>
{
	struct CEnumElem
	{
		TValue	Value;
		cstr		Name;
	};

	CEnumInfo( cstr name, size_t size, size_t align, Array<CEnumElem> elems );

	// TEnumDef implementations
	ILINE uint Count() const
		{ return Elems.size(); }
	ILINE TValue Value(uint i) const
		{ return Elems[i].Value; }
	ILINE cstr Name(uint i) const
		{ return *Elems[i].Name ? Elems[i].Name+nPrefixLength : ""; }
	bool MatchName(uint i, cstr str) const;
	cstr ToName(TValue val) const;

protected:

	Array<CEnumElem>	Elems;
	bool							bRegular;
	TValue						MinValue;
	size_t						nPrefixLength;
};

// Define an irregular enum with TypeInfo

template<class TOwner, class E, E EVal> struct EnumMember
{
	operator bool() const
		{ return *(const TOwner*)this == EVal; }
	bool operator !() const
		{ return *(const TOwner*)this != EVal; }
	TOwner operator++(int)
		{ return *(TOwner*)this = EVal; }
};

#define ENUM_DEFINE_TYPE(name, init)		name init,
#define ENUM_DEFINE_ELEMS(name, init)		{ name, #name },
#define ENUM_DEFINE_MEMBERS(name, init)	EnumMember<TThis, E, E::name> name;

#define DEFINE_ENUM_VALS(EType, Int) \
	struct EType \
	{ \
		enum E { _##EType(ENUM_DEFINE_TYPE) }; \
		typedef EType TThis; \
		typedef Int TInt; \
		TInt Value; \
		ILINE EType(E val = E(0)) : Value(val) {} \
		ILINE operator E() const { return E(Value); } \
		ILINE E operator +() const { return E(Value); } \
		ILINE static uint Count() { return TypeInfo().Count(); } \
		union Aux { _##EType(ENUM_DEFINE_MEMBERS) }; \
		ILINE Aux& operator ()() { return *(Aux*)this; } \
		ILINE const Aux& operator ()() const { return *(const Aux*)this; } \
		static const CEnumInfo& TypeInfo() { \
			static CEnumInfo::CEnumElem elems[] = { _##EType(ENUM_DEFINE_ELEMS) }; \
			static CEnumInfo info( #EType, sizeof(EType), alignof(EType), ARRAY_VAR(elems)); \
			return info; \
		} \
	};

//---------------------------------------------------------------------------
// deep_ptr: acts as a value type, copying the pointee value, and allocating on write.
// To manually force allocation (when bINIT = false): *ptr
// To manually free it: ptr = 0

template<class T, bool bINIT = false>
struct deep_ptr
{
	typedef deep_ptr<T,bINIT> this_t;

	~deep_ptr()
		{ delete m_ptr; }

	deep_ptr()
		: m_ptr( bINIT ? new T() : 0 ) {}

	deep_ptr( T* p )
		: m_ptr(p) {}

	deep_ptr( const this_t& init )
		: m_ptr( init.m_ptr ? new T(*init.m_ptr) : 0) {}

	this_t& operator=( const this_t& init )
	{
		if (init.m_ptr != m_ptr)
		{
			delete m_ptr;
			m_ptr = init.m_ptr ? new T(*init.m_ptr) : 0;
		}
		return *this;
	}

	this_t& operator=( T* p )
	{
		if (p != m_ptr)
		{
			delete m_ptr;
			m_ptr = p;
		}
		return *this;
	}

	// Read access
	operator bool() const
		{ return !!m_ptr; }
	bool operator !() const
		{ return !m_ptr; }

	const T* operator ->() const
	{
		assert(m_ptr);
		return m_ptr;
	}
	T* operator ->()
	{
		// When writing empty value, allocate new.
		if (bINIT)
		{
			assert(m_ptr);
		}
		else if (!m_ptr) 
			m_ptr = new T;
		return m_ptr;
	}

	const T& operator *() const
		{ return *operator->(); }
	T& operator *()
		{ return *operator->(); }

	void GetMemoryUsage(ICrySizer *pSizer) const
	{
		if (m_ptr)
			m_ptr->GetMemoryUsage(pSizer, true);
	}

	CUSTOM_STRUCT_INFO(CCustomInfo)

private:
	T*	m_ptr;

	// Provide interface to pointee data.
	struct CCustomInfo: CTypeInfo
	{
		static const CTypeInfo& ValTypeInfo()
			{ return ::TypeInfo((T*)0); }

		CCustomInfo()
			: CTypeInfo( ValTypeInfo().Name, sizeof(T*), alignof(T*)) {}

		static void FreeDefault(this_t& ptr)
		{
			T objDefault;
			if (ValTypeInfo().ValueEqual(&*ptr, &objDefault))
				ptr = 0;
		}

		virtual bool IsType( CTypeInfo const& Info ) const		
			{ return &Info == this || ValTypeInfo().IsType(Info); }

		// Read functions: read from a default object if not allocated.
		virtual string ToString(const void* data, FToString flags = 0, const void* def_data = 0) const
		{
			const T* ptr = *(const T**)data;
			const T* def_ptr = def_data ? *(const T**)def_data : 0;
			if (ptr)
			{
				return ValTypeInfo().ToString(ptr, flags, def_ptr); 
			}
			else
			{
				T objDefault;
				return ValTypeInfo().ToString(&objDefault, flags, def_ptr);
			}
		}
		virtual bool ToValue(const void* data, void* value, const CTypeInfo& typeVal) const
		{
			const T* ptr = *(const T**)data;
			if (ptr)
			{
				return ValTypeInfo().ToValue(ptr, value, typeVal); 
			}
			else
			{
				T objDefault;
				return ValTypeInfo().ToValue(&objDefault, value, typeVal); 
			}
		}
		virtual bool ValueEqual(const void* data, const void* def_data = 0) const
		{
			const T* ptr = *(const T**)data;
			const T* def_ptr = def_data ? *(const T**)def_data : 0;
			if (!ptr && !def_ptr)
				return true;
			if (ptr && def_ptr)
				return ValTypeInfo().ValueEqual(ptr, def_ptr); 
			T objDefault;
			return ValTypeInfo().ValueEqual(ptr ? ptr : &objDefault, def_ptr ? def_ptr : &objDefault); 
		}

		// Write functions: allocate and copy data only if non-default.
		virtual bool FromString(void* data, cstr str, FFromString flags = 0) const
		{
			this_t& ptr = *(this_t*)data;
			if (!*str)
			{
				if (!flags.SkipEmpty)
					ptr = 0;
				return true;
			}
			bool bSuccess = ValTypeInfo().FromString( &*ptr, str, flags ); 
			FreeDefault(ptr);
			return bSuccess;
		}
		virtual bool FromValue(void* data, const void* value, const CTypeInfo& typeVal) const
		{ 
			this_t& ptr = *(this_t*)data;
			bool bSuccess = ValTypeInfo().FromValue( &*ptr, value, typeVal );
			FreeDefault(ptr);
			return bSuccess;
		}

		virtual void SwapEndian(void* data, size_t nCount, bool bWriting) const
		{
			T* ptr = *(T**)data;
			if (ptr)
				ValTypeInfo().SwapEndian(ptr, nCount, bWriting);
		}
		virtual void GetMemoryUsage(ICrySizer* pSizer, const void* data) const
		{
			const T* ptr = *(const T**)data;
			if (ptr)
				ValTypeInfo().GetMemoryUsage(pSizer, ptr);
		}
	};
};


#pragma warning(pop)
#endif

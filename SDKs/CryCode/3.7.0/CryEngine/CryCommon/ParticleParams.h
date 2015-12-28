////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   ParticleParams.h
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef _PARTICLEPARAMS_H_
#define _PARTICLEPARAMS_H_ 1

#include <FinalizingSpline.h>
#include <Cry_Color.h>
#include <CryString.h>
#include <CryName.h>

#include <CryCustomTypes.h>
#include <Cry_Math.h>

BASIC_TYPE_INFO(CCryName)

///////////////////////////////////////////////////////////////////////

#define fHUGE			1e9f		// Use for practically infinite values, to simplify comparisons.
	// 1e9 s > 30 years, 1e9 m > earth-moon distance.
	// Convertible to int32.

// Convert certain parameters where zero denotes an essentially infinite value.
ILINE float ZeroIsHuge(float f)
{
	return if_pos_else(f, f, fHUGE);
}

// For accessing specific information from variable parameters,
// use VMIN, VMAX (from CryMath.h), and VRANDOM (defined here)
enum type_random { VRANDOM };

///////////////////////////////////////////////////////////////////////
// Trinary enum, encodes yes/no/both state

// Original names, compatible with serialization
DEFINE_ENUM_VALS( ETrinaryNames, uint8,
	If_False = 1,
	If_True,
	Both
)

struct ETrinary: ETrinaryNames
{
	// Initialized with bool value, or Both (default)
	ETrinary(ETrinaryNames::E e = Both)
		: ETrinaryNames(e) {}
	ETrinary(bool b)
		: ETrinaryNames(ETrinaryNames::E(1 << uint8(b))) {}

	// Test against ETrinary, or directly against bool
	uint8 operator *(ETrinary t) const
	{
		return +*this & +t;
	}
	uint8 operator *(bool b) const
	{
		return +*this & (1 << uint8(b));
	}
};

///////////////////////////////////////////////////////////////////////
// Pseudo-random number generation, from a key.

class CChaosKey
{
public:
	// Initialize with an int.
	explicit inline CChaosKey(uint32 uSeed)
		: m_Key(uSeed) {}

	explicit inline CChaosKey(float fSeed)
		: m_Key((uint32)(fSeed * float(0xFFFFFFFF))) {}

	explicit inline CChaosKey(void const* ptr)
		: m_Key(uint32(size_t(ptr))) {}

	CChaosKey Jumble(CChaosKey key2) const
	{
		return CChaosKey( Jumble(m_Key ^ key2.m_Key) );
	}
	CChaosKey Jumble(void const* ptr) const
	{
		return Jumble(CChaosKey(ptr));
	}

	// Scale input range.
	inline float operator *(float fRange) const
	{
		return (float)m_Key / float(0xFFFFFFFF) * fRange;
	}
	inline uint32 operator *(uint32 nRange) const
	{
		return m_Key % nRange;
	}
	inline uint16 operator *(uint16 nRange) const
	{
		return m_Key % nRange;
	}

	uint32 GetKey() const
	{
		return m_Key;
	}

	// Jumble with a range variable to produce a random value.
	template<class T>
	inline T Random(T const& Range) const
	{
		return Jumble(CChaosKey(&Range)) * Range;
	}

private:
	uint32	m_Key;

	static inline uint32 Jumble(uint32 key)
	{
		key += ~rot_left(key, 15);
		key ^= rot_right(key, 10);
		key += rot_left(key, 3);
		key ^= rot_right(key, 6);
		key += ~rot_left(key, 11);
		key ^= rot_right(key, 16);
		return key;
	}

	static inline uint32 rot_left(uint32 u, int n)
	{
		return (u<<n) | (u>>(32-n));
	}
	static inline uint32 rot_right(uint32 u, int n)
	{
		return (u>>n) | (u<<(32-n));
	}
};

//////////////////////////////////////////////////////////////////////////
//
// Float storage
//

typedef TRangedType<float>					SFloat;
typedef TRangedType<float,0>				UFloat;
typedef TRangedType<float,0,1>			UnitFloat;
typedef TRangedType<float,0,2>			Unit2Float;
typedef TRangedType<float,-180,180>	SAngle;
typedef TRangedType<float,0,180>		UHalfAngle;
typedef TRangedType<float,0,360>		UFullAngle;

typedef TSmall<bool>								TSmallBool;
typedef TSmall<bool,uint8,0,1>			TSmallBoolTrue;
typedef TSmall<uint,uint8,1>				PosInt8;

template<class T>
struct TStorageTraits
{
	typedef float				TValue;
	typedef UnitFloat8	TMod;
	typedef UnitFloat		TRandom;
};

template<>
struct TStorageTraits<SFloat>
{
	typedef float				TValue;
	typedef UnitFloat8	TMod;
	typedef Unit2Float	TRandom;
};

template<>
struct TStorageTraits<SAngle>
{
	typedef float				TValue;
	typedef UnitFloat8	TMod;
	typedef Unit2Float	TRandom;
};


template<class TFixed>
bool RandomActivate(const TFixed& chance)
{
	return Random(chance.GetMaxStore()) < chance.GetStore();
}

//////////////////////////////////////////////////////////////////////////
//
// Vec3 TypeInfo
//

// Must override Vec3 constructor to avoid polluting params with NANs
template<class T>
struct Vec3_Zero: Vec3_tpl<T>
{
	Vec3_Zero() : Vec3_tpl<T>(ZERO) {}
	Vec3_Zero(const Vec3& v) : Vec3_tpl<T>(v) {}
};

typedef Vec3_Zero<SFloat> Vec3S;
typedef Vec3_Zero<UFloat> Vec3U;

//////////////////////////////////////////////////////////////////////////
//
// Color specialisations.

template<class T>
struct Color3: Vec3_tpl<T>
{
	Color3( T v = T(0) )
		: Vec3_tpl<T>(v) {}

	Color3( T r, T g, T b )
		: Vec3_tpl<T>(r, g, b) {}

	Color3( Vec3_tpl<T> const& v)
		: Vec3_tpl<T>(v) {}

	template<class T2>
	Color3( Vec3_tpl<T2> const& c )
		: Vec3_tpl<T>(c) {}

	operator ColorF() const
		{ return ColorF(this->x, this->y, this->z, 1.f); }

	// Implement color multiplication.
	Color3& operator *=(Color3 const& c)
		{ this->x *= c.x; this->y *= c.y; this->z *= c.z; return *this; }
};

template<class T>
Color3<T> operator *(Color3<T> const& c, Color3<T> const& d)
	{ return Color3<T>(c.x * d.x, c.y * d.y, c.z * d.z); }

typedef Color3<float> Color3F;
typedef Color3<UnitFloat8> Color3B;

class RandomColor: public UnitFloat8
{
public:
	inline RandomColor(float f = 0.f, bool bRandomHue = false)
		: UnitFloat8(f), m_bRandomHue(bRandomHue)
	{}

	Color3F GetRandom() const
	{
		if (m_bRandomHue)
		{
			ColorB clr(cry_rand32());
			float fScale = float(*this) / 255.f;
			return Color3F(clr.r * fScale, clr.g * fScale, clr.b * fScale);
		}
		else
		{
			return Color3F(Random(float(*this)));
		}
	}

	AUTO_STRUCT_INFO

protected:
	TSmallBool	m_bRandomHue;
};

inline Color3F Random(RandomColor const& rc)
{
	return rc.GetRandom();
}

template<>
struct TStorageTraits<Color3F>
{
	typedef Color3F							TValue;
	typedef Color3<UnitFloat8>	TMod;
	typedef RandomColor					TRandom;
};

///////////////////////////////////////////////////////////////////////
//
// Spline implementation class.
//

template<>
inline const CTypeInfo& TypeInfo(ISplineInterpolator**)
{
	static CTypeInfo Info("ISplineInterpolator*", sizeof(void*), alignof(void*));
	return Info;
}

template<class S>
class TCurve: public spline::OptSpline< typename TStorageTraits<S>::TValue >
{
	typedef TCurve<S> TThis;
	typedef typename TStorageTraits<S>::TValue T;
	typedef typename TStorageTraits<S>::TMod TMod;
	typedef spline::OptSpline<T> super_type;

public:
	using_type(super_type, source_spline);
	using_type(super_type, key_type);

	using super_type::num_keys;
	using super_type::key;
	using super_type::empty;
	using super_type::min_value;
	using super_type::interpolate;

	// Implement serialization.
	string ToString( FToString flags = 0 ) const;
	bool FromString( cstr str, FFromString flags = 0 );

	// Access operators
	T operator ()(float fTime) const
	{
		T val;
		interpolate(fTime, val);
		return val;
	}

	T operator ()(type_max) const
	{
		return T(1);
	}

	T operator ()(type_min) const
	{
		T val;
		min_value(val);
		return val;
	}

	struct CCustomInfo;
	CUSTOM_STRUCT_INFO(CCustomInfo)
};

///////////////////////////////////////////////////////////////////////
//
// Composite parameter types, incorporating base value, randomness, and lifetime curves.
//

template<class S>
struct TVarParam: S
{
	typedef typename TStorageTraits<S>::TValue T;

	// Define random variation capability.
	using_type(TStorageTraits<S>, TRandom);

	struct SRandom: TRandom
	{
		SRandom(TRandom r = TRandom(0))
			: TRandom(r) {}

		const TRandom& Range() const
			{ return static_cast<const TRandom&>(*this); }

		// Define access operators
		T operator () (type_max) const
			{ return T(1); }

		T operator () (type_min) const
			{ return T(1) - Range(); }

		T operator () (float fInterp) const
			{ return T(1) + T(Range() * (fInterp - 1.f)); }

		T operator () (CChaosKey key) const
		{
			if (!!Range())
				return T(1) - T(key.Random(Range()));
			else
				return T(1);
		}

		T operator () (type_random) const
		{
			if (!!Range())
				return T(1) - T(Random(Range()));
			else
				return T(1);
		}
	};

	// Construction.
	TVarParam()
		: S()
	{
	}

	TVarParam(const T& tBase)
		: S(tBase)
	{
	}

	void Set(const T& tBase)
	{
		Base() = tBase;
	}
	void Set(const T& tBase, const TRandom& tRan)
	{
		Base() = tBase;
		m_Random = tRan;
	}

	//
	// Value extraction.
	//
	S& Base()
	{
		return static_cast<S&>(*this);
	}
	const S& Base() const
	{
		return static_cast<const S&>(*this);
	}

	ILINE bool operator !() const
	{
		return Base() == S(0);
	}

	// Value access is base value modified by random function
	template<class R>
	T operator ()(R r) const
	{
		return Base() * m_Random(r);
	}

	// Legacy helper
	ILINE T GetMaxValue() const
	{
		return Base();
	}

	ILINE TRandom GetRandomRange() const
	{
		return m_Random;
	}

	ILINE T GetValueFromMod(T val) const
	{
		return T(Base()) * val;
	}

	AUTO_STRUCT_INFO

protected:

	SRandom			m_Random;			// Random variation, multiplicative.
};

///////////////////////////////////////////////////////////////////////

template<class S>
struct TVarEParam: TVarParam<S>
{
	typedef TVarParam<S> TSuper;
	typedef typename TStorageTraits<S>::TValue	T;

	// Construction.
	ILINE TVarEParam()
	{
	}

	ILINE TVarEParam(const T& tBase)
		: TSuper(tBase)
	{
	}

	//
	// Value extraction.
	//

	T operator ()(type_max) const
	{
		return TSuper::operator()(VMAX);
	}

	T operator ()(type_min) const
	{
		return TSuper::operator()(VMIN) * m_EmitterStrength(VMIN);
	}

	template<class R, class E>
	T operator ()(R r, E e) const
	{
		return TSuper::operator()(r) * m_EmitterStrength(e);
	}

	AUTO_STRUCT_INFO

protected:
	TCurve<S>		m_EmitterStrength;

	// Dependent name nonsense.
	using TSuper::Base;
	using TSuper::m_Random;
};

///////////////////////////////////////////////////////////////////////
template<class S>
struct TVarEPParam: TVarEParam<S>
{
	typedef TVarEParam<S> TSuper;
	typedef typename TStorageTraits<S>::TValue	T;

	// Construction.
	TVarEPParam()
	{}

	TVarEPParam(const T& tBase)
		: TSuper(tBase)
	{}

	//
	// Value extraction.
	//

	T operator ()(type_max) const
	{
		return TSuper::operator()(VMAX);
	}

	T operator ()(type_min) const
	{
		return TSuper::operator()(VMIN) * m_ParticleAge(VMIN);
	}

	template<class R, class E, class P>
	T operator ()(R r, E e, P p) const
	{
		return TSuper::operator()(r, e) * m_ParticleAge(p);
	}

	// Additional helpers
	T GetVarMod(float fEStrength) const
	{
		return m_Random(VRANDOM) * m_EmitterStrength(fEStrength);
	}

	ILINE T GetValueFromBase(T val, float fParticleAge) const
	{
		return val * m_ParticleAge(fParticleAge);
	}

	ILINE T GetValueFromMod(T val, float fParticleAge) const
	{
		return T(Base()) * val * m_ParticleAge(fParticleAge);
	}

	using TSuper::GetValueFromMod;

	AUTO_STRUCT_INFO

protected:

	TCurve<S>	m_ParticleAge;

	// Dependent name nonsense.
	using TSuper::Base;
	using TSuper::m_Random;
	using TSuper::m_EmitterStrength;
};

///////////////////////////////////////////////////////////////////////
template<class S>
struct TRangeParam
{
	S		Min;
	S		Max;

	TRangeParam()
		{}
	TRangeParam(S _min, S _max)
		: Min(_min), Max(_max) {}

	S Interp(float t) const
		{ return Min * (1.f - t) + Max * t; }

	AUTO_STRUCT_INFO
};

///////////////////////////////////////////////////////////////////////
// Special surface type enum

struct CSurfaceTypeIndex
{
	uint16	nIndex;

	STRUCT_INFO
};

///////////////////////////////////////////////////////////////////////
//! Particle system parameters
struct ParticleParams
	: ZeroInit<ParticleParams>
{
	// <Group=Emitter>
	string sComment;
	TSmallBool bEnabled;									// Set false to disable this effect

	DEFINE_ENUM( EInheritance,
		System,
		Standard,
		Zero,
		Parent
	)
	EInheritance eInheritance;						// Source of ParticleParams used as base for this effect (for serialization, display, etc)

	DEFINE_ENUM( ESpawn,
		Direct,
		ParentStart,
		ParentCollide,
		ParentDeath
	)
	ESpawn eSpawnIndirection;							// Direct: spawn from emitter location; else spawn from each particle in parent emitter

	TVarEParam<UFloat> fCount;						// Number of particles alive at once
	struct SMaintainDensity: UFloat
	{
		UFloat fReduceLifeTime;							// 
		UFloat fReduceAlpha;								// <SoftMax=1> Reduce alpha inversely to count increase.
		UFloat fReduceSize;									// 
		AUTO_STRUCT_INFO
	} fMaintainDensity;										// <SoftMax=1> Increase count when emitter moves to maintain spatial density.

	// <Group=Timing>
	TSmallBool bContinuous;								// Emit particles gradually until Count reached (rate = Count / ParticleLifeTime)
	TVarParam<SFloat> fSpawnDelay;				// Delay the emitter start time by this value	
	TVarParam<UFloat> fEmitterLifeTime;		// Lifetime of the emitter, 0 if infinite. Always emits at least Count particles
	TVarParam<SFloat> fPulsePeriod;				// Time between auto-restarts of emitter; 0 if never
	TVarEParam<UFloat> fParticleLifeTime;	// Lifetime of particles, 0 if indefinite (die with emitter)
	TSmallBool bRemainWhileVisible;				// Particles will only die when not rendered (by any viewport)

	// <Group=Location>
	Vec3S vPositionOffset;								// Spawn offset from the emitter position
	Vec3U vRandomOffset;									// Random offset of emission relative position to the spawn position
	UnitFloat fOffsetRoundness;						// Fraction of emit volume corners to round: 0 = box, 1 = ellipsoid
	UnitFloat fOffsetInnerFraction;				// Fraction of inner emit volume to avoid
	EGeomType eAttachType;								// Which geometry to use for attached entity
	EGeomForm eAttachForm;								// Which aspect of attached geometry to emit from

	// <Group=Angles>
	TVarEParam<UHalfAngle> fFocusAngle;		// Angle to vary focus from default (Y axis), for variation
	TVarEParam<SFloat> fFocusAzimuth;			// <SoftMax=360> Angle to rotate focus about default, for variation. 0 = Z axis
	TVarEParam<UnitFloat> fFocusCameraDir;// Rotate emitter focus partially or fully to face camera
	TSmallBool bFocusGravityDir;					// Uses negative gravity dir, rather than emitter Y, as focus dir
	TSmallBool bFocusRotatesEmitter;			// Focus rotation affects offset and particle orientation; else affects just emission direction
	TSmallBool bEmitOffsetDir;						// Default emission direction parallel to emission offset from origin
	TVarEParam<UHalfAngle> fEmitAngle;		// Angle from focus dir (emitter Y), in degrees. RandomVar determines min angle

	DEFINE_ENUM( EFacing,
		Camera,
		CameraX,
		Free,
		Horizontal,
		Velocity,
		Water,
		Terrain,
		Decal
	)
	EFacing eFacing;											// Orientation of particle face
	TSmallBool bOrientToVelocity;					// Particle X axis aligned to velocity direction
	UnitFloat fCurvature;									// For Facing=Camera, fraction that normals are curved to a spherical shape
	
	// <Group=Appearance>
	DEFINE_ENUM_VALS( EBlend, uint8,
		AlphaBased			= OS_ALPHA_BLEND,
		Additive        = OS_ADD_BLEND,
		Multiplicative  = OS_MULTIPLY_BLEND,
		Opaque          = 0,
		_ColorBased     = OS_ADD_BLEND,
		_None           = 0
	)
	EBlend eBlendType;										// Blend rendering type
	CCryName sTexture;										// Texture asset for sprite
	CCryName sMaterial;										// Material (overrides texture)

	struct STextureTiling
	{
		PosInt8	nTilesX, nTilesY;			// Number of tiles texture is split into
		uint8		nFirstTile;						// First (or only) tile to use
		PosInt8	nVariantCount;				// Number of randomly selectable tiles; 0 or 1 if no variation
		PosInt8	nAnimFramesCount;			// Number of tiles (frames) of animation; 0 or 1 if no animation
		DEFINE_ENUM( EAnimationCycle,
			Once,
			Loop,
			Mirror
		)
		EAnimationCycle eAnimCycle;		// How animation cycles
		TSmallBool bAnimBlend;				// Blend textures between frames
		UnitFloat8 fFlipChance;				// Chance each particle will flip in X direction
		UFloat	fAnimFramerate;				// <SoftMax=60> Tex framerate; 0 = 1 cycle / particle life
		TCurve<UFloat> fAnimCurve;		// Animation curve

		STextureTiling()
		{
			TCurve<UFloat>::source_spline source;
			TCurve<UFloat>::key_type key;
			key.time = 0.0f; key.value = 0.0f; key.flags = SPLINE_KEY_TANGENT_LINEAR << SPLINE_KEY_TANGENT_OUT_SHIFT;
			source.insert_key(key);
			key.time = 1.0f; key.value = 1.0f; key.flags = SPLINE_KEY_TANGENT_LINEAR << SPLINE_KEY_TANGENT_IN_SHIFT;
			source.insert_key(key);
			fAnimCurve.from_source(source);
		}

		uint GetTileCount() const
		{
			return nTilesX * nTilesY - nFirstTile;
		}

		uint GetFrameCount() const
		{
			return nVariantCount * nAnimFramesCount;
		}

		float GetAnimPos(float fAge, float fRelativeAge) const
		{
			// Select anim frame based on particle age.
			float fAnimPos = 0.0f;
			if (fAnimFramerate > 0.f)
			{
				fAnimPos = fAge * fAnimFramerate / nAnimFramesCount;
				if (eAnimCycle == eAnimCycle.Loop)
					fAnimPos = fmod(fAnimPos, 1.f);
				else if (eAnimCycle == eAnimCycle.Mirror)
					fAnimPos = 1.f - abs(fmod(fAnimPos, 2.f) - 1.f);
				else
					fAnimPos = min(fAnimPos, 1.f);
			}
			else
				fAnimPos = fRelativeAge;
			fAnimPos = fAnimCurve(fAnimPos);
			return fAnimPos;
		}

		float GetAnimPosScale() const
		{
			// Scale animation position to frame number
			if (eAnimCycle)
				return float(nAnimFramesCount);
			else if (bAnimBlend)
				return float(nAnimFramesCount-1);
			else
				// If not cycling, reducing slightly to avoid hitting 1.
				return float(nAnimFramesCount) - 0.001f;
		}

		void Correct()
		{
			nFirstTile = min<uint>(nFirstTile, nTilesX * nTilesY - 1);
			nAnimFramesCount = min<uint>(nAnimFramesCount, GetTileCount());
			nVariantCount = min<uint>(nVariantCount, GetTileCount() / nAnimFramesCount);
		}

		AUTO_STRUCT_INFO
	};

	STextureTiling TextureTiling;					// Tiling of texture for animation and variation
	TSmallBool bTessellation;							// If hardware supports, tessellate particles for better shadowing and curved connected particles
	TSmallBool bOctagonalShape;						// Use octagonal shape for textures instead of quad
	struct SSoftParticle : TSmallBool
	{
		TVarEPParam<UFloat> fSoftness;					// Soft particles scale
		SSoftParticle()
			:	fSoftness(1.0f) {}
		AUTO_STRUCT_INFO
	} bSoftParticle;											// Soft intersection with background
	CCryName sGeometry;										// Geometry for 3D particles
	DEFINE_ENUM( EGeometryPieces,
		Whole,
		RandomPiece,
		AllPieces
	)
	EGeometryPieces eGeometryPieces;			// Which geometry pieces to emit
	TSmallBool bNoOffset;									// Disable centering of geometry
	TVarEPParam<UFloat> fAlpha;						// <SoftMax=1> Alpha value (opacity, or multiplier for additive)
	struct SAlphaClip
	{
		TRangeParam<UFloat>			fScale;						// <SoftMax=1> Final alpha multiplier, for particle alpha 0 to 1
		TRangeParam<UnitFloat>	fSourceMin;				// Source alpha clip min, for particle alpha 0 to 1
		TRangeParam<UnitFloat>	fSourceWidth;			// Source alpha clip range, for particle alpha 0 to 1

		SAlphaClip()
			: fScale(0, 1), fSourceWidth(1, 1) {}

		AUTO_STRUCT_INFO
	} AlphaClip;													// Alpha clipping settings, for particle alpha 0 to 1
	TVarEPParam<Color3F> cColor;					// Color modulation

	// <Group=Lighting>
	UFloat fDiffuseLighting;							// <SoftMax=1> Multiplier for particle dynamic lighting
	UnitFloat fDiffuseBacklighting;				// Fraction of diffuse lighting applied in all directions
	UFloat fEmissiveLighting;							// <SoftMax=1> Multiplier for particle emissive lighting

	TSmallBool bReceiveShadows;						// Shadows will cast on these particles
	TSmallBool bCastShadows;							// Particles will cast shadows (currently only geom particles)
	TSmallBool bNotAffectedByFog;					// Ignore fog
	TSmallBool bGlobalIllumination;				// Enable global illumination in the shader

	struct SLightSource
	{
		TVarEPParam<UFloat> fRadius;				// <SoftMax=10> Radius of light
		TVarEPParam<UFloat> fIntensity;			// <SoftMax=10> Intensity of light (color from Appearance/Color)
		SFloat fHDRDynamic;									// <SoftMin=-2> $<SoftMax=2>
		AUTO_STRUCT_INFO
	} LightSource;												// Per-particle light generation

	// <Group=Audio>
	CCryName sStartTrigger;								// Audio start trigger to execute.
	CCryName sStopTrigger;								// Audio stop trigger to execute.
	TVarEParam<UFloat> fSoundFXParam;			// Custom real-time sound modulation parameter
	DEFINE_ENUM( ESoundControlTime,
		EmitterLifeTime,
		EmitterExtendedLifeTime,
		EmitterPulsePeriod
	)
	ESoundControlTime eSoundControlTime;	// The sound control time type

	// <Group=Size>
	TVarEPParam<UFloat> fSize;						// <SoftMax=8> Particle radius, for sprites; size scale for geometry
	TVarEPParam<UFloat> fAspect;					// <SoftMax=8> X-to-Y scaling factor
	TVarEPParam<SFloat> fPivotX;					// <SoftMin=-1> <SoftMax=1> Pivot offset in X direction
	TVarEPParam<SFloat> fPivotY;					// <SoftMin=-1> <SoftMax=1> Pivot offset in Y direction

	struct SStretch: TVarEPParam<UFloat>
	{
		SFloat fOffsetRatio;								// <SoftMin=-1> $<SoftMax=1> Move particle center this fraction in direction of stretch
		AUTO_STRUCT_INFO
	} fStretch;														// <SoftMax=1> Stretch particle into moving direction, amount in seconds
	struct STailLength: TVarEPParam<UFloat>
	{
		uint8 nTailSteps;										// <SoftMax=16> Number of tail segments
		AUTO_STRUCT_INFO
	} fTailLength;												// <SoftMax=10> Length of tail in seconds
	UFloat fMinPixels;										// <SoftMax=10> Augment true size with this many pixels

	// Connection
	struct SConnection: TSmallBool
	{
		TSmallBool bConnectParticles;							// Particles are drawn connected serially
		TSmallBool bConnectToOrigin;							// Newest particle connected to emitter origin
		DEFINE_ENUM( ETextureMapping,
			PerParticle,
			PerStream
		)
		ETextureMapping eTextureMapping;					// Basis of texture coord mapping (particle or stream)
		TSmallBool bTextureMirror;								// Mirror alternating texture tiles; else wrap
		SFloat fTextureFrequency;									// <SoftMax=8> Number of texture wraps in line

		SConnection() : 
			bTextureMirror(true), fTextureFrequency(1.f) {}
		AUTO_STRUCT_INFO
	} Connection;

	// <Group=Movement>
	TVarEParam<SFloat> fSpeed;						// Initial speed of a particle
	SFloat fInheritVelocity;							// <SoftMin=0> $<SoftMax=1> Fraction of emitter's velocity to inherit
	struct SAirResistance: TVarEPParam<UFloat>
	{
		UFloat fRotationalDragScale;				// <SoftMax=1> Multiplier to AirResistance, for rotational motion
		UFloat fWindScale;									// <SoftMax=1> Artificial adjustment to environmental wind

		SAirResistance()
			: fRotationalDragScale(1), fWindScale(1) {}
		AUTO_STRUCT_INFO
	}	fAirResistance;											// <SoftMax=10> Air drag value, in inverse seconds
	TVarEPParam<SFloat> fGravityScale;		// <SoftMin=-1> $<SoftMax=1> Multiplier for world gravity
	Vec3S vAcceleration;									// Explicit world-space acceleration vector
	TVarEPParam<UFloat> fTurbulence3DSpeed;// <SoftMax=10> 3D random turbulence force
	TVarEPParam<UFloat> fTurbulenceSize;	// <SoftMax=10> Radius of vortex rotation (axis is direction of movement)
	TVarEPParam<SFloat> fTurbulenceSpeed;	// <SoftMin=-360> $<SoftMax=360> Angular speed of vortex rotation

	DEFINE_ENUM( EMoveRelative,
		No,
		Yes,
		YesWithTail
	)
	EMoveRelative eMoveRelEmitter;				// Particle motion is in emitter space
	TSmallBool bBindEmitterToCamera;			// Emitter attached to main render camera
	TSmallBool bSpaceLoop;								// Loops particles within emission volume, or within Camera Max Distance

	struct STargetAttraction							
	{																			
		DEFINE_ENUM( ETargeting,
			External,
			OwnEmitter,
			Ignore
		)
		ETargeting eTarget;									// Source of target attractor
		TSmallBool bExtendSpeed;						// Extend particle speed as necessary to reach target in normal lifetime
		TSmallBool bShrink;									// Shrink particle as it approaches target
		TSmallBool bOrbit;									// Orbit target at specified distance, rather than disappearing
		TVarEPParam<SFloat> fRadius;				// Radius of attractor, for vanishing or orbiting
		AUTO_STRUCT_INFO
	} TargetAttraction;										// Specify target attractor behavior

	// <Group=Rotation>
	Vec3_Zero<SAngle> vInitAngles;				// Initial rotation in symmetric angles (degrees)
	Vec3_Zero<UFullAngle> vRandomAngles;	// Bidirectional random angle variation
	Vec3S vRotationRate;									// <SoftMin=-360> $<SoftMax=360> Rotation speed (degree/sec)
	Vec3U vRandomRotationRate;						// <SoftMax=360> Random variation of rotation speed

	// <Group=Collision>
	DEFINE_ENUM( EPhysics,
		None,
		SimpleCollision,
		SimplePhysics,
		RigidBody
	)
	EPhysics ePhysicsType;								// What kind of physics simulation to run on particle
	TSmallBool bCollideTerrain;						// Collides with terrain (if Physics <> none)
	TSmallBool bCollideStaticObjects;			// Collides with static physics objects (if Physics <> none)
	TSmallBool bCollideDynamicObjects;		// Collides with dynamic physics objects (if Physics <> none)
	UnitFloat8 fCollisionFraction;				// Fraction of emitted particles that actually perform collisions
	UFloat fCollisionCutoffDistance;			// Maximum distance up until collisions are respected (0 = infinite)
	uint8 nMaxCollisionEvents;						// Max # collision events per particle (0 = no limit)
	DEFINE_ENUM( ECollisionResponse,
		Die,
		Ignore,
		Stop
	)
	ECollisionResponse eFinalCollision;		// What to do on final collision (when MaxCollisions > 0)
	CSurfaceTypeIndex sSurfaceType;				// Surface type for physicalized particles
	SFloat fElasticity;										// <SoftMin=0> $<SoftMax=1> Collision bounce coefficient: 0 = no bounce, 1 = full bounce
	UFloat fDynamicFriction;							// <SoftMax=10> Sliding drag value, in inverse seconds
	UFloat fThickness;										// <SoftMax=1> Lying thickness ratio - for physicalized particles only
	UFloat fDensity;											// <SoftMax=2000> Mass density for physicslized particles

	// <Group=Visibility>
	UFloat fViewDistanceAdjust;						// <SoftMax=1> Multiplier to automatic distance fade-out
	UFloat fCameraMaxDistance;						// <SoftMax=100> Max distance from camera to render particles
	UFloat fCameraMinDistance;						// <SoftMax=100> Min distance from camera to render particles
	SFloat fCameraDistanceOffset;					// <SoftMin=-1> <SoftMax=1> Offset the emitter away from the camera
	SFloat fSortOffset;										// <SoftMin=-1> <SoftMax=1> Offset distance used for sorting
	SFloat fSortBoundsScale;							// <SoftMin=-1> <SoftMax=1> Choose emitter point for sorting; 1 = bounds nearest, 0 = origin, -1 = bounds farthest
	TSmallBool bDrawNear;									// Render particle in near space (weapon)
	TSmallBool bDrawOnTop;								// Render particle on top of everything (no depth test)
	ETrinary tVisibleIndoors;							// Whether visible indoors / outdoors / both
	ETrinary tVisibleUnderwater;					// Whether visible under / above water / both

	// <Group=Advanced>
	DEFINE_ENUM( EForce,
		None,
		Wind,
		Gravity,
		// Compatibility
		_Target
	)
	EForce eForceGeneration;							// Generate physical forces if set
	UFloat fFillRateCost;									// Adjustment to max screen fill allowed per emitter
#ifdef PARTICLE_MOTION_BLUR
	UFloat fMotionBlurScale;							// Multiplier for motion blur texture blurring
	UFloat fMotionBlurStretchScale;				// Multiplier for motion blur sprite stretching based on particle movement
	UFloat fMotionBlurCamStretchScale;		// Multiplier for motion blur sprite stretching based on camera movement
#endif
	TFixed<uint8,MAX_HEATSCALE> fHeatScale;  // Multiplier to thermal vision
	UnitFloat fSphericalApproximation;		// Align the particle to the tangent of the sphere
	UFloat fPlaneAlignBlendDistance;      // Distance when blend to camera plane aligned particles starts

	TRangedType<uint8,0,2> nSortQuality;	// Sort new particles as accurately as possible into list, by main camera distance
	TSmallBool bHalfRes;									// Use half resolution rendering
	TSmallBoolTrue bStreamable;						// Texture/geometry allowed to be streamed

	// <Group=Configuration>
	DEFINE_ENUM( EConfigSpecBrief,
		Low,
		Medium,
		High,
		VeryHigh
	)
	EConfigSpecBrief eConfigMin;					// Minimum config spec this effect runs in
	EConfigSpecBrief eConfigMax;					// Maximum config spec this effect runs in

	struct SPlatforms											
	{
		TSmallBoolTrue PCDX11, PS4, XBoxOne;
		AUTO_STRUCT_INFO
	} Platforms;													// Platforms this effect runs on

	ParticleParams() :
		bEnabled(true),
		fSize(1.f),
		fAspect(1.f),
		cColor(1.f),
		fAlpha(1.f),
		eBlendType(EBlend::AlphaBased),
		fDiffuseLighting(1.f),
		fCurvature(1.f),
		fViewDistanceAdjust(1.f),
		fFillRateCost(1.f),
		fDensity(1000.f),
		fThickness(1.f),
		fCollisionFraction(1.f),
#ifdef PARTICLE_MOTION_BLUR
		fMotionBlurScale(1.f),
		fMotionBlurStretchScale(1.f),
		fMotionBlurCamStretchScale(1.f),
#endif
		fSphericalApproximation(1.f),
		fSoundFXParam(1.f),
		eConfigMax(eConfigMax.VeryHigh)
	{}

	ParticleParams(type_zero)
		{}

	// Derived properties
	bool IsImmortal() const
	{
		return bEnabled && ((bContinuous && !fEmitterLifeTime && fCount) || fPulsePeriod || !fParticleLifeTime);
	}
	bool HasEquilibrium() const
	{
		// Effect reaches a steady state: No emitter lifespan or pulsing.
		return bContinuous && fCount && fParticleLifeTime && !fPulsePeriod && !fEmitterLifeTime;
	}
	float GetMaxSpawnDelay() const
	{
		return eSpawnIndirection >= ESpawn::ParentCollide ? fHUGE : fSpawnDelay.GetMaxValue();
	}
	float GetMaxEmitterLife() const
	{
		return GetMaxSpawnDelay() + (bContinuous ? ZeroIsHuge(fEmitterLifeTime.GetMaxValue()) : 0.f);
	}
	float GetMaxParticleLife() const
	{
		return if_pos_else( fParticleLifeTime.GetMaxValue(), fParticleLifeTime.GetMaxValue(), ZeroIsHuge(fEmitterLifeTime.GetMaxValue()) );
	}
	uint8 GetTailSteps() const
	{
		return fTailLength ? fTailLength.nTailSteps : 0;
	}
	bool HasVariableVertexCount() const
	{
		return GetTailSteps() || Connection;
	}
	float GetMaxRotationAngle() const
	{
		if (vRotationRate.x != 0.f || vRotationRate.z != 0.f || vRandomRotationRate.x != 0 || vRandomRotationRate.z != 0.f)
			return gf_PI;
		return DEG2RAD( max( abs(vInitAngles.x) + abs(vRandomAngles.x), abs(vInitAngles.z) + abs(vRandomAngles.z) ) );
	}
	AABB GetEmitOffsetBounds() const
	{
		return AABB(vPositionOffset - vRandomOffset, vPositionOffset + vRandomOffset);
	}
	float GetFullTextureArea() const
	{
		// World area corresponding to full texture, thus multiplied by tile count.
		return sqr(2.f * fSize.GetMaxValue()) * float(TextureTiling.nTilesX * TextureTiling.nTilesY);
	}
	float GetAlphaFromMod(float fMod) const
	{
		return fAlpha.GetMaxValue() * AlphaClip.fScale.Interp(fMod);
	}

	AUTO_STRUCT_INFO
};

#endif

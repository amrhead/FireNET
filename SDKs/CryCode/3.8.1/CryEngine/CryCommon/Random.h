// Copyright 2001-2015 Crytek GmbH. All rights reserved.

#pragma once

#include "BaseTypes.h"
#include "LCGRandom.h"
#include "MTPseudoRandom.h"

namespace CryRandom_Internal
{
// Private access point to the global random number generator.
extern CRndGen g_random_generator;
}


// Seed the global random number generator.
inline void cry_random_seed(const uint32 nSeed)
{ 
	CryRandom_Internal::g_random_generator.Seed(nSeed);
}

inline uint32 cry_random_uint32()
{
	return CryRandom_Internal::g_random_generator.GenerateUint32();
}

// Ranged function returns random value within the *inclusive* range
// between minValue and maxValue.
// Any orderings work correctly: minValue <= maxValue and 
// minValue >= minValue.
template <class T>
inline T cry_random(const T minValue, const T maxValue)
{
	return CryRandom_Internal::g_random_generator.GetRandom(minValue, maxValue);
}

// Vector (Vec2, Vec3, Vec4) ranged function returns vector with
// every component within the *inclusive* ranges between minValue.component
// and maxValue.component.
// All orderings work correctly: minValue.component <= maxValue.component and
// minValue.component >= maxValue.component.
template <class T>
inline T cry_random_componentwise(const T& minValue, const T& maxValue)
{
	return CryRandom_Internal::g_random_generator.GetRandomComponentwise(minValue, maxValue);
}

// The function returns a random unit vector (Vec2, Vec3, Vec4).
template <class T>
inline T cry_random_unit_vector()
{
	return CryRandom_Internal::g_random_generator.GetRandomUnitVector<T>();
}

// eof

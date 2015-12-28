// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#include "SDLMixerSoundEngine.h"
#include <CryCrc32.h>
#include <CryFile.h>
#include <CryPath.h>

#ifndef SDLMIXER_SOUND_ENGINE_UTIL_H_INCLUDED
#define SDLMIXER_SOUND_ENGINE_UTIL_H_INCLUDED

namespace SDLMixer
{
	inline const TSDLMixerID GetIDFromString(const string& sName)
	{
		string sLowerName = sName;
		sLowerName.MakeLower();
		return CCrc32::Compute(sLowerName.c_str());
	}

	inline const TSDLMixerID GetIDFromFilePath(const string& sFilePath)
	{
		return GetIDFromString(PathUtil::GetFile(sFilePath));
	}

	inline void GetDistanceAngleToObject(const SATLWorldPosition& listener, const SATLWorldPosition& object, float& out_nDistance, float& out_nAngle)
	{
		const Vec3 objectPos = object.GetPositionVec();
		const Vec3 listenerPos = listener.GetPositionVec();
		const Vec3 listenerToObject = objectPos - listenerPos;

		// Distance
		const float fMaxDistance = 30.0f;
		const uint8 nSDLMaxDistance = 255;
		out_nDistance = listenerToObject.len();

		// Angle
		// Project point to plane formed by the listeners position/direction
		Vec3 n = listener.mPosition.TransformVector(Vec3Constants<float>::fVec3_OneZ);
		n.Normalize();
		Vec3 objectDir = Vec3::CreateProjection(listenerToObject, n).normalized();

		// Get angle between listener position and projected point
		const Vec3 listenerDir = listener.GetForwardVec().normalized();
		out_nAngle = RAD2DEG(asin(objectDir.Cross(listenerDir).Dot(n)));
	}
}

#endif // SDLMIXER_SOUND_ENGINE_UTIL_H_INCLUDED

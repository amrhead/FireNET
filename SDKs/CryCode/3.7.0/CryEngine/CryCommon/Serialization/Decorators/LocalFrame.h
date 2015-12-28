#pragma once

#include <Cry_Math.h>
#include "Serialization/Math.h"

namespace Serialization
{

class IArchive;

struct LocalPosition
{
	Vec3* value;
	int space;
	const char* parentName;
	const void* handle;

	LocalPosition(Vec3& vec, int space, const char* parentName, const void* handle)
	: value(&vec)
	, space(space)
	, parentName(parentName)
	, handle(handle)
	{
	}

	void Serialize(IArchive& ar);
};

struct LocalOrientation
{
	Quat* value;
	int space;
	const char* parentName;
	const void* handle;

	LocalOrientation(Quat& vec, int space, const char* parentName, const void* handle)
	: value(&vec)
	, space(space)
	, parentName(parentName)
	, handle(handle)
	{
	}

	void Serialize(IArchive& ar);
};

struct LocalFrame
{
	QuatT* value;
	const char* parentName;
	int space;
	const void* handle;

	LocalFrame(QuatT& vec, int space, const char* parentName, const void* handle)
	: value(&vec)
	, parentName(parentName)
	, space(space)
	, handle(handle)
	{
	}

	void Serialize(IArchive& ar);
};

enum 
{
	SPACE_JOINT,
	SPACE_ENTITY,
	SPACE_JOINT_WITH_PARENT_ROTATION,
	SPACE_JOINT_SET_RELATIVE_TO_BIND_POSE
};


inline LocalFrame LocalToJoint(QuatT& transform, const string& jointName, const void* handle = 0)
{
	return LocalFrame(transform, SPACE_JOINT, jointName.c_str(), handle ? handle : &transform);
}

inline LocalFrame LocalToEntity(QuatT& transform, const void* handle = 0)
{
	return LocalFrame(transform, SPACE_ENTITY, "", handle ? handle : &transform);
}

inline LocalFrame LocalToEntity(QuatT& transform, const string& jointName, const void* handle = 0)
{
	return LocalFrame(transform, SPACE_ENTITY, jointName.c_str(), handle ? handle : &transform);
}

inline LocalFrame LocalToCharacter(QuatT& transform, const string& jointName, const void* handle = 0)
{
	return LocalFrame(transform, SPACE_JOINT_SET_RELATIVE_TO_BIND_POSE, jointName.c_str(), handle ? handle : &transform);
}
inline LocalOrientation LocalToCharacter(Quat& transform, const string& jointName, const void* handle = 0)
{
	return LocalOrientation(transform, SPACE_JOINT_SET_RELATIVE_TO_BIND_POSE, jointName.c_str(), handle ? handle : &transform);
}

inline LocalPosition LocalToCharacter(Vec3& transform, const string& jointName, const void* handle = 0)
{
	return LocalPosition(transform, SPACE_JOINT_SET_RELATIVE_TO_BIND_POSE, jointName.c_str(), handle ? handle : &transform);
}

inline LocalOrientation LocalToJoint(Quat& orientation, const string& jointName, const void* handle = 0)
{
	return LocalOrientation(orientation, SPACE_JOINT, jointName.c_str(), handle ? handle : &orientation);
}


inline LocalOrientation LocalToEntity(Quat& orientation, const void* handle = 0)
{
	return LocalOrientation(orientation, SPACE_ENTITY, "", handle ? handle : &orientation);
}

inline LocalPosition LocalToEntity(Vec3& position, const void* handle = 0)
{
	return LocalPosition(position, SPACE_ENTITY, "", handle ? handle : &position);
}

inline LocalPosition LocalToJoint(Vec3& position, const string& jointName, const void* handle = 0)
{
	return LocalPosition(position, SPACE_JOINT, jointName.c_str(), handle ? handle : &position);
}

bool Serialize(Serialization::IArchive& ar, Serialization::LocalPosition& value, const char* name, const char* label);
bool Serialize(Serialization::IArchive& ar, Serialization::LocalOrientation& value, const char* name, const char* label);
bool Serialize(Serialization::IArchive& ar, Serialization::LocalFrame& value, const char* name, const char* label);

}

#include "LocalFrameImpl.h"

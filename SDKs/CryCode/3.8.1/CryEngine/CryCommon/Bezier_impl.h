// Crytek Engine Header File
// (c) 2015 Crytek GmbH

#ifndef __BEZIER_IMPL_H__
#define __BEZIER_IMPL_H__

#include <Bezier.h>
#include <Serialization/Enum.h>

SERIALIZATION_ENUM_BEGIN_NESTED(SBezierControlPoint, ETangentType, "TangentType")
	SERIALIZATION_ENUM_VALUE_NESTED(SBezierControlPoint, eTangentType_Custom, "Custom")
	SERIALIZATION_ENUM_VALUE_NESTED(SBezierControlPoint, eTangentType_Auto, "Smooth")
	SERIALIZATION_ENUM_VALUE_NESTED(SBezierControlPoint, eTangentType_Zero, "Zero")
	SERIALIZATION_ENUM_VALUE_NESTED(SBezierControlPoint, eTangentType_Step, "Step")
	SERIALIZATION_ENUM_VALUE_NESTED(SBezierControlPoint, eTangentType_Linear, "Linear")
SERIALIZATION_ENUM_END()

#endif
#ifndef cryphysics_h
#define cryphysics_h
#pragma once

#ifdef PHYSICS_EXPORTS
	#define CRYPHYSICS_API __declspec(dllexport)
#else
	#define CRYPHYSICS_API __declspec(dllimport)
	#define vector_class Vec3d
#endif


#include "utils.h"
#include "primitives.h"
#include <physinterface.h> // <> required for Interfuscator

extern "C" CRYPHYSICS_API IPhysicalWorld *CreatePhysicalWorld();

#endif
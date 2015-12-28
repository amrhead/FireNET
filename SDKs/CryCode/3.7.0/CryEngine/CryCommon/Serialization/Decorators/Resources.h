// Copyright (c) 2012 Crytek GmbH
#pragma once
#include "ResourceSelector.h"

namespace Serialization
{
	// animation resources
	template<class T> ResourceSelector<T> AnimationAlias(T& s) { return ResourceSelector<T>(s, "AnimationAlias"); } // "name" from animation set
	template<class T> ResourceSelector<T> AnimationPath(T& s) { return ResourceSelector<T>(s, "Animation"); }
	inline ResourceSelectorWithId AnimationPathWithId(string& s, int id) { return ResourceSelectorWithId(s, "Animation", id); }
	template<class T> ResourceSelector<T> CharacterPath(T& s) { return ResourceSelector<T>(s, "Character"); }
	template<class T> ResourceSelector<T> CharacterPhysicsPath(T& s) { return ResourceSelector<T>(s, "CharacterPhysics"); }
	template<class T> ResourceSelector<T> CharacterRigPath(T& s) { return ResourceSelector<T>(s, "CharacterRig"); }
	template<class T> ResourceSelector<T> SkeletonPath(T& s) { return ResourceSelector<T>(s, "Skeleton"); }
	template<class T> ResourceSelector<T> SkeletonParamsPath(T& s) { return ResourceSelector<T>(s, "SkeletonParams"); } // CHRParams
	template<class T> ResourceSelector<T> JointName(T& s) { return ResourceSelector<T>(s, "Joint"); }
	template<class T> ResourceSelector<T> AttachmentName(T& s) { return ResourceSelector<T>(s, "Attachment"); }

	// miscelaneous resources
	template<class T> ResourceSelector<T> SoundName(T& s) { return ResourceSelector<T>(s, "Sound"); }
	template<class T> ResourceSelector<T> DialogName(T& s) { return ResourceSelector<T>(s, "Dialog"); }
	template<class T> ResourceSelector<T> ForceFeedbackIdName(T& s) { return ResourceSelector<T>(s, "ForceFeedbackId"); }
	template<class T> ResourceSelector<T> ModelFilename(T& s) { return ResourceSelector<T>(s, "Model"); }
	template<class T> ResourceSelector<T> ParticleName(T& s) { return ResourceSelector<T>(s, "Particle"); }
	
	namespace Decorators 
	{
		// Decorators namespace is obsolete now, SHOULD NOT BE USED.
		template<class T> ResourceSelector<T> AnimationName(T& s) { return ResourceSelector<T>(s, "Animation"); }
		using Serialization::SoundName;
		using Serialization::AttachmentName;
		template<class T> ResourceSelector<T> ObjectFilename(T& s) { return ResourceSelector<T>(s, "Model"); }
		using Serialization::JointName;
		using Serialization::ForceFeedbackIdName;
	}
}
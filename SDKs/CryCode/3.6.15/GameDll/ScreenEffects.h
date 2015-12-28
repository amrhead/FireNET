// ScreenEffects - Allows for simultaneous and queued blending of effects
//  John Newfield
//  
//  23-1-2008: Refactored by Benito G.R.

#ifndef _SCREEN_EFFECTS_H_
#define _SCREEN_EFFECTS_H_

#pragma once

struct IBlendType;
struct IBlendedEffect;
class	 CBlendGroup;
struct IActor;

//-ScreenEffects------------------------
class CScreenEffects
{

public:

	//Some prefenided group IDs
	enum ScreenEffectsGroupId
	{
		eSFX_GID_RBlur			= 1,
		eSFX_GID_ZoomIn			= 2,
		eSFX_GID_ZoomOut		= 3,
		eSFX_GID_HitReaction= 4,
		eSFX_GID_MotionBlur = 5,
		eSFX_GID_Last
	};

	enum CameraShakeGroupId
	{
		eCS_GID_Default = 1,
		eCS_GID_Player = 2,
		eCS_GID_Weapon = 3,
		eCS_GID_HitRecoil = 4,
	};

	CScreenEffects();
	~CScreenEffects();

	void Reset();

	void ResetAllBlendGroups(bool resetScreen = false);
	
	//Update functions
	void Update(float frameTime);
	void PostUpdate(float frameTime);

	// Camera shake
	void CamShake(Vec3 rotateShake, Vec3 shiftShake, float freq, float shakeTime, float randomness = 0, int shakeID=eCS_GID_Default);

	// List of effect calls (add custom ones here)....
	void ProcessExplosionEffect(float blurRadius, const Vec3& explosionPos);
	void ProcessZoomInEffect();
	void ProcessZoomOutEffect();
	void ProcessSlidingFX();
	void ProcessSprintingFX(bool sprinting, bool isInAir);
	//~ List of effects

	void GetMemoryStatistics(ICrySizer * s);

private:

	// Update x/y coords
	void SetUpdateCoords(const char *coordsXname, const char *coordsYname, Vec3 pos);

	void EnableBlends(bool enable) {m_enableBlends = enable;};
	void EnableBlends(bool enable, int blendGroup);

	// Clear a blend group (deletes running blends)
	void ClearBlendGroup(int blendGroup, bool resetScreen = false);
	void ClearAllBlendGroups(bool resetScreen = false);

	// Reset a blend group (do not delete the group)
	void ResetBlendGroup(int blendGroup, bool resetScreen = false);
	void ResetScreen();

	void ResetGameEffectPools();

	int GetUniqueID();

	// Start a blend
	void StartBlend(IBlendedEffect *effect, IBlendType *blendType, float speed, int blendGroup);
	bool HasJobs(int blendGroup);


	// Maps blend group IDs to blend groups
	std::map<int, CBlendGroup*> m_blends;
	std::map<int, bool> m_enabledGroups;
	int     m_curUniqueID;
	bool    m_enableBlends;
	bool    m_updatecoords;
	string  m_coordsXname;
	string  m_coordsYname;
	Vec3    m_coords3d;
};

#endif
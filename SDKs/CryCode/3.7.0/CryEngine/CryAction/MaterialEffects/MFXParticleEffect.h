// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef __MFXPARTICLEEFFECT_H__
#define __MFXPARTICLEEFFECT_H__

#pragma once

#include "MFXEffectBase.h"

struct SMFXParticleEntry 
{
  string name;
  string userdata;
  float scale; // base scale
  float maxdist; // max distance for spawning this effect
  float minscale; // min scale (distance == 0)
  float maxscale; // max scale (distance == maxscaledist)
  float maxscaledist; 
  bool  attachToTarget;

  SMFXParticleEntry()
	  : scale(1.0f)
	  , maxdist(0.0f)
	  , minscale(0.0f)
	  , maxscale(0.0f)
	  , maxscaledist(0.0f)
	  , attachToTarget(false)
  {
   
  }  

	void GetMemoryUsage(ICrySizer *pSizer) const
	{
		pSizer->AddObject(name);
		pSizer->AddObject(userdata);
	}
};

typedef std::vector<SMFXParticleEntry> SMFXParticleEntries;

struct SMFXParticleParams 
{  
	enum EDirectionType
	{
		eDT_Normal   = 0,
		eDT_Ricochet,
	};

  SMFXParticleEntries m_entries;
	EDirectionType      directionType;
};

class CMFXParticleEffect :
	public CMFXEffectBase
{
	typedef CryFixedStringT<32> TAttachmentName;

public:

	CMFXParticleEffect();
	virtual ~CMFXParticleEffect();

	//IMFXEffect
	virtual void Execute(const SMFXRunTimeEffectParams& params) OVERRIDE;
	virtual void LoadParamsFromXml(const XmlNodeRef& paramsNode) OVERRIDE;
	virtual void GetResources(SMFXResourceList& resourceList) const OVERRIDE;
	virtual void PreLoadAssets() OVERRIDE;
	virtual void GetMemoryUsage( ICrySizer *pSizer ) const OVERRIDE;
	//~IMFXEffect

protected:

	bool AttachToTarget(const SMFXParticleEntry& particleParams, const SMFXRunTimeEffectParams& params, IParticleEffect* pParticleEffect, const Vec3& dir, float scale);
	bool AttachToCharacter(IEntity& targetEntity, const SMFXParticleEntry& particleParams, const SMFXRunTimeEffectParams& params, const Vec3& dir, float scale);
	bool AttachToEntity(IEntity& targetEntity, const SMFXParticleEntry& particleParams, const SMFXRunTimeEffectParams& params, IParticleEffect* pParticleEffect, const Vec3& dir, float scale);

	void GetNextCharacterAttachmentName(TAttachmentName& attachmentName);

	SMFXParticleParams m_particleParams;
};

#endif //__MFXPARTICLEEFFECT_H__

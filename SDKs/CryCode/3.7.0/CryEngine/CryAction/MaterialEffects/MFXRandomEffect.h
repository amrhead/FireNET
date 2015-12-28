////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2006.
// -------------------------------------------------------------------------
//  File name:   MFXRandomEffect.h
//  Version:     v1.00
//  Created:     28/11/2006 by JohnN/AlexL
//  Compilers:   Visual Studio.NET
//  Description: Random effect (randomly plays one of its child effects)
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __MFXRANDOMEFFECT_H__
#define __MFXRANDOMEFFECT_H__

#pragma once

#include "MFXEffectBase.h"
#include "MFXContainer.h"


class CMFXRandomizerContainer
	: public CMFXContainer
{
public:
	void ExecuteRandomizedEffects(const SMFXRunTimeEffectParams& params);

private:
	TMFXEffectBasePtr ChooseCandidate(const SMFXRunTimeEffectParams& params) const;
};

class CMFXRandomEffect :
	public CMFXEffectBase
{

public:
	CMFXRandomEffect();
	virtual ~CMFXRandomEffect();

	//IMFXEffect
	virtual void Execute(const SMFXRunTimeEffectParams& params) OVERRIDE;
	virtual void LoadParamsFromXml(const XmlNodeRef& paramsNode) OVERRIDE;
	virtual void SetCustomParameter(const char* customParameter, const SMFXCustomParamValue& customParameterValue) OVERRIDE;
	virtual void GetResources(SMFXResourceList& resourceList) const OVERRIDE;
	virtual void PreLoadAssets() OVERRIDE;
	virtual void ReleasePreLoadAssets() OVERRIDE;
	virtual void GetMemoryUsage( ICrySizer *pSizer ) const OVERRIDE;
	//~IMFXEffect

private:
	CMFXRandomizerContainer m_container;
};

#endif

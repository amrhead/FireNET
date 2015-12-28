//  CryEngine Source File.
//  Copyright (C), Crytek, 1999-2014.

#include "StdAfx.h"
#include "MFXEffectBase.h"

#include <IMaterialEffects.h>

CMFXEffectBase::CMFXEffectBase( const uint16 _typeFilterFlag )
{
	m_runtimeExecutionFilter.AddFlags(_typeFilterFlag);
}

void CMFXEffectBase::SetCustomParameter( const char* customParameter, const SMFXCustomParamValue& customParameterValue )
{

}

void CMFXEffectBase::PreLoadAssets()
{

}

void CMFXEffectBase::ReleasePreLoadAssets()
{

}

bool CMFXEffectBase::CanExecute( const SMFXRunTimeEffectParams& params ) const
{
	return ((params.playflags & m_runtimeExecutionFilter.GetRawFlags()) != 0);
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//  CryEngine Source File.
//  Copyright (C), Crytek, 1999-2014.
// ----------------------------------------------------------------------------------------
//  File name:   MFXAudioEffect.h
//  Description: MFX system subclass which takes care of interfacing with audio system
//
//////////////////////////////////////////////////////////////////////////////////////////// 

#ifndef _MFX_AUDIO_H_
#define _MFX_AUDIO_H_

#pragma once

#include "MFXEffectBase.h"
#include <IAudioInterfacesCommonData.h>

struct SAudioTriggerWrapper
{
	SAudioTriggerWrapper()
		: m_triggerID(INVALID_AUDIO_CONTROL_ID)
	{
	}

	void Init(const char* triggerName);

	TAudioControlID GetTriggerId() const 
	{ 
		return m_triggerID;
	} 

	bool IsValid() const
	{
		return (m_triggerID != INVALID_AUDIO_CONTROL_ID);
	}

	const char* GetTriggerName() const
	{
#if defined(MATERIAL_EFFECTS_DEBUG)
		return m_triggerName.c_str();
#else
		return "Trigger Unknown";
#endif
	}

	void GetMemoryUsage( ICrySizer *pSizer ) const
	{
#if defined(MATERIAL_EFFECTS_DEBUG)
		pSizer->Add(m_triggerName);
#endif
	}

private:
	
#if defined(MATERIAL_EFFECTS_DEBUG)
	string          m_triggerName;
#endif
	TAudioControlID m_triggerID;
};

struct SAudioSwitchWrapper
{
	SAudioSwitchWrapper()
		: m_switchID(INVALID_AUDIO_CONTROL_ID) 
		, m_switchStateID(INVALID_AUDIO_SWITCH_STATE_ID)
	{
	}

	void Init(const char* switchName, const char* switchStateName);

	TAudioControlID GetSwitchId() const 
	{ 
		return m_switchID;
	} 

	TAudioSwitchStateID GetSwitchStateId() const 
	{ 
		return m_switchStateID;
	} 

	bool IsValid() const
	{
		return (m_switchID != INVALID_AUDIO_CONTROL_ID) && (m_switchStateID != INVALID_AUDIO_SWITCH_STATE_ID);
	}

	const char* GetSwitchName() const
	{
#if defined(MATERIAL_EFFECTS_DEBUG)
		return m_switchName.c_str();
#else
		return "Switch Unknown";
#endif
	}

	const char* GetSwitchStateName() const
	{
#if defined(MATERIAL_EFFECTS_DEBUG)
		return m_switchStateName.c_str();
#else
		return "Switch State Unknown";
#endif
	}

	void GetMemoryUsage( ICrySizer *pSizer ) const
	{
#if defined(MATERIAL_EFFECTS_DEBUG)
		pSizer->Add(m_switchName);
		pSizer->Add(m_switchStateName);
#endif
	}

private:
#if defined(MATERIAL_EFFECTS_DEBUG)
	string               m_switchName;
	string               m_switchStateName;
#endif
	TAudioControlID      m_switchID;
	TAudioSwitchStateID  m_switchStateID;
};

//////////////////////////////////////////////////////////////////////////

struct SMFXAudioEffectParams 
{
	typedef std::vector<SAudioSwitchWrapper> TSwitches;

	void GetMemoryUsage( ICrySizer *pSizer ) const
	{
		pSizer->AddObject(&trigger);
		pSizer->Add(triggerSwitches);
	}

	SAudioTriggerWrapper trigger;
	TSwitches             triggerSwitches;
};

class CMFXAudioEffect :
	public CMFXEffectBase
{
public:
	CMFXAudioEffect();

	// IMFXEffect
	virtual void Execute( const SMFXRunTimeEffectParams& params ) override;
	virtual void LoadParamsFromXml( const XmlNodeRef& paramsNode ) override;
	virtual void GetResources( SMFXResourceList& resourceList ) const override;
	virtual void GetMemoryUsage( ICrySizer *pSizer ) const override;
	//~IMFXEffect
	
protected:

	SMFXAudioEffectParams m_audioParams;
};

#endif // _MFX_AUDIO_H_


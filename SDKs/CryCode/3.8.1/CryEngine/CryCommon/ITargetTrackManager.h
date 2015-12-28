/*************************************************************************
  Crytek Source File.
  Copyright (C), Crytek Studios, 2001-2009.
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Description: Interface for the AI target track manager
  
 -------------------------------------------------------------------------
  History:
  - 02:25:2010: Created by Kevin Kirst

*************************************************************************/

#ifndef __ITARGET_TRACK_MANAGER_H__
#define __ITARGET_TRACK_MANAGER_H__

#include <IAISystem.h> // <> required for Interfuscator
#include <IAgent.h> // <> required for Interfuscator

struct AgentParameters;
struct ITargetTrack;
struct ITargetTrackManager;

namespace TargetTrackHelpers
{
	// AIEvent stimulus type helper
	enum EAIEventStimulusType
	{
		eEST_Generic,
		eEST_Visual,
		eEST_Sound,
		eEST_BulletRain,
	};

	// Custom stimulus event info
	struct SStimulusEvent
	{
		Vec3 vPos;
		EAITargetThreat eTargetThreat;
		EAIEventStimulusType eStimulusType;

		//////////////////////////////////////////////////////////////////////////
		SStimulusEvent()
		: vPos(ZERO)
		, eTargetThreat(AITHREAT_AGGRESSIVE)
		, eStimulusType(eEST_Generic)
		{ }
	};
}

//! Custom threat modifier helper for game-side specific logic overriding
struct ITargetTrackThreatModifier
{
	// <interfuscator:shuffle>
	virtual ~ITargetTrackThreatModifier() {}

	//////////////////////////////////////////////////////////////////////////
	// ModifyTargetThreat
	//
	// Purpose: Determines which threat value the agent should use for this target
	//
	// In:
	//	ownerAI - the AI agent who is owning this target
	//	targetAI - the AI agent who is the perceived target for the owner
	//	track - The Target Track which contains the information about this target
	//
	// Out:
	//	outThreatRatio - The updated threat ratio (to be used as [0,1]), which 
	//		is stored for you and is for you to use with determining how 
	//		threatening the target is
	//	outThreat - The threat value to be applied to this target now
	//////////////////////////////////////////////////////////////////////////
	virtual void ModifyTargetThreat(IAIObject &ownerAI, IAIObject &targetAI, const ITargetTrack &track, float &outThreatRatio, EAITargetThreat &outThreat) const = 0;
	// </interfuscator:shuffle>
};

struct ITargetTrack
{
	// <interfuscator:shuffle>
	virtual ~ITargetTrack() {}

	virtual const Vec3& GetTargetPos() const = 0;
	virtual const Vec3& GetTargetDir() const = 0;
	virtual float GetTrackValue() const = 0;
	virtual float GetFirstVisualTime() const = 0;
	virtual EAITargetType GetTargetType() const = 0;
	virtual EAITargetThreat GetTargetThreat() const = 0;

	virtual float GetHighestEnvelopeValue() const = 0;
	virtual float GetUpdateInterval() const = 0;
	// </interfuscator:shuffle>
};

struct ITargetTrackManager
{
	// <interfuscator:shuffle>
	virtual ~ITargetTrackManager() {}

	// Threat modifier
	virtual void SetTargetTrackThreatModifier(ITargetTrackThreatModifier *pModifier) = 0;
	virtual void ClearTargetTrackThreatModifier() = 0;

	// Target class mods
	virtual bool SetTargetClassThreat(tAIObjectID aiObjectId, float fClassThreat) = 0;
	virtual float GetTargetClassThreat(tAIObjectID aiObjectId) const = 0;

	// Incoming stimulus handling
	virtual bool HandleStimulusEventInRange(tAIObjectID aiTargetId, const char* szStimulusName, const TargetTrackHelpers::SStimulusEvent &eventInfo, float fRadius) = 0;
	virtual bool HandleStimulusEventForAgent(tAIObjectID aiAgentId, tAIObjectID aiTargetId, const char* szStimulusName, const TargetTrackHelpers::SStimulusEvent &eventInfo) = 0;
	virtual bool TriggerPulse(tAIObjectID aiObjectId, tAIObjectID targetId, const char* szStimulusName, const char* szPulseName) = 0;
	// </interfuscator:shuffle>
};

#endif //__ITARGET_TRACK_MANAGER_H__

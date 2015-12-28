// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.


#ifndef __CommunicationHandler_h__
#define __CommunicationHandler_h__


#pragma once


#include <IAgent.h>


class CommunicationHandler
	: public IAICommunicationHandler
	, public IAnimationGraphStateListener
{
	friend class CommunicationVoiceTestManager;
public:
	CommunicationHandler(CAIProxy& proxy, IEntity* entity);
	virtual ~CommunicationHandler();

	// IAICommunicationHandler
	virtual SCommunicationSound PlaySound(CommPlayID playID, const char* name, IEventListener* listener = 0) OVERRIDE;
	virtual void StopSound(const SCommunicationSound& soundToStop) OVERRIDE;

	virtual SCommunicationSound PlayVoice(CommPlayID playID, const char* variationName, ELipSyncMethod lipSyncMethod, IEventListener* listener = 0) OVERRIDE;
	virtual void StopVoice(const SCommunicationSound& voiceToStop) OVERRIDE;

	virtual void SendDialogueRequest(CommPlayID playID, const char* name, IEventListener* listener = 0) OVERRIDE{}

	virtual void PlayAnimation(CommPlayID playID, const char* name, EAnimationMethod method, IEventListener* listener = 0) OVERRIDE;
	virtual void StopAnimation(CommPlayID playID, const char* name, EAnimationMethod method) OVERRIDE;

	virtual bool IsInAGState(const char* name) OVERRIDE;
	virtual void ResetAnimationState() OVERRIDE;

	virtual bool IsPlayingAnimation() const OVERRIDE;
	virtual bool IsPlayingSound() const OVERRIDE;

	virtual void OnSoundTriggerFinishedToPlay(const TAudioControlID nTriggerID) OVERRIDE;
	//~IAICommunicationHandler

	// IAnimationgGraphStateListener
	virtual void SetOutput(const char* output, const char* value) OVERRIDE {};
	virtual void QueryComplete( TAnimationGraphQueryID queryID, bool succeeded ) OVERRIDE;
	virtual void DestroyedState(IAnimationGraphState*) OVERRIDE;
	//~IAnimationgGraphStateListener

	static void TriggerFinishedCallback(TAudioObjectID const nObjectID, TAudioControlID const nTriggerID, void* const pCookie);

	void Reset();
	void OnReused(IEntity* entity);

	static void OnAudioEvent(SAudioRequestInfo const* const pAudioRequestInfo);

private:
	enum ESoundType
	{
		Sound = 0,
		Voice,
	};

	SCommunicationSound PlaySound(CommPlayID playID, const char* name, ESoundType type, ELipSyncMethod lipSyncMethod, IEventListener* listener = 0);
	IAnimationGraphState* GetAGState();

	CAIProxy& m_proxy;
	EntityId m_entityId;
	IAnimationGraphState* m_agState;

	struct PlayingSound
	{
		PlayingSound()
			: listener(0)
			, type(Sound)
			, playID(0)
			, correspondingStopControlId(INVALID_AUDIO_CONTROL_ID)
		{
		}

		ESoundType type;
		IAICommunicationHandler::IEventListener* listener;
		//Index used to reference this event in listener. Set when sound event started
		CommPlayID playID;
		TAudioControlID correspondingStopControlId;
	};

	struct PlayingAnimation
	{
		PlayingAnimation()
			: listener(0)
			, playing(false)
			, playID(0)
		{
		}

		IAICommunicationHandler::IEventListener* listener;
		string name;

		EAnimationMethod method;
		bool playing;
		//Index used to reference this event in listener. Set when animation event started
		CommPlayID playID;
	};

	typedef std::map<TAnimationGraphQueryID, PlayingAnimation> PlayingAnimations;
	PlayingAnimations m_playingAnimations;
	TAnimationGraphQueryID m_currentQueryID;	// because animation graph can send query result during SetInput, 
	bool m_currentPlaying;										// before we had chance to insert in the map

	typedef std::map<TAudioControlID, PlayingSound> PlayingSounds;
	PlayingSounds m_playingSounds;

	AnimationGraphInputID m_signalInputID;
	AnimationGraphInputID m_actionInputID;
};

#endif
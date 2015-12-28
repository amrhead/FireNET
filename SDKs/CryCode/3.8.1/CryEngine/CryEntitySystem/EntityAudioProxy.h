// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef ENTITY_AUDIO_PROXY_H_INCLUDED
#define ENTITY_AUDIO_PROXY_H_INCLUDED

#include <IAudioSystem.h>
#include <STLPoolAllocator.h>

//////////////////////////////////////////////////////////////////////////
// Description:
//    Handles audio on the entity.
//////////////////////////////////////////////////////////////////////////
struct CEntityAudioProxy : public IEntityAudioProxy
{
public:

	CEntityAudioProxy();
	virtual ~CEntityAudioProxy();

	// IComponent
	virtual void											ProcessEvent(SEntityEvent& event);
	virtual void											Initialize(SComponentInitializer const& init);
	// ~IComponent

	// IEntityProxy
	virtual EEntityProxy							GetType() {return ENTITY_PROXY_AUDIO;}
	virtual void											Release();
	virtual void											Done();
	virtual void											Update(SEntityUpdateContext& ctx) {}
	virtual bool											Init(IEntity* pEntity, SEntitySpawnParams& params ) {return true;}
	virtual void											Reload(IEntity* pEntity, SEntitySpawnParams& params);
	virtual void											SerializeXML(XmlNodeRef &entityNode,bool bLoading) {}
	virtual void											Serialize(TSerialize ser);
	virtual bool											NeedSerialize() {return false;}
	virtual bool											GetSignature(TSerialize signature);
	virtual void											GetMemoryUsage(ICrySizer* pSizer) const {pSizer->AddObject(this, sizeof(*this));}
	// ~IEntityProxy

	// IEntityAudioProxy
	virtual void											SetFadeDistance(float const fFadeDistance) {m_fFadeDistance = fFadeDistance;}
	virtual float											GetFadeDistance() const {return m_fFadeDistance;}
	virtual void											SetEnvironmentFadeDistance(float const fEnvironmentFadeDistance) {m_fEnvironmentFadeDistance = fEnvironmentFadeDistance;}
	virtual float											GetEnvironmentFadeDistance() const {return m_fEnvironmentFadeDistance;}
	virtual void											SetEnvironmentID(TAudioEnvironmentID const nEnvironmentID) {m_nAudioEnvironmentID = nEnvironmentID;}
	virtual TAudioEnvironmentID				GetEnvironmentID() const {return m_nAudioEnvironmentID;}
	virtual TAudioProxyID							CreateAuxAudioProxy();
	virtual bool											RemoveAuxAudioProxy(TAudioProxyID const nAudioProxyLocalID);
	virtual void											SetAuxAudioProxyOffset(SATLWorldPosition const& rOffset, TAudioProxyID const nAudioProxyLocalID = DEFAULT_AUDIO_PROXY_ID);
	virtual SATLWorldPosition const&	GetAuxAudioProxyOffset(TAudioProxyID const nAudioProxyLocalID = DEFAULT_AUDIO_PROXY_ID);
	virtual bool											ExecuteTrigger(TAudioControlID const nTriggerID, ELipSyncMethod const eLipSyncMethod, TAudioProxyID const nAudioProxyLocalID = DEFAULT_AUDIO_PROXY_ID, SAudioCallBackInfos const& rCallBackInfos = SAudioCallBackInfos::GetEmptyObject());
	virtual void											StopTrigger(TAudioControlID const nTriggerID, TAudioProxyID const nAudioProxyLocalID = DEFAULT_AUDIO_PROXY_ID);
	virtual void											SetSwitchState(TAudioControlID const nSwitchID, TAudioSwitchStateID const nStateID, TAudioProxyID const nAudioProxyLocalID = DEFAULT_AUDIO_PROXY_ID);
	virtual void											SetRtpcValue(TAudioControlID const nRtpcID, float const fValue, TAudioProxyID const nAudioProxyLocalID = DEFAULT_AUDIO_PROXY_ID);
	virtual void											SetObstructionCalcType(EAudioObjectObstructionCalcType const eObstructionType, TAudioProxyID const nAudioProxyLocalID = DEFAULT_AUDIO_PROXY_ID);
	virtual void											SetEnvironmentAmount(TAudioEnvironmentID const nEnvironmentID, float const fAmount, TAudioProxyID const nAudioProxyLocalID = DEFAULT_AUDIO_PROXY_ID);
	virtual void											SetCurrentEnvironments(TAudioProxyID const nAudioProxyLocalID = DEFAULT_AUDIO_PROXY_ID);
	virtual void											AuxAudioProxiesMoveWithEntity(bool const bCanMoveWithEntity);
	virtual void											AddAsListenerToAuxAudioProxy(TAudioProxyID const nAudioProxyLocalID, void (*func)(SAudioRequestInfo const* const), EAudioRequestType requestType = eART_AUDIO_ALL_REQUESTS, TATLEnumFlagsType specificRequestMask = ALL_AUDIO_REQUEST_SPECIFIC_TYPE_FLAGS);
	virtual void											RemoveAsListenerFromAuxAudioProxy(TAudioProxyID const nAudioProxyLocalID, void (*func)(SAudioRequestInfo const* const));
	// ~IEntityAudioProxy

private:

	enum EEntityAudioProxyFlags ATL_ENUM_TYPE
	{
		eEAPF_NONE									= 0,
		eEAPF_HIDDEN								= BIT(0),
		eEAPF_CAN_MOVE_WITH_ENTITY	= BIT(1),
	};

	struct SAudioProxyWrapper
	{
		SAudioProxyWrapper(IAudioProxy* const pPassedIAudioProxy)
			:	pIAudioProxy(pPassedIAudioProxy)
		{}

		~SAudioProxyWrapper() {}

		IAudioProxy* const	pIAudioProxy;
		SATLWorldPosition		oOffset;

	private:

		SAudioProxyWrapper()
		:	pIAudioProxy(NULL)
		{}
	};

	typedef std::pair<TAudioProxyID const, SAudioProxyWrapper> TAudioProxyPair;
	typedef std::map<TAudioProxyID const, SAudioProxyWrapper, std::less<TAudioProxyID>, stl::STLPoolAllocator<TAudioProxyPair > >	TAuxAudioProxies;
	static TAudioProxyPair s_oNULLAudioProxyPair;
	static SATLWorldPosition const s_oNULLOffset;

	void							OnListenerEnter(IEntity const* const pEntity);
	void							OnListenerMoveNear(IEntity const* const __restrict pEntity, IEntity const* const __restrict pArea);
	void							OnListenerMoveInside(IEntity const* const pEntity);
	void							OnListenerExclusiveMoveInside(IEntity const* const __restrict pEntity, IEntity const* const __restrict pAreaHigh, IEntity const* const __restrict pAreaLow, float const fFade);
	void							OnMove();
	void							OnEnter(IEntity const* const pEntity);
	void							OnLeaveNear(IEntity const* const pEntity);
	void							OnMoveNear(IEntity const* const __restrict pEntity, IEntity const* const __restrict pArea);
	void							OnMoveInside(IEntity const* const pEntity);
	void							OnExclusiveMoveInside(IEntity const* const __restrict pEntity, IEntity const* const __restrict pEntityAreaHigh, IEntity const* const __restrict pEntityAreaLow, float const fFade);
	void							OnAreaCrossing();
	TAudioProxyPair&	GetAuxAudioProxyPair(TAudioProxyID const nAudioProxyLocalID);
	void							SetEnvironmentAmountInternal(IEntity const* const pEntity, float const fAmount) const;
	void							UpdateHideStatus();

	// Function objects
	struct SReleaseAudioProxy
	{
		SReleaseAudioProxy() {}

		inline void operator()(TAudioProxyPair const& rPair)
		{
			rPair.second.pIAudioProxy->Release();
		}
	};

	struct SResetAudioProxy
	{
		SResetAudioProxy() {}

		inline void operator()(TAudioProxyPair const& rPair)
		{
			rPair.second.pIAudioProxy->Reset();
		}
	};

	struct SInitializeAudioProxy
	{
		SInitializeAudioProxy(char const* const sPassedObjectName)
			:	sObjectName(sPassedObjectName)
		{}

		inline void operator()(TAudioProxyPair const& rPair)
		{
			rPair.second.pIAudioProxy->Initialize(sObjectName);
		}

	private:

		char const* const sObjectName;
	};

	struct SRepositionAudioProxy
	{
		SRepositionAudioProxy(SATLWorldPosition const& rPassedPosition)
			:	rPosition(rPassedPosition)
		{}

		inline void operator()(TAudioProxyPair const& rPair)
		{
			rPair.second.pIAudioProxy->SetPosition(SATLWorldPosition(rPosition.mPosition * rPair.second.oOffset.mPosition));
		}

	private:

		SATLWorldPosition const& rPosition;
	};

	struct SStopTrigger
	{
		SStopTrigger(TAudioControlID const nPassedID)
			:	nTriggerID(nPassedID)
		{}

		inline void operator()(TAudioProxyPair const& rPair)
		{
			rPair.second.pIAudioProxy->StopTrigger(nTriggerID);
		}

	private:
		
		TAudioControlID const nTriggerID;
	};

	struct SSetSwitchState
	{
		SSetSwitchState(TAudioControlID const nPassedSwitchID, TAudioSwitchStateID const nPassedStateID)
			:	nSwitchID(nPassedSwitchID)
			,	nStateID(nPassedStateID)
		{}

		inline void operator()(TAudioProxyPair const& rPair)
		{
			rPair.second.pIAudioProxy->SetSwitchState(nSwitchID, nStateID);
		}

	private:

		TAudioControlID const nSwitchID;
		TAudioSwitchStateID const nStateID;
	};

	struct SSetRtpcValue
	{
		SSetRtpcValue(TAudioControlID const nPassedRtpcID, float const fPassedValue)
			:	nRtpcID(nPassedRtpcID)
			,	fValue(fPassedValue)
		{}

		inline void operator()(TAudioProxyPair const& rPair)
		{
			rPair.second.pIAudioProxy->SetRtpcValue(nRtpcID, fValue);
		}

	private:

		TAudioControlID const nRtpcID;
		float const fValue;
	};

	struct SSetObstructionCalcType
	{
		SSetObstructionCalcType(EAudioObjectObstructionCalcType const ePassedObstructionType)
			:	eObstructionType(ePassedObstructionType)
		{}

		inline void operator()(TAudioProxyPair const& rPair)
		{
			rPair.second.pIAudioProxy->SetObstructionCalcType(eObstructionType);
		}

	private:

		EAudioObjectObstructionCalcType const eObstructionType;
	};

	struct SSetEnvironmentAmount
	{
		SSetEnvironmentAmount(TAudioEnvironmentID const nPassedEnvironmentID, float const fPassedAmount)
			:	nEnvironmentID(nPassedEnvironmentID)
			,	fAmount(fPassedAmount)
		{}

		inline void operator()(TAudioProxyPair const& rPair)
		{
			rPair.second.pIAudioProxy->SetEnvironmentAmount(nEnvironmentID, fAmount);
		}

	private:

		TAudioEnvironmentID const nEnvironmentID;
		float const fAmount;
	};

	struct SSetCurrentEnvironments
	{
		SSetCurrentEnvironments(EntityId const nEntityID)
		:	m_nEntityID(nEntityID)
		{}

		inline void operator()(TAudioProxyPair const& rPair)
		{
			rPair.second.pIAudioProxy->SetCurrentEnvironments(m_nEntityID);
		}

	private:

		EntityId const m_nEntityID;
	};

	struct SSetAuxAudioProxyOffset
	{
		SSetAuxAudioProxyOffset(SATLWorldPosition const& rPassedOffset, Matrix34 const& rPassedEntityPosition)
		:	rOffset(rPassedOffset)
		,	rEntityPosition(rPassedEntityPosition)
		{}

		inline void operator()(TAudioProxyPair& rPair)
		{
			rPair.second.oOffset = rOffset;
			rPair.second.oOffset.NormalizeForwardVec();
			SATLWorldPosition const oPosition(rEntityPosition);
			(SRepositionAudioProxy(oPosition))(rPair);
		}

	private:

		SATLWorldPosition const& rOffset;
		Matrix34 const& rEntityPosition;
	};
	// ~Function objects

	TAuxAudioProxies		m_mapAuxAudioProxies;
	TAudioProxyID				m_nAudioProxyIDCounter;

	TAudioEnvironmentID	m_nAudioEnvironmentID;
	CEntity*						m_pEntity;
	TATLEnumFlagsType		m_nFlags;

	float								m_fFadeDistance;
	float								m_fEnvironmentFadeDistance;
};

#endif // ENTITY_AUDIO_PROXY_H_INCLUDED

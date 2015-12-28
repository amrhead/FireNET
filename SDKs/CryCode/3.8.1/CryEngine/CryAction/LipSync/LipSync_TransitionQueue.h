////////////////////////////////////////////////////////////////////////////
//
//  CryEngine Source File.
//  Copyright (C), Crytek, 1999-2014.
// -------------------------------------------------------------------------
//  File name:   LipSync_TransitionQueue.h
//  Version:     v1.00
//  Created:     2014-08-29 by Christian Werle.
//  Description: Automatic start of facial animation when a sound is being played back.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#pragma once


/// The CLipSyncProvider_TransitionQueue communicates with sound proxy to play synchronized facial animation directly on the transition queue
class CLipSyncProvider_TransitionQueue : public ILipSyncProvider
{
public:
	explicit CLipSyncProvider_TransitionQueue(EntityId entityId);

	// ILipSyncProvider
	void RequestLipSync(IEntityAudioProxy* pProxy, const TAudioControlID nAudioTriggerId, const ELipSyncMethod lipSyncMethod) override;
	void StartLipSync(IEntityAudioProxy* pProxy, const TAudioControlID nAudioTriggerId, const ELipSyncMethod lipSyncMethod) override;
	void PauseLipSync(IEntityAudioProxy* pProxy, const TAudioControlID nAudioTriggerId, const ELipSyncMethod lipSyncMethod) override;
	void UnpauseLipSync(IEntityAudioProxy* pProxy, const TAudioControlID nAudioTriggerId, const ELipSyncMethod lipSyncMethod) override;
	void StopLipSync(IEntityAudioProxy* pProxy, const TAudioControlID nAudioTriggerId, const ELipSyncMethod lipSyncMethod) override;
	void UpdateLipSync(IEntityAudioProxy* pProxy, const TAudioControlID nAudioTriggerId, const ELipSyncMethod lipSyncMethod) override;
	// ~ILipSyncProvider

	void FullSerialize(TSerialize ser);
	void GetEntityPoolSignature(TSerialize signature);

private:
	IEntity* GetEntity();
	ICharacterInstance* GetCharacterInstance();
	void FindMatchingAnim(const TAudioControlID nAudioTriggerId, const ELipSyncMethod lipSyncMethod, ICharacterInstance& character, int* pAnimIdOut, CryCharAnimationParams* pAnimParamsOut) const;
	void FillCharAnimationParams(const bool isDefaultAnim, CryCharAnimationParams* pParams) const;
	void SynchronizeAnimationToSound(const TAudioControlID nAudioTriggerId);

private:
	enum EState
	{
		eS_Init,
		eS_Requested,
		eS_Started,
		eS_Paused,
		eS_Unpaused,
		eS_Stopped,
	};

	static uint32 s_lastAnimationToken;

	EntityId m_entityId;
	int m_nCharacterSlot;           // the lip-sync animation will be played back on the character residing in this slot of the entity
	int m_nAnimLayer;               // the lip-sync animation will be played back on this layer of the character
	string m_sDefaultAnimName;      // fallback animation to play if there is no animation matching the currently playing sound in the character's database

	EState m_state;
	bool m_isSynchronized;

	// Filled when request comes in:
	int m_requestedAnimId;
	CryCharAnimationParams m_requestedAnimParams;
	CAutoResourceCache_CAF m_cachedAnim;

	// Filled when animation is started:
	uint32 m_nCurrentAnimationToken;
	TAudioControlID m_soundId;
};
DECLARE_BOOST_POINTERS(CLipSyncProvider_TransitionQueue);


class CLipSync_TransitionQueue : public CGameObjectExtensionHelper<CLipSync_TransitionQueue, IGameObjectExtension>
{
public:
	// IGameObjectExtension
	virtual void GetMemoryUsage(ICrySizer *pSizer) const override;
	virtual bool Init(IGameObject* pGameObject) override;
	virtual void PostInit(IGameObject* pGameObject) override;
	virtual void InitClient(int channelId) override;
	virtual void PostInitClient(int channelId) override;
	virtual bool ReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params) override;
	virtual void PostReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params) override;
	virtual bool GetEntityPoolSignature(TSerialize signature) override;
	virtual void Release() override;
	virtual void FullSerialize(TSerialize ser) override;
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int pflags) override;
	virtual void PostSerialize() override;
	virtual void SerializeSpawnInfo(TSerialize ser) override;
	virtual ISerializableInfoPtr GetSpawnInfo() override;
	virtual void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	virtual void HandleEvent(const SGameObjectEvent& event) override;
	virtual void ProcessEvent(SEntityEvent& event) override;
	virtual void SetChannelId(uint16 id) override;
	virtual void SetAuthority(bool auth) override;
	virtual void PostUpdate(float frameTime) override;
	virtual void PostRemoteSpawn() override;
	// ~IGameObjectExtension

private:
	void InjectLipSyncProvider();

private:
	CLipSyncProvider_TransitionQueuePtr m_pLipSyncProvider;
};

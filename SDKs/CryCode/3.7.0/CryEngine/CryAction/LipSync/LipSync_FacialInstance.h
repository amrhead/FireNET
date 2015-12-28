////////////////////////////////////////////////////////////////////////////
//
//  CryEngine Source File.
//  Copyright (C), Crytek, 1999-2014.
// -------------------------------------------------------------------------
//  File name:   LipSync_FacialInstance.h
//  Version:     v1.00
//  Created:     2014-08-29 by Christian Werle.
//  Description: Automatic start of facial animation when a sound is being played back.
//               Legacy version that uses CryAnimation's FacialInstance.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#pragma once


class CLipSyncProvider_FacialInstance : public ILipSyncProvider
{
public:
	explicit CLipSyncProvider_FacialInstance(EntityId entityId);

	// ILipSyncProvider
	virtual void RequestLipSync(IEntityAudioProxy* pProxy, const TAudioControlID nAudioTriggerId, const ELipSyncMethod lipSyncMethod) OVERRIDE;
	virtual void StartLipSync(IEntityAudioProxy* pProxy, const TAudioControlID nAudioTriggerId, const ELipSyncMethod lipSyncMethod) OVERRIDE;
	virtual void PauseLipSync(IEntityAudioProxy* pProxy, const TAudioControlID nAudioTriggerId, const ELipSyncMethod lipSyncMethod) OVERRIDE;
	virtual void UnpauseLipSync(IEntityAudioProxy* pProxy, const TAudioControlID nAudioTriggerId, const ELipSyncMethod lipSyncMethod) OVERRIDE;
	virtual void StopLipSync(IEntityAudioProxy* pProxy, const TAudioControlID nAudioTriggerId, const ELipSyncMethod lipSyncMethod) OVERRIDE;
	virtual void UpdateLipSync(IEntityAudioProxy* pProxy, const TAudioControlID nAudioTriggerId, const ELipSyncMethod lipSyncMethod) OVERRIDE;
	// ~ILipSyncProvider

	void FullSerialize(TSerialize ser);
	void GetEntityPoolSignature(TSerialize signature);

private:
	void LipSyncWithSound(const TAudioControlID nAudioTriggerId, bool bStop = false);
	EntityId m_entityId;
};
DECLARE_BOOST_POINTERS(CLipSyncProvider_FacialInstance);


class CLipSync_FacialInstance : public CGameObjectExtensionHelper<CLipSync_FacialInstance, IGameObjectExtension>
{
public:
	// IGameObjectExtension
	virtual void GetMemoryUsage(ICrySizer *pSizer) const OVERRIDE;
	virtual bool Init(IGameObject* pGameObject) OVERRIDE;
	virtual void PostInit(IGameObject* pGameObject) OVERRIDE;
	virtual void InitClient(int channelId) OVERRIDE;
	virtual void PostInitClient(int channelId) OVERRIDE;
	virtual bool ReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params) OVERRIDE;
	virtual void PostReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params) OVERRIDE;
	virtual bool GetEntityPoolSignature(TSerialize signature) OVERRIDE;
	virtual void Release() OVERRIDE;
	virtual void FullSerialize(TSerialize ser) OVERRIDE;
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int pflags) OVERRIDE;
	virtual void PostSerialize() OVERRIDE;
	virtual void SerializeSpawnInfo(TSerialize ser) OVERRIDE;
	virtual ISerializableInfoPtr GetSpawnInfo() OVERRIDE;
	virtual void Update(SEntityUpdateContext& ctx, int updateSlot) OVERRIDE;
	virtual void HandleEvent(const SGameObjectEvent& event) OVERRIDE;
	virtual void ProcessEvent(SEntityEvent& event) OVERRIDE;
	virtual void SetChannelId(uint16 id) OVERRIDE;
	virtual void SetAuthority(bool auth) OVERRIDE;
	virtual void PostUpdate(float frameTime) OVERRIDE;
	virtual void PostRemoteSpawn() OVERRIDE;
	// ~IGameObjectExtension

private:
	void InjectLipSyncProvider();

private:
	CLipSyncProvider_FacialInstancePtr m_pLipSyncProvider;
};

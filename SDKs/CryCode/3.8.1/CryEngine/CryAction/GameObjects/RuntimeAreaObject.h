// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef RUNTIMEAREAOBJECT_H_INCLUDED
#define RUNTIMEAREAOBJECT_H_INCLUDED

class CRuntimeAreaObject : public CGameObjectExtensionHelper<CRuntimeAreaObject, IGameObjectExtension>
{
public:

	typedef unsigned int TSurfaceCRC;
	
	struct SAudioControls
	{
		TAudioControlID	nTriggerID;
		TAudioControlID	nRtpcID;

		explicit SAudioControls(
			TAudioControlID nPassedTriggerID = INVALID_AUDIO_CONTROL_ID,
			TAudioControlID nPassedRtpcID = INVALID_AUDIO_CONTROL_ID)
			: nTriggerID(nPassedTriggerID)
			, nRtpcID(nPassedRtpcID)
		{}
	};

	typedef std::map<TSurfaceCRC, SAudioControls> TAudioControlMap;

	static TAudioControlMap	m_cAudioControls;

	CRuntimeAreaObject();
	virtual ~CRuntimeAreaObject() override;

	// IGameObjectExtension
	virtual bool Init(IGameObject * pGameObject) override;
	virtual void InitClient(int channelId) override {}
	virtual void PostInit(IGameObject * pGameObject) override {}
	virtual void PostInitClient(int channelId) override {}
	virtual bool ReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params) override;
	virtual void PostReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params) override {}
	virtual bool GetEntityPoolSignature(TSerialize signature) override;
	virtual void Release() override;
	virtual void FullSerialize(TSerialize ser) override {}
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) override;
	virtual void PostSerialize() override {}
	virtual void SerializeSpawnInfo(TSerialize ser) override {}
	virtual ISerializableInfoPtr GetSpawnInfo() override {return NULL;}
	virtual void Update(SEntityUpdateContext& ctx, int slot) override {}
	virtual void HandleEvent(const SGameObjectEvent& gameObjectEvent) override {}
	virtual void ProcessEvent(SEntityEvent& entityEvent) override;
	virtual void SetChannelId(uint16 id) override {}
	virtual void SetAuthority(bool auth) override {}
	virtual void PostUpdate(float frameTime) override { CRY_ASSERT(false); }
	virtual void PostRemoteSpawn() override {}
	virtual void GetMemoryUsage(ICrySizer *pSizer) const override;
	// ~IGameObjectExtension

private:

	struct SAreaSoundInfo
	{
		SAudioControls	oAudioControls;
		float						fParameter;

		explicit SAreaSoundInfo(SAudioControls const& rPassedAudioControls, float const fPassedParameter = 0.0f)
			: oAudioControls(rPassedAudioControls)
			, fParameter(fPassedParameter)
		{}

		~SAreaSoundInfo()
		{}
	};
	
	typedef std::map<TSurfaceCRC, SAreaSoundInfo>	TAudioParameterMap;
	typedef std::map<EntityId, TAudioParameterMap> TEntitySoundsMap;

	void UpdateParameterValues(IEntity* const pEntity, TAudioParameterMap& rParamMap);
	void StopEntitySounds(EntityId const nEntityID, TAudioParameterMap& rParamMap);

	TEntitySoundsMap m_cActiveEntitySounds;
};

#endif // RUNTIMEAREAOBJECT_H_INCLUDED

/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014.

-------------------------------------------------------------------------
History:

- 03.12.2014   13:49 : Created by AfroStalin(chernecoff)
- 07.12.2014   18:04 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/
#ifndef __NetworkBuilding_H__
#define __NetworkBuilding_H__

class CWorldState;

class CNetworkBuilding :	public CGameObjectExtensionHelper<CNetworkBuilding, IGameObjectExtension>
{
private:
	enum ePhysicalization
	{
		ePhys_NotPhysicalized,
		ePhys_PhysicalizedRigid,
		ePhys_PhysicalizedStatic,
	};

	enum EState
	{
		eState_NotUsed = 0,
		eState_InUse,
		eState_Done
	};
public:
	CNetworkBuilding();
	~CNetworkBuilding();

	// IGameObjectExtension
	virtual void GetMemoryUsage(ICrySizer *pSizer) const{}
	virtual bool Init( IGameObject * pGameObject );
	virtual void PostInit( IGameObject * pGameObject );
	virtual void InitClient(int channelId){}
	virtual void PostInitClient(int channelId){}
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params ){}
	virtual bool GetEntityPoolSignature( TSerialize signature );
	virtual void Release();
	virtual void FullSerialize( TSerialize ser ){}
	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, uint8 profile, int pflags);
	virtual NetworkAspectType GetNetSerializeAspects() { return eEA_All; }
	virtual void PostSerialize(){}
	virtual void SerializeSpawnInfo( TSerialize ser ){}
	virtual ISerializableInfoPtr GetSpawnInfo(){return 0;}
	virtual void Update( SEntityUpdateContext& ctx, int updateSlot );
	virtual void HandleEvent( const SGameObjectEvent& event ){}
	virtual void ProcessEvent( SEntityEvent& event ){}	
	virtual void SetChannelId(uint16 id){}
	virtual void SetAuthority( bool auth ){}
	virtual void PostUpdate( float frameTime ){}
	virtual void PostRemoteSpawn(){}
	//Script callbacks
	int CanUse(EntityId entityId) const;
	void StartBuild();
	//

private:
	static const NetworkAspectType POSITION_ASPECT = eEA_GameServerStatic;

	bool GetSettings();
	bool Reset();
	bool Building(int part);
	void Physicalize(pe_type phys_type);
	void ReadWorldState();

	EState	m_state;

	int phys_type;
	float phys_mass;
	float phys_density;

	const char* default_model;
	const char* model_1;
	const char* model_2;
	const char* model_3;
	const char* finish_model;
	//
	const char* Model_1_mat;
	const char* Model_2_mat;
	const char* Model_3_mat;
	const char* finishMat;
	//
	const char* entity_name;
	//
	float fStartTime;
	float build_time;
	//
	int build_status;
	//
	Vec3 cur_pos;
	Quat cur_rot;
	//
	IMaterial *defMat;
	//
	CWorldState *ws;
};

#endif

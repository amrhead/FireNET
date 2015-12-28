// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.


#ifndef __GAMEOBJECTSYSTEM_H__
#define __GAMEOBJECTSYSTEM_H__

#pragma once

// FIXME: Cell SDK GCC bug workaround.
#ifndef __IGAMEOBJECTSYSTEM_H__
#include "IGameObjectSystem.h"
#endif

#include "GameObjectDispatch.h"
#include <vector>
#include <map>

struct SEntitySchedulingProfiles
{
	uint32 normal;
	uint32 owned;

	void GetMemoryUsage(ICrySizer *pSizer) const {/*nothing*/}
};

class CGameObjectSystem : public IGameObjectSystem
{
public:
	bool Init();

	IGameObjectSystem::ExtensionID GetID( const char * name );
	const char * GetName( IGameObjectSystem::ExtensionID id );
	uint32 GetExtensionSerializationPriority( IGameObjectSystem::ExtensionID id );
	IGameObjectExtensionPtr Instantiate( IGameObjectSystem::ExtensionID id, IGameObject * pObject );
	VIRTUAL void RegisterExtension( const char * name, IGameObjectExtensionCreatorBase * pCreator, IEntityClassRegistry::SEntityClassDesc * pClsDesc );
	VIRTUAL void DefineProtocol( bool server, IProtocolBuilder * pBuilder );
	VIRTUAL void BroadcastEvent( const SGameObjectEvent& evt );

	VIRTUAL void RegisterEvent( uint32 id, const char* name );
	VIRTUAL uint32 GetEventID( const char* name );
	VIRTUAL const char* GetEventName( uint32 id );

	VIRTUAL IGameObject *CreateGameObjectForEntity(EntityId entityId);
	VIRTUAL IEntityProxyPtr CreateGameObjectEntityProxy(IEntity& entity, IGameObject** pGameObject = NULL) OVERRIDE;

	VIRTUAL void PostUpdate( float frameTime );
	VIRTUAL void SetPostUpdate( IGameObject * pGameObject, bool enable );

	VIRTUAL void Reset();

	const SEntitySchedulingProfiles * GetEntitySchedulerProfiles( IEntity * pEnt );

	void RegisterFactories( IGameFramework * pFW );

	IEntity * CreatePlayerProximityTrigger();
	ILINE IEntityClass * GetPlayerProximityTriggerClass() { return m_pClassPlayerProximityTrigger; }
	ILINE std::vector<IGameObjectSystem::ExtensionID> *GetActivatedExtensionsTop() { return &m_activatedExtensions_top; }

	VIRTUAL void SetSpawnSerializerForEntity( const EntityId entityId, TSerialize* pSerializer );
	VIRTUAL void ClearSpawnSerializerForEntity( const EntityId entityId );
	
	void GetMemoryUsage(ICrySizer * s) const;

  VIRTUAL void AddSink( IGameObjectSystemSink *pSink );
  VIRTUAL void RemoveSink( IGameObjectSystemSink *pSink );
private:
	void LoadSerializationOrderFile();
	TSerialize* GetSpawnSerializerForEntity( const EntityId entityId ) const;

	std::map<string, ExtensionID> m_nameToID;

	struct SExtensionInfo
	{
		string name;
		uint32 serializationPriority; // lower values is higher priority
		IGameObjectExtensionCreatorBase * pFactory;

		void GetMemoryUsage( ICrySizer *pSizer ) const
		{
			pSizer->AddObject(name);
		}
	};
	std::vector<SExtensionInfo> m_extensionInfo;

	static IEntityProxyPtr CreateGameObjectWithPreactivatedExtension( 
		IEntity * pEntity, SEntitySpawnParams &params, void * pUserData );

	CGameObjectDispatch m_dispatch;

	std::vector<IGameObject*> m_postUpdateObjects;
	IEntityClass * m_pClassPlayerProximityTrigger;
	
	struct SSpawnSerializer
	{
		SSpawnSerializer( const EntityId _entityId, TSerialize* _pSerializer )
			: entityId(_entityId)
			, pSerializer(_pSerializer)
		{
		}

		bool operator== (const SSpawnSerializer& otherSerializer) const
		{
			return (entityId == otherSerializer.entityId);
		};

		bool operator== (const EntityId& otherEntityId) const
		{
			return (entityId == otherEntityId);
		}

		EntityId    entityId;
		TSerialize* pSerializer;
	};
	typedef std::vector<SSpawnSerializer> TSpawnSerializers;
	TSpawnSerializers m_spawnSerializers;

	std::map<string, SEntitySchedulingProfiles> m_schedulingParams;
	SEntitySchedulingProfiles m_defaultProfiles;

	// event ID management
	std::map<string, uint32> m_eventNameToID;
	std::map<uint32, string> m_eventIDToName;
  //
  typedef std::list<IGameObjectSystemSink*> SinkList;
  SinkList												m_lstSinks;							// registered sinks get callbacks

	std::vector<IGameObject*> m_tempObjects;

	std::vector<IGameObjectSystem::ExtensionID> m_activatedExtensions_top;
	std::vector<string> m_serializationOrderList;  // defines serialization order for extensions
};

#endif

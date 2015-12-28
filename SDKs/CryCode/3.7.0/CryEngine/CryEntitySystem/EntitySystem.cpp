//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:CEntitySystem.cpp
//  Description: entity's management
//
//	History:
//	-March 07,2001:Originally created by Marco Corbetta
//  -: modified by everyone
//
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "EntitySystem.h"
#include "EntityIt.h"

#include "Entity.h"
#include "EntityCVars.h"
#include "EntityClassRegistry.h"
#include "ComponentEventDistributer.h"
#include "ScriptBind_Entity.h"
#include "PhysicsEventListener.h"
#include "AreaManager.h"
#include "AreaProxy.h"
#include "BreakableManager.h"
#include "EntityArchetype.h"
#include "PartitionGrid.h"
#include "ProximityTriggerSystem.h"
#include "EntityObject.h"
#include "ISerialize.h"
#include "EntityLayer.h"
#include "EntityLoadManager.h"
#include "EntityPoolManager.h"
#include "IStatoscope.h"
#include "IBreakableManager.h"
#include "IGame.h"
#include "IGameFramework.h"
#include "GeomCacheAttachmentManager.h"
#include "CharacterBoneAttachmentManager.h"
#include "TypeInfo_impl.h"	// ARRAY_COUNT macro
#include "BSPTree3D.h"

#include <IRenderer.h>
#include <I3DEngine.h>
#include <ILog.h>
#include <ITimer.h>
#include <ISystem.h>
#include <IPhysics.h>
#include <IRenderAuxGeom.h>
#include <ILiveCreateHost.h>
#include <Cry_Camera.h>
#include <IAgent.h>
#include <IAIActorProxy.h>
#include <IResourceManager.h>
#include <IDeferredCollisionEvent.h>
#include <CryProfileMarker.h>
#include <IRemoteCommand.h>

#pragma warning(disable: 6255)  // _alloca indicates failure by raising a stack overflow exception. Consider using _malloca instead. (Note: _malloca requires _freea.)

#ifndef _RELEASE
	#define PROFILE_ENTITYSYSTEM
#endif

stl::PoolAllocatorNoMT<sizeof(CRenderProxy)> *g_Alloc_RenderProxy = 0;
stl::PoolAllocatorNoMT<sizeof(CEntityObject),8> *g_Alloc_EntitySlot = 0;

namespace 
{
  static inline bool LayerActivationPriority(
      const SPostSerializeLayerActivation& a
    , const SPostSerializeLayerActivation& b)
  {
    if (a.enable && !b.enable)
      return true; 
    if (!a.enable && b.enable)
      return false; 
    return false; 
  }
}

//////////////////////////////////////////////////////////////////////////
void OnRemoveEntityCVarChange( ICVar *pArgs )
{		
	if (gEnv->pEntitySystem)
	{		
		const char *szEntity=pArgs->GetString();
		IEntity *pEnt=gEnv->pEntitySystem->FindEntityByName(szEntity);
		if (pEnt)
			gEnv->pEntitySystem->RemoveEntity(pEnt->GetId());
	}
}

//////////////////////////////////////////////////////////////////////////
void OnActivateEntityCVarChange( ICVar *pArgs )
{	
	if (gEnv->pEntitySystem)
	{		
		const char *szEntity=pArgs->GetString();
		IEntity *pEnt=gEnv->pEntitySystem->FindEntityByName(szEntity);
		if (pEnt)
		{
			CEntity *pcEnt=(CEntity *)(pEnt);
			pcEnt->Activate(true);			
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void OnDeactivateEntityCVarChange( ICVar *pArgs )
{
	if (gEnv->pEntitySystem)
	{		
		const char *szEntity=pArgs->GetString();
		IEntity *pEnt=gEnv->pEntitySystem->FindEntityByName(szEntity);
		if (pEnt)
		{
			CEntity *pcEnt=(CEntity *)(pEnt);
			pcEnt->Activate(false);			
		}
	}
}

//////////////////////////////////////////////////////////////////////
SEntityLoadParams::SEntityLoadParams()
: pReuseEntity(NULL)
, bCallInit(true)
, clonedLayerId(-1)
{
	AddRef();
}

//////////////////////////////////////////////////////////////////////
SEntityLoadParams::SEntityLoadParams(CEntity *pReuseEntity, SEntitySpawnParams &resetParams)
: spawnParams(resetParams)
, pReuseEntity(pReuseEntity)
, bCallInit(true)
, clonedLayerId(-1)
{
	assert(pReuseEntity);

	AddRef();
}

//////////////////////////////////////////////////////////////////////
SEntityLoadParams::~SEntityLoadParams()
{
	RemoveRef();
}

//////////////////////////////////////////////////////////////////////
SEntityLoadParams& SEntityLoadParams::operator =(const SEntityLoadParams& other)
{
	if (this != &other)
	{
		spawnParams = other.spawnParams;
		spawnParams.entityNode = other.spawnParams.entityNode;
		pReuseEntity = other.pReuseEntity;
		bCallInit = other.bCallInit;
		clonedLayerId = other.clonedLayerId;

		AddRef();
	}

	return *this;
}

//////////////////////////////////////////////////////////////////////
void SEntityLoadParams::UseClonedEntityNode(const XmlNodeRef sourceEntityNode, XmlNodeRef parentNode)
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, EMemStatContextFlags::MSF_Instance, "SEntityLoadParams Clone Xml");

	// Create clone of entity node to avoid adding reference to host XML
	bool bCloned = false;
	if (sourceEntityNode)
	{
		if (parentNode)
			spawnParams.entityNode = parentNode->newChild(sourceEntityNode->getTag());
		else
			spawnParams.entityNode = gEnv->pSystem->CreateXmlNode(sourceEntityNode->getTag());

		bCloned = CloneXmlNode(sourceEntityNode, spawnParams.entityNode);
	}

	if (bCloned)
	{
		// Use the char* from the cloned node
		spawnParams.sName = spawnParams.entityNode->getAttr("Name");
		spawnParams.sLayerName = spawnParams.entityNode->getAttr("Layer");
	}
	else
	{
		spawnParams.entityNode = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
bool SEntityLoadParams::CloneXmlNode(const XmlNodeRef sourceNode, XmlNodeRef destNode)
{
	assert( sourceNode );
	assert( destNode );

	if (destNode)
	{
		bool bResult = true;

		destNode->setContent(sourceNode->getContent());

		const int attrCount = sourceNode->getNumAttributes();
		for (int attr = 0; attr < attrCount; ++attr)
		{
			const char* key = 0;
			const char* value = 0;
			if (sourceNode->getAttributeByIndex(attr, &key, &value))
			{
				destNode->setAttr(key, value);
			}
		}

		const int childCount = sourceNode->getChildCount();
		for (int child = 0; child < childCount; ++child)
		{
			XmlNodeRef pSourceChild = sourceNode->getChild(child);
			if (pSourceChild)
			{
				XmlNodeRef pChildClone = destNode->newChild(pSourceChild->getTag());
				bResult &= CloneXmlNode(pSourceChild, pChildClone);
			}
		}

		return bResult;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
void SEntityLoadParams::AddRef()
{
	if (spawnParams.pPropertiesTable)
		spawnParams.pPropertiesTable->AddRef();

	if (spawnParams.pPropertiesInstanceTable)
		spawnParams.pPropertiesInstanceTable->AddRef();
}

//////////////////////////////////////////////////////////////////////
void SEntityLoadParams::RemoveRef()
{
	SAFE_RELEASE(spawnParams.pPropertiesTable);
	SAFE_RELEASE(spawnParams.pPropertiesInstanceTable);
}

//////////////////////////////////////////////////////////////////////
CEntitySystem::CEntitySystem(ISystem *pSystem)
:	m_entityTimeoutList(gEnv->pTimer)
{
	CRenderProxy::SetTimeoutList(&m_entityTimeoutList);

	// Assign allocators.
	g_Alloc_RenderProxy = new stl::PoolAllocatorNoMT<sizeof(CRenderProxy)>(stl::FHeap().FreeWhenEmpty(true));
	g_Alloc_EntitySlot = new stl::PoolAllocatorNoMT<sizeof(CEntityObject), 8>(stl::FHeap().FreeWhenEmpty(true));

	m_onEventSinks.reserve(5);
	for (size_t i = 0; i < SinkMaxEventSubscriptionCount; ++ i)
		m_sinks[i].reserve(8);

	ClearEntityArray();

	m_pISystem = pSystem;
	m_pClassRegistry = 0;
	m_pEntityScriptBinding = NULL;

	CVar::Init( gEnv->pConsole );
	
	m_bTimersPause=false;
	m_nStartPause.SetSeconds(-1.0f);

	m_pAreaManager = new CAreaManager(this);
	m_pBreakableManager = new CBreakableManager(this);
	m_pEntityArchetypeManager = new CEntityArchetypeManager;
	m_pEventDistributer = new CComponentEventDistributer( CVar::pUpdateType->GetIVal() );

	m_pEntityLoadManager = new CEntityLoadManager(this);
	m_pEntityPoolManager = new CEntityPoolManager(this);

	m_pPartitionGrid = new CPartitionGrid;
	m_pProximityTriggerSystem = new CProximityTriggerSystem;

#if defined(USE_GEOM_CACHES)
	m_pGeomCacheAttachmentManager = new CGeomCacheAttachmentManager;
#endif
	m_pCharacterBoneAttachmentManager = new CCharacterBoneAttachmentManager;

	m_idForced = 0;

	m_bReseting = false;

#ifdef SW_ENTITY_ID_USE_GUID
	m_bEntitiesUseGUIDs = true;
	m_nGeneratedFromGuid = 2;
#else
	m_bEntitiesUseGUIDs = false;
#endif

	if (gEnv->pConsole != 0)
	{
	REGISTER_STRING_CB("es_removeEntity","",VF_CHEAT, "Removes an entity",OnRemoveEntityCVarChange );
	REGISTER_STRING_CB("es_activateEntity","",VF_CHEAT, "Activates an entity",OnActivateEntityCVarChange );
	REGISTER_STRING_CB("es_deactivateEntity","",VF_CHEAT, "Deactivates an entity",OnDeactivateEntityCVarChange );
}
}

//////////////////////////////////////////////////////////////////////
CEntitySystem::~CEntitySystem()
{
	CRenderProxy::SetTimeoutList(0);

	Unload();

	SAFE_DELETE(m_pClassRegistry);
	
	SAFE_DELETE(m_pCharacterBoneAttachmentManager);
#if defined(USE_GEOM_CACHES)
	SAFE_DELETE(m_pGeomCacheAttachmentManager);
#endif
	SAFE_DELETE(m_pAreaManager);
	SAFE_DELETE(m_pEntityArchetypeManager);
	SAFE_DELETE(m_pEntityScriptBinding);
	SAFE_DELETE(m_pEventDistributer);
	SAFE_DELETE(m_pEntityLoadManager);
	SAFE_DELETE(m_pEntityPoolManager);
	
	SAFE_DELETE(m_pPhysicsEventListener);

	SAFE_DELETE(m_pProximityTriggerSystem);
	SAFE_DELETE(m_pPartitionGrid);
	SAFE_DELETE(m_pBreakableManager);

	SAFE_DELETE(g_Alloc_RenderProxy);
	SAFE_DELETE(g_Alloc_EntitySlot);
	//ShutDown();
}

//////////////////////////////////////////////////////////////////////
bool CEntitySystem::Init(ISystem *pSystem)
{
	if (!pSystem)
		return false;

	m_pISystem=pSystem;
	ClearEntityArray();

	m_pClassRegistry = new CEntityClassRegistry;
	m_pClassRegistry->InitializeDefaultClasses();

	//////////////////////////////////////////////////////////////////////////
	// Initialize entity script bindings.
	m_pEntityScriptBinding = new CScriptBind_Entity( pSystem->GetIScriptSystem(),pSystem, this );

	// Initialize physics events handler.
	if(pSystem->GetIPhysicalWorld())
		m_pPhysicsEventListener = new CPhysicsEventListener(this,pSystem->GetIPhysicalWorld());

	//////////////////////////////////////////////////////////////////////////
	// Should reallocate grid if level size change.
	m_pPartitionGrid->AllocateGrid( 4096,4096 );

	m_entityTimeoutList.Clear();
	m_bLocked = false;

	return true;
}

//////////////////////////////////////////////////////////////////////////
IEntityClassRegistry* CEntitySystem::GetClassRegistry()
{
	return m_pClassRegistry;
}

//////////////////////////////////////////////////////////////////////
void CEntitySystem::Release()
{
	delete this;
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::RegisterPhysicCallbacks()
{
	if (m_pPhysicsEventListener)
		m_pPhysicsEventListener->RegisterPhysicCallbacks();
# if !defined(_RELEASE)
  CPhysicalProxy::EnableValidation();
# endif 
}

void CEntitySystem::UnregisterPhysicCallbacks()
{
	if (m_pPhysicsEventListener)
		m_pPhysicsEventListener->UnregisterPhysicCallbacks();
# if !defined(_RELEASE)
  CPhysicalProxy::DisableValidation();
# endif 
}

//////////////////////////////////////////////////////////////////////
void CEntitySystem::Unload()
{
	if (gEnv->p3DEngine )
	{
		IDeferredPhysicsEventManager *pDeferredPhysicsEventManager = gEnv->p3DEngine->GetDeferredPhysicsEventManager();
		if( pDeferredPhysicsEventManager )
			pDeferredPhysicsEventManager->ClearDeferredEvents();
	}

	Reset();
	
	UnloadAreas();
	
	CEntity::ClearStaticData();

	stl::free_container(m_currentTimers);
	m_pEntityArchetypeManager->Reset();

	stl::free_container(m_tempActiveEntities);
	stl::free_container(m_deferredUsedEntities);
}

void CEntitySystem::PurgeHeaps()
{
	for (THeaps::iterator it = m_garbageLayerHeaps.begin(); it != m_garbageLayerHeaps.end(); ++it)
	{
		if(it->pHeap)
			it->pHeap->Release();
	}

	stl::free_container(m_garbageLayerHeaps);
}

void CEntitySystem::Reset()
{
	m_pPartitionGrid->BeginReset();
	m_pProximityTriggerSystem->BeginReset();

	m_pEntityLoadManager->Reset();

	m_pEventDistributer->Reset();

	assert(m_pEntityPoolManager);
	m_pEntityPoolManager->Reset();

	// Flush the physics linetest and events queue
	if (gEnv->pPhysicalWorld)
	{
		gEnv->pPhysicalWorld->TracePendingRays(0);
		gEnv->pPhysicalWorld->ClearLoggedEvents();
	}
	GetBreakableManager()->ResetBrokenObjects();

	PurgeDeferredCollisionEvents(true);

	CheckInternalConsistency();

	ClearLayers();
	m_mapEntityNames.clear();

	m_genIdMap.clear();
#ifdef SW_ENTITY_ID_USE_GUID
	m_nGeneratedFromGuid = 2;
#endif

	m_bReseting = true;

	// Delete entities that have already been added to the delete list.
	UpdateDeletedEntities();

	uint32 dwMaxUsed = (uint32)m_EntitySaltBuffer.GetMaxUsed()+1;

	for(uint32 dwI=0;dwI<dwMaxUsed;++dwI)
	{
		CEntity *ent = m_EntityArray[dwI];
		if(ent)
		{
			if (!ent->m_bGarbage)
			{
				ent->m_flags &= ~ENTITY_FLAG_UNREMOVABLE;
				ent->m_nKeepAliveCounter = 0;
				RemoveEntity( ent->GetId(),false );
			}
			else
			{
				stl::push_back_unique(m_deletedEntities, ent);
			}
		}
	}
	// Delete entity that where added to delete list.
	UpdateDeletedEntities();

	ClearEntityArray();
	m_mapActiveEntities.clear();
	m_tempActiveEntitiesValid = false;
	stl::free_container(m_deletedEntities);
	m_guidMap.clear();

	ResetAreas();

	m_EntitySaltBuffer.Reset();
	m_timersMap.clear();

	for (int i = 0; i < ENTITY_EVENT_LAST; i++)
	{
		m_eventListeners[i].clear();
	}
	m_pProximityTriggerSystem->Reset();
	m_pPartitionGrid->Reset();

	m_entityTimeoutList.Clear();
	m_bReseting = false;

	CheckInternalConsistency();
}

//////////////////////////////////////////////////////////////////////
void CEntitySystem::AddSink( IEntitySystemSink *pSink, uint32 subscriptions, uint64 onEventSubscriptions )
{
	assert(pSink);

	if(pSink)
	{
		for (uint i = 0; i < SinkMaxEventSubscriptionCount; ++i)
		{
			if ((subscriptions & (1 << i)) && (i != static_log2<IEntitySystem::OnEvent>::value))
			{
				assert(!stl::find(m_sinks[i], pSink));
				m_sinks[i].push_back(pSink);
			}
		}

		if (subscriptions & IEntitySystem::OnEvent)
			m_onEventSinks.push_back(OnEventSink(onEventSubscriptions, pSink));
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::RemoveSink( IEntitySystemSink *pSink )
{
	assert(pSink);

	for (uint i = 0; i < SinkMaxEventSubscriptionCount; ++i)
	{
		EntitySystemSinks& sinks = m_sinks[i];

		stl::find_and_erase(sinks, pSink);
	}

	EntitySystemOnEventSinks::iterator it = m_onEventSinks.begin();
	EntitySystemOnEventSinks::iterator end = m_onEventSinks.end();

	for ( ; it != end; ++it)
	{
		OnEventSink& sink = *it;
		
		if (sink.pSink == pSink)
		{
			m_onEventSinks.erase(it);
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool CEntitySystem::OnBeforeSpawn( SEntitySpawnParams &params )
{
	EntitySystemSinks& sinks = m_sinks[static_log2<IEntitySystem::OnBeforeSpawn>::value];
	EntitySystemSinks::iterator si = sinks.begin();
	EntitySystemSinks::iterator siEnd = sinks.end();

	for (; si != siEnd; ++si)
	{
		if (!(*si)->OnBeforeSpawn(params))
			return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::OnEntityReused( IEntity *pEntity, SEntitySpawnParams &params )
{
	EntitySystemSinks& sinks = m_sinks[static_log2<IEntitySystem::OnReused>::value];
	EntitySystemSinks::iterator si = sinks.begin();
	EntitySystemSinks::iterator siEnd = sinks.end();

	for ( ; si != siEnd; ++si)
	{
		(*si)->OnReused(pEntity, params);
	}

	assert(m_pEntityPoolManager);
	CEntity *pCEntity = (CEntity*)pEntity;
	if (pCEntity && pCEntity->IsPoolControlled())
		m_pEntityPoolManager->OnPoolEntityInUse(pCEntity, params.prevId);
}

//////////////////////////////////////////////////////////////////////
EntityId CEntitySystem::GenerateEntityId(bool bStaticId)
{
	EntityId returnId = INVALID_ENTITYID;

	if (bStaticId)
		returnId = HandleToId(m_EntitySaltBuffer.InsertStatic());
	else
		returnId = HandleToId(m_EntitySaltBuffer.InsertDynamic());

	return returnId;
}

//load an entity from script,set position and add to the entity system
//////////////////////////////////////////////////////////////////////
IEntity* CEntitySystem::SpawnEntity( SEntitySpawnParams &params,bool bAutoInit )
{
	FUNCTION_PROFILER( m_pISystem,PROFILE_ENTITY );

#ifndef _RELEASE
	if (params.sName)
	{
		unsigned int countIllegalCharacters = 0;

		for (const char * checkEachChar = params.sName; *checkEachChar; ++ checkEachChar)
		{
			countIllegalCharacters += ((*checkEachChar) >= 32 && (*checkEachChar) <= 126) ? 0 : 1;
		}

		if (countIllegalCharacters)
		{
			CryWarning (VALIDATOR_MODULE_ENTITYSYSTEM, VALIDATOR_WARNING, "!Name '%s' contains %u illegal character(s) while creating an instance of '%s'", params.sName, countIllegalCharacters, params.pClass ? params.pClass->GetName() : "no class specified");
		}
	}
#endif

#if ENABLE_STATOSCOPE
	// Disabled for now because statoscope doesn't cope very well with the volume of user markers this generates
	/*if (gEnv->pStatoscope && (params.nFlags & (ENTITY_FLAG_CLIENT_ONLY|ENTITY_FLAG_SERVER_ONLY)) == 0)
	{
		stack_string userMarker;
		userMarker.Format("SpawnEntity (%s : %s)", params.pClass->GetName(), params.sName);
		gEnv->pStatoscope->AddUserMarker(userMarker.c_str());
	}*/
#endif

	// If entity spawns from archetype take class from archetype
	if (params.pArchetype)
		params.pClass = params.pArchetype->GetClass();

	MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_Other, EMemStatContextFlags::MSF_Instance, "SpawnEntity %s", params.pClass ? params.pClass->GetName() : "WITH NO CLASS");

	assert( params.pClass != NULL ); // Class must always be specified

	if (!params.pClass)
	{
		CryWarning(VALIDATOR_MODULE_ENTITYSYSTEM, VALIDATOR_WARNING, "Trying to spawn entity %s with no entity class. Spawning refused.", params.sName);
		return NULL;
	}
	
	if(m_bLocked)
	{
		if(!params.bIgnoreLock)
		{
			CryWarning(VALIDATOR_MODULE_ENTITYSYSTEM, VALIDATOR_WARNING, "Spawning entity %s of class %s refused due to spawn lock! Spawning entities is not allowed at this time.", params.sName, params.pClass->GetName());
			return NULL;
		}
	}

	if (!OnBeforeSpawn(params))
		return NULL;

	CEntity *pEntity=0;
  
	if (m_idForced)
	{
		params.id = m_idForced;
		m_idForced = 0;
	}

	if(!params.id)			// entityid wasn't given
	{
		// get entity id and mark it
		params.id = GenerateEntityId(params.bStaticEntityId);

		if(!params.id)
		{
			EntityWarning( "CEntitySystem::SpawnEntity Failed, Can't spawn entity %s. ID range is full (internal error)",(const char*)params.sName );
			CheckInternalConsistency();
			return NULL;
		}
	}
	else 
	{
		if(m_EntitySaltBuffer.IsUsed(IdToHandle(params.id).GetIndex()))		
		{
			// was reserved or was existing already

			pEntity = m_EntityArray[IdToHandle(params.id).GetIndex()];

			if(pEntity)	
				EntityWarning( "Entity with id=%d, %s already spawned on this client...override",pEntity->GetId(),pEntity->GetEntityTextDescription() );
			else 
				m_EntitySaltBuffer.InsertKnownHandle(IdToHandle(params.id));
		}
		else
		{
			m_EntitySaltBuffer.InsertKnownHandle(IdToHandle(params.id));
		}
	}

	if(!pEntity)
	{
		// Is this entity class pooled, if so take entity from pool.
		if (params.bCreatedThroughPool && m_pEntityPoolManager->IsClassUsingPools(params.pClass))
		{
			pEntity = m_pEntityPoolManager->PrepareDynamicFromPool(params, bAutoInit);
		}

		if (!pEntity)
		{
			// Makes new entity.
			pEntity = new CEntity(params);
			// put it into the entity map
			m_EntityArray[IdToHandle(params.id).GetIndex()]=pEntity;

			if (params.guid)
				RegisterEntityGuid( params.guid,params.id );

			if(bAutoInit)
			{
				if(!InitEntity(pEntity,params))		// calls DeleteEntity() on failure
				{
					return NULL;
				}
			}
		}
	}
  //
  if( CVar::es_debugEntityLifetime )
  {
    CryLog("CEntitySystem::SpawnEntity %s %s 0x%x",pEntity ? pEntity->GetClass()->GetName() : "null",pEntity ? pEntity->GetName() : "null",pEntity ? pEntity->GetId() : 0);
  }
  //
	return pEntity;
}

//////////////////////////////////////////////////////////////////////////
bool CEntitySystem::ResetEntityId(CEntity *pEntity, EntityId newEntityId)
{
	assert(pEntity);
	if (!pEntity)
		return false;

	// New entity Id must not currently be in use either!
	bool bNewHandleInUse = false;
	if (newEntityId)
	{
		const CSaltHandle<> newHandle = IdToHandle(newEntityId);
		bNewHandleInUse = m_EntitySaltBuffer.IsUsed(newHandle.GetIndex());
		if (bNewHandleInUse)
		{
			// If the buffer marks it as in use but no entity exists in the array, this means the id was
			//	reserved, so it is still okay to use.
			CEntity *pUsedEntity = m_EntityArray[newHandle.GetIndex()];
			if (pUsedEntity)
			{
				assert(false);
				return false;
			}
		}
	}

	// Remove the old salt handle reference
	const EntityId oldEntityId = pEntity->GetId();
	const CSaltHandle<> oldHandle = IdToHandle(oldEntityId);
	if (m_EntitySaltBuffer.IsUsed(oldHandle.GetIndex()))
	{
		//m_EntitySaltBuffer.Remove(oldHandle); // TODO What if it was reserved?
		m_EntityArray[oldHandle.GetIndex()] = NULL;
	}

	RemoveEntityFromActiveList(pEntity);

	// Inform the distributer of any change!
	m_pEventDistributer->RemapEntityID( pEntity->GetId(), newEntityId );

	if (newEntityId)
	{
		const CSaltHandle<> newHandle = IdToHandle(newEntityId);

		if (!bNewHandleInUse)
			m_EntitySaltBuffer.InsertKnownHandle(newHandle);

		m_EntityArray[newHandle.GetIndex()] = pEntity;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CEntitySystem::InitEntity( IEntity* pEntity,SEntitySpawnParams &params )
{
	INDENT_LOG_DURING_SCOPE(true, "Initializing entity '%s' of class '%s' (layer='%s')", params.sName, params.pClass->GetName(), params.sLayerName);

	assert( pEntity );

	CRY_DEFINE_ASSET_SCOPE( "Entity",params.sName );

	CEntity *pCEntity = (CEntity*)pEntity;

	// initialize entity
	if(!pCEntity->Init(params))
	{
		// The entity may have already be scheduled for deletion [7/6/2010 evgeny]
		if (std::find(m_deletedEntities.begin(), m_deletedEntities.end(), pCEntity) == m_deletedEntities.end())
		{
			DeleteEntity(pCEntity);
		}

		return false;
	}

	EntitySystemSinks& sinks = m_sinks[static_log2<IEntitySystem::OnSpawn>::value];
	EntitySystemSinks::iterator si = sinks.begin();
	EntitySystemSinks::iterator siEnd = sinks.end();

	for ( ; si != siEnd; ++si)
		(*si)->OnSpawn(pEntity, params);

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::DeleteEntity(CEntity* pEntity)
{
	FUNCTION_PROFILER( m_pISystem,PROFILE_ENTITY );

	if( CVar::es_debugEntityLifetime )
	{
		CryLog("CEntitySystem::DeleteEntity %s %s 0x%x",
			pEntity ? pEntity->GetClass()->GetName() : "null",
			pEntity ? pEntity->GetName() : "null",
			pEntity ? pEntity->GetId() : 0);
	}

	if (pEntity)
	{
		CRY_ASSERT_MESSAGE(!pEntity->IsFromPool(), "About to delete an entity who is still marked as coming from the pool");

		if (gEnv->pGame)
		{
			IGameFramework* pFramework = gEnv->pGame->GetIGameFramework();

			if (pFramework)
			{
				INetContext* pContext = pFramework->GetNetContext();
			
				if (pContext)
				{
					if (pContext->IsBound(pEntity->GetId()))
					{
						// If the network is still there and this entity is still bound to it force an unbind.
						pContext->UnbindObject(pEntity->GetId());
					}
				}
			}
		}

		m_pEventDistributer->OnEntityDeleted( pEntity );

		pEntity->ShutDown();

		// Make sure this entity is not in active list anymore.
		RemoveEntityFromActiveList(pEntity);

		m_EntityArray[IdToHandle(pEntity->GetId()).GetIndex()] = 0;
		m_EntitySaltBuffer.Remove(IdToHandle(pEntity->GetId()));

		if (pEntity->m_guid)
			UnregisterEntityGuid(pEntity->m_guid);

		delete pEntity;
	}
}


//////////////////////////////////////////////////////////////////////
void CEntitySystem::ClearEntityArray()
{
	CheckInternalConsistency();

	uint32 dwMaxIndex = (uint32)(m_EntitySaltBuffer.GetTSize());

	m_EntityArray.resize(dwMaxIndex);

	for(uint32 dwI=0;dwI<dwMaxIndex;++dwI)
	{
		CRY_ASSERT_TRACE(m_EntityArray[dwI] == NULL, ("About to \"leak\" entity id %d (%s)", dwI, m_EntityArray[dwI]->GetName()));
		m_EntityArray[dwI] = 0;
	}

	m_tempActiveEntitiesValid = false;

	CheckInternalConsistency();
}


//////////////////////////////////////////////////////////////////////
void CEntitySystem::RemoveEntity( EntityId entity,bool bForceRemoveNow )
{
	ENTITY_PROFILER
	assert( (bool)IdToHandle(entity) );

	CEntity *pEntity = static_cast<CEntity*>(GetEntity(entity));
  //
  if( CVar::es_debugEntityLifetime )
  {
    CryLog("CEntitySystem::RemoveEntity %s %s 0x%x",pEntity ? pEntity->GetClass()->GetName() : "null" ,pEntity ? pEntity->GetName() : "null",pEntity ? pEntity->GetId() : 0);
  }
  //  
  if(pEntity)
	{ 

		if(m_bLocked)
			CryWarning(VALIDATOR_MODULE_ENTITYSYSTEM, VALIDATOR_WARNING, "Removing entity during system lock : %s with id %i", pEntity->GetName(), (int)entity);

		if (pEntity->GetId() != entity)
		{
			EntityWarning("Trying to remove entity with mismatching salts. id1=%d id2=%d", entity, pEntity->GetId());
			CheckInternalConsistency();
			if (ICVar * pVar = gEnv->pConsole->GetCVar("net_assertlogging"))
			{
				if (pVar->GetIVal())
				{
					gEnv->pSystem->debug_LogCallStack();
				}
			}
			return;
		}

		if (pEntity->IsFromPool())
		{
			m_pEntityPoolManager->ReturnToPool(entity, false);
		}
		else if (!pEntity->m_bGarbage)
		{
			EntitySystemSinks& sinks = m_sinks[static_log2<IEntitySystem::OnRemove>::value];
			EntitySystemSinks::iterator si = sinks.begin();
			EntitySystemSinks::iterator siEnd = sinks.end();

			for ( ; si != siEnd; ++si)
			{
				if (!(*si)->OnRemove(pEntity))
				{
					// basically unremovable... but hide it anyway to be polite
					pEntity->Hide(true);
					return;
				}
			}

			// Send deactivate event.
			SEntityEvent entevent;
			entevent.event = ENTITY_EVENT_DONE;
			entevent.nParam[0] = pEntity->GetId();
			pEntity->SendEvent(entevent);

			// Make sure this entity is not in active list anymore.
			RemoveEntityFromActiveList(pEntity);

			if ( !(pEntity->m_flags&ENTITY_FLAG_UNREMOVABLE) && pEntity->m_nKeepAliveCounter == 0)
			{
				pEntity->m_bGarbage = true;
				if (bForceRemoveNow)
					DeleteEntity(pEntity);
				else
				{
					// add entity to deleted list, and actually delete entity on next update.
					stl::push_back_unique(m_deletedEntities, pEntity);
				}
			}
			else
			{
				// Unremovable entities. are hidden and deactivated.
				pEntity->Hide(true);

				pEntity->m_bGarbage = true;

				// remember kept alive entities to ged rid of them as soon as they are no longer needed
				if( pEntity->m_nKeepAliveCounter > 0 && !(pEntity->m_flags&ENTITY_FLAG_UNREMOVABLE) )
				{
					m_deferredUsedEntities.push_back(pEntity);
				}
			}
		}
		else if (bForceRemoveNow)
		{
			//DeleteEntity(pEntity);
		}
	}
}

//////////////////////////////////////////////////////////////////////
void CEntitySystem::RemoveEntityFromActiveList(CEntity *pEntity)
{
	if (pEntity)
	{
		m_mapActiveEntities.erase( pEntity->GetId() );
		ActivatePrePhysicsUpdateForEntity( pEntity, false );
		m_tempActiveEntitiesValid = false;
		pEntity->m_bActive = false;
		if (pEntity->m_bInActiveList)
		{
			pEntity->m_bInActiveList = false;
			SEntityEvent event(ENTITY_EVENT_DEACTIVATED);
			pEntity->SendEvent(event);
		}
	}

	assert(pEntity && m_mapActiveEntities.count(pEntity->GetId())==0);
	assert(pEntity && m_mapPrePhysicsEntities.count(pEntity->GetId())==0);
}

//////////////////////////////////////////////////////////////////////
IEntity* CEntitySystem::GetEntity( EntityId id ) const
{
	return GetEntityFromID(id);
}

CEntity* CEntitySystem::GetEntityFromID( EntityId id ) const
{
	uint32 dwMaxUsed = m_EntitySaltBuffer.GetMaxUsed();
	uint32 hdl = IdToIndex(id);

	assert(hdl<=dwMaxUsed);		// bad input id parameter

	uint32 hd1cond = hdl<=dwMaxUsed;
	hd1cond = (uint32)((int32)hd1cond | -(int32)hd1cond ); //0 for hdl>dwMaxUsed, 0xFFFFFFFF for hdl<=dwMaxUsed
	hdl = hd1cond & hdl;

	CEntity *const pEntity = m_EntityArray[hdl];

	if(pEntity)
	{
		if (pEntity->CEntity::GetId()==id)
			return pEntity;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
EntityId CEntitySystem::GetClonedEntityId( EntityId origId, EntityId refId ) const
{
	CEntity* pEntity = GetEntityFromID(refId);

	// If the reference entity exists, and it's from a cloned layer, use the entity load
	// manager to remap the original id to the cloned version in that layer.
	if (pEntity != NULL && pEntity->GetCloneLayerId() != -1)
	{
		return m_pEntityLoadManager->GetClonedId(pEntity->GetCloneLayerId(), origId);
	}

	return origId;
}

//////////////////////////////////////////////////////////////////////////
IEntity* CEntitySystem::FindEntityByName( const char *sEntityName ) const
{
	if (!sEntityName || !sEntityName[0])
		return 0; // no entity name specified

	if (CVar::es_DebugFindEntity != 0)
	{
		CryLog( "FindEntityByName: %s",sEntityName );
		if (CVar::es_DebugFindEntity == 2)
			GetISystem()->debug_LogCallStack();
	}

	// Find first object with this name.
	EntityNamesMap::const_iterator it = m_mapEntityNames.lower_bound(sEntityName);
	if (it != m_mapEntityNames.end())
	{
		if (stricmp(it->first,sEntityName) == 0)
			return GetEntity(it->second);
	}
	
	return 0;		// name not found
}

//////////////////////////////////////////////////////////////////////
uint32 CEntitySystem::GetNumEntities() const
{
	uint32 dwRet=0;
	uint32 dwMaxUsed = (uint32)m_EntitySaltBuffer.GetMaxUsed()+1;

	for(uint32 dwI=0;dwI<dwMaxUsed;++dwI)
	{
		CEntity *ce = m_EntityArray[dwI];

		if(ce)
			++dwRet;
	}

	return dwRet;
}


//////////////////////////////////////////////////////////////////////////
IEntity* CEntitySystem::GetEntityFromPhysics( IPhysicalEntity *pPhysEntity ) const
{
	assert(pPhysEntity);
	CEntity *pEntity = NULL;
	if(pPhysEntity)
		pEntity = (CEntity*)pPhysEntity->GetForeignData(PHYS_FOREIGN_ID_ENTITY);
	return pEntity;

	//CPhysicalProxy *pPhysProxy = (CPhysicalProxy*)pPhysEntity->GetForeignData(PHYS_FOREIGN_ID_ENTITY);
	//if (pPhysProxy)
		//return pPhysProxy->GetEntity();
	//return NULL;
}

//////////////////////////////////////////////////////////////////////
int CEntitySystem::GetPhysicalEntitiesInBox(const Vec3 &origin, float radius,IPhysicalEntity **&pList, int physFlags) const
{
	assert(m_pISystem);

	Vec3 bmin = origin - Vec3(radius, radius, radius);
	Vec3 bmax = origin + Vec3(radius, radius, radius);

	return m_pISystem->GetIPhysicalWorld()->GetEntitiesInBox(bmin, bmax, pList, physFlags);
}

//////////////////////////////////////////////////////////////////////////
int CEntitySystem::QueryProximity( SEntityProximityQuery &query )
{
	SPartitionGridQuery q;
	q.aabb = query.box;
	q.nEntityFlags = query.nEntityFlags;
	q.pEntityClass = query.pEntityClass;
	m_pPartitionGrid->GetEntitiesInBox( q );
	query.pEntities = 0;
	query.nCount = (int)q.pEntities->size();
	if (q.pEntities && query.nCount > 0)
	{
		query.pEntities = &((*q.pEntities)[0]);
	}
	return query.nCount;
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::UpdateDeletedEntities()
{
	while (!m_deletedEntities.empty())
	{
		CEntity *pEntity = m_deletedEntities.back();
		m_deletedEntities.pop_back();
		DeleteEntity(pEntity);
	}
}

//////////////////////////////////////////////////////////////////////
void CEntitySystem::UpdateNotSeenTimeouts()
{
	FUNCTION_PROFILER_FAST( m_pISystem,PROFILE_ENTITY,g_bProfilerEnabled );

	float timeoutVal = float(CVar::pNotSeenTimeout->GetIVal());
	if (timeoutVal == 0.0f)
		return;

	while (EntityId timeoutEntityId = m_entityTimeoutList.PopTimeoutEntity(timeoutVal))
	{
		CEntity* pEntity = GetEntityFromID(timeoutEntityId);
		if (pEntity)
		{
			SEntityEvent entityEvent(ENTITY_EVENT_NOT_SEEN_TIMEOUT);
			pEntity->SendEvent(entityEvent);
		}
	}
}

//////////////////////////////////////////////////////////////////////
void CEntitySystem::PrePhysicsUpdate()
{
	CRYPROFILE_SCOPE_PROFILE_MARKER("EntitySystem::PrePhysicsUpdate");

	if( static_cast<CComponentEventDistributer*> (m_pEventDistributer)->IsEnabled() )
	{
		DoPrePhysicsUpdateFast();
	}
	else
	{
		DoPrePhysicsUpdate();
	}
}

//update the entity system
//////////////////////////////////////////////////////////////////////
void CEntitySystem::Update()
{
	CRYPROFILE_SCOPE_PROFILE_MARKER("EntitySystem::Update");
	const float fFrameTime = gEnv->pTimer->GetFrameTime();
	if (fFrameTime > FLT_EPSILON)
	{
#ifdef PROFILE_ENTITYSYSTEM
		if (CVar::pProfileEntities->GetIVal() < 0)
			GetISystem()->GetIProfilingSystem()->VTuneResume();
#endif

		FUNCTION_PROFILER_FAST( m_pISystem,PROFILE_ENTITY,g_bProfilerEnabled );

		UpdateTimers();

		UpdateEngineCVars();

		m_pEntityPoolManager->Update();

#if defined(USE_GEOM_CACHES)
		m_pGeomCacheAttachmentManager->Update();
#endif
		m_pCharacterBoneAttachmentManager->Update();

		DoUpdateLoop(fFrameTime);

		// Update info on proximity triggers.
		m_pProximityTriggerSystem->Update();
	
		// Now update area manager to send enter/leave events from areas.
		m_pAreaManager->Update();

		// Delete entities that must be deleted.
		if (!m_deletedEntities.empty())
			UpdateDeletedEntities();

		UpdateNotSeenTimeouts();

		if (CVar::pEntityBBoxes->GetIVal() != 0)
		{
			// Render bboxes of all entities.
			uint32 dwMaxUsed = (uint32)m_EntitySaltBuffer.GetMaxUsed()+1;
			for(uint32 dwI=0;dwI<dwMaxUsed;++dwI)
			{
				CEntity *pEntity = m_EntityArray[dwI];
				if(pEntity)
					DebugDraw(pEntity,-1);
			}
		}

		for (THeaps::iterator it = m_garbageLayerHeaps.begin(); it != m_garbageLayerHeaps.end();)
		{
			SEntityLayerGarbage& lg = *it;
			if (lg.pHeap->Cleanup())
			{
				lg.pHeap->Release();
				it = m_garbageLayerHeaps.erase(it);
			}
			else
			{
				++ lg.nAge;
				if (lg.nAge == 32)
				{
					CryWarning(VALIDATOR_MODULE_ENTITYSYSTEM, VALIDATOR_WARNING, "Layer '%s', heap %p, has gone for %i frames without releasing its heap. Possible leak?", lg.layerName.c_str(), lg.pHeap, lg.nAge);
				}

				++ it;
			}
		}

		PurgeDeferredCollisionEvents();
	}

	if (CVar::pDrawAreas->GetIVal() != 0 || CVar::pDrawAreaDebug->GetIVal() != 0)
		m_pAreaManager->DrawAreas(gEnv->pSystem);

	if (CVar::pDrawAreaGrid->GetIVal() != 0)
		m_pAreaManager->DrawGrid();

	if (CVar::es_DebugPool != 0)
		m_pEntityPoolManager->DebugDraw();

	if (CVar::es_DebugEntityUsage > 0)
		DebugDrawEntityUsage();

	if (NULL != gEnv->pLiveCreateHost)
	{
		gEnv->pLiveCreateHost->DrawOverlay();
	}

#ifdef PROFILE_ENTITYSYSTEM
	if (CVar::pProfileEntities->GetIVal() < 0)
		GetISystem()->GetIProfilingSystem()->VTunePause();
#endif

	if (CVar::es_LayerDebugInfo > 0)
		DebugDrawLayerInfo();
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::DebugDrawEntityUsage()
{
	static float fLastUpdate = 0.0f;
	float fCurrTime = gEnv->pTimer->GetFrameStartTime().GetSeconds();
	
	typedef std::pair<EntityId, uint32> TEntityPair;
	typedef std::vector<TEntityPair> TEntities;
	typedef std::map<IEntityClass*, TEntities> TClassEntitiesMap;
	static TClassEntitiesMap classEntitiesMap;

	ICrySizer	*pSizer = gEnv->pSystem->CreateSizer();

	if (fCurrTime-fLastUpdate >= 0.001f * max(CVar::es_DebugEntityUsage,1000))
	{
		fLastUpdate = fCurrTime;
		
		string sFilter = CVar::es_DebugEntityUsageFilter;
		sFilter.MakeLower();

		classEntitiesMap.clear();
		std::vector<CEntity *>::const_iterator itEntity = m_EntityArray.begin();
		std::vector<CEntity *>::const_iterator itEntityEnd = m_EntityArray.end();
		for (; itEntity != itEntityEnd; ++itEntity)
		{
			const CEntity *pEntity = *itEntity;
			if (pEntity)
			{
				if (!sFilter.empty())
				{
					IEntityClass *pClass = pEntity->GetClass();
					string szName = pClass->GetName();
					szName.MakeLower();
					if (szName.find(sFilter) == string::npos)
						continue;
				}

				// Calculate memory usage

				const uint32 prevMemoryUsage = pSizer->GetTotalSize();

				pEntity->GetMemoryUsage(pSizer);

				const uint32 uMemoryUsage = pSizer->GetTotalSize() - prevMemoryUsage;

				classEntitiesMap[pEntity->GetClass()].push_back(TEntityPair(pEntity->GetId(),uMemoryUsage));
			}
		}
	}

	pSizer->Release();

	if (!classEntitiesMap.empty())
	{
		IRenderer *pRenderer = gEnv->pRenderer;
		assert(pRenderer);

		float fColumnY = 11.0f;
		const float colWhite[] = {1.0f,1.0f,1.0f,1.0f};
		const float colGreen[] = {0.0f,1.0f,0.0f,1.0f};

		const float fColumnX_Class			= 50.0f;
		const float fColumnX_TotalCount		= 250.0f;
		const float fColumnX_ActiveCount	= 350.0f;
		const float fColumnX_HiddenCount	= 450.0f;
		const float fColumnX_MemoryUsage	= 550.0f;
		const float fColumnX_MemoryHidden	= 650.0f;

		string sTitle = "Entity Class";
		if (CVar::es_DebugEntityUsageFilter && CVar::es_DebugEntityUsageFilter[0])
		{
			sTitle.append(" [");
			sTitle.append(CVar::es_DebugEntityUsageFilter);
			sTitle.append("]");
		}

		pRenderer->Draw2dLabel(fColumnX_Class, fColumnY, 1.2f, colWhite, false, sTitle.c_str());
		pRenderer->Draw2dLabel(fColumnX_TotalCount, fColumnY, 1.2f, colWhite, true, "Total / Pool Counts");
		pRenderer->Draw2dLabel(fColumnX_ActiveCount, fColumnY, 1.2f, colWhite, true, "Active");
		pRenderer->Draw2dLabel(fColumnX_HiddenCount, fColumnY, 1.2f, colWhite, true, "Hidden");
		pRenderer->Draw2dLabel(fColumnX_MemoryUsage, fColumnY, 1.2f, colWhite, true, "Memory Usage");
		pRenderer->Draw2dLabel(fColumnX_MemoryHidden, fColumnY, 1.2f, colWhite, true, "[Only Hidden]");
		fColumnY += 15.0f;

		TClassEntitiesMap::const_iterator itClass = classEntitiesMap.begin();
		TClassEntitiesMap::const_iterator itClassEnd = classEntitiesMap.end();
		for (; itClass != itClassEnd; ++itClass)
		{
			IEntityClass *pClass = itClass->first;
			const char* szName = pClass->GetName();

			// Skip if empty
			const TEntities &entities = itClass->second;
			if (entities.empty())
				continue;

			// Generate counts
			uint32 uTotalCount = 0, uActiveCount = 0, uHiddenCount = 0, uTotalMemory = 0, uHiddenMemory = 0;
			TEntities::const_iterator itEntity = entities.begin();
			TEntities::const_iterator itEntityEnd = entities.end();
			for (; itEntity != itEntityEnd; ++itEntity)
			{
				EntityId entityId = itEntity->first;
				const CEntity *pEntity = GetEntityFromID(entityId);
				if (!pEntity)
					continue;

				uTotalCount++;
				uTotalMemory += itEntity->second;

				if (pEntity->IsActive())
				{
					uActiveCount++;
				}

				if (pEntity->IsHidden())
				{
					uHiddenCount++;
					uHiddenMemory += itEntity->second;
				}
			}

			pRenderer->Draw2dLabel(fColumnX_Class, fColumnY, 1.0f, colWhite, false, szName);
			pRenderer->Draw2dLabel(fColumnX_TotalCount, fColumnY, 1.0f, colWhite, true, "%u / %u", uTotalCount, m_pEntityPoolManager->GetPoolSizeByClass(szName));
			pRenderer->Draw2dLabel(fColumnX_ActiveCount, fColumnY, 1.0f, colWhite, true, "%u", uActiveCount);
			pRenderer->Draw2dLabel(fColumnX_HiddenCount, fColumnY, 1.0f, colWhite, true, "%u", uHiddenCount);
			pRenderer->Draw2dLabel(fColumnX_MemoryUsage, fColumnY, 1.0f, colWhite, true, "%u (%uKb)", uTotalMemory, uTotalMemory/1000);
			pRenderer->Draw2dLabel(fColumnX_MemoryHidden, fColumnY, 1.0f, colGreen, true, "%u (%uKb)", uHiddenMemory, uHiddenMemory/1000);
			fColumnY += 12.0f;
		}
	}
}
//////////////////////////////////////////////////////////////////////////

#ifdef ENABLE_PROFILING_CODE

namespace
{
	void DrawText( const float x, const float y,ColorF c,const char * format, ... )
	{
		va_list args;
		va_start(args, format);

		SDrawTextInfo ti;
		ti.flags = eDrawText_FixedSize | eDrawText_2D | eDrawText_Monospace;
		ti.xscale = ti.yscale = 1.2f;
		ti.color[0] = c.r;
		ti.color[1] = c.g;
		ti.color[2] = c.b;
		ti.color[3] = c.a;
		gEnv->pRenderer->DrawTextQueued( Vec3(x,y,1.0f),ti,format,args );
		va_end(args);
	}
}

#endif

void CEntitySystem::DebugDrawLayerInfo()
{

#ifdef ENABLE_PROFILING_CODE

	bool bRenderAllLayerStats = CVar::es_LayerDebugInfo >= 2;
	bool bRenderMemoryStats = CVar::es_LayerDebugInfo == 3;
	bool bRenderAllLayerPakStats = CVar::es_LayerDebugInfo == 4;
	bool bShowLayerActivation = CVar::es_LayerDebugInfo == 5;

	float tx = 0;
	float ty = 30;
	float ystep = 12.0f;
	ColorF clText(0,1,1,1);

	if(bShowLayerActivation) // Show which layer was switched on or off
	{
		const float fShowTime = 10.0f; // 10 seconds
		float fCurTime = gEnv->pTimer->GetCurrTime();
		float fPrevTime = 0;
		std::vector<SLayerProfile>::iterator ppClearProfile = m_layerProfiles.end();
		for (std::vector<SLayerProfile>::iterator ppProfiles = m_layerProfiles.begin(); ppProfiles!=m_layerProfiles.end(); ++ ppProfiles)
		{
			SLayerProfile& profile = (*ppProfiles);
			CEntityLayer* pLayer = profile.pLayer;

			ColorF clTextProfiledTime(0,1,1,1);
			if( profile.fTimeMS > 50 ) // Red color for more then 50 ms
				clTextProfiledTime = ColorF (1, 0.3f, 0.3f, 1);
			else if ( profile.fTimeMS > 10 )  // Yellow color for more then 10 ms
				clTextProfiledTime = ColorF (1, 1, 0.3f, 1);

			if (!profile.isEnable)
				clTextProfiledTime -= ColorF(0.3f, 0.3f, 0.3f, 0);

			float xstep = 0.0f;
			if (pLayer->GetParentName().length() != 0)
			{
				xstep += 20;
				CEntityLayer* pParenLayer = FindLayer(pLayer->GetParentName());
				if(pParenLayer && pParenLayer->GetParentName().length() != 0)
					xstep += 20;
			}
			if (profile.fTimeOn != fPrevTime)
			{
				ty+=10.0f;
				fPrevTime = profile.fTimeOn;
			}

			DrawText( tx+xstep, ty+=ystep, clTextProfiledTime, "%.1f ms: %s (%s)", profile.fTimeMS, pLayer->GetName().c_str(), profile.isEnable ? "On":"Off");
			if(ppClearProfile==m_layerProfiles.end() && fCurTime - profile.fTimeOn > fShowTime)
				ppClearProfile = ppProfiles;
		}
		if(ppClearProfile!=m_layerProfiles.end())
			m_layerProfiles.erase(ppClearProfile, m_layerProfiles.end());
		return;
	}


	typedef std::map<string, std::vector<CEntityLayer*> > TParentLayerMap;
	TParentLayerMap parentLayers;

	for (TLayers::iterator it = m_layers.begin(); it != m_layers.end(); ++it)
	{
		CEntityLayer* pLayer = it->second;
		if (pLayer->GetParentName().length() == 0) 
		{
			TParentLayerMap::iterator itFindRes = parentLayers.find(pLayer->GetName());
			if (itFindRes == parentLayers.end())
				parentLayers[pLayer->GetName()] = std::vector<CEntityLayer*>();
		} else {
			parentLayers[pLayer->GetParentName()].push_back(pLayer);
		}
	}

	SLayerPakStats layerPakStats;
	layerPakStats.m_MaxSize = 0;
	layerPakStats.m_UsedSize = 0;
	IResourceManager* pResMan = gEnv->pSystem->GetIResourceManager();
	if (pResMan)
		pResMan->GetLayerPakStats(layerPakStats, bRenderAllLayerPakStats);

	if (bRenderAllLayerPakStats)
	{
		DrawText( tx,ty+=ystep,clText, "Layer Pak Stats: %1.1f MB / %1.1f MB)",
			(float) layerPakStats.m_UsedSize / (1024.f * 1024.f),
			(float) layerPakStats.m_MaxSize / (1024.f * 1024.f));

		for (SLayerPakStats::TEntries::iterator it = layerPakStats.m_entries.begin(); it != layerPakStats.m_entries.end(); ++it)
		{
			SLayerPakStats::SEntry& entry = *it;

			DrawText( tx,ty+=ystep,clText, "  %20s: %1.1f MB - %s)", entry.name.c_str(),
				(float)entry.nSize / (1024.f * 1024.f), entry.status.c_str());
		}
		ty+=ystep;
		DrawText( tx,ty+=ystep,clText, "All Layers:");
	}
	else
	{
		string tmp = "Active Brush Layers";
		if (bRenderAllLayerStats)
			tmp = "All Layers";

		DrawText( tx,ty+=ystep,clText, "%s (PakInfo: %1.1f MB / %1.1f MB):", tmp.c_str(),
			(float) layerPakStats.m_UsedSize / (1024.f * 1024.f),
			(float) layerPakStats.m_MaxSize / (1024.f * 1024.f));
	}

	ty+=ystep;

	ICrySizer* pSizer = 0;
	if (bRenderMemoryStats)
		pSizer = gEnv->pSystem->CreateSizer();

	for (TParentLayerMap::iterator it = parentLayers.begin(); it != parentLayers.end(); ++it)
	{
		const string& parentName = it->first;
		std::vector<CEntityLayer*>& children = it->second;

		CEntityLayer* pParent = FindLayer(parentName);

		bool bIsEnabled = false;
		if (bRenderAllLayerStats)
		{
			bIsEnabled = true;
		}
		else 
		{
			if (pParent)
				bIsEnabled = pParent->IsEnabledBrush();

			if (!bIsEnabled)
			{
				for (size_t i = 0; i < children.size(); ++i)
				{
					CEntityLayer* pChild = children[i];
					if (pChild->IsEnabledBrush())
					{
						bIsEnabled = true;
						break;
					}
				}
			}
		}	

		if (!bIsEnabled)
			continue;

		SLayerPakStats::SEntry* pLayerPakEntry = 0;
		for (SLayerPakStats::TEntries::iterator it2 = layerPakStats.m_entries.begin(); 
			it2 != layerPakStats.m_entries.end(); ++it2)
		{
			if (it2->name == parentName)
			{
				pLayerPakEntry = &(*it2);
				break;
			}
		}

		if (pLayerPakEntry)
		{
			DrawText( tx,ty+=ystep,clText, "%s (PakInfo - %1.1f MB - %s)", parentName.c_str(), 
				(float)pLayerPakEntry->nSize / (1024.f * 1024.f), pLayerPakEntry->status.c_str());
		}
		else
		{
			DrawText( tx,ty+=ystep,clText, "%s", parentName.c_str());
		}

		for (size_t i = 0; i < children.size(); ++i)
		{
			CEntityLayer* pChild = children[i];

			if (bRenderAllLayerStats)
			{
				const char* state = "enabled";
				ColorF clTextState(0, 1, 1, 1);
				if (pChild->IsEnabled() && !pChild->IsEnabledBrush())
				{
					// a layer was not disabled by Flowgraph in time when level is starting
					state = "was not disabled";
					clTextState = ColorF (1, 0.3f, 0.3f, 1); // redish
				}
				else if (!pChild->IsEnabled())
				{
					state = "disabled";
					clTextState = ColorF (0.7f, 0.7f, 0.7f, 1); // grayish
				}
				ty+=ystep;
				DrawText(tx, ty, clTextState, "  %s (%s)", pChild->GetName().c_str(), state);

				if (bRenderMemoryStats && pSizer)
				{
					pSizer->Reset();

					int numBrushes, numDecals;
					gEnv->p3DEngine->GetLayerMemoryUsage(pChild->GetId(), pSizer, &numBrushes, &numDecals);

					int numEntities;
					pChild->GetMemoryUsage(pSizer, &numEntities);
					const float memorySize = float(pSizer->GetTotalSize())/1024.f;

					const int kColumnPos = 350;
					if (numDecals)
 						DrawText(tx + kColumnPos, ty, clTextState, "Brushes: %d, Decals: %d, Entities: %d; Mem: %.2f Kb", numBrushes, numDecals, numEntities, memorySize);
					else
						DrawText(tx + kColumnPos, ty, clTextState, "Brushes: %d, Entities: %d; Mem: %.2f Kb", numBrushes, numEntities, memorySize);
				}
			}
			else if (pChild->IsEnabledBrush())
			{
				DrawText( tx,ty+=ystep,clText, "  %s", pChild->GetName().c_str());
			}
		}
	}

	if (pSizer)
		pSizer->Release();

#endif //ENABLE_PROFILING_CODE
}

//////////////////////////////////////////////////////////////////////////
ILINE IEntityClass * CEntitySystem::GetClassForEntity(CEntity * p)
{
	return p->m_pClass;
}

class CEntitySystem::CCompareEntityIdsByClass
{
public:
	CCompareEntityIdsByClass( CEntity ** ppEntities ) : m_ppEntities(ppEntities) {}
	
	ILINE bool operator()( EntityId a, EntityId b ) const
	{
		CEntity * pA = m_ppEntities[a&0xffff];
		CEntity * pB = m_ppEntities[b&0xffff];
		IEntityClass * pClsA = pA? GetClassForEntity(pA) : 0;
		IEntityClass * pClsB = pB? GetClassForEntity(pB) : 0;
		return pClsA < pClsB;
	}

private:
	CEntity ** m_ppEntities;
};

void CEntitySystem::UpdateTempActiveEntities()
{
	if (!m_tempActiveEntitiesValid)
	{
		int i = 0;
		int numActive = m_mapActiveEntities.size();
		m_tempActiveEntities.resize( numActive );
		for (EntitiesMap::iterator it = m_mapActiveEntities.begin(); it != m_mapActiveEntities.end(); ++it)
		{
			m_tempActiveEntities[i++] = it->first;
		}
		if (CVar::es_SortUpdatesByClass)
			std::sort( m_tempActiveEntities.begin(), m_tempActiveEntities.end(), CCompareEntityIdsByClass(&m_EntityArray[0]) );
		m_tempActiveEntitiesValid = true;
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::DoUpdateLoop(float fFrameTime)
{
	CCamera Cam=m_pISystem->GetViewCamera();
	int nRendererFrameID = 0;
	if(m_pISystem)
	{
		IRenderer *pRen=gEnv->pRenderer;
		if(pRen)
			nRendererFrameID=pRen->GetFrameID(false);
		//			nRendererFrameID=pRen->GetFrameUpdateID();
	}

	bool bDebug = false;
	if (CVar::pDebug->GetIVal())
		bDebug = true;

	bool bUpdateEntities=CVar::pUpdateEntities->GetIVal()?true:false;
	bool bProfileEntities = CVar::pProfileEntities->GetIVal() >= 1 || bDebug;
	bool bProfileEntitiesToLog = CVar::pProfileEntities->GetIVal() == 2;
	bool bProfileEntitiesAll = CVar::pProfileEntities->GetIVal() == 3;
	bool bProfileEntitiesDesigner = CVar::pProfileEntities->GetIVal() == 4;

	if (bProfileEntitiesAll)
		bProfileEntitiesToLog=true;

	SEntityUpdateContext ctx;
	ctx.nFrameID = nRendererFrameID;
	ctx.pCamera = &Cam;
	ctx.fCurrTime = gEnv->pTimer->GetCurrTime();
	ctx.fFrameTime = fFrameTime;
	ctx.bProfileToLog = bProfileEntitiesToLog;
	ctx.numVisibleEntities = 0;
	ctx.numUpdatedEntities = 0;
	ctx.fMaxViewDist = Cam.GetFarPlane();
	ctx.fMaxViewDistSquared = ctx.fMaxViewDist*ctx.fMaxViewDist;
	ctx.vCameraPos = Cam.GetPosition();

	if (!bProfileEntities)
	{	
		// Copy active entity ids into temporary buffer, this is needed because some entity can be added or deleted during Update call.
		UpdateTempActiveEntities();
		int numActive = m_tempActiveEntities.size();
		for (int i = 0; i < numActive; i++)
		{
			CEntity *pEntity = GetEntityFromID( m_tempActiveEntities[i] );
			if (pEntity)
			{
#ifdef _DEBUG
				INDENT_LOG_DURING_SCOPE(true, "While updating %s...", pEntity->GetEntityTextDescription());
#endif
				pEntity->Update( ctx );
			}
		}
	}
	else
	{		
		if (bProfileEntitiesToLog)
			CryLogAlways( "================= Entity Update Times =================" );

		char szProfInfo[256];
//		int prevNumUpdated;
		float fProfileStartTime;

		float xpos=10;
		float ypos=70;

		// test consistency before running updates
		CheckInternalConsistency();

		int nCounter=0;

		// Copy active entity ids into temporary buffer, this is needed because some entity can be added or deleted during Update call.
		int i = 0;
		UpdateTempActiveEntities();
		int numActive = m_tempActiveEntities.size();
		for (i = 0; i < numActive; i++)
		{
			CEntity *ce = GetEntityFromID( m_tempActiveEntities[i] );

			fProfileStartTime = gEnv->pTimer->GetAsyncCurTime();
//			prevNumUpdated = ctx.numUpdatedEntities;

			if (!ce || ce->m_bGarbage)
				continue;

#ifdef _DEBUG
			INDENT_LOG_DURING_SCOPE(true, "While updating %s...", ce->GetEntityTextDescription());
#endif
			ce->Update( ctx );
			ctx.numUpdatedEntities++;

			//if (prevNumUpdated != ctx.numUpdatedEntities || bProfileEntitiesAll)
			{
				float time = gEnv->pTimer->GetAsyncCurTime() - fProfileStartTime;
				if (time < 0)
					time = 0;

				float timeMs = time*1000.0f;
				if (bProfileEntitiesToLog || bProfileEntitiesDesigner)
				{
					bool bAIEnabled=false;
					bool bIsAI=false;

					if (IAIObject* aiObject = ce->GetAI())
					{
						bIsAI=true;
						if (!(bAIEnabled = aiObject->IsEnabled()))
							bAIEnabled = aiObject->GetProxy()->GetLinkedDriverEntityId() != 0;
					}

					CRenderProxy *pProxy=ce->GetRenderProxy();

					float fDiff=-1;
					if (pProxy)
					{
						fDiff=gEnv->pTimer->GetCurrTime() - pProxy->GetLastSeenTime();
						if (bProfileEntitiesToLog)
							CryLogAlways( "(%d) %.3f ms : %s (was last visible %0.2f seconds ago)-AI =%s (%s)",nCounter,timeMs,ce->GetEntityTextDescription(),fDiff,bIsAI?"":"NOT an AI",bAIEnabled?"true":"false" );
					}
					else
					{
						if (bProfileEntitiesToLog)
							CryLogAlways( "(%d) %.3f ms : %s -AI =%s (%s) ",nCounter,timeMs,ce->GetEntityTextDescription(),bIsAI?"":"NOT an AI",bAIEnabled?"true":"false" );
					}

					if (bProfileEntitiesDesigner && (pProxy && bIsAI))
					{
						if (((fDiff>180) && ce->HasAI()) || !bAIEnabled)
						{
							if ((fDiff>180) && ce->HasAI())
								sprintf(szProfInfo,"Entity: %s is updated, but is not visible for more then 3 min",ce->GetEntityTextDescription());
							else
								sprintf(szProfInfo,"Entity: %s is force being updated, but is not required by AI",ce->GetEntityTextDescription());
							float colors[4]={1,1,1,1};
							gEnv->pRenderer->Draw2dLabel( xpos,ypos,1.5f,colors,false,szProfInfo );
							ypos+=12;
						}
					}
				}

				DebugDraw( ce,timeMs );
			}
		} //it

		int nNumRenderable = 0;
		int nNumPhysicalize = 0;
		int nNumScriptable = 0;
		if (bDebug || bProfileEntitiesToLog)
		{
			// Draw the rest of not active entities
			uint32 dwMaxUsed = (uint32)m_EntitySaltBuffer.GetMaxUsed()+1;

			for(uint32 dwI=0;dwI<dwMaxUsed;++dwI)
			{
				CEntity *ce = m_EntityArray[dwI];

				if (!ce)
					continue;

				if (ce->GetProxy(ENTITY_PROXY_RENDER))
					nNumRenderable++;
				if (ce->GetProxy(ENTITY_PROXY_PHYSICS))
					nNumPhysicalize++;
				if (ce->GetProxy(ENTITY_PROXY_SCRIPT))
					nNumScriptable++;

				if (ce->m_bGarbage || ce->GetUpdateStatus())
					continue;

				DebugDraw( ce,0 );
			}
		}

		//ctx.numUpdatedEntities = (int)m_mapActiveEntities.size();

		int numEnts = GetNumEntities();
			
		if (bProfileEntitiesToLog)
		{
			CryLogAlways( "================= Entity Update Times =================" );
			CryLogAlways( "%d Entities Updated.",ctx.numUpdatedEntities );
			CryLogAlways( "%d Visible Entities Updated.",ctx.numVisibleEntities );
			CryLogAlways( "%d Active Entity Timers.",(int)m_timersMap.size() );			
			CryLogAlways("Entities: Total=%d, Active=%d, Renderable=%d, Phys=%d, Script=%d",numEnts,ctx.numUpdatedEntities,
				nNumRenderable,nNumPhysicalize,nNumScriptable );

			CVar::pProfileEntities->Set(0);

			if (bProfileEntitiesAll)
			{			
				uint32 dwRet=0;
				uint32 dwMaxUsed = (uint32)m_EntitySaltBuffer.GetMaxUsed()+1;

				for(uint32 dwI=0;dwI<dwMaxUsed;++dwI)
				{
					CEntity *ce = m_EntityArray[dwI];

					if(ce)
					{						
						CryLogAlways( "(%d) : %s ",dwRet,ce->GetEntityTextDescription());
						++dwRet;
					}
				} //dwI
				CVar::pProfileEntities->Set(0);
			}			

		}

		if (bDebug)
			sprintf(szProfInfo,"Entities: Total=%d, Active=%d, Renderable=%d, Phys=%d, Script=%d",numEnts,ctx.numUpdatedEntities,
			nNumRenderable,nNumPhysicalize,nNumScriptable );
		else
			sprintf(szProfInfo,"Entities: Total=%d Active=%d",numEnts,ctx.numUpdatedEntities );
		float colors[4]={1,1,1,1};
		gEnv->pRenderer->Draw2dLabel( 10,10,1.5,colors,false,szProfInfo );
	}
}


//////////////////////////////////////////////////////////////////////////
void CEntitySystem::CheckInternalConsistency() const
{
	// slow - code should be kept commented out unless specific problems needs to be tracked
	/*
	std::map<EntityId,CEntity*>::const_iterator it, end=m_mapActiveEntities.end();

	for(it=m_mapActiveEntities.begin(); it!=end; ++it)
	{
		CEntity *ce= it->second;
		EntityId id=it->first;

		CEntity *pSaltBufferEntitiyPtr = GetEntityFromID(id);

		assert(ce==pSaltBufferEntitiyPtr);		// internal consistency is broken
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
IEntityIt *CEntitySystem::GetEntityIterator()
{
	return (IEntityIt*)new CEntityItMap(this);
}

//////////////////////////////////////////////////////////////////////////
bool CEntitySystem::IsIDUsed( EntityId nID ) const
{
	return m_EntitySaltBuffer.IsUsed(nID);
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::SendEventViaEntityEvent( IEntity* piEntity, SEntityEvent &event )
{
	m_pEventDistributer->SendEvent( event );
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::SendEventToAll( SEntityEvent &event )
{
	uint32 dwMaxUsed = (uint32)m_EntitySaltBuffer.GetMaxUsed()+1;

	for(uint32 dwI=0;dwI<dwMaxUsed;++dwI)
	{
		CEntity *pEntity = m_EntityArray[dwI];

		if(pEntity)
			pEntity->SendEvent( event );
	}

	if(event.event == ENTITY_EVENT_RESET)
	{
		// This stuff is necessary because the reset event currently recreates 
		// the character instance and thus removes all attachment data previously set up.
		for(uint32 dwI=0;dwI<dwMaxUsed;++dwI)
		{
			CEntity *pEntity = m_EntityArray[dwI];

			if(pEntity)
			{
				// Restore the bone attachment by the deprecated CharAttachHelper.
				if(strcmp(pEntity->GetClass()->GetName(), "CharacterAttachHelper") == 0
					&& pEntity->GetParent())
				{
					SEntityEvent attachEvent(ENTITY_EVENT_ATTACH_THIS);   
					attachEvent.nParam[0] = pEntity->GetParent()->GetId(); 
					pEntity->SendEvent(attachEvent);
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::GetMemoryStatistics(ICrySizer *pSizer) const
{
	pSizer->AddContainer(m_guidMap);
	pSizer->AddObject(m_pClassRegistry);
	pSizer->AddContainer(m_mapEntityNames);
	
	{
		SIZER_COMPONENT_NAME(pSizer,"Entities");
		pSizer->AddObject( m_EntityArray );
		pSizer->AddContainer( m_deletedEntities );
		pSizer->AddContainer( m_mapActiveEntities );
	}
	
	for (uint i = 0; i < SinkMaxEventSubscriptionCount; ++i)
		pSizer->AddContainer(m_sinks[i]);
	pSizer->AddObject(m_pPartitionGrid)	;	
	pSizer->AddObject(m_pProximityTriggerSystem);
	pSizer->AddObject(this, sizeof(*this));

	{
		SIZER_COMPONENT_NAME(pSizer,"EntityPool");
		m_pEntityPoolManager->GetMemoryStatistics(pSizer);
	}

	{
		SIZER_COMPONENT_NAME(pSizer,"EntityLoad");
		m_pEntityLoadManager->GetMemoryStatistics(pSizer);
	}

#ifndef _LIB // Only when compiling as dynamic library
	{
		//SIZER_COMPONENT_NAME(pSizer,"Strings");
		//pSizer->AddObject( (this+1),string::_usedMemory(0) );
	}
	{
		SIZER_COMPONENT_NAME(pSizer,"STL Allocator Waste");
		CryModuleMemoryInfo meminfo;
		ZeroStruct(meminfo);
		CryGetMemoryInfoForModule( &meminfo );
		pSizer->AddObject( (this+2),(size_t)meminfo.STL_wasted );
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::AddTimerEvent( SEntityTimerEvent &event, CTimeValue startTime )
{
	CTimeValue millis;
	millis.SetMilliSeconds(event.nMilliSeconds);
	CTimeValue nTriggerTime = startTime + millis;
	m_timersMap.insert( EntityTimersMap::value_type(nTriggerTime,event) );


	if (CVar::es_DebugTimers)
	{
		CEntity *pEntity = GetEntityFromID(	event.entityId );
		if (pEntity)
			CryLogAlways( "SetTimer (timerID=%d,time=%dms) for Entity %s",event.nTimerId,event.nMilliSeconds,pEntity->GetEntityTextDescription() );
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::RemoveTimerEvent( EntityId id,int nTimerId )
{
	if (nTimerId < 0)
	{
		// Delete all timers of this entity.
		EntityTimersMap::iterator next;
		for (EntityTimersMap::iterator it = m_timersMap.begin(); it != m_timersMap.end(); it = next)
		{
			next = it; ++next;
			if (id == it->second.entityId)
			{
				// Remove this item.
				m_timersMap.erase( it ); 
			}
		}
	}
	else
	{
		for (EntityTimersMap::iterator it = m_timersMap.begin(); it != m_timersMap.end(); ++it)
		{
			if (id == it->second.entityId && nTimerId == it->second.nTimerId)
			{
				// Remove this item.
				m_timersMap.erase( it ); 
				break;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::PauseTimers(bool bPause,bool bResume)
{	
	m_bTimersPause=bPause;	
	if (bResume)
	{
		m_nStartPause.SetSeconds(-1.0f);
		return; // just allow timers to be updated next time
	}

	if (bPause)
	{ 
		// record when timers pause was called
		m_nStartPause = gEnv->pTimer->GetFrameStartTime();
	}
	else if (m_nStartPause > CTimeValue(0.0f))
	{		
		// increase the timers by adding the delay time passed since when
		// it was paused
		CTimeValue nCurrTimeMillis = gEnv->pTimer->GetFrameStartTime();
		CTimeValue nAdditionalTriggerTime = nCurrTimeMillis-m_nStartPause;
		
		EntityTimersMap::iterator it;
		EntityTimersMap lstTemp;

		for (it = m_timersMap.begin();it!=m_timersMap.end();++it)
		{
			lstTemp.insert( EntityTimersMap::value_type(it->first,it->second) );			
		} //it

		m_timersMap.clear();

		for (it = lstTemp.begin();it!=lstTemp.end();++it)
		{
			CTimeValue nUpdatedTimer = nAdditionalTriggerTime + it->first;
			m_timersMap.insert( EntityTimersMap::value_type(nUpdatedTimer,it->second) );			
		} //it	

		m_nStartPause.SetSeconds(-1.0f);
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::UpdateTimers()
{
	if (m_timersMap.empty())
		return;

	FUNCTION_PROFILER_FAST( m_pISystem,PROFILE_ENTITY,g_bProfilerEnabled );

	CTimeValue nCurrTimeMillis = gEnv->pTimer->GetFrameStartTime();

	// Iterate thru all matching timers.
	EntityTimersMap::iterator first = m_timersMap.begin();
	EntityTimersMap::iterator last = m_timersMap.upper_bound( nCurrTimeMillis );
	if (last != first)
	{
		// Make a separate list, because OnTrigger call can modify original timers map.
		m_currentTimers.resize(0);
		m_currentTimers.reserve(10); 
		
		for (EntityTimersMap::iterator it = first; it != last; ++it)
			m_currentTimers.push_back(it->second); 

		// Delete these items from map.
		m_timersMap.erase(first, last);

		//////////////////////////////////////////////////////////////////////////
		// Execute OnTimer events.

		EntityTimersVector::iterator it = m_currentTimers.begin();
		EntityTimersVector::iterator end = m_currentTimers.end();

		SEntityEvent entityEvent;
		entityEvent.event = ENTITY_EVENT_TIMER;

		for ( ; it != end; ++it)
		{
			SEntityTimerEvent &event = *it;

			if (CEntity *pEntity = (CEntity*)GetEntity(event.entityId))
			{
				// Send Timer event to the entity.
				entityEvent.nParam[0] = event.nTimerId;
				entityEvent.nParam[1] = event.nMilliSeconds;
				entityEvent.nParam[2] = event.entityId;
				pEntity->SendEvent(entityEvent);

				if (CVar::es_DebugTimers)
				{
					if (pEntity)
						CryLogAlways( "OnTimer Event (timerID=%d,time=%dms) for Entity %s (which is %s)",event.nTimerId,event.nMilliSeconds,pEntity->GetEntityTextDescription(), pEntity->IsActive() ? "active" : "inactive" );
				}
			}
		}
	}
} 


//////////////////////////////////////////////////////////////////////////
void CEntitySystem::ReserveEntityId( const EntityId id )
{
	assert(id);

	const CSaltHandle<> hdl = IdToHandle(id);
	if (m_EntitySaltBuffer.IsUsed(hdl.GetIndex())) // Do not reserve if already used.
		return;
	//assert(m_EntitySaltBuffer.IsUsed(hdl.GetIndex()) == false);	// don't reserve twice or overriding in used one

	m_EntitySaltBuffer.InsertKnownHandle(hdl);
}

//////////////////////////////////////////////////////////////////////////
EntityId CEntitySystem::ReserveUnknownEntityId()
{
	return GenerateEntityId(false);
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::ActivateEntity( CEntity *pEntity,bool bActivate )
{
	assert(pEntity==GetEntityFromID(pEntity->GetId()));

	if (bActivate)
	{
		m_mapActiveEntities[pEntity->GetId()] = pEntity;

		if (!pEntity->m_bInActiveList)
		{
			pEntity->m_bInActiveList = true;
			SEntityEvent event(ENTITY_EVENT_ACTIVATED);
			pEntity->SendEvent(event);
		}
	}
	else
	{
		m_mapActiveEntities.erase(pEntity->GetId());
		
		if (pEntity->m_bInActiveList)
		{
			pEntity->m_bInActiveList = false;
			SEntityEvent event(ENTITY_EVENT_DEACTIVATED);
			pEntity->SendEvent(event);
		}
	}

	m_tempActiveEntitiesValid = false;
}

void CEntitySystem::ActivatePrePhysicsUpdateForEntity( CEntity *pEntity,bool bActivate )
{
	const EntityId entityID = pEntity->GetId();
	if (bActivate)
	{
		if( m_mapPrePhysicsEntities.count( entityID )==0 )
		{
			m_pEventDistributer->EnableEventForEntity( pEntity->GetId(), ENTITY_EVENT_PREPHYSICSUPDATE, true );
		}
		m_mapPrePhysicsEntities.insert(entityID);
	}
	else
	{
		if( m_mapPrePhysicsEntities.count( entityID )>0 )
		{
			m_pEventDistributer->EnableEventForEntity( pEntity->GetId(), ENTITY_EVENT_PREPHYSICSUPDATE, false );
		}
		m_mapPrePhysicsEntities.erase( entityID );
	}

}

bool CEntitySystem::IsPrePhysicsActive( CEntity * pEntity )
{
	return m_mapPrePhysicsEntities.find(pEntity->GetId()) != m_mapPrePhysicsEntities.end();
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::OnEntityEvent( CEntity *pEntity,SEntityEvent &event )
{
	EntitySystemOnEventSinks::const_iterator oeIt = m_onEventSinks.begin();
	EntitySystemOnEventSinks::const_iterator oeEnd = m_onEventSinks.end();

	for ( ; oeIt != oeEnd; ++oeIt)
	{
		const OnEventSink& sink = *oeIt;
		if (sink.subscriptions & (1ll << event.event))
			sink.pSink->OnEvent( pEntity, event );
	}

	// Just an optimization, most entities do not have event listeners.
	if (pEntity->m_bHaveEventListeners)
	{
		// Copy listeners to local list
		// This is a local workaround for the issue where listeners where removed from the container 
		// in the listener->OnEntityEvent() while iterating over the container, corrupting the iterator
		//
		// Note: This assumes that listeners that have been removed from the container 
		// and are in the local list are still valid
		// Copy to local list
		uint32 nNumElements = 0;
		EventListenersMap &entitiesMap = m_eventListeners[event.event];
		EventListenersMap::iterator itBegin = entitiesMap.lower_bound(pEntity->m_nID);		
		EventListenersMap::iterator itEnd = entitiesMap.end();		

		// Count num elements
		EventListenersMap::iterator it = itBegin;		
		while (it != itEnd && it->first == pEntity->m_nID)
		{
			++nNumElements;
			++it;
		}

		if(nNumElements > 0)
		{
			// Allocate listener list on stack
			IEntityEventListener** ppListeners = (IEntityEventListener**)alloca(sizeof(IEntityEventListener*) * nNumElements);

			// Copy all listener pointers to list on stack
			it = itBegin;
			for(uint32 i = 0; i < nNumElements; ++i)
			{
				ppListeners[i] = it->second;
				++it;
			}
			
			// Iterate over all listeners
			for(uint32 i = 0; i < nNumElements; ++i)
			{
				IEntityEventListener* pListener = ppListeners[i];
				ENTITY_EVENT_ENTITY_LISTENER(pListener);

				if(pListener)
					pListener->OnEntityEvent(pEntity,event);
			}
		}
	}	
}

//////////////////////////////////////////////////////////////////////////
bool CEntitySystem::OnLoadLevel(const char* szLevelPath)
{
	int nTerrainSize = gEnv->p3DEngine ? gEnv->p3DEngine->GetTerrainSize() : 0;
	ResizeProximityGrid(nTerrainSize, nTerrainSize);

	assert(m_pEntityPoolManager);
	return m_pEntityPoolManager->OnLoadLevel(szLevelPath);
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::OnLevelLoadStart()
{
	assert(m_pEntityPoolManager);
	m_pEntityPoolManager->OnLevelLoadStart();
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::LoadEntities( XmlNodeRef &objectsNode, bool bIsLoadingLevelFile )
{
	LOADING_TIME_PROFILE_SECTION;
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "LoadEntities");

	//Update loading screen and important tick functions
	SYNCHRONOUS_LOADING_TICK();

	assert(m_pEntityPoolManager);
	if (!m_pEntityPoolManager->CreatePools())
	{
		EntityWarning("CEntitySystem::LoadEntities : Failed when prepairing the Entity Pools.");
	}

	//Update loading screen and important tick functions
	SYNCHRONOUS_LOADING_TICK();

	assert(m_pEntityLoadManager);
	if (!m_pEntityLoadManager->LoadEntities(objectsNode, bIsLoadingLevelFile))
	{
		EntityWarning("CEntitySystem::LoadEntities : Not all entities were loaded correctly.");
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::LoadEntities( XmlNodeRef &objectsNode, bool bIsLoadingLevelFile, const Vec3 &segmentOffset, std::vector<IEntity *> *outGlobalEntityIds, std::vector<IEntity *> *outLocalEntityIds )
{
	assert(m_pEntityLoadManager);
	if (!m_pEntityLoadManager->LoadEntities(objectsNode, bIsLoadingLevelFile, segmentOffset, outGlobalEntityIds, outLocalEntityIds))
	{
		EntityWarning("CEntitySystem::LoadEntities : Not all entities were loaded correctly.");
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::HoldLayerEntities( const char* pLayerName )
{
	m_pEntityLoadManager->HoldLayerEntities(pLayerName);
}

void CEntitySystem::CloneHeldLayerEntities( const char* pLayerName, const Vec3& localOffset, const Matrix34& l2w, const char** pExcludeLayers, int numExcludeLayers )
{
	m_pEntityLoadManager->CloneHeldLayerEntities(pLayerName, localOffset, l2w, pExcludeLayers, numExcludeLayers);
}

void CEntitySystem::ReleaseHeldEntities()
{
	m_pEntityLoadManager->ReleaseHeldEntities();
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::AddEntityEventListener( EntityId nEntity,EEntityEvent event,IEntityEventListener *pListener )
{
	CEntity *pCEntity = (CEntity*)GetEntity( nEntity );
	if (pCEntity)
	{
		pCEntity->m_bHaveEventListeners = true; // Just an optimization, most entities dont have it.
		m_eventListeners[event].insert( EventListenersMap::value_type(nEntity,pListener) );
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::RemoveEntityEventListener( EntityId nEntity,EEntityEvent event,IEntityEventListener *pListener )
{
	EventListenersMap &entitiesMap = m_eventListeners[event];
	EventListenersMap::iterator it = entitiesMap.lower_bound(nEntity);
	while (it != entitiesMap.end() && it->first == nEntity)
	{
		if (pListener == it->second)
		{
			entitiesMap.erase(it);
			break;
		}
		++it;
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::RemoveEntityEventListeners( CEntity *pEntity )
{
	assert(pEntity);
	EntityId nEntity = pEntity->GetId();

	for (int i = 0; i < ENTITY_EVENT_LAST; i++)
	{
		EventListenersMap &entitiesMap = m_eventListeners[i];
		EventListenersMap::iterator it = entitiesMap.lower_bound(nEntity);
		while (it != entitiesMap.end() && it->first == nEntity)
		{
			EventListenersMap::iterator itThis = it++;
			entitiesMap.erase(itThis);
		}
	}

	pEntity->m_bHaveEventListeners = false; 
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::DebugDraw( CEntity *ce,float timeMs )
{
	char szProfInfo[256];

	IRenderer *pRender = gEnv->pRenderer;
	IRenderAuxGeom *pRenderAux = pRender->GetIRenderAuxGeom();

	Vec3 wp = ce->GetWorldTM().GetTranslation();

	if (wp.IsZero())
		return;

	//float z = 1.0f / max(0.01f,wp.GetDistance(gEnv->pSystem->GetViewCamera().GetPosition()) );
	//float fFontSize = 

	ColorB boundsColor;

	if(CVar::es_debugDrawEntityIDs == 0)
	{
		// draw information about timing, physics, position, and textual representation of the entity (but no EntityId)

		if (ce->GetUpdateStatus() && timeMs >= 0)
		{
			float colors[4]={1,1,1,1};
			float colorsYellow[4]={1,1,0,1};
			float colorsRed[4]={1,0,0,1};

			CRenderProxy *pProxy=ce->GetRenderProxy();
			if (pProxy)
				sprintf(szProfInfo, "%.3f ms : %s (%0.2f ago)",timeMs,ce->GetEntityTextDescription(),gEnv->pTimer->GetCurrTime() - pProxy->GetLastSeenTime() );
			else
				sprintf(szProfInfo,"%.3f ms : %s",timeMs,ce->GetEntityTextDescription() );
			if (timeMs > 0.5f)
				pRender->DrawLabelEx(wp,1.3f,colorsYellow,true,true,szProfInfo);
			else if (timeMs > 1.0f)
				pRender->DrawLabelEx(wp,1.6f,colorsRed,true,true,szProfInfo);
			else
			{
				pRender->DrawLabelEx(wp,1.1f,colors,true,true,szProfInfo);

				if( ce->GetPhysicalProxy() && ce->GetPhysicalProxy()->GetPhysicalEntity() )
				{
					pe_status_pos pe;
					ce->GetPhysicalProxy()->GetPhysicalEntity()->GetStatus( &pe );
					sprintf( szProfInfo, "Physics: %8.5f %8.5f %8.5f", pe.pos.x, pe.pos.y, pe.pos.z );
					wp.z-= 0.1f;
					pRender->DrawLabelEx(wp,1.1f,colors,true,true,szProfInfo);
				}

				Vec3 entPos = ce->GetPos();
				sprintf( szProfInfo, "Entity:  %8.5f %8.5f %8.5f", entPos.x, entPos.y, entPos.z );
				wp.z-= 0.1f;
				pRender->DrawLabelEx(wp,1.1f,colors,true,true,szProfInfo);
			}
		}
		else
		{
			float colors[4]={1,1,1,1};
			pRender->DrawLabelEx( wp,1.2f,colors,true,true,ce->GetEntityTextDescription() );
		}

		boundsColor.set(255, 255, 0, 255);
	}
	else
	{
		// draw only the EntityId (no other information, like position, physics status, etc.)

		static const ColorF colorsToChooseFrom[] =
		{
			Col_Green,
			Col_Blue,
			Col_Red,
			Col_Yellow,
			Col_Gray,
			Col_Orange,
			Col_Pink,
			Col_Cyan,
			Col_Magenta
		};

		char textToRender[256];
		sprintf(textToRender, "EntityId: %u", ce->GetId());
		const ColorF &color = colorsToChooseFrom[ce->GetId() % ARRAY_COUNT(colorsToChooseFrom)];

		// Draw text.
		float colors[4]= { color.r, color.g, color.b, 1.0f };
		gEnv->pRenderer->DrawLabelEx(ce->GetPos(), 1.2f, colors, true, true, textToRender);

		// specify color for drawing bounds below
		boundsColor.set((uint8)(color.r*255.0f), (uint8)(color.g*255.0f), (uint8)(color.b*255.0f), 255);
	}

	// Draw bounds.
	AABB box;
	ce->GetLocalBounds( box );
	if (box.min == box.max)
	{
		box.min = wp - Vec3(0.1f,0.1f,0.1f);
		box.max = wp + Vec3(0.1f,0.1f,0.1f);
	}

	pRenderAux->DrawAABB( box,ce->GetWorldTM(),false,boundsColor, eBBD_Extremes_Color_Encoded );
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::RegisterEntityGuid( const EntityGUID &guid,EntityId id )
{
	m_guidMap.insert( EntityGuidMap::value_type(guid,id) );
	CEntity* pCEntity = (CEntity*)GetEntity( id );
	if (pCEntity)
		pCEntity->m_guid = guid;
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::UnregisterEntityGuid( const EntityGUID &guid )
{
	m_guidMap.erase(guid);
}

//////////////////////////////////////////////////////////////////////////
EntityId CEntitySystem::FindEntityByGuid( const EntityGUID &guid ) const
{
	if (CVar::es_DebugFindEntity != 0)
	{
#if defined(_MSC_VER)
		CryLog( "FindEntityByGuid: %I64X",guid );
#else
		CryLog( "FindEntityByGuid: %llX",(long long)guid );
#endif
	}
	return stl::find_in_map(m_guidMap,guid,0);
}

//////////////////////////////////////////////////////////////////////////
EntityId CEntitySystem::FindEntityByEditorGuid( const char* pGuid ) const
{
	return(m_pEntityLoadManager->FindEntityByEditorGuid(pGuid));
}

//////////////////////////////////////////////////////////////////////////
EntityId CEntitySystem::GenerateEntityIdFromGuid( const EntityGUID &guid )
{
	if(!m_bEntitiesUseGUIDs)
		return INVALID_ENTITYID;

	EntityGuidMap::iterator it = m_genIdMap.find(guid);
	if (it == m_genIdMap.end())
	{
		EntityId newId = m_nGeneratedFromGuid++;
		m_genIdMap[guid] = newId;
		return newId;
	}
	else
	{
		return it->second;
	}
}

//////////////////////////////////////////////////////////////////////////
IEntityArchetype* CEntitySystem::LoadEntityArchetype( const char *sArchetype )
{
	return m_pEntityArchetypeManager->LoadArchetype( sArchetype );
}
//////////////////////////////////////////////////////////////////////////
IEntityArchetype* CEntitySystem::LoadEntityArchetype( XmlNodeRef oArchetype)
{
	IEntityArchetype*	piArchetype(NULL);

	if (!oArchetype)
	{
		return NULL;
	}

	if (oArchetype->isTag("EntityPrototype"))
	{
		const char *name = oArchetype->getAttr( "Name" );
		const char *library = oArchetype->getAttr( "Library" );

		string strArchetypename;

		strArchetypename.Format("%s.%s",library,name);
		piArchetype=m_pEntityArchetypeManager->FindArchetype(strArchetypename);

		if (!piArchetype)
		{
			const char* className = oArchetype->getAttr( "Class" );

			IEntityClass *pClass = m_pClassRegistry->FindClass( className );
			if (!pClass)
			{
				// No such entity class.
				EntityWarning( "EntityArchetype %s references unknown entity class %s",strArchetypename.c_str(),className );
				return NULL;
			}

			piArchetype=m_pEntityArchetypeManager->CreateArchetype( pClass,strArchetypename );
		}
	}

	if (piArchetype != NULL)
	{
		// We need to pass the properties node to the LoadFromXML
		XmlNodeRef propNode = oArchetype->findChild("Properties");
		XmlNodeRef objectVarsNode = oArchetype->findChild("ObjectVars");
		if (propNode != NULL)
		{
			piArchetype->LoadFromXML(propNode, objectVarsNode);
		}
	}

	return piArchetype;
}
//////////////////////////////////////////////////////////////////////////
IEntityArchetype* CEntitySystem::CreateEntityArchetype( IEntityClass *pClass,const char *sArchetype )
{
	return m_pEntityArchetypeManager->CreateArchetype( pClass,sArchetype );
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::Serialize(TSerialize ser)
{
	if(ser.GetSerializationTarget() != eST_Network)
	{
		ser.BeginGroup("Timers");

		ser.Value("Paused", m_bTimersPause);
		ser.Value("PauseStart", m_nStartPause);

		int count = m_timersMap.size();
		ser.Value("timerCount", count);

		SEntityTimerEvent tempEvent;
		if(ser.IsWriting())
		{
			EntityTimersMap::iterator it;
			for(it = m_timersMap.begin(); it != m_timersMap.end(); ++it)
			{
				tempEvent = it->second;

				ser.BeginGroup("Timer");
				ser.Value("entityID", tempEvent.entityId);
				ser.Value("eventTime", tempEvent.nMilliSeconds);
				ser.Value("timerID", tempEvent.nTimerId);
				CTimeValue start = it->first; 
				ser.Value("startTime", start);
				ser.EndGroup();
			}
		}
		else
		{
			m_timersMap.clear();

			CTimeValue start;
			for(int c = 0; c < count; ++c)
			{
				ser.BeginGroup("Timer");
				ser.Value("entityID", tempEvent.entityId);
				ser.Value("eventTime", tempEvent.nMilliSeconds);
				ser.Value("timerID", tempEvent.nTimerId);
				ser.Value("startTime", start);
				ser.EndGroup();
				start.SetMilliSeconds((int64)(start.GetMilliSeconds()-tempEvent.nMilliSeconds));
				AddTimerEvent(tempEvent, start);

				//assert(GetEntity(tempEvent.entityId));
			}
		}

		ser.EndGroup();

		if (gEnv->pScriptSystem)
		{
			gEnv->pScriptSystem->SerializeTimers( GetImpl(ser) );
		}

		ser.BeginGroup("Layers");
    TLayerActivationOpVec deferredLayerActivations; 

		for(TLayers::const_iterator iLayer = m_layers.begin(), iEnd = m_layers.end(); iLayer != iEnd; ++ iLayer)
			iLayer->second->Serialize(ser, deferredLayerActivations);

    if (ser.IsReading() && deferredLayerActivations.size())
    {
      std::sort(deferredLayerActivations.begin(), deferredLayerActivations.end(), LayerActivationPriority);

      for (std::size_t i=deferredLayerActivations.size(); i>0; i--)
      {
        SPostSerializeLayerActivation &op = deferredLayerActivations[i-1]; 
        assert (op.m_layer && op.m_func);
        (op.m_layer->*op.m_func)(op.enable);
      }
    }
    else if (deferredLayerActivations.size())
      CryFatalError("added a deferred layer activation on writing !?"); 


		ser.EndGroup();
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::ResizeProximityGrid( int nWidth,int nHeight )
{
	if (m_pPartitionGrid)
		m_pPartitionGrid->AllocateGrid( (float)nWidth,(float)nHeight );
}


//////////////////////////////////////////////////////////////////////////
void CEntitySystem::SetNextSpawnId(EntityId id)
{
	if(id)
		RemoveEntity(id,true);
	m_idForced = id;
}

void CEntitySystem::ResetAreas()
{
	m_pAreaManager->ResetAreas();
	CAreaProxy::ResetTempState();
}

void CEntitySystem::UnloadAreas()
{
	ResetAreas();
	m_pAreaManager->UnloadAreas();
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::DumpEntities()
{
  IEntityItPtr it	= GetEntityIterator();
  it->MoveFirst();

  int count = 0;
  int acount = 0;

  CryLogAlways("--------------------------------------------------------------------------------");
  while(IEntity *pEntity = it->Next())
  {
    DumpEntity(pEntity);

    if (pEntity->IsActive())
      ++acount;

    ++count;
  }
  CryLogAlways("--------------------------------------------------------------------------------");
  CryLogAlways(" %d entities (%d active)", count, acount);
  CryLogAlways("--------------------------------------------------------------------------------");
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::ResumePhysicsForSuppressedEntities(bool bWakeUp)
{
	IEntityItPtr it	= GetEntityIterator();
	it->MoveFirst();

	while(IEntity *pEntity = it->Next())
	{
		if (pEntity->CheckFlags(ENTITY_FLAG_IGNORE_PHYSICS_UPDATE))
		{
			pEntity->ClearFlags(ENTITY_FLAG_IGNORE_PHYSICS_UPDATE);

			// Recreate the physics object
			pEntity->EnablePhysics(false);
			pEntity->EnablePhysics(true);

			IPhysicalEntity* pPhysEnt = pEntity->GetPhysics();
			if (NULL != pPhysEnt)
			{
				// Sync position of the entity with physical entity 
				pe_params_pos params;
				params.pos = pEntity->GetPos();
				params.q = pEntity->GetRotation();
				pPhysEnt->SetParams(&params);

				// Only need to wake up sleeping entities
				if (bWakeUp)
				{
					pe_status_awake status;
					if (pPhysEnt->GetStatus(&status) == 0)
					{
						pe_action_awake wake;
						wake.bAwake = 1;
						pPhysEnt->Action(&wake);
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::SaveInternalState(IDataWriteStream& writer) const
{
	// entities are exported in CObjectManager in editor
}

void CEntitySystem::LoadInternalState(IDataReadStream& reader)
{
	// get layers in the level
	std::vector<CEntityLayer*> allLayers;
	for (TLayers::iterator it=m_layers.begin(); 
		it != m_layers.end(); ++it)
	{
		CEntityLayer* pLayer = it->second;
		allLayers.push_back(pLayer);
	}

	// enable layers that are explicitly listed 
	const uint32 numLayers = reader.ReadUint32();
	for (uint32 i=0; i<numLayers; ++i)
	{
		const string layerName = reader.ReadString();

		// find local layer
		CEntityLayer* pLayer = NULL;
		for (TLayers::iterator it=m_layers.begin(); 
			it != m_layers.end(); ++it)
		{
			if (it->second->GetName() == layerName)
			{
				pLayer = it->second;
				break;
			}
		}

		// No layer found
		if (NULL == pLayer)
		{
			gEnv->pLog->LogAlways("Layer with name '%s' not found in local data",
				layerName.c_str());
			continue;
		}

		// remove from the "all layers" list - what's left will be hidden
		std::vector<CEntityLayer*>::iterator it = std::find(allLayers.begin(), allLayers.end(), pLayer);
		if (it != allLayers.end())
		{
			allLayers.erase(it);
		}

		pLayer->Enable(true,false,false); // NON RECURSIVE

		// show/hide the objects also
		if (pLayer->GetId() != 0)
		{
			gEnv->p3DEngine->ActivateObjectsLayer(pLayer->GetId(), true, true, true, true, pLayer->GetName(), NULL, false);
		}
	}

	// Hide rest of the layers
	for (std::vector<CEntityLayer*>::const_iterator it = allLayers.begin();
		it != allLayers.end(); ++it)
	{
		CEntityLayer* pLayer = *it;
		pLayer->Enable(false,false,false); // NON RECURSIVE

		// show/hide the objects also
		if (pLayer->GetId() != 0)
		{
			gEnv->p3DEngine->ActivateObjectsLayer(pLayer->GetId(), false, true, true, true, pLayer->GetName(), NULL, false);
		}
	}

	// Load entities data
	{
		// get current entities on the level
		typedef std::set<EntityGUID> TEntitySet;
		TEntitySet currentEntities;
		for (uint32 i=0; i<m_EntityArray.size(); ++i)
		{
			CEntity* pEntity = m_EntityArray[i];
			if (NULL != pEntity)
			{
				gEnv->pLog->LogAlways("ExistingEntity: '%s' '%s' (ID=%d, GUID=%llu, Flags=0x%X) %i", 
					pEntity->GetName(), pEntity->GetClass()->GetName(), 
					pEntity->GetId(), pEntity->GetGuid(),
					pEntity->GetFlags(), pEntity->IsLoadedFromLevelFile() );

				currentEntities.insert(pEntity->GetGuid());
			}
		}

		// create update packet
		XmlNodeRef xmlRoot = gEnv->pSystem->CreateXmlNode("Entities");

		// parse the update packet
		const uint32 numEntities = reader.ReadUint32();
		for (uint32 i=0; i<numEntities; ++i)
		{
			// load entity ID
			const EntityGUID entityGUID = reader.ReadUint64();

			// get existing entity with that ID
			TEntitySet::iterator it = currentEntities.find(entityGUID);
			if (it != currentEntities.end())
			{	
				currentEntities.erase(it);
			}

			// find entity in the current file
			EntityId entityId = FindEntityByGuid(entityGUID);
			IEntity* pEntity = GetEntity(entityId);
			if (NULL != pEntity)
			{
				// this is not an entity loaded from LevelFile, we cant override it!
				if (!pEntity->IsLoadedFromLevelFile())
				{
					// entity sill there :)
					gEnv->pLog->LogAlways("Could not override entity '%s' '%s' (ID=%d, GUID=%llu, Flags=0x%X). Original entity is non from Level.",
						pEntity->GetName(), pEntity->GetClass()->GetName(), 
						pEntity->GetId(), pEntity->GetGuid(), pEntity->GetFlags() );

					reader.SkipString();
					continue;
				}

				// entity does already exist, delete it (we will be restoring a new version)
				pEntity->ClearFlags(ENTITY_FLAG_UNREMOVABLE);
				gEnv->pEntitySystem->RemoveEntity(entityId);
			}

			// load this entity
			const string xmlString = reader.ReadString();
			XmlNodeRef xml = gEnv->pSystem->LoadXmlFromBuffer(xmlString.c_str(), xmlString.length());
			if (NULL != xml)
			{
				xmlRoot->addChild(xml);
			}
		}

		// delete entities that were not in the sync packet
		if (!currentEntities.empty())
		{
			for (TEntitySet::iterator it = currentEntities.begin();
				it != currentEntities.end(); ++it)
			{
				const EntityGUID entityGUID = *it;

				const EntityId entityId = FindEntityByGuid(entityGUID);
				IEntity* pEntity = GetEntity(entityId);
				if (NULL != pEntity)
				{
					// we can't delete local player
					if (pEntity->CheckFlags(ENTITY_FLAG_LOCAL_PLAYER))
					{
						continue;
					}

					// we can't delete entities that were not loaded from file
					if (!pEntity->IsLoadedFromLevelFile())
					{
						continue;
					}

					// special case for GameRules...
					if (0 == strcmp(pEntity->GetName(),"GameRules"))
					{
						continue;
					}

					gEnv->pLog->LogAlways("Deleting entity: '%s' '%s' (ID=%d, GUID=%llu, Flags=0x%X", pEntity->GetName(), pEntity->GetClass()->GetName(), 
						pEntity->GetId(), pEntity->GetGuid(), pEntity->GetFlags());

					// Remove the entity
					pEntity->ClearFlags(ENTITY_FLAG_UNREMOVABLE);
					RemoveEntity(entityId);
				}
			}
		}

		// finish entity deletions
		DeletePendingEntities();

		// load new entities
		LoadEntities(xmlRoot, true); // when loading LiveCreate entities we assume that they come from level
	}
}

//////////////////////////////////////////////////////////////////////////
int CEntitySystem::GetLayerId(const char* szLayerName) const
{
	std::map<string,CEntityLayer*>::const_iterator found = m_layers.find(CONST_TEMP_STRING(szLayerName));
	if (found!=m_layers.end())
	{
		return found->second->GetId();
	}
	else
	{
		return -1;
	}
}

//////////////////////////////////////////////////////////////////////////
const char* CEntitySystem::GetLayerName(int layerId) const
{
	std::map<string,CEntityLayer*>::const_iterator it = m_layers.begin();
	for (; it != m_layers.end(); ++it)
	{
		if (it->second->GetId() == layerId)
			return it->first.c_str();
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
int CEntitySystem::GetLayerChildCount(const char* szLayerName) const
{
	std::map<string,CEntityLayer*>::const_iterator found = m_layers.find(CONST_TEMP_STRING(szLayerName));
	if (found!=m_layers.end())
	{
		return found->second->GetNumChildren();
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
const char* CEntitySystem::GetLayerChild(const char* szLayerName, int childIdx) const
{
	std::map<string,CEntityLayer*>::const_iterator found = m_layers.find(CONST_TEMP_STRING(szLayerName));
	if (found!=m_layers.end())
	{
		CEntityLayer* pChildLayer = found->second->GetChild(childIdx);
		if (pChildLayer != NULL)
			return pChildLayer->GetName().c_str();
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::ToggleLayerVisibility(const char* layer, bool isEnabled, bool includeParent)
{
	CEntityLayer* pLayer = FindLayer(layer);
	if(pLayer)
	{
		const bool bChange = pLayer->IsEnabled() != isEnabled;
		if (bChange)
		{
			// call the Enable() with a special set of parameters for faster operation (LiveCreate specific)
			pLayer->Enable(isEnabled, false, false);

			// load/unload layer paks
			IResourceManager* pResMan = gEnv->pSystem->GetIResourceManager();
			if (pResMan)
			{
				if (isEnabled)
				{
					if (includeParent && (pLayer->GetParentName().length() > 0))
					{
						pResMan->LoadLayerPak(pLayer->GetParentName().c_str());
					}
					else
					{
						pResMan->LoadLayerPak(pLayer->GetName().c_str());
					}
				}
				else
				{
					if (includeParent && pLayer->GetParentName().length() > 0)
					{
						pResMan->UnloadLayerPak(pLayer->GetParentName().c_str());
					}
					else
					{
						pResMan->UnloadLayerPak(pLayer->GetName().c_str());
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::ToggleLayersBySubstring(const char* pSearchSubstring, const char* pExceptionSubstring, bool isEnable)
{
	// This toggles visibility of all layers containing pSearchSubstring with the exception of any containing pExceptionSubstring.
	// This does not modify parent layers by passing false to ToggleLayerVisibility
	string exceptionSubstring = pExceptionSubstring;
	string disableSubstring = pSearchSubstring;

	exceptionSubstring.MakeLower();
	disableSubstring.MakeLower();

	for(TLayers::iterator it = m_layers.begin(); it != m_layers.end(); ++it)
	{
		CEntityLayer* pLayer = it->second;
		string pLayerName = pLayer->GetName();
		pLayerName.MakeLower();

		if (strstr(pLayerName, disableSubstring) != NULL)
		{
			if (exceptionSubstring.empty() || strstr(pLayerName, exceptionSubstring) == NULL)
			{
				ToggleLayerVisibility(pLayer->GetName(), isEnable, false);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
int CEntitySystem::GetVisibleLayerIDs(uint8* pLayerMask, const uint32 maxCount) const
{
	memset(pLayerMask, 0, maxCount/8);

	int maxLayerId = 0;
	for (std::map<string,CEntityLayer*>::const_iterator it = m_layers.begin();
		it != m_layers.end(); ++it)
	{
		CEntityLayer* pLayer = it->second;
		if (pLayer->IsEnabled())
		{
			const int id = pLayer->GetId();
			if (id < (int)maxCount)
			{
				pLayerMask[id >> 3] |= (1<<(id & 7));
			}

			if (id > maxLayerId)
			{
				maxLayerId = id;
			}
		}
	}

	return maxLayerId;
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::DumpEntity(IEntity *pEntity)
{
  static struct SFlagData
  {
    unsigned mask;
    const char * name;
  } flagData[] = {
    {ENTITY_FLAG_CLIENT_ONLY, "Client"},
    {ENTITY_FLAG_SERVER_ONLY, "Server"},
    {ENTITY_FLAG_UNREMOVABLE, "NoRemove"},
    {ENTITY_FLAG_CLIENTSIDE_STATE, "ClientState"},
    {ENTITY_FLAG_MODIFIED_BY_PHYSICS, "PhysMod"},
  };

  string name(pEntity->GetName());
  name += string("[$9") + pEntity->GetClass()->GetName() + string("$o]");
  Vec3 pos(pEntity->GetWorldPos());
  const char *sStatus = pEntity->IsActive() ? "[$3Active$o]" : "[$9Inactive$o]";
  if (pEntity->IsHidden())
    sStatus = "[$9Hidden$o]";

  std::set<string> allFlags;
  for (size_t i=0; i<sizeof(flagData)/sizeof(*flagData); ++i)
  {
    if ((flagData[i].mask & pEntity->GetFlags()) == flagData[i].mask)
      allFlags.insert(flagData[i].name);
  }
  string flags;
  for (std::set<string>::iterator iter = allFlags.begin(); iter != allFlags.end(); ++iter)
  {
    flags += ' ';
    flags += *iter;
  }

	CryLogAlways("%5d: Salt:%d  %-42s  %-14s  pos: (%.2f %.2f %.2f) %d %s",
		pEntity->GetId()&0xffff,pEntity->GetId()>>16,
		name.c_str(), sStatus, pos.x, pos.y, pos.z, pEntity->GetId(), flags.c_str());
}

void CEntitySystem::DeletePendingEntities()
{
	UpdateDeletedEntities();
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::ChangeEntityName( CEntity *pEntity,const char *sNewName )
{
	if (!pEntity->m_szName.empty())
	{
		// Remove old name.
		EntityNamesMap::iterator it = m_mapEntityNames.lower_bound(pEntity->m_szName.c_str());
		for (; it != m_mapEntityNames.end(); ++it)
		{
			if (it->second == pEntity->m_nID)
			{
				m_mapEntityNames.erase(it);
				break;
			}
			if (stricmp(it->first,pEntity->m_szName.c_str()) != 0)
				break;
		}
	}

	if (sNewName[0] != 0)
	{
		pEntity->m_szName = sNewName;
		// Insert new name into the map.
		m_mapEntityNames.insert( EntityNamesMap::value_type(pEntity->m_szName.c_str(),pEntity->m_nID) );
	}
	else
	{
		pEntity->m_szName = sNewName;
	}
}


//////////////////////////////////////////////////////////////////////////
void CEntitySystem::ClearLayers()
{
	for ( std::map<string,CEntityLayer*>::iterator it = m_layers.begin(); it != m_layers.end(); ++it )
	{
		CEntityLayer* pLayer = it->second;
		delete pLayer;
	}
	m_layers.clear();

#ifdef ENABLE_PROFILING_CODE
	g_pIEntitySystem->m_layerProfiles.clear();
#endif //ENABLE_PROFILING_CODE
}


//////////////////////////////////////////////////////////////////////////
CEntityLayer* CEntitySystem::AddLayer(const char* name, const char* parent, 
														 uint16 id, bool bHasPhysics, int specs, bool bDefaultLoaded)
{
	CEntityLayer* pLayer = new CEntityLayer(name, id, bHasPhysics, specs, bDefaultLoaded, m_garbageLayerHeaps);
	if(parent && *parent)
	{
		pLayer->SetParentName(parent);
	}
	m_layers[name] = pLayer;
	return pLayer;
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::LinkLayerChildren()
{
	// make sure parent - child relation is valid
	for (TLayers::iterator it = m_layers.begin(); it != m_layers.end(); ++it)
	{
		CEntityLayer* pLayer = it->second;
		if (pLayer->GetParentName().length() == 0)
			continue;

		CEntityLayer* pParent = FindLayer(pLayer->GetParentName());
		if(pParent)
			pParent->AddChild(pLayer);
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::LoadLayers(const char* dataFile)
{
	LOADING_TIME_PROFILE_SECTION;

	bool bClearSkippedLayers = true;

	ClearLayers();
	XmlNodeRef root = GetISystem()->LoadXmlFromFile(dataFile);
	if (root)
	{
		XmlNodeRef layersNode = root->findChild("Layers");
		if (layersNode)
		{
			for (int i = 0; i < layersNode->getChildCount(); i++)
			{
				XmlNodeRef layerNode = layersNode->getChild(i);
				if (layerNode->isTag("Layer"))
				{
					string name = layerNode->getAttr("Name");
					string parent = layerNode->getAttr("Parent");
					uint16 id;
					layerNode->getAttr("Id", id);
					int nHavePhysics = 1;
					layerNode->getAttr("Physics", nHavePhysics);
					int specs = eSpecType_All;
					layerNode->getAttr("Specs", specs);
					int nDefaultLayer = 0;
					layerNode->getAttr("DefaultLoaded", nDefaultLayer);
					CEntityLayer* pLayer = AddLayer(name, parent, id, nHavePhysics ? true : false, specs, nDefaultLayer ? true : false);

					if(pLayer->IsSkippedBySpec())
					{
						gEnv->p3DEngine->SkipLayerLoading(id, bClearSkippedLayers);
						bClearSkippedLayers = false;
					}
				}
			}
		}
	}
	LinkLayerChildren();
}


//////////////////////////////////////////////////////////////////////////
CEntityLayer* CEntitySystem::FindLayer(const char* layer)
{
	if (layer)
	{
		std::map<string,CEntityLayer*>::iterator found=m_layers.find(CONST_TEMP_STRING(layer));
		if(found!=m_layers.end())
			return found->second;
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::AddEntityToLayer(const char* layer, EntityId id)
{
	CEntityLayer* pLayer = FindLayer(layer);
	if(pLayer)
		pLayer->AddObject(id);
}

///////////////////////////////////////// /////////////////////////////////
void CEntitySystem::RemoveEntityFromLayers(EntityId id)
{
	for ( std::map<string,CEntityLayer*>::iterator it = m_layers.begin(); it != m_layers.end(); ++it )
	{
		(it->second)->RemoveObject(id);
	}
}

//////////////////////////////////////////////////////////////////////////
CEntityLayer* CEntitySystem::GetLayerForEntity( EntityId id )
{
	TLayers::const_iterator it = m_layers.begin();
	TLayers::const_iterator end = m_layers.end();
	for(; it != end; ++it)
	{
		if(it->second->IncludesEntity(id))
			return it->second;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::EnableDefaultLayers(bool isSerialized)
{
	for(TLayers::iterator it = m_layers.begin(); it != m_layers.end(); ++it)
	{
		CEntityLayer* pLayer = it->second;
		if (pLayer->IsDefaultLoaded())
			EnableLayer(it->first, true, isSerialized);
	}
}
//////////////////////////////////////////////////////////////////////////
void CEntitySystem::EnableLayer(const char* layer, bool isEnable, bool isSerialized)
{
	if(!gEnv->p3DEngine->IsAreaActivationInUse())
		return;
	CEntityLayer* pLayer = FindLayer(layer);
	if(pLayer)
	{
		bool bEnableChange = pLayer->IsEnabledBrush() != isEnable;

#if ENABLE_STATOSCOPE
		if (gEnv->pStatoscope && (pLayer->IsEnabled() != isEnable))
		{
			string userMarker = "LayerSwitching: ";
			userMarker += isEnable ? "Enable " : "Disable ";
			userMarker += layer;
			gEnv->pStatoscope->AddUserMarker("LayerSwitching", userMarker.c_str());
		}
#endif

		pLayer->Enable(isEnable, isSerialized);

		IResourceManager* pResMan = gEnv->pSystem->GetIResourceManager();
		if (bEnableChange && pResMan)
		{
			if (isEnable)
			{
				if (pLayer->GetParentName().length() > 0)	
					pResMan->LoadLayerPak(pLayer->GetParentName().c_str());
				else
					pResMan->LoadLayerPak(pLayer->GetName().c_str());
			} else {
				if (pLayer->GetParentName().length() > 0)	
					pResMan->UnloadLayerPak(pLayer->GetParentName().c_str());
				else
					pResMan->UnloadLayerPak(pLayer->GetName().c_str());
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool CEntitySystem::IsLayerEnabled(const char* layer, bool bMustBeLoaded) const
{
	const CEntityLayer* pLayer = const_cast<CEntitySystem*>(this)->FindLayer(layer);
	if(pLayer)
	{
		if (pLayer->IsEnabled())
		{
			return pLayer->IsSerialized() || !bMustBeLoaded;
		}
	}

	return false;
}

bool CEntitySystem::ShouldSerializedEntity( IEntity* pEntity )
{
	if(!pEntity)
		return false;

	//entity flag
	if(pEntity->GetFlags() & ENTITY_FLAG_NO_SAVE)
		return false;

	//lua flag
	if(CVar::es_SaveLoadUseLUANoSaveFlag != 0)
	{
		IScriptTable* pEntityScript = pEntity->GetScriptTable();
		SmartScriptTable props;
		if (pEntityScript && pEntityScript->GetValue("Properties", props))
		{
			bool bSerialize = true;
			if (props->GetValue("bSerialize", bSerialize) && (bSerialize == false))
				return false;
		}
	}

	// layer settings
	int iSerMode = CVar::es_LayerSaveLoadSerialization;
	if(iSerMode == 0)
		return true;

	CEntityLayer *pLayer = GetLayerForEntity(pEntity->GetId());
	if(!pLayer)
		return true;

	if(iSerMode == 1)
		return pLayer->IsEnabled();

	return pLayer->IsSerialized();
}

void CEntitySystem::UpdateEngineCVars()
{
	static ICVar * p_e_view_dist_min					= gEnv->pConsole->GetCVar("e_ViewDistMin");
	static ICVar * p_e_view_dist_ratio				= gEnv->pConsole->GetCVar("e_ViewDistRatio");
	static ICVar * p_e_view_dist_ratio_custom	= gEnv->pConsole->GetCVar("e_ViewDistRatioCustom");
	static ICVar * p_e_view_dist_ratio_detail = gEnv->pConsole->GetCVar("e_ViewDistRatioDetail");
	
	assert(p_e_view_dist_min && p_e_view_dist_ratio && p_e_view_dist_ratio_detail);

	CRenderProxy::SetViewDistMin(p_e_view_dist_min->GetFVal());
	CRenderProxy::SetViewDistRatio(p_e_view_dist_ratio->GetFVal());
	CRenderProxy::SetViewDistRatioCustom(p_e_view_dist_ratio_custom->GetFVal());
	CRenderProxy::SetViewDistRatioDetail(p_e_view_dist_ratio_detail->GetFVal());
}

//////////////////////////////////////////////////////////////////////////
bool CEntitySystem::ExtractEntityLoadParams(XmlNodeRef &entityNode, SEntitySpawnParams &spawnParams) const
{
	return(m_pEntityLoadManager->ExtractEntityLoadParams(entityNode,spawnParams));
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::BeginCreateEntities(int nAmtToCreate)
{
	m_pEntityLoadManager->PrepareBatchCreation(nAmtToCreate);
}

//////////////////////////////////////////////////////////////////////////
bool CEntitySystem::CreateEntity(XmlNodeRef &entityNode,SEntitySpawnParams &pParams,EntityId &outUsingId)
{
	return(m_pEntityLoadManager->CreateEntity(entityNode,pParams,outUsingId));
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::EndCreateEntities()
{
	m_pEntityLoadManager->OnBatchCreationCompleted();
}

void CEntitySystem::PurgeDeferredCollisionEvents( bool bForce )
{
	// make sure we deleted entities which needed to keep alive for deferred execution when they are no longer needed
	for( std::vector<CEntity*>::iterator it = m_deferredUsedEntities.begin() ; it != m_deferredUsedEntities.end() ; )
	{
		if( (*it)->m_nKeepAliveCounter == 0 || bForce )
		{
			stl::push_back_unique(m_deletedEntities, *it);
			it = m_deferredUsedEntities.erase(it);
		}			
		else
		{
			++it;
		}
	}
}

void CEntitySystem::ComponentRegister( EntityId id, IComponentPtr pComponent, const int flags )
{
	pComponent->SetDistributer( m_pEventDistributer, id );
	if( (flags & IComponent::EComponentFlags_LazyRegistration) == 0 )
	{
		m_pEventDistributer->RegisterComponent( id, pComponent, (flags & IComponent::EComponentFlags_Enable) != 0 );
	}
}

void CEntitySystem::ComponentEnableEvent( const EntityId id, const int eventID, const bool enable )
{
	m_pEventDistributer->EnableEventForEntity( id, eventID, enable );
}

void CEntitySystem::DebugDraw()
{
	if (CVar::es_DrawProximityTriggers > 0)
	{
		DebugDrawProximityTriggers();
	}
}

void CEntitySystem::DebugDrawProximityTriggers()
{
	IRenderAuxGeom* pRenderAuxGeom = gEnv->pRenderer->GetIRenderAuxGeom();
	assert(pRenderAuxGeom);
	if (!pRenderAuxGeom)
		return;

	IEntityItPtr it = GetEntityIterator();
	while (IEntity* pEntity = it->Next())
	{
		if (!strcmp(pEntity->GetClass()->GetName(), "ProximityTrigger"))
		{
			SAuxGeomRenderFlags oldFlags = pRenderAuxGeom->GetRenderFlags();
			SAuxGeomRenderFlags newFlags = oldFlags;
			newFlags.SetCullMode(e_CullModeNone);
			newFlags.SetAlphaBlendMode(e_AlphaBlended);
			pRenderAuxGeom->SetRenderFlags(newFlags);
			AABB aabb;
			pEntity->GetLocalBounds(aabb);
			uint8 alpha = (CVar::es_DrawProximityTriggers != 1) ? CVar::es_DrawProximityTriggers : 70;

			ColorB color(255, 0, 0);	// Red indicates an error

			if (IEntityScriptProxy* pEntityScriptProxy = static_cast<IEntityScriptProxy*>(pEntity->GetProxy(ENTITY_PROXY_SCRIPT)))
			{
				if (IScriptTable* pScriptTable = pEntityScriptProxy->GetScriptTable())
				{
					bool bEnabled;
					if (pScriptTable->GetValue("enabled", bEnabled))
					{
						if (bEnabled)
						{
							bool bTriggered;
							if (pScriptTable->GetValue("triggered", bTriggered) && bTriggered)
							{
								color.Set(0, 0, 255);
							}
							else
							{
								color.Set(0, 255, 0);
							}
						}
						else
						{
							color.Set(255, 255, 255);
						}
					}
				}
			}

			color.a = alpha;
			pRenderAuxGeom->DrawAABB(aabb, pEntity->GetWorldTM(), true, color, eBBD_Faceted);

			pRenderAuxGeom->SetRenderFlags(oldFlags);
		}
	}
}

void CEntitySystem::DoPrePhysicsUpdate()
{
#ifdef PROFILE_ENTITYSYSTEM
	if (CVar::pProfileEntities->GetIVal() < 0)
		GetISystem()->GetIProfilingSystem()->VTuneResume();
#endif

	g_bProfilerEnabled = gEnv->pFrameProfileSystem->IsProfiling();

	FUNCTION_PROFILER_FAST( m_pISystem,PROFILE_ENTITY,g_bProfilerEnabled );

	float fFrameTime = gEnv->pTimer->GetFrameTime();
	for (EntitiesSet::iterator it = m_mapPrePhysicsEntities.begin(); it != m_mapPrePhysicsEntities.end();)
	{
		EntityId eid = *it;
		EntitiesSet::iterator next = it;
		++next;
		CEntity *pEntity = (CEntity*)GetEntity(eid);
		if (pEntity)
		{
#ifdef _DEBUG
			INDENT_LOG_DURING_SCOPE(true, "While doing pre-physics update of %s...", pEntity->GetEntityTextDescription());
#endif
			pEntity->PrePhysicsUpdate(fFrameTime);
		}
		CRY_ASSERT_TRACE(pEntity, ("Non-valid entity exists in m_mapPrePhysicsEntities. Id = %u", eid));
		it = next;
	}

#ifdef PROFILE_ENTITYSYSTEM
	if (CVar::pProfileEntities->GetIVal() < 0)
		GetISystem()->GetIProfilingSystem()->VTunePause();
#endif
}

void CEntitySystem::DoPrePhysicsUpdateFast()
{
#ifdef PROFILE_ENTITYSYSTEM
	if (CVar::pProfileEntities->GetIVal() < 0)
		GetISystem()->GetIProfilingSystem()->VTuneResume();
#endif

	g_bProfilerEnabled = gEnv->pFrameProfileSystem->IsProfiling();

	FUNCTION_PROFILER_FAST( m_pISystem,PROFILE_ENTITY,g_bProfilerEnabled );

	SEntityEvent evt( ENTITY_EVENT_PREPHYSICSUPDATE );
	evt.fParam[0] = gEnv->pTimer->GetFrameTime();

	// Dispatch the event.
	m_pEventDistributer->SendEvent( evt );

#ifdef PROFILE_ENTITYSYSTEM
	if (CVar::pProfileEntities->GetIVal() < 0)
		GetISystem()->GetIProfilingSystem()->VTunePause();
#endif
}

void CEntitySystem::RegisterCharactersForRendering()
{
	CRenderProxy::RegisterCharactersForRendering();
}

IBSPTree3D* CEntitySystem::CreateBSPTree3D( const IBSPTree3D::FaceList& faceList )
{
	return new CBSPTree3D(faceList);
}

void CEntitySystem::ReleaseBSPTree3D(IBSPTree3D*& pTree)
{
	SAFE_DELETE(pTree);
}

#include UNIQUE_VIRTUAL_WRAPPER(IEntityIt)
#include UNIQUE_VIRTUAL_WRAPPER(IEntitySystem)


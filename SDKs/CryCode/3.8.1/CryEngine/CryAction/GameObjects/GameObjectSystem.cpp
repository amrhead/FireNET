// CryEngine Source File.
// Copyright (C), Crytek, 1999-2014.


#include "StdAfx.h"
#include "GameObjects/GameObject.h"
#include "GameObjectSystem.h"
#include "IGameObject.h"

static bool StringToKey( const char * s, uint32& key )
{
	const size_t len = strlen(s);
	if (len > 4)
		return false;

	key = 0;
	for (size_t i=0; i<len; i++)
	{
		key <<= 8;
		key |= uint8(s[i]);
	}

	return true;
}

bool CGameObjectSystem::Init()
{
	IEntityClassRegistry::SEntityClassDesc desc;
	desc.sName = "PlayerProximityTrigger";
	desc.flags = ECLF_INVISIBLE;
	if(gEnv->pEntitySystem)
	{
		IEntityClassRegistry * pClsReg = gEnv->pEntitySystem->GetClassRegistry();
		m_pClassPlayerProximityTrigger = pClsReg->RegisterStdClass( desc );
		if (!m_pClassPlayerProximityTrigger)
			return false;
	}

	memset(&m_defaultProfiles, 0, sizeof(m_defaultProfiles));

	if (XmlNodeRef schedParams = gEnv->pSystem->LoadXmlFromFile("Scripts/Network/EntityScheduler.xml"))
	{
		uint32 defaultPolicy = 0;

		if (XmlNodeRef defpol = schedParams->findChild("Default"))
		{
			if (!StringToKey(defpol->getAttr("policy"), defaultPolicy))
			{
				GameWarning("Unable to read Default from EntityScheduler.xml");
			}
		}

		m_defaultProfiles.normal = m_defaultProfiles.owned = defaultPolicy;

		for (int i=0; i<schedParams->getChildCount(); i++)
		{
			XmlNodeRef node = schedParams->getChild(i);
			if (0 != strcmp(node->getTag(), "Class"))
				continue;

			string name = node->getAttr("name");

			SEntitySchedulingProfiles p;
			p.normal = defaultPolicy;
			if (node->haveAttr("policy"))
				StringToKey(node->getAttr("policy"), p.normal);
			p.owned = p.normal;
			if (node->haveAttr("own"))
				StringToKey(node->getAttr("own"), p.owned);

#if !defined(_RELEASE)
			TSchedulingProfiles::iterator iter = m_schedulingParams.find(CONST_TEMP_STRING(name));
			if (iter != m_schedulingParams.end())
			{
				GameWarning("Class '%s' has been defined multiple times in EntityScheduler.xml", name.c_str());
			}
#endif //#if !defined(_RELEASE)

			m_schedulingParams[name] = p;
		}
	}

	m_spawnSerializers.reserve(8);

  LoadSerializationOrderFile();
	
	return true;
}

void CGameObjectSystem::Reset()
{
#if !defined(_RELEASE)
	if(gEnv->bMultiplayer)
	{
		IEntityClassRegistry * pClassReg = gEnv->pEntitySystem->GetClassRegistry();

		TSchedulingProfiles::iterator iter = m_schedulingParams.begin();
		TSchedulingProfiles::iterator end = m_schedulingParams.end();
		for(; iter != end; ++iter)
		{
			string className = iter->first;
			if(!pClassReg->FindClass(className.c_str()))
			{
				GameWarning("EntityScheduler.xml has a class defined '%s' but unable to find it in ClassRegistry", className.c_str());
			}
		}
	}
#endif //#if !defined(_RELEASE)

	stl::free_container(m_tempObjects);
	stl::free_container(m_postUpdateObjects);
	stl::free_container(m_activatedExtensions_top);
}



//////////////////////////////////////////////////////////////////////////
// this reads the .xml that defines in which order the extensions are serialized inside a game object

void CGameObjectSystem::LoadSerializationOrderFile()
{
	static const char* SERIALIZATIONORDER_FILE = "Scripts/GameObjectSerializationOrder.xml";

	XmlNodeRef xmlNodeRoot = GetISystem()->LoadXmlFromFile( SERIALIZATIONORDER_FILE );

	if (xmlNodeRoot==(IXmlNode*)NULL)
	{
		CryWarning( VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CGameObjectSystem::LoadSerializationOrderFile() - Failed to load '%s'. sorting disabled.", SERIALIZATIONORDER_FILE);
		return;
	}

	uint32 count = xmlNodeRoot->getChildCount();
	m_serializationOrderList.resize( count );
	std::set<const char*, stl::less_stricmp<const char*> > duplicateCheckerList;

	bool duplicatedEntriesInXML = false;
	for (uint32 i=0; i<count; ++i)
	{
		string& name = m_serializationOrderList[i];
		XmlNodeRef xmlNodeExtension = xmlNodeRoot->getChild( i );
		name = xmlNodeExtension->getTag();
		name.MakeLower();
		if ( (duplicateCheckerList.insert( xmlNodeExtension->getTag() )).second!=true )
		{
			duplicatedEntriesInXML = true;
			CryWarning( VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CGameObjectSystem::LoadSerializationOrderFile() - Duplicated game object extension: '%s'. Savegames wont have properly sorted game object extensions now", name.c_str() );
		}
	}
	assert( !duplicatedEntriesInXML );
}

//////////////////////////////////////////////////////////////////////////
uint32 CGameObjectSystem::GetExtensionSerializationPriority( IGameObjectSystem::ExtensionID id )
{
	if (id > m_extensionInfo.size())
		return 0xffffffff; // minimum possible priority
	else
		return m_extensionInfo[id].serializationPriority;
}

//////////////////////////////////////////////////////////////////////////
IEntity * CGameObjectSystem::CreatePlayerProximityTrigger()
{
	IEntitySystem * pES = gEnv->pEntitySystem;
	SEntitySpawnParams params;
	params.nFlags = ENTITY_FLAG_CLIENT_ONLY|ENTITY_FLAG_NO_SAVE;
	params.pClass = m_pClassPlayerProximityTrigger;
  params.sName = "PlayerProximityTrigger";
	IEntity * pEntity = pES->SpawnEntity( params );
	if (!pEntity)
		return NULL;
	if (!pEntity->CreateProxy(ENTITY_PROXY_TRIGGER))
	{
		pES->RemoveEntity( pEntity->GetId() );
		return NULL;
	}
	return pEntity;
}

void CGameObjectSystem::RegisterExtension( const char * name, IGameObjectExtensionCreatorBase * pCreator, IEntityClassRegistry::SEntityClassDesc * pClsDesc )
{
	string sName = name;

	if (m_nameToID.find(sName) != m_nameToID.end())
		CryFatalError("Duplicate game object extension %s found", name);

	SExtensionInfo info;
	info.name = sName;
	info.pFactory = pCreator;

	string nameLower( name );
	nameLower.MakeLower();
	std::vector<string>::const_iterator result = std::find( m_serializationOrderList.begin(), m_serializationOrderList.end(), nameLower );
	std::vector<string>::const_iterator firstElem = m_serializationOrderList.begin();
	info.serializationPriority = uint32(std::distance(firstElem, result)); // is a valid value even if name was not found (minimum priority in that case)

	ExtensionID id = m_extensionInfo.size();
	m_extensionInfo.push_back(info);
	m_nameToID[sName] = id;

	CRY_ASSERT( GetName(GetID(sName)) == sName );

	// bind network interface
	void * pRMI;
	size_t nRMI;
	pCreator->GetGameObjectExtensionRMIData( &pRMI, &nRMI );
	m_dispatch.RegisterInterface( (SGameObjectExtensionRMI*)pRMI, nRMI );

	if (pClsDesc)
	{
		pClsDesc->pUserProxyCreateFunc = CreateGameObjectWithPreactivatedExtension;
//		pClsDesc->pUserProxyData = new SSpawnUserData(sName);
		if (!gEnv->pEntitySystem->GetClassRegistry()->RegisterStdClass(*pClsDesc))
		{
			CRY_ASSERT_TRACE(0, ("Unable to register entity class '%s'", name));
			return;
		}
	}
}

void CGameObjectSystem::RegisterSchedulingProfile( const char* szEntityClassName, const char* szNormalPolicy, const char* szOwnedPolicy )
{
#if !defined(_RELEASE)
	TSchedulingProfiles::iterator iter = m_schedulingParams.find(CONST_TEMP_STRING(szEntityClassName));
	if (iter != m_schedulingParams.end())
	{
		GameWarning("Class '%s' has already a profile assigned", szEntityClassName);
	}
#endif //#if !defined(_RELEASE)

	SEntitySchedulingProfiles schedulingProfile = m_defaultProfiles;
	
	if (szNormalPolicy)
	{
		StringToKey(szNormalPolicy, schedulingProfile.normal);
		schedulingProfile.owned = schedulingProfile.normal;
	}

	if (szOwnedPolicy)
	{
		StringToKey(szOwnedPolicy, schedulingProfile.owned);
	}

	m_schedulingParams.insert(TSchedulingProfiles::value_type(szEntityClassName, schedulingProfile));
}

void CGameObjectSystem::DefineProtocol( bool server, IProtocolBuilder * pBuilder )
{
	INetMessageSink * pSink = server? m_dispatch.GetServerSink() : m_dispatch.GetClientSink();
	pSink->DefineProtocol( pBuilder );
}

IGameObjectSystem::ExtensionID CGameObjectSystem::GetID( const char * name )
{
	std::map<string, ExtensionID>::const_iterator iter = m_nameToID.find(CONST_TEMP_STRING(name));
	if (iter != m_nameToID.end())
		return iter->second;
	else
		return InvalidExtensionID;
}

const char * CGameObjectSystem::GetName( ExtensionID id )
{
	if (id > m_extensionInfo.size())
		return NULL;
	else
		return m_extensionInfo[id].name.c_str();
}

void CGameObjectSystem::BroadcastEvent( const SGameObjectEvent& evt )
{
	FUNCTION_PROFILER(GetISystem(), PROFILE_ACTION);

	//CryLog("BroadcastEvent called");

	IEntityItPtr pEntIt = gEnv->pEntitySystem->GetEntityIterator();
	while (IEntity * pEntity = pEntIt->Next())
	{
		if (CGameObject * pGameObject = (CGameObject*)pEntity->GetProxy(ENTITY_PROXY_USER))
		{
			pGameObject->SendEvent( evt );
		}
	}
}

void CGameObjectSystem::RegisterEvent( uint32 id, const char* name )
{
	if (GetEventID(name) != InvalidEventID || GetEventName(id) != 0)
	{
		CryFatalError("Duplicate game object event (%d - %s) found", id, name);
	}

	m_eventNameToID[name] = id;
	m_eventIDToName[id] = name;
}

uint32 CGameObjectSystem::GetEventID( const char* name )
{
	if (!name)
		return InvalidEventID;

	std::map<string, uint32>::const_iterator iter = m_eventNameToID.find(CONST_TEMP_STRING(name));
	if (iter != m_eventNameToID.end())
		return iter->second;
	else
		return InvalidEventID;
}

const char* CGameObjectSystem::GetEventName( uint32 id )
{
	std::map<uint32, string>::const_iterator iter = m_eventIDToName.find(id);
	if (iter != m_eventIDToName.end())
		return iter->second.c_str();
	else
		return 0;
}


IGameObject *CGameObjectSystem::CreateGameObjectForEntity(EntityId entityId)
{
	if (IGameObject *pGameObject=CCryAction::GetCryAction()->GetGameObject(entityId))
		return pGameObject;

	IEntity *pEntity = gEnv->pEntitySystem->GetEntity(entityId);
	if (pEntity)
	{
		CGameObjectPtr pGameObject = ComponentCreateAndRegister_DeleteWithRelease<CGameObject>( IComponent::SComponentInitializer( pEntity ), IComponent::EComponentFlags_LazyRegistration );
		pEntity->SetProxy(ENTITY_PROXY_USER, pGameObject);

		SEntitySpawnParams spawnParams;
		pGameObject->Init(pEntity, spawnParams);
    // call sink
    for (SinkList::iterator si=m_lstSinks.begin();si!=m_lstSinks.end();++si)
    {
      (*si)->OnAfterInit(pGameObject.get());
    }
    //
		return pGameObject.get();
	}

	return 0;
}

IEntityProxyPtr CGameObjectSystem::CreateGameObjectEntityProxy(IEntity& entity, IGameObject** ppGameObject)
{
	CGameObjectPtr	pGameObject = ComponentCreateAndRegister_DeleteWithRelease<CGameObject>( IComponent::SComponentInitializer( &entity ), IComponent::EComponentFlags_LazyRegistration );
	if (ppGameObject)
	{
		*ppGameObject = pGameObject.get();
	}
	return pGameObject;
}

IGameObjectExtensionPtr CGameObjectSystem::Instantiate( ExtensionID id, IGameObject * pObject )
{
	if (id > m_extensionInfo.size())
		return IGameObjectExtensionPtr();
	IGameObjectExtensionPtr pExt = m_extensionInfo[id].pFactory->Create();
	if (!pExt)
		return IGameObjectExtensionPtr();
	IEntity* pEntity = pObject->GetEntity();
	pEntity->RegisterComponent( crycomponent_cast<IComponentPtr> (pExt), IComponent::EComponentFlags_Enable|IComponent::EComponentFlags_LazyRegistration );
	pExt->Initialize( IComponent::SComponentInitializer( pEntity ) );
	
	TSerialize* pSpawnSerializer = GetSpawnSerializerForEntity( pEntity->GetId() );
	if (pSpawnSerializer)
		pExt->SerializeSpawnInfo( *pSpawnSerializer );
	
	if (!pExt->Init( pObject ))
	{
		pEntity->RegisterComponent( crycomponent_cast<IComponentPtr> (pExt), false );
		pExt.reset();
		return IGameObjectExtensionPtr();
	}
	return pExt;
}

/* static */
IEntityProxyPtr CGameObjectSystem::CreateGameObjectWithPreactivatedExtension(IEntity *pEntity, SEntitySpawnParams &params, void *pUserData)
{
	CGameObjectPtr pGameObject = ComponentCreateAndRegister_DeleteWithRelease<CGameObject>( IComponent::SComponentInitializer( pEntity ), IComponent::EComponentFlags_LazyRegistration );
	if (!pGameObject->ActivateExtension( params.pClass->GetName() ))
	{
		pEntity->RegisterComponent( pGameObject, false );
		pGameObject.reset();
		return IEntityProxyPtr();
	}

	if (params.pUserData)
	{
		SEntitySpawnParamsForGameObjectWithPreactivatedExtension * pParams = 
			static_cast<SEntitySpawnParamsForGameObjectWithPreactivatedExtension*>(params.pUserData);
		if (!pParams->hookFunction( pEntity, pGameObject.get(), pParams->pUserData ))
		{
			pEntity->RegisterComponent( pGameObject, false );
			pGameObject.reset();
			return IEntityProxyPtr();
		}
	}

#if GAME_OBJECT_SUPPORTS_CUSTOM_USER_DATA
	pGameObject->SetUserData(params.pUserData);
#endif

	return pGameObject;
}

void CGameObjectSystem::PostUpdate( float frameTime )
{
	m_tempObjects = m_postUpdateObjects;
	for (std::vector<IGameObject*>::const_iterator iter = m_tempObjects.begin(); iter != m_tempObjects.end(); ++iter)
	{
		(*iter)->PostUpdate(frameTime);
	}
}

void CGameObjectSystem::SetPostUpdate( IGameObject * pGameObject, bool enable )
{
	if (enable)
		stl::push_back_unique( m_postUpdateObjects, pGameObject );
	else
		stl::find_and_erase( m_postUpdateObjects, pGameObject );
}

const SEntitySchedulingProfiles * CGameObjectSystem::GetEntitySchedulerProfiles( IEntity * pEnt )
{
	if (!gEnv->bMultiplayer)
		return &m_defaultProfiles;

	if (pEnt->GetFlags() & (ENTITY_FLAG_CLIENT_ONLY | ENTITY_FLAG_SERVER_ONLY))
		return &m_defaultProfiles;

	TSchedulingProfiles::iterator iter = m_schedulingParams.find(CONST_TEMP_STRING(pEnt->GetClass()->GetName()));

	if (iter == m_schedulingParams.end())
	{
		GameWarning("No network scheduling parameters set for entities of class '%s' in EntityScheduler.xml", pEnt->GetClass()->GetName());
		return &m_defaultProfiles;
	}
	return &iter->second;
}

//////////////////////////////////////////////////////////////////////////

void CGameObjectSystem::SetSpawnSerializerForEntity( const EntityId entityId, TSerialize* pSerializer )
{
	CRY_ASSERT(GetSpawnSerializerForEntity(entityId) == NULL);
	if (GetSpawnSerializerForEntity(entityId) != NULL)
	{
		DEBUG_BREAK;
	}

	m_spawnSerializers.push_back(SSpawnSerializer(entityId, pSerializer));
}

void CGameObjectSystem::ClearSpawnSerializerForEntity( const EntityId entityId )
{
	stl::find_and_erase(m_spawnSerializers, entityId);
}

//////////////////////////////////////////////////////////////////////////

TSerialize* CGameObjectSystem::GetSpawnSerializerForEntity( const EntityId entityId ) const
{
	TSpawnSerializers::const_iterator it = std::find(m_spawnSerializers.begin(), m_spawnSerializers.end(), entityId);

	return (it != m_spawnSerializers.end()) ? (*it).pSerializer : NULL;
}

//////////////////////////////////////////////////////////////////////////

void CGameObjectSystem::GetMemoryUsage(ICrySizer * s) const
{
	SIZER_SUBCOMPONENT_NAME(s, "GameObjectSystem");

	s->AddObject(this, sizeof(*this));
	s->AddObject(m_nameToID);
	s->AddObject(m_extensionInfo);	
	s->AddObject(m_dispatch);
	s->AddObject(m_postUpdateObjects);	
	s->AddObject(m_schedulingParams);
	
	IEntityItPtr pIt = gEnv->pEntitySystem->GetEntityIterator();
	while (IEntity * pEnt = pIt->Next())
	{
		s->AddObject((CGameObject *)pEnt->GetProxy(ENTITY_PROXY_USER));		
	}
}

//////////////////////////////////////////////////////////////////////
void CGameObjectSystem::AddSink( IGameObjectSystemSink *pSink )
{
  CRY_ASSERT(pSink);

  if(pSink)
    stl::push_back_unique( m_lstSinks, pSink );
}

//////////////////////////////////////////////////////////////////////////
void CGameObjectSystem::RemoveSink( IGameObjectSystemSink *pSink )
{
  CRY_ASSERT(pSink);

  m_lstSinks.remove(pSink);
}



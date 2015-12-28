/*************************************************************************
  Crytek Source File.
  Copyright (C), Crytek Studios, 2001-2010.
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Description: Handles a pool of entities and references to potential
				entities derived from level loading
  
 -------------------------------------------------------------------------
  History:
  - 15:03:2010: Created by Kevin Kirst

*************************************************************************/

#ifndef __ENTITYPOOLMANAGER_H__
#define __ENTITYPOOLMANAGER_H__

#include "IEntityPoolManager.h"
#include "EntitySystem.h"

#include "EntityPoolCommon.h"

#include "ISerializeHelper.h"
#include "CryListenerSet.h"

class SmartScriptTable;

class CEntityPoolManager : public IEntityPoolManager
{
public:
	CEntityPoolManager(CEntitySystem *pEntitySystem);
	~CEntityPoolManager();
	
	void GetMemoryStatistics(ICrySizer *pSizer) const;

	CEntitySystem* GetEntitySystem() const { return m_pEntitySystem; }

	const CEntityPoolDefinition* GetEntityPoolDefinition(TEntityPoolDefinitionId id) const;

	// IEntityPoolManager
	virtual void Serialize(TSerialize ser);
	virtual void Enable(bool bEnable);
	virtual void ResetPools(bool bSaveState = true);
	virtual void AddListener(IEntityPoolListener *pListener, const char* szWho, uint32 uSubscriptions);
	virtual void RemoveListener(IEntityPoolListener *pListener);
	virtual bool PrepareFromPool(EntityId entityId, bool bPrepareNow = false);
	virtual bool ReturnToPool(EntityId entityId, bool bSaveState = true);
	virtual void ResetBookmark(EntityId entityId);
	virtual bool IsClassDefaultBookmarked(const char* szClassName) const;
	virtual bool IsEntityBookmarked(EntityId entityId) const;
	virtual const char* GetBookmarkedClassName(EntityId entityId) const;
	virtual const char* GetBookmarkedEntityName(EntityId entityId) const;
	virtual bool IsPreparingEntity(SPreparingParams& outParams) const;
	//~IEntityPoolManager

	// Overload to force preparing an entity from a pool using the given poolId (for serialization)
	bool PrepareFromPool(EntityId entityId, EntityId forcePoolId, bool bCallInit, bool bPrepareNow = false);
	CEntity* PrepareDynamicFromPool(SEntitySpawnParams &params, bool bCallInit);
	bool ReturnToPool(EntityId entityId, bool bSaveState, bool bCallInit);
	bool IsClassForcedBookmarked(IEntityClass *pClass) const;

	bool IsUsingPools() const;
	bool IsClassUsingPools(IEntityClass *pClass) const;
	uint32 GetPoolSizeByClass(const char* szClassName) const;

	void Reset();
	void Update();
	bool OnLoadLevel(const char* szLevelPath);
	void OnLevelLoadStart();
	bool CreatePools();

	void OnPoolEntityInUse(CEntity *pEntity, EntityId prevId);

	// Pool bookmark usage
	bool AddPoolBookmark(SEntityLoadParams &loadParams, bool bIsDynamicEntity = false);
	bool RemovePoolBookmark(EntityId entityId);
	bool UpdatePoolBookmark(EntityId entityId, EntityId poolId);
	bool AddAttachmentToBookmark(EntityId entityId, const SEntityAttachment &entityAttachment);

	float GetReturnToPoolWeight(CEntity *pEntity, bool bIsUrgent) const;

	void DebugDraw() const;
	void DumpEntityBookmarks(const char* szFilterName = 0, bool bWriteSIDToDisk = false) const;

private:
	// Entity bookmark definition - Unique info about a level-placed Entity which is set to use pools
	struct SEntityBookmark
	{
		SEntityLoadParams loadParams;
		string sName, sLayerName;
		_smart_ptr<ISerializedObject> pLastState;
		EntityId inUseId;
		EntityId savedPoolId;
		TEntityPoolId owningPoolId;
		tAIObjectID aiObjectId;
		bool bIsDynamic;

		typedef std::vector<SEntityAttachment> TQueuedAttachments;
		TQueuedAttachments queuedAttachments;

		SEntityBookmark(bool bIsDynamicEntity, ISerializeHelper *pSerializeHelper);
		SEntityBookmark(const SEntityLoadParams &_loadParams, bool bIsDynamicEntity, ISerializeHelper *pSerializeHelper);

		void Serialize(TSerialize ser);
		void PreSerialize(bool bIsReading);
	};

	// Entity prepare request - Used for queuing up requests to spread them out over multiple frames
	struct SEntityPrepareRequest
	{
		EntityId entityId;
		EntityId forcePoolId;
		bool bCallInit;
		bool bIsDynamic;

		SEntityPrepareRequest() : entityId(0), forcePoolId(0), bCallInit(false), bIsDynamic(false) {}
		SEntityPrepareRequest(EntityId entityId, EntityId forcePoolId, bool bCallInit, bool bIsDynamic)
			: entityId(entityId), forcePoolId(forcePoolId), bCallInit(bCallInit), bIsDynamic(bIsDynamic) {}

		void Serialize(TSerialize ser);
	};
	typedef std::queue<SEntityPrepareRequest> TPrepareRequestQueue;

	// Dynamic entity minimal data needed to remake it on savegame load
	struct SDynamicSerializeHelper
	{
		string sName;
		string sLayerName;
		string sClass;
		string sArchetype;
		EntityGUID guid;
		int flags;

		SDynamicSerializeHelper() : flags(0) {}
		SDynamicSerializeHelper(const SEntityBookmark &bookmark);
		void Serialize(TSerialize ser);
		void InitLoadParams(SEntityLoadParams &outLoadParams, CEntitySystem *pEntitySystem) const;
	};
	
private:
	// Pool definition loading
	bool LoadDefinitions();
	void ReleaseDefinitions();

	// Bookmark serialization helpers
	void SerializeBookmarkEntity(SEntityBookmark &bookmark, CEntity *pEntity, bool bIsLoading) const;
	static bool OnBookmarkEntitySerialize(TSerialize serialize, void *pVEntity);

	bool PreparePools();
	void ReleasePools();
	bool LoadBookmarkedFromPool(EntityId entityId, EntityId forcedPoolId = 0, bool bCallInit = true);
	
	bool ProcessPrepareRequest(const SEntityPrepareRequest& request);
	
	CEntity* PrepareDynamicFromPool(SEntityBookmark &bookmark, EntityId forcePoolId, bool bCallInit);
	CEntity* LoadDynamicFromPool(SEntityBookmark &bookmark, EntityId forcedPoolId = 0, bool bCallInit = true);

	CEntity* GetPoolEntity(IEntityClass *pClass, EntityId forcedPoolId, TEntityPoolId &outOwningPoolId, bool &outHasAI, bool &outIsActive);

	// Listeners
	typedef CListenerSet<IEntityPoolListener*> TListenerSet;
	typedef std::vector<TListenerSet> TListeners;
	TListenerSet& GetListenerSet(IEntityPoolListener::ESubscriptions subscription);
	void NotifyEntityPreparedFromPool(EntityId entityId);
	
	// Creates a table from an Xml node, with child tables for child elements
	static bool CreateTableFromXml(XmlNodeRef pXml, SmartScriptTable &outTable);

private:
	bool m_bEnabled;
	XmlNodeRef m_pRootSID;
	CEntitySystem *m_pEntitySystem;
	_smart_ptr<ISerializeHelper> m_pSerializeHelper;

	TListeners m_Listeners;

	// Entity pools
	typedef std::map<TEntityPoolId, CEntityPool*> TEntityPools;
	TEntityPools m_EntityPools;

	// Pool definitions
	typedef std::map<TEntityPoolDefinitionId, CEntityPoolDefinition*> TPoolDefinitions;
	TPoolDefinitions m_PoolDefinitions;

	// Pool bookmark use
	typedef std::map<EntityId, SEntityBookmark> TPoolBookmarks;
	TPoolBookmarks m_PoolBookmarks;

	// Pool prepare requests
	TPrepareRequestQueue m_PrepareRequestQueue;
	uint32 m_uFramePrepareRequests;

	// current status
	SPreparingParams m_currentParams;
	bool m_preparingFromPool;
};

#endif //__ENTITYPOOLMANAGER_H__

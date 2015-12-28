/*************************************************************************
  Crytek Source File.
  Copyright (C), Crytek Studios, 2001-2010.
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Description: Manages AI proxies
  
 -------------------------------------------------------------------------
  History:
  - 16:04:2010: Created by Kevin Kirst

*************************************************************************/

#ifndef __AIPROXYMANAGER_H__
#define __AIPROXYMANAGER_H__

#include "IEntitySystem.h"
#include "IAIActorProxy.h"
#include "IAIGroupProxy.h"

class CAIProxyManager
	: public IEntitySystemSink
	, public IAIActorProxyFactory
	, public IAIGroupProxyFactory
{
public:
	CAIProxyManager();
	virtual ~CAIProxyManager();

	void Init();
	void Shutdown();

	IAIActorProxy *GetAIActorProxy(EntityId entityid) const;
	void OnAIProxyDestroyed(IAIActorProxy *pProxy);

	// IAIActorProxyFactory
	virtual IAIActorProxy* CreateActorProxy(EntityId entityID);
	// ~IAIActorProxyFactory

	// IAIGroupProxyFactory
	virtual IAIGroupProxy* CreateGroupProxy(int groupID);
	// ~IAIGroupProxyFactory

	// IEntitySystemSink
	virtual bool OnBeforeSpawn( SEntitySpawnParams &params );
	virtual void OnSpawn( IEntity *pEntity,SEntitySpawnParams &params );
	virtual bool OnRemove( IEntity *pEntity );
	virtual void OnReused( IEntity *pEntity, SEntitySpawnParams &params );
	virtual void OnEvent( IEntity *pEntity, SEntityEvent &event );
	//~IEntitySystemSink

private:
	typedef std::map< EntityId, CAIProxy* > TAIProxyMap;
	TAIProxyMap m_aiProxyMap;
};

#endif //__AIPROXYMANAGER_H__

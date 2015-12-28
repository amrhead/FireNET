// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.


#ifndef _CUSTOMACTIONMANAGER_H_
#define _CUSTOMACTIONMANAGER_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <IFlowSystem.h>
#include <ICustomActions.h>
#include <CryListenerSet.h>
#include "CustomAction.h"
#include <IEntityPoolManager.h>

///////////////////////////////////////////////////
// CCustomActionManager keeps track of all CustomActions
///////////////////////////////////////////////////
class CCustomActionManager : public ICustomActionManager, public IEntityPoolListener
{
public:
	CCustomActionManager();
	virtual ~CCustomActionManager();

public:
	// ICustomActionManager
	VIRTUAL bool StartAction( IEntity* pObject, const char* szCustomActionGraphName, ICustomActionListener* pListener = NULL );
	VIRTUAL bool SucceedAction( IEntity* pObject, const char* szCustomActionGraphName, ICustomActionListener* pListener = NULL );
	VIRTUAL bool SucceedWaitAction( IEntity* pObject );
	VIRTUAL bool SucceedWaitCompleteAction( IEntity* pObject );
	VIRTUAL bool AbortAction( IEntity* pObject );
	VIRTUAL bool EndAction( IEntity* pObject, bool bSuccess );
	VIRTUAL void LoadLibraryActions( const char* sPath );
	VIRTUAL void ClearActiveActions();
	VIRTUAL void ClearLibraryActions();
	VIRTUAL size_t GetNumberOfCustomActionsFromLibrary() const { return m_actionsLib.size(); }
	VIRTUAL ICustomAction* GetCustomActionFromLibrary( const char* szCustomActionGraphName );
	VIRTUAL ICustomAction* GetCustomActionFromLibrary( const size_t index );
	VIRTUAL size_t GetNumberOfActiveCustomActions() const;
	VIRTUAL ICustomAction* GetActiveCustomAction( const IEntity* pObject );
	VIRTUAL ICustomAction* GetActiveCustomAction( const size_t index );
	VIRTUAL bool UnregisterListener( ICustomActionListener* pEventListener );
	VIRTUAL void Serialize( TSerialize ser );
	// ~ICustomActionManager

	// IEntityPoolListener
	virtual void OnEntityReturningToPool(EntityId entityId, IEntity *pEntity);
	// ~IEntityPoolListener

	// Removes deleted Action from the list of active actions
	void Update();

	void GetMemoryUsage(ICrySizer *pSizer ) const
	{
		SIZER_COMPONENT_NAME(pSizer, "CustomActionManager");

		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_actionsLib);
		pSizer->AddObject(m_activeActions);
	}

protected:
	// Adds an Action in the list of active actions
	ICustomAction* AddActiveCustomAction( IEntity* pObject, const char* szCustomActionGraphName, ICustomActionListener* pListener = NULL );

	// Called when entity is removed
	void OnEntityRemove( IEntity* pEntity );

private:
	// Library of all defined Actions
	typedef std::map<string, CCustomAction> TCustomActionsLib;
	TCustomActionsLib m_actionsLib;

	// List of all active Actions (including suspended and to be deleted)
	typedef std::list<CCustomAction> TActiveActions;
	TActiveActions m_activeActions;
};

#endif

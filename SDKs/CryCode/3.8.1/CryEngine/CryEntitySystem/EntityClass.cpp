////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   EntityClass.cpp
//  Version:     v1.00
//  Created:     18/5/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EntityClass.h"
#include "EntityScript.h"

//////////////////////////////////////////////////////////////////////////
CEntityClass::CEntityClass()
{
	m_pfnUserProxyCreate = NULL;
	m_pUserProxyUserData = NULL;
	m_pPropertyHandler = NULL;
	m_pEventHandler = NULL;
	m_pScriptFileHandler = NULL;

	m_pEntityScript = NULL;
	m_bScriptLoaded = false;
}

//////////////////////////////////////////////////////////////////////////
CEntityClass::~CEntityClass()
{
	SAFE_RELEASE(m_pEntityScript);
}

//////////////////////////////////////////////////////////////////////////
IScriptTable* CEntityClass::GetScriptTable() const
{
	IScriptTable *pScriptTable = NULL;

	if (m_pEntityScript)
	{
		CEntityScript *pScript = (CEntityScript*)m_pEntityScript;
		pScriptTable = (pScript ? pScript->GetScriptTable() : NULL);
	}

	return pScriptTable;
}

//////////////////////////////////////////////////////////////////////////
bool CEntityClass::LoadScript( bool bForceReload )
{
	bool bRes = true;
	if (m_pEntityScript)
	{
		CEntityScript *pScript = (CEntityScript*)m_pEntityScript;
		bRes = pScript->LoadScript(bForceReload);

		m_bScriptLoaded = true;
	}

	if (m_pScriptFileHandler && bForceReload)
		m_pScriptFileHandler->ReloadScriptFile();

	if (m_pPropertyHandler && bForceReload)
		m_pPropertyHandler->RefreshProperties();

	if (m_pEventHandler && bForceReload)
		m_pEventHandler->RefreshEvents();

	return bRes;
}

/////////////////////////////////////////////////////////////////////////
int CEntityClass::GetEventCount()
{
	if (m_pEventHandler)
		return m_pEventHandler->GetEventCount();

	if (!m_bScriptLoaded)
		LoadScript(false);
	
	if (!m_pEntityScript)
		return 0;

	return ((CEntityScript*)m_pEntityScript)->GetEventCount();
}

//////////////////////////////////////////////////////////////////////////
CEntityClass::SEventInfo CEntityClass::GetEventInfo( int nIndex )
{
	SEventInfo info;

	if (m_pEventHandler)
	{
		IEntityEventHandler::SEventInfo eventInfo;

		if (m_pEventHandler->GetEventInfo(nIndex, eventInfo))
		{
			info.name = eventInfo.name;
			info.bOutput = (eventInfo.type == IEntityEventHandler::Output);

			switch (eventInfo.valueType)
			{
			case IEntityEventHandler::Int:
				info.type = EVT_INT;
				break;
			case IEntityEventHandler::Float:
				info.type = EVT_FLOAT;
				break;
			case IEntityEventHandler::Bool:
				info.type = EVT_BOOL;
				break;
			case IEntityEventHandler::Vector:
				info.type = EVT_VECTOR;
				break;
			case IEntityEventHandler::Entity:
				info.type = EVT_ENTITY;
				break;
			case IEntityEventHandler::String:
				info.type = EVT_STRING;
				break;
			default:
				assert(0);
				break;
			}
			info.type = (EventValueType)eventInfo.valueType;
		}
		else
		{
			info.name = "";
			info.bOutput = false;
		}

		return info;
	}

	if (!m_bScriptLoaded)
		LoadScript(false);

	assert( nIndex >= 0 && nIndex < GetEventCount() );

	if (m_pEntityScript)
	{
		const SEntityScriptEvent &scriptEvent = ((CEntityScript*)m_pEntityScript)->GetEvent( nIndex );
		info.name = scriptEvent.name.c_str();
		info.bOutput = scriptEvent.bOutput;
		info.type = scriptEvent.valueType;
	}
	else
	{
		info.name = "";
		info.bOutput = false;
	}

	return info;
}

//////////////////////////////////////////////////////////////////////////
bool CEntityClass::FindEventInfo( const char *sEvent, SEventInfo &event )
{
	if (!m_bScriptLoaded)
		LoadScript(false);

	if (!m_pEntityScript)
		return false;

	const SEntityScriptEvent *pScriptEvent = ((CEntityScript*)m_pEntityScript)->FindEvent( sEvent );
	if (!pScriptEvent)
		return false;

	event.name = pScriptEvent->name.c_str();
	event.bOutput = pScriptEvent->bOutput;
	event.type = pScriptEvent->valueType;

	return true;
}

//////////////////////////////////////////////////////////////////////////
TEntityAttributeArray& CEntityClass::GetClassAttributes()
{
	return m_classAttributes;
}

//////////////////////////////////////////////////////////////////////////
const TEntityAttributeArray& CEntityClass::GetClassAttributes() const
{
	return m_classAttributes;
}

//////////////////////////////////////////////////////////////////////////
TEntityAttributeArray& CEntityClass::GetEntityAttributes()
{
	return m_entityAttributes;
}

//////////////////////////////////////////////////////////////////////////
const TEntityAttributeArray& CEntityClass::GetEntityAttributes() const
{
	return m_entityAttributes;
}

void CEntityClass::SetName( const char *sName )
{
	m_sName = sName;
}

void CEntityClass::SetScriptFile( const char *sScriptFile )
{
	m_sScriptFile = sScriptFile;
}

void CEntityClass::SetEntityScript( IEntityScript *pScript )
{
	m_pEntityScript = pScript;
}

void CEntityClass::SetUserProxyCreateFunc( UserProxyCreateFunc pFunc, void *pUserData/*=NULL */ )
{
	m_pfnUserProxyCreate = pFunc;
	m_pUserProxyUserData=pUserData;
}

void CEntityClass::SetPropertyHandler( IEntityPropertyHandler* pPropertyHandler )
{
	m_pPropertyHandler = pPropertyHandler;
}

void CEntityClass::SetEventHandler( IEntityEventHandler* pEventHandler )
{
	m_pEventHandler = pEventHandler;
}

void CEntityClass::SetScriptFileHandler( IEntityScriptFileHandler* pScriptFileHandler )
{
	m_pScriptFileHandler = pScriptFileHandler;
}

IEntityPropertyHandler* CEntityClass::GetPropertyHandler() const
{
	return m_pPropertyHandler;
}

IEntityEventHandler* CEntityClass::GetEventHandler() const
{
	return m_pEventHandler;
}

IEntityScriptFileHandler* CEntityClass::GetScriptFileHandler() const
{
	return m_pScriptFileHandler;
}

const SEditorClassInfo& CEntityClass::GetEditorClassInfo() const
{
	return m_EditorClassInfo;
}

void CEntityClass::SetEditorClassInfo(const SEditorClassInfo& editorClassInfo)
{
	m_EditorClassInfo = editorClassInfo;
}

//////////////////////////////////////////////////////////////////////////
void CEntityClass::SetEntityAttributes( const TEntityAttributeArray& attributes )
{
	m_entityAttributes = attributes;
}

//////////////////////////////////////////////////////////////////////////
void CEntityClass::SetClassAttributes( const TEntityAttributeArray& attributes )
{
	m_classAttributes = attributes;
}



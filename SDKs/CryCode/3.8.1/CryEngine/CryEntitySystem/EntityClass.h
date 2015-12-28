////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   EntityClass.h
//  Version:     v1.00
//  Created:     18/5/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __EntityClass_h__
#define __EntityClass_h__
#pragma once

#include <IEntityClass.h>

//////////////////////////////////////////////////////////////////////////
// Description:
//    Standard implementation of the IEntityClass interface.
//////////////////////////////////////////////////////////////////////////
class CEntityClass : public IEntityClass
{
public:
	CEntityClass();
	virtual ~CEntityClass();
	
	//////////////////////////////////////////////////////////////////////////
	// IEntityClass interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void Release() { delete this; };

	virtual uint32 GetFlags() const { return m_nFlags; };
	virtual void SetFlags( uint32 nFlags ) { m_nFlags = nFlags; };
	
	virtual const char* GetName() const { return m_sName.c_str(); }
	virtual const char* GetScriptFile() const { return m_sScriptFile.c_str(); }

	virtual IEntityScript* GetIEntityScript() const { return m_pEntityScript; }
	virtual IScriptTable* GetScriptTable() const;
	virtual bool LoadScript( bool bForceReload );
	virtual UserProxyCreateFunc GetUserProxyCreateFunc() const { return m_pfnUserProxyCreate; };
	virtual void* GetUserProxyData() const { return m_pUserProxyUserData; };

	virtual IEntityPropertyHandler* GetPropertyHandler() const;
	virtual IEntityEventHandler* GetEventHandler() const;
	virtual IEntityScriptFileHandler* GetScriptFileHandler() const;

	virtual const SEditorClassInfo& GetEditorClassInfo() const;
	virtual void SetEditorClassInfo(const SEditorClassInfo& editorClassInfo);

	virtual int GetEventCount();
	virtual IEntityClass::SEventInfo GetEventInfo( int nIndex );
	virtual bool FindEventInfo( const char *sEvent,SEventInfo &event );

	virtual TEntityAttributeArray& GetClassAttributes() override;
	virtual const TEntityAttributeArray& GetClassAttributes() const override;
	virtual TEntityAttributeArray& GetEntityAttributes() override;
	virtual const TEntityAttributeArray& GetEntityAttributes() const override;

	//////////////////////////////////////////////////////////////////////////

	void SetName( const char *sName );
	void SetScriptFile( const char *sScriptFile );
	void SetEntityScript( IEntityScript *pScript );

	void SetUserProxyCreateFunc( UserProxyCreateFunc pFunc, void *pUserData=NULL );
	void SetPropertyHandler( IEntityPropertyHandler* pPropertyHandler );
	void SetEventHandler( IEntityEventHandler* pEventHandler );
	void SetScriptFileHandler( IEntityScriptFileHandler* pScriptFileHandler );
	void SetEntityAttributes( const TEntityAttributeArray& attributes );
	void SetClassAttributes( const TEntityAttributeArray& attributes );

	void GetMemoryUsage( ICrySizer *pSizer ) const
	{
		pSizer->AddObject(m_sName);
		pSizer->AddObject(m_sScriptFile);
		pSizer->AddObject(m_pEntityScript);
		pSizer->AddObject(m_pPropertyHandler);
		pSizer->AddObject(m_pEventHandler);
		pSizer->AddObject(m_pScriptFileHandler);
	}
private:
	uint32 m_nFlags;
	string m_sName;
	string m_sScriptFile;
	IEntityScript *m_pEntityScript;

	UserProxyCreateFunc	m_pfnUserProxyCreate;
	void*								m_pUserProxyUserData;

	bool m_bScriptLoaded;

	IEntityPropertyHandler* m_pPropertyHandler;
	IEntityEventHandler* m_pEventHandler;
	IEntityScriptFileHandler* m_pScriptFileHandler;

	SEditorClassInfo m_EditorClassInfo;

	TEntityAttributeArray	m_entityAttributes;
	TEntityAttributeArray	m_classAttributes;
};

#endif // __EntityClass_h__

////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2005.
// -------------------------------------------------------------------------
//  File name:   EntityArchetype.h
//  Version:     v1.00
//  Created:     19/9/2005 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EntityClass.h"
#include "EntityScript.h"
#include "EntityArchetype.h"
#include "ScriptProperties.h"
#include "CryPath.h"
#include "EntitySystem.h"

#define ENTITY_ARCHETYPES_LIBS_PATH "/Libs/EntityArchetypes/"

//////////////////////////////////////////////////////////////////////////
CEntityArchetype::CEntityArchetype( IEntityClass *pClass )
{
	assert( pClass );
	assert( pClass->GetScriptFile() );
	m_pClass = pClass;

	// Try to load the script if it is not yet valid.
	if (CEntityScript *pScript = (CEntityScript*)m_pClass->GetIEntityScript())
	{
		pScript->LoadScript();

		if (pScript->GetPropertiesTable())
		{
			m_pProperties.Create( gEnv->pScriptSystem );
			m_pProperties->Clone( pScript->GetPropertiesTable(),true,true );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityArchetype::LoadFromXML( XmlNodeRef &propertiesNode, XmlNodeRef &objectVarsNode )
{
	if (IEntityPropertyHandler* pPropertyHandler = m_pClass->GetPropertyHandler())
	{
		pPropertyHandler->LoadArchetypeXMLProperties(m_name, propertiesNode);
	}
	else
	{
		m_ObjectVars = objectVarsNode->clone();

		if (!m_pProperties)
			return;

		CScriptProperties scriptProps;
		// Initialize properties.
		scriptProps.Assign( propertiesNode,m_pProperties );

		// Here we add the additional archetype properties, which are not 
		// loaded before, as they are not supposed to have equivalent script
		// properties.
		// Still, we inject those properties into the script environment so
		// they will be available during entity creation.
		XmlNodeRef	rAdditionalProperties=propertiesNode->findChild("AdditionalArchetypeProperties");
		if (rAdditionalProperties)
		{
			const int			nNumberOfAttributes(rAdditionalProperties->getNumAttributes());
			int						nCurrentAttribute(0);

			const char*				  pszKey(NULL);
			const char*				  pszValue(NULL);

			const char*					pszCurrentValue(NULL);

			for (/*nCurrentAttribute=0*/;nCurrentAttribute<nNumberOfAttributes;++nCurrentAttribute)
			{
				rAdditionalProperties->getAttributeByIndex(nCurrentAttribute,&pszKey,&pszValue);
				if (!m_pProperties->GetValue(pszKey,pszCurrentValue))
				{
					m_pProperties->SetValue(pszKey,pszValue);
				}
				else
				{
					// In editor this will happen, but shouldn't generate a warning.
					if (!gEnv->IsEditor())
					{
						CryLog("[EntitySystem] CEntityArchetype::LoadFromXML: attribute %s couldn't be injected into the script, as it was already present there.",pszKey);
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
IEntityArchetype* CEntityArchetypeManager::CreateArchetype( IEntityClass *pClass,const char *sArchetype )
{
	CEntityArchetype *pArchetype = stl::find_in_map( m_nameToArchetypeMap,sArchetype,NULL );
	if (pArchetype && stricmp(pClass->GetName(),pArchetype->GetName())==0)
		return pArchetype;
	pArchetype = new CEntityArchetype( (CEntityClass*)pClass );
	pArchetype->SetName( sArchetype );
	m_nameToArchetypeMap[pArchetype->GetName()] = pArchetype;
	return pArchetype;
}

//////////////////////////////////////////////////////////////////////////
IEntityArchetype* CEntityArchetypeManager::FindArchetype( const char *sArchetype )
{
	CEntityArchetype *pArchetype = stl::find_in_map( m_nameToArchetypeMap,sArchetype,NULL );
	return pArchetype;
}

//////////////////////////////////////////////////////////////////////////
IEntityArchetype* CEntityArchetypeManager::LoadArchetype( const char *sArchetype )
{
	IEntityArchetype *pArchetype = FindArchetype( sArchetype );
	if (pArchetype)
		return pArchetype;

	const string& sLibName = GetLibraryFromName( sArchetype );

	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_ArchetypeLib, 0, sLibName.c_str());

	// If archetype is not found try to load the library first.
	if (LoadLibrary( sLibName ))
	{
		pArchetype = FindArchetype( sArchetype );
	}

	return pArchetype;
}

//////////////////////////////////////////////////////////////////////////
void CEntityArchetypeManager::Reset()
{
	MEMSTAT_LABEL_SCOPED("CEntityArchetypeManager::Reset");

	m_nameToArchetypeMap.clear();
	DynArray<string>().swap(m_loadedLibs);
}

//////////////////////////////////////////////////////////////////////////
string CEntityArchetypeManager::GetLibraryFromName( const string &sArchetypeName )
{
	string libname = sArchetypeName.SpanExcluding( "." );
	return libname;
}

//////////////////////////////////////////////////////////////////////////
bool CEntityArchetypeManager::LoadLibrary( const string &library )
{
	if (stl::find( m_loadedLibs,library ))
		return true;

	string filename;
	if(library == "Level")
	{
		filename = gEnv->p3DEngine->GetLevelFilePath("LevelPrototypes.xml");
	}
	else
	{
		filename = PathUtil::Make(PathUtil::GetGameFolder() + ENTITY_ARCHETYPES_LIBS_PATH, library, "xml");
	}

	XmlNodeRef rootNode = GetISystem()->LoadXmlFromFile( filename );
	if (!rootNode)
		return false;
	
	IEntityClassRegistry *pClassRegistry = GetIEntitySystem()->GetClassRegistry();
	// Load all archetypes from library.

	for (int i = 0; i < rootNode->getChildCount(); i++)
	{
		XmlNodeRef node = rootNode->getChild(i);
		if (node->isTag("EntityPrototype"))
		{
			const char *name = node->getAttr( "Name" );
			const char* className = node->getAttr( "Class" );

			IEntityClass *pClass = pClassRegistry->FindClass( className );
			if (!pClass)
			{
				// No such entity class.
				EntityWarning( "EntityArchetype %s references unknown entity class %s",name,className );
				continue;
			}

			string fullname = library + "." + name;
			IEntityArchetype *pArchetype = CreateArchetype( pClass,fullname );
			if (!pArchetype)
				continue;

			// Load properties.
			XmlNodeRef props = node->findChild( "Properties" );
			XmlNodeRef objVars = node->findChild( "ObjectVars" );
			if (props)
			{
				pArchetype->LoadFromXML( props, objVars );
			}
		}
	}


	// Add this library to the list of loaded archetype libs.
	m_loadedLibs.push_back(library);

	return true;
}



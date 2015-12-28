// CryEngine Source File.
// Copyright (C), Crytek, 1999-2014.


#include "StdAfx.h"
#include "XmlLoadGame.h"
#include "XmlSerializeHelper.h"

#include <IPlatformOS.h>

//#define XML_LOADGAME_USE_COMPRESSION   // must be in sync with XmlSaveGame.cpp
//#undef  XML_LOADGAME_USE_COMPRESSION   // undef because does NOT work with aliases yet

struct CXmlLoadGame::Impl
{
	XmlNodeRef root;
	XmlNodeRef metadata;
	string		 inputFile;

	std::vector<_smart_ptr<CXmlSerializeHelper> > sections;
};

CXmlLoadGame::CXmlLoadGame() : m_pImpl(new Impl)
{
}

CXmlLoadGame::~CXmlLoadGame()
{
}

bool CXmlLoadGame::Init( const char * name )
{
#ifdef XML_LOADGAME_USE_COMPRESSION
	#if defined(XENON) || defined(PS3)
		CryWarning(VALIDATOR_MODULE_GAME,VALIDATOR_ERROR, "CXmlLoadGame::Init  not supported yet, strings cannot grow beyond 32767 chars\n");
		return false;
	#endif

	const unsigned int nFileSizeBits = GetISystem()->GetCompressedFileSize(name);
	const unsigned int nFileSizeBytes = nFileSizeBits / 8 + ((nFileSizeBits & 7) + 7) / 8;
	if (nFileSizeBytes <= 0)
	{
		return false;
	}

	char* const pXmlData = new char[nFileSizeBytes+16];
	GetISystem()->ReadCompressedFile(name, pXmlData,nFileSizeBits);

	m_pImpl->root = GetISystem()->LoadXmlFromBuffer(pXmlData, nFileSizeBytes);

	delete []pXmlData;
#else
	if (GetISystem()->GetPlatformOS()->UsePlatformSavingAPI() )
	{
		IPlatformOS::ISaveReaderPtr pSaveReader = GetISystem()->GetPlatformOS()->SaveGetReader(name);
		if (!pSaveReader)
		{
			return false;
		}

		size_t nFileSize;

		if ((pSaveReader->GetNumBytes(nFileSize) == IPlatformOS::eFOC_Failure) || (nFileSize <= 0))
		{
			return false;
		}

		std::vector<char> xmlData;
		xmlData.resize(nFileSize);
		
		if (pSaveReader->ReadBytes(&xmlData[0], nFileSize) == IPlatformOS::eFOC_Failure)
		{
			return false;
		}

		m_pImpl->root = GetISystem()->LoadXmlFromBuffer(&xmlData[0], nFileSize);
	}
	else
	{
		m_pImpl->root = GetISystem()->LoadXmlFromFile(name);
	}
#endif

	if (!m_pImpl->root)
		return false;

	m_pImpl->inputFile = name;

	m_pImpl->metadata = m_pImpl->root->findChild("Metadata");
	if (!m_pImpl->metadata)
		return false;

	return true;
}

bool CXmlLoadGame::Init( const XmlNodeRef& root, const char * fileName )
{
	m_pImpl->root = root;
	if (!m_pImpl->root)
		return false;

	m_pImpl->inputFile = fileName;

	m_pImpl->metadata = m_pImpl->root->findChild("Metadata");
	if (!m_pImpl->metadata)
		return false;

	return true;
}

const char * CXmlLoadGame::GetMetadata( const char * tag )
{
	return m_pImpl->metadata->getAttr(tag);
}

bool CXmlLoadGame::GetMetadata( const char * tag, int& value )
{
	return m_pImpl->metadata->getAttr(tag, value);
}

bool CXmlLoadGame::HaveMetadata( const char * tag )
{
	return m_pImpl->metadata->haveAttr(tag);
}

std::auto_ptr<TSerialize> CXmlLoadGame::GetSection( const char * section )
{
	XmlNodeRef node = m_pImpl->root->findChild(section);
	if (!node)
		return std::auto_ptr<TSerialize>();

	_smart_ptr<CXmlSerializeHelper> pSerializer = new CXmlSerializeHelper;
	m_pImpl->sections.push_back( pSerializer );
	return std::auto_ptr<TSerialize>( new TSerialize(pSerializer->GetReader(node)) );
}

bool CXmlLoadGame::HaveSection( const char * section )
{
	return m_pImpl->root->findChild(section) != 0;
}

void CXmlLoadGame::Complete()
{
	delete this;
}

const char *CXmlLoadGame::GetFileName() const
{
	if(m_pImpl.get())
		return m_pImpl.get()->inputFile;
	return NULL;
}

////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2006.
// -------------------------------------------------------------------------
//  File name:   ItemString.h
//  Version:     v1.00
//  Created:     20/02/2007 by AlexL
//  Compilers:   Visual Studio.NET
//  Description: Special version of CCryName
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __ITEMSTRING_H__
#define __ITEMSTRING_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include <ISystem.h>
#include <StlUtils.h>

#if !defined(_RELEASE)
	#define SHARED_STRING_TRACK_LEVEL_HEAP_LEAKS	1
#else
	#define SHARED_STRING_TRACK_LEVEL_HEAP_LEAKS	0
#endif

namespace SharedString
{
	// Name entry header, immediately after this header in memory starts actual string data.
	struct SNameEntry
	{
		int nRefCount;		// Reference count of this string.
		int nLength;			// Current length of string.
		int nAllocSize;		// Size of memory allocated at the end of this class.
#if SHARED_STRING_TRACK_LEVEL_HEAP_LEAKS
		bool allocatedOnLevelHeap;
#endif

		// Here in memory starts character buffer of size nAllocSize.
		//char data[nAllocSize]

		char* GetStr()  { return (char*)(this+1); }
		void  AddRef()  { ++nRefCount; };
		int   Release() { return --nRefCount; };
	};

	//////////////////////////////////////////////////////////////////////////
	class CNameTable
	{
	public:
		CNameTable() 
		{
#if SHARED_STRING_TRACK_LEVEL_HEAP_LEAKS
			m_trackLevelHeapAllocs = false;
#endif
		}

		~CNameTable()
		{
		}

		// Only finds an existing name table entry, return 0 if not found.
		SNameEntry* FindEntry( const char *str )
		{
			SNameEntry *pEntry = stl::find_in_map( m_nameMap,str,0 );
			return pEntry;
		}

		// Finds an existing name table entry, or creates a new one if not found.
		SNameEntry* GetEntry( const char *str )
		{
			SNameEntry *pEntry = stl::find_in_map( m_nameMap,str,0 );
			if (!pEntry)
			{
				// Create a new entry.
				unsigned int nLen = strlen(str);
				unsigned int allocLen = sizeof(SNameEntry) + (nLen+1)*sizeof(char);
				pEntry = (SNameEntry*)malloc( allocLen );
				pEntry->nRefCount = 0;
				pEntry->nLength = nLen;
				pEntry->nAllocSize = allocLen;
#if SHARED_STRING_TRACK_LEVEL_HEAP_LEAKS
				pEntry->allocatedOnLevelHeap = (m_trackLevelHeapAllocs);
#endif
				// Copy string to the end of name entry.
				memcpy( pEntry->GetStr(),str,nLen+1 );

				// put in map.
				m_nameMap.insert( NameMap::value_type(pEntry->GetStr(),pEntry) );
			}
			return pEntry;
		}

		// Release existing name table entry.
		void Release( SNameEntry *pEntry )
		{
			assert(pEntry);
			m_nameMap.erase( pEntry->GetStr() );
			free(pEntry);
		}

		void Dump()
		{
			CryLogAlways("NameTable: %" PRISIZE_T " entries", m_nameMap.size());
			NameMap::const_iterator iter = m_nameMap.begin();
			NameMap::const_iterator iterEnd = m_nameMap.end();
			while (iter != iterEnd)
			{
				CryLogAlways("'%s'", iter->first);
				++iter;
			}
		}

#if SHARED_STRING_TRACK_LEVEL_HEAP_LEAKS
		void TrackLevelHeapAllocs(bool trackAllocs)
		{
			m_trackLevelHeapAllocs = trackAllocs;
		}

		void DumpLevelHeapLeakedStrings()
		{
			NameMap::const_iterator iter = m_nameMap.begin();
			NameMap::const_iterator iterEnd = m_nameMap.end();
			while (iter != iterEnd)
			{
				if (iter->second->allocatedOnLevelHeap)
				{
					CRY_ASSERT_TRACE(false, ("Level allocated SharedString leaking '%s'", iter->first));
					CryLogAlways("Level allocated SharedString leaking '%s' : Ref count: '%d'", iter->first, iter->second->nRefCount);
				}
				++iter;
			}
		}
#endif

	private:
		typedef stl::hash_map<const char*,SNameEntry*, stl::hash_strcmp<const char*> > NameMap;
		NameMap m_nameMap;
#if SHARED_STRING_TRACK_LEVEL_HEAP_LEAKS
		bool	m_trackLevelHeapAllocs;
#endif
	};

	///////////////////////////////////////////////////////////////////////////////
	// Class CSharedString.
	//////////////////////////////////////////////////////////////////////////
	class	CSharedString
	{
	public:
		CSharedString();
		CSharedString( const CSharedString& n );
		CSharedString( const char *s );
		CSharedString( const char *s,bool bOnlyFind );
		~CSharedString();

		CSharedString& operator=( const CSharedString& n );
		CSharedString& operator=( const char *s );

		operator bool() { return m_str != 0; }
		operator bool() const { return m_str != 0; }
		bool operator!() { return m_str == 0; }
		bool operator!() const { return m_str == 0; }

		bool	operator==( const CSharedString &n ) const;
		bool	operator!=( const CSharedString &n ) const;

		bool	operator==( const char *s ) const;
		bool	operator!=( const char *s ) const;

		bool	operator<( const CSharedString &n ) const;
		bool	operator>( const CSharedString &n ) const;

		bool	empty() const { return length() == 0; }
		void	reset()	{	_release(m_str);	m_str = 0; }
		void  clear() { reset(); }
		const	char*	c_str() const { return (m_str) ? m_str: ""; }
		int	length() const { return _length(); };

		static bool find( const char *str ) { return GetNameTable()->FindEntry(str) != 0; }

		static void DumpNameTable()
		{
			GetNameTable()->Dump();
		}

#if SHARED_STRING_TRACK_LEVEL_HEAP_LEAKS
		static void TrackLevelHeapAllocs(bool trackAllocs)
		{
			GetNameTable()->TrackLevelHeapAllocs(trackAllocs);
		}

		static void DumpLevelHeapLeakedStrings()
		{
			GetNameTable()->DumpLevelHeapLeakedStrings();
		}
#endif

		void GetMemoryUsage( ICrySizer *pSizer ) const 
		{
			/*nothing for now*/
		}	
	private:
		operator int () {return 0;}
		operator int () const {return 0;}

		static CNameTable* GetNameTable()
		{
			static CNameTable table;
			return &table;
		}


		SNameEntry* _entry( const char *pBuffer ) const { assert(pBuffer); return ((SNameEntry*)pBuffer)-1; }
		int  _length() const { return (m_str) ? _entry(m_str)->nLength : 0; };
		void _addref( const char *pBuffer ) { if (pBuffer) _entry(pBuffer)->AddRef(); }
		void _release( const char *pBuffer )
		{
			if (pBuffer && _entry(pBuffer)->Release() <= 0)
			{
				if (CNameTable* pNT = GetNameTable())
					pNT->Release(_entry(pBuffer));
			}
		}

		const char *m_str;
	};

	//////////////////////////////////////////////////////////////////////////
	inline CSharedString::CSharedString()
	{
		m_str = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	inline CSharedString::CSharedString( const CSharedString& n )
	{
		_addref( n.m_str );
		m_str = n.m_str;
	}

	//////////////////////////////////////////////////////////////////////////
	inline CSharedString::CSharedString( const char *s )
	{
		m_str = 0;
		*this = s;
	}

	//////////////////////////////////////////////////////////////////////////
	inline CSharedString::CSharedString( const char *s,bool bOnlyFind )
	{
		assert(s);
		m_str = 0;
		if (*s) // if not empty
		{
			SNameEntry *pNameEntry = GetNameTable()->FindEntry(s);
			if (pNameEntry)
			{
				m_str = pNameEntry->GetStr();
				_addref(m_str);
			}
		}
	}

	inline CSharedString::~CSharedString()
	{
		_release(m_str);
	}

	//////////////////////////////////////////////////////////////////////////
	inline CSharedString&	CSharedString::operator=( const CSharedString &n )
	{
		if (m_str != n.m_str)
		{
			_release(m_str);
			m_str = n.m_str;
			_addref(m_str);
		}
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	inline CSharedString&	CSharedString::operator=( const char *s )
	{
		//assert(s); // AlexL: we currenly allow 0 assignment because Items currently do this a lot
		const char *pBuf = 0;
		if (s && *s) // if not empty
		{
			pBuf = GetNameTable()->GetEntry(s)->GetStr();
		}
		else if (s == 0)
		{
			; // debugging here
		}
		if (m_str != pBuf)
		{
			_release(m_str);
			m_str = pBuf;
			_addref(m_str);
		}
		return *this;
	}


	//////////////////////////////////////////////////////////////////////////
	inline bool	CSharedString::operator==( const CSharedString &n ) const {
		return m_str == n.m_str;
	}

	inline bool	CSharedString::operator!=( const CSharedString &n ) const {
		return !(*this == n);
	}

	inline bool	CSharedString::operator==( const char* str ) const {
		return m_str && strcmp(m_str,str) == 0;
	}

	inline bool	CSharedString::operator!=( const char* str ) const {
		if (!m_str)
			return true;
		return strcmp(m_str,str) != 0;
	}

	inline bool	CSharedString::operator<( const CSharedString &n ) const {
		return m_str < n.m_str;
	}

	inline bool	CSharedString::operator>( const CSharedString &n ) const {
		return m_str > n.m_str;
	}

}; // _ItemString


#define ITEM_USE_SHAREDSTRING

#ifdef ITEM_USE_SHAREDSTRING
typedef SharedString::CSharedString ItemString;
#define CONST_TEMPITEM_STRING(a) a
#else
typedef string ItemString;
#define CONST_TEMPITEM_STRING(a) CONST_TEMP_STRING(a)
#endif

#endif

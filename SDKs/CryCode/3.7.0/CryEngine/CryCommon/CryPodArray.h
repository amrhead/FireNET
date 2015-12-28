////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   CryPodArray.h
//  Created:     28/5/2002 by Vladimir Kajalin
//  Description: Simple POD types container
// -------------------------------------------------------------------------
//  History:
//      Refactored 07/03/2007 by Timur.
//
////////////////////////////////////////////////////////////////////////////

#ifndef __CRY_POD_ARRAY_H__
#define __CRY_POD_ARRAY_H__
#pragma once

//////////////////////////////////////////////////////////////////////////
// POD Array
// vector like class (random access O(1)) without construction/destructor/copy constructor/assignment handling
// over-allocation allows safe prefetching where required without worrying about memory page boundaries
//////////////////////////////////////////////////////////////////////////
template <class T, size_t overAllocBytes = 0>
class PodArray
{
  T * m_pElements;
  int m_nCount;
  int m_nAllocatedCount;

public:
	typedef T        value_type;
	typedef T*       iterator;
	typedef const T* const_iterator;

	//////////////////////////////////////////////////////////////////////////
	// STL compatible interface
	//////////////////////////////////////////////////////////////////////////
	void resize (size_t numElements)
	{
		int nOldCount = m_nCount;
		PreAllocate ((int)numElements, (int)numElements);
		assert (numElements == size());
		for (int nElement = nOldCount; nElement < m_nCount; ++nElement)
			new (&(*this)[nElement]) T;
		MEMSTAT_USAGE(begin(), sizeof(T) * size());
	}
	//////////////////////////////////////////////////////////////////////////
	ILINE void reserve (unsigned numElements) { PreAllocate ((int)numElements); }
	ILINE void push_back (const T& rElement) { Add (rElement); }
	ILINE size_t size() const	{ return (size_t)m_nCount; }
	ILINE size_t capacity() const { return (size_t)m_nAllocatedCount; }

	ILINE void clear() { Clear(); }
	ILINE T* begin() { return m_pElements; }
	ILINE T* end() { return m_pElements+m_nCount; }
	ILINE const T* begin()const { return m_pElements; }
	ILINE const T* end()const { return m_pElements+m_nCount; }
	ILINE bool empty() const { return m_nCount==0; }

	ILINE const T& front()const { assert(m_nCount>0); return m_pElements[0]; }
	ILINE T& front() { assert(m_nCount>0); return m_pElements[0]; }

	ILINE const T& back()const { assert(m_nCount>0); return m_pElements[m_nCount-1]; }
	ILINE T& back() { assert(m_nCount>0); return m_pElements[m_nCount-1]; }
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	PodArray() : m_nCount(0),m_pElements(0),m_nAllocatedCount(0)
	{
		MEMSTAT_REGISTER_CONTAINER(this, EMemStatContainerType::MSC_Vector, T);
	}
  PodArray(int elem_count, int nNewCount = 0) : m_nCount(0),m_pElements(0),m_nAllocatedCount(0) 
	{
		MEMSTAT_REGISTER_CONTAINER(this, EMemStatContainerType::MSC_Vector, T);
		PreAllocate(elem_count, nNewCount); 
	}
	PodArray( const PodArray<T> &from ) : m_nCount(0),m_pElements(0),m_nAllocatedCount(0)
	{
		MEMSTAT_REGISTER_CONTAINER(this, EMemStatContainerType::MSC_Vector, T);
		AddList( from );
	}
  ~PodArray()
	{
		Free(); 
		MEMSTAT_UNREGISTER_CONTAINER(this);
	}

  void Reset() { Free(); }
	void Free()
  {    
    if(m_pElements)
		{
			size_t nOldSizeInBytes = (m_nAllocatedCount*sizeof(T)) + overAllocBytes;
      InternalRealloc(m_pElements,0,nOldSizeInBytes);
		}
    m_pElements=0;
    m_nCount=0; 
    m_nAllocatedCount= 0; 
  }

  ILINE void Clear()
	{ 
		m_nCount=0;
		MEMSTAT_USAGE(begin(), (sizeof(T) * size()) + overAllocBytes);
	}

  int Find(const T & p)
  {
    for(int i=0; i<m_nCount; i++)
      if(p == (*this)[i])
        return i;

    return -1;;
  }

  inline void AddList(const PodArray<T> & lstAnother)
  {
    PreAllocate(m_nCount + lstAnother.Count());

    memcpy(&m_pElements[m_nCount],&lstAnother.m_pElements[0],sizeof(m_pElements[0])*lstAnother.Count());

    m_nCount += lstAnother.Count();
		MEMSTAT_USAGE(begin(), (sizeof(T) * size()) + overAllocBytes);
  }

  inline void AddList(T * pAnotherArray, int nAnotherCount)
  {
    PreAllocate(m_nCount + nAnotherCount);

    memcpy(&m_pElements[m_nCount],pAnotherArray,sizeof(m_pElements[0])*nAnotherCount);

    m_nCount += nAnotherCount;
		MEMSTAT_USAGE(begin(), (sizeof(T) * size()) + overAllocBytes);
  }

  ILINE void Add(const T & p)
  {
    if( m_nCount >= m_nAllocatedCount )
    {
      assert(&p<m_pElements || &p>=(m_pElements+m_nAllocatedCount));
			size_t nOldSizeInBytes = (m_nAllocatedCount*sizeof(T)) + overAllocBytes;
      m_nAllocatedCount = m_nCount*2 + 8;

			//keep alignment requirement of T if memory man is used on PS3
			m_pElements = InternalRealloc(m_pElements, (m_nAllocatedCount*sizeof(T)) + overAllocBytes, nOldSizeInBytes );
			assert(m_pElements!=NULL);

			MEMSTAT_BIND_TO_CONTAINER(this, m_pElements);
    }

		//this is twice as fast on PS3, on PC there is no call to memcpy so no performance win
#if defined(PS3)
		if(sizeof(T)==1)
			*(uint8*)&m_pElements[m_nCount]=*(uint8*)&p;
		else
		if(sizeof(T)==2)
			*(uint16*)&m_pElements[m_nCount]=*(uint16*)&p;
		else
		if(sizeof(T)==4)
			*(uint32*)&m_pElements[m_nCount]=*(uint32*)&p;
		else
#endif
	    memcpy(&m_pElements[m_nCount],&p,sizeof(m_pElements[m_nCount]));
    m_nCount++;
		MEMSTAT_USAGE(begin(), (sizeof(T) * size()) + overAllocBytes);
  }

  ILINE T & AddNew()
  {
    if( m_nCount >= m_nAllocatedCount )
    {
			size_t nOldSizeInBytes = (m_nAllocatedCount*sizeof(T)) + overAllocBytes;
      m_nAllocatedCount = m_nCount*2 + 8;
      m_pElements = InternalRealloc(m_pElements,(m_nAllocatedCount*sizeof(T)) + overAllocBytes, nOldSizeInBytes);
			assert(m_pElements!=NULL);
			MEMSTAT_BIND_TO_CONTAINER(this, m_pElements);
    }

    m_nCount++;

    memset(&Last(), 0, sizeof(T));

		MEMSTAT_USAGE(begin(), (sizeof(T) * size()) + overAllocBytes);
    return Last();
  }

  void InsertBefore(const T & p, const unsigned int nBefore)
  {
    assert( nBefore>=0 && nBefore<=(unsigned int)m_nCount );
		T tmp; Add(tmp); // add empty object to increase memory buffer
#if defined(__SPU__)
		for( int i = 0 ; i < m_nCount-nBefore-1 ; ++i )
			m_pElements[nBefore+1+i] = m_pElements[nBefore+i];
#else
    memmove(&(m_pElements[nBefore+1]), &(m_pElements[nBefore]), sizeof(T)*(m_nCount-nBefore-1));
#endif
    m_pElements[nBefore] = p;
  }

  void PreAllocate(int elem_count, int nNewCount = -1)
  {
    if( elem_count > m_nAllocatedCount )
    {
			int nPrevAllocCount = m_nAllocatedCount;
			int nAddedCount = elem_count - m_nAllocatedCount;
			size_t nOldSizeInBytes = (m_nAllocatedCount*sizeof(T)) + overAllocBytes;

      m_nAllocatedCount = elem_count;

			m_pElements = InternalRealloc(m_pElements,(m_nAllocatedCount*sizeof(T)) + overAllocBytes, nOldSizeInBytes);
      assert(m_pElements);
      memset(m_pElements + nPrevAllocCount, 0, (sizeof(T)*nAddedCount) + overAllocBytes);

			MEMSTAT_BIND_TO_CONTAINER(this, m_pElements);
    }
    
    if(nNewCount>=0)
      m_nCount = nNewCount;
		MEMSTAT_USAGE(begin(), sizeof(T) * size());
  }

  inline void Delete(const int nElemId, const int nElemCount = 1)
  {
    assert( nElemId >= 0 && nElemId+nElemCount <= m_nCount );
#if defined(__SPU__)
		for( int i = 0 ; i < m_nCount-nElemId-nElemCount ; ++i )
			m_pElements[nElemId+i] = m_pElements[nElemId+nElemCount+i];
#else
    memmove(&(m_pElements[nElemId]), &(m_pElements[nElemId+nElemCount]), sizeof(T)*(m_nCount-nElemId-nElemCount));
#endif
    m_nCount-=nElemCount;
		MEMSTAT_USAGE(begin(), (sizeof(T) * size()) + overAllocBytes);
  }

  inline void DeleteFastUnsorted(const int nElemId, const int nElemCount = 1)
  {
		assert( nElemId >= 0 && nElemId+nElemCount <= m_nCount );
#if defined(__SPU__)
		for( int i = 0 ; i < nElemCount ; ++i )
			m_pElements[nElemId+i] = m_pElements[m_nCount-nElemCount+i];
#else
		memmove(&(m_pElements[nElemId]), &(m_pElements[m_nCount-nElemCount]), sizeof(T)*nElemCount);
#endif
		m_nCount-=nElemCount;
		MEMSTAT_USAGE(begin(), (sizeof(T) * size()) + overAllocBytes);
  }

  inline bool Delete(const T & del)
  {
		bool bFound = false;    
		for( int i=0; i<Count(); i++ )
    if(m_pElements[i] == del)
    { 
      Delete(i); 
      i--; 
			bFound = true;
    }
		return bFound;
  }

  ILINE  int Count() const { return m_nCount; }
  ILINE  unsigned int Size() const { return m_nCount; }
  
  ILINE  int IsEmpty() const { return m_nCount==0; }

  ILINE  T & operator [] (int i) const { assert( i>=0 && i<m_nCount ); return m_pElements[i]; }
  ILINE  T & GetAt (int i) const { assert( i>=0 && i<m_nCount ); return  m_pElements[i]; }
  ILINE  T * Get   (int i) const { assert( i>=0 && i<m_nCount ); return &m_pElements[i]; }
  ILINE  T * GetElements() { return m_pElements; }
	ILINE  const T * GetElements() const { return m_pElements; }
	ILINE  unsigned int GetDataSize() const { return m_nCount*sizeof(T); }

  T & Last() const { assert(m_nCount); return m_pElements[m_nCount-1]; }
  ILINE void DeleteLast() 
	{
		assert(m_nCount); m_nCount--;
		MEMSTAT_USAGE(begin(), (sizeof(T) * size()) + overAllocBytes);
	}

  PodArray<T>& operator = ( const PodArray<T> & source_list)
  {
    Clear();
		AddList( source_list );
    
    return *this;
  }
  
	//////////////////////////////////////////////////////////////////////////
	// Return true if arrays have the same data.
  bool Compare( const PodArray<T> &l ) const
  {
    if(Count() != l.Count())
      return 0;

    if(memcmp(m_pElements, l.m_pElements, m_nCount*sizeof(T))!=0)
      return 0;

    return 1;
  }

	// for statistics
	ILINE size_t ComputeSizeInMemory() const
	{ 
		return (sizeof(*this)+sizeof(T)*m_nAllocatedCount) + overAllocBytes; 
	}

private:
	T* InternalRealloc( T* pBuffer,size_t nNewSize, size_t nOldSize )
	{
		PREFAST_SUPPRESS_WARNING (6326)
#if defined(__SPU__)		
		T* pNewArray = (T*)CryModuleMemalign(nNewSize, __alignof__(T));
		if(pBuffer)
		{
			if(pNewArray && nOldSize)
			{				
				int nBytesToCopy = nNewSize < nOldSize ? nNewSize : nOldSize;
				memcpy( SPU_MAIN_PTR(pNewArray), SPU_MAIN_PTR(pBuffer), nBytesToCopy);
			}
			CryModuleMemalignFree(pBuffer);
		}
		return pNewArray;
#else
		if (alignof(T) > _CRY_DEFAULT_MALLOC_ALIGNMENT)
		{
			return (T*)CryModuleReallocAlign(pBuffer, nNewSize,alignof(T));
		}
		else
		{
			return (T*)CryModuleRealloc( pBuffer,nNewSize );
		}
#endif
	}

};

#endif // __CRY_POD_ARRAY_H__

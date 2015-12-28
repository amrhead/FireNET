////////////////////////////////////////////////////////////////////////////
//
//  CryEngine Source File.
//  Copyright (C), Crytek Studios, 2001-2010.
// -------------------------------------------------------------------------
//  File name:   CustomMemoryHeap.h
//  Version:     v1.00
//  Created:     17/08/2010 by Timur.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __CustomMemoryHeap_h__
#define __CustomMemoryHeap_h__
#pragma once

#include "IMemory.h"

class CCustomMemoryHeap;

//////////////////////////////////////////////////////////////////////////
class CCustomMemoryHeapBlock : public ICustomMemoryBlock
{
public:
	CCustomMemoryHeapBlock( CCustomMemoryHeap *pHeap );
	virtual ~CCustomMemoryHeapBlock();

	//////////////////////////////////////////////////////////////////////////
	// IMemoryBlock
	//////////////////////////////////////////////////////////////////////////
	virtual void *GetData();
	virtual int GetSize() { return m_nSize; }
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// ICustomMemoryBlock
	//////////////////////////////////////////////////////////////////////////
	virtual void CopyMemoryRegion( void *pOutputBuffer,size_t nOffset,size_t nSize );
	//////////////////////////////////////////////////////////////////////////

private:
	friend class CCustomMemoryHeap;
	CCustomMemoryHeap *m_pHeap;
	string m_sUsage;
	void* m_pData;
	uint32 m_nGPUHandle;
	size_t m_nSize;
};

//////////////////////////////////////////////////////////////////////////
class CCustomMemoryHeap : public ICustomMemoryHeap
{
public:

	explicit CCustomMemoryHeap(IMemoryManager::EAllocPolicy const eAllocPolicy);
	~CCustomMemoryHeap();

	//////////////////////////////////////////////////////////////////////////
	// ICustomMemoryHeap
	//////////////////////////////////////////////////////////////////////////
	virtual ICustomMemoryBlock* AllocateBlock( size_t const nAllocateSize,char const* const sUsage,size_t const nAlignment = 16 );
	virtual void GetMemoryUsage( ICrySizer *pSizer );
	virtual size_t GetAllocated();
	//////////////////////////////////////////////////////////////////////////

	void DeallocateBlock( CCustomMemoryHeapBlock* pBlock );

private:

	friend class CCustomMemoryHeapBlock;
	int m_nAllocatedSize;
	IMemoryManager::EAllocPolicy m_eAllocPolicy;
	IMemoryManager::HeapHandle m_nTraceHeapHandle;
};

#endif // __CustomMemoryHeap_h__

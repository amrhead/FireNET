//---------------------------------------------------------------------------
// Copyright 2010 Crytek GmbH
// Created by: Michael Kopietz
// Modified: -
//	
//---------------------------------------------------------------------------

#ifndef __CCRYPOOLALLOC__
#define __CCRYPOOLALLOC__

#if defined(POOLALLOCTESTSUIT)
//cheat just for unit testing on windows
#include "BaseTypes.h"
#define ILINE			inline
#endif



#if defined(LINUX) || defined(APPLE) || defined(PS3) || defined(ORBIS)
#define CPA_ALLOC							memalign
#define CPA_FREE							free
#else
#define CPA_ALLOC							_aligned_malloc
#define CPA_FREE							_aligned_free
#endif
#define CPA_ASSERT						assert
#define CPA_ASSERT_STATIC(X)	{uint8 assertdata[(X)?0:1];}
#define CPA_BREAK							__debugbreak()

#if defined(APPLE)
ILINE void* memalign(size_t nAlign, size_t nSize)
{
    void* pBlock(NULL);
    posix_memalign(&pBlock, nSize, nAlign);
    return pBlock;
}
#endif // defined(APPLE)

#include "List.h"
#include "Memory.h"
#include "Container.h"
#include "Allocator.h"
#include "Defrag.h"
#include "STLWrapper.h"
#include "Inspector.h"
#include "Fallback.h"
#if !defined(POOLALLOCTESTSUIT)
#include "ThreadSafe.h"
#endif

#undef CPA_ASSERT
#undef CPA_ASSERT_STATIC
#undef CPA_BREAK

#endif


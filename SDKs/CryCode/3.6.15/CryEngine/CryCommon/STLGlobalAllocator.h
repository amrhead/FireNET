//---------------------------------------------------------------------------
// Copyright 2010 Crytek GmbH
// Created by: Stephen Barnett
//---------------------------------------------------------------------------
#ifndef __STLGLOBALALLOCATOR_H__
#define __STLGLOBALALLOCATOR_H__

//---------------------------------------------------------------------------
// STL-compatible interface for an std::allocator using the global heap.
//---------------------------------------------------------------------------

#include <stddef.h>
#include <climits>

#include "CryMemoryManager.h"

class ICrySizer;
namespace stl
{
	template <class T>
	class STLGlobalAllocator
	{
	public:
		typedef size_t    size_type;
		typedef ptrdiff_t difference_type;
		typedef T*        pointer;
		typedef const T*  const_pointer;
		typedef T&        reference;
		typedef const T&  const_reference;
		typedef T         value_type;

		template <class U> struct rebind
		{
			typedef STLGlobalAllocator<U> other;
		};

		STLGlobalAllocator() throw()
		{
		}

		STLGlobalAllocator(const STLGlobalAllocator&) throw()
		{
		}

		template <class U> STLGlobalAllocator(const STLGlobalAllocator<U>&) throw()
		{
		}

		~STLGlobalAllocator() throw()
		{
		}

		pointer address(reference x) const
		{
			return &x;
		}

		const_pointer address(const_reference x) const
		{
			return &x;
		}

		pointer allocate(size_type n = 1, const void* hint = 0)
		{
			(void)hint;
			ScopedSwitchToGlobalHeap useGlobalHeap;
			return static_cast<pointer>(CryModuleMalloc(n * sizeof(T)));
		}

		void deallocate(pointer p, size_type n = 1)
		{
			CryModuleFree(p);
		}

		size_type max_size() const throw()
		{
			return INT_MAX;
		}
#if !defined(_LIBCPP_VERSION)
		void construct(pointer p, const T& val)
		{
			new(static_cast<void*>(p)) T(val);
		}

		void construct(pointer p)
		{
			new(static_cast<void*>(p)) T();
		}
#endif // !_LIBCPP_VERSION
 		void destroy(pointer p)
		{
			p->~T();
		}

		pointer new_pointer()
		{
			return new(allocate()) T();
		}

		pointer new_pointer(const T& val)
		{
			return new(allocate()) T(val);
		}

		void delete_pointer(pointer p)
		{
			p->~T();
			deallocate(p);
		}

		bool operator==(const STLGlobalAllocator&) {return true;}
		bool operator!=(const STLGlobalAllocator&) {return false;}

		static void GetMemoryUsage( ICrySizer *pSizer )
		{
		}
	};

	template <> class STLGlobalAllocator<void>
	{
	public:
		typedef void* pointer;
		typedef const void* const_pointer;
		typedef void value_type;
		template <class U> 
		struct rebind { typedef STLGlobalAllocator<U> other; };
	};    
}

#endif //__STLGLOBALALLOCATOR_H__

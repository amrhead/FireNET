// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef STL_SOUND_ALLOCATOR_H_INCLUDED
#define STL_SOUND_ALLOCATOR_H_INCLUDED

#define AUDIO_MEMORY_ALIGNMENT MEMORY_ALLOCATION_ALIGNMENT

class ICrySizer;

//////////////////////////////////////////////////////////////////////////
// <title STLSoundAllocator>
// Summary:
//			An STL allocator that uses the module's memory pool 
//////////////////////////////////////////////////////////////////////////
template <class T>
class STLSoundAllocator
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
		typedef STLSoundAllocator<U> other;
	};

	STLSoundAllocator() throw() { }
	STLSoundAllocator(const STLSoundAllocator&) throw() { }
	template <class U> STLSoundAllocator(const STLSoundAllocator<U>&) throw() { }

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
		return static_cast<pointer>(AUDIO_ALLOCATOR_MEMORY_POOL.AllocateRaw(n * sizeof(T), AUDIO_MEMORY_ALIGNMENT, "Audio-STL"));
	}

	void deallocate(pointer p, size_type n = 1)
	{
		(void)n;
		AUDIO_ALLOCATOR_MEMORY_POOL.Free(p);
	}

	size_type max_size() const throw()
	{
		return INT_MAX;
	}

	template<class U>
	void construct(U* p, const U& val)
	{
		new((void*)p) U(val);
	}

	template<class U>
	void construct(U* p)
	{
		new((void*)p) U();
	}

	template<class U>
	void destroy(U* p)
	{
		p->~U();
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

	bool operator==(const STLSoundAllocator&) {return true;}
	bool operator!=(const STLSoundAllocator&) {return false;}

	static void GetMemoryUsage( ICrySizer *pSizer ) { }
};

#endif

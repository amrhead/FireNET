#pragma once 
#include <stdlib.h> // for malloc and free

namespace Serialization
{

// Black box is used to store opaque data blobs in a format internal to
// specific Archive. For example it can be used to store sections of the JSON
// or binary archive.
//
// This is useful for the Editor to store portions of files with unfamiliar
// structure.
//
// We store deallocation function here so we can safely pass the blob
// across DLLs with different memory allocators.
struct SBlackBox
{
	const char* format;
	void* data;
	size_t size;
	typedef void(*FreeFunction)(void*);
	FreeFunction freeFunction;

	SBlackBox()
	: format("")
	, data(0)
	, size(0)
	, freeFunction(0)
	{
	}

	SBlackBox(const SBlackBox& rhs)
	: format("")
	, data(0)
	, size(0)
	, freeFunction(0)
	{
		*this = rhs;
	}

	void set(const char* format, const void* data, size_t size)
	{
		if (data && freeFunction) 
		{
			freeFunction(this->data);
			this->data = 0;
			this->size = 0;
			freeFunction = 0;
		}
		this->format = format;
		if (data && size) 
		{
			this->data = malloc(size);
			memcpy(this->data, data, size);
			this->size = size;
			freeFunction = &free;
		}
	}

	SBlackBox& operator=(const SBlackBox& rhs)
	{
		set(rhs.format, rhs.data, rhs.size);
		return *this;
	}

	~SBlackBox()
	{
		set("", 0, 0);
	}
};

}

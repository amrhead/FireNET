#pragma once

#include "Serializer.h"

// ---------------------------------------------------------------------------

namespace Serialization {

typedef int(*AddRefFunc)(void*);
typedef int(*DecRefFunc)(void*);

// represents a reference to the persistent object
class Object
{
public:
	Object()
	: address_(0)
	, addRefFunc_(0)
	, decRefFunc_(0)
	, serializeFunc_(0)
	{
	}

	Object(const Object& o)
	: address_(o.address_)
	, type_(o.type_)
	, addRefFunc_(o.addRefFunc_)
	, decRefFunc_(o.decRefFunc_)
	, serializeFunc_(o.serializeFunc_)
	{
		addRef();
	}

	Object(const SStruct& ser)
	: address_(ser.pointer())
	, type_(ser.type())
	, addRefFunc_(0)
	, decRefFunc_(0)
	, serializeFunc_(ser.serializeFunc())
	{
	}

	Object(void* address, const TypeID& type, AddRefFunc addRefFunc, DecRefFunc decRefFunc, SerializeStructFunc serializeFunc)
	: address_(address)
	, type_(type)
	, addRefFunc_(addRefFunc)
	, decRefFunc_(decRefFunc)
	, serializeFunc_(serializeFunc)
	{
		addRef();
	}

	~Object()
	{
		if (address_) {
			decRef();
			address_ = 0;
		}
	}

	void* address() const{ return address_; }
	const TypeID& type() const{ return type_; }
	bool isSet() { return serializeFunc_ != 0; }

	int addRef()
	{
		if (!addRefFunc_)
			return 1;
		if(!address_)
			return -1;
		return addRefFunc_(address_);
	}

	int decRef()
	{
		if (!decRefFunc_)
			return 1;
		if(!address_)
			return -1;
		return decRefFunc_(address_);
	}

	bool operator()(IArchive& ar) const
	{
		if (!serializeFunc_ || !address_)
			return false;
		return serializeFunc_(address_, ar);
	}

	SStruct serializer() const {
		return SStruct(type_, address_, 0, serializeFunc_);
	}

	Object& operator=(const Object& o)
	{
		if (this == &o)
			return *this;

		if (address_)
			decRef();

		address_ = o.address_;
		type_ = o.type_;
		addRefFunc_ = o.addRefFunc_;
		decRefFunc_ = o.decRefFunc_;
		serializeFunc_ = o.serializeFunc_;

		addRef();
		return *this;
	}

private:
	void* address_;
	TypeID type_;
	AddRefFunc addRefFunc_;
	DecRefFunc decRefFunc_;
	SerializeStructFunc serializeFunc_;
};

}

// ---------------------------------------------------------------------------

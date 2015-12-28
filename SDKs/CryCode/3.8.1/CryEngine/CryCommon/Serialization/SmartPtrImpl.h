#pragma once

#include "SmartPtr.h"
#include <Serialization/Serializer.h>
#include "ClassFactory.h"

// Exposes _smart_ptr<> as serializeable type for Serialization::IArchive
template<class T>
class SmartPtrSerializer : public Serialization::IPointer
{
public:
	SmartPtrSerializer(_smart_ptr<T>& ptr)
	: m_ptr(ptr)
	{}

	const char* registeredTypeName() const override
	{
		if (m_ptr)
			return Serialization::ClassFactory<T>::the().getRegisteredTypeName(m_ptr.get());
		else
			return "";
	}

	void create(const char* registeredTypeName) const override
	{
		CRY_ASSERT(!m_ptr || m_ptr->NumRefs() == 1);
		if (registeredTypeName && registeredTypeName[0] != '\0')
			m_ptr.reset(Serialization::ClassFactory<T>::the().create(registeredTypeName));
		else
			m_ptr.reset((T*)0);
	}
	Serialization::TypeID baseType() const{ return Serialization::TypeID::get<T>(); }
	virtual Serialization::SStruct serializer() const{ return Serialization::SStruct(*m_ptr); }
	void* get() const{ return reinterpret_cast<void*>(m_ptr.get()); }
	const void* handle() const { return &m_ptr; }
	Serialization::TypeID pointerType() const { return Serialization::TypeID::get<_smart_ptr<T> >(); }
	Serialization::IClassFactory* factory() const{ return &Serialization::ClassFactory<T>::the(); }
protected:
	_smart_ptr<T>& m_ptr;
};

template<class T>
bool Serialize(Serialization::IArchive& ar, _smart_ptr<T>& ptr, const char* name, const char* label)
{
	SmartPtrSerializer<T> serializer(ptr);
	return ar(static_cast<Serialization::IPointer&>(serializer), name, label);
}

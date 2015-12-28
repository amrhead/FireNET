#pragma once

#include <BoostHelpers.h>
#include <Serialization/Serializer.h>

#include "ClassFactory.h"

template <class T>
class BoostSharedPtrSerializer : public Serialization::IPointer
{
public:
	BoostSharedPtrSerializer( boost::shared_ptr<T>& ptr )
		: m_ptr( ptr )
	{
	}

	Serialization::TypeID type() const override
	{
		if( m_ptr )
			return factoryOverride().getTypeID( m_ptr.get() );
		else
			return Serialization::TypeID();
	}

	void create( Serialization::TypeID type ) const override
	{
		CRY_ASSERT( !m_ptr || m_ptr.use_count() == 1 );
		if( type )
			m_ptr.reset( factoryOverride().create( type ) );
		else
			m_ptr.reset();
	}

	Serialization::TypeID baseType() const override
	{
		return Serialization::TypeID::get<T>();
	}

	virtual Serialization::SStruct serializer() const override
	{
		return Serialization::SStruct( *m_ptr );
	}

	void* get() const
	{
		return reinterpret_cast<void*>( m_ptr.get() );
	}

	const void* handle() const
	{
		return &m_ptr;
	}

	Serialization::TypeID pointerType() const override
	{
		return Serialization::TypeID::get<boost::shared_ptr<T>>();
	}

	Serialization::ClassFactory<T>* factory() const override
	{
		return &factoryOverride();
	}

	virtual Serialization::ClassFactory<T>& factoryOverride() const 
	{
		return Serialization::ClassFactory<T>::the(); 
	}

protected:
	boost::shared_ptr<T>& m_ptr;
};

namespace boost
{
	template <class T>
	bool Serialize( Serialization::IArchive& ar, boost::shared_ptr<T>& ptr, const char* name, const char* label )
	{
		BoostSharedPtrSerializer<T> serializer( ptr );
		return ar( static_cast<Serialization::IPointer&>( serializer ), name, label );
	}
}

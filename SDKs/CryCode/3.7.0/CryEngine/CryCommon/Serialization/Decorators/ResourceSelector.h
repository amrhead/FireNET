#pragma once

namespace Serialization
{

struct IResourceSelector
{
	const char* resourceType;

	virtual ~IResourceSelector() {}
	virtual const char* GetValue() const = 0;
	virtual void SetValue( const char* s ) = 0;
	virtual int GetId() const{ return -1; }
	virtual const void* GetHandle() const = 0;
	virtual Serialization::TypeID GetType() const = 0;
};

// Provides a way to annotate resource reference so different UI can be used
// for them. See IResourceSelector.h to see how selectors for specific types
// are registered.
//
// TString could be SCRCRef or CCryName as well.
//
// Do not use this class directly, instead use function that wraps it for
// specific type, see Resources.h for example.
template<class TString>
struct ResourceSelector : IResourceSelector
{
	TString& value;

	const char* GetValue() const { return value.c_str(); } 
	void SetValue( const char* s ) { value = s; }
	const void* GetHandle() const { return &value; }
	Serialization::TypeID GetType() const { return Serialization::TypeID::get<TString>(); }

	ResourceSelector(TString& value, const char* resourceType)
	: value(value)
	{
		this->resourceType = resourceType;
	}
};

struct ResourceSelectorWithId : IResourceSelector
{
	string& value;
	int id;

	const char* GetValue() const { return value.c_str(); } 
	void SetValue( const char* s ) { value = s; }
	int GetId() const { return id; }
	const void* GetHandle() const { return &value; }
	Serialization::TypeID GetType() const { return Serialization::TypeID::get<string>(); }

	ResourceSelectorWithId(string& value, const char* resourceType, int id)
	: value(value)
	, id(id)
	{
		this->resourceType = resourceType;
	}
};

template<class T>
bool Serialize(IArchive& ar, ResourceSelector<T>& value, const char* name, const char* label)
{
	if ( ar.IsEdit() )
		return ar(Serialization::SStruct::ForEdit(static_cast<IResourceSelector&>(value)), name, label);
	else
		return ar(value.value, name, label);
}

inline bool Serialize(IArchive& ar, ResourceSelectorWithId& value, const char* name, const char* label)
{
	if ( ar.IsEdit() )
		return ar(Serialization::SStruct::ForEdit(static_cast<IResourceSelector&>(value)), name, label);
	else
		return ar(value.value, name, label);
}

}

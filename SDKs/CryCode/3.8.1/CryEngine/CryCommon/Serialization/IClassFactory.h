// Copyright (c) 2012 Crytek GmbH
// Authors: Evgeny Andreeshchev, Alexander Kotliar
// Based on: Yasli - the serialization library.

#pragma once
#include <map>

#include "Serialization/Assert.h"
#include "Serialization/TypeID.h"

namespace Serialization{

class IArchive;
class TypeDescription
{
public:
	TypeDescription(const char* name, const char *label)
	: name_(name)
	, label_(label)
	{
	}
	const char* name() const{ return name_; }
	const char* label() const{ return label_; }

protected:
	const char* name_;
	const char* label_;
};

class IClassFactory
{
public: 
	IClassFactory(TypeID baseType)
	: baseType_(baseType)
	, nullLabel_(0)
	{
	}

	virtual ~IClassFactory() { }

	virtual size_t size() const = 0;
	virtual const TypeDescription* descriptionByIndex(int index) const = 0;	
	virtual const TypeDescription* descriptionByRegisteredName(const char* typeName) const = 0;
	virtual const char* findAnnotation(const char* registeredTypeName, const char* annotationName) const = 0;
	virtual void serializeNewByIndex(IArchive& ar, int index, const char* name, const char* label) = 0;

	bool setNullLabel(const char* label){ nullLabel_ = label ? label : ""; return true; }
	const char* nullLabel() const{ return nullLabel_; }
protected:
	TypeID baseType_;
	const char* nullLabel_;
};


struct TypeNameWithFactory
{
	string registeredName;
	IClassFactory* factory;

	TypeNameWithFactory(const char* registeredName, IClassFactory* factory = 0)
	: registeredName(registeredName)
	, factory(factory)
	{
	}
};

bool Serialize(Serialization::IArchive& ar, Serialization::TypeNameWithFactory& value, const char* name, const char* label);

}

// vim:ts=4 sw=4:
